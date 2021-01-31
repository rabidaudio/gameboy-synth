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

let sinWavetable: Wavetable = {
    Array(0..<32).map { i in
        let s = sin(2 * Double.pi * Double(i) / 32)
        return UInt8((s * 8) + 4)
    }
}()

let squareWavetable: Wavetable = {
    Array(0..<32).map { i in
        return i >= 16 ? 0x0F : 0x00
    }
}()

let noiseWavetable: Wavetable = {
    Array(0..<32).map { i in
        return UInt8.random(in: 0x00...0x0F)
    }
}()

let triangleWavetable: Wavetable = {
    Array(0..<32).map { i in
        if i >= 16 {
            return UInt8(32 - i - 1)
        } else {
            return UInt8(i)
        }
    }
}()

let rampUpWavetable: Wavetable = {
    Array(0..<32).map { i in
        return UInt8(i % 16)
    }
}()

let rampDownWavetable: Wavetable = {
    Array(0..<32).map { i in
        return UInt8(16 - (i % 16))
    }
}()

