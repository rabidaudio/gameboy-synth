//
//  DrawableWavetableView.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

struct DrawableWavetableView: View {
    @State private var storedWavetable: Wavetable?
    @Binding private var initialWavetable: DefaultWavetables

    private var wavetable: Wavetable {
        get { return storedWavetable ?? initialWavetable.wavetable }
        set { storedWavetable = newValue }
    }

    init(externallySwitchableDefaultWavetable wt: Binding<DefaultWavetables>) {
        self._initialWavetable = wt
        self._storedWavetable = State(initialValue: nil)
    }

    init(initialWavetable wt: Wavetable = DefaultWavetables.sine.wavetable) {
        self._storedWavetable = State(initialValue: wt)
        self._initialWavetable = .constant(.sine) // doesn't matter
    }

    private func drawAt(_ x: Int, _ y: Int) {
        guard (0...31).contains(x) && (0...15).contains(y) else {
            return
        }
        if storedWavetable == nil {
            storedWavetable = initialWavetable.wavetable
        }
        storedWavetable![x] =  UInt8(y)
    }

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
                            drawAt(x, y)
                        })
        }
    }
}

struct WavetableView: View {
    @State var selected: DefaultWavetables = .triangle

    var body: some View {
        VStack {
            Picker("Presets", selection: $selected) {
                ForEach(DefaultWavetables.allCases) { wt in
                    Text(wt.rawValue).tag(wt)
                }
            }.pickerStyle(SegmentedPickerStyle())
            DrawableWavetableView(externallySwitchableDefaultWavetable: $selected)
        }
    }
}

struct WavetableView_Previews: PreviewProvider {
    static var previews: some View {
        WavetableView()
    }
}

extension Comparable {
    fileprivate func clamped(to limits: ClosedRange<Self>) -> Self {
        return min(max(self, limits.lowerBound), limits.upperBound)
    }
}
