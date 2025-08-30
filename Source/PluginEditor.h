/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define BOX_W 120
#define BOX_H 45

//==============================================================================
/**
*/
class SimplerStereoSamplerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, public juce::ChangeListener
{
public:
    SimplerStereoSamplerAudioProcessorEditor (SimplerStereoSamplerAudioProcessor&);
    ~SimplerStereoSamplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    int loadFile(juce::File file);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void updateSample();
    double rootFrequency = 55;
    juce::File fileToLoad{""};
    void buttonClicked(juce::Button* button) override;

    juce::FileChooser sampleChooser{ "Choose a Sample to Load...", juce::File::getSpecialLocation(juce::File::userMusicDirectory), "*.wav;*.flac" };
    juce::TextButton loadButton{ "Load Sample (55Hz/A1)..." };
    juce::TextButton nextSampleButton{ "Next Sample" };
    juce::TextButton prevSampleButton{ "Prev. Sample" };
    juce::TextButton ejectSampleButton{ "Eject Sample" };
    juce::TextButton resetButton{ "Reset Sample" };
    juce::TextButton resetAllButton{ "Reset ALL" };
    juce::TextButton panicButton{ "MIDI Panic!" };
    juce::Label sampleNameBox{ "sampleNameBox", "Slot 0 - Not Loaded" };

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimplerStereoSamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplerStereoSamplerAudioProcessorEditor)
};
