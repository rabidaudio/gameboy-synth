package midivoice_test

import (
	"testing"

	"github.com/rabidaudio/gameboy-synth/midivoice"
	"gotest.tools/v3/assert"
)

type result struct {
	note     midivoice.Note
	velocity midivoice.Velocity
}

func TestManager(t *testing.T) {
	cases := []struct {
		name          string
		channel       midivoice.Channel
		voices        uint
		events        []midivoice.MidiEvent
		expectedState map[int]result
	}{
		{
			name:    "wrong channel events are ignored",
			channel: 2,
			voices:  1,
			events: []midivoice.MidiEvent{
				{Channel: 1, Note: 45, Velocity: 127},
			},
			expectedState: map[int]result{
				0: {0, 0},
			},
		},
		{
			name:   "one voice one note",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 45, Velocity: 127},
			},
			expectedState: map[int]result{
				0: {45, 127},
			},
		},
		{
			name:   "one voice note release",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 45, Velocity: 127},
				{Note: 45, Velocity: 0},
			},
			expectedState: map[int]result{
				0: {45, 0},
			},
		},
		{
			name:   "one voice steal",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 45, Velocity: 127},
				{Note: 50, Velocity: 127},
			},
			expectedState: map[int]result{
				0: {50, 127},
			},
		},
		{
			name:   "one voice release",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 45, Velocity: 127},
				{Note: 50, Velocity: 127},
				{Note: 50, Velocity: 0},
			},
			expectedState: map[int]result{
				0: {45, 127},
			},
		},
		{
			name:   "update velocity",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 50, Velocity: 127},
				{Note: 50, Velocity: 64},
			},
			expectedState: map[int]result{
				0: {50, 64},
			},
		},
		{
			name:   "queued voice release",
			voices: 1,
			events: []midivoice.MidiEvent{
				{Note: 50, Velocity: 127},
				{Note: 45, Velocity: 127},
				{Note: 50, Velocity: 0},
				{Note: 45, Velocity: 0},
			},
			expectedState: map[int]result{
				0: {45, 0},
			},
		},
		{
			name:   "multiple voices steal",
			voices: 4,
			events: []midivoice.MidiEvent{
				{Note: 50, Velocity: 127},
				{Note: 51, Velocity: 127},
				{Note: 52, Velocity: 127},
				{Note: 53, Velocity: 127},
				{Note: 54, Velocity: 127},
			},
			expectedState: map[int]result{
				0: {54, 127},
				1: {51, 127},
				2: {52, 127},
				3: {53, 127},
			},
		},
		{
			name:   "multiple voices basic",
			voices: 4,
			events: []midivoice.MidiEvent{
				{Note: 50, Velocity: 127},
				{Note: 51, Velocity: 127},
				{Note: 52, Velocity: 127},
				{Note: 53, Velocity: 127},
				{Note: 54, Velocity: 127},
			},
			expectedState: map[int]result{
				0: {54, 127},
				1: {51, 127},
				2: {52, 127},
				3: {53, 127},
			},
		},
		// {
		// 	name:   "multiple voices multiple steal",
		// 	voices: 4,
		// 	events: []midivoice.MidiEvent{
		// 		{Note: 50, Velocity: 127},
		// 		{Note: 51, Velocity: 127},
		// 		{Note: 52, Velocity: 127},
		// 		{Note: 53, Velocity: 127},
		// 		{Note: 54, Velocity: 127},
		// 		{Note: 55, Velocity: 127},
		// 		{Note: 56, Velocity: 127},
		// 		{Note: 57, Velocity: 127},
		// 	},
		// 	expectedState: map[int]result{
		// 		0: {54, 127},
		// 		1: {55, 127},
		// 		2: {56, 127},
		// 		3: {57, 127},
		// 	},
		// },
	}

	for _, test := range cases {
		t.Run(test.name, func(t *testing.T) {
			m := midivoice.NewManager(test.channel, test.voices)
			for _, event := range test.events {
				m.Handle(event)
			}
			for voice, state := range test.expectedState {
				note, velocity := m.State(voice)
				assert.Equal(t, state.note, note)
				assert.Equal(t, state.velocity, velocity)
			}
		})
	}
}
