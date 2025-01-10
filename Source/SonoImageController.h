/*
  ==============================================================================

    SonoImageController.h
    Created: 14 Feb 2024 11:18:08pm
    Author:  u1

  ==============================================================================
*/

#pragma once

struct Channels {
    float lL = 0.f;
    float lR = 0.f;
    //float lM = 0.f;
    //float lS = 0.f;
    float color = 0.f;
};


struct ChannelLevel {
    ChannelLevel(int n) : num(n) { ch = new Channels[n]; }
    ~ChannelLevel() { delete[] ch; }

    int num = 0;
    Channels* ch = nullptr;
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

    void setAnalyserPath(const int channel, std::shared_ptr<LineChData> ldata);
    void addLineSono(const int arrSize, const int ch);

    int getCurLine();

    //LineChannelData* getLevelArrayPtr(const int channel, const bool reset);

    int sonoColorRender = 0;
    float colorSonoL = 330.0f;
    float colorSonoR = 60.0f;
    float colorSonoM = 180.0f;//????
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

    std::shared_ptr<LineChData> imgDataL = nullptr;
    std::shared_ptr<LineChData> imgDataR = nullptr;

private:

    juce::CriticalSection pathDrawLock;

    void resizeImg();
    void drawNextLineOfSonogram(const int arrWidth, const int y);

    int height = 350;
    int iW = 800;
    int iH = height - scaleTopLineHeightInt;
    int iB = iH + 4;
    juce::Image* sonogramImage = nullptr;
    std::shared_ptr<ChannelLevel> channelLevels = std::make_shared<ChannelLevel>(iW);
    

    juce::Rectangle<int>   copyImgBound { 0, 0, iW, iH };
    juce::Rectangle<float> pastImgBound {0.0f, scaleTopLineHeightFloat, float(iW), float(iH)}; //?? float whai?

    bool resize = true;
    bool ready = false;

    bool chL = false;
    bool chR = false;

    int lineChBufSize = 0;

    int curWrtLine = 0;/////////??????????

};

