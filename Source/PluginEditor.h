/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
//#include "Analyser.h"

//==============================================================================
class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour (juce::TextButton::textColourOffId, juce::Colours::black);
        setColour (juce::Label::textColourId, juce::Colours::black);
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::GroupComponent::textColourId, juce::Colour::fromRGB(160, 160, 160));
    }
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        return { juce::jmin (24.0f, (float) buttonHeight * 1.0f) };
    }
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const  juce::Colour& backgroundColour, bool, bool isButtonDown) override
    {
        g.setColour (juce::Colours::darkgrey);
        auto buttonArea = button.getLocalBounds().toFloat();
        g.fillRect(buttonArea);
    }
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // fill
        g.setColour (juce::Colours::darkgrey);
        g.fillEllipse (rx, ry, rw, rw);
 
        // outline
        g.setColour (juce::Colours::black);
        g.drawEllipse (rx, ry, rw, rw, 4.0f);
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 4.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        
        // pointer
        g.setColour (juce::Colours::black);
        g.fillPath (p);
    }
};
//==============================================================================

//==============================================================================
class SetBarMenu : public juce::GroupComponent
{
public:
    SetBarMenu()
    {
    }
    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::darkgrey);
        auto fillArea = getLocalBounds().toFloat();
        fillArea.removeFromTop(6.0);
//        g.setOpacity(0.80f);
        g.fillRoundedRectangle(fillArea, 8.0f);
    }
};
//==============================================================================

//==============================================================================
class SetSliderS1 : public juce::Component
{
public:
    SetSliderS1(const juce::String& labelText,
                const juce::String& ValueSuffix,
                int sx, int sy)
{
        setBounds(sx, sy, 80, 100);
        pframe.setTextLabelPosition(juce::Justification::centred);
        pframe.setText(labelText);
        pframe.setColour(juce::GroupComponent::outlineColourId, juce::Colours::grey);
//        pframe.setColour(juce::GroupComponent::textColourId, juce::Colours::grey);
        addAndMakeVisible (pframe);
        
        pframe.setBounds(0, 0, 80, 100);
        
        pSlider.setTextValueSuffix (ValueSuffix);
        pSlider.setBounds(0, 20, 80, 80);
        pSlider.setMouseDragSensitivity(160);
        pSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        pSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 60, 30);
        pSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(175, 175, 175));
//        pSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
        pSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        pframe.addAndMakeVisible (pSlider);
    }
    void setParamFloat(float min, float max, float def, float step, float& gVar) {
        pSlider.setRange (min, max, step);
        pSlider.setDoubleClickReturnValue (true, def);
        pSlider.setValue(gVar);
        pSlider.onValueChange = [this, &gVar] { gVar = pSlider.getValue(); };
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetSliderS1)
    
//    AudioAnalyzerAudioProcessor& sPr;

    juce::GroupComponent    pframe;
    juce::Label             pLabel ;
    juce::Slider            pSlider;
};
//==============================================================================


//==============================================================================
class AudioAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor,
//public juce::Slider::Listener,
//public juce::ChangeListener,
                                           private juce::Timer
{
public:
//    void extracted();
    
AudioAnalyzerAudioProcessorEditor (AudioAnalyzerAudioProcessor&);
~AudioAnalyzerAudioProcessorEditor() override;

    void extracted(juce::Graphics &g, float height, float width);
    
//==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
//    void changeListenerCallback (juce::ChangeBroadcaster* sender) override;
    
    void timerCallback() override;
    
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
//    void mouseDown (const juce::MouseEvent& e) override;
    
    float logScale(const float value, const float min, const float max);
    float invLogScale(const float value, const float min, const float max);
       
//    juce::Image *sonogramImage;
    
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
    
    void drawPanel();
    void repaintPanel();
    void showSetPanel();
    void hidePanel();
    
    void drawFreqGrid(juce::Graphics &g, bool bg, bool fV, bool fL, bool vVL, int fColor);
    void drawSpectrogram(juce::Graphics &g);
//    void drawNextLineOfSonogram(juce::Graphics &g);

#ifdef JUCE_OPENGL
    juce::OpenGLContext     openGLContext;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAnalyzerAudioProcessorEditor)
    
    OtherLookAndFeel otherLookAndFeel;
    
    juce::Rectangle<int>          plotFrame;
    juce::Path                    analyserPath;

    
    juce::GroupComponent      frame;
    juce::ConcertinaPanel     panel;
    
    juce::GroupComponent      settingsFrame;
    juce::GroupComponent      setFftGroup;
    juce::GroupComponent      setOverlapGroup;
    juce::GroupComponent      setWinGroup;
    
    
    SetBarMenu  setbarmenu;
    SetBarMenu  setFrameBG;
    
    enum RadioButtonIds
    {
        ModeButtons = 1001,
        fftSizeButtons,
        fftSizeWindow,
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
    
    juce::TextButton setlinerbutton      {"LINER"};
//    juce::TextButton setNormbutton       {"NORM"};

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
    
    juce::Label     freqLabel {"10 freq"};;
    
    DSETTINGS* cS = &aP.cS;
    DSETTINGS defV;
    bool flagStart = true;

    const juce::Colour colorSB = juce::Colour::fromRGBA(75, 75, 175, 255);
    const juce::Colour color1L = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.5f, 1.0f);
    const juce::Colour color1R = juce::Colour::fromFloatRGBA(1.0f, 0.0f, 0.5f, 1.0f);
    
//    juce::Button::Listener  button;
//    juce::DrawableButton    drawbutton;
//    juce::ShapeButton       shbutton("ShapeButton", colorSB, colorSB, colorSB);
//    juce::ToggleButton      togglebutton;

//    void createSonogramLine (juce::Path& destPath, const juce::Path& sourcePath);

};

