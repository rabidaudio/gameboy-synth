//
//  GameBoyAudioSynthApp.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

@main
struct GameBoyAudioSynthApp: App {
    var body: some Scene {
        WindowGroup {
            WavetableDrawView(data: sinWavetable)
        }
    }
}
