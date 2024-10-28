/*
  ==============================================================================

	SonoImageController.cpp
	Created: 14 Feb 2024 11:18:08pm
	Author:  u1

  ==============================================================================
*/

#include "SonoImageController.h"

SonoImage::SonoImage() {}

SonoImage::~SonoImage()
{
	if (sonogramImage != nullptr) {
		sonogramImage->~Image();
	}
}

void SonoImage::drawSonogram(juce::Graphics& g) {
	if (!resize && sonogramImage != nullptr) {

		const int cwline = curWrtLine;
		const int start1 = cwline < iH ? 0 : cwline - iH;

		if (cwline) {
			copyImgBound.setY(start1);
			copyImgBound.setHeight(cwline);

			pastImgBound.setY(float(height - cwline + start1));
			pastImgBound.setHeight(float(cwline));
			g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
		}

		if (!start1) {
			const int h2 = iH - cwline;
			copyImgBound.setY(cwline + 4);
			copyImgBound.setHeight(h2);

			pastImgBound.setY(scaleTopLineHeightFloat);
			pastImgBound.setHeight(float(h2));
			g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
		}
	}
}

void SonoImage::setColorL(const float c) { colorSonoL = c; }
void SonoImage::setColorR(const float c) { colorSonoR = c; }
void SonoImage::setSaturation(const float s) { saturatSono = s; }

void SonoImage::setSizeImg(const int w, const int h) {
	height = h;
	iW = w;
	iH = h - scaleTopLineHeightInt;
	iB = iH + 4;
	curWrtLine = 0;
	resize = true;
}

void SonoImage::resizeImg() {
	if (sonogramImage != nullptr) { sonogramImage->~Image(); }
	sonogramImage = new juce::Image(juce::Image::RGB, iW, iB, true);
	copyImgBound.setWidth(iW);
	pastImgBound.setWidth(float(iW));
	resize = false;
}

void SonoImage::setAnalyserPath(const int channel, LineChannelData* ldata) {
	if (channel == 0) { chL = true; imgDataL = ldata; }
	if (channel == 1) { chR = true; imgDataR = ldata; }
}

void SonoImage::addLineSono(const int arrSize, const int ch) {
	if (chL && chR) {
		chL = chR = false;

		const juce::ScopedLock lockedForDraw(pathDrawLock);
		const int y = getCurLine();
		//DBG("SonoImage::addLineSono:ch " << ch << " Line: " << y);
		drawNextLineOfSonogram(arrSize, y);
	}
}

int SonoImage::getCurLine() {
	curWrtLine = (curWrtLine < iB) ? curWrtLine + 1 : 0;
	return curWrtLine;
}

void SonoImage::drawNextLineOfSonogram(const int arrWidth, const int y)
{
	if (resize) { resizeImg(); return; }

	const float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);
	const float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);

	juce::Colour bgL = juce::Colours::black;
	juce::Colour bgR = juce::Colours::transparentBlack;

	juce::Colour(*MyCallback)(const float l, const float r) = nullptr;
	switch (sonoColorRender) {
	case 1:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA(l, r, l * r, 1.f);
			};
		break;
	case 2:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA(l, (l * r), r, 1.f);
			};
		break;
	case 3:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA((l * r), l, r, 1.f);
			};
		break;
	case 4:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA(l, l, r, 1.f);
			};
		break;
	case 5:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA(l * l, r * r, l * r, 1.f);
			};
		break;
	case 6:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float s = l * r;
			return juce::Colour::fromFloatRGBA(l, r, s * s * s, 1.f);
			};
		break;
	case 7:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float s = l > r ? l - r : r - l;
			const float m = l > r ? l - s : r - s;
			return juce::Colour::fromFloatRGBA(m, 0.f, s, 1.f);
			};
		break;
	case 8:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float s = l > r ? l - r : r - l;
			const float m = l > r ? l - s : r - s;
			return juce::Colour::fromFloatRGBA(m, s, s, 1.f);
			};
		break;
	case 9:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float v = l * r;
			const float s = l > r ? l - r : r - l;
			const float m = l > r ? l - s : r - s;
			return juce::Colour::fromFloatRGBA(m, v, s, 1.f);
			};
		break;
	case 10:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float s = l > r ? l - r : r - l;
			const float m = l > r ? l - s : r - s;
			return juce::Colour::fromFloatRGBA(m, m, s, 1.f);
			};
		break;
	}

	float x1, x2 = 0.f;
	float yL1, yR1, yL2 = imgDataL[0].y, yR2 = imgDataR[0].y;


	for (int a = 0; a++ < arrWidth; )
	{
		x1 = x2;
		x2 = imgDataL[a].x;

		yL1 = yL2;
		yL2 = imgDataL[a].y;

		yR1 = yR2;
		yR2 = imgDataR[a].y;

		const float bx = x2 - x1;

		const float byL = yL2 - yL1;
		const float byR = yR2 - yR1;

		float lvlL = yL1;
		float lvlR = yR1;

		const float lkoefL = byL / bx;
		const float lkoefR = byR / bx;
		for (int x = 0; x < bx; x++) { // opimizat 
			if (MyCallback) {
				sonogramImage->setPixelAt(x1 + x, y, MyCallback(lvlL, lvlR));
			}
			else {
				bgL = juce::Colour::fromHSL(colorL, SonoImage::saturatSono, lvlL, lvlL);
				bgR = juce::Colour::fromHSL(colorR, SonoImage::saturatSono, lvlR, lvlR);
				//sonogramImage->setPixelAt(x, y, bgL.interpolatedWith(bgR, 0.5f));
				sonogramImage->setPixelAt(x1 + x, y, bgL.overlaidWith(bgR));
			}

			lvlL = yL1 + (lkoefL * x);
			lvlR = yR1 + (lkoefR * x);
		}
	}
}

LineData::LineData() {}
LineData::~LineData() {
	cleanCache();
}
void LineData::cleanCache() {
	delete[] lineCache;
	delete[] ldata;
	delete[] xcrdlog;
	delete[] xcrdlin;
	delete[] freqIndexLog;
	delete[] freqIndexLin;
}

sLineCache* LineData::genCacheData(const int s,
	const float width,
	const float slope,
	const float sampleRate,
	const int   fftSize,
	const float minFreq) {

	if (numSmpls != s || cWidth != width || cSlope != slope) {
		if (lineCache != nullptr) {
			cleanCache();
		}
		numSmpls = s;
		cWidth = width;
		cSlope = slope;
		lineCache = new sLineCache[numSmpls];
		xcrdlog = new xCordCache[numSmpls];
		xcrdlin = new xCordCache[numSmpls];
		ldata = new LineChannelData[numSmpls];
		freqIndexLog = new FreqIndex[int(cWidth) * 2];
		freqIndexLin = new FreqIndex[int(cWidth) * 2];

		const float sumDb = (slope * 12.0f);
		const float xkoef = sumDb / width;
		const float maxFreq = sampleRate * 0.5f;

		float xlog = 0.f;
		float xlin = 0.f;
		int logCount = 0;
		int linCount = 0;

		for (int i = 0; i < numSmpls; ++i) {
			const float freq = (sampleRate * (i)) / fftSize;
			const float b = std::log(maxFreq / minFreq) / (maxFreq - minFreq);
			const float a = maxFreq / std::exp(maxFreq * b);
			const float position = freq ? std::log(freq / a) / b : 1.f;

			lineCache[i].freq = freq;

			float x = (width * position / (maxFreq - minFreq));
			xcrdlog[i].x = round(x);
			lineCache[i].slopeGain = x * xkoef;

			if (xcrdlog[i].x != xlog) {
				freqIndexLog[logCount].v = i;
				xlog = xcrdlog[i].x;
				logCount++;
			}
			x = (width * freq / (maxFreq - minFreq));
			xcrdlin[i].x = round(x);
			if (xcrdlin[i].x != xlin) {
				freqIndexLin[linCount].v = i;
				xlin = xcrdlin[i].x;
				linCount++;
			}
		}

		DBG("width:   " << width);
		DBG("logCount: " << logCount);
		DBG("linCount: " << linCount);
		freqIndexSizeLog = logCount;
		freqIndexSizeLin = linCount;

		lineCache[0].slopeGain = 0.f;
	}
	return lineCache;
}

