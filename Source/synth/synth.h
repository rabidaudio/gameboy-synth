#ifndef _SYNTH_H_
#define _SYNTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "midi.h"

#define GB_NR50 0xFF24
#define GB_NR51 0xFF25
#define GB_NR52 0xFF26
#define GB_NR10 0xFF10
#define GB_NR11 0xFF11
#define GB_NR12 0xFF12
#define GB_NR13 0xFF13
#define GB_NR14 0xFF14
#define GB_NR15 0xFF15 // not an audio register?
#define GB_NR21 0xFF16
#define GB_NR22 0xFF17
#define GB_NR23 0xFF18
#define GB_NR24 0xFF19
#define GB_NR30 0xFF1A
#define GB_NR31 0xFF1B
#define GB_NR32 0xFF1C
#define GB_NR33 0xFF1D
#define GB_NR34 0xFF1E
#define GB_NR41 0xFF20
#define GB_NR42 0xFF21 
#define GB_NR43 0xFF22
#define GB_NR44 0xFF23

#define SYNTH_NUM_OSCS 4

#define OSC3_WAV_RAM 0xFF30
#define OSC3_WAV_RAM_SIZE 16 // 32x 4bit samples

#define SYNTH_OSC1 0
#define SYNTH_OSC2 1
#define SYNTH_OSC3 2
#define SYNTH_OSC4 3

// duty[2]+len[6] for 1,2, len[8] for 3, len[6] for 4
#define NRx1(oscid) (GB_NR11 + 5*oscid)
// volume+env for 1,2,4
#define NRx2(oscid) (GB_NR12 + 5*oscid)
// period-low for 1,2,3
#define NRx3(oscid) (GB_NR13 + 5*oscid)
// trigger,len-enable, period high for 1,2,3
#define NRx4(oscid) (GB_NR14 + 5*oscid)

#define SYNTH_DUTY_12_5 0x00
#define SYNTH_DUTY_25 0x01
#define SYNTH_DUTY_50 0x02
#define SYNTH_DUTY_75 0x03


#define SYNTH_PAN_L (1 << 4)
#define SYNTH_PAN_R (1 << 0)
#define SYNTH_PAN_BOTH (SYNTH_PAN_L | SYNTH_PAN_R)

#define SYNTH_ENV_UP (1 << 3)
#define SYNTH_ENV_DOWN (0 << 3)

#define SYNTH_CHANNEL_STATE_OMNIMODE (1 << 7)
#define SYNTH_CHANNEL_STATE_POLYMODE (1 << 6)

#define SYNTH_LENGTH_HOLD 0

// state byte holds note playback state machine:
// [0] note on or off
// [1] is in envelope (attack or release)
// [2] _reserved_
// [3] if in envelope, is it attack (1) or release (0). matches `SYNTH_ENV_UP/_DOWN`
// [4] is the note being held until midi release (1), or is it using a fixed length (0)
#define SYNTH_STATE_OFF 0
#define SYTNTH_STATE_ON 1
//#define SYNTH_STATE_ENV_ACTIVE (1 << 1)
//#define SYNTH_STATE_ENV_UP SYNTH_ENV_UP // or down
#define SYNTH_STATE_HOLD (1 << 4)


/*
 [Channel]      [Sweep] [Frequency]     [Wave Form] [Length Timer]  [Volume]
 Square 1       Sweep   Period Counter  Duty        Length Timer    Envelope
 Square 2               Period Counter  Duty        Length Timer    Envelope
 Wave                   Period Counter  Wave        Length Timer    Volume
 Noise                  Period Counter  LFSR        Length Timer    Envelope
 */

// this is for state that would persist across presets
typedef struct {
    // offset the incoming MIDI note
    int8_t transpose;
    uint8_t volume; // stored as 8 bits for better resolution. Hardware resolution is 2 bits for osc3 else 4 bits
    uint8_t pan; // 4 bits, see SYNTH_PAN_*
    
    // if 0, keep note active for as long as note is held down
    // otherwise use the note-length trigger.
    // ticks at 256Hz, counting up to 64 (CH1, CH2, and CH4) or 256 (CH3)
    // meaning max length is 250ms or 1s
    // includes duty for osc1+2
    uint8_t length; // 6 bits for osc1,2 4. 8 bits for 3
    // TODO: can this bit go elsewhere?
    // TODO: applyVelocity implies len==0
    bool applyVelocity; // if true, adjust volume by current velocity
} OscConfig;

// this is for transient state
typedef struct {
    // the note currently played on keys (independant of other pitch offsets)
    uint8_t note;
    uint8_t velocity;
    // envelope state
    
    // need to store this separately for pitch bends, portamento, lfos, etc
    int16_t periodOffset;    
} OscState;

typedef struct {
    OscConfig confs[SYNTH_NUM_OSCS];
    OscState states[SYNTH_NUM_OSCS];
    // store this here so it isn't duplicated
    uint8_t osc3_wavetable[OSC3_WAV_RAM_SIZE];

    // MIDI Channel State
    // [7] Omni Mode [6] Poly Mode .. [3] Ch4 enabled [2] Ch3 [1] Ch2 [0] Ch1
    uint8_t channelStates[MIDI_NUM_CHANNELS];
    
    uint8_t masterVolume;
    int8_t masterPan;
} Synth;

// By declaring a single instance, we keep the gameboy arguments minimal.
// by declaring as extern, we allow the calling code to configure it's memory location
extern Synth GLOBAL_SYNTH;
// // this is the interface this library uses to control the APU
extern void apu_writeRegister(uint16_t addr, uint8_t val);
extern uint8_t apu_readRegister(uint16_t addr);

void synth_init(void);

// load the configuration of a preset from memory location into the synth.
// if data == NULL, loads the default settings
void synth_loadPreset(uint8_t* data);

// write the current configuration as a preset to the given memory location. Returns number of bytes written
uint8_t synth_savePreset(uint8_t* data);

/**
 midiChannel: 0-15
 flags:
    `SYNTH_CHANNEL_STATE_OMNIMODE` allow oscillator to respond to all channels
    `SYNTH_CHANNEL_STATE_POLYMODE` enable polyphonic playback (notes will be round-robined through assigned oscillators)
    `(1 << SYNTH_OSCx)` to turn on oscillators for the given MIDI channel. An oscillator assigned to multiple channels
 */
//void synth_setChannel(Synth*, uint8_t midiChannel, uint8_t flags);

/**
 Volume has resolution of 8 bits but will be scaled down to the oscillator's resolution (4 or 2 bits).
 
 Takes effect on next note.
 */
void synth_setVolume(uint8_t oscid, uint8_t volume);

/**
 pan: one of `SYNTH_PAN_*`
 */
void synth_setPan(uint8_t oscid, uint8_t pan);

/**
 Volume has a set resolution of 8 bits. Actual resolution is at most 3 bits, but will be dependant on masterPan as it's the same register.
 */
//void synth_setMasterVolume(Synth*, uint8_t volume);

/**
 Unlike per-oscillator pan, this has a resolution of 4 bits signed. 0=center, -7 = full left, 7=full right.
 The resolution will be dependant on masterVolume as it's the same register.
 */
//void synth_setMasterPan(Synth*, uint8_t oscid, int8_t pan);

/**
 Set the hold time for the note. If set to `SYNTH_LENGTH_HOLD`, is held for as long as the note is held down. Otherwise,
 for Osc 1, 2, and 4: 6 bits, number of 256Hz ticks (i.e. ~4ms to ~246ms)
 for Osc 3: 8 bits, number of 256Hz ticks (i.e. ~4ms to ~1s)
 
 Takes effect on next note.
 */
void synth_setLength(uint8_t oscid, uint8_t length);

/**
 Transpose an incoming number by a number of chromatic steps. Will wrap around.
 Note that supported note ranges are 36 to 127 inclusive for osc1-3.  If note after offset is outside
 this range the note will not sound.
 
 Takes effect on next note.
 */
void synth_setTranspose(uint8_t oscid, int8_t offset);

/**
 Set duty cycle for osc1 and 2 to one of `SYNTH_DUTY_*`. Ignored for osc3 and 4.
 Changes immediately, which could cause transient spikes or phasing issues.
 */
void synth_setDutyCycle(uint8_t oscid, uint8_t dutyCycle);

void synth_setNote(uint8_t oscid, uint8_t note);

void synth_triggerNote(uint8_t oscid);

void synth_handleMidiEvent(MidiEvent*);

// TODO: getter and setter methods for synth params

void synth_stop(void);

#ifdef __cplusplus
}
#endif

#endif // _SYNTH_H_
