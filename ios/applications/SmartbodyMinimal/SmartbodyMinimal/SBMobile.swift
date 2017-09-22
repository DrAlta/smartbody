//
//  SBMobile.swift
//  SmartbodyMinimal
//
//  Created by Anton Leuski on 9/10/17.
//

import Foundation
import CoreGraphics

class SBMobile {
  public static var assetsURL =
    Bundle.main.url(forResource: "assets", withExtension: nil)
  
  public static let shared = SBMobile(assetsURL: assetsURL)
  
  public init(assetsURL: URL?) {
    guard let assetsURL = assetsURL else {
      fatalError("assets url must be defined")
    }
    
    assetsURL.withUnsafeFileSystemRepresentation {
      SBIOSInitialize($0, { (cString) in
        if let cString = cString {
          let text = String(cString: cString)
          //          if text.contains("lib/python2.7") {
          //            raise(SIGINT)
          //          }
          Swift.print(">>> \(text)")
        }
      })
    }
  }
  
  public func setupDrawing(size: CGSize) {
    SBIOSSetupDrawing(Int32(size.width), Int32(size.height))
  }
  
  public func drawFrame() {
    SBIOSDrawFrame()
  }
  
  public func reloadTexture() {
    SBIOSReloadTexture()
  }
  
  public func update(_ time: TimeInterval) {
    SBUpdate(Float(time))
  }
  
  public func execute(command: String) {
    SBExecuteCmd(command)
  }
  
  public func executePython(command: String) {
    SBExecutePythonCmd(command)
  }
  
  public func cameraOperation(dx: Float, dy: Float, mode: Int) {
    SBCameraOperation(dx, dy, Int32(mode))
  }
  
  public func int(forKey key: String) -> Int {
    return Int(SBGetIntAttribute(key))
  }
  
  public func string(forKey key: String) -> String {
    return String(cString: SBGetStringAttribute(key))
  }
  
  public func bool(forKey key: String) -> Bool {
    return SBGetBoolAttribute(key) != 0
  }
  
  public func double(forKey key: String) -> Double {
    return SBGetDoubleAttribute(key)
  }

}
