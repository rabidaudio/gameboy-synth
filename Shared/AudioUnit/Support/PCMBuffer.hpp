//
//  PCMBuffer.c
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/30/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

//#include "PCMBuffer.h"

// TODO: which of these are needed?
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>
#include <stdio.h>

// Reusable non-ObjC class, accessible from render thread.
// NOTE: Ideally we'd use standard 16bit int PCM format, but AVAudioPCMBuffer
// operates on deinterleaved Float32.
// So instead of a normal double-buffer, we've got to triple-buffer. We use
// NSMutableData to store the int16 output of the APU, then we convert that to
// float32 into the AVAudioPCMBuffer.
// TODO: NSMutableData came from when this logic was in obj-c, but now that
// we're in C land we are free to malloc instead. I don't know enough about
// the platform to know the consequences of this either way, so I'm leaving
// it for now, I don't believe it's hurting anything.
struct PCMBuffer {
    AUAudioUnitBus* bus = nullptr;

    AUAudioFrameCount maxFrames;

    AVAudioPCMBuffer* float32PcmBuffer = nullptr;
    NSMutableData* int16PcmBuffer = nullptr;

    AudioBufferList* mutableAudioBufferList = nullptr;
    AVAudioFormat* outFormat = nullptr;

    void init(AVAudioFormat* defaultFormat, AVAudioChannelCount maxChannels) {
        // set the default frameCount to match the 60 fps of the APU.
        // The OS can request a different frameCount before allocateRenderResources
        maxFrames = defaultFormat.sampleRate/60;
        outFormat = defaultFormat;
        float32PcmBuffer = nullptr;
        int16PcmBuffer = nullptr;
        mutableAudioBufferList = nullptr;

        bus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
        bus.maximumChannelCount = maxChannels;
    }

    void allocateRenderResources() {
        float32PcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bus.format frameCapacity: maxFrames];
        mutableAudioBufferList = float32PcmBuffer.mutableAudioBufferList;
        int16PcmBuffer = [NSMutableData dataWithLength:sizeof(blip_sample_t) * maxFrames];
    }

    void deallocateRenderResources() {
        float32PcmBuffer = nullptr;
        mutableAudioBufferList = nullptr;
        int16PcmBuffer = nullptr;
    }

    // A pointer to write int16 wave data to
    blip_sample_t* int16Buffer() {
        return (blip_sample_t*) int16PcmBuffer.mutableBytes;
    }

    void convertToFloat32(long frameCount) {
        for (int channel = 0; channel < mutableAudioBufferList->mNumberBuffers; ++channel) {
            for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                int frameOffset = frameIndex;
                blip_sample_t* in = int16Buffer() + frameOffset;
                float* out = (float*)mutableAudioBufferList->mBuffers[channel].mData + frameOffset;
                *out = (float)(*in) / 65536;
            }
        }
    }
};
