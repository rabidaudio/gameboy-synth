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

        /*
         Control/Status
          NR50 FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol
          NR51 FF25 NW21 NW21 Left enables, Right enables
          NR52 FF26 P--- NW21 Power control/status, Channel length statuses
         */
        apu.write(0x80, toRegister: 0xff26)
        apu.write(0x11, toRegister: 0xff25)

        /*
         FF11 - NR11 - Channel 1 Sound length/Wave pattern duty (R/W)
         Bit 7-6 - Wave Pattern Duty (Read/Write)
         Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
         */
        apu.write(0x80, toRegister: 0xff11)

        /*
         FF10 - NR10 - Channel 1 Sweep register (R/W)
         Bit 6-4 - Sweep Time
         Bit 3   - Sweep Increase/Decrease
                    0: Addition    (frequency increases)
                    1: Subtraction (frequency decreases)
         Bit 2-0 - Number of sweep shift (n: 0-7)
         */
        apu.write(0x00, toRegister: 0xFF10)

        let freq: UInt16 = 1750 // approx A 440Hz
        /*
         #FF13 - NR13 - Channel 1 Frequency lo (Write Only)
         Lower 8 bits of 11 bit frequency (x). Next 3 bit are in NR14 ($FF14)
         */
        apu.write(UInt8(freq & 0xff), toRegister: 0xff13)
        /*
         FF12 - NR12 - Channel 1 Volume Envelope (R/W)
          Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
          Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
          Bit 2-0 - Number of envelope sweep (n: 0-7)
                    (If zero, stop envelope operation.)
         */
        apu.write(0xf0, toRegister: 0xff12)
        /*
         #FF14 - NR14 - Channel 1 Frequency hi (R/W)
         Bit 7   - Initial (1=Restart Sound)     (Write Only)`
         Bit 6   - Counter/consecutive selection (Read/Write)`
                   (1=Stop output when length in NR11 expires)`
         Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)`
         */
        apu.write(UInt8(freq >> 8) | 0x80, toRegister: 0xff14)
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
