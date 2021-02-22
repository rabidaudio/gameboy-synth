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
SquareOscComponent::SquareOscComponent() :
    enableButton("Enable"),
    pwmSlider("PWM")
{
    addAndMakeVisible(enableButton);
    pwmSlider.setSliderStyle(juce::Slider::Rotary);
    pwmSlider.setRange(12.5, 75, 12.5);
    pwmSlider.setValue(50);
    addAndMakeVisible(pwmSlider);
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
//    enableButton.setBounds(getLocalBounds());
    pwmSlider.setBounds(getLocalBounds());
//    enableButton.setBounds(0, 0, enableButton.getWidth(), enableButton.getHeight());
//    pwmSlider.setBounds(enableButton.getWidth(), 0, enableButton.getWidth() + pwmSlider.getWidth(), pwmSlider.getHeight());
}
