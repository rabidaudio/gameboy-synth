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

void synth_writeMaskedRegister(Synth* synth, uint16_t addr, uint8_t val, uint8_t mask) {
    uint8_t current = synth->readRegister(addr);
    val = (current & ~mask) | (val & mask); // TODO: is this right?
    synth->writeRegister(addr, val);
}

// set a 4 bit volume
void synth_setVolume(Synth* synth, uint8_t oscid, uint8_t volume) {
    volume &= 0x0F; // ensure 4 bits
    if (oscid == SYNTH_OSC3) {
        volume = volume >> 2;
        if (volume == 0) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_OFF;
        else if (volume == 1) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_25;
        else if (volume == 2) synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_50;
        else synth->confs[SYNTH_OSC3].volume = SYNTH_OSC3_VOLUME_FULL;
        synth->writeRegister(GB_NR32, synth->confs[SYNTH_OSC3].volume << 5);
    } else {
        synth->confs[oscid].volume = volume;
        // TODO: set now or as part of envelope?
    }
}

void synth_setPan(Synth* synth, uint8_t oscid, uint8_t pan) {
    synth->confs[oscid].pan = pan;
    synth_writeMaskedRegister(synth, GB_NR51, pan << oscid, SYNTH_PAN_BOTH << oscid);
}

void synth_setNote(Synth* s, uint8_t oscid, uint8_t note) {
    s->states[oscid].note = note;
    // period is 11 bits for osc1,2,3
    if (oscid == SYNTH_OSC4) {
        // TODO
    } else {
        uint8_t trueNote = note + s->confs[oscid].transpose;
        uint16_t period = MIDI_NOTE_NUM_TO_PERIOD[trueNote];
        period += s->states[oscid].periodOffset;
        // period low
        s->writeRegister(NRX3(oscid), (uint8_t)(period & 0xFF));
        // period high and trigger
        uint8_t lenEnable = s->confs[oscid].length == 0 ? 0 : (1 << 6);
        s->writeRegister(NRX4(oscid), (uint8_t)(period > 8) | lenEnable | (1 << 7) /*trigger*/);
    }
}

void synth_init(Synth* synth) {
    // initialize default settings
    for (uint8_t i = 0; i < SYNTH_NUM_OSCS; i++) {
        synth_setVolume(synth, i, SYNTH_FULL_VOLUME);
        synth_setPan(synth, i, SYNTH_PAN_BOTH);
        synth->confs[i].applyVelocity = false;
        synth->confs[i].transpose = 0;
        
        if (i == SYNTH_OSC1 || i == SYNTH_OSC2) {
            synth->confs[i].osc12.duty = SYNTH_DUTY_50;
        }
    }
//    memcpy(synth->osc3_wavetable, WAVE_TABLE_SQUARE, OSC3_WAV_RAM_SIZE);

    for (uint8_t c = 0; c < MIDI_NUM_CHANNELS; c++) {
        synth->channelStates[c] = 0;
    }

    // NR52: Audio master control
    // [7] Audio on/off	 [6:4] __ [3r] CH4 on? [2r] CH3 on? [1r] CH2 on? [0r] CH1 on?
    synth->writeRegister(GB_NR52, (1 << 7)); // audio on
    //  NR50: Master volume & VIN panning
    // [7] VIN Left [6:4] Left Volume [3] VIN Right [2:0] Right Volume
    synth->writeRegister(GB_NR50, 0b01110111); // full volume L+R, VIN disabled
}

void synth_triggerNote(Synth* s, uint8_t oscid) {
    // TODO: handle envelopes
    uint8_t trueNote = s->states[oscid].note + s->confs[oscid].transpose;
    if (oscid == SYNTH_OSC4) {
        // TODO
    } else {
        if (trueNote >= MIDI_NOTE_LOW && trueNote < 127) {
            uint8_t period = MIDI_NOTE_NUM_TO_PERIOD[trueNote - MIDI_NOTE_LOW];
            period += s->states[oscid].periodOffset;
            // set period low
            s->writeRegister(NRX3(oscid), (uint8_t) (period & 0xFF));
            // set period high and trigger
            uint8_t lenEnable = s->confs[oscid].length == 0 ? 0 : (1 << 6);
            uint8_t periodUpper = (uint8_t)(period >> 8) & 0x07;
            s->writeRegister(NRX4(oscid), periodUpper|lenEnable|(1<<7) /* trigger */);
        }
    }
}

void synth_handleMidiEvent(Synth* s, MidiEvent* e) {
    uint8_t channel = midi_getChannel(e);
    uint8_t channelState = s->channelStates[channel];

    if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
        // rather than banks, this controls which oscillators are enabled
        // for this channel
        if (e->controller == MIDI_CONTROLLER_BANK_SELECT) {
            channelState = (channelState & 0xF0) | (e->controllerEventValue | 0x0F);
        } else if (e->controller == MIDI_CONTROLLER_ALL_SOUND_OFF) {
            // TODO: disable all oscillators immediately
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
        s->channelStates[channel] = channelState;
    } else if (e-> type == MIDI_EVENT_PROGRAM_CHANGE) {
        // TODO: switch preset
    }
    
    for (uint8_t oscid = 0; oscid < SYNTH_NUM_OSCS; oscid++) {
        // TODO: handle polyphony
        // TODO: handle voice memory
        OscState* state = &s->states[oscid];
        OscConfig* conf = &s->confs[oscid];
        if (e->type == MIDI_EVENT_NOTE_ON) {
            // immediately change notes
            state->note = e->note;
            state->velocity = e->velocity;
            synth_triggerNote(s, oscid);
        } else if (e->type == MIDI_EVENT_NOTE_OFF) {
            // turn off only if note matches
            if (e->note == state->note) {
                state->velocity = 0; // ignore velocity value
            }
            // TODO: if len==0 stop note
        } else if (e->type == MIDI_EVENT_NOTE_AFTERTOUCH) {
            if (e->note == state->note) {
                state->velocity = e->velocity;
                // TODO: if velocity volume enabled, update volume
            }
        } else if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
            if (e->controller == MIDI_CONTROLLER_VOLUME) {
                synth_setVolume(s, oscid, e->controllerEventValue >> 3); // 7 bits to 4 bits
            } else if (e->controller == MIDI_CONTROLLER_PAN) {
                // only support hard-pan settings
                if (e->controllerEventValue == 0x00) {
                    synth_setPan(s, oscid, SYNTH_PAN_L);
                } else if (e->controllerEventValue == 0x3F) {
                    synth_setPan(s, oscid, SYNTH_PAN_R);
                } else {
                    synth_setPan(s, oscid, SYNTH_PAN_BOTH);
                }
            } else if (e->controller == MIDI_CONTROLLER_ALL_NOTES_OFF) {
                // if we got here, we aren't in omni mode
                state->velocity = 0;
                // TODO: if len==0 stop note (define synth_updateVelocity)
            }
        }
    }
}

void synth_stop(Synth* synth) {
    synth->writeRegister(GB_NR52, 0x00); // audio off
}
