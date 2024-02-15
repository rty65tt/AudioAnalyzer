/*
  ==============================================================================

    SonoImageController.cpp
    Created: 14 Feb 2024 11:18:08pm
    Author:  u1

  ==============================================================================
*/

#include "SonoImageController.h"

SonoImage::SonoImage()
{
}

SonoImage::~SonoImage()
{
    if (sonogramImage != nullptr) {
        sonogramImage->~Image();
    }
}

void SonoImage::drawSonogram(juce::Graphics& g, const juce::Rectangle<float> b) const {
    if (!resize && sonogramImage != nullptr) {
        g.drawImage(*sonogramImage, b);
    }
}

void SonoImage::setColorCh1L(float c) { colorSonoL = c; }
void SonoImage::setColorCh1R(float c) { colorSonoR = c; }

void SonoImage::setSizeImg(int w, int h) {
    iW = w;
    iH = h;
    resize = true;
}

void SonoImage::resizeImg() {
    if (sonogramImage != nullptr) { sonogramImage->~Image(); }
    sonogramImage = new juce::Image(juce::Image::ARGB, iW, iH, true);
    iHeight = iH - 1;
    resize = false;
}

void SonoImage::setAnalyserPath(int channel, juce::Path* p) {
    if (channel == 0) { aPathCh1L = p; }
    if (channel == 1) { aPathCh1R = p; }
    if (ready) {
        ready = false;
        drawNextLineOfSonogram();
    }
    else { ready = true; }
}

void SonoImage::drawNextLineOfSonogram()
{
    juce::Path::Iterator analyserPointL(*aPathCh1L);
    juce::Path::Iterator analyserPointR(*aPathCh1R);

    if (resize) { resizeImg(); return; }

    DBG("----------------------------");
    DBG("drawNextLineOfSonogram:START");
    countInst++;

        DBG("drawNextLineOfSonogram:countInst: " << countInst);

    sonogramImage->moveImageSection(0, 0, 0, 1, iW, iHeight);

    int x = 0;
    float xL1, yL1, xL2 = 0.f, yL2 = 0.f, xR1, yR1, xR2 = 0.f, yR2 = 0.f;

    float levelL, levelR;
    float bxL, byL, bxR, byR;
    float lvlL, lvlR;
    float colorL = juce::jmap(colorSonoL, 0.0f, 360.0f, 0.0f, 1.0f);
    float colorR = juce::jmap(colorSonoR, 0.0f, 360.0f, 0.0f, 1.0f);

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

        bxL = xL2 - xL1;
        byL = yL2 - yL1;

        bxR = xR2 - xR1;
        byR = yR2 - yR1;

        lvlL = yL1;
        lvlR = yR1;

        float lkoefL = byL / bxL;
        float lkoefR = byR / bxR;

        juce::Colour bgL;
        juce::Colour bgR;

        for (int i = 0; i < bxL; ++i) {

            x++;

            if (ch1L) {
                levelL = juce::jmap(lvlL, 0.0f, (float)iHeight, 1.0f, 0.0f);
                bgL = juce::Colour::fromHSL(colorL, 1.0, levelL, levelL);
            }
            else {
                bgL = juce::Colours::black;
            }
            if (ch1R) {
                levelR = juce::jmap(lvlR, 0.0f, (float)iHeight, 1.0f, 0.0f);
                bgR = juce::Colour::fromHSL(colorR, 1.0, levelR, levelR);
            }
            else {
                bgR = juce::Colours::black.withAlpha(0.0f);
            }
            juce::Colour newC = bgL.overlaidWith(bgR);

            sonogramImage->setPixelAt(x, iHeight, newC);

            lvlL = lvlL + lkoefL;
            lvlR = lvlR + lkoefR;
        }
    } while (analyserPointR.next() && analyserPointL.next());

    analyserPointL.~Iterator();
    analyserPointR.~Iterator();
    countInst--;
    DBG("drawNextLineOfSonogram:END");
}
