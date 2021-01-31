//
//  MainViewController.swift
//  GameBoyAudioSynth iOS
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import Cocoa
import GameBoyAudioSynthFramework

class MainViewController: NSViewController {

    let audioUnitManager = AudioUnitManager()

//    init() {
//        super.init(nibName: nil, bundle: nil)
//    }

//    public override init(nibName: NSNib.Name?, bundle: Bundle?) {
//        // Pass a reference to the owning framework bundle
//        super.init(nibName: nibName, bundle: Bundle(for: type(of: self)))
//    }

//    required init?(coder: NSCoder) {
//        fatalError("init(coder:) has not been implemented")
//    }

    override func viewDidLoad() {
        super.viewDidLoad()
        embedPlugInView()
        populatePresetMenu()
//        audioUnitManager.delegate = self
    }

    override func viewWillAppear() {
        super.viewWillAppear()
        view.window?.delegate = self
    }

    override func loadView() {
      self.view = NSView()
    }

    private func embedPlugInView() {
        guard let controller = audioUnitManager.viewController else {
            fatalError("Could not load audio unit's view controller.")
        }

        // Present the view controller's view.
        addChild(controller)
        view.addSubview(controller.view)
        controller.view.pinToSuperviewEdges()
    }

    private func populatePresetMenu() {
        guard let presetMenu = NSApplication.shared.mainMenu?.item(withTag: 666)?.submenu else { return }
        for preset in audioUnitManager.presets {
            let menuItem = NSMenuItem(title: preset.name,
                                      action: #selector(handleMenuSelection(_:)),
                                      keyEquivalent: "\(preset.number + 1)")
            menuItem.tag = preset.number
            presetMenu.addItem(menuItem)
        }

        if let currentPreset = audioUnitManager.currentPreset {
            presetMenu.item(at: currentPreset.number)?.state = .on
        }
    }

    @objc
    private func handleMenuSelection(_ sender: NSMenuItem) {
        sender.menu?.items.forEach { $0.state = .off }
        sender.state = .on
        audioUnitManager.currentPreset = audioUnitManager.presets[sender.tag]
    }
}

extension MainViewController: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {
        audioUnitManager.cleanup()
    }
}

//extension MainViewController: AUManagerDelegate {
//
//    func cutoffValueDidChange(_ value: Float) {
//
//        // Normalize the vaue from 0-1
//        var normalizedValue = (value - defaultMinHertz) / (defaultMaxHertz - defaultMinHertz)
//
//        // Map to 2^0 - 2^9 (slider range)
//        normalizedValue = (normalizedValue * 511.0) + 1
//
//        cutoffSlider.floatValue = Float(logValueForNumber(normalizedValue))
//        cutoffTextField.text = String(format: "%.f", value)
//    }
//
//    func resonanceValueDidChange(_ value: Float) {
//        resonanceSlider.floatValue = value
//        resonanceTextField.text = String(format: "%.2f", value)
//    }
//}

