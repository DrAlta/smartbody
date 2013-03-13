//
//  EAGLView.m
//  Chases
//
//  Created by Linda MacPhee-Cobb on 3/24/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//



#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"

#define USE_DEPTH_BUFFER 1

// A class extension to declare private methods
@interface EAGLView ()



- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation EAGLView

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
        
        animationInterval = 15.0 / 60.0;
		[self setupView];
    }
    return self;
}

- (void)drawView 
{    
    static double timer = 0.0;
    timer += 0.016;
	SBMUpdateX(timer); 
   
    glClearColor(0.5f,0.5f,0.5f,1);
	glClear(GL_COLOR_BUFFER_BIT);
    glViewport( 0, 0, width, height);
	
    [EAGLContext setCurrentContext:context];
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
		
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf (projection);
//	glOrthof(left, right, bottom, top, near, far);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glLoadMatrixf (modelview);
//	glTranslatef(xposition, yposition, zposition);	// move things into view
    
    getBoneData();
    
/*
    // draw a plane first
    GLfloat planeSize  = 200.0f;
    GLfloat quad[12] = { planeSize, 0.f, planeSize, -planeSize, 0.f, planeSize, -planeSize,0.f,-planeSize, planeSize, 0.f, -planeSize };
    GLubyte indices[] = {0,2,1, 0,3,2};
    
    glColor4f(1, 1, 0, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &quad[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    
*/
    // draw bones
    glPointSize(2.0f);
    glLineWidth(1.0f);
    glColor4f(1, 1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (const GLfloat*)&jointPos[0]);
    glDrawArrays(GL_POINTS, 0, numJoints);
    glDrawElements(GL_LINES,numJoints*2,GL_UNSIGNED_SHORT, boneIdx);
    glDisableClientState(GL_VERTEX_ARRAY);   
    
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (const GLfloat*)&jointPos1[0]);
    glDrawArrays(GL_POINTS, 0, numJoints);
    glDrawElements(GL_LINES,numJoints*2,GL_UNSIGNED_SHORT, boneIdx);
    glDisableClientState(GL_VERTEX_ARRAY);     

	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];


}

- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    [self drawView];
}


- (BOOL)createFramebuffer {
    
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (USE_DEPTH_BUFFER) {
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation {
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation {
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer {
    [animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval {
    
    animationInterval = interval;
    if (animationTimer) {
        [self stopAnimation];
        [self startAnimation];
    }
}


-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch *touch = [touches anyObject];
	startPoint = [touch locationInView:self];
    prevPoint = startPoint;
}


-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch *touch = [touches anyObject];
	endPoint = [touch locationInView:self];
    CGPoint diff;
    diff.x = endPoint.x - startPoint.x;
    diff.y = endPoint.y - startPoint.y;    
    getCamera(diff.y, diff.y, startPoint.x, startPoint.y, endPoint.y, endPoint.y, cameraMode);    
}


-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [[event touchesForView:self] anyObject];
	currentPoint = [touch previousLocationInView:self];	
    CGPoint diff;
    diff.x = currentPoint.x - prevPoint.x;
    diff.y = currentPoint.y - prevPoint.y;
    prevPoint = currentPoint;
}


- (void)setupView
{
    static bool once = true;
    if (once)
    {
        
        NSString *dirPath = [[[NSBundle mainBundle] resourcePath]
                                   stringByAppendingPathComponent:@"assests"];
        const char* mediaPath = [dirPath UTF8String];
        SBInitialize(mediaPath);
        once = false;
    }
    
    getCamera(0, 0, 0, 0, 0, 0, -1);
    cameraMode = 0;
    
	// get screen dimensions
	CGRect rect = [[UIScreen mainScreen] bounds];
	width = rect.size.width;
	height = rect.size.height;
	
	// world dimensions
	left = -width/2.0;							// divide by 2 to center everything
	right = width/2.0;
	bottom = -height/2.0;	
	top = height/2.0;
	//near = 1.0f;
	//far =  300.0f;
	near = 100.0f;
	far = -100.0f;
	//near = -1.0f;
    //far = -300.0f;
	
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);	
		
	// init world location
	xposition = 0.0;
	yposition = 0.0;
	zposition = -2.0;
    
    swidth = width;
    sheight = height;
	
}


- (void)dealloc {
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

@end
