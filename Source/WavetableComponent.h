/*
  ==============================================================================

    WavetableComponent.h
    Created: 13 Mar 2021 12:19:30am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"

//==============================================================================
/*
*/
class WavetableComponent  : public juce::Component,
                            public juce::ChangeBroadcaster
{
public:
    WavetableComponent();
    ~WavetableComponent() override;

    uint8_t wavetable[WAVE_TABLE_SIZE];

    void paint (juce::Graphics&) override;
    void resized() override;

    void loadDefaultWavetable(const uint8_t* defaultWavetable);

    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    bool changed_ = false;

    int padX();
    int padY();
    int scaleFactor();
    juce::Rectangle<int> drawingBounds();
    juce::Rectangle<int> boundsOfWavePixel(int x, int y);
    void enableWavePixel(const juce::MouseEvent &event);
    void wavetableChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableComponent)
};
