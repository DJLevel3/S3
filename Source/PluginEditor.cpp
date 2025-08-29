/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimplerStereoSamplerAudioProcessorEditor::SimplerStereoSamplerAudioProcessorEditor (SimplerStereoSamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (BOX_W * 4 + 10, BOX_H * 3 + 10);
    setResizable(false, false);
    setTitle("SimplerStereoSampler");

    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    addAndMakeVisible(nextSampleButton);
    nextSampleButton.addListener(this);

    addAndMakeVisible(prevSampleButton);
    prevSampleButton.addListener(this);

    addAndMakeVisible(ejectSampleButton);
    ejectSampleButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(128, 40, 40));
    ejectSampleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(255, 40, 40));
    ejectSampleButton.addListener(this);

    addAndMakeVisible(resetButton);
    resetButton.addListener(this);

    addAndMakeVisible(resetAllButton);
    resetAllButton.addListener(this);

    addAndMakeVisible(panicButton);
    panicButton.addListener(this);
    panicButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(128, 40, 40));
    panicButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(255, 40, 40));


    addAndMakeVisible(sampleNameBox);
    sampleNameBox.setJustificationType(juce::Justification::centred);

    updateSample();

    audioProcessor.addChangeListener(this);

}

SimplerStereoSamplerAudioProcessorEditor::~SimplerStereoSamplerAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void SimplerStereoSamplerAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &loadButton) {
        sampleChooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& chooser)
        {
            juce::File sampleFile(chooser.getResult());
            int result = loadFile(sampleFile);
            if (result >= 0) audioProcessor.synth.chooseSample(result);
            updateSample();
        });
    }
    else if (button == &nextSampleButton) {
        audioProcessor.synth.chooseNextSample();
        updateSample();
    }
    else if (button == &prevSampleButton) {
        audioProcessor.synth.choosePrevSample();
        updateSample();
    }
    else if (button == &ejectSampleButton) {
        audioProcessor.synth.unloadSample(audioProcessor.synth.getCurrentSample());
        audioProcessor.synth.chooseNextSample();
        updateSample();
    }
    else if (button == &resetButton) {
        audioProcessor.synth.reset();
    }
    else if (button == &resetAllButton) {
        audioProcessor.synth.resetAllSamples();
    }
    else if (button == &panicButton) {
        audioProcessor.synth.noteOff();
    }
}

int SimplerStereoSamplerAudioProcessorEditor::loadFile(juce::File file) {
    if (file.getFileExtension() == ".wav" || file.getFileExtension() == ".flac") {
        int s = audioProcessor.synth.loadSample(file, rootFrequency, audioProcessor.synth.getOpenSample(), true);
        if (s >= 0) {
            return audioProcessor.synth.chooseSample(s);
        }
    }
    return -1;
}

//==============================================================================
void SimplerStereoSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    auto textArea = getLocalBounds().reduced(5).removeFromTop(BOX_H);

    g.setFont (juce::FontOptions (30.0f));
    g.drawFittedText("S3", textArea.removeFromLeft(BOX_W).reduced(5), juce::Justification::centred, 1);
}

void SimplerStereoSamplerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    auto areaA = bounds.removeFromTop(BOX_H);

    areaA.removeFromLeft(BOX_W); // Title text
    loadButton.setBounds(areaA.removeFromLeft(BOX_W * 2).reduced(5));

    areaA = bounds.removeFromBottom(BOX_H);
    resetButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    nextSampleButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    prevSampleButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    ejectSampleButton.setBounds(areaA.removeFromLeft(BOX_W).reduced(5));

    areaA = bounds.removeFromBottom(BOX_H);
    resetAllButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    panicButton.setBounds(areaA.removeFromLeft(BOX_W).reduced(5));
    sampleNameBox.setBounds(areaA.reduced(5));
}


void SimplerStereoSamplerAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &audioProcessor) {
        updateSample();
    }
}