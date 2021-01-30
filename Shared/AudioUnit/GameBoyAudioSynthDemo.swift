/*
See LICENSE folder for this sample’s licensing information.

Abstract:
An AUAudioUnit subclass implementing a low-pass filter with resonance.
*/

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

public class GameBoyAudioSynthDemo: AUAudioUnit {

    // TODO: may want to turn down the frame count to improve resolution
    private let frameCount: AUAudioFrameCount = 512
    private let numChannels: AVAudioChannelCount = 2
    private let sampleRate: Double = 44100

    private var format: AVAudioFormat {
        // NOTE: Ideally we'd use standard 16bit int PCM format, but AVAudioPCMBuffer
        // operates on deinterleaved Float32
        return AVAudioFormat(standardFormatWithSampleRate: sampleRate, channels: numChannels)!
//        return AVAudioFormat(commonFormat: AVAudioCommonFormat.pcmFormatInt16, sampleRate: sampleRate, channels: numChannels, interleaved: true)!
    }

    private let parameters: GameBoyAudioSynthDemoParameters

    private let apu = ApuAdapter()

    private var outputBus: AUAudioUnitBus!
    private var buffer: AVAudioPCMBuffer!
    private var outputBusArray: AUAudioUnitBusArray!

    // The owning view controller
    weak var viewController: GameBoyAudioSynthDemoViewController?

    public override var outputBusses: AUAudioUnitBusArray {
        return outputBusArray
    }
    
    /// The tree of parameters provided by this AU.
    public override var parameterTree: AUParameterTree? {
        get { return parameters.parameterTree }
        set { /* TODO allow modification */ }
    }

    public override var maximumFramesToRender: AUAudioFrameCount {
        get { return frameCount }
        set { /* TODO allow modification */ }
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
            
            // Factory presets need to always have a number >= 0.
            if preset.number >= 0 {
                let values = factoryPresetValues[preset.number]
                parameters.setParameterValues(cutoff: values.cutoff, resonance: values.resonance)
                _currentPreset = preset
            }
            // User presets are always negative.
            else {
                // Attempt to restore the archived state for this user preset.
                do {
                    fullStateForDocument = try presetState(for: preset)
                    // Set the currentPreset after we've successfully restored the state.
                    _currentPreset = preset
                } catch {
                    print("Unable to restore set for preset \(preset.name)")
                }
            }
        }
    }
    
    /// Indicates that this Audio Unit supports persisting user presets.
    public override var supportsUserPresets: Bool {
        return true
    }

    public override init(componentDescription: AudioComponentDescription,
                         options: AudioComponentInstantiationOptions = []) throws {
        parameters = GameBoyAudioSynthDemoParameters()
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
        apu.write(0b1_0000000, toRegister: 0xFF26, at: 0)
        apu.write(0xFF, toRegister: 0xFF24, at: 0)
        apu.write(0xFF, toRegister: 0xFF25, at: 0)

        /*
         FF10 - NR10 - Channel 1 Sweep register (R/W)
         Bit 6-4 - Sweep Time
         Bit 3   - Sweep Increase/Decrease
                    0: Addition    (frequency increases)
                    1: Subtraction (frequency decreases)
         Bit 2-0 - Number of sweep shift (n: 0-7)
         */
        apu.write(0x00, toRegister: 0xFF10, at: 0)
        /*
         FF11 - NR11 - Channel 1 Sound length/Wave pattern duty (R/W)
         Bit 7-6 - Wave Pattern Duty (Read/Write)
         Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
         */
        apu.write(0b10_000000, toRegister: 0xFF11, at: 0)
        /*
         FF12 - NR12 - Channel 1 Volume Envelope (R/W)
          Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
          Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
          Bit 2-0 - Number of envelope sweep (n: 0-7)
                    (If zero, stop envelope operation.)
         */
        apu.write(0xF0, toRegister: 0xFF12, at: 0)

        let period: UInt16 = 1750
        /*
         #FF13 - NR13 - Channel 1 Frequency lo (Write Only)
         Lower 8 bits of 11 bit frequency (x). Next 3 bit are in NR14 ($FF14)
         */
        /*
         #FF14 - NR14 - Channel 1 Frequency hi (R/W)
         Bit 7   - Initial (1=Restart Sound)     (Write Only)`
         Bit 6   - Counter/consecutive selection (Read/Write)`
                   (1=Stop output when length in NR11 expires)`
         Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)`
         */
        apu.write(UInt8(period & 0x0F), toRegister: 0xFF13, at: 0)
        apu.write(0b10_000000 | (UInt8(period >> 8) & 0b00000_111), toRegister: 0xFF13, at: 0)
    }

    public override func allocateRenderResources() throws {
        try super.allocateRenderResources()
        buffer = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: frameCount)!
        apu.allocateRenderResources()
    }

    public override func deallocateRenderResources() {
        super.deallocateRenderResources()
        // TODO find some way to dealloc the buffer
        apu.deallocateRenderResources()
    }

//    private var freq: Float32 = 1000.0
    private var samples: UInt32 {
        return UInt32(sampleRate / 1000.0 / 2)
    }

    var q = false

    // https://developer.apple.com/documentation/audiotoolbox/auinternalrenderblock
    private func render(
        actionFlags: UnsafeMutablePointer<AudioUnitRenderActionFlags>,
        timestamp: UnsafePointer<AudioTimeStamp>,
        frameCount: AUAudioFrameCount,
        outputBusNumber: Int,
        outputData: UnsafeMutablePointer<AudioBufferList>,
        realtimeEventListHead: UnsafePointer<AURenderEvent>?,
        pullInputBlock: AURenderPullInputBlock?
    ) -> AUAudioUnitStatus {
        let nullPtr = UnsafeMutablePointer<AudioBufferList>(nil)
        if outputData == nullPtr {
            // it's supposed to be our responsibility to init the buffer if it's null
            // but it never seems to be null and the format doesn't sem to match that of `buffer`
            fatalError("null outputData")
            // outputData.assign(from: buffer.mutableAudioBufferList, count: 1)
        }
        if outputBusNumber != 0 {
            fatalError("outputbus \(outputBusNumber)")
        }
        // mBuffers is an array but Swift's bridging is screwing up so it can't be accessed as an array
        withUnsafeMutablePointer(to: &outputData.pointee.mBuffers) { bufs in
            // Note: as an optimization, sets the left and right channels at the same time.
            // if we ever supported more channels we'll need to make this a loop
            let lbuf = bufs.pointee
            let rbuf = bufs.successor().pointee
            var lptr = lbuf.mData!.assumingMemoryBound(to: Float32.self)
            var rptr = rbuf.mData!.assumingMemoryBound(to: Float32.self)
            let value: Float32 = q ? -0.5 : 0.5
            for _ in 0..<frameCount {
                lptr.initialize(to: value)
                lptr = lptr.advanced(by: 1)
                rptr.initialize(to: value)
                rptr = rptr.advanced(by: 1)
            }
            q = !q
        }
        return noErr
    }

    public override var internalRenderBlock: AUInternalRenderBlock {
//        return self.render
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
