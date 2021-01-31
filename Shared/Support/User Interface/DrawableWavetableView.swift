//
//  DrawableWavetableView.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

class WavetableController: ObservableObject {
    @Published var wavetable: Wavetable

    init(wavetable: Wavetable) {
        self.wavetable = wavetable
    }

    convenience init(defaultWavetable: DefaultWavetables = .sine) {
        self.init(wavetable: defaultWavetable.wavetable)
    }

    func restoreDefault(_ defaultWavetable: DefaultWavetables) {
        wavetable = defaultWavetable.wavetable
    }
}

struct DrawableWavetableView: View {
    @ObservedObject var wavetableController = WavetableController()

    private func drawAt(_ x: Int, _ y: Int) {
        guard (0...31).contains(x) && (0...15).contains(y) else {
            return
        }
        wavetableController.wavetable[x] = UInt8(y)
    }

    var body: some View {
        GeometryReader { geometry in
            let sqWidth = geometry.size.width / 32
            let sqHeight = geometry.size.height / 16
            Path { path in
                for (i, v) in wavetableController.wavetable.enumerated() {
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
                            let y = Int(drag.location.y / sqHeight)
                            drawAt(x, y)
                        })
        }
    }
}

struct WavetableView: View {
    @ObservedObject var wavetableController: WavetableController
    @State var selected: DefaultWavetables

    init(initialSelection: DefaultWavetables = .sine) {
        self.wavetableController = WavetableController(defaultWavetable: initialSelection)
        self._selected = State(initialValue: initialSelection)
    }

    var body: some View {
        VStack {
            Picker("Presets", selection: $selected) {
                ForEach(DefaultWavetables.allCases) { wt in
                    Text(wt.rawValue).tag(wt)
                }
            }.pickerStyle(SegmentedPickerStyle())
            DrawableWavetableView(wavetableController: wavetableController)
        }.onChange(of: selected, perform: { value in
            wavetableController.restoreDefault(value)
        })
    }
}

struct WavetableView_Previews: PreviewProvider {
    static var previews: some View {
        WavetableView()
    }
}
