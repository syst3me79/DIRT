#include "PluginEditor.h"

DirtAudioProcessorEditor::DirtAudioProcessorEditor (DirtAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (600, 470);
    title.setText ("DIRT  /  AUDIO LAB", juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setFont (juce::Font (juce::FontOptions (32.0f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, juce::Colour (0xfff5f4f4));
    addAndMakeVisible (title);

    const std::array<const char*, 10> names { "TUBE", "MECH", "CUTOFF", "RES", "MODE", "SLOPE", "COMP", "PHAT", "VOLUME", "MIX" };
    const std::array<const char*, 10> ids { "tube", "mech", "cutoff", "resonance", "mode", "slope", "comp", "phat", "volume", "mix" };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& slider = knobs[i];
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
        slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff73bbc4));
        slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff4f2f2));
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffc7c4d2));
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);
        labels[i].setText (names[i], juce::dontSendNotification);
        labels[i].setJustificationType (juce::Justification::centred);
        labels[i].setColour (juce::Label::textColourId, juce::Colour (i == 0 ? 0xfff08a45 : 0xffa19daf));
        labels[i].setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
        addAndMakeVisible (labels[i]);
        attachments[i] = std::make_unique<Attachment> (processor.parameters, ids[i], slider);
    }
}

void DirtAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff101014));
    g.setColour (juce::Colour (0xff26252d));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (12.0f), 15.0f, 1.0f);
    g.setColour (juce::Colour (0xff72b9c3));
    g.fillEllipse (getWidth() * 0.5f - 3.0f, 83.0f, 6.0f, 6.0f);

    juce::Path filterIcon;
    filterIcon.addCentredArc (getWidth() - 95.0f, 111.0f, 19.0f, 19.0f, 0.0f, 0.3f, 5.3f, true);
    g.setColour (juce::Colour (0xff718d39));
    g.strokePath (filterIcon, juce::PathStrokeType (4.0f));
}

void DirtAudioProcessorEditor::resized()
{
    title.setBounds (20, 18, getWidth() - 40, 48);
    const int left = 28, top = 88, cellW = (getWidth() - 56) / 3;
    for (int i = 0; i < 10; ++i)
    {
        const int row = i < 6 ? 0 : 1;
        const int col = i < 6 ? i % 3 : (i - 6) % 4;
        const int width = row == 0 ? cellW : (getWidth() - 56) / 4;
        const int x = left + col * width;
        const int y = row == 0 ? top + (i / 3) * 12 : 290;
        knobs[static_cast<size_t> (i)].setBounds (x + 7, y, width - 14, row == 0 ? 92 : 84);
        labels[static_cast<size_t> (i)].setBounds (x, y + (row == 0 ? 86 : 80), width, 22);
    }
}
