/*
  ==============================================================================

    ResizeCacheData.cpp
    Created: 25 Dec 2024 5:13:09pm
    Author:  u1

  ==============================================================================
*/
//#include <memory>
#include <JuceHeader.h>
#include "settings.h"
#include "CacheData.h"


// ==== Start LineData =======================================================
LineData::LineData(DefaultPreferences* dp) : cs(dp) { genCacheData(); }
LineData::~LineData() {
	DBG("=*=> LineData::DESTRUCTOR ---***====================================");
	delete[] lineCache;
	delete[] xcrdlog;
	delete[] xcrdlin;
	delete[] freqIndexLog;
	delete[] freqIndexLin;
}

void LineData::genCacheData() {
	//if (numSmpls != s || cWidth != cs->newW || cSlope != cs->slope) {
	DBG("=*=> LineData::genCacheData ---***====================================");

		fftOrder = *cs->fftOrder;
		const int fftSize = 1 << fftOrder;
		numSmpls = fftSize / 2;
		cWidth = cs->newW;
		cSlope = cs->slope;
		lineCache = new sLineCache[numSmpls];
		xcrdlog = new xCordCache[numSmpls];
		xcrdlin = new xCordCache[numSmpls];
		freqIndexLog = new FreqIndex[int(cWidth) * 2];
		freqIndexLin = new FreqIndex[int(cWidth) * 2];

		const float sumDb = (cSlope * 12.0f);
		const float xkoef = sumDb / cWidth;
		const float sampleRate = cs->sampleRate;
		const float minFreq = cs->minFreq;
		const float maxFreq = cs->sampleRate * 0.5f;

		int xlog = 0;
		int xlin = 0;
		int logCount = 0;
		int linCount = 0;

		for (int i = 0; i < numSmpls; ++i) {
			const float freq = (sampleRate * (i)) / fftSize;
			const float b = std::log(maxFreq / minFreq) / (maxFreq - minFreq);
			const float a = maxFreq / std::exp(maxFreq * b);
			const float position = freq ? std::log(freq / a) / b : 1.f;

			lineCache[i].freq = freq;

			float x = (cWidth * position / (maxFreq - minFreq));
			xcrdlog[i].x = static_cast<int>(round(x));
			lineCache[i].slopeGain = x * xkoef;

			if (xcrdlog[i].x != xlog) {
				freqIndexLog[logCount].v = i;
				freqIndexLog[logCount].bx = xcrdlog[i].x - xlog;
				xlog = xcrdlog[i].x;
				++logCount;
			}
			x = (cWidth * freq / (maxFreq - minFreq));
			xcrdlin[i].x = static_cast<int>(round(x));
			if (xcrdlin[i].x != xlin) {
				freqIndexLin[linCount].v = i;
				freqIndexLin[linCount].bx = xcrdlin[i].x - xlin;
				xlin = xcrdlin[i].x;
				++linCount;
			}
		}

		//freqIndexLog[0].v = 0;
		//xcrdlog[0].x = 0;
		//xcrdlin[0].x = 0;

		freqIndexSizeLog = logCount;
		freqIndexSizeLin = linCount;
		lineCache[0].slopeGain = 0.f;

		DBG("cWidth:   " << cWidth);
		DBG("numSmpls: " << numSmpls);
		DBG("logCount: " << logCount);
		DBG("linCount: " << linCount);

	//}
	//return lineCache;
}

//sLineCache* LineData::getCacheData() const {
//	return lineCache;
//}

// ==== END LineData =========================================================
