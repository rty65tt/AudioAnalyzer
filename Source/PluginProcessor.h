/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <memory>

#include <JuceHeader.h>
#include "Analyser.h"
#include "SonoImageController.h"

//==============================================================================
class AudioAnalyzerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioAnalyzerAudioProcessor();
    ~AudioAnalyzerAudioProcessor() override;

    juce::AudioProcessor::BusesProperties makeBusesProperties();
    //==============================================================================
    void prepareToPlay (double newSampleRate, int newSamplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //====================
    void createAnalyserPlot ();
    
    DefaultPreferences cS;

    SonoImage sImg;

    std::shared_ptr<CacheManager> cacheMngr = std::make_shared<CacheManager>(&cS);
    
    juce::Path      analyserPathCh1L;
    juce::Path      analyserPathCh1R;
    juce::Path      analyserPathCh2L;
    juce::Path      analyserPathCh2R;

private:

    Analyser<float> inputAnalyserL1 { cacheMngr, 0, "Thread for Channel L1" };
    Analyser<float> inputAnalyserR1 { cacheMngr, 1, "Thread for Channel R1" };
    Analyser<float> inputAnalyserL2 { cacheMngr, 2, "Thread for Channel L2" };
    Analyser<float> inputAnalyserR2 { cacheMngr, 3, "Thread for Channel R2" };

    double sampleRate = 0;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAnalyzerAudioProcessor)
};
