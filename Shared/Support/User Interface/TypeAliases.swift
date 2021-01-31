/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Type alias mapping to normalize AppKit and UIKit interfaces to support cross-platform code reuse.
*/

#if os(iOS)
import UIKit
public typealias _Color = UIColor
public typealias Font = UIFont

public typealias Storyboard = UIStoryboard

public typealias _View = UIView
public typealias _TextField = UITextField
public typealias _Label = UILabel
public typealias _Button = UIButton
public typealias _Slider = UISlider

#elseif os(macOS)
import AppKit
public typealias _Color = NSColor
public typealias Font = NSFont

public typealias Storyboard = NSStoryboard

public typealias _View = NSView
public typealias _TextField = NSTextField
public typealias _Label = NSTextField
public typealias _Button = NSButton
public typealias _Slider = NSSlider

public var tintColor: NSColor! = NSColor.controlAccentColor.usingColorSpace(.deviceRGB)

#endif
