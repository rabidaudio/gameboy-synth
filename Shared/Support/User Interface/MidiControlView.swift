//
//  MidiControlView.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import SwiftUI

struct MidiConfig {
    var enabled: Bool = false
    @BoundedTo(1...4) var voice: Int = 1
    @BoundedTo(1...2) var channel: Int = 1
    @BoundedTo(-48...48) var transpose: Int = 0
}

struct MidiControlView: View {
    @State var config: MidiConfig

    var body: some View {
        HStack(content: {
            Button(action: {
                config.enabled = !config.enabled
            }, label: {
                Image(systemName: "power")
                    .foregroundColor(config.enabled ? .blue : .black)
            }).padding()
            .shadow(radius: config.enabled ? 10 : 0)
            Spacer()
            VStack(content: {
                Picker("Voice", selection: $config.voice) {
                    ForEach(Array(1...4), id: \.self) { i in
                        Text("\(i)")
                    }
                }
                Picker("Channel", selection: $config.channel) {
                    ForEach(Array(1...2), id: \.self) { i in
                        Text("\(i)")
                    }
                }
            })
            Spacer()
            Stepper("Transpose: \(config.transpose)", value: $config.transpose)
        })
    }
}

struct PMidiControlViewMidiControlView_Previews: PreviewProvider {
    static var previews: some View {
        MidiControlView(config: MidiConfig())
        MidiControlView(config: MidiConfig(enabled: true))
    }
}
