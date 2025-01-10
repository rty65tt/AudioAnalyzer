/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GuiPreferences.h"

//==============================================================================
class AudioAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:

    AudioAnalyzerAudioProcessorEditor (AudioAnalyzerAudioProcessor&);
    ~AudioAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    
    float logScale(const float value, const float min, const float max);
    float invLogScale(const float value, const float min, const float max);
       
    enum { spec = 1, sono, wave } curAnalyzerMode;
//    static enum { fft4096=12, fft8192=13, fft16384=14 } curFftSize;

    enum
    {
        defW = 800,
        defH = 350
    };
  
private:
 
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioAnalyzerAudioProcessor& aP;
    
    void PanelInit();
    void drawPanel();
    void showSetPanel();
    void repaintPanel();
    void hidePanel();
    void hideColorSliders();
    
    void createFreqGrid();
    void drawFreqGrid(juce::Graphics &g) const;
    void drawSpectrogram(juce::Graphics &g);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAnalyzerAudioProcessorEditor)

    juce::Image* gridImage = nullptr;
    
    OtherLookAndFeel otherLookAndFeel;
    
    juce::Rectangle<int>          plotFrame;
    juce::Path                    analyserPath;

    juce::GroupComponent      frame;
    juce::ConcertinaPanel     panel;
    
    juce::GroupComponent      settingsFrame;
    juce::GroupComponent      setFftGroup;
    juce::GroupComponent      setOverlapGroup;
    juce::GroupComponent      setWinGroup;
    juce::GroupComponent      setSonoColorRenderGroup;
    
    SetBarMenu  setbarmenu;
    SetBarMenu  setFrameBG;
    
    enum RadioButtonIds
    {
        ModeButtons = 1001,
        fftSizeButtons,
        fftSizeWindow,
        colorRenderSwitch,
        overlapButtons
    };

    juce::TextButton specbutton;
    juce::TextButton sonobutton;
    juce::TextButton wavebutton;
    juce::TextButton hidebutton;
    juce::TextButton showbutton;
    
    juce::TextButton set_button;
    
//    juce::TextButton stereo_midside;
    juce::TextButton ch1Lbutton;
    juce::TextButton ch1Rbutton;
    juce::TextButton ch2Lbutton;
    juce::TextButton ch2Rbutton;
 //   juce::TextButton ch1Mbutton;
 //   juce::TextButton ch1Sbutton;
 //   juce::TextButton ch2Mbutton;
 //   juce::TextButton ch2Sbutton;
    
    bool menuBarHide  = false;
    bool menuChSwitch = true;
    bool showSettings = false;
    
    bool ch1L = true;
    bool ch1R = true;
    bool ch2L = false;
    bool ch2R = false;
 //   bool ch1M = true;
 //   bool ch1S = true;
 //   bool ch2M = false;
 //   bool ch2S = false;
    
    SetSliderS1 gainSlider      {"GAIN",     " dB",  10,  20 };
    SetSliderS1 slopeSlider     {"SLOPE",    " dB",  90,  20 };
    SetSliderS1 floorSlider     {"FLOOR",    " dB", 170,  20 };
    SetSliderS1 crLineSlider    {"CR LINE",  " dB", 350,  20 };
    SetSliderS1 colorLSlider    {"COLOR L", " Hue", 350,  20 };
    SetSliderS1 colorRSlider    {"COLOR R", " Hue", 430,  20 };
    SetSliderS1 saturatSlider   {"SATUR",   " Sat", 510,  20 };
    
    juce::TextButton setlinerbutton      {"LINER"};

    float gain      = 0.0f;
    float slope     = 0.0f;
    float floor     = 0.0f;
    float smooth    = 0.0f;
        
    juce::TextButton fft12_button {"4096"};
    juce::TextButton fft13_button {"8192"};
    juce::TextButton fft14_button {"16384"};
       
    juce::TextButton overlap2button  {"x2"};
    juce::TextButton overlap4button  {"x4"};
    juce::TextButton overlap8button  {"x8"};
    juce::TextButton overlap16button {"x16"};
    
//    enum { hann, hamming, blackman, blackmanHarris, flatTop, kaiser } curFftWindow;
//    juce::dsp::WindowingFunction<float>::WindowingMethod curFftWindow;
//    int winMet = juce::dsp::WindowingFunction<float>::hann;
    
    juce::TextButton fftWin1button {"HANN"};
    juce::TextButton fftWin2button {"HAMMINMG"};
    juce::TextButton fftWin3button {"BLACKMAN"};
    juce::TextButton fftWin4button {"BLACKMAN HARRIS"};
    juce::TextButton fftWin5button {"FlatTop"};

    juce::TextButton colortheme00{ "CUSTOM" };
    juce::TextButton colortheme01{ "LRV" };
    juce::TextButton colortheme02{ "LVR" };
    juce::TextButton colortheme03{ "VLR" };
    juce::TextButton colortheme04{ "LLR" };
    juce::TextButton colortheme05{ "***" };
    juce::TextButton colortheme06{ "LR*" };
    juce::TextButton colortheme07{ "M-S" };
    juce::TextButton colortheme08{ "MSS" };
    juce::TextButton colortheme09{ "MVS" };
    juce::TextButton colortheme10{ "MMS" };

    juce::Label     freqLabel {"10 freq"};;
    
    DefaultPreferences* cS = &aP.cS;
    DefaultPreferences defV;
    bool flagStart = true;

    const juce::Colour colorSB = juce::Colour::fromRGBA(75, 75, 175, 255);
    const juce::Colour color1L = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.5f, 1.0f);
    const juce::Colour color1R = juce::Colour::fromFloatRGBA(1.0f, 0.0f, 0.5f, 1.0f);
    
    juce::Label         curVersionLabel;
    juce::TextButton    urlVersionButton;
};

