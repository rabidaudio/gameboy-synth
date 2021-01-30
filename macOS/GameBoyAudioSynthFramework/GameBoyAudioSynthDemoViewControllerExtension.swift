/*
See LICENSE folder for this sample’s licensing information.

Abstract:GameBoyAudioSynthDemoViewController is the app extension's principal class, responsible for creating both the audio unit and its view.
*/

import CoreAudioKit

extension GameBoyAudioSynthDemoViewController: AUAudioUnitFactory {
    public func createAudioUnit(with componentDescription: AudioComponentDescription) throws -> AUAudioUnit {
        audioUnit = try GameBoyAudioSynthDemo(componentDescription: componentDescription, options: [])
        return audioUnit!
    }
}
