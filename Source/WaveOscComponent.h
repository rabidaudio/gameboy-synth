/*
  ==============================================================================

    WaveOscComponent.h
    Created: 13 Mar 2021 2:27:35am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BasicControlsComponent.h"
#include "WavetableComponent.h"

//==============================================================================
/*
*/
class WaveOscComponent  : public juce::Component,
                            public juce::ComboBox::Listener,
                            public juce::ChangeListener
{
public:
    WaveOscComponent();
    ~WaveOscComponent() override;

    WavetableComponent wavetable;

    void paint (juce::Graphics&) override;
    void resized() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    BasicControlsComponent controls;
    juce::ComboBox shapePicker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveOscComponent)
};
