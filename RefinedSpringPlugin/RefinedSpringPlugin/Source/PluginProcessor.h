/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RefinedSpringModal.h"
#include "PA_Limiter.h"

//==============================================================================
/**
*/
class RefinedSpringPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RefinedSpringPluginAudioProcessor();
    ~RefinedSpringPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
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

private:

    // Initialise main spring class 
    RefinedSpringModal springOne;

    //Initialise limiter class from Physical Audio
    PA_Limiter limit;

    // Check to ensure memory allocation performed only once
    bool memoryAllocated = false;

    // Plugin Parameters initialised
    juce::AudioParameterFloat* lParam;
    juce::AudioParameterFloat* rParam;
    juce::AudioParameterFloat* decayParam;
    juce::AudioParameterBool* darkParam;
    juce::AudioParameterBool* realParam;
    juce::AudioParameterFloat* inputGain;
    juce::AudioParameterFloat* dryGain;
    juce::AudioParameterFloat* wetGain;

    // Default values for parameters initialised
    float prev_l = 0.0f;
    float prev_r = 0.0f;
    float prev_decay = 0.0f;
    bool prev_dark = false;
    bool prev_real = true;
    bool paramChanged = false;
    float sliderGain = 0.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefinedSpringPluginAudioProcessor)
};
