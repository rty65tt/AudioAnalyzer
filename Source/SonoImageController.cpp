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

        juce::Rectangle<int> c = { 0, 0, iW, iH };
        juce::Rectangle<float> p = { 0.f, 20.f, float(iW), float(iH) };
        g.drawImage(sonogramImage->getClippedImage(c), p);
        return;

        if (cwline > 2 ) {
            copyImgBound.setY(0);
            copyImgBound.setHeight(cwline - 2);

            pastImgBound.setY(float(height - cwline + 2));
            pastImgBound.setHeight(float(cwline - 2));
            g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
        }

        if ((iB - cwline) > 2) {
            copyImgBound.setY(cwline + 2);
            copyImgBound.setHeight(iB - cwline);

            pastImgBound.setY(0.f + scaleTopLineHeightFloat);
            pastImgBound.setHeight(float(iB - cwline));
            g.drawImage(sonogramImage->getClippedImage(copyImgBound), pastImgBound);
        }
    }
}

void SonoImage::setColorL(float c) { colorSonoL = c; }
void SonoImage::setColorR(float c) { colorSonoR = c; }

void SonoImage::setSizeImg(int w, int h) {
    height = h;
    iW = w;
    iH = h - scaleTopLineHeightInt;
    iB = iH + 2;
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

void SonoImage::setAnalyserPath(int channel, juce::Path* p) {
    if (channel == 0) { aPathCh1L = p; chL = true; }
    if (channel == 1) { aPathCh1R = p; chR = true; }
    if (chL && chR) {
        chL = chR = false;
        drawNextLineOfSonogram();
    }
}

void SonoImage::drawNextLineOfSonogram()
{
    if (resize) { resizeImg(); return; }

    //countThreads++;
    //if (countThreads > 1) {
    //    DBG("drawNextLineOfSonogram:countThreads: " << countThreads);
    //    countThreads--;
    //    return;
    //}

    juce::Path::Iterator  analyserPointL(*aPathCh1L);
    juce::Path::Iterator  analyserPointR(*aPathCh1R);

    int x = 0;
    const int y = curWrtLine = (curWrtLine < iB) ? curWrtLine + 1 : 0;
    
    //DBG("curWrtLine" << curWrtLine);
    
    float xL1, yL1, xL2 = 0.f, yL2 = 0.f, xR1, yR1, xR2 = 0.f, yR2 = 0.f;

    const float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);
    const float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);

    juce::Image sonoImg (juce::Image::RGB, iW, 1, true);
    juce::Colour bgL = juce::Colours::black;
    juce::Colour bgR = juce::Colours::transparentBlack;

    for (int f = 0; f < iW; ++f) {
        sonogramImage->setPixelAt(f, y, juce::Colours::black);
    }

    do
    {
        xL1 = xL2;
        yL1 = yL2;
        xL2 = analyserPointL.x1;
        yL2 = analyserPointL.y1;

        xR1 = xR2;
        yR1 = yR2;
        xR2 = analyserPointR.x1;
        yR2 = analyserPointR.y1;

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
    } while (analyserPointR.next() && analyserPointL.next());


    //sonogramImage->moveImageSection(0, 0, 0, 1, iW, iH);
    //juce::Graphics g(*sonogramImage);
    //g.drawImage(sonoImg, 0, iH, iW, iH+1, 0, 0, iW, 1);

    //analyserPointL.~Iterator();
    //analyserPointR.~Iterator();
    countThreads--;
}

