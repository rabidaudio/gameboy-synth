//
//  Structs.h
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 2/13/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

#ifndef Structs_h
#define Structs_h

struct MidiConfig {
    bool enabled;
    unsigned int channel;
    unsigned int voice;
    int offset;
};

enum DutyCycle {
    DUTY_CYCLE_12_5 = 0x00, DUTY_CYCLE_25 = 0x01, DUTY_CYCLE_50 = 0x10, DUTY_CYCLE_75 = 0x11

};

// unlike the square and noise waves with velocity of 4 bits,
// the wave has 2 bits: 00=0%, 01=100%, 10=50%, 11=25%
enum WaveVolume {
    WAVE_VOL_OFF = 0x00, WAVE_VOL_FULL = 0x01, WAVE_VOL_50 = 0x10, WAVE_VOL_25 = 0x11
};

#endif /* Structs_h */
