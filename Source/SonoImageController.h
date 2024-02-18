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

    void drawSonogram(juce::Graphics& g) const;

    void setColorL(float c);
    void setColorR(float c);

    void setSizeImg(int w, int h);

    void setAnalyserPath(int channel, juce::Path* p);

    float colorSonoL = 330.0f;
    float colorSonoR = 60.0f;

    bool ch1L = true;
    bool ch1R = true;
    bool ch2L = false;
    bool ch2R = false;
    //bool ch1M = true;
    //bool ch1S = true; 
    //bool ch2M = false;
    //bool ch2S = false;

    float lineCR = 30.0f;
    const float scaleTopLineHeightInt = 20;
    const float scaleTopLineHeightFloat = float(scaleTopLineHeightInt);

private:

    void resizeImg();
    void drawNextLineOfSonogram();

    juce::Image* sonogramImage = nullptr;

    int iW = 800;
    int iH = 350 - scaleTopLineHeightInt;
    juce::Rectangle<int>   copyImgBound { 0, 0, iW, iH };
    juce::Rectangle<float> pastImgBound {0.0f, scaleTopLineHeightFloat, float(iW), float(iH)};

    bool resize = true;
    bool ready = false;

    bool chL = false;
    bool chR = false;

    juce::Path *aPathCh1L = nullptr;
    juce::Path *aPathCh1R = nullptr;

    inline static int countThreads = 0;
};

