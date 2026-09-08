#include "synth.h"

// unlike the square and noise waves with velocity of 4 bits,
// the wave has 2 bits: 00=0%, 01=100%, 10=50%, 11=25%
#define SYNTH_OSC3_VOLUME_OFF 0x00
#define SYNTH_OSC3_VOLUME_FULL 0x01
#define SYNTH_OSC3_VOLUME_50 0x02
#define SYNTH_OSC3_VOLUME_25 0x03

// OSC 1+2 only
// MIDI: 36 to 127 (C2 to G9)
#define MIDI_NOTE_LOW 36
// generate via ruby:
/*
# midi notes 0-127. midi note 0 = C-1. A4 is note 69 (nice)
note_names = (0..127).map { |n| %w(C C# D D# E F F# G G# A A# B)[n % 12] + ((n / 12)-1).to_s }
# For A4, n = 0 -> 440 * 2^(0/12) = 440 Hz.
# n = (octave * 12) + semitones[key] - ((4 * 12) + semitones['A'])
# 440.0 * (2.0 ** (n / 12.0))
note_pitches = (0..127).map { |n| 440.0 * 2 ** ((n-69)/12.0) }
# Hz = 131072/(2048-period)
# period = (-131072.0 / f)+2048
# where period is a signed 11bit integer (0 to 2042)
# all_frequencies = (0..2047).map { |p| 131072.0/(2048-p)}
# hz = 440*2^(n-12) -> hz/440=2^(n-12) -> log2(hz/440)-12=n
# because the lowest we can go is 64Hz, we'll start the lookup table at C2 (36)
start = 36
note_names = note_names[start..]
note_pitches = note_pitches[start..]
periods = note_pitches.map { |f| ((-131072.0 / f)+2048).round }
puts periods.each_with_index.map { |p, i| "#{p}," }.each_slice(12).map { |s| s.join(" ") }.join("\n")
*/
static const uint16_t MIDI_NOTE_NUM_TO_PERIOD[] = {
    44, /* 36,C2 */ 157, /* 37,C#2 */ 263, /* 38,D2 */ 363, /* 39,D#2 */ 457, /* 40,E2 */ 547, /* 41,F2 */ 631, /* 42,F#2 */ 711, /* 43,G2 */ 786, /* 44,G#2 */ 856, /* 45,A2 */ 923, /* 46,A#2 */ 986, /* 47,B2 */
    1046, /* 48,C3 */ 1102, /* 49,C#3 */ 1155, /* 50,D3 */ 1205, /* 51,D#3 */ 1253, /* 52,E3 */ 1297, /* 53,F3 */ 1339, /* 54,F#3 */ 1379, /* 55,G3 */ 1417, /* 56,G#3 */ 1452, /* 57,A3 */ 1486, /* 58,A#3 */ 1517, /* 59,B3 */
    1547, /* 60,C4 */ 1575, /* 61,C#4 */ 1602, /* 62,D4 */ 1627, /* 63,D#4 */ 1650, /* 64,E4 */ 1673, /* 65,F4 */ 1694, /* 66,F#4 */ 1714, /* 67,G4 */ 1732, /* 68,G#4 */ 1750, /* 69,A4 */ 1767, /* 70,A#4 */ 1783, /* 71,B4 */
    1798, /* 72,C5 */ 1812, /* 73,C#5 */ 1825, /* 74,D5 */ 1837, /* 75,D#5 */ 1849, /* 76,E5 */ 1860, /* 77,F5 */ 1871, /* 78,F#5 */ 1881, /* 79,G5 */ 1890, /* 80,G#5 */ 1899, /* 81,A5 */ 1907, /* 82,A#5 */ 1915, /* 83,B5 */
    1923, /* 84,C6 */ 1930, /* 85,C#6 */ 1936, /* 86,D6 */ 1943, /* 87,D#6 */ 1949, /* 88,E6 */ 1954, /* 89,F6 */ 1959, /* 90,F#6 */ 1964, /* 91,G6 */ 1969, /* 92,G#6 */ 1974, /* 93,A6 */ 1978, /* 94,A#6 */ 1982, /* 95,B6 */
    1985, /* 96,C7 */ 1989, /* 97,C#7 */ 1992, /* 98,D7 */ 1995, /* 99,D#7 */ 1998, /* 100,E7 */ 2001, /* 101,F7 */ 2004, /* 102,F#7 */ 2006, /* 103,G7 */ 2009, /* 104,G#7 */ 2011, /* 105,A7 */ 2013, /* 106,A#7 */ 2015, /* 107,B7 */
    2017, /* 108,C8 */ 2018, /* 109,C#8 */ 2020, /* 110,D8 */ 2022, /* 111,D#8 */ 2023, /* 112,E8 */ 2025, /* 113,F8 */ 2026, /* 114,F#8 */ 2027, /* 115,G8 */ 2028, /* 116,G#8 */ 2029, /* 117,A8 */ 2030, /* 118,A#8 */ 2031, /* 119,B8 */
    2032, /* 120,C9 */ 2033, /* 121,C#9 */ 2034, /* 122,D9 */ 2035, /* 123,D#9 */ 2036, /* 124,E9 */ 2036, /* 125,F9 */ 2037, /* 126,F#9 */ 2038, /* 127,G9 */
};

// OSC 3 only

// generated with ruby:
// puts (0...32).map { |i| i < 16 ? 15 : 0 }.map(&:round).each_slice(2).map { |(a, b)| "0x#{a.to_s(16)}#{b.to_s(16)}" }.join(", ")
static const uint8_t WAVE_TABLE_SQUARE[OSC3_WAV_RAM_SIZE] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// generated with ruby:
// puts (0...32).map { |i| Math.sin(2*Math::PI*i/32.0) }.map { |v| (v+1)*7.5 }.map(&:round).each_slice(2).map { |(a, b)| "0x#{a.to_s(16)}#{b.to_s(16)}" }.join(", ")
static const uint8_t WAVE_TABLE_SINE[OSC3_WAV_RAM_SIZE] = {
    0x89, 0xac, 0xde, 0xef, 0xff, 0xee, 0xdc, 0xa9, 0x86, 0x53, 0x21, 0x10, 0x00, 0x11, 0x23, 0x56
};


// generated with ruby:
// puts (0...32).map { |v| v / 2 }.each_slice(2).map { |(a, b)| "0x#{a.to_s(16)}#{b.to_s(16)}" }.join(", ")
static const uint8_t WAVE_TABLE_SAW[OSC3_WAV_RAM_SIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

// generated with ruby:
// puts (0...32).map { |v| (v < 16 ? v : 31-v) % 16 }.each_slice(2).map { |(a, b)| "0x#{a.to_s(16)}#{b.to_s(16)}" }.join(", ")
static const uint8_t WAVE_TABLE_TRIANGLE[OSC3_WAV_RAM_SIZE] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

// generated with ruby:
// puts (0...32).map { |v| rand * 15 }.map(&:round).each_slice(2).map { |(a, b)| "0x#{a.to_s(16)}#{b.to_s(16)}" }.join(", ")
static const uint8_t WAVE_TABLE_NOISE[OSC3_WAV_RAM_SIZE] = {
    0x0c, 0x8b, 0xed, 0x9e, 0xa5, 0xe6, 0x90, 0x48, 0x65, 0xb4, 0x31, 0x82, 0xe7, 0x80, 0x64, 0x02
};

// OSC4

// generated with ruby:
/*
reg = (0..255)
clock_shift = reg.map { |r| r >> 4 }
width = reg.map { |r| (r >> 3) & 0x01 }
clock_div = reg.map { |r| r & 0x07 }.map { |c| c == 0 ? 0.5 : c }
# =262144/(IF(D2=0,0.5,D2)*POW(2,B2))
freq = reg.map { |r| clock_shift[r] >= 14 ? 0 : 262144.0 / (clock_div[r] * (2 ** clock_shift[r])) }
freq_map = reg.zip(freq).to_h
all_freqs = freq.uniq.sort
# freq.uniq.count = 61
seq_regs = all_freqs.map { |f| freq_map.find { |k, v| (k & 8) == 0 && v == f} }.map(&:first)
puts [0, (1 << 3)].map { |width| seq_regs.map { |r| r | width } }.flatten.map { |v| "#{v}, " }.each_slice(10).map(&:join).join("\n")
*/
static const uint16_t MIDI_NOTE_NUM_TO_OSC4_LFSR[128] = {
    224, 215, 214, 213, 212, 199, 198, 197, 196, 183, 
    182, 181, 180, 167, 166, 165, 164, 151, 150, 149, 
    148, 135, 134, 133, 132, 119, 118, 117, 116, 103, 
    102, 101, 100, 87, 86, 85, 84, 71, 70, 69, 
    68, 55, 54, 53, 52, 39, 38, 37, 36, 23, 
    22, 21, 20, 7, 6, 5, 4, 3, 2, 1, 
    0, 232, 223, 222, 221, 220, 207, 206, 205, 204, 
    191, 190, 189, 188, 175, 174, 173, 172, 159, 158, 
    157, 156, 143, 142, 141, 140, 127, 126, 125, 124, 
    111, 110, 109, 108, 95, 94, 93, 92, 79, 78, 
    77, 76, 63, 62, 61, 60, 47, 46, 45, 44, 
    31, 30, 29, 28, 15, 14, 13, 12, 11, 10, 9, 8, 
};

// TARGET_INLINE void apu_writeMaskedRegister(uint16_t addr, uint8_t val, uint8_t mask) {
//     uint8_t current = apu_readRegister(addr);
//     val = (current & ~mask) | (val & mask); // TODO: is this right?
//     apu_writeRegister(addr, val);
// }

void synth_loadDefaults(void) {
    // initialize default settings
    OscConfig* conf = GLOBAL_SYNTH.confs;
    for (uint8_t i = 0; i < SYNTH_NUM_OSCS; i++) {
        conf->volume = 0xFF; // full volume
        conf->channelState = 0; // enabled=off, poly=off, len=off, velocityMode=off
        conf->transpose = 0;
        conf->length = 0;
        conf++;
    }
    GLOBAL_SYNTH.pan = 0xFF; // set pan center for all osc
    apu_writeRegister(GB_NR51, 0xFF);

    synth_setDutyCycle(SYNTH_OSC1, SYNTH_DUTY_50);
    synth_setDutyCycle(SYNTH_OSC2, SYNTH_DUTY_50);
//    memcpy(synth->osc3_wavetable, WAVE_TABLE_SQUARE, OSC3_WAV_RAM_SIZE);

    GLOBAL_SYNTH.masterVolume = 0x77; // full volume
    apu_writeRegister(GB_NR50, GLOBAL_SYNTH.masterVolume);

    // turn on osc1 for MIDI channel 1
    GLOBAL_SYNTH.confs[SYNTH_OSC1].channelState |= SYNTH_CHANNEL_STATE_ENABLED;
}

void synth_init(void) {
    // disable period sweep on osc 1
    apu_writeRegister(GB_NR10, 0);
    // NR52: Audio master control
    // [7] Audio on/off     [6:4] __ [3r] CH4 on? [2r] CH3 on? [1r] CH2 on? [0r] CH1 on?
    apu_writeRegister(GB_NR52, 0x8F); // audio on
    //  NR50: Master volume & VIN panning
    // [7] VIN Left [6:4] Left Volume [3] VIN Right [2:0] Right Volume
    synth_loadDefaults();

    // reset states
    for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
        // TODO: memset zero instead?
        GLOBAL_SYNTH.states[oscid].envelope.state = SYNTH_ENV_STATE_OFF;
        GLOBAL_SYNTH.states[oscid].envelope.ticksRem = 0;
        GLOBAL_SYNTH.states[oscid].note = 0;
        GLOBAL_SYNTH.states[oscid].periodOffset = 0;
        GLOBAL_SYNTH.states[oscid].velocity = 0;
    }
}

void synth_loadPreset(uint8_t* data) {
    if (data == 0x00 /* NULL */) {
        synth_loadDefaults();
        return;
    }
//    uint8_t index = 0;
    // TODO: implement after other functionality
}

uint8_t synth_savePreset(uint8_t* data) {
    // TODO: implement after other functionality
    return 0;
}

TARGET_INLINE void synth_setMasterVolume(uint8_t volumeL, uint8_t volumeR) {
    volumeL &= 0x07;
    volumeR &= 0x07;
    GLOBAL_SYNTH.masterVolume = volumeL << 4 | volumeR;
    apu_writeRegister(GB_NR50, GLOBAL_SYNTH.masterVolume);
}

TARGET_INLINE void synth_setChannel(uint8_t oscid, uint8_t channelState) {
    GLOBAL_SYNTH.confs[oscid].channelState = channelState;
}

TARGET_INLINE void synth_setVolume(uint8_t oscid, uint8_t volume) {
    GLOBAL_SYNTH.confs[oscid].volume = volume;
    // NOTE: no register changes, takes effect on next note
}

TARGET_INLINE void synth_setPan(uint8_t oscid, uint8_t pan) {
    pan = (pan << oscid);
    uint8_t mask = (SYNTH_PAN_BOTH << oscid);
    uint8_t gPan = (GLOBAL_SYNTH.pan & ~mask) | (pan & mask);
    GLOBAL_SYNTH.pan = gPan;
    apu_writeRegister(GB_NR51, gPan);
}

TARGET_INLINE void synth_setLength(uint8_t oscid, uint8_t length) {
    if (oscid != SYNTH_OSC3 && length > 0x1F) {
        length = 0x1F; // clamp to max value
    }
    GLOBAL_SYNTH.confs[oscid].length = length;
    // NOTE: no register changes, takes effect on next note
}

TARGET_INLINE void synth_setTranspose(uint8_t oscid, int8_t offset) {
    GLOBAL_SYNTH.confs[oscid].transpose = offset;
    // NOTE: no register changes, takes effect on next note
}

void synth_setDutyCycle(uint8_t oscid, uint8_t dutyCycle) {
    if (oscid > SYNTH_OSC2) return;
    dutyCycle &= 0x03; // ensure 2 bits
    uint8_t len = (dutyCycle << 6) | (GLOBAL_SYNTH.confs[oscid].length & 0x1F);
    GLOBAL_SYNTH.confs[oscid].length = len;
    apu_writeRegister(NRx1(oscid), len);
}

TARGET_INLINE void synth_setEnvelope(uint8_t oscid, uint8_t attackRate, uint8_t releaseRate) {
    uint8_t pace = ((attackRate & 0x07) << 4) | (releaseRate & 0x07);
    GLOBAL_SYNTH.confs[oscid].envelopeSweepPace = pace;
}

TARGET_INLINE void synth_setNote(uint8_t oscid, uint8_t note) {
    GLOBAL_SYNTH.states[oscid].note = note;
    // NOTE: doesn't take effect until triggered
}

void synth_triggerNote(uint8_t oscid) {
    OscState* state = &GLOBAL_SYNTH.states[oscid];
    OscConfig* conf = &GLOBAL_SYNTH.confs[oscid];

    uint8_t trueNote = state->note + conf->transpose;
    // ignore out-of-bounds notes
    if (trueNote > 127) return;
    // OSC4 supports down to note 0 but 1+2 support down to MIDI_NOTE_LOW
    if (oscid != SYNTH_OSC4 && trueNote < MIDI_NOTE_LOW) return;

    // configure length
    uint8_t lenEnable = (conf->channelState & SYNTH_CHANNEL_STATE_FIXEDLEN) ? (1 << 6) : 0;
    if (lenEnable) {
        apu_writeRegister(NRx1(oscid), conf->length);
    }

    // volume/velocity/envelope
    uint8_t volume = conf->volume;
    if (conf->channelState & SYNTH_CHANNEL_STATE_VELOCITY) {
        // TODO: apply velocity
        // multiplication/division is too expensive for the gameboy
        // since we only have 2-4 bits of volume resolution anyway, a precise
        // scaling is not important.
        // instead, we can take the source volume and divide it by 2
        // for the most significant bit of velocity.
        // velocity == 0x7F -> no shift
        // velocity == 0x3F -> shift 1
        // velocity == 0x1F -> shift 2
        // velocity == 0x0F -> shift 3
        // velocity == 0x07 -> shift 4
        // velocity == 0x03 -> shift 5
        // velocity == 0x01 -> shift 6
        // velocity == 0 -> set volume to zero
    }
    if (oscid == SYNTH_OSC3) {
        // osc3 doesn't support envelopes, and only supports 2 bit volumes

        // TODO: apply 2 bit velocity
//                if (volume == 0) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_OFF;
//                else if (volume == 1) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_25;
//                else if (volume == 2) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_50;
//                else synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_FULL;
//                synth->writeRegister(GB_NR32, synth->confs[SYNTH_OSC3].volume << 5);
    } else {
        if (state->envelope.state == SYNTH_ENV_STATE_ATTACK) {
            // attack just finished, so play main note
            apu_writeRegister(NRx2(oscid), (volume & 0xF0));
        } else {
            // this must be the start of a new note. set up the attack envelope
            uint8_t attackRate = (conf->envelopeSweepPace >> 4) & 0x07;
            if (attackRate == 0) {
                // skip attack, just play the note
                state->envelope.state = SYNTH_ENV_STATE_ON;
                state->envelope.ticksRem = lenEnable ? conf->length : 0;
                apu_writeRegister(NRx2(oscid), (volume & 0xF0));
            } else {
                // start attack
                // ticksRem is the number of ticks to reach volume, attackRate*volume
                // multiplying 4bits by 3 bits, worst case loop = 7 cycles
                uint8_t ticksRem = 0;
                for (uint8_t i = 0; i < attackRate; i++) {
                    ticksRem += (volume >> 4);
                }
                state->envelope.state = SYNTH_ENV_STATE_ATTACK;
                state->envelope.ticksRem = ticksRem;
                apu_writeRegister(NRx2(oscid), (0 & 0xF0) /* initial volume=0 */ | SYNTH_ENV_UP | attackRate);
            }
        }
    }

    // pitch and trigger
    uint8_t periodUpper = 0;
    if (oscid == SYNTH_OSC4) {
        uint8_t lfsrSetting = MIDI_NOTE_NUM_TO_OSC4_LFSR[trueNote];
        apu_writeRegister(GB_NR43, lfsrSetting);
    } else { // osc 1-3
        uint16_t period = MIDI_NOTE_NUM_TO_PERIOD[trueNote - MIDI_NOTE_LOW];
        period += state->periodOffset;
        // set period low
        apu_writeRegister(NRx3(oscid), (uint8_t) (period & 0xFF));
        // set period high and trigger
        periodUpper = (uint8_t)(period >> 8) & 0x07;
    }
    // trigger
    apu_writeRegister(NRx4(oscid), periodUpper|lenEnable|(1<<7) /* trigger */);
}

TARGET_INLINE void synth_stopNote(uint8_t oscid) {
    GLOBAL_SYNTH.states[oscid].envelope.state = SYNTH_ENV_STATE_OFF;
    GLOBAL_SYNTH.states[oscid].envelope.ticksRem = 0;
    apu_writeRegister(NRx2(oscid), 0x00);
}

void synth_handleMidiEvent(MidiEvent* e) {
    uint8_t channel = e->type & 0x0F;

    if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
        // rather than banks, this controls which oscillators are enabled
        // for this channel
        if (e->controller == MIDI_CONTROLLER_BANK_SELECT) {
            for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
                uint8_t channelState = GLOBAL_SYNTH.confs[oscid].channelState;
                bool enabled = e->controllerEventValue & (1 << oscid);
                channelState = (channelState & 0x7F) | (enabled ? SYNTH_CHANNEL_STATE_ENABLED : 0);
                GLOBAL_SYNTH.confs[oscid].channelState = channelState;
            }
            return;
        } else if (e->controller == MIDI_CONTROLLER_ALL_SOUND_OFF) {
            // disable all channels
            apu_writeRegister(GB_NR52, 0x00);
            // stop all notes
            for (uint8_t i = 0; i < SYNTH_NUM_OSCS; i++) {
                synth_stopNote(i);
            }
            // re-enable channels
            apu_writeRegister(GB_NR52, 0x8F);
            return;
        }
    } else if (e-> type == MIDI_EVENT_PROGRAM_CHANGE) {
        // TODO: switch preset
        return;
    }
    
    OscState* state = GLOBAL_SYNTH.states;
    OscConfig* conf = GLOBAL_SYNTH.confs;
    for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
        uint8_t channelState = conf->channelState;

        if ((channelState & SYNTH_CHANNEL_STATE_ENABLED) == 0) {
            conf++; state++;
            continue; // osc not enabled for this channel
        }
        
        // TODO: handle polyphony
        // TODO: handle voice memory
        if (e->type == MIDI_EVENT_NOTE_ON) {
            // immediately change notes
            state->note = e->note;
            state->velocity = e->velocity;
            synth_triggerNote(oscid);
        } else if (e->type == MIDI_EVENT_NOTE_OFF) {
            // turn off only if note matches
            if (e->note == state->note) {
                state->velocity = 0; // ignore velocity value
                // if not using length, stop the note
                if ((conf->channelState & SYNTH_CHANNEL_STATE_FIXEDLEN) == 0)
                    synth_stopNote(oscid);
            }
        } else if (e->type == MIDI_EVENT_NOTE_AFTERTOUCH) {
            if (e->note == state->note) {
                state->velocity = e->velocity;
                // TODO: if velocity volume enabled, update volume
                if (channelState & SYNTH_CHANNEL_STATE_VELOCITY) {
                    // retrigger with updated volume
                    // TODO: re-trigger needs to not apply envelope
                    synth_triggerNote(oscid);
                }
            }
        } else if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
            if (e->controller == MIDI_CONTROLLER_VOLUME) {
                conf->volume = e->controllerEventValue << 1; // 7 bits to 8 bits
            } else if (e->controller == MIDI_CONTROLLER_PAN) {
                // only support hard-pan settings
                if (e->controllerEventValue == 0x00) {
                    synth_setPan(oscid, SYNTH_PAN_L);
                } else if (e->controllerEventValue == 0x3F) {
                    synth_setPan(oscid, SYNTH_PAN_R);
                } else {
                    synth_setPan(oscid, SYNTH_PAN_BOTH);
                }
            } else if (e->controller == MIDI_CONTROLLER_ALL_NOTES_OFF) {
                state->velocity = 0;
                if ((conf->channelState & SYNTH_CHANNEL_STATE_FIXEDLEN) == 0)
                    synth_stopNote(oscid);
            } else if (e->controller == MIDI_CONTROLLER_MONO_MODE) {
                GLOBAL_SYNTH.confs[oscid].channelState &= ~SYNTH_CHANNEL_STATE_POLYMODE; // disable poly
            } else if (e->controller == MIDI_CONTROLLER_POLY_MODE) {
                GLOBAL_SYNTH.confs[oscid].channelState |= SYNTH_CHANNEL_STATE_POLYMODE; // enable poly
            }
        }
        conf++;
        state++;
    }
}

void synth_configureTimers() {
    apu_writeRegister(GB_TAC, 0); // slow (4096Hz) clock divider
    apu_writeRegister(GB_TMA, 0xFF-16); // clock divide additional 16 -> 256Hz
    apu_writeRegister(GB_TIMA, 0); // restart clock
    apu_writeRegister(GB_TAC, (1<<3)); // start timer

    // TODO: synchronize timer with audio timer
}

uint8_t timerTick = 0;

void synth_handleTimer(void) {
    timerTick++;
    
    OscConfig* conf = GLOBAL_SYNTH.confs;
    OscState* state = GLOBAL_SYNTH.states;
    uint8_t envState;
    // 256Hz - LFOs, Env state ON

    for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
        if (oscid == SYNTH_OSC3) {
            // osc3 doesn't support envelopes
            goto ENV_EXIT_LOOP;
        }

        envState = state->envelope.state;
        // if ticksRem is 0, envelope is complete or disabled
        if (state->envelope.ticksRem == 0) {
            goto ENV_EXIT_LOOP;
        }
        // if state is off, we're waiting for the next trigger
        if (envState == SYNTH_ENV_STATE_OFF) {
            goto ENV_EXIT_LOOP;
        }
        if (timerTick % 4 != 0 && state->envelope.state != SYNTH_ENV_STATE_ON) {
            // attack/release tick at 64Hz (so it's not time), but len state ticks at 256Hz
            goto ENV_EXIT_LOOP;
        }
        // tick down, and check for state change
        if (--state->envelope.ticksRem == 0) {
            // trigger envelope change
            if (envState == SYNTH_ENV_STATE_ATTACK) {
                // if hold mode, envelope stops
                if ((conf->channelState & SYNTH_CHANNEL_STATE_FIXEDLEN) == 0) {
                    state->envelope.ticksRem = 0; // set to zero to disable counter
                } else {
                    state->envelope.ticksRem = conf->length; // ticksRem is now a count of 256Hz ticks
                }
                // state->envelope.state = SYNTH_ENV_STATE_ON; // triggerNote will set state
                synth_triggerNote(oscid);
            } else if (envState == SYNTH_ENV_STATE_ON) {
                // if we got here, ticksRem was previously non-zero so we must be in FIXDLEN mode
                uint8_t releaseRate = conf->envelopeSweepPace & 0x0F;
                if (releaseRate == 0) {
                    // skip release state
                    synth_stopNote(oscid);
                } else {
                    state->envelope.state = SYNTH_ENV_STATE_RELEASE; // triggerNote will set state
                    uint8_t volume = conf->volume >> 4;
                    uint8_t ticksRem = 0;
                    for (uint8_t i = 0; i < releaseRate; i++) { // volume*releaseRate, worst case 7 cycles
                        ticksRem += volume;
                    }
                    state->envelope.ticksRem = ticksRem;
                    apu_writeRegister(NRx2(oscid), (volume << 4) | SYNTH_ENV_DOWN | releaseRate);
                }
            } else if (envState == SYNTH_ENV_STATE_RELEASE) {
                synth_stopNote(oscid);
            }
        }

        ENV_EXIT_LOOP:
        state++;
        conf++;
    }

    // if (timerTick % 2 == 0) {
    //     // 128Hz - period sweeps (portamento)
    // }
}

TARGET_INLINE void synth_stop(void) {
    apu_writeRegister(GB_NR52, 0x00); // audio off
}
