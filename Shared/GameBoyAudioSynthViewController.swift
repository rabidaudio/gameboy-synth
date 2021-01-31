//
//  GameBoyAudioSynthViewController.swift
//  GameBoyAudioSynth
//
//  Created by Charles Julian Knight on 1/31/21.
//  Copyright © 2021 Charles Julian Knight. All rights reserved.
//

import CoreAudioKit
import SwiftUI

#if os(iOS)
import UIKit
#elseif os(macOS)
import AppKit
#endif

public class GameBoyAudioSynthViewController: AUViewController {

    let compact = AUAudioUnitViewConfiguration(width: 400, height: 100, hostHasController: false)
    let expanded = AUAudioUnitViewConfiguration(width: 800, height: 500, hostHasController: false)

    private var viewConfig: AUAudioUnitViewConfiguration!

    public var viewConfigurations: [AUAudioUnitViewConfiguration] {
        // width: 0 height:0  is always supported, should be the default, largest view.
        return [expanded, compact]
    }

    public var audioUnit: GameBoyAudioSynth? {
        didSet {
            audioUnit?.viewController = self
        }
    }

    let state = SynthState()

    #if os(macOS)
    public override init(nibName: NSNib.Name?, bundle: Bundle?) {
        // Pass a reference to the owning framework bundle
        super.init(nibName: nibName, bundle: Bundle(for: type(of: self)))
    }
    #endif

    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }

    // MARK: Lifecycle

    public override func loadView() {
      self.view = NSView()
    }

    public override func viewDidLoad() {
        super.viewDidLoad()
        
        preferredContentSize = CGSize(width: 800, height: 500)

        let contentView: some View = MainView(state: state)
        let hostingController = _HostingController(rootView: contentView)

        #if os(iOS)
        // Present the view controller's view.
        if let view = controller.view {
            addChild(controller)
            view.frame = containerView.bounds
            containerView.addSubview(view)
            view.pinToSuperviewEdges()
            controller.didMove(toParent: self)
        }
        #elseif os(macOS)
        // Present the view controller's view.
        addChild(hostingController)
        view.addSubview(hostingController.view)
        hostingController.view.pinToSuperviewEdges()
        #endif

        // Set the default view configuration.
        viewConfig = expanded
    }

    // MARK: View Configuration Selection

    // TODO: this is from the sample code, but I'm not sure what it's doing or if it
    // will be necessary
    public func toggleViewConfiguration() {
        // Let the audio unit call selectViewConfiguration instead of calling
        // it directly to ensure validate the audio unit's behavior.
        audioUnit?.select(viewConfig == expanded ? compact : expanded)
    }

    func selectViewConfiguration(_ viewConfig: AUAudioUnitViewConfiguration) {
        // If requested configuration is already active, do nothing
        guard self.viewConfig != viewConfig else { return }

        self.viewConfig = viewConfig

//        let isDefault = viewConfig.width >= expanded.width &&
//                        viewConfig.height >= expanded.height
//        let fromView = isDefault ? compactView : expandedView
//        let toView = isDefault ? expandedView : compactView
//
//        performOnMain {
//            #if os(iOS)
//            UIView.transition(from: fromView!,
//                              to: toView!,
//                              duration: 0.2,
//                              options: [.transitionCrossDissolve, .layoutSubviews])
//
//            if toView == self.expandedView {
//                toView?.pinToSuperviewEdges()
//            }
//
//            #elseif os(macOS)
//            self.view.addSubview(toView!)
//            fromView!.removeFromSuperview()
//            toView!.pinToSuperviewEdges()
//            #endif
//        }
    }

    private func performOnMain(_ operation: @escaping () -> Void) {
        if Thread.isMainThread {
            operation()
        } else {
            DispatchQueue.main.async {
                operation()
            }
        }
    }
}
