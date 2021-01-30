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
//@property (nonatomic, readonly) AUAudioUnitBus *inputBus;
@property (nonatomic, readonly) AUAudioUnitBus *outputBus;

//- (void)setParameter:(AUParameter *)parameter value:(AUValue)value;
//- (AUValue)valueForParameter:(AUParameter *)parameter;

- (void)write:(unsigned char)value toRegister:(unsigned int)addr at:(int)time;
- (unsigned char)readFromRegister:(unsigned int)addr at:(int)time;
- (void)allocateRenderResources;
- (void)deallocateRenderResources;
- (AUInternalRenderBlock)internalRenderBlock;

//- (NSArray<NSNumber *> *)magnitudesForFrequencies:(NSArray<NSNumber *> *)frequencies;

@end

NS_ASSUME_NONNULL_END
