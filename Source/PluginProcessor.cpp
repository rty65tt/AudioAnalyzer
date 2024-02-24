/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessor::BusesProperties AudioAnalyzerAudioProcessor::makeBusesProperties()
{
    BusesProperties bp;
    bp.addBus(true, "Input", juce::AudioChannelSet::stereo(), true);
    bp.addBus(false, "Output", juce::AudioChannelSet::stereo(), true);
    if (!juce::JUCEApplicationBase::isStandaloneApp())
    {
        bp.addBus(true, "Sidechain", juce::AudioChannelSet::stereo(), true);
    }
    return bp;
}

//==============================================================================
AudioAnalyzerAudioProcessor::AudioAnalyzerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     //: AudioProcessor (BusesProperties()
     //                #if ! JucePlugin_IsMidiEffect
     //                 #if ! JucePlugin_IsSynth
     //                  .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
     //                  .withInput  ("SideChain", juce::AudioChannelSet::stereo(), false)
     //                 #endif
     //                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
     //                #endif
     //                  )
    : AudioProcessor(makeBusesProperties())

#endif
{
}

AudioAnalyzerAudioProcessor::~AudioAnalyzerAudioProcessor()
{
    inputAnalyserL1.stopThread (1000);
    inputAnalyserR1.stopThread (1000);
    inputAnalyserL2.stopThread (1000);
    inputAnalyserR2.stopThread (1000);
}

//==============================================================================
const juce::String AudioAnalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioAnalyzerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioAnalyzerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioAnalyzerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioAnalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioAnalyzerAudioProcessor::getNumPrograms()
{
    return 1;
    // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AudioAnalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioAnalyzerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioAnalyzerAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioAnalyzerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioAnalyzerAudioProcessor::prepareToPlay (double newSampleRate, int newSamplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    sampleRate = newSampleRate;
    cS.maxFreq = getSampleRate() * 0.5;

    inputAnalyserL1.setupAnalyser ( int (sampleRate), float (sampleRate), &sImg );
    inputAnalyserR1.setupAnalyser ( int (sampleRate), float (sampleRate), &sImg );
    inputAnalyserL2.setupAnalyser ( int (sampleRate), float (sampleRate), &sImg );
    inputAnalyserR2.setupAnalyser ( int (sampleRate), float (sampleRate), &sImg );

}

void AudioAnalyzerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    inputAnalyserL1.stopThread (1000);
    inputAnalyserR1.stopThread (1000);
    inputAnalyserL2.stopThread (1000);
    inputAnalyserR2.stopThread (1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioAnalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();

    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    if (mainOut != stereo && mainOut != mono)
        return false;

    if (wrapperType != wrapperType_Standalone)
    {
        const auto scIn = layouts.getChannelSet(true, 1);
        if (!scIn.isDisabled())
            if (scIn != stereo && scIn != mono)
                return false;
    }
    return true;
}
#endif

void AudioAnalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();

//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());

    buffer.applyGain ( juce::Decibels::decibelsToGain( cS.gain ) );
    
    if (getActiveEditor() != nullptr)
    {
        inputAnalyserL1.addAudioData (buffer, 0, 0);
        inputAnalyserR1.addAudioData (buffer, 1, 0);
        if (getTotalNumInputChannels() >= 4) { // fix for VST3
            inputAnalyserL2.addAudioData (buffer, 2, 0);
            inputAnalyserR2.addAudioData (buffer, 3, 0);
        }
    }
    //    juce::dsp::AudioBlock<float> block (buffer);
}

//==============================================================================
bool AudioAnalyzerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioAnalyzerAudioProcessor::createEditor()
{
    return new AudioAnalyzerAudioProcessorEditor (*this);
}

//==============================================================================
void AudioAnalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml (new juce::XmlElement ("AudioAnalyzer"));
    xml->setAttribute ("mode",       (int)       cS.mode);
    xml->setAttribute ("newW",       (int)       cS.newW);
    xml->setAttribute ("newH",       (int)       cS.newH);
    xml->setAttribute ("menuBarHide",(bool)      cS.menuBarHide);
    xml->setAttribute ("gain",      (double)     cS.gain);
    xml->setAttribute ("floor",     (double)     cS.floor);
    xml->setAttribute ("slope",     (double)     cS.slope);
    xml->setAttribute ("crline",    (double)     sImg.lineCR);
    xml->setAttribute ("colorSonoL",     (double)    cS.colorSonoL);
    xml->setAttribute ("colorSonoR",     (double)    cS.colorSonoR);
    xml->setAttribute ("saturatSono",    (double)    cS.colorSonoR);
    xml->setAttribute ("fftOrderSpec",      (int)         cS.fftOrderSpec);
    xml->setAttribute ("fftOrderSono",      (int)         cS.fftOrderSono);
    xml->setAttribute ("overlapSpec",       (int)         cS.overlapSpec);
    xml->setAttribute ("overlapSono",       (int)         cS.overlapSono);
    xml->setAttribute ("ch1L",    (bool)    sImg.ch1L);
    xml->setAttribute ("ch1R",    (bool)    sImg.ch1R);
    xml->setAttribute ("ch2L",    (bool)    sImg.ch2L);
    xml->setAttribute ("ch2R",    (bool)    sImg.ch2R);
//    xml->setAttribute ("ch1M",    (bool)       cS.ch1M);
//    xml->setAttribute ("ch1S",    (bool)       cS.ch1S);
//    xml->setAttribute ("ch2M",    (bool)       cS.ch2M);
//    xml->setAttribute ("ch2S",    (bool)       cS.ch2S);
    copyXmlToBinary (*xml, destData);
}

void AudioAnalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName ("AudioAnalyzer"))
        {
            cS.mode   = xmlState->getIntAttribute("mode");
            cS.newW   = xmlState->getIntAttribute("newW");
            cS.newH   = xmlState->getIntAttribute("newH");
            cS.menuBarHide   = xmlState->getBoolAttribute("menuBarHide");
            cS.gain   = (float) xmlState->getDoubleAttribute ("gain", 1.0);
            cS.floor  = (float) xmlState->getDoubleAttribute ("floor", -60.0);
            cS.slope  = (float) xmlState->getDoubleAttribute ("slope", 3.0);
            sImg.lineCR = (float) xmlState->getDoubleAttribute ("crline", 30.0);
            cS.colorSonoL = (float) xmlState->getDoubleAttribute ("colorSonoL", 330.0);
            cS.colorSonoR = (float) xmlState->getDoubleAttribute ("colorSonoR", 120.0);
            cS.colorSonoR = (float) xmlState->getDoubleAttribute ("saturatSono", 1.0);
            cS.fftOrderSpec = (int) xmlState->getIntAttribute ("fftOrderSpec", 12);
            cS.fftOrderSono = (int)xmlState->getIntAttribute("fftOrderSono", 12);
            cS.overlapSpec = (int) xmlState->getIntAttribute ("overlapSpec", 2);
            cS.overlapSono = (int)xmlState->getIntAttribute("overlapSono", 16);
            sImg.ch1L   = xmlState->getBoolAttribute("ch1L");
            sImg.ch1R   = xmlState->getBoolAttribute("ch1R");
            sImg.ch2L   = xmlState->getBoolAttribute("ch2L");
            sImg.ch2R   = xmlState->getBoolAttribute("ch2R");
//            cS.ch1M   = xmlState->getBoolAttribute("ch1M");
//            cS.ch1S   = xmlState->getBoolAttribute("ch1S");
//            cS.ch2M   = xmlState->getBoolAttribute("ch2M");
//            cS.ch2S   = xmlState->getBoolAttribute("ch2S");
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioAnalyzerAudioProcessor();
}

//==============================================================================

void AudioAnalyzerAudioProcessor::createAnalyserPlot ()
{
    inputAnalyserL1.createPath (analyserPathCh1L);
    inputAnalyserR1.createPath (analyserPathCh1R);
    
    if (sImg.ch2L)
    { inputAnalyserL2.createPath (analyserPathCh2L); }
    
    if (sImg.ch2R)
    { inputAnalyserR2.createPath (analyserPathCh2R); }
}


