//
//  Structs.h
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 2/13/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

#ifndef Structs_h
#define Structs_h

struct GBMidiConfig {
    bool enabled;
    unsigned int channel;
    unsigned int voice;
    int transpose;
};

enum GBDutyCycle: uint8_t {
    DUTY_CYCLE_12_5 = 0x00, DUTY_CYCLE_25 = 0x01, DUTY_CYCLE_50 = 0x02, DUTY_CYCLE_75 = 0x03
};

// unlike the square and noise waves with velocity of 4 bits,
// the wave has 2 bits: 00=0%, 01=100%, 10=50%, 11=25%
enum GBWaveVolume: uint8_t {
    WAVE_VOL_OFF = 0x00, WAVE_VOL_FULL = 0x01, WAVE_VOL_50 = 0x02, WAVE_VOL_25 = 0x03
};

#endif /* Structs_h */
