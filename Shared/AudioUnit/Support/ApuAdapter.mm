//
//  ApuAdapter.mm
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

#define HAVE_CONFIG_H

#import <AVFoundation/AVFoundation.h>
#import <CoreAudioKit/AUViewController.h>
#import "Basic_Gb_Apu.h"
#import "ApuAdapter.h"
#import "PCMBuffer.hpp"

// This obj-c class acts as a wrapper around the Apu and a C struct containing audio buffers.
// This allows Swift to access the functionality, and avoids Swift/obj-c memory behavior (ARC
// and whatever else) from making allocations during the render loop.
@implementation ApuAdapter {
    // C++ members need to be ivars; they would be copied on access if they were properties.
    Basic_Gb_Apu _apu;
    PCMBuffer _buffer;
}

static int const channels = 2;
static int const sampleRate = 44100;

- (instancetype)init {

    if (self = [super init]) {
        AVAudioFormat* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:sampleRate channels:channels];
        _buffer.init(format, channels);
        // Set sample rate and check for out of memory error
        if (_apu.set_sample_rate(sampleRate) != blargg_success) {
            return nil;
        }
    }
    return self;
}

- (void)write:(unsigned char)data toRegister:(gb_addr_t)addr {
    _apu.write_register(addr, data);
}

- (unsigned char)readFromRegister:(gb_addr_t)addr {
    return _apu.read_register(addr);
}

- (AUAudioFrameCount)maximumFramesToRender {
    return _buffer.maxFrames;
}

- (void)setMaximumFramesToRender:(AUAudioFrameCount)maximumFramesToRender {
    _buffer.maxFrames = maximumFramesToRender;
}

- (AVAudioFormat *)format {
    return _buffer.outFormat;
}

- (void)allocateRenderResources {
    _buffer.allocateRenderResources();
}

- (void)deallocateRenderResources {
    _buffer.deallocateRenderResources();
}

#pragma mark - AUAudioUnit (AUAudioUnitImplementation)

// // https://developer.apple.com/documentation/audiotoolbox/auinternalrenderblock
// Subclassers must provide a AUInternalRenderBlock (via a getter) to implement rendering.
- (AUInternalRenderBlock)internalRenderBlock {
    /*
     Capture in locals to avoid ObjC member lookups. If "self" is captured in
     render, we're doing it wrong.
     */
    // Specify captured objects are mutable.
    __block Basic_Gb_Apu *apu = &_apu;
    __block PCMBuffer *buffer = &_buffer;

    return ^AUAudioUnitStatus(AudioUnitRenderActionFlags *actionFlags,
                              const AudioTimeStamp       *timestamp,
                              AVAudioFrameCount           frameCount,
                              NSInteger                   outputBusNumber,
                              AudioBufferList            *outputData,
                              const AURenderEvent        *realtimeEventListHead,
                              AURenderPullInputBlock      pullInputBlock) {

//        AudioUnitRenderActionFlags pullFlags = 0;
//
//        if (frameCount > state->maximumFramesToRender()) {
//            return kAudioUnitErr_TooManyFramesToProcess;
//        }

        // If passed null output buffer pointers, process in-our own buffer.
        AudioBufferList *outAudioBufferList = outputData;
        if (outAudioBufferList->mBuffers[0].mData == nullptr) {
            for (UInt32 i = 0; i < outAudioBufferList->mNumberBuffers; ++i) {
                outAudioBufferList->mBuffers[i].mData = buffer->mutableAudioBufferList->mBuffers[i].mData;
            }
        }
        // whichever buffer we're using, make sure PCMBuffer has the pointer to it
        buffer->mutableAudioBufferList = outAudioBufferList;

        // Generate 1/60 second of sound into APU's sample buffer
        while (apu->samples_avail() < frameCount) {
            apu->end_frame();
        }
        blip_sample_t *data = buffer->int16Buffer();
        long count = apu->read_samples(data, frameCount);
        // now convert the int16 data into float32 on the way out
        buffer->convertToFloat32(count);
        return noErr;
    };
}

@end
