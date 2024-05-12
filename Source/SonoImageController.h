/*
  ==============================================================================

    SonoImageController.h
    Created: 14 Feb 2024 11:18:08pm
    Author:  u1

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


struct LineChannelData {
    float x = 0.f;
    float y = 0.f;
};

struct sLineCache {
    float freq = 0.f;
    float slopeGain = 0.f;
};

struct xCordCache {
    float x = 0.f;
};


struct FreqIndex {
    int v = 0;
};

class LineData {
public:
    LineData();
    ~LineData();
    void cleanCache();

    sLineCache* genCacheData(const int s,
                            const float width,
                            const float slope,
                            const float sampleRate,
                            const int   fftSize,
                            const float minFreq);

    int numSmpls = 0;
    int cacheSize = 0;
    int freqIndexSizeLin = 0;
    int freqIndexSizeLog = 0;
    sLineCache* lineCache = nullptr;
    xCordCache* xcrdlog = nullptr;
    xCordCache* xcrdlin = nullptr;
    LineChannelData*    ldata = nullptr;
    FreqIndex*          freqIndexLog = nullptr;
    FreqIndex*          freqIndexLin = nullptr;
private:
    float cWidth = 0;
    float cSlope = 0;
};

class SonoImage
{
public:
    SonoImage();
    ~SonoImage();

    void drawSonogram(juce::Graphics& g);

    void setColorL(const float c);
    void setColorR(const float c);
    void setSaturation(const float s);

    void setSizeImg(const int w, const int h);

    void setAnalyserPath(const int channel, LineChannelData* ldata);
    void addLineSono(const int arrSize, const int ch);

    int getCurLine();

    //LineChannelData* getLevelArrayPtr(const int channel, const bool reset);

    float colorSonoL = 330.0f;
    float colorSonoR = 60.0f;
    float saturatSono = 1.0f;

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

    LineChannelData* imgDataL = nullptr;
    LineChannelData* imgDataR = nullptr;

private:

    juce::CriticalSection pathDrawLock;

    void resizeImg();
    void drawNextLineOfSonogram(const int arrWidth, const int y);

    int height = 350;
    int iW = 800;
    int iH = height - scaleTopLineHeightInt;
    int iB = iH + 4;
    juce::Image* sonogramImage = nullptr;

    juce::Rectangle<int>   copyImgBound { 0, 0, iW, iH };
    juce::Rectangle<float> pastImgBound {0.0f, scaleTopLineHeightFloat, float(iW), float(iH)};

    bool resize = true;
    bool ready = false;

    bool chL = false;
    bool chR = false;

    int lineChBufSize = 0;

    int curWrtLine = 0;/////////??????????

};

