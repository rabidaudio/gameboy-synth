/*
  ==============================================================================

    WaveOscComponent.cpp
    Created: 13 Mar 2021 2:27:35am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveOscComponent.h"
#include "Theme.h"

//==============================================================================
WaveOscComponent::WaveOscComponent() : shapePicker("Shape")
{
    shapePicker.addItem("sine", 1);
    shapePicker.addItem("square", 2);
    shapePicker.addItem("triangle", 3);
    shapePicker.addItem("ramp up", 4);
    shapePicker.addItem("ramp down", 5);
    shapePicker.addItem("noise", 6);
    shapePicker.addListener(this);
    shapePicker.setSelectedId(1);
    addAndMakeVisible(shapePicker);

    addAndMakeVisible(wavetable);
}

WaveOscComponent::~WaveOscComponent() {}

void WaveOscComponent::paint(juce::Graphics& g) {
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void WaveOscComponent::resized()
{
    static int pickerPad = 5;
    static int pickerHeight = 25;
    static int pickerWidth = 150;
    shapePicker.setBounds(getLocalBounds().getWidth() - pickerWidth - pickerPad, pickerPad, pickerWidth, pickerHeight);
    juce::Rectangle<int> wavetableBounds = getLocalBounds();
    wavetableBounds.removeFromTop(pickerHeight + 2*pickerPad);
    wavetable.setBounds(wavetableBounds);
}

void WaveOscComponent::comboBoxChanged(juce::ComboBox *comboBox)
{
    jassert(comboBox == &shapePicker);
    switch (comboBox->getSelectedId()) {
        case 1:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_SINE);
        case 2:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_SQUARE);
        case 3:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_TRIANGLE);
        case 4:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_RAMP_UP);
        case 5:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_RAMP_DOWN);
        case 6:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_NOISE);
    }
}
