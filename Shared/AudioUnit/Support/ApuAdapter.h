/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Adapter object providing a Swift-accessible interface to the filter's underlying DSP code.
*/

#import <AudioToolbox/AudioToolbox.h>

@class GameBoyAudioSynthDemoViewController;

NS_ASSUME_NONNULL_BEGIN

@interface ApuAdapter : NSObject

@property (nonatomic) AUAudioFrameCount maximumFramesToRender;
//@property (nonatomic, readonly) AUAudioUnitBus *outputBus;
@property (nonatomic, readonly) AVAudioFormat *format;

- (void)write:(unsigned char)value toRegister:(unsigned int)addr;
- (unsigned char)readFromRegister:(unsigned int)addr;
- (void)allocateRenderResources;
- (void)deallocateRenderResources;
- (AUInternalRenderBlock)internalRenderBlock;

@end

NS_ASSUME_NONNULL_END
