//
//  GameBoyAudioSynth.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import Foundation
import AudioToolbox
import AVFoundation
import CoreAudioKit

fileprivate extension AUAudioUnitPreset {
    convenience init(number: Int, name: String) {
        self.init()
        self.number = number
        self.name = name
    }
}

public class GameBoyAudioSynth: AUAudioUnit {

    private var format: AVAudioFormat {
        return apu.format
    }

    private var parameters: SynthParameters!
    private let apu = ApuAdapter()

    private var outputBus: AUAudioUnitBus!
    private var outputBusArray: AUAudioUnitBusArray!

    // The owning view controller
    weak var viewController: GameBoyAudioSynthViewController? {
        didSet {
            guard let state = viewController?.state else { return }
            parameters = SynthParameters(state: state)
        }
    }

    public override var outputBusses: AUAudioUnitBusArray {
        return outputBusArray
    }
    
    /// The tree of parameters provided by this AU.
    public override var parameterTree: AUParameterTree? {
        get { return parameters.parameterTree }
        set {
            /* TODO allow modification */
            fatalError("Cannot change parameterTree")
        }
    }

    public override var maximumFramesToRender: AUAudioFrameCount {
        get { return apu.maximumFramesToRender }
        set { apu.maximumFramesToRender = newValue }
    }

    public override var factoryPresets: [AUAudioUnitPreset] {
        return [
            AUAudioUnitPreset(number: 0, name: "Prominent"),
            AUAudioUnitPreset(number: 1, name: "Bright"),
            AUAudioUnitPreset(number: 2, name: "Warm")
        ]
    }

    private let factoryPresetValues:[(cutoff: AUValue, resonance: AUValue)] = [
        (2500.0, 5.0),    // "Prominent"
        (14_000.0, 12.0), // "Bright"
        (384.0, -3.0)     // "Warm"
    ]

    private var _currentPreset: AUAudioUnitPreset?
    
    /// The currently selected preset.
    public override var currentPreset: AUAudioUnitPreset? {
        get { return _currentPreset }
        set {
            // If the newValue is nil, return.
            guard let preset = newValue else {
                _currentPreset = nil
                return
            }

            // TODO: initalize from presets
//            // Factory presets need to always have a number >= 0.
//            if preset.number >= 0 {
//                let values = factoryPresetValues[preset.number]
//                parameters.setParameterValues(cutoff: values.cutoff, resonance: values.resonance)
//                _currentPreset = preset
//            }
//            // User presets are always negative.
//            else {
//                // Attempt to restore the archived state for this user preset.
//                do {
//                    fullStateForDocument = try presetState(for: preset)
//                    // Set the currentPreset after we've successfully restored the state.
//                    _currentPreset = preset
//                } catch {
//                    print("Unable to restore set for preset \(preset.name)")
//                }
//            }
        }
    }
    
    /// Indicates that this Audio Unit supports persisting user presets.
    public override var supportsUserPresets: Bool {
        return true
    }

    public override init(componentDescription: AudioComponentDescription,
                         options: AudioComponentInstantiationOptions = []) throws {
        try super.init(componentDescription: componentDescription, options: options)
        outputBus = try AUAudioUnitBus(format: format)
        outputBusArray = AUAudioUnitBusArray(audioUnit: self,
                                                busType: .output,
                                                busses: [outputBus])
        
        // Set the default preset
        currentPreset = factoryPresets.first

        // TODO: remove, testing only
        let config = MidiConfig(enabled: true)
        apu.configure(0, with: config.toGB())
    }

    public override func allocateRenderResources() throws {
        try super.allocateRenderResources()
        apu.allocateRenderResources()
    }

    public override func deallocateRenderResources() {
        super.deallocateRenderResources()
        apu.deallocateRenderResources()
    }

    public override var internalRenderBlock: AUInternalRenderBlock {
        return apu.internalRenderBlock()
    }

    // MARK: View Configurations
    public override func supportedViewConfigurations(_ availableViewConfigurations: [AUAudioUnitViewConfiguration]) -> IndexSet {
        var indexSet = IndexSet()

        let min = CGSize(width: 400, height: 100)
        let max = CGSize(width: 800, height: 500)

        for (index, config) in availableViewConfigurations.enumerated() {

            let size = CGSize(width: config.width, height: config.height)

            if size.width <= min.width && size.height <= min.height ||
                size.width >= max.width && size.height >= max.height ||
                size == .zero {

                indexSet.insert(index)
            }
        }
        return indexSet
    }

    public override func select(_ viewConfiguration: AUAudioUnitViewConfiguration) {
        viewController?.selectViewConfiguration(viewConfiguration)
    }
}

extension MidiConfig {
    func toGB() -> GBMidiConfig {
        return GBMidiConfig(enabled: self.enabled, channel: UInt32(self.channel - 1), voice: UInt32(self.voice - 1), transpose: Int32(self.transpose))
    }
}
