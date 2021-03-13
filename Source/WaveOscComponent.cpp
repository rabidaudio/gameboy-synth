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
WaveOscComponent::WaveOscComponent() : controls(2), shapePicker("Shape")
{
    addAndMakeVisible(controls);

    shapePicker.addItem("square", 1);
    shapePicker.addItem("sine", 2);
    shapePicker.addItem("triangle", 3);
    shapePicker.addItem("saw", 4);
    shapePicker.addItem("noise", 5);
    shapePicker.addListener(this);
    shapePicker.setSelectedId(2);
    shapePicker.setTextWhenNothingSelected("custom");
    addAndMakeVisible(shapePicker);

    wavetable.addChangeListener(this);
    addAndMakeVisible(wavetable);
}

WaveOscComponent::~WaveOscComponent() {}

void WaveOscComponent::paint(juce::Graphics& g) {
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void WaveOscComponent::resized()
{
    int upperBlockUnit = getLocalBounds().proportionOfHeight(0.25);
    controls.setBounds(0, 0, getLocalBounds().getWidth(), upperBlockUnit);

    static int pickerPad = 5;
    static int pickerHeight = 25;
    static int pickerWidth = 150;
    shapePicker.setBounds(getLocalBounds().getWidth() - pickerWidth - pickerPad, upperBlockUnit + pickerPad, pickerWidth, pickerHeight);
    juce::Rectangle<int> wavetableBounds = getLocalBounds();
    wavetableBounds.removeFromTop(upperBlockUnit + pickerHeight + 2*pickerPad);
    wavetable.setBounds(wavetableBounds);
}

void WaveOscComponent::comboBoxChanged(juce::ComboBox *comboBox)
{
    jassert(comboBox == &shapePicker);
    switch (comboBox->getSelectedId()) {
        case 1:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_SQUARE);
        case 2:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_SINE);
        case 3:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_TRIANGLE);
        case 4:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_SAW);
        case 5:
            return wavetable.loadDefaultWavetable(WAVE_TABLE_NOISE);
    }
}

void WaveOscComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    jassert(source == &wavetable);
    shapePicker.setSelectedId(0);
}
