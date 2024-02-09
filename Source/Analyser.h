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

//==============================================================================

template<typename Type>
class Analyser : public juce::Thread
{
public:
    Analyser(DSETTINGS *i) : juce::Thread ("AudioAnalyser")
    {
        cS = i;
        averager.clear();
    }

    ~Analyser() override = default;

    void addAudioData (const juce::AudioBuffer<Type>& buffer, int startChannel, int numChannels)
    {
        
        cChannel = startChannel;
        if (abstractFifo.getFreeSpace() < buffer.getNumSamples())
            return;
        
        if (winMet != cS->winMet) {
            winMet = cS->winMet;
            windowing.fillWindowingTables(size_t (fftSize), winMet);
        }
        
        if (fftOrder != cS->fftOrder) {
            fftOrder = cS->fftOrder;
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

    void setupAnalyser (int audioFifoSize, Type sampleRateToUse, juce::Path *iChPath)
    {
        sampleRate = sampleRateToUse;
        sonogramLine = iChPath;
        audioFifo.setSize (1, audioFifoSize);
        abstractFifo.setTotalSize (audioFifoSize);
        startThread ();
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
                abstractFifo.finishedRead ((block1 + block2) / cS->overlap );

                windowing.multiplyWithWindowingTable (fftBuffer.getWritePointer (0), size_t (fft.getSize()));
                fft.performFrequencyOnlyForwardTransform (fftBuffer.getWritePointer (0));

                juce::ScopedLock lockedForWriting (pathCreationLock);
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples(), -1.0f);
                averager.copyFrom (averagerPtr, 0, fftBuffer.getReadPointer (0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples());
                if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;

                newDataAvailable = true;
                
                if ( cS->mode == 2 && cChannel < 2 ) {
                    createPath (*sonogramLine);
                    if(cChannel == 0 ) {cS->chL = true;} else {cS->chR = true;}
                    if(cS->chL && cS->chR) {
                        cS->chL = cS->chR = false;
                        drawNextLineOfSonogram();
                    }
                }
            }

            if (abstractFifo.getNumReady() < fft.getSize())
                waitForData.wait (100);
        }
    }
    
    void drawNextLineOfSonogram()
    {        
        if(cS->resize) {
            if(sonogramImage != nullptr) { sonogramImage->~Image(); }
            sonogramImage = new juce::Image(juce::Image::ARGB, cS->newW, cS->newH-sonoTopLineHeight, true);
//            sonogramImage->duplicateIfShared(); //?
            cS->resize = false;
        }
        int iHeight = sonogramImage->getHeight() - 1;
        
        sonogramImage->moveImageSection (0, 0, 0, 1,
                                            sonogramImage->getWidth(),
                                            iHeight);
        
        juce::PathFlatteningIterator analyserPointL ( *SanalyserPathCh1L );
        juce::PathFlatteningIterator analyserPointR ( *SanalyserPathCh1R );
        
        int x = 0.0f;
        int xL1,yL1,xL2,yL2, xR1,yR1,xR2,yR2;

        float levelL, levelR;
        float bxL,byL, bxR, byR;
        float lvlL, lvlR;
        float colorL = juce::jmap( cS->colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f );
        float colorR = juce::jmap( cS->colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f );

        do
        {
            xL1 = analyserPointL.x1;
            yL1 = analyserPointL.y1;
            xL2 = analyserPointL.x2;
            yL2 = analyserPointL.y2;
            
            xR1 = analyserPointR.x1;
            yR1 = analyserPointR.y1;
            xR2 = analyserPointR.x2;
            yR2 = analyserPointR.y2;
            
            bxL = xL2-xL1;
            byL = yL2-yL1;
            
            bxR = xR2-xR1;
            byR = yR2-yR1;
            
            lvlL = yL1;
            lvlR = yR1;

            float lkoefL = byL / bxL;
            float lkoefR = byR / bxR;
            
            juce::Colour bgL;
            juce::Colour bgR;
            
            for (int i = 0; i < bxL; ++i) {
                x++;

                if (cS->ch1L) {
                    levelL  = juce::jmap (lvlL, 0.0f, (float)iHeight, 1.0f, 0.0f);
                    bgL = juce::Colour::fromHSL(colorL, 1.0, levelL, levelL);
                } else {
                    bgL = juce::Colours::black;
                }
                if (cS->ch1R) {
                    levelR  = juce::jmap (lvlR, 0.0f, (float)iHeight, 1.0f, 0.0f);
                    bgR = juce::Colour::fromHSL(colorR, 1.0, levelR, levelR);
                } else {
                    bgR = juce::Colours::black.withAlpha(0.0f);
                }
                juce::Colour newC = bgL.overlaidWith(bgR);
                
                sonogramImage->setPixelAt (x, iHeight, newC);
                lvlL = lvlL + lkoefL;
                lvlR = lvlR + lkoefR;
            }
        } while(analyserPointR.next() && analyserPointL.next());
    }
    
    void drawSono (juce::Graphics &g, const juce::Rectangle<float> bounds) {
        juce::Rectangle<float> b = bounds.withTop(sonoTopLineHeight);
        g.drawImage ( *sonogramImage, b );
    }
    
    void createPath (juce::Path& p)
    {
        p.clear();
        float minFreq = cS->minFreq;
        p.preallocateSpace (8 + averager.getNumSamples() * 3);
        
        juce::ScopedLock lockedForReading (pathCreationLock);
        const auto* fftData = averager.getReadPointer (0);
        
        p.startNewSubPath (0.0f, cS->newH);

        float width  = cS->newW;
        float height = cS->newH;
        const float maxFreq = sampleRate * 0.5f;
        
        const float sumDb = (cS->slope * 12.0);
        const float xkoef = sumDb / width;
        float gain = 0.0f;
        
        for (int i = 0; i < averager.getNumSamples(); ++i)
        {
            float x, y;
            const float freq = (sampleRate * i) / fftSize;

            const float infinity = cS->floor;
            y = juce::jmap ( juce::Decibels::gainToDecibels ( fftData[i], (infinity - gain) ) + gain,
                            infinity, 0.0f, height, 20.f );
            
            
            if (freq < minFreq) { p.lineTo (0.0f, y); continue; };
            if (freq > maxFreq) { p.lineTo (width, height); continue; };

            const float b = std::log(maxFreq / minFreq) / (maxFreq - minFreq);
            const float a = maxFreq / std::exp(maxFreq * b);
            const float position = std::log(freq / a) / b;
            x = (width * position / (maxFreq - minFreq));
            gain = x * xkoef;
            
            if (cS->setLiner) {
                x = ( width * freq / (maxFreq - minFreq) );
            }
            
//            const float infinity = cS->floor;
            y = juce::jmap ( juce::Decibels::gainToDecibels ( fftData[i], (infinity - gain) ) + gain,
                            infinity, 0.0f, height, 20.f );
            p.lineTo (x, y);
        }
    }

    bool checkForNewData()
    {
        auto available = newDataAvailable.load();
        newDataAvailable.store (false);
        return available;
    }

private:

    juce::WaitableEvent waitForData;
    juce::CriticalSection pathCreationLock;

    juce::Path *sonogramLine;
    Type sampleRate {};
    int cChannel;
    bool readyChFlag = false;
    
    int sonoTopLineHeight = 20;

    DSETTINGS* cS;
    
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

    std::atomic<bool> newDataAvailable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyser)
};
