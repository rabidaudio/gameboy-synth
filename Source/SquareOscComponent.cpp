/*
  ==============================================================================

    SquareOscComponent.cpp
    Created: 21 Feb 2021 5:09:02pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SquareOscComponent.h"
#include "Synth.h"
#include "Theme.h"

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

//==============================================================================
SquareOscComponent::SquareOscComponent() :
    enableButton("Enable"),
    volSlider("Volume"),
    pwmSlider("PWM"),
    voicePicker("Voice"),
    channelPicker("Channel"),
    transposePicker("Transpose")
{
    // enable
    addAndMakeVisible(enableButton);
    // volume
    volSlider.setSliderStyle(juce::Slider::Rotary);
    volSlider.setRange(0, 16, 1);
    volSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
    volSlider.setNumDecimalPlacesToDisplay(0);
    volSlider.setValue(15);
    addAndMakeVisible(volSlider);
    // pwm
    pwmSlider.setSliderStyle(juce::Slider::Rotary);
    pwmSlider.setNormalisableRange(PWMRange());
    pwmSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, 0, 0);
    pwmSlider.setNumDecimalPlacesToDisplay(1);
    pwmSlider.setValue(50);
    addAndMakeVisible(pwmSlider);
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
}

SquareOscComponent::~SquareOscComponent()
{
}

void SquareOscComponent::paint(juce::Graphics& g)
{
    g.setColour(getLookAndFeel().findColour(GameBoyColorIds::OscOutlineColorId));
    g.drawRect(getLocalBounds());
}

void SquareOscComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    juce::Rectangle<int> bounds = getLocalBounds();
    int upperBlockUnit = bounds.proportionOfHeight(0.25);
    // enable
    enableButton.setBounds(0, 0, upperBlockUnit, upperBlockUnit);
    // volume
    volSlider.setBounds(enableButton.getBounds().getRight(), 0, upperBlockUnit, upperBlockUnit);
    volSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, volSlider.getBounds().getWidth(), volSlider.getBounds().getHeight()/4);
    // pwm
    pwmSlider.setBounds(volSlider.getBounds().getRight(), 0, upperBlockUnit, upperBlockUnit);
    pwmSlider.setTextBoxStyle(pwmSlider.TextBoxBelow, true, pwmSlider.getBounds().getWidth(), pwmSlider.getBounds().getHeight()/4);
    // channel
    int pickerHeight = 25;
    int pickerPad = (upperBlockUnit - (3 * pickerHeight)) / 2;
    voicePicker.setBounds(pwmSlider.getBounds().getRight(), pickerPad, upperBlockUnit, pickerHeight);
    channelPicker.setBounds(pwmSlider.getBounds().getRight(), voicePicker.getBounds().getBottom(), upperBlockUnit, pickerHeight);
    transposePicker.setBounds(pwmSlider.getBounds().getRight(), channelPicker.getBounds().getBottom(), upperBlockUnit, pickerHeight);
}
