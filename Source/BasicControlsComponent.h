/*
  ==============================================================================

    BasicControlsComponent.h
    Created: 13 Mar 2021 10:16:57am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"

//==============================================================================
/*
*/
class BasicControlsComponent  : public juce::Component,
                                public juce::Button::Listener,
                                public juce::Slider::Listener,
                                public juce::ComboBox::Listener
{
public:
    BasicControlsComponent(OSCID id);
    ~BasicControlsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox *comboBox) override;

private:
    OSCID id_;
    juce::ToggleButton enableButton;
    juce::Slider volSlider;
    juce::Slider pwmSlider;
    juce::ComboBox voicePicker;
    juce::ComboBox channelPicker;
    juce::ComboBox transposePicker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicControlsComponent)
};
