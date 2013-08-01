//
//  ApplicationViewer.h
//  Chases
//
//  Created by Yuyu Xu on 8/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface ApplicationViewController : UIViewController < UITextFieldDelegate >
{
    EAGLView *glView;
	UIViewController* m_currentController;
    IBOutlet UITextField *text;    
    UISegmentedControl *segmentedControl;
}

@property (nonatomic, strong) IBOutlet EAGLView* glView;
@property (nonatomic, strong) IBOutlet UITextField* text;
@property (nonatomic, strong) IBOutlet UISegmentedControl* segmentedControl;


- (IBAction)changeCommand;
- (IBAction)segmentedControlIndexChanged;

@end

