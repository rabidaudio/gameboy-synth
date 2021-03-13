/*
  ==============================================================================

    SquareOscComponent.h
    Created: 21 Feb 2021 5:09:02pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"
#include "BasicControlsComponent.h"

//==============================================================================
/*
*/
class SquareOscComponent  : public juce::Component
{
public:
    SquareOscComponent(OSCID id);
    ~SquareOscComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BasicControlsComponent controls;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareOscComponent)
};
