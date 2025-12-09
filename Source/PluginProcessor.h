/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SamplerSynthesizer.h"

struct MidiOnOff {
    int time = 0;
    int note = 0;
    bool on = false;
    bool transport = false;
    static bool sortTime(const MidiOnOff& a, const MidiOnOff& b)
    {
        return a.time < b.time;
    }
};


//==============================================================================
/**
*/
class SimplerStereoSamplerAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorParameter::Listener, public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    SimplerStereoSamplerAudioProcessor();
    ~SimplerStereoSamplerAudioProcessor() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {

    }

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

    SamplerSynthesizer synth;

    juce::AudioParameterInt* slotNum;
    juce::AudioParameterBool* resetOne;
    juce::AudioParameterBool* resetAll;
    juce::AudioParameterBool* resetStart;

    juce::AudioParameterFloat* frequencyFactor;
    juce::AudioParameterInt* tuning;

private:
    int lastSlotNum = 0;
    bool lastResetOne = false;
    bool lastResetAll = false;
    bool lastResetStart = true;
    bool lastPlaying = false;
    float lastFrequencyFactor;
    int lastTuning;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplerStereoSamplerAudioProcessor)
};
