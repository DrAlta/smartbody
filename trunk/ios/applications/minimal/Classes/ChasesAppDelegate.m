//
//  ChasesAppDelegate.m
//  Chases
//
//  Created by Linda MacPhee-Cobb on 3/24/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "ChasesAppDelegate.h"
#import "EAGLView.h"
#import "ApplicationViewer.h"
#import <sys/utsname.h>
#import <QuartzCore/QuartzCore.h>

@implementation ChasesAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(UIApplication *)application
{    
    UIDevice* thisDevice = [UIDevice currentDevice];
    
    if(thisDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad)
        m_rootViewController = [[ApplicationViewController alloc] initWithNibName:@"Window-iPad" bundle:nil];
    else
        m_rootViewController = [[ApplicationViewController alloc] initWithNibName:@"Window" bundle:nil];

    
	[window addSubview:m_rootViewController.view];
    [window makeKeyAndVisible]; 
}

- (void)dealloc {
	[window release];
	[m_rootViewController release];
	[super dealloc];
}

/*
- (void)applicationDidFinishLaunching:(UIApplication *)application {
	glView.animationInterval = 2.0 / 60.0;
	[glView startAnimation];
}


- (void)applicationWillResignActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	glView.animationInterval = 2.0 / 60.0;
}


- (void)dealloc {
	[window release];
	[glView release];
	[super dealloc];
}
*/
@end
