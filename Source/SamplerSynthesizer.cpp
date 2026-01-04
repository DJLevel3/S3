/*
  ==============================================================================

    SamplerSynthesizer.cpp
    Created: 28 Aug 2025 7:23:16am
    Author:  DJ_Level_3

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SamplerSynthesizer.h"

//==============================================================================
SamplerSynthesizer::SamplerSynthesizer()
{
    manager.registerBasicFormats();
    for (int i = 0; i < MAX_SAMPLES; i++) {
        samples[i] = SampleSlot();
    }
}

SamplerSynthesizer::~SamplerSynthesizer()
{
    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (samples[i].buffer != nullptr) {
            delete samples[i].buffer;
        }
    }
}

void SamplerSynthesizer::processBlock(juce::AudioBuffer<float>& buffer, int beginSample, int endSample) {
    if (waitingForOuterReset) {
        time = 0;
        waitingForOuterReset = false;
    }

    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (samples[i].waitingForReset) {
            frequency = targetFrequency * frequencyFactor;
            sourceFrequency = targetFrequency;
            samples[i].sampleTime = 0;
            samples[i].waitingForReset = false;
        }
    }

    // If the current sample is invalid, gtfo
    if (currentSample < 0 || currentSample >= MAX_SAMPLES) {
        for (int i = beginSample; i < endSample; i++) {
            buffer.setSample(0, i, 0);
            buffer.setSample(1, i, 0);
        }
        return;
    }

    // If there's no sample loaded, or if we're not playing right now, gtfo
    if ((samples[currentSample].loaded == false) || (playing == false)) {
        for (int i = beginSample; i < endSample; i++) {
            buffer.setSample(0, i, 0);
            buffer.setSample(1, i, 0);
        }
        return;
    }

    int sampleNow = beginSample;
    while (sampleNow < endSample) {
        for (int c = 0; c < 2; c++) {
            // Calculate sample value
            int index = int(time);
            if (index < samples[currentSample].buffer->getNumSamples() && samples[currentSample].waitingForReset == false) {
                float sampleHere = samples[currentSample].buffer->getSample(c, index);
                float sampleNext = samples[currentSample].buffer->getSample(c, (index + 1) % samples[currentSample].buffer->getNumSamples());

                buffer.setSample(c, sampleNow, lerp_f(sampleHere, sampleNext, float(time - index)));
            }
            else {
                buffer.setSample(c, sampleNow, 0);
            }
        }
        // Increment time
        double actualPB = lerp_f(lastPB, pitchBend, (sampleNow - beginSample) / float(endSample - beginSample));
        double increment = tuning * samples[currentSample].rootSampleRate / samples[currentSample].rootFrequency * frequency / sampleRate * actualPB;
        time = time + increment;
        sampleNow++;
        
        // If we've hit the end of the sample:
        if (int(time) >= samples[currentSample].buffer->getNumSamples()) {
            // Loop if we're supposed to
            if (samples[currentSample].loop) {
                time = time - samples[currentSample].buffer->getNumSamples();
            }
            // Otherwise gtfo
            else {
                samples[currentSample].waitingForReset = true;
                while (sampleNow < endSample) {
                    buffer.setSample(0, sampleNow, 0);
                    buffer.setSample(1, sampleNow, 0);
                    sampleNow++;
                }
            }
        }
    }
     lastPB = pitchBend;
}

// Returns -1 if sample is occupied, -2 if sample position is out of bounds, -3 if file is invalid, -4 if file loading failed, otherwise returns position of loaded sample
int SamplerSynthesizer::loadSample(juce::File audioFile, double rootFrequency, int samplePosition, bool loop) {
    if (samplePosition < 0 || samplePosition >= MAX_SAMPLES) return -2;
    if (samples[samplePosition].loaded) return -1;

    juce::AudioFormatReader* reader = manager.createReaderFor(audioFile);
    if (reader == nullptr) return -3;

    samples[samplePosition].filePath = audioFile.getFullPathName();
    samples[samplePosition].fileName = audioFile.getFileName();
    samples[samplePosition].loop = loop;
    samples[samplePosition].rootFrequency = rootFrequency;
    samples[samplePosition].rootSampleRate = reader->sampleRate;
    samples[samplePosition].buffer = new juce::AudioBuffer<float>;
    samples[samplePosition].sampleTime = 0;
    samples[samplePosition].loaded = true;
    samples[samplePosition].buffer->setSize(2, int(reader->lengthInSamples));
    if (reader->read(samples[samplePosition].buffer, 0, int(reader->lengthInSamples), 0, true, true) == false) unloadSample(samplePosition);
    else if (samples[samplePosition].buffer->getNumSamples() < 1) unloadSample(samplePosition);
    recalculateNumSamples();

    delete reader;

    if (samples[samplePosition].loaded == true) return samplePosition;
    else return -4;
}

// Returns true if a sample was deleted
bool SamplerSynthesizer::unloadSample(int samplePosition) {
    if (samplePosition < 0 || samplePosition >= MAX_SAMPLES) return false;
    if (samples[samplePosition].loaded == false) return false;
    samples[samplePosition].loaded = false;
    delete(samples[samplePosition].buffer);
    samples[samplePosition].buffer = nullptr;
    samples[samplePosition].filePath = "";
    samples[samplePosition].fileName = "Not Loaded";
    recalculateNumSamples();
    return true;
}

// Returns -1 if sample is out of bounds, otherwise returns position of current sample
int SamplerSynthesizer::chooseSample(int samplePosition) {
    if (samplePosition < 0 || samplePosition >= MAX_SAMPLES) return -1;
    if (currentSample >= 0 && currentSample < MAX_SAMPLES && samples[currentSample].loaded) samples[currentSample].sampleTime = time;
    currentSample = samplePosition;
    time = samples[currentSample].sampleTime;
    return currentSample;
}

void SamplerSynthesizer::noteOn(int note) {
    playing = true;
    this->note = note;
    sourceFrequency = targetFrequency;
    targetFrequency = midiNoteNumberToFrequency(note);
    frequency = targetFrequency * frequencyFactor;
    if (samples[currentSample].waitingForReset) {
        time = 0;
        samples[currentSample].waitingForReset = false;
    }
}

void SamplerSynthesizer::noteOff(int note) {
    if (note == this->note) playing = false;
}

void SamplerSynthesizer::reset(int pos) {
    samples[pos].waitingForReset = true;
}

// This basically uses a linked list which I hate but I kinda have to do it
void SamplerSynthesizer::getXmlState(juce::XmlElement* parent) {
    juce::XmlElement* main = parent->createNewChildElement("Synth");
    main->setAttribute("waitingForOuterReset", waitingForOuterReset);
    main->setAttribute("currentSample", currentSample);
    juce::XmlElement* slot = main;
    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (samples[i].loaded) {
            slot = slot->createNewChildElement("Slot");
            slot->setAttribute("slot", i);
            slot->setAttribute("rootFrequency", samples[i].rootFrequency);
            slot->setAttribute("loop", samples[i].loop);
            slot->setAttribute("sampleTime", samples[i].sampleTime);
            slot->setAttribute("filePath", samples[i].filePath);
            slot->setAttribute("waitingForReset", samples[i].waitingForReset);
        }
    }
}

void SamplerSynthesizer::loadXmlState(juce::XmlElement* state) {
    if (state == nullptr) return;
    waitingForOuterReset = state->getBoolAttribute("waitingForOuterReset", true);
    currentSample = state->getIntAttribute("currentSample", 0);
    juce::XmlElement* slot = state->getChildByName("Slot");
    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (slot != nullptr) {
            int slotNum = slot->getIntAttribute("slot", -1);
            if (slotNum >= 0 && slotNum < MAX_SAMPLES) {
                loadSample(juce::File(slot->getStringAttribute("filePath")), slot->getDoubleAttribute("rootFrequency"), slotNum, slot->getBoolAttribute("loop", true));
                samples[slotNum].sampleTime = slot->getDoubleAttribute("sampleTime", 0);
                samples[slotNum].waitingForReset = slot->getBoolAttribute("waitingForReset", true);
            }
            slot = slot->getChildByName("Slot");
        }
        else break;
    }
}

void SamplerSynthesizer::transpose(int semitones, double cents) {
    if (samples[currentSample].loaded) {
        samples[currentSample].rootFrequency = samples[currentSample].rootFrequency * (std::pow(2.0, (semitones + (cents / 100.0)) / 12.0));
    }
}
void SamplerSynthesizer::transpose(double newFrequency) {
    if (samples[currentSample].loaded) {
        samples[currentSample].rootFrequency = std::max(0.1, newFrequency);
    }
}