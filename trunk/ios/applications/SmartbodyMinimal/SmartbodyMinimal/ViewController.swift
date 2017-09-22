//
//  ViewController.swift
//  SmartbodyMinimal
//
//  Created by Anton Leuski on 9/9/17.
//

import UIKit
import GLKit

class ViewController: GLKViewController, GLKViewControllerDelegate {
  private var _reloadTexture = true
  private var _time: TimeInterval = 0

  override func viewDidLoad() {
    super.viewDidLoad()
    self.delegate = self
    // Do any additional setup after loading the view, typically from a nib.
    isPaused = false

    guard let glview = view as? GLKView else {
      fatalError("view must be a subclass of GLKView")
    }

    guard let context = EAGLContext.init(api: .openGLES3) else {
      fatalError("failed to initialize OpenGL context")
    }
    EAGLContext.setCurrent(context)

    glview.context = context

    SBMobile.shared.setupDrawing(size: view.frame.size)
  }

  override func didReceiveMemoryWarning() {
    super.didReceiveMemoryWarning()
    // Dispose of any resources that can be recreated.
  }

  override func glkView(_ view: GLKView, drawIn rect: CGRect) {
    if _reloadTexture {
      SBMobile.shared.reloadTexture()
      _reloadTexture = false
    }
    SBMobile.shared.drawFrame()
  }
  
  override func viewDidLayoutSubviews() {
    super.viewDidLayoutSubviews()
  }
  
  func glkViewController(_ controller: GLKViewController, willPause pause: Bool)
  {
    if pause {
      _time += timeSinceLastResume
    }
  }
  
  func glkViewControllerUpdate(_ controller: GLKViewController) {
    SBMobile.shared.update(_time+timeSinceLastResume)
  }
}

class SceneController: ViewController, UITextFieldDelegate {
  
  override func viewDidLoad() {
    super.viewDidLoad()
    
    let pinch = UIPinchGestureRecognizer(target: self, action: #selector(SceneController.pinchRecognized(_:)))
    view.addGestureRecognizer(pinch)

    let pan = UIPanGestureRecognizer(target: self, action: #selector(SceneController.panRecognized(_:)))
    pan.minimumNumberOfTouches = 1
    pan.maximumNumberOfTouches = 1
    view.addGestureRecognizer(pan)
  }

  private var _lastScale: CGFloat = 0
  private var _lastRotation = CGPoint()
  
  @objc func pinchRecognized(_ recognizer: UIPinchGestureRecognizer) {
    if recognizer.state == .changed {
      let delta = 100 * (recognizer.scale - _lastScale)
      _lastScale = recognizer.scale
      print("scale \(delta)")
      SBMobile.shared.cameraOperation(dx: Float(delta), dy: Float(delta), mode: 0)
    }
  }

  @objc func panRecognized(_ recognizer: UIPanGestureRecognizer) {
    if recognizer.state == .changed {
      let location = recognizer.translation(in: view)
      let dx = location.x - _lastRotation.x
      let dy = location.y - _lastRotation.y
      _lastRotation = location
      print("rotate \(dx) \(dy)")
      SBMobile.shared.cameraOperation(dx: Float(dx), dy: Float(dy), mode: 1)
    }
  }

  @IBAction func resetCamera(_ sender: Any) {
    SBMobile.shared.cameraOperation(dx: 0, dy: 0, mode: 2)
    SBMobile.shared.executePython(command: "bml.interruptCharacter(\"ChrRachel\",0)")
  }
  
  func textFieldShouldReturn(_ textField: UITextField) -> Bool
  {
    textField.resignFirstResponder()
    return true
  }
  
  @IBAction func commandEditingDidEnd(_ sender: UITextField) {
    guard let text = sender.text else { return }
    print("command is \(text)")
    SBMobile.shared.executePython(command: text)
  }
}
