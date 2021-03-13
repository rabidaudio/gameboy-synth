/*
  ==============================================================================

    BasicControlsComponent.cpp
    Created: 13 Mar 2021 10:16:57am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BasicControlsComponent.h"
#include "Synth.h"

//==============================================================================

class PWMRange : public juce::NormalisableRange<double> {
public:
    static double normalize(double rangeStart, double rangeEnd, double valueToRemap)
    {
        return valueToRemap / 100;
    }
    static double denormalize(double rangeStart, double rangeEnd, double valueToRemap)
    {
        return valueToRemap * 100;
    }
    static double snap(double rangeStart, double rangeEnd, double valueToRemap)
    {
        return SquareOscilator::normalizeDutyCycle(valueToRemap);
    }
    PWMRange() : juce::NormalisableRange<double>(0, 100, denormalize, normalize, snap) {}
};

BasicControlsComponent::BasicControlsComponent(OSCID id) :
    enableButton("Enable"),
    volSlider("Volume"),
    pwmSlider("PWM"),
    voicePicker("Voice"),
    channelPicker("Channel"),
    transposePicker("Transpose")
{
    id_ = id;

    // enable
    addAndMakeVisible(enableButton);
    // volume
    volSlider.setSliderStyle(juce::Slider::Rotary);
    volSlider.setRange(0, 15, 1);
    volSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
    volSlider.setNumDecimalPlacesToDisplay(0);
    volSlider.setValue(15);
    addAndMakeVisible(volSlider);
    // pwm
    if (id == 0 || id == 1) {
        pwmSlider.setSliderStyle(juce::Slider::Rotary);
        pwmSlider.setNormalisableRange(PWMRange());
        pwmSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
        pwmSlider.setNumDecimalPlacesToDisplay(1);
        pwmSlider.setValue(50);
        addAndMakeVisible(pwmSlider);
    }
    // voice
    for (int i = 1; i <= 4; i++) {
        voicePicker.addItem(std::to_string(i), i);
    }
    voicePicker.setSelectedId(1);
    addAndMakeVisible(voicePicker);
    // channel
    for (int i = 1; i <= 16; i++) {
        channelPicker.addItem(std::to_string(i), i);
    }
    channelPicker.setSelectedId(1);
    addAndMakeVisible(channelPicker);
    // transpose
    for (int i = -48; i <= 48; i++) {
        transposePicker.addItem(std::to_string(i), i + 48 + 1);
    }
    transposePicker.setSelectedId(48 + 1);
    addAndMakeVisible(transposePicker);

    enableButton.addListener(this);
    enableButton.setToggleState(true, juce::sendNotification);
    volSlider.addListener(this);
    pwmSlider.addListener(this);
    voicePicker.addListener(this);
}

BasicControlsComponent::~BasicControlsComponent() {}

void BasicControlsComponent::paint (juce::Graphics& g) {}

void BasicControlsComponent::resized()
{
    juce::Rectangle<int> bounds = getLocalBounds();
    int upperBlockUnit = bounds.proportionOfHeight(0.25);
    int left = 0;
    // enable
    enableButton.setBounds(left, 0, upperBlockUnit, upperBlockUnit);
    left = enableButton.getBounds().getRight();
    // volume
    volSlider.setBounds(left, 0, upperBlockUnit, upperBlockUnit);
    volSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, volSlider.getBounds().getWidth(), volSlider.getBounds().getHeight()/4);
    left = volSlider.getBounds().getRight();
    // pwm
    if (id_ == 0 || id_ == 1) {
        pwmSlider.setBounds(left, 0, upperBlockUnit, upperBlockUnit);
        pwmSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, pwmSlider.getBounds().getWidth(), pwmSlider.getBounds().getHeight()/4);
        left = pwmSlider.getBounds().getRight();
    }
    // pickers
    static int pickerHeight = 25;
    int pickerPad = (upperBlockUnit - (3 * pickerHeight)) / 2;
    voicePicker.setBounds(left, pickerPad, upperBlockUnit, pickerHeight);
    channelPicker.setBounds(left, voicePicker.getBounds().getBottom(), upperBlockUnit, pickerHeight);
    transposePicker.setBounds(left, channelPicker.getBounds().getBottom(), upperBlockUnit, pickerHeight);
}

void BasicControlsComponent::buttonClicked(juce::Button* button)
{
    Synth::INSTANCE.setEnabled(id_, button->getToggleState());
}

void BasicControlsComponent::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &pwmSlider) {
        jassert(id_ == 0 || id_ == 1);
        Synth::INSTANCE.setDutyCycle(id_, slider->getValue());
    } else if (slider == &volSlider) {
        // TODO: less if osc3
        Synth::INSTANCE.setVolume(id_, ((float) slider->getValue()) / 15.0);
    }
}

void BasicControlsComponent::comboBoxChanged(juce::ComboBox *comboBox)
{
    if (comboBox == &voicePicker) {
        Synth::INSTANCE.setMIDIVoice(id_, comboBox->getSelectedId() - 1);
    } else if (comboBox == &channelPicker) {
        Synth::INSTANCE.setMIDIChannel(id_, comboBox->getSelectedId() - 1);
    } else if (comboBox == &transposePicker) {
        Synth::INSTANCE.setTranspose(id_, comboBox->getSelectedId() - 48 - 1);
    }
}
