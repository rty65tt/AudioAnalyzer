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
#include "CacheData.h"
#include "SonoImageController.h"

//==============================================================================

template<typename Type>
class Analyser : public juce::Thread
{
public:
	Analyser(std::shared_ptr<CacheManager> cm, const int nChannel, juce::String tName)
		: juce::Thread(tName)
		, cChannel(nChannel)
		, cacheMngr(cm)
	{
		averager.clear();
	}

	~Analyser() override = default;

	void setupAnalyser(int audioFifoSize, Type sampleRateToUse, SonoImage* sImg)
	{
		sonoImage  = sImg;
		sampleRate = sampleRateToUse;
		audioFifo.setSize(1, audioFifoSize);
		abstractFifo.setTotalSize(audioFifoSize);
		startThread();
	}

	void addAudioData(const juce::AudioBuffer<Type>& buffer, int startChannel, int numChannels)
	{
		if (abstractFifo.getFreeSpace() < buffer.getNumSamples()) { return; }

		int start1, block1, start2, block2;
		abstractFifo.prepareToWrite(buffer.getNumSamples(), start1, block1, start2, block2);
		audioFifo.copyFrom(0, start1, buffer.getReadPointer(startChannel), block1);
		if (block2 > 0)
		{
			audioFifo.copyFrom(0, start2, buffer.getReadPointer(startChannel, block1), block2);
		}

		for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
		{
			if (block1 > 0) audioFifo.addFrom(0, start1, buffer.getReadPointer(channel), block1);
			if (block2 > 0) audioFifo.addFrom(0, start2, buffer.getReadPointer(channel, block1), block2);
		}
		abstractFifo.finishedWrite(block1 + block2);
		waitForData.signal();
	}

	void run() override
	{
		while (!threadShouldExit())
		{
			if (abstractFifo.getNumReady() >= fft.getSize())
			{
				ld = cacheMngr->getCacheData();
				if (winMet != cacheMngr->dp_ptr->winMet || fftOrder != *cacheMngr->dp_ptr->fftOrder) {
					//DBG("\n\n\n### >>>>>>>>>> change WIN or FFT on Channel: " << cChannel);
					winMet = cacheMngr->dp_ptr->winMet;

					fftOrder = *cacheMngr->dp_ptr->fftOrder;
					fftSize = 1 << fftOrder;
					fftBuffer.setSize(1, fftSize * 2);
					averager.setSize(5, fftSize / 2);
					averager.clear();
					fft = juce::dsp::FFT(fftOrder);

					windowing.fillWindowingTables(size_t(fftSize), winMet);
				}
				fftBuffer.clear();

				int start1, block1, start2, block2;
				abstractFifo.prepareToRead(fftSize, start1, block1, start2, block2);
				if (block1 > 0) fftBuffer.copyFrom(0, 0, audioFifo.getReadPointer(0, start1), block1);
				if (block2 > 0) fftBuffer.copyFrom(0, block1, audioFifo.getReadPointer(0, start2), block2);
				abstractFifo.finishedRead((block1 + block2) / *cacheMngr->dp_ptr->overlap);

				if (cacheMngr->dp_ptr->gain)
				{
					fftBuffer.applyGain(juce::Decibels::decibelsToGain(cacheMngr->dp_ptr->gain));
				}

				//fftBuffer.applyGain(juce::Decibels::decibelsToGain(cS->gain - (cS->slope * 7)));

				windowing.multiplyWithWindowingTable(fftBuffer.getWritePointer(0), size_t(fft.getSize()));
				fft.performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));
				{
					const juce::ScopedLock lockedForWriting(pathCreationLock);
					averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples(), -1.0f);
					averager.copyFrom(averagerPtr, 0, fftBuffer.getReadPointer(0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
					averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples());
					if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;
				}

				//newDataAvailable = true;
				updateAvailable = true;
				
				if (cacheMngr->dp_ptr->mode == 2 && cChannel < 2) {

					ldata = linebuffer->getLineBuffer(averager.getNumSamples());
					createPath(sonogramLine);

					sonoImage->setAnalyserPath(cChannel, ldata);

					sonoImage->addLineSono(ld->cacheSize, cChannel);
				}
			//DBG("      run END   channel: " << cChannel);
			}

			if (abstractFifo.getNumReady() < fft.getSize()) { waitForData.wait(100); }
		}
	}

	void createPath(juce::Path& p)
	{
		//DBG("   createPath START channel: " << cChannel);

		const bool sono = (cacheMngr->dp_ptr->mode == 2) ? true : false;
		const float infinity = cacheMngr->dp_ptr->floor;
		const float hmin = sono ? 0.0f : cacheMngr->dp_ptr->newH;
		const float hmax = sono ? 1.0f : sonoImage->scaleTopLineHeightFloat;

		if (!sono) {
			p.clear();
			p.preallocateSpace(8 + ld->numSmpls * 3);
			p.startNewSubPath(0.0f, ld->cWidth);
			p.lineTo(0.f, hmin);
		}


		const juce::ScopedLock lockedForReading(pathCreationLock); ///??????
		if (!cacheMngr->dp_ptr->channels[cChannel]) { averager.clear(); }  ////???????
		const auto* fftData = averager.getReadPointer(0);

		const int n = cacheMngr->dp_ptr->setLiner ? ld->freqIndexSizeLin : ld->freqIndexSizeLog;
		const FreqIndex* fi = cacheMngr->dp_ptr->setLiner ? ld->freqIndexLin : ld->freqIndexLog;
		const xCordCache* xcord = cacheMngr->dp_ptr->setLiner ? ld->xcrdlin : ld->xcrdlog;
		const sLineCache* g = ld->lineCache;

		for (int i = 0; i < n; ++i)
		{
			const int a = fi[i].v;
			const float gain = g[a].slopeGain;

			float cc = 0.f;
			for (int c = a; c < fi[i + 1].v; ++c) { // find max level for freq range
				cc = fftData[c] > cc ? fftData[c] : cc;
			}

			const float y = juce::jmap(
				juce::Decibels::gainToDecibels(cc, (infinity - gain)) + gain,
				infinity, 0.0f, hmin, hmax);

			if (sono) {
				ldata->x[i] = xcord[a].x;
				ldata->bx[i] = fi[i].bx;
				ldata->y[i] = y;
			}
			else {
				p.lineTo(xcord[a].x, y);
			}
		}

		ld->cacheSize = n;   //////??????????
		//DBG("      createPath END channel: " << cChannel);
	}

	//bool checkForNewData()
	//{
	//    auto available = newDataAvailable.load();
	//    newDataAvailable.store (false);
	//    return available;
	//}
	bool checkUpdate()
	{
	    auto available = updateAvailable;
		updateAvailable = false;
	    return available;
	}

	std::shared_ptr<CacheManager> cacheMngr = nullptr;
	std::shared_ptr<LineData> ld = nullptr;
private:

	juce::WaitableEvent   waitForData;
	juce::CriticalSection pathCreationLock;


	std::shared_ptr<LineDataBuffer> linebuffer = std::make_shared<LineDataBuffer>();
	std::shared_ptr<LineChData> ldata = nullptr;
	sLineCache* lc = nullptr; ////??????

	SonoImage* sonoImage;
	juce::Path sonogramLine;
	Type sampleRate{};
	int  cChannel;
	bool readyChFlag = false; ////?????????

	int fftOrder = 12;
	int fftSize  = 1 << fftOrder;
	juce::dsp::WindowingFunction<float>::WindowingMethod winMet = juce::dsp::WindowingFunction<float>::hann;

	juce::dsp::FFT fft{ fftOrder };
	juce::dsp::WindowingFunction<Type> windowing{ size_t(fftSize), juce::dsp::WindowingFunction<Type>::hann, true };

	juce::AudioBuffer<float> fftBuffer{ 1, fftSize * 2 };

	juce::AudioBuffer<float> averager{ 5, fftSize / 2 };

	int averagerPtr = 1;

	juce::AbstractFifo abstractFifo{ 48000 };
	juce::AudioBuffer<Type> audioFifo;

	//std::atomic<bool> newDataAvailable;

	bool updateAvailable;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Analyser)
};
