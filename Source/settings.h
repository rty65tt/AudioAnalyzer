/*
  ==============================================================================

    settings.h
    Created: 15 Dec 2023 9:44:41pm
    Author:  u1

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum { fft4096=12, fft8192=13, fft16384=14 } curFftSize;
//enum { spec = 1, sono, wave } curAnalyzerMode;

struct DSETTINGS {
    
    int  fftOrder = 12;
//    int  fftSize = 1 << fftOrder;
    bool setLiner = false;
    juce::dsp::WindowingFunction<float>::WindowingMethod winMet = juce::dsp::WindowingFunction<float>::hann;
    bool setNorm = false;
    
    int newW = 0;
    int newH = 0;
    int mode = 1;
    
    float gain          = 0.0f;
    float slope         = 3.0f;
    float floor         = -60.0f; //dB
    float smooth        = 0.0f;
    float avengeing     = 0.0f; //ms
    float lineCR        = 30.0f;
    int   overlap       = 2;
    
    float colorSonoL        = 330.0f;
    float colorSonoR        = 60.0f;
    
    bool menuStereoMidSideSwitch = true;
    bool ch1L = true;
    bool ch1R = true;
    bool ch2L = false;
    bool ch2R = false;
    bool ch1M = true;
    bool ch1S = true;
    bool ch2M = false;
    bool ch2S = false;
    
    bool menuBarHide  = false;
    bool showSettings = false;
    bool menuChSwitch = true;
    
};

//DSETTINGS* cursetup = &setup;
