#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RavenField.h"

namespace raven
{
    const juce::Colour night  { 0xff0b0a12 };
    const juce::Colour indigo { 0xff1b1838 };
    const juce::Colour violet { 0xff8b6cff };
    const juce::Colour silver { 0xffc9c6dd };
    const juce::Colour gold   { 0xffffc247 };
}

class RavenLook : public juce::LookAndFeel_V4
{
public:
    RavenLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle, juce::Slider&) override;
};

// Zeigt live die Übertragungskurve (Input -> Output) mit aktuellen Reglern.
class CurveView : public juce::Component
{
public:
    explicit CurveView (juce::AudioProcessorValueTreeState& s) : apvts (s)
    {
        setInterceptsMouseClicks (false, false);
    }
    void paint (juce::Graphics&) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
};

class RavenClipperAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RavenClipperAudioProcessorEditor (RavenClipperAudioProcessor&);
    ~RavenClipperAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct Knob { juce::Slider slider; juce::Label label; };

    RavenClipperAudioProcessor& proc;
    RavenLook look;

    RavenField ravens;
    CurveView curve;
    Knob driveK, edgeK, ceilK, mixK, outK;
    juce::ComboBox shapeBox, osBox;
    juce::Label shapeL, osL;
    juce::ToggleButton autoBtn;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> driveA, edgeA, ceilA, mixA, outA;
    std::unique_ptr<CA> shapeA, osA;
    std::unique_ptr<BA> autoA;

    float inLvl = 0.0f, outLvl = 0.0f, agitation = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RavenClipperAudioProcessorEditor)
};
