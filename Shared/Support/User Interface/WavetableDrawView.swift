//
//  WavetableDrawView.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

struct WavetableDrawView: View {
    @State var data: Wavetable
    var body: some View {
        GeometryReader { geometry in
            Path { path in
                let sqWidth = geometry.size.width / 32
                let sqHeight = geometry.size.height / 16

                for (i, v) in data.enumerated() {
                    let y = CGFloat(16 - (v & 0x0F)) * sqHeight
                    let x = CGFloat(i) * sqWidth
                    path.addRect(CGRect(origin: CGPoint(x: x, y: y), size: CGSize(width: sqWidth, height: sqHeight)), transform: .identity)
                }
            }
            .fill(Color.blue)
        }
    }
}

struct WavetableDrawView_Previews: PreviewProvider {
    static var previews: some View {
        WavetableDrawView(data: sinWavetable)
    }
}
