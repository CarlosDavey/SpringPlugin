/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class RefinedSpringPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RefinedSpringPluginAudioProcessorEditor (RefinedSpringPluginAudioProcessor&);
    ~RefinedSpringPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RefinedSpringPluginAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefinedSpringPluginAudioProcessorEditor)
};
