/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Synth.h"

//==============================================================================
GameBoySynthAudioProcessorEditor::GameBoySynthAudioProcessorEditor(GameBoySynthAudioProcessor& p)
    : AudioProcessorEditor (&p),
        audioProcessor (p),
        osc0(0),
        osc1(1),
        keyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    theme(getLookAndFeel());
    setSize(WindowWidth, WindowHeight);

    addAndMakeVisible(osc0);
    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    addAndMakeVisible(osc3);
    // keyboard
    keyboardState.addListener(audioProcessor.getMidiCollector());
    addAndMakeVisible(keyboard);
}

GameBoySynthAudioProcessorEditor::~GameBoySynthAudioProcessorEditor() {}

//==============================================================================
void GameBoySynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GameBoySynthAudioProcessorEditor::resized()
{
    osc0.setBounds(0, 0, OscBoxWidth, OscBoxHeight);
    osc1.setBounds(OscBoxWidth, 0, OscBoxWidth, OscBoxHeight);
    osc2.setBounds(0, OscBoxHeight, OscBoxWidth, OscBoxHeight);
    osc3.setBounds(OscBoxWidth, OscBoxHeight, OscBoxWidth, OscBoxHeight);
    keyboard.setBounds(0, WindowHeight-KeyboardHeight, WindowWidth, KeyboardHeight);
}
