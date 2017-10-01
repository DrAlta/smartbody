//
//  SBViewController.swift
//  Smartbody
//
//  Created by Anton Leuski on 9/23/17.
//  Copyright © 2017 Smartbody Project. All rights reserved.
//

import UIKit
import GLKit
import AVFoundation
 
open class SBViewController: GLKViewController, GLKViewControllerDelegate {
  private var _reloadTexture = true
  private var _time: TimeInterval = 0
  private var _lastSize = CGSize()
  private var _audioWasPlaying = false
  
  open var context: SBContext?
  
  open func makeOpenGLContext() -> EAGLContext? {
    return EAGLContext.init(api: .openGLES3)
  }
  
  open func setupOpenGL() {
    
  }

  open func teardownOpenGL() {
    
  }

  open override func viewDidLoad() {
    super.viewDidLoad()
    self.delegate = self
    // Do any additional setup after loading the view, typically from a nib.
    isPaused = false
    
    guard let glview = view as? GLKView else {
      fatalError("view must be a subclass of GLKView")
    }
    
    glview.delegate = self;
    glview.drawableColorFormat = .RGBA8888
    glview.drawableDepthFormat = .format16
    
    guard let context = makeOpenGLContext() else {
      fatalError("failed to initialize OpenGL context")
    }

    glview.context = context
    EAGLContext.setCurrent(context)
    setupOpenGL()
  }
  
  deinit {
    if let context = (self.view as? GLKView)?.context {
      EAGLContext.setCurrent(context)
      teardownOpenGL()
    }
  }
  
  open override func didReceiveMemoryWarning() {
    super.didReceiveMemoryWarning()
    // Dispose of any resources that can be recreated.
  }
  
  open override func glkView(_ view: GLKView, drawIn rect: CGRect) {
    let size = CGSize(width: view.drawableWidth, height: view.drawableHeight)
    if _lastSize != size {
      _lastSize = size
      context?.setupDrawing(size: size)
    }
    if _reloadTexture {
      context?.reloadTexture()
      _reloadTexture = false
    }
    context?.drawFrame()
  }
  
  open override func viewDidLayoutSubviews() {
    super.viewDidLayoutSubviews()
  }
  
  open func glkViewController(_ controller: GLKViewController, willPause pause: Bool)
  {
    if pause {
      _time += timeSinceLastResume
      _audioWasPlaying = context?.audioPlayer?.isPlaying ?? false
      context?.audioPlayer?.pause()
    } else {
      if _audioWasPlaying {
        context?.audioPlayer?.play()
        _audioWasPlaying = false
      }
    }
  }
  
  open func glkViewControllerUpdate(_ controller: GLKViewController) {
    context?.update(_time+timeSinceLastResume)
  }
}

