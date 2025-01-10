/*
  ==============================================================================

    ResizeCacheData.h
    Created: 25 Dec 2024 5:13:09pm
    Author:  u1

  ==============================================================================
*/

#pragma once

#include "settings.h"

struct sLineCache {
    float freq = 0.f;
    float slopeGain = 0.f;
};

struct xCordCache {
    int x = 0;
};


struct FreqIndex {
    int v = 0;
    int bx = 0;
};

struct LineData {
    LineData(DefaultPreferences* dp);
    ~LineData();

    void genCacheData();
    //sLineCache* getCacheData() const;

    int fftOrder = 0;
    int numSmpls = 0;
    int cacheSize = 0;
    int freqIndexSizeLin = 0;
    int freqIndexSizeLog = 0;
    sLineCache* lineCache = nullptr;
    xCordCache* xcrdlog = nullptr;
    xCordCache* xcrdlin = nullptr;
    FreqIndex* freqIndexLog = nullptr;
    FreqIndex* freqIndexLin = nullptr;

    DefaultPreferences* cs;
    float cWidth = cs->newW;
    float cSlope = 0;
};

struct LineChData {
    LineChData(int n) : numsmpls(n) {}
    ~LineChData() {
        delete[] x;
        delete[] bx;
        delete[] y;
    }
    int width = 0;
    int numsmpls = 0;
    int* x = new int[numsmpls];
    int* bx = new int[numsmpls];
    float* y  = new float[numsmpls];
};


struct CacheManager
{
    CacheManager(DefaultPreferences* s) : dp_ptr(s) {}
    ~CacheManager() {}
    std::shared_ptr<LineData> getCacheData() const { return cd; }

    void regen() { cd = std::make_shared<LineData>(dp_ptr); }

    void checkUpdate() { 
        if (cd->fftOrder != *dp_ptr->fftOrder || cd->cSlope != dp_ptr->slope) {
            regen();
        }
     }
    
    //int fftOrder = dp_ptr->fftOrder;
    //float cSlope = dp_ptr->slope;

    DefaultPreferences* dp_ptr;
    std::shared_ptr<LineData> cd = std::make_shared<LineData>(dp_ptr);
};

struct LineDataBuffer
{
    LineDataBuffer() {}
    ~LineDataBuffer() {}
    std::shared_ptr<LineChData> getLineBuffer(const int n) {
        if (n != numsmpls) {
            numsmpls = n;
            ld = std::make_shared<LineChData>(numsmpls);
        }
        return ld;
    }
    int numsmpls = 0;
    std::shared_ptr<LineChData> ld = std::make_shared<LineChData>(numsmpls);
};
