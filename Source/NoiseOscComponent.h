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
class NoiseOscComponent  : public juce::Component
{
public:
    NoiseOscComponent();
    ~NoiseOscComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BasicControlsComponent controls;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseOscComponent)
};
