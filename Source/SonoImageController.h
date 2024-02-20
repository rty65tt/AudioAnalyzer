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

    void drawSonogram(juce::Graphics& g);

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
    const int scaleTopLineHeightInt = 20;
    const float scaleTopLineHeightFloat = float(scaleTopLineHeightInt);

private:

    void resizeImg();
    void drawNextLineOfSonogram();

    juce::Image* sonogramImage = nullptr;

    int height = 350;
    int iW = 800;
    int iH = height - scaleTopLineHeightInt;
    int iB = iH + 4;
    juce::Rectangle<int>   copyImgBound { 0, 0, iW, iH };
    juce::Rectangle<float> pastImgBound {0.0f, scaleTopLineHeightFloat, float(iW), float(iH)};

    bool resize = true;
    bool ready = false;

    bool chL = false;
    bool chR = false;

    juce::Path* aPathCh1L;
    juce::Path* aPathCh1R;

    //juce::Graphics& gPtr;
    //bool redraw = false;

    inline static int curWrtLine = 0;

    inline static int countThreads = 0;
};

