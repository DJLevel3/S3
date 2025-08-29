/*
  ==============================================================================

    SamplerSynthesizer.h
    Created: 28 Aug 2025 7:23:16am
    Author:  DJ_Level_3

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define MAX_SAMPLES 100

struct SampleSlot {
    juce::AudioBuffer<float>* buffer = nullptr;
    double rootFrequency = 0;
    double rootSampleRate = 192000;
    bool loop = true;
    bool loaded = false;
    bool waitingForReset = true;
    double sampleTime = 0;
    juce::String fileName = "Not Loaded";
    juce::String filePath = "";
};

//==============================================================================
/*
*/
class SamplerSynthesizer
{
public:
    SamplerSynthesizer();
    ~SamplerSynthesizer();

    void prepareToPlay(double sampleRate) {
        this->sampleRate = sampleRate;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, int beginSample, int endSample);

    int loadSample(juce::File audioFile, double rootFrequency, int samplePosition, bool loop = true);
    int loadSample(juce::File audioFile, int rootNote, int samplePosition, bool loop = true) {
        return loadSample(audioFile, midiNoteNumberToFrequency(rootNote), samplePosition, loop);
    }

    bool unloadSample(int samplePosition);
    int chooseSample(int samplePosition);
    bool moveSample(int sourcePosition, int destPosition, bool force);

    int getNumSamples() {
        return numSamples;
    }

    int getCurrentSample() {
        return currentSample;
    }

    juce::String getCurrentSampleName() {
        return samples[currentSample].fileName;
    }

    juce::String getSampleName(int sample) {
        if (sample < 0 || sample >= MAX_SAMPLES) return "";
        return samples[sample].fileName;
    }

    void setCurrentSampleLoop(bool loop) {
        if (currentSample < 0) return;
        samples[currentSample].loop = loop;
    }
    void setCurrentSampleRootFrequency(double frequency) {
        if (currentSample < 0) return;
        samples[currentSample].rootFrequency = frequency;
    }
    void setCurrentSampleRootNote(int note) {
        if (currentSample < 0) return;
        samples[currentSample].rootFrequency = midiNoteNumberToFrequency(note);
    }

    std::vector<int> getLoadedSamples() {
        std::vector<int> loaded;
        numSamples = 0;
        for (int i = 0; i < MAX_SAMPLES; i++) {
            if (samples[i].loaded) {
                loaded.push_back(i);
                numSamples++;
            }
        }
        return loaded;
    }

    void noteOn(int number);
    void noteOff(int number);
    void noteOff() {
        playing = false;
    }

    void reset(int pos);
    void reset() {
        reset(currentSample);
        waitingForOuterReset = true;
    }
    void resetAllSamples() {
        for (int i = 0; i < MAX_SAMPLES; i++) {
            reset(i);
        }
        waitingForOuterReset = true;
    }

    int getOpenSample() {
        for (int i = 0; i < MAX_SAMPLES; i++) {
            if (!samples[i].loaded) {
                return i;
            }
        }
        return -1;
    }

    int chooseNextSample() {
        for (int i = currentSample + 1; i < MAX_SAMPLES; i++) {
            if (samples[i].loaded) {
                return chooseSample(i);
            }
        }
        for (int i = 0; i <= currentSample; i++) {
            if (samples[i].loaded) {
                return chooseSample(i);
            }
        }
        return -1;
    }

    int choosePrevSample() {
        for (int i = currentSample - 1; i >= 0; i--) {
            if (samples[i].loaded) {
                return chooseSample(i);
            }
        }
        for (int i = MAX_SAMPLES - 1; i >= currentSample; i--) {
            if (samples[i].loaded) {
                return chooseSample(i);
            }
        }
        return -1;
    }

    void getXmlState(juce::XmlElement* parent);
    void loadXmlState(juce::XmlElement* state);

private:
    void recalculateNumSamples() {
        numSamples = 0;
        for (int i = 0; i < MAX_SAMPLES; i++) {
            if (samples[i].loaded) {
                numSamples++;
            }
        }
    }

    static double midiNoteNumberToFrequency(int midiNoteNumber) {
        return 440.0 * std::pow(2.0, (midiNoteNumber - 69.0) / 12.0);
    }

    static float lerp_f(float start, float end, float t) {
        return (end - start) * t + start;
    }

    double sampleRate = 192000;

    SampleSlot samples[MAX_SAMPLES];
    int numSamples = 0;
    int currentSample = -1;

    double time = 0;
    int note = -1;
    bool playing = false;
    double frequency = -1;
    double sourceFrequency = -1;
    double targetFrequency = -1;

    bool waitingForOuterReset = true;

    juce::AudioFormatManager manager;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthesizer)
};
