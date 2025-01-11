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
    ChannelLevel(const int w) : width(w) {
        DBG("-----==== ChannelLevel Constructor ====----- : " << this->width);
    }
    ~ChannelLevel() {
        DBG("-----==== ChannelLevel Destructor  ====----- : " << this->width);
        delete[] ch;
     }

    int width = 0;
    Channels* ch = new Channels[width];
};

struct sonoLineBuffer {
    sonoLineBuffer(const int w) :  width(w) {
        DBG("-----==== sonoLineBuffer Constructor ====----- : " << this->width);
    }
    ~sonoLineBuffer() {
        DBG("-----==== sonoLineBuffer Destructor Color ====----- : " << this->width);
        delete[] color;
        DBG("-----==== sonoLineBuffer Destructor lL ====----- : " << this->width);
        delete[] lL;
        DBG("-----==== sonoLineBuffer Destructor lR ====----- : " << this->width);
        delete[] lR;
        DBG("-----==== sonoLineBuffer Destructor END ====----- : " << this->width);
    }
    int width = 0;
    float* lL       = new float[width];
    float* lR       = new float[width];
    float* color    = new float[width];
};

struct sonoLineBufferMngr
{
    sonoLineBufferMngr(const int w) : width(w) {}
    ~sonoLineBufferMngr() {}
    std::shared_ptr<sonoLineBuffer> getBuffer(const int w) {
        if (w != width) {
            width = w;
            sb = std::make_shared<sonoLineBuffer>(width);
        }
        return sb;
    }

    int width = 0;
    std::shared_ptr<sonoLineBuffer> sb = std::make_shared<sonoLineBuffer>(width);
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
    const float scaleTopLineHeightFloat = float(scaleTopLineHeightInt);////?????

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
    std::shared_ptr<sonoLineBufferMngr> channelLevels = std::make_shared<sonoLineBufferMngr>(iW);
    
    juce::Rectangle<int> copyImgBound { 0, 0, iW, iH };
    juce::Rectangle<float> pastImgBound {0.f, scaleTopLineHeightFloat, float(iW), float(iH) }; //?? float whai?

    bool resize = true;
    bool ready = false;

    bool chL = false;
    bool chR = false;

    int lineChBufSize = 0;

    int curWrtLine = 0;/////////??????????

};

