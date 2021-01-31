//
//  main.swift
//  GameBoyAudioSynth iOS
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import AppKit

// https://stackoverflow.com/a/50750540
let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate

_ = NSApplicationMain(CommandLine.argc, CommandLine.unsafeArgv)
