// Copyright 2021 Charles Julian Knight
// https://github.com/rabidaudio/midi-voicesteal
#ifndef LIB_MIDIMANAGER_MIDIMANAGER_H_
#define LIB_MIDIMANAGER_MIDIMANAGER_H_

#include "./types.h"
#include "./staticlinkedlist.h"

struct MidiEvent {
  uint8_t note;
  uint8_t velocity;
};

// A MidiManager tracks the state of pressed keys through
// MIDI messages, and assigns keys to a given number of voices.
// New keys will "steal" voices from earlier keys if no more
// voices are available, but pressed keys are remembered such
// that releasing a key assigned to a voice will assign that
// voice to the most-recently-pressed waiting key. It tracks
// changes in velocity for different keys.
// VSize is the max number of voices supported, while MSize is
// the maximum number of pressed keys to track. Reducing MSize
// will reduce the memory footprint, but if MSize is exceeded
// then the oldest pressed keys will be forgotten.
// The default number of voices is VSize but this can actually
// be reduced at runtime.
// Voices are assigned least-recently-used first.

template <size_t MSize, size_t VSize> class MidiManager {
 private:
  typedef size_t Voice;
  MidiEvent voices_[VSize];
  size_t supportedVoices_;
  StaticLinkedList<Voice, VSize> unassigned_voices_;
  StaticLinkedList<Voice, VSize> assigned_voices_;
  StaticLinkedList<MidiEvent, MSize> pending_notes_;

 public:
  MidiManager() {
    supportedVoices_ = VSize;
    reset();
  }

  void handle(uint8_t note, uint8_t velocity) {
    MidiEvent e;
    e.note = note & 0x7F;
    e.velocity = velocity & 0x7F;
    if (e.velocity > 0) {
      // update the velocity if it's already playing
      Iterator<Voice> i = assigned_voices_.iterator();
      while (i.hasNext()) {
        Voice& v = i.next();
        if (voices_[v].note == e.note) {
          voices_[v].velocity = e.velocity;
          return;
        }
      }
      // update the velocity if it's already tracked
      Iterator<MidiEvent> ni = pending_notes_.iterator();
      while (ni.hasNext()) {
        MidiEvent& n = ni.next();
        if (n.note == e.note) {
          n.velocity = e.velocity;
          return;
        }
      }
      // must be a new note
      Voice v;
      if (unassigned_voices_.isEmpty()) {
        v = assigned_voices_.pop();  // steal oldest voice
        // save that voice's current note for later
        pending_notes_.pushStack(voices_[v]);
      } else {
        v = unassigned_voices_.pop();
      }
      assigned_voices_.pushQueue(v);
      voices_[v] = e;
      return;
    }
    // else note off

    // if it's a queued note simply forget about it
    Iterator<MidiEvent> ni = pending_notes_.iterator();
    size_t i = 0;
    while (ni.hasNext()) {
      MidiEvent& n = ni.next();
      if (n.note == e.note) {
        n.velocity = 0;
        pending_notes_.removeAt(i);
        return;
      }
      i++;
    }

    Iterator<Voice> vi = assigned_voices_.iterator();
    i = 0;
    while (vi.hasNext()) {
      Voice v = vi.next();
      if (voices_[v].note != e.note) {
        i++;
        continue;
      }
      // pressed note is now off
      assigned_voices_.removeAt(i);
      if (pending_notes_.isEmpty()) {
        voices_[v].velocity = 0;
        unassigned_voices_.pushQueue(v);
      } else {
        MidiEvent pending = pending_notes_.pop();
        voices_[v] = pending;
        assigned_voices_.pushQueue(v);
      }
      return;
    }
    // otherwise we got a note off for a note we weren't tracking.
    // this can happen if notes were pressed before we initialized,
    // or if we overran the max size of pending notes
  }

  void reset() {
    pending_notes_.clear();
    assigned_voices_.clear();
    unassigned_voices_.clear();
    for (Voice v = 0; v < supportedVoices_; v++) {
      voices_[v].velocity = 0;
      voices_[v].note = 0;
      unassigned_voices_.pushQueue(v);
    }
  }

  // reduce the number of supported voices at runtime. This
  // will reset the manager.
  void setVoices(size_t voices) {
    if (voices >= VSize || supportedVoices_ == voices) {
      return;
    }
    supportedVoices_ = voices;
    reset();
  }

  size_t voices() {
    return supportedVoices_;
  }

  MidiEvent get(Voice voice) {
    return voices_[voice];
  }
};

#endif  // LIB_MIDIMANAGER_MIDIMANAGER_H_
