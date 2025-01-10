/*
  ==============================================================================

	SonoImageController.cpp
	Created: 14 Feb 2024 11:18:08pm
	Author:  u1

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CacheData.h"
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
	channelLevels = std::make_shared<ChannelLevel>(iW);
	//std::shared_ptr<Channels[]> channelLevels = std::make_shared<Channels[]>(iW);
	copyImgBound.setWidth(iW);
	pastImgBound.setWidth(float(iW));
	resize = false;
}

void SonoImage::setAnalyserPath(const int channel, std::shared_ptr<LineChData> ldata) {
	if (channel == 0) { chL = true; imgDataL = ldata; }
	if (channel == 1) { chR = true; imgDataR = ldata; }
}

void SonoImage::addLineSono(const int arrSize, const int ch) {
	if (chL && chR) {
		chL = chR = false;

		const juce::ScopedLock lockedForDraw(pathDrawLock); ///?????
		const int y = getCurLine();
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

	std::shared_ptr<ChannelLevel> cl = channelLevels; /////??????

	const float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);///????
	const float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);///?????
	const float colorM = juce::jmap(colorSonoM, 0.0f, 360.0f, 0.0f, 1.0f);///?????

	juce::Colour bgL = juce::Colours::black;
	juce::Colour bgR = juce::Colours::transparentBlack;

	juce::Colour(*MyCallback)(const float l, const float r) = nullptr;
	switch (sonoColorRender) {
	case 1:
		MyCallback = [](const float l, const float r)->juce::Colour {
			return juce::Colour::fromFloatRGBA(l, r, r, 1.f);
			};
		break;
	case 2:
		MyCallback = [](const float l, const float r)->juce::Colour {
			const float v = (l > r) ? l : r;
			return juce::Colour::fromFloatRGBA(l, v, r, 1.f);
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
			const float s = l > r ? l - r : r - l;
			const float m = l > r ? l - s : r - s;
			const float v = m + s;
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

	int x1 = 0, x2 = 0;
	float yL1, yR1, yL2 = imgDataL->y[0], yR2 = imgDataR->y[0];

	//juce::PerformanceCounter pc("drawNextLineOfSonogram", 100, juce::File());
	//pc.start();
	for (int a = 0; a < arrWidth; ++a)
	{
		//x1 = x2;
		x2 = imgDataL->x[a];  ////?????? COPY ???????

		yL1 = yL2;
		yL2 = imgDataL->y[a];////?????? COPY ???????

		yR1 = yR2;
		yR2 = imgDataR->y[a];

		const int bx = x2 - x1;			////?????? COPY ???????
		//const int bx = imgDataL->bx[a]; ////?????? glitch some wrong ???????

		const float byL = yL2 - yL1;
		const float byR = yR2 - yR1;

		float lvlL = yL1;
		float lvlR = yR1;

		if (bx > 1) {
			const float lkoefL = byL / bx;
			const float lkoefR = byR / bx;

			for (int x = 0; x < bx; ++x) {
				cl->ch[x1].lL = lvlL;
				cl->ch[x1].lR = lvlR;
				lvlL = yL1 + (lkoefL * x);
				lvlR = yR1 + (lkoefR * x);
				++x1;
			}
		} else {
			cl->ch[x1].lL = lvlL;
			cl->ch[x1].lR = lvlR;
			++x1;
		}
	}

	if (MyCallback) {
		for (int x = 0; x < iW; ++x) {
			sonogramImage->setPixelAt(x, y, MyCallback(cl->ch[x].lL, cl->ch[x].lR));
		}
	}
	else {
		for (int x = 0; x < iW; ++x) {
			bgL = juce::Colour::fromHSL(colorL, SonoImage::saturatSono, cl->ch[x].lL, cl->ch[x].lL);
			bgR = juce::Colour::fromHSL(colorR, SonoImage::saturatSono, cl->ch[x].lR, cl->ch[x].lR);
			//sonogramImage->setPixelAt(x1, y, bgL.interpolatedWith(bgR, 0.5f));
			sonogramImage->setPixelAt(x, y, bgL.overlaidWith(bgR));
		}
	}

	//pc.stop();
}
