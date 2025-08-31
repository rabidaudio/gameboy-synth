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
NoiseOscComponent::NoiseOscComponent() :
    controls(3),
    ratioSlider("Ratio"),
    shiftWidthPicker("Width"),
    shiftFrequencySlider("Frequency")
{
    addAndMakeVisible(controls);

    ratioSlider.addListener(this);
    ratioSlider.setSliderStyle(juce::Slider::Rotary);
    ratioSlider.setRange(0, 7, 1);
    ratioSlider.setValue(7);
    ratioSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(ratioSlider);

    shiftWidthPicker.addListener(this);
    shiftWidthPicker.addItem("15", 1);
    shiftWidthPicker.addItem("7", 2);
    shiftWidthPicker.setSelectedId(1);
    addAndMakeVisible(shiftWidthPicker);

    shiftFrequencySlider.addListener(this);
    shiftFrequencySlider.setSliderStyle(juce::Slider::Rotary);
    shiftFrequencySlider.setRange(0, 15, 1);
    shiftFrequencySlider.setValue(5);
    shiftFrequencySlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(shiftFrequencySlider);
}

NoiseOscComponent::~NoiseOscComponent() {}

void NoiseOscComponent::paint(juce::Graphics& g)
{
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void NoiseOscComponent::resized()
{
    juce::Rectangle<int> bounds = getLocalBounds();
    int rowUnit = bounds.proportionOfHeight(0.25);
    controls.setBounds(0, 0, bounds.getWidth(), rowUnit);

    int left = 0;
    ratioSlider.setBounds(left, rowUnit, rowUnit, rowUnit);
    ratioSlider.setTextBoxStyle(ratioSlider.TextBoxBelow, true, rowUnit, rowUnit / 4);
    left = ratioSlider.getBounds().getRight();

    static int pickerHeight = 25;
    shiftWidthPicker.setBounds(left, rowUnit * 2 - pickerHeight, rowUnit, pickerHeight);
    left = shiftWidthPicker.getBounds().getRight();

    shiftFrequencySlider.setBounds(left, rowUnit, rowUnit, rowUnit);
    shiftFrequencySlider.setTextBoxStyle(shiftFrequencySlider.TextBoxBelow, true, rowUnit, rowUnit / 4);
}

void NoiseOscComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &ratioSlider) {
        Synth::INSTANCE.setDividingRatio((uint8_t) slider->getValue());
    } else if (slider == &shiftFrequencySlider) {
        Synth::INSTANCE.setShiftFrequency((uint8_t) slider->getValue());
    }
}

void NoiseOscComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &shiftWidthPicker) {
        Synth::INSTANCE.setShiftWidth((NoiseShiftWidth) (comboBox->getSelectedId() - 1));
    }
}
