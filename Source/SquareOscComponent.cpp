/*
  ==============================================================================

    SquareOscComponent.cpp
    Created: 21 Feb 2021 5:09:02pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SquareOscComponent.h"

//==============================================================================
SquareOscComponent::SquareOscComponent() : enableButton("Enable")
{
    addAndMakeVisible(enableButton);
}

SquareOscComponent::~SquareOscComponent()
{
}

void SquareOscComponent::paint (juce::Graphics& g)
{
}

void SquareOscComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    enableButton.setBounds(getLocalBounds());
}
