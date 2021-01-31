//
//  DrawableWavetableView.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

struct DrawableWavetableView: View {
    @State var wavetable: Wavetable

    var body: some View {
        GeometryReader { geometry in
            let sqWidth = geometry.size.width / 32
            let sqHeight = geometry.size.height / 16
            Path { path in
                for (i, v) in wavetable.enumerated() {
                    let y = CGFloat(v) * sqHeight
                    let x = CGFloat(i) * sqWidth
                    let r = CGRect(
                        origin: CGPoint(x: x, y: y),
                        size: CGSize(width: sqWidth, height: sqHeight)
                    )
                    path.addRect(r)
                }
            }
            .fill(Color.blue)
            .background(Color.init(white: 0.95))
            .gesture(DragGesture(minimumDistance: 0.1)
                        .onChanged { drag in
                            let x = Int(drag.location.x / sqWidth)
                            let y = Int(drag.location.y / sqWidth)
                            guard (0...31).contains(x) && (0...15).contains(y) else {
                                return
                            }
                            wavetable[x] = UInt8(y)
                        })
        }
    }
}

struct DrawableWavetableView_Previews: PreviewProvider {
    static var previews: some View {
        DrawableWavetableView(wavetable: sinWavetable)
    }
}

extension Comparable {
    fileprivate func clamped(to limits: ClosedRange<Self>) -> Self {
        return min(max(self, limits.lowerBound), limits.upperBound)
    }
}
