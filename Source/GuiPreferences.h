/*
  ==============================================================================

    GuiPreferences.h
    Created: 25 Dec 2024 11:01:15pm
    Author:  u1

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        //setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB(40,40,40));
        setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);
        setColour(juce::Label::textColourId, juce::Colours::black);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::GroupComponent::textColourId, juce::Colour::fromRGB(160, 160, 160));
    }
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return { juce::jmin(24.0f, (float)buttonHeight * 1.0f) };
    }
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override
    {
        g.setColour(juce::Colour::fromRGB(130, 130, 130));
        auto buttonArea = button.getLocalBounds().toFloat();
        g.fillRect(buttonArea);
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // fill
        g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(rx, ry, rw, rw);

        // outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(rx, ry, rw, rw, 4.0f);
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 4.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // pointer
        g.setColour(juce::Colours::black);
        g.fillPath(p);
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
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::darkgrey);
        auto fillArea = getLocalBounds().toFloat();
        fillArea.removeFromTop(6.0);
        g.setOpacity(0.90f);
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
        addAndMakeVisible(pframe);

        pframe.setBounds(0, 0, 80, 100);

        pSlider.setTextValueSuffix(ValueSuffix);
        pSlider.setBounds(0, 20, 80, 80);
        pSlider.setMouseDragSensitivity(160);
        pSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        pSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 30);
        pSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(175, 175, 175));
        //        pSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
        pSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        pframe.addAndMakeVisible(pSlider);
    }
    void setParamFloat(float min, float max, float def, float step, float& gVar, bool regen=false) {
        pSlider.setRange(min, max, step);
        pSlider.setDoubleClickReturnValue(true, def);
        pSlider.setValue(gVar);
        pSlider.onValueChange = [this, &gVar] { gVar = pSlider.getValue(); };
        if (regen) {}
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetSliderS1)

        juce::GroupComponent    pframe;
    juce::Label             pLabel;
    juce::Slider            pSlider;
};
//==============================================================================

