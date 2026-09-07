#ifndef _MIDI_H_
#define _MIDI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MIDI_NUM_CHANNELS 16

// https://www.mixagesoftware.com/en/midikit/help/HTML/midi_events.html
// https://midi.org/summary-of-midi-1-0-messages
// http://midi.teragonaudio.com/tech/midispec.htm
#define MIDI_EVENT_NOTE_ON 0x90
#define MIDI_EVENT_NOTE_OFF 0x80
#define MIDI_EVENT_NOTE_AFTERTOUCH 0xA0
#define MIDI_EVENT_CONTROLLER_EVENT 0xB0
// SELECT preset
#define MIDI_EVENT_PROGRAM_CHANGE 0xC0
// channel aftertouch?
#define MIDI_EVENT_PITCHBEND 0xE0

// used to enable/disable oscillators for this channel
// bit flag [3:0]
#define MIDI_CONTROLLER_BANK_SELECT 0x00
#define MIDI_CONTROLLER_VOLUME 0x07
#define MIDI_CONTROLLER_PAN 0x0A

// #define MIDI_CONTROLLER_ATTACK 0x49
// #define MIDI_CONTROLLER_RELEASE 0x48
#define MIDI_CONTROLLER_ALL_SOUND_OFF 0x78
// #define MIDI_CONTROLLER_RESET 0x79
#define MIDI_CONTROLLER_ALL_NOTES_OFF 0x7B
// #define MIDI_CONTROLLER_OMNI_MODE_ON 0x7D
// #define MIDI_CONTROLLER_OMNI_MODE_OFF 0x7C
#define MIDI_CONTROLLER_MONO_MODE 0x7E
#define MIDI_CONTROLLER_POLY_MODE 0x7F

// also expose LFOs, pitch offset, duty, etc

// timing? clock can't be synchronized, but LFO timers could

// MIDI event, 3 bytes
// TODO: create union from uint8_t[3]
typedef struct {
    // [7:4] type [3:0] channel
    uint8_t type;
    union {
        uint8_t args[2];
        // note on/off, aftertouch
        struct {
            uint8_t note;
            uint8_t velocity;
        };
        // controller event
        struct {
            uint8_t controller;
            uint8_t controllerEventValue;
        };
        // program change
        struct {
            uint8_t programNumber;
        };
        // pitch bend
        struct {
            uint16_t amount;
        };
    };
} MidiEvent;

#ifdef __cplusplus
}
#endif

#endif // _MIDI_H_
