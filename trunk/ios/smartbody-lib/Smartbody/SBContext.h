//
//  SBContext.h
//  Smartbody
//
//  Created by Anton Leuski on 9/24/17.
//  Copyright © 2017 Smartbody Project. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#if !defined(SWIFT_WARN_UNUSED_RESULT)
# if __has_attribute(warn_unused_result)
#  define SWIFT_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
# else
#  define SWIFT_WARN_UNUSED_RESULT
# endif
#endif

#if !defined(SWIFT_UNAVAILABLE)
# define SWIFT_UNAVAILABLE __attribute__((unavailable))
#endif

#if !defined(OBJC_DESIGNATED_INITIALIZER)
# if __has_attribute(objc_designated_initializer)
#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
# else
#  define OBJC_DESIGNATED_INITIALIZER
# endif
#endif

@protocol SBContextDelegate <NSObject>
- (void)log:(NSString * _Nonnull)message;
@end

@class AVAudioPlayer;

// make it final
__attribute__((objc_subclassing_restricted))
@interface SBContext : NSObject
/// Note that we keep a strong reference to the delegate here
@property (nonatomic, strong) id<SBContextDelegate> _Nullable delegate;
@property (nonatomic, strong) AVAudioPlayer * _Nullable audioPlayer;
- (nonnull instancetype)initWithAssetsURL:(NSURL * _Nonnull)assetsURL;
- (nonnull instancetype)initWithAssetsURL:(NSURL * _Nonnull)assetsURL delegate:(id<SBContextDelegate> _Nullable)delegate OBJC_DESIGNATED_INITIALIZER;
- (void)setupDrawingWithSize:(CGSize)size NS_SWIFT_NAME(setupDrawing(size:)); 
- (void)drawFrame;
- (void)reloadTexture;
- (void)update:(NSTimeInterval)time;
- (void)executeWithCommand:(NSString * _Nonnull)command NS_SWIFT_NAME(execute(command:));
- (void)executePythonWithCommand:(NSString * _Nonnull)command NS_SWIFT_NAME(execute(pythonCommand:));
- (void)cameraOperationWithDx:(float)dx dy:(float)dy mode:(NSInteger)mode NS_SWIFT_NAME(cameraOperation(dx:dy:mode:));
- (NSInteger)intForKey:(NSString * _Nonnull)key SWIFT_WARN_UNUSED_RESULT;
- (NSString * _Nonnull)stringForKey:(NSString * _Nonnull)key SWIFT_WARN_UNUSED_RESULT;
- (BOOL)boolForKey:(NSString * _Nonnull)key SWIFT_WARN_UNUSED_RESULT;
- (double)doubleForKey:(NSString * _Nonnull)key SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
@end
