/*
  ==============================================================================

    NoiseOscComponent.cpp
    Created: 13 Mar 2021 2:09:59pm
    Author:  Julien Dorothy Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NoiseOscComponent.h"
#include "Theme.h"

//==============================================================================
NoiseOscComponent::NoiseOscComponent() :
    controls(3),
    shiftWidthPicker("Width")
{
    addAndMakeVisible(controls);

    shiftWidthPicker.addListener(this);
    shiftWidthPicker.addItem("15", 1);
    shiftWidthPicker.addItem("7", 2);
    shiftWidthPicker.setSelectedId(1);
    addAndMakeVisible(shiftWidthPicker);
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

    int left = rowUnit;
    static int pickerHeight = 25;
    shiftWidthPicker.setBounds(left, rowUnit + rowUnit / 2 - pickerHeight / 2, rowUnit, pickerHeight);
}

void NoiseOscComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &shiftWidthPicker) {
//        APU_INSTANCE.setShiftWidth((NoiseShiftWidth) (comboBox->getSelectedId() - 1));
    }
}
