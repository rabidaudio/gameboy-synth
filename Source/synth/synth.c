#include "synth.h"

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

void init_common(Osc* osc) {
    osc->noteOffset = 0;
    osc->volume = 0;
    osc->pan = SYNTH_PAN_BOTH; // both
}

void init_osc12(Osc12* osc) {
    init_common(&osc->common);
    osc->duty = SYNTH_DUTY_50;
    osc->envelope = 0;
}

void synth_init(Synth* synth) {
    init_osc12(&synth->osc1);
    init_osc12(&synth->osc2);
    // TODO: init 3 and 4

    for (uint8_t c = 0; c < MIDI_NUM_CHANNELS; c++) {
        synth->channelStates[c] = 0;
    }

    // NR52: Audio master control
    // [7] Audio on/off	 [6:4] __ [3r] CH4 on? [2r] CH3 on? [1r] CH2 on? [0r] CH1 on?
    synth->setRegister(NR52_REG, (uint8_t)(1 << 7)); // audio on
    //  NR50: Master volume & VIN panning
    // [7] VIN Left [6:4] Left Volume [3] VIN Right [2:0] Right Volume
    synth->setRegister(NR50_REG, 0b01110111); // full volume L+R, VIN disabled
    // FF25 — NR51: Sound panning
    //  CH4L CH3L CH2L CH1L CH4R CH3R CH2R CH1R
    synth->setRegister(NR51_REG, (synth->osc1.common.pan << 0) | (synth->osc2.common.pan << 1));

    // NR10: Channel 1 sweep
    // NR11: Channel 1 length timer & duty cycle
    synth->setRegister(NR11_REG, (synth->osc1.duty << 5) | 50); // set duty cycle, L=50
    // NR12: Channel 1 volume & envelope
    synth->setRegister(NR12_REG, 0b11110000); // full volume, no envelope

    // NR13: Channel 1 period low [write-only]
}

void synth_stop(Synth* synth) {
    synth->setRegister(NR52_REG, 0x00); // audio off
}

// TODO: separate preset-related memory from state machine memory
// then memcpy presets into synth, reseting state machines

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

    if (synth_isChannelEnabled(channelState, SYNTH_OSC1)) {
        synth_commonHandleMidiEvent(&s->osc1.common, e);
    }
    if (synth_isChannelEnabled(channelState, SYNTH_OSC2)) {
        synth_osc12HandleMidiEvent(&s->osc2, e);
    }
    // TODO: 3 and 4
}

void synth_commonHandleMidiEvent(Osc* o, MidiEvent* e) {
    // TODO: handle polyphony
    // TODO: handle voice memory
    if (e->type == MIDI_EVENT_NOTE_ON) {
        // immediately change notes
        o->state.note = e->note;
        o->state.velocity = e->velocity;
    } else if (e->type == MIDI_EVENT_NOTE_OFF) {
        if (e->note == o->state.note) {
            o->state.velocity = 0; // ignore velocity value
        }
    } else if (e->type == MIDI_EVENT_NOTE_AFTERTOUCH) {
        if (e->note == o->state.note) {
            o->state.velocity = e->velocity;
        }
    } else if (e->type == MIDI_EVENT_CONTROLLER_EVENT) {
        if (e->controller == MIDI_CONTROLLER_VOLUME) {
            o->volume = e->controllerEventValue >> 3; // 7 bits to 4 bits
        } else if (e->controller == MIDI_CONTROLLER_PAN) {
            // only support hard-pan settings
            if (e->controllerEventValue == 0x00) {
                o->pan = SYNTH_PAN_L;
            } else if (e->controllerEventValue == 0x3F) {
                o->pan = SYNTH_PAN_R;
            } else {
                o->pan = SYNTH_PAN_BOTH;
            }
        } else if (e->controller == MIDI_CONTROLLER_ALL_NOTES_OFF) {
            // if we got here, we aren't in omni mode
            o->state.velocity = e->velocity;
        }
    }
    // TODO set registers immediately or on next audio tick?
}
