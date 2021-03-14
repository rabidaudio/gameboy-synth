/*
  ==============================================================================

    NoiseOscComponent.h
    Created: 13 Mar 2021 2:09:59pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BasicControlsComponent.h"

//==============================================================================
/*
*/
class NoiseOscComponent  : public juce::Component,
                            public juce::Button::Listener,
                            public juce::Slider::Listener
{
public:
    NoiseOscComponent();
    ~NoiseOscComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    BasicControlsComponent controls;
    juce::Slider shiftSlider;
    juce::ToggleButton counterStep;
    juce::Slider ratioSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseOscComponent)
};
