/*
  ==============================================================================

    Analyser.h
    Created: 12 Jul 2018 11:27:50pm
    Author:  Daniel Walz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "settings.h"
#include "SonoImageController.h"

//==============================================================================

template<typename Type>
class Analyser : public juce::Thread
{
public:
    Analyser(defSettings *i) : juce::Thread ("AudioAnalyser")
    {
        cS = i;
        averager.clear();
    }

    ~Analyser() override = default;

    void setupAnalyser(int audioFifoSize, Type sampleRateToUse, SonoImage* sI)
    {
        sonoImage = sI;
        sampleRate = sampleRateToUse;
        audioFifo.setSize(1, audioFifoSize);
        abstractFifo.setTotalSize(audioFifoSize);
        createFreqXCache();
        startThread();
    }

    void addAudioData (const juce::AudioBuffer<Type>& buffer, int startChannel, int numChannels)
    {
        
        cChannel = startChannel;
        if (abstractFifo.getFreeSpace() < buffer.getNumSamples())
            return;

        if (winMet != cS->winMet) {
            winMet = cS->winMet;
            windowing.fillWindowingTables(size_t (fftSize), winMet);
        }
        
        if (fftOrder != *cS->fftOrder) {
            fftOrder = *cS->fftOrder;
            fftSize  = 1 << fftOrder;
            averager.clear();
            
            fftBuffer.setSize(1, fftSize * 2);
            averager.setSize(5, fftSize / 2);
            fft = juce::dsp::FFT(fftOrder);
            windowing.fillWindowingTables(size_t (fftSize), cS->winMet);
            createFreqXCache();
        }
        
        int start1, block1, start2, block2;
        abstractFifo.prepareToWrite (buffer.getNumSamples(), start1, block1, start2, block2);
        audioFifo.copyFrom (0, start1, buffer.getReadPointer (startChannel), block1);
        if (block2 > 0)
            { audioFifo.copyFrom (0, start2, buffer.getReadPointer (startChannel, block1), block2); }

        for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
        {
            if (block1 > 0) audioFifo.addFrom (0, start1, buffer.getReadPointer (channel), block1);
            if (block2 > 0) audioFifo.addFrom (0, start2, buffer.getReadPointer (channel, block1), block2);
        }
        abstractFifo.finishedWrite (block1 + block2);
        waitForData.signal();
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (abstractFifo.getNumReady() >= fft.getSize())
            {
                fftBuffer.clear();

                int start1, block1, start2, block2;
                abstractFifo.prepareToRead (fftSize, start1, block1, start2, block2);
                if (block1 > 0) fftBuffer.copyFrom (0, 0, audioFifo.getReadPointer (0, start1), block1);
                if (block2 > 0) fftBuffer.copyFrom (0, block1, audioFifo.getReadPointer (0, start2), block2);
                abstractFifo.finishedRead ((block1 + block2) / *cS->overlap );

                windowing.multiplyWithWindowingTable (fftBuffer.getWritePointer (0), size_t (fft.getSize()));
                fft.performFrequencyOnlyForwardTransform (fftBuffer.getWritePointer (0));

                juce::ScopedLock lockedForWriting (pathCreationLock);
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples(), -1.0f);
                averager.copyFrom (averagerPtr, 0, fftBuffer.getReadPointer (0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples());
                if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;

                //newDataAvailable = true;

                if ( cS->mode == 2 && cChannel < 2 ) {
                    createPath (sonogramLine);
                    sonoImage->setAnalyserPath(cChannel, &sonogramLine);
                }
            }

            if (abstractFifo.getNumReady() < fft.getSize())
                waitForData.wait (100);
        }
    }
    
    void createPath (juce::Path& p)
    {
        reinit();

        p.clear();
        
        p.preallocateSpace (8 + averager.getNumSamples() * 3);
        
        juce::ScopedLock lockedForReading (pathCreationLock);
        const auto* fftData = averager.getReadPointer (0);
        
        p.startNewSubPath (0.0f, height);

        const float sumDb = (cS->slope * 12.0);
        const float xkoef = sumDb / width;
        const float infinity = cS->floor;
        float gain = 0.0f;
        const float hmin = (cS->mode == 2) ? 0.0 : height;
        const float hmax = (cS->mode == 2) ? 1.0 : sonoImage->scaleTopLineHeightFloat;
        for (int i = 0; i < averager.getNumSamples(); ++i)
        {
            const float freq = freq_cache[i];
            const float x = x_freq_cache[i];
            const float y = juce::jmap ( juce::Decibels::gainToDecibels ( fftData[i], 
                            (infinity - gain) ) + gain,
                            infinity, 0.0f, hmin, hmax );
            
            if (freq < minFreq) { p.lineTo (0.0f, y); continue; };
            if (freq > maxFreq) { p.lineTo (width, hmin); continue; };
            gain = x * xkoef;
            p.lineTo (x, y);
        }
    }

    //bool checkForNewData()
    //{
    //    auto available = newDataAvailable.load();
    //    newDataAvailable.store (false);
    //    return available;
    //}

private:

    void createFreqXCache()
    {
        if (x_freq_cache != nullptr) {
            delete(freq_cache);
            delete(x_freq_cache);
        }

        width = cS->newW;
        height = cS->newH;
        minFreq = cS->minFreq;
        maxFreq = sampleRate * 0.5f;
        scale = cS->setLiner;

        const int cSmpls = averager.getNumSamples();

        freq_cache = new float[cSmpls];
        x_freq_cache = new float [cSmpls];

        for (int i = 0; i < cSmpls; ++i) {
            const float freq = (sampleRate * i) / fftSize;
            const float b = std::log(maxFreq / minFreq) / (maxFreq - minFreq);
            const float a = maxFreq / std::exp(maxFreq * b);
            const float position = std::log(freq / a) / b;
            float x = (width * position / (maxFreq - minFreq));

            if (cS->setLiner) {
                x = (width * freq / (maxFreq - minFreq));
            }
            freq_cache[i] = freq;
            x_freq_cache[i] = x;
        }
    }

    void reinit() {
        if (width != cS->newW || height != cS->newH || scale != cS->setLiner) {
            createFreqXCache();
        }
    }

    float width;
    float height;
    float minFreq;
    float maxFreq;
    bool scale;

    float* freq_cache = nullptr;
    float* x_freq_cache = nullptr;

    juce::WaitableEvent waitForData;
    juce::CriticalSection pathCreationLock;

    SonoImage* sonoImage;
    juce::Path sonogramLine;
    Type sampleRate {};
    int cChannel;
    bool readyChFlag = false;
    
    defSettings* cS;
    
    int fftOrder   = 12;
    int fftSize    = 1 << fftOrder;
    juce::dsp::WindowingFunction<float>::WindowingMethod winMet = juce::dsp::WindowingFunction<float>::hann;
    
    juce::dsp::FFT fft                           { fftOrder };
    juce::dsp::WindowingFunction<Type> windowing { size_t (fftSize), juce::dsp::WindowingFunction<Type>::hann, true };

    juce::AudioBuffer<float> fftBuffer           { 1, fftSize * 2 };

    juce::AudioBuffer<float> averager            { 5, fftSize / 2 };

    int averagerPtr = 1;

    juce::AbstractFifo abstractFifo              { 48000 };
    juce::AudioBuffer<Type> audioFifo;

    //std::atomic<bool> newDataAvailable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyser)
};
