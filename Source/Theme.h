/*
  ==============================================================================

    Theme.h
    Created: 27 Feb 2021 11:38:27pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

static const int KeyboardHeight = 64;
static const int WindowWidth = 800;
static const int WindowHeight = 600;
static const int OscBoxWidth = WindowWidth / 2;
static const int OscBoxHeight = (WindowHeight - KeyboardHeight) / 2;

enum GameBoyColorIds {
    OscOutlineColorId
};

static void theme(juce::LookAndFeel& lookAndFeel)
{
    lookAndFeel.setColour(GameBoyColorIds::OscOutlineColorId, juce::Colours::black);
}
