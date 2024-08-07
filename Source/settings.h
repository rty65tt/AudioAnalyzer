/*
  ==============================================================================

    settings.h
    Created: 15 Dec 2023 9:44:41pm
    Author:  u1

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct defSettings {
    
    int fftOrderSpec = 12;
    int fftOrderSono = 13;
    int* fftOrder = &fftOrderSono;
    bool setLiner = false;
    juce::dsp::WindowingFunction<float>::WindowingMethod winMet = juce::dsp::WindowingFunction<float>::hann;
    bool setNorm = false;
    
    bool resize = false;
    int newW = 0;
    int newH = 0;
    int mode = 2;

    juce::Rectangle<float>  plotFrameSono;
    
    float minFreq = 10.0f;
    float maxFreq = 24000.0f;
    
    float gain          = 0.0f;
    float slope         = 3.0f;
    float floor         = -60.0f; //dB
    float smooth        = 0.0f;
    float avengeing     = 0.0f; //ms
    float lineCR        = 30.0f;
    int   overlapSpec = 2;
    int   overlapSono = 16;
    int*  overlap = &overlapSono;

    int sonoColorRender = 0;

    float colorSonoL        = 330.0f;
    float colorSonoR        = 60.0f;
    float saturatSono        = 1.0f;
    
    bool menuStereoMidSideSwitch = true;

    
    bool menuBarHide  = false;
    bool showSettings = true;
    bool menuChSwitch = true;
    
    bool readyCH = false;
    bool chL = false;
    bool chR = false;

    int channels[4] = { 1,1,0,0 };
    bool ch1L = true;
    bool ch1R = true;
    bool ch2L = false;
    bool ch2R = false;

};




