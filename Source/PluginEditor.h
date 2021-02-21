/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SquareOscComponent.h"

//==============================================================================
/**
*/
class GameBoySynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    GameBoySynthAudioProcessorEditor (GameBoySynthAudioProcessor&);
    ~GameBoySynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GameBoySynthAudioProcessor& audioProcessor;
    SquareOscComponent osc1;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GameBoySynthAudioProcessorEditor)
};
