//
//  LFSR.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/30/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation


//    The linear feedback shift register (LFSR) generates a pseudo-random bit sequence.
//    It has a 15-bit shift register with feedback. When clocked by the frequency timer,
//    the low two bits (0 and 1) are XORed, all bits are shifted right by one, and the
//    result of the XOR is put into the now-empty high bit. If width mode is 1 (NR43),
//    the XOR result is ALSO put into bit 6 AFTER the shift, resulting in a 7-bit LFSR.
//    The waveform output is bit 0 of the LFSR, INVERTED.
class LFSR {
    private var shiftRegister: UInt16 = 0x0

    var widthMode = false

    func next() -> Bool {
        let a = shiftRegister & 0x02
        let b = shiftRegister & 0x01
        let c = a ^ b
        shiftRegister = (shiftRegister >> 1) | (c << 14)
        if widthMode {
            shiftRegister = (shiftRegister & ~(1 << 6)) | (c << 6)
        }
        return (shiftRegister & 0x01) == 0x00
    }
}
