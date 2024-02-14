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

    juce::Image& getImgPtr();

    void drawSonogram(juce::Graphics& g, const juce::Rectangle<float> b) const;

    void setColorCh1L(float c);
    void setColorCh1R(float c);

    void setSizeImg(int w, int h);
    void resizeImg();

    void setAnalyserPath(int channel, juce::Path p);

    void drawNextLineOfSonogram();

    juce::Image *sonogramImage;


private:

    int iW = 800;
    int iH = 350;
    bool resize = true;
    bool ready = false;

    bool ch1L = true;
    bool ch1R = true;

    juce::Path aPathCh1L;
    juce::Path aPathCh1R;

    float colorSonoL = 330.0f;
    float colorSonoR = 60.0f;
};
