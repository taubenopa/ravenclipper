#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout RavenClipperAudioProcessor::createLayout()
{
    auto dbText    = [] (float v, int) { return String (v, 1) + " dB"; };
    auto pctText   = [] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; };
    auto pctParse  = [] (const String& t) { return t.getFloatValue() / 100.0f; };

    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive",
        NormalisableRange<float> (0.0f, 24.0f, 0.01f), 4.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "edge", 1 }, "Edge",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f,
        AudioParameterFloatAttributes().withStringFromValueFunction (pctText)
                                       .withValueFromStringFunction (pctParse)));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "ceiling", 1 }, "Ceiling",
        NormalisableRange<float> (-12.0f, 0.0f, 0.01f), -0.3f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (pctText)
                                       .withValueFromStringFunction (pctParse)));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "output", 1 }, "Output",
        NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "shape", 1 }, "Shape",
        StringArray { "Tanh (smooth)", "Cubic (round)", "Sine (musical)", "Warm (asymmetric)" }, 0));

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "os", 1 }, "Oversampling",
        StringArray { "Off", "2x", "4x", "8x" }, 2));

    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "autogain", 1 }, "Auto Gain", false));

    return { p.begin(), p.end() };
}

RavenClipperAudioProcessor::RavenClipperAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout()),
      mixer (1024)
{
    pDrive = apvts.getRawParameterValue ("drive");
    pEdge  = apvts.getRawParameterValue ("edge");
    pShape = apvts.getRawParameterValue ("shape");
    pCeil  = apvts.getRawParameterValue ("ceiling");
    pMix   = apvts.getRawParameterValue ("mix");
    pOut   = apvts.getRawParameterValue ("output");
    pOs    = apvts.getRawParameterValue ("os");
    pAuto  = apvts.getRawParameterValue ("autogain");
}

bool RavenClipperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && (in == AudioChannelSet::mono() || in == AudioChannelSet::stereo());
}

void RavenClipperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto numCh = (uint32) 2;
    dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, numCh };

    mixer.prepare (spec);
    mixer.setMixingRule (dsp::DryWetMixingRule::linear);

    for (int i = 1; i < (int) oversamplers.size(); ++i)
    {
        // Linear-phase FIR: saubere Phase beim Parallel-Mix, Latenz wird dem Host gemeldet.
        oversamplers[(size_t) i] = std::make_unique<dsp::Oversampling<float>> (
            (size_t) numCh, (size_t) i,
            dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
        oversamplers[(size_t) i]->initProcessing ((size_t) samplesPerBlock);
    }

    const auto g = rc::computeGains (pDrive->load(), pCeil->load(), pOut->load(), pAuto->load() > 0.5f);
    preSm.reset  (sampleRate, 0.03);  preSm.setCurrentAndTargetValue (g.pre);
    postSm.reset (sampleRate, 0.03);  postSm.setCurrentAndTargetValue (g.post);
    edgeSm.reset (sampleRate, 0.05);  edgeSm.setCurrentAndTargetValue (rc::edgeToThreshold (pEdge->load()));

    activeOs = -1;
    updateOversampling ((int) pOs->load());
    mixer.reset();
}

void RavenClipperAudioProcessor::updateOversampling (int index)
{
    index = jlimit (0, 3, index);
    activeOs = index;

    float latency = 0.0f;
    if (index > 0 && oversamplers[(size_t) index] != nullptr)
    {
        oversamplers[(size_t) index]->reset();
        latency = oversamplers[(size_t) index]->getLatencyInSamples();
    }
    mixer.setWetLatency (latency);
    setLatencySamples (roundToInt (latency));
}

void RavenClipperAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const int n     = buffer.getNumSamples();
    const int numCh = jmin (getTotalNumInputChannels(), buffer.getNumChannels());
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, n);
    if (n == 0 || numCh == 0)
        return;

    const int  shape = (int) pShape->load();
    const int  osIdx = (int) pOs->load();
    const auto gains = rc::computeGains (pDrive->load(), pCeil->load(), pOut->load(), pAuto->load() > 0.5f);

    if (osIdx != activeOs)
        updateOversampling (osIdx);

    preSm.setTargetValue  (gains.pre);
    postSm.setTargetValue (gains.post);
    edgeSm.setTargetValue (rc::edgeToThreshold (pEdge->load()));

    float inMag = 0.0f;
    for (int ch = 0; ch < numCh; ++ch)
        inMag = jmax (inMag, buffer.getMagnitude (ch, 0, n));

    dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);

    mixer.setWetMixProportion (pMix->load());
    mixer.pushDrySamples (dsp::AudioBlock<const float> (block));

    // 1) Drive (+ Normierung auf Ceiling), sample-genau geglättet
    for (int i = 0; i < n; ++i)
    {
        const float g = preSm.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            buffer.getWritePointer (ch)[i] *= g;
    }

    // 2) Clipper, im oversampleten Bereich
    const float t0 = edgeSm.getCurrentValue();
    edgeSm.skip (n);
    const float t1 = edgeSm.getCurrentValue();

    size_t hot = 0, total = 0;
    auto clipBlock = [&] (dsp::AudioBlock<float>& b)
    {
        const size_t len = b.getNumSamples();
        const float  inv = 1.0f / (float) len;
        for (size_t ch = 0; ch < b.getNumChannels(); ++ch)
        {
            float* d = b.getChannelPointer (ch);
            for (size_t i = 0; i < len; ++i)
            {
                const float t = t0 + (t1 - t0) * (float) i * inv;
                const float x = d[i];
                if (std::abs (x) > 0.8f) ++hot;
                d[i] = rc::softClip (shape, x, t);
            }
        }
        total += len * b.getNumChannels();
    };

    if (activeOs <= 0 || oversamplers[(size_t) activeOs] == nullptr)
    {
        clipBlock (block);
    }
    else
    {
        auto& os = *oversamplers[(size_t) activeOs];
        auto up = os.processSamplesUp (dsp::AudioBlock<const float> (block));
        clipBlock (up);
        os.processSamplesDown (block);
    }

    // 3) zurück auf Ceiling, Output-Gain, (Auto-Gain)
    for (int i = 0; i < n; ++i)
    {
        const float g = postSm.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            buffer.getWritePointer (ch)[i] *= g;
    }

    // 4) Parallel-Mix (Dry wird um die Oversampling-Latenz verzögert)
    mixer.mixWetSamples (block);

    float outMag = 0.0f;
    for (int ch = 0; ch < numCh; ++ch)
        outMag = jmax (outMag, buffer.getMagnitude (ch, 0, n));

    inPeak.store  (jmax (inPeak.load(),  inMag));
    outPeak.store (jmax (outPeak.load(), outMag));
    activity.store (jmax (activity.load(), total > 0 ? (float) hot / (float) total : 0.0f));
}

AudioProcessorEditor* RavenClipperAudioProcessor::createEditor()
{
    return new RavenClipperAudioProcessorEditor (*this);
}

void RavenClipperAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RavenClipperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RavenClipperAudioProcessor();
}
