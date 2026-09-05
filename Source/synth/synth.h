#ifndef _SYNTH_H_
#define _SYNTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "midi.h"

#ifndef NR50_REG

// if not in GBDK, define the registers
// define all the register addresses
#define NR50_REG 0xFF24
#define NR51_REG 0xFF25
#define NR52_REG 0xFF26
#define NR10_REG 0xFF10
#define NR11_REG 0xFF11
#define NR12_REG 0xFF12
#define NR13_REG 0xFF13
#define NR14_REG 0xFF14
#define NR21_REG 0xFF16
#define NR22_REG 0xFF17
#define NR23_REG 0xFF18
#define NR24_REG 0xFF19
#define NR30_REG 0xFF1A
#define NR31_REG 0xFF1B
#define NR32_REG 0xFF1C
#define NR33_REG 0xFF1D
#define NR34_REG 0xFF1E
#define NR41_REG 0xFF20
#define NR42_REG 0xFF21 
#define NR43_REG 0xFF22
#define NR44_REG 0xFF23

#endif

#define OSC3_WAV_RAM 0xFF30
#define OSC3_WAV_RAM_SIZE 16 // 32x 4bit samples

#define REG_OSC1 NR11_REG
#define REG_OSC2 NR21_REG
#define REG_NRX1 0 // length+duty for osc1+2
#define REG_NRX2 1 // vol+env for osc1+2
#define REG_NRX3 2 // period low for osc1+2
#define REG_NRX4 3 // period high and ctrl for osc1+2

// this is the interface this library uses to control the APU
extern void apu_setRegister(uint16_t addr, uint8_t value);
extern uint8_t apu_getRegister(uint16_t addr);

#define SYNTH_OSC1 0
#define SYNTH_OSC2 1
#define SYNTH_OSC3 2
#define SYNTH_OSC4 3

#define SYNTH_DUTY_12_5 0x00
#define SYNTH_DUTY_25 0x01
#define SYNTH_DUTY_50 0x02
#define SYNTH_DUTY_75 0x03

#define SYNTH_PAN_BOTH 0x11
#define SYNTH_PAN_L 0x10
#define SYNTH_PAN_R 0x01

#define SYNTH_CHANNEL_STATE_OMNIMODE (1 << 7)
#define SYNTH_CHANNEL_STATE_POLYMODE (1 << 6)

typedef struct {
    uint8_t note;
    uint8_t velocity;
    // envelope state

    // for LFOs and pitch bends, the amount to adjust the
    // period by
    int16_t pitchOffset;
} OscState;

// settings common to all 4 osc
typedef struct {
    // offset the incoming MIDI note
    int8_t noteOffset;
    uint8_t volume; // 4 bits
    uint8_t pan; // 2 bits, SYNTH_PAN_*
    bool applyVelocity; // if true, adjust volume by current velocity
    
    OscState state;
} Osc;

typedef struct {
    Osc common;
    uint8_t duty; // 2 bits
    uint8_t envelope; // 4 bits attack, 4 bits release
} Osc12;

typedef struct {
    Osc common;
    uint8_t wavetable[OSC3_WAV_RAM_SIZE];
} Osc3;

typedef struct {
    Osc common;
} Osc4;

typedef struct {
    Osc12 osc1;
    Osc12 osc2;
    // Osc3 osc3;
    // Osc4 osc4;

    // Channel State
    // [7] Omni Mode [6] Poly Mode .. [3] Ch4 enabled [2] Ch3 [1] Ch2 [0] Ch1
    uint8_t channelStates[MIDI_NUM_CHANNELS];
} Synth;


void synth_init(Synth*);

void synth_handleMidiEvent(Synth*, MidiEvent*);

#ifdef __cplusplus
}
#endif

#endif // _SYNTH_H_
