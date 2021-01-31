//
//  Controller.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import Foundation

struct MidiConfig {
    var enabled: Bool = false
    @BoundedTo(1...4) var voice: Int = 1
    @BoundedTo(1...2) var channel: Int = 1
    @BoundedTo(-48...48) var transpose: Int = 0
}

class SynthState : ObservableObject {
    @Published var osc1MidiConfig = MidiConfig()
}

class WavetableController: ObservableObject {
    @Published var wavetable: Wavetable
    @Published var defaultWavetable: DefaultWavetables {
        didSet {
            self.wavetable = defaultWavetable.wavetable
        }
    }

    init(wavetable: Wavetable, defaultWavetable: DefaultWavetables = .sine) {
        self.wavetable = wavetable
        self.defaultWavetable = defaultWavetable
    }

    convenience init(defaultWavetable: DefaultWavetables = .sine) {
        self.init(wavetable: defaultWavetable.wavetable, defaultWavetable: defaultWavetable)
    }
}
