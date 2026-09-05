/*
  ==============================================================================

    BasicControlsComponent.cpp
    Created: 13 Mar 2021 10:16:57am
    Author:  Julien Dorothy Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BasicControlsComponent.h"
#include "Apu.h"

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
        return 0;
//        return SquareOscilator::normalizeDutyCycle(valueToRemap);
    }
    PWMRange() : juce::NormalisableRange<double>(0, 100, denormalize, normalize, snap) {}
};

BasicControlsComponent::BasicControlsComponent(OSCID id) :
    enableButton("Enable"),
    pwmSlider("PWM"),
    voicePicker("Voice"),
    channelPicker("Channel"),
    transposePicker("Transpose"),
    attackSlider("Attack"),
    releaseSlider("Release")
{
    id_ = id;

    // enable
    enableButton.addListener(this);
    enableButton.setToggleState(id == 0 || id == 1, juce::sendNotification);
    addAndMakeVisible(enableButton);
    // pwm
    if (id == 0 || id == 1) {
        pwmSlider.addListener(this);
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
    voicePicker.addListener(this);
    voicePicker.setSelectedId(id == 1 ? 2 : 1);
    addAndMakeVisible(voicePicker);
    // channel
    for (int i = 1; i <= 16; i++) {
        channelPicker.addItem(std::to_string(i), i);
    }
    channelPicker.addListener(this);
    channelPicker.setSelectedId(1);
    addAndMakeVisible(channelPicker);
    // transpose
    for (int i = -48; i <= 48; i++) {
        transposePicker.addItem(std::to_string(i), i + 48 + 1);
    }
    transposePicker.addListener(this);
    transposePicker.setSelectedId(48 + 1);
    addAndMakeVisible(transposePicker);
    // envelope
    if (id != 2) {
        attackSlider.addListener(this);
        attackSlider.setSliderStyle(juce::Slider::Rotary);
        attackSlider.setRange(0, 7, 1);
        attackSlider.setValue(0);
        attackSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
        attackSlider.setNumDecimalPlacesToDisplay(0);
        addAndMakeVisible(attackSlider);
        
        releaseSlider.addListener(this);
        releaseSlider.setSliderStyle(juce::Slider::Rotary);
        releaseSlider.setRange(0, 7, 1);
        releaseSlider.setValue(0);
        releaseSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
        releaseSlider.setNumDecimalPlacesToDisplay(0);
        addAndMakeVisible(releaseSlider);
    }
}

BasicControlsComponent::~BasicControlsComponent() {}

void BasicControlsComponent::paint (juce::Graphics& g) {}

void BasicControlsComponent::resized()
{
    juce::Rectangle<int> bounds = getLocalBounds();
    int left = 0;
    int height = bounds.getHeight();
    // enable
    enableButton.setBounds(left, 0, height, height);
    left = enableButton.getBounds().getRight();
    // pwm
    if (id_ == 0 || id_ == 1) {
        pwmSlider.setBounds(left, 0, height, height);
        pwmSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, pwmSlider.getBounds().getWidth(), pwmSlider.getBounds().getHeight()/4);
        left = pwmSlider.getBounds().getRight();
    }
    // pickers
    static int pickerHeight = 25;
    int pickerPad = std::max((height - (3 * pickerHeight)) / 2, 0);
    voicePicker.setBounds(left, pickerPad, height, pickerHeight);
    channelPicker.setBounds(left, voicePicker.getBounds().getBottom(), height, pickerHeight);
    transposePicker.setBounds(left, channelPicker.getBounds().getBottom(), height, pickerHeight);
    left = transposePicker.getBounds().getRight();
    // envelope
    attackSlider.setBounds(left, 0, height, height);
    attackSlider.setTextBoxStyle(attackSlider.TextBoxBelow, true, attackSlider.getBounds().getWidth(), attackSlider.getBounds().getHeight()/4);
    left = attackSlider.getBounds().getRight();
    
    releaseSlider.setBounds(left, 0, height, height);
    releaseSlider.setTextBoxStyle(releaseSlider.TextBoxBelow, true, releaseSlider.getBounds().getWidth(), releaseSlider.getBounds().getHeight()/4);
    left = releaseSlider.getBounds().getRight();
}

void BasicControlsComponent::buttonClicked(juce::Button* button)
{
//    APU_INSTANCE.setEnabled(id_, button->getToggleState());
}

void BasicControlsComponent::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &pwmSlider) {
        jassert(id_ == 0 || id_ == 1);
//        APU_INSTANCE.setDutyCycle(id_, slider->getValue());
    } else if (slider == &attackSlider) {
        jassert(id_ != 2);
//        APU_INSTANCE.setAttackPeriod(id_, slider->getValue());
    } else if (slider == &releaseSlider) {
        jassert(id_ != 2);
//        APU_INSTANCE.setReleasePeriod(id_, slider->getValue());
    }
}

void BasicControlsComponent::comboBoxChanged(juce::ComboBox *comboBox)
{
    if (comboBox == &voicePicker) {
//        APU_INSTANCE.setMIDIVoice(id_, comboBox->getSelectedId() - 1);
    } else if (comboBox == &channelPicker) {
//        APU_INSTANCE.setMIDIChannel(id_, comboBox->getSelectedId() - 1);
    } else if (comboBox == &transposePicker) {
//        APU_INSTANCE.setTranspose(id_, comboBox->getSelectedId() - 48 - 1);
    }
}
