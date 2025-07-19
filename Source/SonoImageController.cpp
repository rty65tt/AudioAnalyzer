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
	DBG("-----==== drawSonogram START ====-----");
	if (!resize && sonogramImage != nullptr) {
		DBG("-----==== drawSonogram DRAW     =======-----");
		//return;
		const int cwline = curWrtLine;
		const int start1 = cwline < iH ? 0 : cwline - iH;

		if (cwline) {
			//DBG("-----==== drawSonogram DRAW (cwline)  ====[" << cwline <<"]====-----");
			copyImgBound.setWidth(sonogramImage->getWidth());
			copyImgBound.setY(start1);
			copyImgBound.setHeight(cwline);

			pastImgBound.setWidth(float(sonogramImage->getWidth()));
			pastImgBound.setY(float(height - cwline + start1));
			pastImgBound.setHeight(float(cwline));
			g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
		}

		if (!start1) {
			//DBG("-----==== drawSonogram DRAW (!start1) ====[" << start1 << "]====-----");
			const int h2 = iH - cwline;
			copyImgBound.setWidth(sonogramImage->getWidth());
			copyImgBound.setY(cwline + 4);
			copyImgBound.setHeight(h2);

			pastImgBound.setWidth(float(sonogramImage->getWidth()));
			pastImgBound.setY(scaleTopLineHeightFloat);
			pastImgBound.setHeight(float(h2));
			g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
		}
		//DBG("-----==== drawSonogram DRAW END =======-----");
	}
	DBG("-----==== drawSonogram END   ====-----");
}

void SonoImage::setColorL(const float c) { colorSonoL = c; }
void SonoImage::setColorR(const float c) { colorSonoR = c; }
void SonoImage::setSaturation(const float s) { saturatSono = s; }

void SonoImage::setSizeImg(const int w, const int h) {
	//DBG("-----==== setSizeImg START ====-----");
	height = h;
	iW = w;
	iH = h - scaleTopLineHeightInt;
	iB = iH + 4;
	curWrtLine = 0;
	resize = true;
	//DBG("-----==== setSizeImg END   ====-----");
}

void SonoImage::resizeImg() {
	DBG("-----==== resizeImg START ====-----");
	if (sonogramImage != nullptr) { sonogramImage->~Image(); }
	DBG("-----==== resizeImg new sonogramImage ====-----");
	sonogramImage = new juce::Image(juce::Image::RGB, iW, iB, true);
	DBG("-----==== resizeImg create cache  ====-----");
	channelLevels.getBuffer(sonogramImage->getWidth());
	DBG("-----==== resizeImg resize = false ====-----");
	resize = false;
	DBG("-----==== resizeImg END   ====-----");
}

void SonoImage::setAnalyserPath(const int channel, std::shared_ptr<LineChData> ldata) {
	if (channel == 0) { chL = true; imgDataL = ldata; }
	if (channel == 1) { chR = true; imgDataR = ldata; }
}

void SonoImage::addLineSono(const int arrSize, const int ch) {
	const juce::ScopedLock lockedForDraw(pathDrawLock); ///?????
	if (chL && chR) {
		chL = chR = false;
		if (imgDataL->width == imgDataR->width) {
			const int y = getCurLine();
			drawNextLineOfSonogram(arrSize, y);
		}
	}
}

int SonoImage::getCurLine() {
	curWrtLine = (curWrtLine < iB) ? curWrtLine + 1 : 0;
	return curWrtLine;
}

void SonoImage::drawNextLineOfSonogram(const int arrWidth, const int y)
{
	DBG("\n-----==== drawNextLineOfSonogram START ====----- : " << y);
	if (resize) { 
		resizeImg(); 
		DBG("-----==== drawNextLineOfSonogram END   ====----- : " << y);
		return; 
	}

	if (imgDataL->width != sonogramImage->getWidth()) { return; } // ????????

	DBG("-----==== drawNextLineOfSonogram getBuffer    ====----- : " << y);
	std::shared_ptr<sonoLineBuffer> cl = channelLevels.getBuffer(sonogramImage->getWidth()); /////??????
	DBG("-----==== drawNextLineOfSonogram cl.use_count ====----- : " << cl.use_count());

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
	DBG("-----==== drawNextLineOfSonogram LOOP ====----- " << y);
	//juce::PerformanceCounter pc("drawNextLineOfSonogram", 100, juce::File());
	//pc.start();
	for (int a = 0; a < arrWidth; ++a)
	{
		//x1 = x2;
		x2 = imgDataL->x[a];  ////?????? COPY ???????

		yL1 = yL2;
		yL2 = imgDataL->y[a]; ////?????? COPY ???????

		yR1 = yR2;
		yR2 = imgDataR->y[a];

		const int bx = x2 - x1;			////?????? COPY ???????
		//const int bx = imgDataL->bx[a]; ////?????? glitch some wrong ???????

		//cl->lL[x1] = yL1;
		//cl->lR[x1] = yR1;

		if (bx != 1) {
			float lvlL = yL1;
			float lvlR = yR1;
			const float byL = yL2 - yL1;
			const float byR = yR2 - yR1;
			const float lkoefL = byL / bx;
			const float lkoefR = byR / bx;

			for (int x = 0; x < bx; ++x) {
				cl->lL[x1] = lvlL;
				cl->lR[x1] = lvlR;
				lvlL = yL1 + (lkoefL * x);
				lvlR = yR1 + (lkoefR * x);
				++x1;
			}
		} else {
			cl->lL[x1] = yL1; // WTF!?
			cl->lR[x1] = yR1;
			++x1;
		}
	}

	if (MyCallback) {
		for (int x = 0; x < sonogramImage->getWidth(); ++x) { ///iW error
			sonogramImage->setPixelAt(x, y, MyCallback(cl->lL[x], cl->lR[x]));
		}
	}
	else {
		for (int x = 0; x < sonogramImage->getWidth(); ++x) {
			bgL = juce::Colour::fromHSL(colorL, SonoImage::saturatSono, cl->lL[x], cl->lL[x]);
			bgR = juce::Colour::fromHSL(colorR, SonoImage::saturatSono, cl->lR[x], cl->lR[x]);
			//sonogramImage->setPixelAt(x1, y, bgL.interpolatedWith(bgR, 0.5f));
			sonogramImage->setPixelAt(x, y, bgL.overlaidWith(bgR));
		}
	}
	//pc.stop();
	
	DBG("-----==== drawNextLineOfSonogram END   ====----- : " << y);
}
