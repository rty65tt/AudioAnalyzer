/*
  ==============================================================================

    SonoImageController.h
    Created: 14 Feb 2024 11:18:08pm
    Author:  u1

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SonoImage
{
 public:
    SonoImage();
    ~SonoImage();

    void drawSonogram(juce::Graphics& g, const juce::Rectangle<float> b) const;

    void setColorCh1L(float c);
    void setColorCh1R(float c);

    void setSizeImg(int w, int h);

    void setAnalyserPath(int channel, juce::Path* p);

private:

    void resizeImg();

    void drawNextLineOfSonogram();

    juce::Image *sonogramImage = nullptr;

    int iW = 800;
    int iH = 350;
    bool resize = true;
    bool ready = false;

    int iHeight = iH - 1;

    bool ch1L = true;
    bool ch1R = true;

    juce::Path* aPathCh1L;
    juce::Path* aPathCh1R;

    float colorSonoL = 330.0f;
    float colorSonoR = 60.0f;

    inline static int countInst = 0;
};

