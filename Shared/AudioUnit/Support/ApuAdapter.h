//
//  ApuAdapter.h
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>
#include "Structs.h"

@class GameBoyAudioSynthDemoViewController;

NS_ASSUME_NONNULL_BEGIN

@interface ApuAdapter : NSObject

@property (nonatomic) AUAudioFrameCount maximumFramesToRender;
@property (nonatomic, readonly) AUAudioUnitBus *outputBus;
@property (nonatomic, readonly) AVAudioFormat *format;

//- (void)write:(unsigned char)value toRegister:(unsigned int)addr;
//- (unsigned char)readFromRegister:(unsigned int)addr;
- (void)configure:(UInt8)oscillator with:(struct MidiConfig)config;
- (void)allocateRenderResources;
- (void)deallocateRenderResources;
- (AUInternalRenderBlock)internalRenderBlock;

@end

NS_ASSUME_NONNULL_END
