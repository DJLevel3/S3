/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimplerStereoSamplerAudioProcessor::SimplerStereoSamplerAudioProcessor()
{
    synth.chooseSample(0);
    addParameter(slotNum = new juce::AudioParameterInt("slotNum", "Slot #", 0, MAX_SAMPLES - 1, 0));
    addParameter(resetOne = new juce::AudioParameterBool("resetOne", "Reset Current", false));
    addParameter(resetAll = new juce::AudioParameterBool("resetAll", "Reset All", false));
    addParameter(resetStart = new juce::AudioParameterBool("resetStart", "Reset on Transport Start", true));

    slotNum->addListener(this);
    resetOne->addListener(this);
    resetAll->addListener(this);
}

SimplerStereoSamplerAudioProcessor::~SimplerStereoSamplerAudioProcessor()
{
}


void SimplerStereoSamplerAudioProcessor::parameterValueChanged(int parameterIndex, float newValue) {
    if (parameterIndex == slotNum->getParameterIndex()) {
        if (*slotNum != lastSlotNum) {
            synth.chooseSample(*slotNum);
            lastSlotNum = *slotNum;
            sendChangeMessage();
        }
    }
    else if (parameterIndex == resetOne->getParameterIndex()) {
        if (*resetOne != lastResetOne) {
            synth.reset();
            lastResetOne = *resetOne;
        }
    }
    else if (parameterIndex == resetAll->getParameterIndex()) {
        if (*resetOne != lastResetOne) {
            synth.resetAllSamples();
            lastResetAll = *resetAll;
        }
    }
    else if (parameterIndex == resetStart->getParameterIndex()) {
        if (*resetStart != lastResetStart) {
            lastResetStart = *resetStart;
            sendChangeMessage();
        }
    }
    else {
        return;
    }
}

//==============================================================================
const juce::String SimplerStereoSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimplerStereoSamplerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SimplerStereoSamplerAudioProcessor::producesMidi() const
{
    return false;
}

bool SimplerStereoSamplerAudioProcessor::isMidiEffect() const
{
    return false;
}

double SimplerStereoSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimplerStereoSamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimplerStereoSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimplerStereoSamplerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimplerStereoSamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimplerStereoSamplerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimplerStereoSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SimplerStereoSamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SimplerStereoSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    std::vector<MidiOnOff> mid;
    MidiOnOff tempMid;
    juce::AudioPlayHead* transport = getPlayHead();
    auto transportState = transport->getPosition();
    if (transportState.hasValue()) {
        if (transportState->getIsPlaying() && lastPlaying == false) {
            tempMid.time = 0;
            tempMid.note = 0;
            tempMid.on = true;
            tempMid.transport = true;
            mid.push_back(tempMid);
        }
        lastPlaying = transportState->getIsPlaying();
    }
    for (auto it : midiMessages)
    {
        juce::MidiMessage msg = it.getMessage();
        if (msg.isNoteOnOrOff()) {
            tempMid.time = msg.getTimeStamp();
            tempMid.note = msg.getNoteNumber();
            tempMid.on = msg.isNoteOn();
            tempMid.transport = false;
            mid.push_back(tempMid);
        }
    }
    std::sort(mid.begin(), mid.end(), MidiOnOff::sortTime);
    int timeNow = 0;
    int numMessages = mid.size();
    int messageNow = 0;
    if (numMessages == 0) {
        synth.processBlock(buffer, 0, buffer.getNumSamples());
    }
    else {
        while (messageNow < numMessages) {
            if (mid[messageNow].time > timeNow) synth.processBlock(buffer, timeNow, mid[messageNow].time);
            if (mid[messageNow].transport == false) {
                // This is a midi note, handle that
                synth.noteMessage(mid[messageNow].note, mid[messageNow].on);
            } else if (*resetStart && (mid[messageNow].on == true)) {
                // Transport is starting and we want to reset when that happens
                synth.resetAllSamples();
            }
            messageNow++;
        }
        if (timeNow < buffer.getNumSamples()) {
            synth.processBlock(buffer, timeNow, buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool SimplerStereoSamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimplerStereoSamplerAudioProcessor::createEditor()
{
    return new SimplerStereoSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void SimplerStereoSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> s3(new juce::XmlElement("S3"));
    s3->setAttribute("slotNum", *slotNum);
    s3->setAttribute("lastSlotNum", lastSlotNum);

    s3->setAttribute("resetOne", *resetOne);
    s3->setAttribute("lastResetOne", lastResetOne);

    s3->setAttribute("resetAll", *resetAll);
    s3->setAttribute("lastResetAll", lastResetAll);

    s3->setAttribute("resetStart", *resetStart);
    s3->setAttribute("lastResetStart", lastResetStart);

    synth.getXmlState(s3.get());
    copyXmlToBinary(*s3, destData);
}

void SimplerStereoSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> s3State(getXmlFromBinary(data, sizeInBytes));
    if (s3State.get() != nullptr) {
        if (s3State->hasTagName("S3")) {
            *slotNum = s3State->getIntAttribute("slotNum", 0);
            lastSlotNum = s3State->getIntAttribute("lastSlotNum", 0);

            *resetOne = s3State->getBoolAttribute("resetOne", false);
            lastResetOne = s3State->getBoolAttribute("lastResetOne", false);

            *resetAll = s3State->getBoolAttribute("resetAll", false);
            lastResetAll = s3State->getBoolAttribute("lastResetAll", false);

            *resetStart = s3State->getBoolAttribute("resetStart", true);
            lastResetStart = s3State->getBoolAttribute("lastResetStart", true);

            synth.loadXmlState(s3State->getChildByName("Synth"));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimplerStereoSamplerAudioProcessor();
}
