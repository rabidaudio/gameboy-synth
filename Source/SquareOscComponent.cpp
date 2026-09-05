/*
  ==============================================================================

    SquareOscComponent.cpp
    Created: 21 Feb 2021 5:09:02pm
    Author:  Julien Dorothy Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SquareOscComponent.h"
#include "Theme.h"



//==============================================================================
SquareOscComponent::SquareOscComponent(size_t id) : controls(id)
{
    addAndMakeVisible(controls);
}

SquareOscComponent::~SquareOscComponent() {}

void SquareOscComponent::paint(juce::Graphics& g)
{
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void SquareOscComponent::resized()
{
    int upperBlockUnit = getLocalBounds().proportionOfHeight(0.25);
    controls.setBounds(0, 0, getLocalBounds().getWidth(), upperBlockUnit);
}
