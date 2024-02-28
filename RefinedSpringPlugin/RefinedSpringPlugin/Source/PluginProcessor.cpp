/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RefinedSpringPluginAudioProcessor::RefinedSpringPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                        )
#endif
{
    // Create parameters with types, ranges, default values and skews

    addParameter(lParam = new juce::AudioParameterFloat("lParam", "Density", 0.0f, 1.0f, 0.0f));
    addParameter(rParam = new juce::AudioParameterFloat("rParam", "Pitch", 0.0f, 1.0f, 0.0f));
    addParameter(decayParam = new juce::AudioParameterFloat("decayParam", "Damping Amount", juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), 1.0f));
    addParameter(darkParam = new juce::AudioParameterBool("darkParam", "Enable Dark Mode", true));
    addParameter(realParam = new juce::AudioParameterBool("realParam", "Use Physical Parameters", true));
    addParameter(inputGain = new juce::AudioParameterFloat("inputGain", "Input Gain", juce::NormalisableRange<float> (0.0f, 5.0f, 0.01f, 0.2f), 1.0f));
    addParameter(dryGain = new juce::AudioParameterFloat("dryGain", "Dry Gain", juce::NormalisableRange<float>(0.0f, 5.0f, 0.01f, 0.2f), 1.0f));
    addParameter(wetGain = new juce::AudioParameterFloat("wetGain", "Wet Gain", juce::NormalisableRange<float>(0.0f, 5.0f, 0.01f, 0.2f), 1.0f));
}

RefinedSpringPluginAudioProcessor::~RefinedSpringPluginAudioProcessor()
{
}

//==============================================================================
const juce::String RefinedSpringPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RefinedSpringPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RefinedSpringPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RefinedSpringPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RefinedSpringPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RefinedSpringPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RefinedSpringPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RefinedSpringPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RefinedSpringPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void RefinedSpringPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RefinedSpringPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (!memoryAllocated)
    {
        springOne.quickStart_Ex(); // Initialised plugin with default parameters
        memoryAllocated = true; // Prevents this stage from happening again
        limit.reset();  // Initialised limiter 
    }

    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RefinedSpringPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RefinedSpringPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RefinedSpringPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    jassert(totalNumInputChannels > 0);

    // Create left and right channel buffers

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(0);

    if (totalNumInputChannels == 2)
    {
        // Check is stereo
        rightChannel = buffer.getWritePointer(1);
    }

    // Get left and right buffer data

    auto* inputLeft = buffer.getReadPointer(0);
    auto* inputRight = buffer.getReadPointer(0);

    if (totalNumOutputChannels == 2)
    {
        // Check is stereo 
        inputRight = buffer.getReadPointer(1);
    }

    // Set param checker, updates only performed if the user changes a parameter
    paramChanged = false;

    // Update decay parameters if the user changes them, i.e. the current val is different to the previous
    if (std::fabsf(*decayParam - prev_decay) > 1.0e-4f)
    {
        springOne.updateT60(*decayParam);
        paramChanged = true;
        prev_decay = *decayParam;   // Update the "previous" value
        sliderGain = (*decayParam / 10.0f) + 0.9f; // Gain compensation from the change in damping
    }

    // Update parameters if pitch, density, dark or real parameters are changed
    if (std::fabsf(*rParam - prev_r) > 1.0e-4f || std::fabsf(*lParam - prev_l) > 1.0e-4f || *darkParam != prev_dark || *realParam != prev_real)
    {
        springOne.updateReal(*realParam);

        // If real parameters are used, dark mode is possible
        if (*realParam == true)
        {
            springOne.updateDark(*darkParam);
        }
        else
        {
            springOne.updateDark(false);
        }

        // Update interpolation parameters
        springOne.updateParams(*rParam, *lParam);
        paramChanged = true;

        // Update "previous" parameters
        prev_l = *lParam;
        prev_r = *rParam;
        prev_dark = *darkParam;
        prev_real = *realParam;
    }
    
 

    for (int j = 0; j < buffer.getNumSamples(); ++j)
    {
        // Take mono of input audio buffer
        float inputRaw = (inputLeft[j] + inputRight[j])/2;

        // Gain adjustments
        float inputSample = inputRaw * *inputGain * sliderGain;

        // Get the output from the spring
        float outputSample = springOne.schemeUpdateSSE_Ex(inputSample)/2;

        // Gain adjustments on output
        float outleft = outputSample * (*wetGain) + inputRaw * (*dryGain);
        float outright = outputSample * (*wetGain) + inputRaw * (*dryGain);

        // Pass output through limiter
        limit.update(outleft, outright, outleft, outright);

        // Update output buffer
        leftChannel[j] = outleft;
        rightChannel[j] = outright;

        if (j == buffer.getNumSamples() / 2)
        {
            // Perform state updates halfway through audio buffer to minimise cpu spikes
            if (paramChanged)
            {
                springOne.updateCoefficients_Ex();
            }

        }

    }
}

//==============================================================================
bool RefinedSpringPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RefinedSpringPluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void RefinedSpringPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RefinedSpringPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RefinedSpringPluginAudioProcessor();
}
