/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The app's delegate object.
*/

import Cocoa
import GameBoyAudioSynthFramework

class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow!

//    override init() {
//        super.init()
//    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        let controller = MainViewController()
//        window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 600, height: 400), styleMask: .titled, backing: .buffered, defer: false)
        window = NSWindow(contentViewController: controller)

//        controller.view.frame = NSRect(x: 0, y: 0, width: 600, height: 400)
//        window.contentView?.addSubview(controller.view)
         window.makeKeyAndOrderFront(nil)
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }
}

class Application: NSApplication {

    let strongDelegate = AppDelegate()

    override init() {
        super.init()
        self.delegate = strongDelegate
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

}

