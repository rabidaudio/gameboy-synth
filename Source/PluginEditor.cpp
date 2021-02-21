/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GameBoySynthAudioProcessorEditor::GameBoySynthAudioProcessorEditor (GameBoySynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
        keyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize (400, 300);
    addAndMakeVisible(osc1);
    addAndMakeVisible(keyboard);
    keyboardState.addListener(audioProcessor.getMidiCollector());
}

GameBoySynthAudioProcessorEditor::~GameBoySynthAudioProcessorEditor()
{
}

//==============================================================================
void GameBoySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void GameBoySynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    osc1.setBounds(getLocalBounds());
//    keyboard.setBounds(0, getHeight()-64, getWidth(), getHeight());
    keyboard.setBounds(0, getHeight()-64, getWidth(), 64);
}
