/*
  ==============================================================================

    EventManager.h
    Created: 21 Feb 2021 1:25:48pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "midimanager/midimanager.h"
#include "Gb_Snd_Emu-0.1.4/gb_apu/Gb_Apu.h"
#include "Gb_Snd_Emu-0.1.4/gb_apu/Multi_Buffer.h"

static const gb_time_t CLOCK_SPEED = 4194304;
static const gb_time_t CLOCKS_PER_INSTRUCTION = 4;

// Register definitions
typedef uint8_t OSCID;
static const OSCID NUM_OSC = 4;

static const uint16_t Sq1Addr = 0xFF10; // Square 1 (with freq envelope)
static const uint16_t Sq2Addr = 0xFF15; // Square 2
static const uint16_t WaveAddr = 0xFF1A; // Wave
static const uint16_t NoiseAddr = 0xFF1F; // Noise

// NR10 FF10 -PPP NSSS Sweep period, negate, shift (Osc 1 only)
// NR30 FF1A E--- ---- DAC power (Osc 3 only)
static const uint16_t NRX0 = 0;

// NR11 FF11 DDLL LLLL Duty, Length load (64-L) (Osc 1,2,4)
// NR31 FF1B LLLL LLLL Length load (256-L) (Osc 3)
static const uint16_t NRX1 = 1;

// NR12 FF12 VVVV APPP Starting volume, Envelope add mode, period (Osc 1,2,4)
// NR32 FF1C -VV- ---- Volume code (00=0%, 01=100%, 10=50%, 11=25%) (Osc 3)
static const uint16_t NRX2 = 2;

// NR13 FF13 FFFF FFFF Frequency LSB (Osc 1,2,3)
// NR43 FF22 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code (Osc 4)
static const uint16_t NRX3 = 3;

// NR14 FF14 TL-- -FFF Trigger, Length enable, Frequency MSB (Osc 1,2,3)
// NR44 FF23 TL-- ---- Trigger, Length enable (Osc 4)
static const uint16_t NRX4 = 4;

// NR50 FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol
static const uint16_t NR50 = 0xFF24;
//NR51 FF25 NW21 NW21 Left enables, Right enables
static const uint16_t NR51 = 0xFF25;
// NR52 FF26 P--- NW21 Power control/status, Channel length statuses
static const uint16_t NR52 = 0xFF26;
static const uint16_t WaveTableAddr = 0xFF30;

struct MidiConfig {
    bool enabled;
    uint8_t channel;
    uint8_t voice;
    int8_t transpose;
};

enum class DutyCycle: uint8_t
{
    duty12_5 = 0x00, duty25 = 0x01, duty50 = 0x02, duty75 = 0x03
};

// unlike the square and noise waves with velocity of 4 bits,
// the wave has 2 bits: 00=0%, 01=100%, 10=50%, 11=25%
enum GBWaveVolume: uint8_t
{
    WAVE_VOL_OFF = 0x00, WAVE_VOL_FULL = 0x01, WAVE_VOL_50 = 0x02, WAVE_VOL_25 = 0x03
};

class Apu
{
private:
    Gb_Apu apu_;
    Stereo_Buffer sbuf_;
    Mono_Buffer mbuf_;
    Multi_Buffer* buf_;
    bool stereo_;
    blip_time_t clock_;
    blip_sample_t samples_[2];

public:
    Apu();
    ~Apu();

    void configure(double sampleRate, int channels);
    void writeRegister(gb_addr_t addr, uint8_t data);
    uint8_t readRegister(gb_addr_t addr);

    long samplesAvailable();
    void readSamples(juce::AudioBuffer<float>* out);

    void reset();

private:
    blip_time_t tick() { return clock_ += 4; }
};

class Oscillator {
protected:
    Apu* apu_;
    uint16_t startAddr_;

    virtual void afterInit() = 0;

public:
    Oscillator(OSCID id)
    {
        static const uint16_t addrs[NUM_OSC] = {Sq1Addr, Sq2Addr, WaveAddr, NoiseAddr};
        startAddr_ = addrs[id];
    }
    virtual ~Oscillator() = 0;

    void setApu(Apu* apu)
    {
        apu_ = apu;
        afterInit();
    }
    virtual void setEvent(MidiEvent event) = 0;

private:
    uint16_t midiNoteToPeriod(uint8_t note);
    uint8_t midiVelocityTo4BitVolume(uint8_t velocity);

protected:
    // NRX3 and lower NRX4, Osc 1,2,3 only
    void set11BitPeriod(uint8_t note);

    // NRX2, Osc 1,2,4 only
    // Note: if you want to trigger the envelope, you must set it before NRX3
    void setVolumeEnvelope(uint8_t startVelocity, bool increasing, uint8_t period);
    void setConstantVolume(uint8_t velocity);
};

class SquareOscilator: public Oscillator
{
private:
    DutyCycle duty_ = DutyCycle::duty50;

public:
    SquareOscilator(OSCID id) : Oscillator(id) {};
    ~SquareOscilator() {}
    void setDuty(DutyCycle duty);
    void setEvent(MidiEvent event);

    static DutyCycle dutyCycleFromValue(double value)
    {
        if (value < 18.75) {
            return DutyCycle::duty12_5;
        } else if (value < 37.5) {
            return DutyCycle::duty25;
        } else if (value < 62.5) {
            return DutyCycle::duty50;
        } else {
            return DutyCycle::duty75;
        }
    }

protected:
    void afterInit();
};

class SquareOscilatorOne : public SquareOscilator
{
    // TODO: frequency envelope
public:
    SquareOscilatorOne() : SquareOscilator(0) {}
    ~SquareOscilatorOne() {}
};

class SquareOscilatorTwo : public SquareOscilator
{
public:
    SquareOscilatorTwo() : SquareOscilator(1) {}
    ~SquareOscilatorTwo() {}
};

class WaveOscillator : public Oscillator
{
public:
    WaveOscillator(): Oscillator(2) {}
    ~WaveOscillator() {}
    void setEvent(MidiEvent event);
    void setWaveTable(uint8_t* samples);

protected:
    void afterInit();
private:
    GBWaveVolume midiVelocityToWaveVolume(uint8_t velocity);
    void setVelocity(uint8_t velocity);
};

class NoiseOscillator : public Oscillator
{
public:
    NoiseOscillator(): Oscillator(3) {}
    ~NoiseOscillator() {}
    void setEvent(MidiEvent event);

protected:
    void afterInit();
};

// Track MIDI state, which is separate from the register settings,
// and convert MIDI events into register calls
// TODO: this guy can also be a FIFO queue for changes from the UI
class Synth
{
private:
    // since there are only 4 oscillators, we won't need more than
    // 4 channels, so we pre-allocate all of them.
    // TODO: support channels
//    MidiManager<16, NUM_OSC> managers_[NUM_OSC];
//    uint channels_[NUM_OSC]; // key = manager index, value = channel id
    MidiConfig configs_[NUM_OSC];
    MidiManager<16, 4> manager_;
    Apu apu_;
    SquareOscilatorOne osc1;
    SquareOscilatorTwo osc2;
    WaveOscillator osc3;
    NoiseOscillator osc4;
    Oscillator* oscs_[NUM_OSC] = { &osc1, &osc2, &osc3, &osc4 };

public:
    Synth();

    void configure(double sampleRate, int channels);

    void setEnabled(OSCID oscillator, bool enabled);
    void setTranspose(OSCID oscillator, int8_t transpose);
    void setMIDIVoice(OSCID oscillator, uint8_t voice);
    void setMIDIChannel(OSCID oscillator, uint8_t channel);

    void setDutyCycle(OSCID oscillator, double value)
    {
        DutyCycle duty = SquareOscilator::dutyCycleFromValue(value);
        switch (oscillator)
        {
            case 0: return osc1.setDuty(duty);
            case 1: return osc2.setDuty(duty);
            default: return;
        }
    }

    void setWaveTable(uint8_t* samples)
    {
        osc3.setWaveTable(samples);
    }

    void handleMIDI(juce::MidiBuffer& midiMessages);
    void readSamples(juce::AudioBuffer<float>* out);

    void setDefaults();
    void stop();

private:
    void reconfigure(OSCID oscillator);
    void handleMIDIEvent(juce::MidiMessage msg);
};
