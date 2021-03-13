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
NoiseOscComponent::NoiseOscComponent() : controls(3)
{
    addAndMakeVisible(controls);
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
}
