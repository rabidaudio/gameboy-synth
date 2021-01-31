/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Non-UI controller object used to manage the interaction with the GameBoyAudioSynthDemo audio unit.
*/

import Foundation
import AudioToolbox
import CoreAudioKit
import AVFoundation

// A simple wrapper type to prevent exposing the Core Audio AUAudioUnitPreset in the UI layer.
public struct Preset {
    fileprivate init(preset: AUAudioUnitPreset) {
        audioUnitPreset = preset
    }
    fileprivate let audioUnitPreset: AUAudioUnitPreset
    public var number: Int { return audioUnitPreset.number }
    public var name: String { return audioUnitPreset.name }
}

// Delegate protocol to be adopted to be notified of parameter value changes.
public protocol AUManagerDelegate: AnyObject {
    func cutoffValueDidChange(_ value: Float)
    func resonanceValueDidChange(_ value: Float)
}

// Controller object used to manage the interaction with the audio unit and its user interface.
public class AudioUnitManager {

    /// The user-selected audio unit.
    private var audioUnit: GameBoyAudioSynth?

    public weak var delegate: AUManagerDelegate? {
        didSet {
            updateCutoff()
            updateResonance()
        }
    }

    public private(set) var viewController: GameBoyAudioSynthViewController!

    public var cutoffValue: Float = 0.0 {
        didSet {
            cutoffParameter.value = cutoffValue
        }
    }

    public var resonanceValue: Float = 0.0 {
        didSet {
            resonanceParameter.value = resonanceValue
        }
    }

    // Gets the audio unit's defined presets.
    public var presets: [Preset] {
        guard let audioUnitPresets = audioUnit?.factoryPresets else {
            return []
        }
        return audioUnitPresets.map { preset -> Preset in
            return Preset(preset: preset)
        }
    }

    // Retrieves or sets the audio unit's current preset.
    public var currentPreset: Preset? {
        get {
            guard let preset = audioUnit?.currentPreset else { return nil }
            return Preset(preset: preset)
        }
        set {
            audioUnit?.currentPreset = newValue?.audioUnitPreset
        }
    }

    /// The playback engine used to play audio.
    private let playEngine = SimplePlayEngine()

    // The audio unit's filter cutoff frequency parameter object.
    private var cutoffParameter: AUParameter!

    // The audio unit's filter resonance parameter object.
    private var resonanceParameter: AUParameter!

    // A token for our registration to observe parameter value changes.
    private var parameterObserverToken: AUParameterObserverToken!

    // The AudioComponentDescription matching the GameBoyAudioSynthExtension Info.plist
    private var componentDescription: AudioComponentDescription = {

        // Ensure that AudioUnit type, subtype, and manufacturer match the extension's Info.plist values
        var componentDescription = AudioComponentDescription()
        componentDescription.componentType = kAudioUnitType_MusicDevice // kAudioUnitType_Effect
        componentDescription.componentSubType = 0x47424153 /*'GBAS'*/
        componentDescription.componentManufacturer = 0x5F434A4B /*'_CJK'*/
        componentDescription.componentFlags = 0
        componentDescription.componentFlagsMask = 0

        return componentDescription
    }()

    private let componentName = "GameBoyAudioSynth"
    private let version: UInt32 = 1

    public init() {
        viewController = loadViewController()

        AUAudioUnit.registerSubclass(GameBoyAudioSynth.self,
                                     as: componentDescription,
                                     name: componentName,
                                     version: version)

        AVAudioUnit.instantiate(with: componentDescription) { audioUnit, error in
            guard error == nil, let audioUnit = audioUnit else {
                fatalError("Could not instantiate audio unit: \(String(describing: error))")
            }
            self.audioUnit = audioUnit.auAudioUnit as? GameBoyAudioSynth
            self.connectParametersToControls()
            self.playEngine.connect(avAudioUnit: audioUnit)
        }
    }

    // Loads the audio unit's view controller from the extension bundle.
    private func loadViewController() -> GameBoyAudioSynthViewController {
        // Locate the app extension's bundle in the main app's PlugIns directory
        guard let url = Bundle.main.builtInPlugInsURL?.appendingPathComponent("GameBoyAudioSynthExtension.appex"),
            let appexBundle = Bundle(url: url) else {
                fatalError("Could not find app extension bundle URL.")
        }

        #if os(iOS)
        // TODO: initalize
//        let storyboard = Storyboard(name: "MainInterface", bundle: appexBundle)
//        guard let controller = storyboard.instantiateInitialViewController() as? GameBoyAudioSynthDemoViewController else {
//            fatalError("Unable to instantiate GameBoyAudioSynthDemoViewController")
//        }
//        return controller
        #elseif os(macOS)
        return GameBoyAudioSynthViewController(nibName: nil, bundle: appexBundle)
        #endif
    }

    /**
     Called after instantiating our audio unit, to find the AU's parameters and
     connect them to our controls.
     */
    private func connectParametersToControls() {

        guard let audioUnit = audioUnit else {
            fatalError("Couldn't locate GameBoyAudioSynth")
        }

        viewController.audioUnit = audioUnit

        // Find our parameters by their identifiers.
        guard let parameterTree = audioUnit.parameterTree else {
            fatalError("GameBoyAudioSynth does not define any parameters.")
        }

        cutoffParameter = parameterTree.value(forKey: "cutoff") as? AUParameter
        resonanceParameter = parameterTree.value(forKey: "resonance") as? AUParameter

        parameterObserverToken = parameterTree.token(byAddingParameterObserver: { [weak self] address, _ in
            guard let self = self else { return }
            /*
             This is called when one of the parameter values changes.
             We can only update UI from the main queue.
             */
            DispatchQueue.main.async {
                if address == self.cutoffParameter.address {
                    self.updateCutoff()
                } else if address == self.resonanceParameter.address {
                    self.updateResonance()
                }
            }
        })
    }

    // Callbacks to update controls from parameters.
    func updateCutoff() {
        guard let param = cutoffParameter else { return }
        delegate?.cutoffValueDidChange(param.value)
    }

    func updateResonance() {
        guard let param = resonanceParameter else { return }
        delegate?.resonanceValueDidChange(param.value)
    }

    @discardableResult
    public func togglePlayback() -> Bool {
        return playEngine.togglePlay()
    }

    public func toggleView() {
        viewController.toggleViewConfiguration()
    }

    public func cleanup() {
        playEngine.stopPlaying()

        guard let parameterTree = audioUnit?.parameterTree else { return }
        parameterTree.removeParameterObserver(parameterObserverToken)
    }
}
