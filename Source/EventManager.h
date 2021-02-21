/*
  ==============================================================================

    EventManager.h
    Created: 21 Feb 2021 1:25:48pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#pragma once

#include "midimanager/midimanager.h"
#include "Gb_Snd_Emu-0.1.4/Basic_Gb_Apu.h"

typedef unsigned int uint;

// Register definitions
static const uint OscCount = 4;

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

struct GBMidiConfig {
    bool enabled;
    unsigned int channel;
    unsigned int voice;
    int transpose;
};

enum GBDutyCycle: uint8_t {
    DUTY_CYCLE_12_5 = 0x00, DUTY_CYCLE_25 = 0x01, DUTY_CYCLE_50 = 0x02, DUTY_CYCLE_75 = 0x03
};

// unlike the square and noise waves with velocity of 4 bits,
// the wave has 2 bits: 00=0%, 01=100%, 10=50%, 11=25%
enum GBWaveVolume: uint8_t {
    WAVE_VOL_OFF = 0x00, WAVE_VOL_FULL = 0x01, WAVE_VOL_50 = 0x02, WAVE_VOL_25 = 0x03
};

class Oscillator {
protected:
    Basic_Gb_Apu* apu_;
    uint16_t startAddr_;

    virtual void afterInit() = 0;

public:
    Oscillator(uint16_t startAddr) {
        startAddr_ = startAddr;
    }
    virtual ~Oscillator() = 0;

    void setApu(Basic_Gb_Apu* apu) {
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

class SquareOscilator: public Oscillator {
private:
    GBDutyCycle duty_ = DUTY_CYCLE_50;

public:
    SquareOscilator(uint16_t startAddr) : Oscillator(startAddr) {};
    ~SquareOscilator() {}
    void setDuty(GBDutyCycle duty);
    void setEvent(MidiEvent event);

protected:
    void afterInit();
};

class SquareOscilatorOne : public SquareOscilator {
    // TODO: frequency envelope
public:
    SquareOscilatorOne() : SquareOscilator(Sq1Addr) {}
    ~SquareOscilatorOne() {}
};

class SquareOscilatorTwo : public SquareOscilator {
public:
    SquareOscilatorTwo() : SquareOscilator(Sq2Addr) {}
    ~SquareOscilatorTwo() {}
};

class WaveOscillator : public Oscillator {
public:
    WaveOscillator(): Oscillator(WaveAddr) {}
    ~WaveOscillator() {}
    void setEvent(MidiEvent event);

protected:
    void afterInit();
private:
    GBWaveVolume midiVelocityToWaveVolume(uint8_t velocity);
    void setVelocity(uint8_t velocity);
    void setWaveTable(uint8_t* samples);
};

// Track MIDI state, which is separate from the register settings,
// and convert MIDI events into register calls
// TODO: this guy can also be a FIFO queue for changes from the UI
class EventManager {
private:
    // since there are only 4 oscillators, we won't need more than
    // 4 channels, so we pre-allocate all of them.
    // TODO: support channels
//    MidiManager<16, OscCount> managers_[OscCount];
//    uint channels_[OscCount]; // key = manager index, value = channel id
    GBMidiConfig configs_[OscCount];
    MidiManager<16, 4> manager_;
    Basic_Gb_Apu* apu_;
    SquareOscilatorOne osc_1_;
    SquareOscilatorTwo osc_2_;
    WaveOscillator osc_3_;
    // NoiseOscillator osc_4_;
    Oscillator* oscs_[OscCount] = { &osc_1_, &osc_2_, &osc_3_ /*, &osc_4_*/ };

public:
    // since this class is used from objc, it can't have an argument constructor.
    // instead we initialize it with an init method
    // https://stackoverflow.com/a/12467491
    EventManager() {}

    void init(Basic_Gb_Apu* apu);
    void setConfig(uint oscillator, GBMidiConfig config);
    void handleMIDIEvent(long time, const uint8_t* data);
};
