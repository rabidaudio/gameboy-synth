/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Adapter object providing a Swift-accessible interface to the filter's underlying DSP code.
*/

#import <AVFoundation/AVFoundation.h>
#import <CoreAudioKit/AUViewController.h>
//#import "FilterDSPKernel.hpp"
//#import "BufferedAudioBus.hpp"
#import "Basic_Gb_Apu.h"
#import "ApuAdapter.h"

@implementation ApuAdapter {
    // C++ members need to be ivars; they would be copied on access if they were properties.
//    FilterDSPKernel  _kernel;
//    BufferedInputBus _inputBus;
    Basic_Gb_Apu _apu;
//    Blip_Buffer _buffer;
    NSMutableData* _data;
    AVAudioPCMBuffer* _outbuf;
    BOOL _bypassed;
}

- (instancetype)init {

    if (self = [super init]) {
        _bypassed = false;
        AVAudioFormat *format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100 channels:2];
        // Create a DSP kernel to handle the signal processing.
//        _kernel.init(format.channelCount, format.sampleRate);
//        _kernel.setParameter(FilterParamCutoff, 0);
//        _kernel.setParameter(FilterParamResonance, 0);

        // Create the input and output busses.
//        _inputBus.init(format, 8);
        _outputBus = [[AUAudioUnitBus alloc] initWithFormat:format error:nil];
        _data = [NSMutableData dataWithLength:sizeof(blip_sample_t) * 512];
        _outbuf = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:512];

        // framCount is 512 converted to ms
//        if (!_buffer.set_sample_rate(44100, 44100/512)) {
//            return nil;
//        }
        // Set sample rate and check for out of memory error
        if (_apu.set_sample_rate(44100) != blargg_success) {
            return nil;
        }
    }
    return self;
}

- (void)write:(unsigned char)data toRegister:(gb_addr_t)addr {
//    if ( addr >= _apu.start_addr && addr <= apu.end_addr )
//    _apu.write_register( addr, data );
}

- (unsigned char)readFromRegister:(gb_addr_t)addr {
//    return _apu.read_register(addr);
    return 0x00;
}

//- (NSArray<NSNumber *> *)magnitudesForFrequencies:(NSArray<NSNumber *> *)frequencies {
//    FilterDSPKernel::BiquadCoefficients coefficients;
//
//    double inverseNyquist = 2.0 / self.outputBus.format.sampleRate;
//
//    coefficients.calculateLopassParams(_kernel.cutoffRamper.getUIValue(), _kernel.resonanceRamper.getUIValue());
//
//    NSMutableArray<NSNumber *> *magnitudes = [NSMutableArray arrayWithCapacity:frequencies.count];
//
//    for (NSNumber *number in frequencies) {
//        double frequency = [number doubleValue];
//        double magnitude = coefficients.magnitudeForFrequency(frequency * inverseNyquist);
//
//        [magnitudes addObject:@(magnitude)];
//    }
//
//    return [NSArray arrayWithArray:magnitudes];
//}

- (BOOL)shouldBypassEffect {
    return _bypassed;
}

- (void)setShouldBypassEffect:(BOOL)bypass {
    _bypassed = bypass;
//    _apu.write_register(0xFF26, (bypass ? 1 : 0) << 7);
}

- (void)allocateRenderResources {
//    _inputBus.allocateRenderResources(self.maximumFramesToRender);
//    _kernel.init(self.outputBus.format.channelCount, self.outputBus.format.sampleRate);
//    _kernel.reset();
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
//    __block FilterDSPKernel *state = &_kernel;
//    __block BufferedInputBus *input = &_inputBus;
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
//
//        AUAudioUnitStatus err = input->pullInput(&pullFlags, timestamp, frameCount, 0, pullInputBlock);
//
//        if (err != 0) { return err; }
//
//        AudioBufferList *inAudioBufferList = input->mutableAudioBufferList;

        /*
         Important:
         If the caller passed non-null output pointers (outputData->mBuffers[x].mData), use those.

         If the caller passed null output buffer pointers, process in memory owned by the Audio Unit
         and modify the (outputData->mBuffers[x].mData) pointers to point to this owned memory.
         The Audio Unit is responsible for preserving the validity of this memory until the next call to render,
         or deallocateRenderResources is called.

         If your algorithm cannot process in-place, you will need to preallocate an output buffer
         and use it here.

         See the description of the canProcessInPlace property.
         */
        // If passed null output buffer pointers, process in-our own buffer.
        AudioBufferList *outAudioBufferList = outputData;
        if (outAudioBufferList->mBuffers[0].mData == nullptr) {
            for (UInt32 i = 0; i < outAudioBufferList->mNumberBuffers; ++i) {
                outAudioBufferList->mBuffers[i].mData = outbuf.mutableAudioBufferList->mBuffers[i].mData;
            }
        }
//        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
//            apu->end_frame(2 * GB_APU_OVERCLOCK);
//            buffer->end_frame(2 * GB_APU_OVERCLOCK);
//        }

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
        apu->end_frame();

        // TODO: while apu->samples_avail() < frameCount
        long count = apu->read_samples(data, MIN(apu->samples_avail(), frameCount));

        // now convert the int16 data into float32 on the way out
        for (int channel = 0; channel < outAudioBufferList->mNumberBuffers; ++channel) {
            for (int frameIndex = 0; frameIndex < count; ++frameIndex) {
//                AVAudioFrameCount framesRead = 0;
//                while (framesRead < frameCount) {
//                    blip_sample_t *offset = data + framesRead;
//                    long r = buffer->read_samples(offset, frameCount - framesRead);
//                    framesRead += (AVAudioFrameCount) r;
//                }


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
