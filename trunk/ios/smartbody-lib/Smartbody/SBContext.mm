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

@interface SBContext () {
  SceneListener _sceneListener;
  ESContext esContext;
	int curH, curW;
}
@property (nonatomic, strong) AVAudioPlayer* audioPlayer;
- (void)playSoundFromFileAtPath:(NSString*)path loop:(BOOL)loop;
- (void)stopSound;
@end

static SBContext *sharedInstance = nil;

void SceneListener::OnLogMessage( const std::string & message ) {
  id<SBContextDelegate> del = sharedInstance.delegate;
  if (del) {
    [del log:[NSString stringWithUTF8String: message.c_str()]];
  }
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
  esContext.width = size.width;
  esContext.height = size.height;
  SBSetupDrawing(size.width,size.height, &esContext);
  curW = size.width;
  curH = size.height;
}

- (void)drawFrame
{
  SrMat identity;
  //SBDrawFrame(VHEngine::curW, VHEngine::curH, id);
  SBDrawFrame_ES20(curW, curH, &esContext, identity);
}

- (void)reloadTexture
{
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
  self.audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path] error:nil];
  if (!self.audioPlayer) return;
  self.audioPlayer.numberOfLoops = loop ? -1 : 0;
  [self.audioPlayer play];
}

- (void)stopSound
{
  [self.audioPlayer stop];
  self.audioPlayer = nil;
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

