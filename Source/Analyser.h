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
    Analyser(defSettings *i, juce::String tName) : juce::Thread (tName)
    {
        cS = i;
        averager.clear();
    }

    ~Analyser() override = default;

    void setupAnalyser(int audioFifoSize, Type sampleRateToUse, SonoImage* sImg)
    {
        sonoImage = sImg;
        sampleRate = sampleRateToUse;
        audioFifo.setSize(1, audioFifoSize);
        abstractFifo.setTotalSize(audioFifoSize);
        startThread();
    }

    void addAudioData (const juce::AudioBuffer<Type>& buffer, int startChannel, int numChannels)
    {
        cChannel = startChannel;
        if (abstractFifo.getFreeSpace() < buffer.getNumSamples()) { return; }

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
                {
                    const juce::ScopedLock lockedForWriting(pathCreationLock);
                    averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples(), -1.0f);
                    averager.copyFrom(averagerPtr, 0, fftBuffer.getReadPointer(0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                    averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples());
                    if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;
                }
                //newDataAvailable = true;

                if ( cS->mode == 2 && cChannel < 2) {
                    createPath (sonogramLine);
                    sonoImage->setAnalyserPath(cChannel, ld.ldata);
                    
                    //const juce::ScopedLock lockedForDraw(pathDrawLock);
                    //DBG("Analyzer: " << cChannel);
                    sonoImage->addLineSono(ld.cacheSize, cChannel);
                    //const int a = sonoImage->getCurLine();
                    
                    //if (cChannel) {
                    //}
                }
            }

            if (abstractFifo.getNumReady() < fft.getSize()) { waitForData.wait(100);  }
        }
    }
    
    void createPath (juce::Path& p)
    {
        lc = ld.genCacheData(averager.getNumSamples(),
                        cS->newW,
                        cS->slope,
                        sampleRate,
                        fftSize,
                        cS->minFreq);

        const bool sono = (cS->mode == 2) ? true : false;

        if (!sono) {
            p.clear();
            p.preallocateSpace(8 + ld.numSmpls * 3);
            p.startNewSubPath(0.0f, cS->newH);
        }

        const float infinity = cS->floor;
        const float hmin = sono ? 0.0f : cS->newH;
        const float hmax = sono ? 1.0f : sonoImage->scaleTopLineHeightFloat;

        const juce::ScopedLock lockedForReading (pathCreationLock);
        const auto* fftData = averager.getReadPointer (0);
        //int sizeLine = 0;

        //const float minFreqMinusOne = cS->minFreq - 1;
        //const float maxFreq = cS->maxFreq;

        const int n = cS->setLiner ? ld.freqIndexSizeLin : ld.freqIndexSizeLog;
        const FreqIndex* fi = cS->setLiner ? ld.freqIndexLin : ld.freqIndexLog;
        const xCordCache* xcord = cS->setLiner ? ld.xcrdlin : ld.xcrdlog;

        for (int i = 0; i < n; ++i)
        {
            const int a = fi[i].v;
            //const float freq = lc[a].freq;
            const float gain = lc[a].slopeGain;

            //if (freq < minFreqMinusOne) { continue; }
            //if (freq > maxFreq)         { continue; }

            //const float x = xcord[a].x;

            float cc=0.f;
            for (int c = a; c < fi[i+1].v; c++) {
                cc = fftData[c] > cc ? fftData[c] : cc;
            }

            // need approxymator for y
            const float y = juce::jmap ( 
                    juce::Decibels::gainToDecibels ( cc, (infinity - gain) ) + gain,
                            infinity, 0.0f, hmin, hmax );

            if (sono)  {
                ld.ldata[i].x = xcord[a].x;
                ld.ldata[i].y = y;
            }
            else { 
                p.lineTo(xcord[a].x, y);
            }
        }

        ld.cacheSize = n;
    }

    //bool checkForNewData()
    //{
    //    auto available = newDataAvailable.load();
    //    newDataAvailable.store (false);
    //    return available;
    //}

private:

    LineData ld;
    sLineCache* lc = nullptr;

    juce::WaitableEvent   waitForData;
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
