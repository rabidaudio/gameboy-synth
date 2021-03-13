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
    // osc 1
    osc1.enableButton.addListener(this);
    osc1.enableButton.setToggleState(true, juce::sendNotification);
    osc1.volSlider.addListener(this);
    osc1.pwmSlider.addListener(this);
    osc1.voicePicker.addListener(this);
    osc1.transposePicker.addListener(this);
    addAndMakeVisible(osc1);
    // osc 2
    osc2.enableButton.addListener(this);
    osc2.enableButton.setToggleState(true, juce::sendNotification);
    osc2.volSlider.addListener(this);
    osc2.pwmSlider.addListener(this);
    osc2.voicePicker.addListener(this);
    osc2.transposePicker.addListener(this);
    addAndMakeVisible(osc2);
    // osc 3
    osc3.wavetable.addChangeListener(this);
    addAndMakeVisible(osc3);
    // keyboard
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
    osc3.setBounds(0, OscBoxHeight, OscBoxWidth, OscBoxHeight);
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
    } else if (slider == &osc1.volSlider) {
        audioProcessor.synth.setVolume(0, ((float) slider->getValue()) / 15.0);
    } else if (slider == &osc2.volSlider) {
        audioProcessor.synth.setVolume(1, ((float) slider->getValue()) / 15.0);
    }
}

void GameBoySynthAudioProcessorEditor::comboBoxChanged(juce::ComboBox *comboBox)
{
    if (comboBox == &osc1.voicePicker) {
        audioProcessor.synth.setMIDIVoice(0, comboBox->getSelectedId() - 1);
    } else if (comboBox == &osc2.voicePicker) {
        audioProcessor.synth.setMIDIVoice(1, comboBox->getSelectedId() - 1);
    } else if (comboBox == &osc1.channelPicker) {
        audioProcessor.synth.setMIDIChannel(0, comboBox->getSelectedId() - 1);
    } else if (comboBox == &osc2.channelPicker) {
        audioProcessor.synth.setMIDIChannel(1, comboBox->getSelectedId() - 1);
    } else if (comboBox == &osc1.transposePicker) {
        audioProcessor.synth.setTranspose(0, comboBox->getSelectedId() - 48 - 1);
    } else if (comboBox == &osc2.transposePicker) {
        audioProcessor.synth.setTranspose(1, comboBox->getSelectedId() - 48 - 1);
    }
}

void GameBoySynthAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &osc3.wavetable) {
        audioProcessor.synth.setWaveTable(osc3.wavetable.wavetable);
    }
}
