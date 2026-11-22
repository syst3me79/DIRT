#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DirtAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DirtAudioProcessorEditor (DirtAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    DirtAudioProcessor& processor;
    juce::Label title;
    std::array<juce::Slider, 10> knobs;
    std::array<juce::Label, 10> labels;
    std::array<std::unique_ptr<Attachment>, 10> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirtAudioProcessorEditor)
};
