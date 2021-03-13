/*
  ==============================================================================

    WavetableComponent.cpp
    Created: 13 Mar 2021 12:19:30am
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WavetableComponent.h"


//==============================================================================

static int widthUnits = WAVE_TABLE_SIZE;
static int heightUnits = 16;

WavetableComponent::WavetableComponent() {}

WavetableComponent::~WavetableComponent() {}

void WavetableComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black);
    g.fillRect(drawingBounds());
    g.setColour(juce::Colours::white);
    for (int x = 0; x < widthUnits; x++) {
        uint8_t y = wavetable[x];
        g.fillRect(boundsOfWavePixel(x, y));
    }
}

void WavetableComponent::resized() {}

int WavetableComponent::scaleFactor()
{
    return std::min(getLocalBounds().getWidth() / widthUnits, getLocalBounds().getHeight() / heightUnits);
}

int WavetableComponent::padX()
{
    return (getLocalBounds().getWidth() - WAVE_TABLE_SIZE*scaleFactor()) / 2;
}

int WavetableComponent::padY()
{
    return (getLocalBounds().getHeight() - heightUnits*scaleFactor()) / 2;
}

juce::Rectangle<int> WavetableComponent::drawingBounds()
{
    int sf = scaleFactor();
    return juce::Rectangle<int>(padX(), padY(), widthUnits*sf, heightUnits*sf);
}

juce::Rectangle<int> WavetableComponent::boundsOfWavePixel(int x, int y)
{
    int sf = scaleFactor();
    return juce::Rectangle<int>(x * sf + padX(), (heightUnits - y - 1) * sf + padY(), sf, sf);
}

void WavetableComponent::mouseDown(const juce::MouseEvent &event)
{
    enableWavePixel(event);
}

void WavetableComponent::mouseDrag(const juce::MouseEvent &event)
{
    enableWavePixel(event);
}

void WavetableComponent::mouseUp(const juce::MouseEvent &event)
{
    enableWavePixel(event);
    if (changed_) {
        // TODO: trigger wavetable update
        changed_ = false;
    }
}

void WavetableComponent::enableWavePixel(const juce::MouseEvent &event)
{
    if (!drawingBounds().contains(event.getPosition())) return;

    int sf = scaleFactor();
    int x = (event.getPosition().getX() - padX()) / sf;
    int y = heightUnits - ((event.getPosition().getY() - padY()) / sf) - 1;
    uint8_t currentValue = wavetable[x];
    if (currentValue != y) {
        changed_ = true;
        wavetable[x] = y;
        repaint(drawingBounds());
    }
}

void WavetableComponent::loadDefaultWavetable(const uint8_t* defaultWavetable)
{
    std::memcpy(wavetable, defaultWavetable, WAVE_TABLE_SIZE);
    repaint(drawingBounds());
}
