/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Adapter object providing a Swift-accessible interface to the filter's underlying DSP code.
*/

#define HAVE_CONFIG_H

#import <AVFoundation/AVFoundation.h>
#import <CoreAudioKit/AUViewController.h>
#import "Basic_Gb_Apu.h"
#import "ApuAdapter.h"

@implementation ApuAdapter {
    // C++ members need to be ivars; they would be copied on access if they were properties.
    Basic_Gb_Apu _apu;
    NSMutableData* _data;
    AVAudioPCMBuffer* _outbuf;
    AVAudioFormat* _format;
    BOOL _bypassed;
}

static int const sampleRate = 44100;
// TODO: should we match the frames for the 60fps of the APU?
static AVAudioFrameCount const maxFrameCount = 512;

- (instancetype)init {

    if (self = [super init]) {
        _bypassed = false;
        _format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:sampleRate channels:2];

        // Create the input and output busses.
//        _outputBus = [[AUAudioUnitBus alloc] initWithFormat:_format error:nil];
        _data = [NSMutableData dataWithLength:sizeof(blip_sample_t) * maxFrameCount];
        _outbuf = [[AVAudioPCMBuffer alloc] initWithPCMFormat:_format frameCapacity:maxFrameCount];

        // framCount is 512 converted to ms
//        if (!_buffer.set_sample_rate(sampleRate, sampleSize/sampleSize)) {
//            return nil;
//        }
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

- (BOOL)shouldBypassEffect {
    return _bypassed;
}

- (void)setShouldBypassEffect:(BOOL)bypass {
    _bypassed = bypass;
//    _apu.write_register(0xFF26, (bypass ? 1 : 0) << 7);
}

- (AUAudioFrameCount)maximumFramesToRender {
    return maxFrameCount;
}

- (AVAudioFormat *)format {
    return _format;
}

- (void)allocateRenderResources {
//    _inputBus.allocateRenderResources(self.maximumFramesToRender);
}

- (void)deallocateRenderResources {
//    _inputBus.deallocateRenderResources();
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
//    __block Blip_Buffer *buffer = &_buffer;
    __block blip_sample_t *data = (blip_sample_t*) _data.mutableBytes;
    __block AVAudioPCMBuffer *outbuf = _outbuf;

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
                outAudioBufferList->mBuffers[i].mData = outbuf.mutableAudioBufferList->mBuffers[i].mData;
            }
        }

        static int delay;
        if ( --delay <= 0 )
        {
            delay = 12;

            // Start a new random tone
            int chan = rand() & 0x11;
            apu->write_register( 0xff26, 0x80 );
            apu->write_register( 0xff25, chan ? chan : 0x11 );
            apu->write_register( 0xff11, 0x80 );
            int freq = (rand() & 0x3ff) + 0x300;
            apu->write_register( 0xff13, freq & 0xff );
            apu->write_register( 0xff12, 0xf1 );
            apu->write_register( 0xff14, (freq >> 8) | 0x80 );
        }

        // Generate 1/60 second of sound into APU's sample buffer
        while (apu->samples_avail() < frameCount) {
            apu->end_frame();
        }
        long count = apu->read_samples(data, frameCount);

        // now convert the int16 data into float32 on the way out
        for (int channel = 0; channel < outAudioBufferList->mNumberBuffers; ++channel) {
            for (int frameIndex = 0; frameIndex < count; ++frameIndex) {
                int frameOffset = frameIndex;
                blip_sample_t* in = data + frameOffset;
                float* out = (float*)outAudioBufferList->mBuffers[channel].mData + frameOffset;
                *out = (float)(*in) / 65536;
            }
        }
        return noErr;
    };
}

@end
