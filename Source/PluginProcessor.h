#pragma once
#include <JuceHeader.h>
#include "ClipperDSP.h"

class RavenClipperAudioProcessor : public juce::AudioProcessor
{
public:
    RavenClipperAudioProcessor();
    ~RavenClipperAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RavenClipper"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // Vom Audio-Thread geschrieben, vom Editor gelesen (exchange(0) = "seit letztem Mal")
    std::atomic<float> inPeak { 0.0f }, outPeak { 0.0f }, activity { 0.0f };

private:
    void updateOversampling (int index);

    juce::dsp::DryWetMixer<float> mixer;
    std::array<std::unique_ptr<juce::dsp::Oversampling<float>>, 4> oversamplers; // [0] = aus
    int activeOs = -1;

    juce::SmoothedValue<float> preSm, postSm, edgeSm;

    std::atomic<float> *pDrive = nullptr, *pEdge = nullptr, *pShape = nullptr, *pCeil = nullptr,
                       *pMix = nullptr, *pOut = nullptr, *pOs = nullptr, *pAuto = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RavenClipperAudioProcessor)
};
