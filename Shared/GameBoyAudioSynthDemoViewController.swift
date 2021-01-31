/*
See LICENSE folder for this sample’s licensing information.

Abstract:
View controller for the GameBoyAudioSynthDemo audio unit. Manages the interactions between a FilterView and the audio unit's parameters.
*/

import CoreAudioKit
import SwiftUI

#if os(iOS)
import UIKit
#elseif os(macOS)
import AppKit
#endif

public class GameBoyAudioSynthDemoViewController: AUViewController {

    let compact = AUAudioUnitViewConfiguration(width: 400, height: 100, hostHasController: false)
    let expanded = AUAudioUnitViewConfiguration(width: 800, height: 500, hostHasController: false)

    private var viewConfig: AUAudioUnitViewConfiguration!

//    private var cutoffParameter: AUParameter!
//    private var resonanceParameter: AUParameter!
    private var parameterObserverToken: AUParameterObserverToken?
    
    var observer: NSKeyValueObservation?

    var needsConnection = true

//    @IBOutlet var expandedView: _View! {
//        didSet {
//            expandedView.setBorder(color: .black, width: 1)
//        }
//    }
//
//    @IBOutlet var compactView: _View! {
//        didSet {
//            compactView.setBorder(color: .black, width: 1)
//        }
//    }

    private var state: WavetableController = WavetableController()

    public var viewConfigurations: [AUAudioUnitViewConfiguration] {
        // width: 0 height:0  is always supported, should be the default, largest view.
        return [expanded, compact]
    }

    /*
     When this view controller is instantiated within the FilterDemoApp, its
     audio unit is created independently, and passed to the view controller here.
     */
    public var audioUnit: GameBoyAudioSynthDemo? {
        didSet {
            audioUnit?.viewController = self
            /*
             We may be on a dispatch worker queue processing an XPC request at
             this time, and quite possibly the main queue is busy creating the
             view. To be thread-safe, dispatch onto the main queue.

             It's also possible that we are already on the main queue, so to
             protect against deadlock in that case, dispatch asynchronously.
             */
            performOnMain {
                if self.isViewLoaded {
                    self.connectViewToAU()
                }
            }
        }
    }

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

        let contentView: some View = WavetableView(withExternalController: state)
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
        
//        view.addSubview(expandedView)
//        expandedView.pinToSuperviewEdges()

        // Set the default view configuration.
        viewConfig = expanded

        // Respond to changes in the filterView (frequency and/or response changes).
//        filterView.delegate = self

        

        guard audioUnit != nil else { return }

        // Connect the user interface to the AU parameters, if needed.
        connectViewToAU()
    }

    private func connectViewToAU() {
        guard needsConnection, let paramTree = audioUnit?.parameterTree else { return }

//        // Find the cutoff and resonance parameters in the parameter tree.
//        guard let cutoff = paramTree.value(forKey: "cutoff") as? AUParameter,
//            let resonance = paramTree.value(forKey: "resonance") as? AUParameter else {
//                fatalError("Required AU parameters not found.")
//        }
//
//        // Set the instance variables.
//        cutoffParameter = cutoff
//        resonanceParameter = resonance

        // Observe major state changes like a user selecting a user preset.
//        observer = audioUnit?.observe(\.allParameterValues) { object, change in
//            DispatchQueue.main.async {
//                self.updateUI()
//            }
//        }

        // Observe value changes made to the cutoff and resonance parameters.
        parameterObserverToken =
            paramTree.token(byAddingParameterObserver: { [weak self] address, value in
                guard let self = self else { return }

//                // This closure is being called by an arbitrary queue. Ensure
//                // all UI updates are dispatched back to the main thread.
//                if [cutoff.address, resonance.address].contains(address) {
//                    DispatchQueue.main.async {
//                        self.updateUI()
//                    }
//                }
            })

        // Indicate the view and AU are connected
        needsConnection = false

        // Sync UI with parameter state
//        updateUI()
    }

    // MARK: View Configuration Selection

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
