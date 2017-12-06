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

#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPawn.h>
#include <sb/SBScene.h>
#include <sb/SBSceneListener.h>
#include <sbm/GPU/SbmTexture.h>
#include <sr/sr_camera.h>

#import "SBMobile.h"
#import "SBWrapper.h"

#pragma clang diagnostic pop

#import <simd/matrix.h>
#import "SBContext.h"

struct SceneListener : public SmartBody::SBSceneListener {
  void OnLogMessage( const std::string & message ) override;
  __weak SBContext * _context;
};

@interface SBContext () <AVAudioPlayerDelegate> {
  SceneListener _sceneListener;
  ESContext esContext;
  CGSize lastSize;
  BOOL reloadTexture;
}
@end

@interface NSString (Ext)
+ (nonnull instancetype)stringWithCPPString: (const std::string&)string;
@end
  
@implementation NSString (Ext)
+ (nonnull instancetype)stringWithCPPString: (const std::string&)string {
  return [NSString stringWithUTF8String: string.c_str()];
}
@end

static SBContext *sharedInstance = nil;

void SceneListener::OnLogMessage( const std::string & message ) {
  id<SBContextDelegate> del = _context.delegate;
  if (del) {
    [del context:sharedInstance
             log:[NSString stringWithCPPString: message]];
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

- (NSNumber* _Nullable)numberWithInt {
#ifndef SB_NO_PYTHON
  try {
    return [NSNumber numberWithInteger:boost::python::extract<int>(self->object)];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

- (NSNumber* _Nullable)numberWithBool {
#ifndef SB_NO_PYTHON
  try {
    return [NSNumber numberWithBool:boost::python::extract<bool>(self->object)];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

- (NSNumber* _Nullable)numberWithFloat {
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
    return [NSString stringWithCPPString: result];
  } catch (...) {
    PyErr_Print();
  }
#endif
  return nil;
}

@end

static inline simd_float3 vector2vector(const SrVec& vec) {
  simd_float3 vector;
  memcpy(&vector, vec.data(), 3 * sizeof(float));
  return vector;
}

static inline matrix_float4x4 matrix2matrix(const SrMat& mat) {
  matrix_float4x4 matrix;
  memcpy(&matrix, mat.data(), sizeof(float) * 16);
  return matrix;
}

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

- (void)dealloc
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  if (scene) {
    _sceneListener._context = nil;
    scene->removeSceneListener(&_sceneListener);
  }
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

- (matrix_float4x4)modelViewMatrix:(CGSize)size
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  SrCamera& cam = *scene->getActiveCamera();
  
  float aspectRatio = size.width / size.height;
  cam.setAspectRatio(aspectRatio);
  //SmartBody::util::log("First render, aspect ratio = %f", aspectRatio);
  
  SrMat matModelView;
  cam.get_view_mat(matModelView);
  matModelView *= cam.getScale();
  return matrix2matrix(matModelView);
}

- (matrix_float4x4)projectionMatrix:(CGSize)size
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  SrCamera& cam = *scene->getActiveCamera();
  
  float aspectRatio = size.width / size.height;
  cam.setAspectRatio(aspectRatio);
  //SmartBody::util::log("First render, aspect ratio = %f", aspectRatio);
  
  SrMat matPerspective;
  cam.get_perspective_mat(matPerspective);
  return matrix2matrix(matPerspective);
}

- (matrix_float2x3)boundingBoxForCharacter:(NSString*)name {
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  SrBox box = scene->getCharacter(name.UTF8String)->getBoundingBox();
  matrix_float2x3 m;
  m.columns[0].x = box.a.x;
  m.columns[0].y = box.a.y;
  m.columns[0].z = box.a.z;
  m.columns[1].x = box.b.x;
  m.columns[1].y = box.b.y;
  m.columns[1].z = box.b.z;
  return m;
}

- (matrix_float4x4)transformForJoint:(NSString*)joint
                           character:(NSString*)character
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  return matrix2matrix(scene->getCharacter(character.UTF8String)
                       ->getSkeleton()
                       ->getJointByName(joint.UTF8String)
                       ->getMatrixGlobal());
}

- (void)setGazeTarget:(simd_float3)target forKey:(NSString*)name
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  SmartBody::SBPawn* gazeTarget = scene->getPawn(name.UTF8String);
  SrVec newGazePos((const float*)&target);
  gazeTarget->setPosition(newGazePos);
}

- (simd_float3)gazeTargetForKey:(NSString*)name
{
  SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
  SmartBody::SBPawn* gazeTarget = scene->getPawn(name.UTF8String);
  SrVec target = gazeTarget->getPosition();
  return vector2vector(target);
}

- (void)drawFrame:(CGSize)size
  modelViewMatrix:(matrix_float4x4)modelViewMatrix
 projectionMatrix:(matrix_float4x4)projectionMatrix
{
  [self setupDrawingWithSize:size];
  // Warning: we are relying on the storage models to be the same.
  SrMat modelView((const float*)&modelViewMatrix);
  SrMat projection((const float*)&projectionMatrix);
  
  SBDrawFrameAR((int)size.width, (int)size.height,
                &esContext, modelView, projection);
}

- (void)drawFrame:(CGSize)size
  modelViewMatrix:(matrix_float4x4)modelViewMatrix
 projectionMatrix:(matrix_float4x4)projectionMatrix
     gazeAtCamera:(BOOL)gazeAtCamera
{
  [self drawFrame:size
  modelViewMatrix:modelViewMatrix
 projectionMatrix:projectionMatrix];
  
  if (!gazeAtCamera) { return; }
  
  matrix_float4x4 invModelView = simd_inverse(modelViewMatrix);
  simd_float3 target;
  memcpy(&target, ((const float*)&invModelView)+12, 3 * sizeof(float));
  [self setGazeTarget:target forKey:@"None"];
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

- (void)executePythonCommand:(NSString * _Nonnull)command
{
  SBExecutePythonCmd([command UTF8String]);
}

- (SBPythonObject* _Nullable)evaluatePythonCommand:(NSString * _Nonnull)command
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
  return [NSString stringWithCPPString: SmartBody::SBScene::getScene()
          ->getStringAttribute([key UTF8String])];
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
  [self.delegate contextWillStartPlayingAudio:self];
  [self.audioPlayer play];
  [self.delegate contextDidStartPlayingAudio:self];
}

- (void)stopSound
{
  [self.audioPlayer stop];
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
  NSURL* url = player.url;
  if (!url) return;
  [self.delegate context:self didFinishPlayingAudioSuccessfully:flag];
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
  [sharedInstance
   playSoundFromFileAtPath: [NSString stringWithCPPString: soundFilePath]
                      loop: looping];
}

void SBMobile::stopSound()
{
  [sharedInstance stopSound];
}

