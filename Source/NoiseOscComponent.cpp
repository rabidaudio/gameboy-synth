/*
  ==============================================================================

    NoiseOscComponent.cpp
    Created: 13 Mar 2021 2:09:59pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NoiseOscComponent.h"
#include "Theme.h"

//==============================================================================
NoiseOscComponent::NoiseOscComponent() : controls(3),
shiftSlider("Clock Shift"),
counterStep("Width"),
ratioSlider("Divisor")
{
    addAndMakeVisible(controls);

    // 524288 / r / 2^(s+1) (r=0.5 instead of 0)
    shiftSlider.setRange(0, 15, 1);
    shiftSlider.setSliderStyle(juce::Slider::Rotary);
    shiftSlider.setValue(0);
    shiftSlider.setTextBoxStyle(shiftSlider.TextBoxBelow, true, 0, 0);
    addAndMakeVisible(shiftSlider);

    addAndMakeVisible(counterStep);

    ratioSlider.setRange(0, 7, 1);
    ratioSlider.setSliderStyle(juce::Slider::Rotary);
    ratioSlider.setTextBoxStyle(ratioSlider.TextBoxBelow, true, 0, 0);
    ratioSlider.setValue(0);
    addAndMakeVisible(ratioSlider);

    shiftSlider.addListener(this);
    counterStep.addListener(this);
    ratioSlider.addListener(this);
}

NoiseOscComponent::~NoiseOscComponent() {}

void NoiseOscComponent::paint(juce::Graphics& g)
{
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void NoiseOscComponent::resized()
{
    int upperBlockUnit = getLocalBounds().proportionOfHeight(0.25);
    controls.setBounds(0, 0, getLocalBounds().getWidth(), upperBlockUnit);

    shiftSlider.setBounds(0, upperBlockUnit, upperBlockUnit, upperBlockUnit);
    counterStep.setBounds(upperBlockUnit, upperBlockUnit, upperBlockUnit, upperBlockUnit);
    ratioSlider.setBounds(upperBlockUnit*2, upperBlockUnit, upperBlockUnit, upperBlockUnit);
}

void NoiseOscComponent::buttonClicked(juce::Button *button)
{
    jassert(button == &counterStep);
    Synth::INSTANCE.setNoiseCounterWidth(button->getToggleStateValue().getValue());
}

void NoiseOscComponent::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &shiftSlider) {
        Synth::INSTANCE.setNoiseShiftFrequency(slider->getValue());
    } else if (slider == &ratioSlider) {
        Synth::INSTANCE.setNoiseDividerRatio(slider->getValue());
    }
}
