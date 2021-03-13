/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SquareOscComponent.h"
#include "WaveOscComponent.h"
#include "Theme.h"

//==============================================================================
/**
*/
class GameBoySynthAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                            public juce::Button::Listener,
                                            public juce::Slider::Listener,
                                            public juce::ComboBox::Listener,
                                            public juce::ChangeListener
{
public:
    GameBoySynthAudioProcessorEditor (GameBoySynthAudioProcessor&);
    ~GameBoySynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox *comboBox) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GameBoySynthAudioProcessor& audioProcessor;
    SquareOscComponent osc1;
    SquareOscComponent osc2;
    WaveOscComponent osc3;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GameBoySynthAudioProcessorEditor)
};
