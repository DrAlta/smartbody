//
//  SBContext.m
//  Smartbody
//
//  Created by Anton Leuski on 9/24/17.
//  Copyright © 2017 Smartbody Project. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wcomma"

#include <sb/SBScene.h>
#include <sb/SBPawn.h>
#include <sb/SBSceneListener.h>
#include <sbm/GPU/SbmTexture.h>

#import "SBMobile.h"
#import "SBWrapper.h"

#pragma clang diagnostic pop

#import "SBContext.h"

struct SceneListener : public SmartBody::SBSceneListener {
  void OnLogMessage( const std::string & message ) override;
  SBContext * _context;
};

@interface SBContext () <AVAudioPlayerDelegate> {
  SceneListener _sceneListener;
  ESContext esContext;
  CGSize lastSize;
  BOOL reloadTexture;
}
- (void)playSoundFromFileAtPath:(NSString*)path loop:(BOOL)loop;
- (void)stopSound;
@end

static SBContext *sharedInstance = nil;

void SceneListener::OnLogMessage( const std::string & message ) {
  id<SBContextDelegate> del = sharedInstance.delegate;
  if (del) {
    [del context:sharedInstance
             log:[NSString stringWithUTF8String: message.c_str()]];
  }
}

@interface SBPythonObject () {
  boost::python::object object;
}

@end

@implementation SBPythonObject
- (nonnull instancetype)initWith:(const boost::python::object&)object
{
  if (self = [super init]) {
    self->object = object;
  }
  return self;
}

- (NSNumber* _Nullable)intValue {
#ifndef SB_NO_PYTHON
  try {
    return [NSNumber numberWithInteger:boost::python::extract<int>(self->object)];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

- (NSNumber* _Nullable)boolValue {
#ifndef SB_NO_PYTHON
  try {
    return [NSNumber numberWithBool:boost::python::extract<bool>(self->object)];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

- (NSNumber* _Nullable)floatValue {
#ifndef SB_NO_PYTHON
  try {
    return [NSNumber numberWithFloat:boost::python::extract<float>(self->object)];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

- (NSString* _Nullable)stringValue {
#ifndef SB_NO_PYTHON
  try {
    std::string result = boost::python::extract<std::string>(self->object);
    return [NSString stringWithUTF8String:result.c_str()];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

@end

@implementation SBContext

- (nonnull instancetype)initWithAssetsURL:(NSURL * _Nonnull)assetsURL
{
  return [self initWithAssetsURL:assetsURL delegate:nil];
}

- (nonnull instancetype)initWithAssetsURL:(NSURL * _Nonnull)assetsURL
                                 delegate:(id<SBContextDelegate> _Nullable)delegate
{
  static dispatch_once_t onceToken;
  if (self = [super init]) {
    dispatch_once(&onceToken, ^{
      self->reloadTexture = true;
      self.delegate = delegate;
      
      SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
      _sceneListener._context = self;
      scene->addSceneListener(&_sceneListener);
      
      SmartBody::util::log("Start running SBIOSInitialize");
      std::string path1 = [assetsURL.path UTF8String];
      path1 += "/";
      SBSetup(path1.c_str(), "setup.py");
      SBInitialize();
      initSBMobilePythonModule(); // initialize Python APIs for SBMobile
      
      scene->addAssetPath("script", "scripts");
      SBInitScene("init.py");
      SmartBody::util::log("After running SBIOSInitialize");
      sharedInstance = self;
    });
  }
  return sharedInstance;
}

- (void)setupDrawingWithSize:(CGSize)size
{
  if (CGSizeEqualToSize(lastSize, size)) { return; }
  lastSize = size;
  esContext.width = size.width;
  esContext.height = size.height;
  SBSetupDrawing(size.width,size.height, &esContext);
}

- (void)drawFrame:(CGSize)size
{
  [self setupDrawingWithSize:size];
  SrMat identity;
  //SBDrawFrame(VHEngine::curW, VHEngine::curH, id);
  SBDrawFrame_ES20((int)size.width, (int)size.height,
                   &esContext, identity);
}

- (void)drawFrame:(CGSize)size
  modelViewMatrix:(matrix_float4x4)modelViewMatrix
 projectionMatrix:(matrix_float4x4)projectionMatrix
     gazeAtCamera:(BOOL)gazeAtCamera
{
  [self setupDrawingWithSize:size];
  // Warning: we are relying on the storage models to be the same.
  SrMat modelView((const float*)&modelViewMatrix);
  SrMat projection((const float*)&projectionMatrix);
  
  SBDrawFrameAR((int)size.width, (int)size.height,
                &esContext, modelView, projection);
  
  if (!gazeAtCamera) { return; }
  
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  if (!scene) { return; }
  SmartBody::SBPawn* gazeTarget = scene->getPawn("None");
  
  SrMat invModelView = modelView.inverse();
  SrVec newGazePos = invModelView.get_translation();
  gazeTarget->setPosition(newGazePos);
}

- (void)reloadTexture
{
  if (!self->reloadTexture) { return; }
  self->reloadTexture = false;
  SBInitGraphics(&esContext);
  SbmTextureManager& texm = SbmTextureManager::singleton();
  texm.reloadTexture();
}

- (void)update:(NSTimeInterval)time
{
  SBUpdate(time);
}

- (void)executeWithCommand:(NSString * _Nonnull)command
{
  SBExecuteCmd([command UTF8String]);
}

- (void)executePythonWithCommand:(NSString * _Nonnull)command
{
  SBExecutePythonCmd([command UTF8String]);
}

- (SBPythonObject* _Nullable)returnValueFromPythonCommand:(NSString * _Nonnull)command
{
#ifndef SB_NO_PYTHON
  try
  {
    boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
    return [[SBPythonObject alloc] initWith:boost::python::eval([command UTF8String], mainDict)];
  }
  catch (...)
  {
    PyErr_Print();
  }
#endif
  return nil;
}

- (void)cameraOperationWithDx:(float)dx dy:(float)dy mode:(NSInteger)mode
{
  SBCameraOperation(dx, dy, (int)mode);
}

- (NSInteger)intForKey:(NSString * _Nonnull)key
{
  return SmartBody::SBScene::getScene()->getIntAttribute([key UTF8String]);
}

- (NSString * _Nonnull)stringForKey:(NSString * _Nonnull)key
{
  return [NSString stringWithUTF8String: SmartBody::SBScene::getScene()->getStringAttribute([key UTF8String]).c_str()];
}

- (BOOL)boolForKey:(NSString * _Nonnull)key
{
  return SmartBody::SBScene::getScene()->getBoolAttribute([key UTF8String]) != 0;
}

- (double)doubleForKey:(NSString * _Nonnull)key
{
  return SmartBody::SBScene::getScene()->getDoubleAttribute([key UTF8String]);
}

- (void)playSoundFromFileAtPath:(NSString*)path loop:(BOOL)loop
{
  [self stopSound];
  NSURL* url = [NSURL fileURLWithPath:path];
  self.audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
  if (!self.audioPlayer) return;
  self.audioPlayer.delegate = self;
  self.audioPlayer.numberOfLoops = loop ? -1 : 0;
  [self.audioPlayer play];
  [self.delegate context:self didStartPlayingAudioAtURL:url];
}

- (void)stopSound
{
  [self.audioPlayer stop];
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
  NSURL* url = player.url;
  if (!url) return;
  [self.delegate context:self didFinishPlayingAudioAtURL:url successfully:flag];
}

@end

void SBMobile::playVideo(std::string videoViewName, std::string videoFilePath, bool looping)
{
}

void SBMobile::stopVideo(std::string videoViewName)
{
}

void SBMobile::playSound(std::string soundFilePath, bool looping)
{
  [sharedInstance playSoundFromFileAtPath:[NSString stringWithUTF8String:soundFilePath.c_str()] loop:looping];
}

void SBMobile::stopSound()
{
  [sharedInstance stopSound];
}

