#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto tubeId = "tube";
constexpr auto mechId = "mech";
constexpr auto cutoffId = "cutoff";
constexpr auto resonanceId = "resonance";
constexpr auto modeId = "mode";
constexpr auto slopeId = "slope";
constexpr auto compId = "comp";
constexpr auto phatId = "phat";
constexpr auto volumeId = "volume";
constexpr auto mixId = "mix";
}

DirtAudioProcessor::DirtAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "DIRT_STATE", createParameterLayout())
{
}

DirtAudioProcessor::Parameters::ParameterLayout DirtAudioProcessor::createParameterLayout()
{
    Parameters::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { tubeId, 1 }, "Tube", juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { mechId, 1 }, "Mech", juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { cutoffId, 1 }, "Cutoff", juce::NormalisableRange<float> { 20.0f, 20000.0f, 1.0f, 0.25f }, 12000.0f, "Hz"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { resonanceId, 1 }, "Resonance", juce::NormalisableRange<float> { 0.1f, 10.0f }, 0.707f));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { modeId, 1 }, "Filter mode", juce::StringArray { "LP", "BP", "HP" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { slopeId, 1 }, "Slope", juce::StringArray { "12", "24", "36" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { compId, 1 }, "Comp", juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { phatId, 1 }, "Phat", juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { volumeId, 1 }, "Volume", juce::NormalisableRange<float> { -24.0f, 12.0f }, 0.0f, "dB"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { mixId, 1 }, "Mix", juce::NormalisableRange<float> { 0.0f, 1.0f }, 1.0f));
    return layout;
}

void DirtAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    envelope.fill (0.0f);
    for (auto& channel : filters)
        for (auto& filter : channel)
            filter.reset();
}

void DirtAudioProcessor::releaseResources() {}

bool DirtAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void DirtAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();
    for (int ch = inputChannels; ch < outputChannels; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const auto value = [this] (const char* id) { return parameters.getRawParameterValue (id)->load(); };
    const float tube = value (tubeId), mech = value (mechId);
    const float cutoff = juce::jlimit (20.0f, static_cast<float> (currentSampleRate * 0.45), value (cutoffId));
    const float resonance = value (resonanceId);
    const int mode = static_cast<int> (value (modeId));
    const int stages = static_cast<int> (value (slopeId)) + 1;
    const float comp = value (compId), phat = value (phatId);
    const float outputGain = juce::Decibels::decibelsToGain (value (volumeId));
    const float mix = value (mixId);

    for (int ch = 0; ch < juce::jmin (inputChannels, 2); ++ch)
    {
        auto& channelFilters = filters[static_cast<size_t> (ch)];
        for (int stage = 0; stage < 3; ++stage)
        {
            if (mode == 0)
                *channelFilters[static_cast<size_t> (stage)].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, cutoff, resonance);
            else if (mode == 1)
                *channelFilters[static_cast<size_t> (stage)].coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (currentSampleRate, cutoff, resonance);
            else
                *channelFilters[static_cast<size_t> (stage)].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, cutoff, resonance);
        }

        auto* samples = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = samples[i];
            float x = dry;
            const float level = std::abs (x);
            envelope[static_cast<size_t> (ch)] += (level - envelope[static_cast<size_t> (ch)]) * (level > envelope[static_cast<size_t> (ch)] ? 0.08f : 0.002f);

            // Simple, intentionally broadband dynamics: COMP attenuates peaks; PHAT lifts quieter material.
            const float compGain = 1.0f / (1.0f + comp * juce::jmax (0.0f, envelope[static_cast<size_t> (ch)] - 0.25f) * 5.0f);
            const float phatGain = 1.0f + phat * juce::jmax (0.0f, 0.25f - envelope[static_cast<size_t> (ch)]) * 2.0f;
            x *= compGain * phatGain;

            if (tube > 0.0001f)
            {
                const float drive = 1.0f + tube * 9.0f;
                const float shaped = std::tanh (x * drive) / std::tanh (drive);
                x = juce::jmap (tube, x, shaped);
            }
            if (mech > 0.0001f)
            {
                const float bias = mech * 0.22f;
                const float shaped = (2.0f / juce::MathConstants<float>::pi) * std::atan ((x + bias) * (1.0f + mech * 10.0f));
                x = juce::jmap (mech, x, shaped);
            }

            for (int stage = 0; stage < stages; ++stage)
                x = channelFilters[static_cast<size_t> (stage)].processSample (x);
            samples[i] = ((1.0f - mix) * dry + mix * x) * outputGain;
        }
    }
}

juce::AudioProcessorEditor* DirtAudioProcessor::createEditor() { return new DirtAudioProcessorEditor (*this); }

void DirtAudioProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    if (auto state = parameters.copyState(); auto xml = state.createXml())
        copyXmlToBinary (*xml, destination);
}

void DirtAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size); xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DirtAudioProcessor(); }
