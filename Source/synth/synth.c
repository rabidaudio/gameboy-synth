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
const uint16_t MIDI_NOTE_NUM_TO_PERIOD[] = {
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

// TODO: midi note to NR43 (shift,width,div)

void apu_writeMaskedRegister(uint16_t addr, uint8_t val, uint8_t mask) {
    uint8_t current = apu_readRegister(addr);
    val = (current & ~mask) | (val & mask); // TODO: is this right?
    apu_writeRegister(addr, val);
}


void synth_loadDefaults(void) {
    // initialize default settings
    for (uint8_t i = 0; i < SYNTH_NUM_OSCS; i++) {
        GLOBAL_SYNTH.confs[i].volume = 0xFF; // full volume
        synth_setPan(i, SYNTH_PAN_BOTH);
        GLOBAL_SYNTH.confs[i].applyVelocity = false;
        GLOBAL_SYNTH.confs[i].transpose = 0;
        GLOBAL_SYNTH.confs[i].length = SYNTH_LENGTH_HOLD; // disable length
    }
    synth_setDutyCycle(SYNTH_OSC1, SYNTH_DUTY_50);
    synth_setDutyCycle(SYNTH_OSC2, SYNTH_DUTY_50);
//    memcpy(synth->osc3_wavetable, WAVE_TABLE_SQUARE, OSC3_WAV_RAM_SIZE);

    for (uint8_t c = 0; c < MIDI_NUM_CHANNELS; c++) {
        GLOBAL_SYNTH.channelStates[c] = 0;
    }
    // TODO: useful for testing, probably not good defaults
    GLOBAL_SYNTH.channelStates[0] = (1 << SYNTH_OSC1) | (1 << SYNTH_OSC2);
    GLOBAL_SYNTH.confs[SYNTH_OSC2].transpose = 7; // 5th above
    GLOBAL_SYNTH.confs[SYNTH_OSC1].length = 52;
    GLOBAL_SYNTH.confs[SYNTH_OSC2].length = 26;
}

void synth_init(void) {
    // disable period sweep on osc 1
    apu_writeRegister(GB_NR10, 0);
    // NR52: Audio master control
    // [7] Audio on/off     [6:4] __ [3r] CH4 on? [2r] CH3 on? [1r] CH2 on? [0r] CH1 on?
    apu_writeRegister(GB_NR52, 0x8F); // audio on
    //  NR50: Master volume & VIN panning
    // [7] VIN Left [6:4] Left Volume [3] VIN Right [2:0] Right Volume
    apu_writeRegister(GB_NR50, 0x77); // full volume L+R, VIN disabled
    synth_loadDefaults();
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

void synth_setVolume(uint8_t oscid, uint8_t volume) {
    GLOBAL_SYNTH.confs[oscid].volume = volume;
    // NOTE: no register changes, takes effect on next note
}

void synth_setPan(uint8_t oscid, uint8_t pan) {
    GLOBAL_SYNTH.confs[oscid].pan = pan;
    apu_writeMaskedRegister(GB_NR51, pan << oscid, SYNTH_PAN_BOTH << oscid);
}

void synth_setLength(uint8_t oscid, uint8_t length) {
    if (oscid != SYNTH_OSC3 && length >= 64) {
        length = 63; // clamp to max value
    }
    GLOBAL_SYNTH.confs[oscid].length = length;
    // NOTE: no register changes, takes effect on next note
}

bool synth_holdMode(uint8_t oscid) {
    uint8_t len = GLOBAL_SYNTH.confs[oscid].length;
    if (oscid == SYNTH_OSC3) return len == 0;
    return (len & 0x1F) == 0;
}

void synth_setDutyCycle(uint8_t oscid, uint8_t dutyCycle) {
    if (oscid > SYNTH_OSC2) return;
    dutyCycle &= 0x03; // ensure 2 bits
    uint8_t len = (dutyCycle << 6) | (GLOBAL_SYNTH.confs[oscid].length & 0x1F);
    GLOBAL_SYNTH.confs[oscid].length = len;
    apu_writeRegister(NRx1(oscid), len);
}

void synth_setNote(uint8_t oscid, uint8_t note) {
    GLOBAL_SYNTH.states[oscid].note = note;
    // NOTE: doesn't take effect until triggered
}

void synth_triggerNote(uint8_t oscid) {
    OscState* state = &GLOBAL_SYNTH.states[oscid];
    OscConfig* conf = &GLOBAL_SYNTH.confs[oscid];
    uint8_t trueNote =state->note + conf->transpose;
    if (oscid == SYNTH_OSC4) {
        // TODO
    } else {
        if (trueNote >= MIDI_NOTE_LOW && trueNote <= 127) {
            uint16_t period = MIDI_NOTE_NUM_TO_PERIOD[trueNote - MIDI_NOTE_LOW];
            period += state->periodOffset;
            // volume
            uint8_t volume = conf->volume;
            if (conf->applyVelocity) {
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
            // configure length
            uint8_t lenEnable = synth_holdMode(oscid) ? 0 : (1 << 6);
            if (lenEnable) {
                apu_writeRegister(NRx1(oscid), conf->length);
            }
            if (oscid == SYNTH_OSC3) {
                // TODO: apply 2 bit velocity
//                if (volume == 0) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_OFF;
//                else if (volume == 1) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_25;
//                else if (volume == 2) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_50;
//                else synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_FULL;
//                synth->writeRegister(GB_NR32, synth->confs[SYNTH_OSC3].volume << 5);
            } else {
                // TODO: compute envelope
                uint8_t envelope = 0x00;
                apu_writeRegister(NRx2(oscid), (volume & 0xF0) | envelope);
            }
            // set period low
            apu_writeRegister(NRx3(oscid), (uint8_t) (period & 0xFF));
            // set period high and trigger
            uint8_t periodUpper = (uint8_t)(period >> 8) & 0x07;
            apu_writeRegister(NRx4(oscid), periodUpper|lenEnable|(1<<7) /* trigger */);
        }
    }
}

void synth_stopNote(uint8_t oscid) {
    apu_writeRegister(NRx2(oscid), 0x00);
}

void synth_handleMidiEvent(MidiEvent* e) {
    uint8_t channel = e->type & 0x0F;
    uint8_t channelState = GLOBAL_SYNTH.channelStates[channel];

    if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
        // rather than banks, this controls which oscillators are enabled
        // for this channel
        if (e->controller == MIDI_CONTROLLER_BANK_SELECT) {
            channelState = (channelState & 0xF0) | (e->controllerEventValue | 0x0F);
        } else if (e->controller == MIDI_CONTROLLER_ALL_SOUND_OFF) {
            // disable all channels
            apu_writeRegister(GB_NR52, 0x00);
            // stop all notes
            for (uint8_t i = 0; i < SYNTH_NUM_OSCS; i++) {
                synth_stopNote(i);
            }
            // re-enable channels
            apu_writeRegister(GB_NR52, 0x8F);
        } else if (e->controller == MIDI_CONTROLLER_ALL_NOTES_OFF) {
            if (channelState & SYNTH_CHANNEL_STATE_OMNIMODE) {
                // should be ignored in omni mode
                return;
            }
            // otherwise, channel-specific code will turn off
        } else if (e->controller == MIDI_CONTROLLER_OMNI_MODE_ON) {
            channelState |= SYNTH_CHANNEL_STATE_OMNIMODE;
        } else if (e->controller == MIDI_CONTROLLER_OMNI_MODE_OFF) {
            channelState &= ~SYNTH_CHANNEL_STATE_OMNIMODE;
        } else if (e->controller == MIDI_CONTROLLER_MONO_MODE) {
            channelState |= SYNTH_CHANNEL_STATE_POLYMODE;
        } else if (e->controller == MIDI_CONTROLLER_POLY_MODE) {
            channelState &= ~SYNTH_CHANNEL_STATE_POLYMODE;
        }
        // update channel state
        GLOBAL_SYNTH.channelStates[channel] = channelState;
    } else if (e-> type == MIDI_EVENT_PROGRAM_CHANGE) {
        // TODO: switch preset
    }
    
    for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
        if ((channelState & (1 << oscid)) == 0) {
            continue; // osc not enabled for this channel
        }
        
        // TODO: handle polyphony
        // TODO: handle voice memory
        OscState* state = &GLOBAL_SYNTH.states[oscid];
        OscConfig* conf = &GLOBAL_SYNTH.confs[oscid];
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
                if (synth_holdMode(oscid)) synth_stopNote(oscid);
            }
        } else if (e->type == MIDI_EVENT_NOTE_AFTERTOUCH) {
            if (e->note == state->note) {
                state->velocity = e->velocity;
                // TODO: if velocity volume enabled, update volume
                if (conf->applyVelocity) {
                    // retrigger with updated volume
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
                // if we got here, we aren't in omni mode
                state->velocity = 0;
                if (synth_holdMode(oscid)) synth_stopNote(oscid);
            }
        }
    }
}

void synth_stop() {
    apu_writeRegister(GB_NR52, 0x00); // audio off
}
