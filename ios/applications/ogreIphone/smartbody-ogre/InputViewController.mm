//
//  InputViewController.m
//  Test
//
//  Created by Yuyu Xu on 9/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "InputViewController.h"
#include "test.h"
#include "OgreFramework.h"
using namespace Ogre; 

@implementation InputViewController

- (CGFloat)distanceBetweenTwoPoints: (CGPoint)fromPoint toPoint: (CGPoint)toPoint
{
    
    float x = toPoint.x - fromPoint.x;
    float y = toPoint.y - fromPoint.y;
    return sqrt(x * x + y * y);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event 
{
    NSSet *allTouches = [event allTouches];
    switch ([allTouches count]) 
    {
        case 1: { //Single touch
            
            //Get the first touch.
            UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
            
            switch ([touch tapCount])
            {
                case 1: //Single Tap.
                {
                    
                } break;
                case 2: {//Double tap. 
                    
                } break;
            }
        } break;
        case 2: { //Double Touch
            
            
            //Track the initial distance between two fingers.
            UITouch *touch1 = [[allTouches allObjects] objectAtIndex:0];
            UITouch *touch2 = [[allTouches allObjects] objectAtIndex:1];
            
            
            initialDistance = [self distanceBetweenTwoPoints:[touch1 locationInView:self] 
                                                     toPoint:[touch2 locationInView:self]];
            prevFov = OgreFramework::getSingletonPtr()->m_pCamera->getFOVy().valueRadians();
            
        } break;
        default:
            break;  
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
{
    NSSet *allTouches = [event allTouches];
    
    switch ([allTouches count])
    {
        case 1: {
            UITouch *touch = [[touches allObjects] objectAtIndex:0];
            CGPoint currentLoc = [touch locationInView:self];
            CGPoint prevLoc = [touch previousLocationInView:self];
            float origTransX = currentLoc.x - prevLoc.x;
            float origTransY = currentLoc.y - prevLoc.y;
            OgreFramework::getSingletonPtr()->m_pCamera->yaw(Degree(origTransX * -0.1));
            OgreFramework::getSingletonPtr()->m_pCamera->pitch(Degree(origTransY * -0.1));  
        } break;
        case 2: {
           
            UITouch *touch1 = [[allTouches allObjects] objectAtIndex:0];
            UITouch *touch2 = [[allTouches allObjects] objectAtIndex:1];
            
            //Calculate the distance between the two fingers.
            
            
            CGFloat finalDistance = [self distanceBetweenTwoPoints:[touch1 locationInView:self]
                                                   toPoint:[touch2 locationInView:self]];	
            
            float diff = initialDistance - finalDistance;
            diff *= 0.001;
            float camera = diff + prevFov;
            if (camera > 0.1 && camera < 1.8)
                OgreFramework::getSingletonPtr()->m_pCamera->setFOVy(Ogre::Radian(camera));
        } break;
    }    
 /*   
    NSSet *allTouches = [event allTouches];
    UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
    switch ([touch tapCount])    
    {
        case 1:
            {
                UITouch *touch = [[touches allObjects] objectAtIndex:0];
                CGPoint currentLoc = [touch locationInView:self];
                CGPoint prevLoc = [touch previousLocationInView:self];
                float origTransX = currentLoc.x - prevLoc.x;
                float origTransY = currentLoc.y - prevLoc.y;
                OgreFramework::getSingletonPtr()->m_pCamera->yaw(Degree(origTransX * -0.1));
                OgreFramework::getSingletonPtr()->m_pCamera->pitch(Degree(origTransY * -0.1));        
            }
            break;
        case 2:
            {
                UITouch *touch1 = [[touches allObjects] objectAtIndex:0];
                CGPoint currentLoc1 = [touch1 locationInView:self];
                CGPoint prevLoc1 = [touch1 previousLocationInView:self];
                UITouch *touch2 = [[touches allObjects] objectAtIndex:1];
                CGPoint currentLoc2 = [touch2 locationInView:self];
                CGPoint prevLoc2 = [touch2 previousLocationInView:self];
                float xd1 = currentLoc2.x - currentLoc1.x;
                float yd1 = currentLoc2.y - currentLoc1.y;
                float xd2 = prevLoc2.x - prevLoc1.x;
                float yd2 = prevLoc2.y - prevLoc1.y;
                float dist1 = sqrt(xd1 * xd1 + yd1 * yd1);
                float dist2 = sqrt(xd2 * xd2 + yd2 * yd2);
                float dist = dist1 - dist2;
            
                OgreFramework::getSingletonPtr()->m_pCamera->setFOVy(OgreFramework::getSingletonPtr()->m_pCamera->getFOVy() + Radian(dist));       
            }
            break;
        default:
            break;
    }
 */ 
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField 
{
    [textField resignFirstResponder];
    return NO;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    const char* cString = [textField.text UTF8String];     
    NSLog(@"%s", cString);
    SBMExecuteCmd(cString);
}

@end