/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GameBoySynthAudioProcessorEditor::GameBoySynthAudioProcessorEditor(GameBoySynthAudioProcessor& p)
    : AudioProcessorEditor (&p),
        audioProcessor (p),
        keyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    theme(getLookAndFeel());
    setSize(WindowWidth, WindowHeight);
    osc1.enableButton.addListener(this);
    osc1.enableButton.setToggleState(true, juce::sendNotification);
    osc1.pwmSlider.addListener(this);
    addAndMakeVisible(osc1);
    osc2.enableButton.addListener(this);
    osc2.enableButton.setToggleState(true, juce::sendNotification);
    osc2.pwmSlider.addListener(this);
    addAndMakeVisible(osc2);
    keyboardState.addListener(audioProcessor.getMidiCollector());
    addAndMakeVisible(keyboard);
}

GameBoySynthAudioProcessorEditor::~GameBoySynthAudioProcessorEditor() {}

//==============================================================================
void GameBoySynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GameBoySynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    osc1.setBounds(0, 0, OscBoxWidth, OscBoxHeight);
    osc2.setBounds(OscBoxWidth, 0, OscBoxWidth, OscBoxHeight);
    keyboard.setBounds(0, WindowHeight-KeyboardHeight, WindowWidth, KeyboardHeight);
}

void GameBoySynthAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &osc1.enableButton) {
        audioProcessor.synth.setEnabled(0, button->getToggleState());
    } else if (button == &osc2.enableButton) {
        audioProcessor.synth.setEnabled(1, button->getToggleState());
    }
}

void GameBoySynthAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &osc1.pwmSlider) {
        audioProcessor.synth.setDutyCycle(0, slider->getValue());
    } else if (slider == &osc2.pwmSlider) {
        audioProcessor.synth.setDutyCycle(1, slider->getValue());
    }
}
