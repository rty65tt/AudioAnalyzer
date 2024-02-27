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

            pastImgBound.setY(float(height - cwline+start1));
            pastImgBound.setHeight(float(cwline));
            g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
        }

        if (!start1) {
            const int h2 = iH - cwline;
            copyImgBound.setY(cwline+4);
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
        DBG("SonoImage::addLineSono:ch " << ch << " Line: " << y);
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

    int x = 0;
    //const int y = getCurLine();
    
    float xL1, yL1, xL2 = 0.f, yL2 = imgDataL[0].y;
    float xR1, yR1, xR2 = 0.f, yR2 = imgDataR[0].y;

    const float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);
    const float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);

    juce::Image sonoImg (juce::Image::RGB, iW, 1, true);
    juce::Colour bgL = juce::Colours::black;
    juce::Colour bgR = juce::Colours::transparentBlack;

    for(int a = 0; a++ < arrWidth; )
    {
        xL1 = xL2;
        yL1 = yL2;
        xL2 = imgDataL[a].x;
        yL2 = imgDataL[a].y;

        xR1 = xR2;
        yR1 = yR2;
        xR2 = imgDataR[a].x;
        yR2 = imgDataR[a].y;

        const float bxL = xL2 - xL1;
        const float byL = yL2 - yL1;

        const float bxR = xR2 - xR1;
        const float byR = yR2 - yR1;

        float lvlL = yL1;
        float lvlR = yR1;

        const float lkoefL = byL / bxL;
        const float lkoefR = byR / bxR;

        for (int i = 0; i < bxL; ++i) { // opimizat 

            if (ch1L) { bgL = juce::Colour::fromHSL(colorL, saturatSono, lvlL, lvlL); }
            if (ch1R) { bgR = juce::Colour::fromHSL(colorR, saturatSono, lvlR, lvlR); }

            //sonogramImage->setPixelAt(x, y, bgL.interpolatedWith(bgR, 0.5f));
            sonogramImage->setPixelAt(x, y, bgL.overlaidWith(bgR));

            lvlL += lkoefL;
            lvlR += lkoefR;
            x++;
        }
    }
}

LineData::LineData() {}
LineData::~LineData() {
    cleanCache();
}
void LineData::cleanCache() {
    delete[] lineCache;
}

sLineCache* LineData::genCacheData(   const int s,
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
        lineCache   = new sLineCache[numSmpls];
        ldata       = new LineChannelData[numSmpls];

        const float sumDb = (slope * 12.0f);
        const float xkoef = sumDb / width;
        const float maxFreq = sampleRate * 0.5f;

        for (int i = 0; i < numSmpls; ++i) {
            const float freq = (sampleRate * i) / fftSize;
            const float b = std::log(maxFreq / minFreq) / (maxFreq - minFreq);
            const float a = maxFreq / std::exp(maxFreq * b);
            const float position = std::log(freq / a) / b;

            lineCache[i].freq = freq;
            
            float x = (width * position / (maxFreq - minFreq));
            lineCache[i].xcrd_log = round(x);
            lineCache[i].slopeGain = x * xkoef;

            x = (width * freq / (maxFreq - minFreq));
            lineCache[i].xcrd_lin = round(x);
        }
        lineCache[0].slopeGain = 0.f;
    }
    return lineCache;
}

