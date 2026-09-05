/*
  ==============================================================================

    SynthWrapper.h
    Created: 5 Sep 2026 5:29:46pm
    Author:  Julien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "synth/synth.h"
#include "Gb_Snd_Emu-0.1.4-patched/gb_apu/Gb_Apu.h"
#include "Gb_Snd_Emu-0.1.4-patched/gb_apu/Multi_Buffer.h"

static const gb_time_t CLOCK_SPEED = 4194304;
static const gb_time_t CLOCKS_PER_INSTRUCTION = 4;
static const gb_time_t CLOCKS_PER_FRAME = 16384;

void apuSetRegister(uint16_t addr, uint8_t value);
uint8_t apuGetRegister(uint16_t addr);

// This class bridges JUCE, GB_Apu, and synth together.
// It has a singleton INSTANCE
class SynthWrapper
{
private:
    Gb_Apu apu_;
    Synth synth_;
    Stereo_Buffer sbuf_;
    Mono_Buffer mbuf_;
    Multi_Buffer* buf_;
    bool stereo_;
    blip_time_t clock_;
    blip_sample_t samples_[2];
    blip_time_t timeToNextFrame_;
    
public:
    SynthWrapper();
    ~SynthWrapper();
    
    static SynthWrapper INSTANCE;
    
    void configure(double sampleRate, int channels);
    uint8_t readRegister(gb_addr_t addr);
    void writeRegister(gb_addr_t addr, uint8_t value);
    long samplesAvailable();
    gb_time_t readSamples(juce::AudioBuffer<float>* out);
    void handleMIDI(juce::MidiBuffer& midiMessages);
    void reset();
    
private:
    blip_time_t tick(blip_time_t step);
};
