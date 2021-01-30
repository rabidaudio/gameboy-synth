/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Adapter object providing a Swift-accessible interface to the filter's underlying DSP code.
*/

#import <AVFoundation/AVFoundation.h>
#import <CoreAudioKit/AUViewController.h>
//#import "FilterDSPKernel.hpp"
//#import "BufferedAudioBus.hpp"
#import "Gb_Apu.h"
#import "ApuAdapter.h"

@implementation ApuAdapter {
    // C++ members need to be ivars; they would be copied on access if they were properties.
//    FilterDSPKernel  _kernel;
//    BufferedInputBus _inputBus;
    Gb_Apu _apu;
    Blip_Buffer _buffer;
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
        if (_buffer.set_sample_rate(44100, 44100/512) != 0) {
            return nil;
        }
        _apu.set_output(&_buffer, nullptr, nullptr, 0);
        _apu.reset();
    }
    return self;
}

- (void)write:(unsigned char)value toRegister:(unsigned int)addr at:(int)time {
    _apu.write_register(time, addr, value);
}

- (unsigned char)readFromRegister:(unsigned int)addr at:(int)time {
    return _apu.read_register(time, addr);
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
    _apu.write_register(0, 0xFF26, (bypass ? 1 : 0) << 7);
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
    __block Gb_Apu *apu = &_apu;
    __block Blip_Buffer *buffer = &_buffer;
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
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            apu->end_frame(2 * GB_APU_OVERCLOCK);
            buffer->end_frame(2 * GB_APU_OVERCLOCK);
        }
        // now convert the int16 data into float32 on the way out
        for (int channel = 0; channel < outAudioBufferList->mNumberBuffers; ++channel) {
            for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                AVAudioFrameCount framesRead = 0;
                while (framesRead < frameCount) {
                    blip_sample_t *offset = data + framesRead;
                    long r = buffer->read_samples(offset, frameCount - framesRead);
                    framesRead += (AVAudioFrameCount) r;
                }

                int frameOffset = frameIndex;
//                float* in  = (float*)inBufferListPtr->mBuffers[channel].mData  + frameOffset;
                blip_sample_t* in = data + frameOffset;
                float* out = (float*)outAudioBufferList->mBuffers[channel].mData + frameOffset;
                *out = (float)(*in) / 65526;
            }
        }
//        apu->end_frame(512);
        return noErr;
    };
}

@end
