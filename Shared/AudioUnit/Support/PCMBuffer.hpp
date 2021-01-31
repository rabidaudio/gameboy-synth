//
//  PCMBuffer.c
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/30/21.
//  Copyright © 2021 Apple. All rights reserved.
//

//#include "PCMBuffer.h"

// TODO: which of these are needed?
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>
#include <stdio.h>

// Reusable non-ObjC class, accessible from render thread.
struct BufferedAudioBus { // TODO: rename
    AUAudioUnitBus* bus = nullptr;
    AUAudioFrameCount maxFrames = 0;

    AVAudioPCMBuffer* pcmBuffer = nullptr;
    NSMutableData* data = nullptr; // TODO: rename

    AudioBufferList const* originalAudioBufferList = nullptr; // TODO: remove
    AudioBufferList* mutableAudioBufferList = nullptr; // TODO: rename outAudioBufferList

    void init(AVAudioFormat* defaultFormat, AVAudioChannelCount maxChannels) {
        maxFrames = 0;
        pcmBuffer = nullptr;
        originalAudioBufferList = nullptr;
        mutableAudioBufferList = nullptr;

        bus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];

        bus.maximumChannelCount = maxChannels;
    }

    void allocateRenderResources(AUAudioFrameCount inMaxFrames) {
        maxFrames = inMaxFrames;

        pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bus.format frameCapacity: maxFrames];

        originalAudioBufferList = pcmBuffer.audioBufferList;
        mutableAudioBufferList = pcmBuffer.mutableAudioBufferList;

        data = [NSMutableData dataWithLength:sizeof(blip_sample_t) * maxFrames];
    }

    void deallocateRenderResources() {
        pcmBuffer = nullptr;
        originalAudioBufferList = nullptr;
        mutableAudioBufferList = nullptr;
        data = nullptr;
    }
};
