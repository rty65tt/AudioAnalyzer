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
        //sonogramImage->duplicateIfShared(); //?
        //g.drawImage(*sonogramImage, pastImgBound);
        //g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
        const int cwline = curWrtLine;

        //juce::Rectangle<int> c = { 0, 0, iW, iH };
        //juce::Rectangle<float> p = { 0.f, 20.f, float(iW), float(iH) };
        //g.drawImage(sonogramImage->getClippedImage(c), p);
        //return;

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
    //copyImgBound.setHeight(iH);
    pastImgBound.setWidth(float(iW));
    //pastImgBound.setHeight(float(iH));
    //pastImgBound.setTop(float(scaleTopLineHeightInt));
    resize = false;
}

void SonoImage::setAnalyserPath(const int channel, LineChannelData* ldata) {
    if (channel == 0) { chL = true; imgDataL = ldata; }
    if (channel == 1) { chR = true; imgDataR = ldata; }
    //if (chL && chR) {
    //    chL = chR = false;
    //    drawNextLineOfSonogram();
    //}
}

void SonoImage::addLineSono(const int arrSize) {
    if (chL && chR) {
        chL = chR = false;
        drawNextLineOfSonogram(arrSize);
    }
}

void SonoImage::drawNextLineOfSonogram(const int arrWidth)
{
    if (resize) { resizeImg(); return; }

    countThreads++;
    if (countThreads > 1) {
        DBG("drawNextLineOfSonogram:countThreads: " << countThreads);
        countThreads--;
        return;
    }

    int x = 0;
    const int y = curWrtLine = (curWrtLine < iB) ? curWrtLine + 1 : 0;
    
    float xL1, yL1, xL2 = 0.f, yL2 = imgDataL[0].y;
    float xR1, yR1, xR2 = 0.f, yR2 = imgDataR[0].y;

    const float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);
    const float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);

    juce::Image sonoImg (juce::Image::RGB, iW, 1, true);
    juce::Colour bgL = juce::Colours::black;
    juce::Colour bgR = juce::Colours::transparentBlack;

    //for (int f = 0; f < iW; ++f) {
    //    sonogramImage->setPixelAt(f, y, juce::Colours::black);
    //}

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

        for (int i = 0; i < bxL; ++i) {

            if (ch1L) { bgL = juce::Colour::fromHSL(colorL, 1.0, lvlL, lvlL); }
            if (ch1R) { bgR = juce::Colour::fromHSL(colorR, 1.0, lvlR, lvlR); }

            //sonogramImage->setPixelAt(x, 0, bgL.interpolatedWith(bgR, 0.5f));
            sonogramImage->setPixelAt(x, y, bgL.overlaidWith(bgR));

            lvlL += lkoefL;
            lvlR += lkoefR;
            x++;
        }
    }// while (analyserPointR.next() && analyserPointL.next());

    //analyserPointL.~Iterator();
    //analyserPointR.~Iterator();
    countThreads--;
}

LineData::LineData() {}
LineData::~LineData() {
    delete[] lineCache;
    delete[] ldata;
}

sLineCache* LineData::genCacheData(   const int s,
                            const float width,
                            const float slope,
                            const float sampleRate,
                            const int   fftSize,
                            const float minFreq) {

    if (numSmpls != s || cWidth != width || cSlope != slope) {
        if (lineCache != nullptr) {
            delete[] lineCache;
            delete[] ldata;
        }
        numSmpls = s;
        cWidth = width;
        cSlope = slope;
        lineCache = new sLineCache[numSmpls];
        ldata = new LineChannelData[numSmpls];

        DBG("Gen New Cache: numSmpls: " << numSmpls << " s: " << s << " width: " << width);
        const float sumDb = (slope * 12.0);
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

