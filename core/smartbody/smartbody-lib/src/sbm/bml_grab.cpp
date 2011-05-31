/*
 *  bml_gaze.cpp - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Wei-Wen Feng, USC
 */

#include "vhcl.h"
#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include <sr/sr_vec.h>
#include <sr/sr_alg.h>

#include "bml_grab.hpp"

#include "mcontrol_util.h"
#include "me_ct_hand.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


#define TEST_GAZE_LOCOMOTION 0 // set to 1 if want to test gaze+locomotion control when reaching

////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";

////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ICT.SBM";

////// XML ATTRIBUTES
const XMLCh ATTR_WRIST[] = L"sbm:wrist";
//const XMLCh ATTR_TARGET_POS[] = L"sbm:target-pos";
const XMLCh ATTR_SOURCE_JOINT[] = L"sbm:source-joint";
const XMLCh ATTR_ATTACH_PAWN[] = L"sbm:attach-pawn";
const XMLCh ATTR_RELEASE_PAWN[] = L"sbm:release-pawn";
const XMLCh ATTR_GRAB_VELOCITY[] = L"sbm:grab-velocity";
const XMLCh ATTR_GRAB_STATE[] = L"sbm:grab-state";
const XMLCh ATTR_FADE_OUT[]		= L"sbm:fade-out";
const XMLCh ATTR_FADE_IN[]		= L"sbm:fade-in";
//const XMLCh ATTR_APEX_DURATION[] = L"sbm:apex-duration";



using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_grab( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    
	const XMLCh* tag      = elem->getTagName();
	std::wstringstream wstrstr;	
	MeCtHand* handCt = NULL; 
	const XMLCh* attrHandle = elem->getAttribute( ATTR_HANDLE );
	std::string handle = "";
	if( attrHandle && XMLString::stringLen( attrHandle ) ) {
		handle = asciiString(attrHandle);
		// look for a gaze controller with that handle
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		const SbmCharacter* character = request->actor;
		if (character)
		{
			MeControllerTreeRoot* controllerTree = character->ct_tree_p;
			MeController* controller = controllerTree->findControllerByHandle(handle);

			handCt = dynamic_cast<MeCtHand*>(controller);
		}

		if (!handCt)
		{
			LOG("Handle : %s, controller not found.",handle.c_str());
		}
	}

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	const SbmPawn* target_pawn = NULL;
	if (attrTarget && XMLString::stringLen( attrTarget ))
	{
		target_pawn = parse_target_pawn( tag, attrTarget, mcu );		
	}

	const XMLCh* attrWrist = NULL;
	const char* wristName = NULL;
	attrWrist = elem->getAttribute(ATTR_WRIST);	
	SkJoint* wristJoint = NULL;
	if( attrWrist && XMLString::stringLen( attrWrist ) ) 
	{
		wristName = asciiString(attrWrist);			
		wristJoint = request->actor->skeleton_p->search_joint(wristName);		
	}

	const XMLCh* attrSourceJoint = NULL;
	const char* sourceJointName = NULL;
	attrSourceJoint = elem->getAttribute(ATTR_SOURCE_JOINT);		
	if( attrSourceJoint && XMLString::stringLen( attrSourceJoint ) ) 
	{
		sourceJointName = asciiString(attrSourceJoint);						
	}

	const XMLCh* attrAttachPawn = elem->getAttribute( ATTR_ATTACH_PAWN );
	const SbmPawn* attachPawn = NULL;
	if (attrAttachPawn && XMLString::stringLen( attrAttachPawn ))
	{
		attachPawn = parse_target_pawn( tag, attrAttachPawn, mcu );		
	}

	if (wristJoint == NULL && !handCt) {  // Invalid target.  Assume parse_target(..) printed error.
		return BehaviorRequestPtr();  // a.k.a., NULL
	}

	const XMLCh* attrGrabVelocity = elem->getAttribute( ATTR_GRAB_VELOCITY );
	float grabVelocity = -1.f;
	if(attrGrabVelocity != NULL && attrGrabVelocity[0] != '\0') 
	{
		if( !( wistringstream( attrGrabVelocity ) >> grabVelocity) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse grab-velocity interval attribute \""<< XMLString::transcode(attrGrabVelocity) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

	float fadeOutTime = -1.0;
	float fadeInTime = -1.0;
	const XMLCh* attrFadeOut = elem->getAttribute( ATTR_FADE_OUT );
	if(attrFadeOut != NULL && attrFadeOut[0] != '\0') 
	{
		if( !( wistringstream( attrFadeOut ) >> fadeOutTime ) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse fade-out interval attribute \""<< XMLString::transcode(attrFadeOut) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

	const XMLCh* attrFadeIn = elem->getAttribute( ATTR_FADE_IN );
	if(attrFadeIn != NULL && attrFadeIn[0] != '\0') 
	{
		if( !( wistringstream( attrFadeIn ) >> fadeInTime ) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse fade-in interval attribute \""<< XMLString::transcode(attrFadeIn) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	bool bCreateNewController = false;
	
	if (!handCt)
	{
		handCt = new MeCtHand(request->actor->skeleton_p, wristJoint);		
		handCt->handle(handle);
		SbmCharacter* chr = const_cast<SbmCharacter*>(request->actor);
		
		handCt->init(chr->getReachHandData(),chr->getGrabHandData(),chr->getReleaseHandData());		
		if (grabVelocity > 0)
			handCt->grabVelocity = grabVelocity;		
		bCreateNewController = true;
	}

	const XMLCh* attrGrabState = NULL;
	attrGrabState = elem->getAttribute(ATTR_GRAB_STATE);
	if( attrGrabState && XMLString::stringLen( attrGrabState ) ) 
	{
		if( XMLString::compareIString( attrGrabState, L"start" )==0 ) 
		{			
			handCt->setGrabState(MeCtHand::GRAB_START);
		}
		else if( XMLString::compareIString( attrGrabState, L"reach" )==0 )
		{			
			handCt->setGrabState(MeCtHand::GRAB_REACH);
		}
		else if( XMLString::compareIString( attrGrabState, L"finish" )==0 )
		{			
			handCt->setGrabState(MeCtHand::GRAB_FINISH);
		}
		else if( XMLString::compareIString( attrGrabState, L"return" )==0 )
		{			
			handCt->setGrabState(MeCtHand::GRAB_RETURN);
		}
	}	

	const XMLCh* attrReleasePawn = NULL;
	attrReleasePawn = elem->getAttribute(ATTR_RELEASE_PAWN);
	if( attrReleasePawn && XMLString::stringLen( attrReleasePawn ) ) 
	{
		if( XMLString::compareIString( attrReleasePawn, L"true" )==0 ) 
		{			
			handCt->releasePawn();
		}
	}

	if (grabVelocity > 0)
		handCt->grabVelocity = grabVelocity;

	if( target_pawn && target_pawn->colObj_p)	{		
		handCt->setGrabTargetObject(target_pawn->colObj_p);		
	}

	if (attachPawn && sourceJointName)
	{
		std::string jointName = sourceJointName;
		SbmPawn* pawn = const_cast<SbmPawn*>(attachPawn);
		handCt->attachPawnTarget(pawn,jointName);
	}

	if (fadeInTime >= 0.0)
		handCt->setFadeIn(fadeInTime);

	if (fadeOutTime >= 0.0)
		handCt->setFadeOut(fadeOutTime);

	boost::shared_ptr<MeControllerRequest> ct_request;
	ct_request.reset();
	//if (bCreateNewController)
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, handCt, request->actor->grab_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}	
	return ct_request;
}
