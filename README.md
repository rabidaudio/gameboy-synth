# Game Boy Synthesizer Plugin

**Work in progress**

This is an VST3/AU3 wrapper around the Game Boy sound unit. This exposes the Game Boy APU as a MIDI-controllable sythesizer instrument, usable as a standalone app or as a plugin for a DAW. It is powered by [Gb_Snd_Emu](http://blargg.8bitalley.com/libs/audio.html#Gb_Snd_Emu), the audio implementation used by many emulators including [Visual Boy Advance - M](https://github.com/visualboyadvance-m/visualboyadvance-m).

As an instrument, the Game Boy APU is a pretty constrained synthesizer. It contains 2 square wave oscillators, a 32-sample 4-bit arbitary wavetable oscillator, and a noise oscillator with a variable frequency. All oscillators except the wave have a linear volume envelope. One of the square waves has a linear frequency envelope. The resolution in both time and amplitude is relatively low, so the pitches are slightly out of tune. There's no controllable filter, only a static passive high-pass filter. The limited CPU of the platform limits the resolution at which these parameters are controllable. All of this adds up to a limited but iconic sound.

## Reference

- https://gbdev.io/pandocs/#sound-controller
- https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
- https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins/creating_custom_audio_effects
- http://blargg.8bitalley.com/libs/audio.html#Gb_Snd_Emu
- https://www.rockhoppertech.com/blog/writing-an-audio-unit-v3-instrument/
- http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing
- http://devnotes.kymatica.com/auv3_parameters.html
- https://juce.com/

## License

This project [contains code](Source/Gb_Snd_Emu-0.1.4) from [Gb_Snd_Emu](http://blargg.8bitalley.com/libs/audio.html#Gb_Snd_Emu) which is copyright Shay Green. This code is licenced under LGPL. It also [contains code](Source/midimanager) from [my separate polyphonic MIDI state machine](https://github.com/rabidaudio/midi-voicesteal) which is MIT licensed. The project is created using the [JUCE framework](https://juce.com/) under their GPLv3 license. Thus the remaining code of this project is GPLv3.
