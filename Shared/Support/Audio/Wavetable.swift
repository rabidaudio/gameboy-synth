//
//  Wavetable.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import Foundation

// A 4-bit, 32-sample wave. Upper 4 bits of UInt8 are ignored
typealias Wavetable = [UInt8]

enum DefaultWavetables: String {
    case sine, square, triangle, rampUp, rampDown, noise

    var wavetable: Wavetable {
        switch self {
        case .sine:
            return Array(0..<32).map { i in
                let s = sin(2 * Double.pi * Double(i) / 32)
                let i = Int((s * 8) + 8).clamped(to: 0...15)
                return UInt8(i)
            }
        case .square:
            return Array(0..<32).map { i in
                return i >= 16 ? 0x0F : 0x00
            }
        case .triangle:
            return Array(0..<32).map { i in
                if i >= 16 {
                    return UInt8(16 - (32 - i - 1) - 1)
                } else {
                    return UInt8(16 - i - 1)
                }
            }
        case .rampDown:
            return     Array(0..<32).map { i in
                return UInt8(i % 16)
            }
        case .rampUp:
            return Array(0..<32).map { i in
                return UInt8(16 - (i % 16) - 1)
            }
        case .noise:
            return Array(0..<32).map { i in
                return UInt8.random(in: 0x00...0x0F)
            }
        }
    }
}

extension DefaultWavetables: CaseIterable, Identifiable, Equatable {
    var id: String {
        return rawValue
    }
}
