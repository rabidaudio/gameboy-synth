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
#import "EventManager.h"
#import "PCMBuffer.hpp"

// This obj-c class acts as a wrapper around the Apu and a C struct containing audio buffers.
// This allows Swift to access the functionality, and avoids Swift/obj-c memory behavior (ARC
// and whatever else) from making allocations during the render loop.
@implementation ApuAdapter {
    // C++ members need to be ivars; they would be copied on access if they were properties.

    // TODO: if we need block-free syncronization between the UI and the audio render loop,
    // we could use double buffering by keeping two Apus, and writing to one while we
    // read from the other. At this point that optimization might be premature; it seems like
    // write_register is relatively atomic.
    // Actually I think a better way is to queue up changes from the UI and run them
    // at the beginning of the next render loop
    Basic_Gb_Apu apu_;
    PCMBuffer buffer_;
    EventManager eventManager_;
}

static int const channels = 2;
static int const sampleRate = 44100;

- (instancetype)init {

    if (self = [super init]) {
        AVAudioFormat* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:sampleRate channels:channels];
        buffer_.init(format, channels);
        // Set sample rate and check for out of memory error
        if (apu_.set_sample_rate(sampleRate) != blargg_success) {
            return nil;
        }
        eventManager_.init(&apu_);
    }
    return self;
}

//- (void)write:(unsigned char)data toRegister:(gb_addr_t)addr {
//    apu_.write_register(addr, data);
//}
//
//- (unsigned char)readFromRegister:(gb_addr_t)addr {
//    return apu_.read_register(addr);
//}

- (void)configure:(UInt8)oscillator with:(struct GBMidiConfig)config {
    eventManager_.setConfig(oscillator, config);
}

- (AUAudioFrameCount)maximumFramesToRender {
    return buffer_.maxFrames;
}

- (void)setMaximumFramesToRender:(AUAudioFrameCount)maximumFramesToRender {
    buffer_.maxFrames = maximumFramesToRender;
}

- (AVAudioFormat *)format {
    return buffer_.outFormat;
}

- (void)allocateRenderResources {
    buffer_.allocateRenderResources();
}

- (void)deallocateRenderResources {
    buffer_.deallocateRenderResources();
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
    __block Basic_Gb_Apu *apu = &apu_;
    __block PCMBuffer *buffer = &buffer_;
    __block EventManager *eventManager = &eventManager_;

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

        const AURenderEvent *event = realtimeEventListHead;
        while (event != nullptr) {
            switch (event->head.eventType) {
                case AURenderEventParameter:
                case AURenderEventParameterRamp:
                    // AUParameterEvent const& paramEvent = event->parameter;
                    // paramEvent.parameterAddress, paramEvent.value, paramEvent.rampDurationSampleFrames
                    break; // TODO: ramp parameters
                case AURenderEventMIDI:
                    // eventManager->handleMIDIEvent(event->MIDI.eventSampleTime, event->MIDI.data);
                    break;
                case AURenderEventMIDISysEx:
                    break; // unsupported
                default:
                    break;
            }
            event = event->head.next;
        }

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
        // TODO: a custom implementation of Blip_Buffer might let us avoid
        // having 3 buffers, and getting that down to 2
        buffer->convertToFloat32(count);
        return noErr;
    };
}

@end
