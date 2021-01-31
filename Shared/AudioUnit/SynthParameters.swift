//
//  SynthParameters.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//
import Foundation

class SynthParameters {

    enum Channel: UInt8 {
        case square1 = 1, square2, wave, noise
    }

    enum SquareWaveOscParam: UInt16 {
        case duty, volume
        // TODO: midi voice, midi channel, transpose,
        // volume envelope, frequency envelope, (play timer?)

        func address(channel: Channel) -> AUParameterAddress {
            return AUParameterAddress.encode(channel: channel, param: self)
        }
    }

    var osc1Params: AUParameterGroup = {
        return AUParameterTree.createGroup(withIdentifier: "OSC1", name: "Oscillator 1", children: [
            AUParameterTree.createParameter(withIdentifier: "duty",
                                            name: "Duty Cycle",
                                            address: SquareWaveOscParam.duty.address(channel: .square1),
                                            min: 12.5,
                                            max: 75,
                                            unit: .percent,
                                            unitName: nil,
                                            flags: [.flag_IsReadable,
                                                    .flag_IsWritable,
                                                    .flag_CanRamp],
                                            valueStrings: nil,
                                            dependentParameters: nil),
            AUParameterTree.createParameter(withIdentifier: "volume",
                                            name: "Volume",
                                            address: SquareWaveOscParam.volume.address(channel: .square1),
                                            min: 0,
                                            max: 1,
                                            unit: .mixerFaderCurve1,
                                            unitName: nil,
                                            flags: [.flag_IsReadable,
                                                    .flag_IsWritable,
                                                    .flag_CanRamp],
                                            valueStrings: nil,
                                            // TODO: can effect volume envelope
                                            dependentParameters: nil),
        ])
    }()

    let parameterTree: AUParameterTree

    init(state: SynthState) {

        // Create the audio unit's tree of parameters
        parameterTree = AUParameterTree.createTree(withChildren: [osc1Params])

        // Closure observing all externally-generated parameter value changes.
        parameterTree.implementorValueObserver = { param, value in
            guard let (channel, type) = param.address.decode() else {
                print("WARN: got update from unknown parameter \(param.address)")
                return
            }
            switch channel {
            case .square1:
                switch type {
                case .duty:
                    return // TODO: update channel
                case .volume:
                    return // TODO: update channel
                }
            default:
                return // TODO: update channel
            }
        }

        // Closure returning state of requested parameter.
        parameterTree.implementorValueProvider = { param in
            // TODO: read from controller
            guard let (_, type) = param.address.decode() else {
                print("WARN: read requested from unknown parameter \(param.address)")
                return AUValue()
            }
            switch type {
            case .duty:
                return AUValue(0.5)
            case .volume:
                return AUValue(1)
            }
        }

        // Closure returning string representation of requested parameter value.
        parameterTree.implementorStringFromValueCallback = { param, value in
            guard let (_, type) = param.address.decode() else {
                return "?"
            }

            switch type {
            case SquareWaveOscParam.duty:
                return String(format: "%.1f%%", value ?? param.value)
            case SquareWaveOscParam.volume:
                return String(format: "%.f", value ?? param.value)
            }
        }
    }
    
//    func setParameterValues(cutoff: AUValue, resonance: AUValue) {
//        cutoffParam.value = cutoff
//        resonanceParam.value = resonance
//    }
}

fileprivate extension AUParameterAddress {
    static func encode(channel: SynthParameters.Channel, param: SynthParameters.SquareWaveOscParam ) -> AUParameterAddress {
        return AUParameterAddress(UInt64(channel.rawValue) + (UInt64(param.rawValue) << 8))
    }

    func decode() -> (SynthParameters.Channel, SynthParameters.SquareWaveOscParam)? {
        let c = SynthParameters.Channel(rawValue: UInt8(self & 0x0F))
        let p = SynthParameters.SquareWaveOscParam(rawValue: UInt16((self >> 8) & 0xFFFF))

        guard let cc = c, let pp = p else {
            return nil
        }
        return (cc, pp)
    }
}
