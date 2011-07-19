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

#include "bml_bodyreach.hpp"

#include "mcontrol_util.h"
#include "me_ct_example_body_reach.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


#define TEST_GAZE_LOCOMOTION 0 // set to 1 if want to test gaze+locomotion control when reaching

////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";

////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ICT.SBM";

////// XML ATTRIBUTES
//const XMLCh ATTR_EFFECTOR[] = L"effector";
//const XMLCh ATTR_ROOT[] = L"sbm:root";
const XMLCh ATTR_CONS_JOINT[] = L"sbm:cons-joint";
const XMLCh ATTR_CONS_TARGET[] = L"sbm:cons-target";
const XMLCh ATTR_TARGET_POS[] = L"sbm:target-pos";
//const XMLCh ATTR_TARGET_PAWN[] = L"sbm:target-pawn";
const XMLCh ATTR_REACH_VELOCITY[] = L"sbm:reach-velocity";
const XMLCh ATTR_REACH_DURATION[] = L"sbm:reach-duration";
const XMLCh ATTR_REACH_TYPE[] = L"sbm:reach-type";
const XMLCh ATTR_REACH_FINISH[] = L"sbm:reach-finish";
const XMLCh ATTR_FOOT_IK[] = L"sbm:foot-ik";
const XMLCh ATTR_REACH_ACTION[] = L"sbm:action";
const XMLCh ATTR_FADE_OUT[]		= L"sbm:fade-out";
const XMLCh ATTR_FADE_IN[]		= L"sbm:fade-in";
const XMLCh ATTR_OBSTACLE[] = L"sbm:obstacle";
//const XMLCh ATTR_APEX_DURATION[] = L"sbm:apex-duration";



using namespace std;
using namespace BML;
using namespace xml_utils;

BehaviorRequestPtr BML::parse_bml_bodyreach( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	std::wstringstream wstrstr;	

	MeCtExampleBodyReach* bodyReachCt = NULL; 

	SbmCharacter* curCharacter = const_cast<SbmCharacter*>(request->actor);

	std::map<int,MeCtReachEngine*>& reMap = curCharacter->getReachEngineMap();
	if (reMap.size() == 0)
	{
		LOG("Character : %s, no reach engine initialized.",request->actor->name);
		return BehaviorRequestPtr();
	}

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

			bodyReachCt = dynamic_cast<MeCtExampleBodyReach*>(controller);
		}

		if (!bodyReachCt)
		{
			LOG("Handle : %s, controller not found.",handle.c_str());
		}
	}

// 	if( !bodyReachCt && (!attrTarget || !XMLString::stringLen( attrTarget ) ) ) {		
//         wstrstr << "WARNING: BML::parse_bml_reach(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute.";
// 		std::string str = convertWStringToString(wstrstr.str());
// 		LOG(str.c_str());
// 		return BehaviorRequestPtr();  // a.k.a., NULL
//     }

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET);
	const SbmPawn* targetPawn = NULL;
	const SkJoint* targetJoint = NULL;
	if (attrTarget && XMLString::stringLen( attrTarget ))
	{
		//target_joint = parse_target( tag, attrTarget, mcu );		
		targetPawn = parse_target_pawn(tag,attrTarget,mcu);
		if (!targetPawn)
			targetJoint = parse_target(tag,attrTarget,mcu);
	}

// 	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
// 	const SkJoint* targetJoint = NULL;
// 	if (attrTarget && XMLString::stringLen( attrTarget ))
// 	{
// 		targetJoint = parse_target( tag, attrTarget, mcu );		
// 		//targetPawn = parse_target_pawn(tag,attrTarget,mcu);
// 	}

// 	const XMLCh* attrEffector = NULL;
// 	const char* effectorName = NULL;
// 	attrEffector = elem->getAttribute(ATTR_EFFECTOR);	
// 	SkJoint* effectorJoint = NULL;
// 	if( attrEffector && XMLString::stringLen( attrEffector ) ) 
// 	{
// 		effectorName = asciiString(attrEffector);
// 		effectorJoint = request->actor->skeleton_p->search_joint(effectorName);		
// 	}

	const XMLCh* attrObstracle = elem->getAttribute( ATTR_OBSTACLE );
	const char* obstacleName = NULL;
	const SbmPawn* obstacle_pawn = NULL;
	if (attrObstracle && XMLString::stringLen( attrObstracle ))
	{
		obstacleName = asciiString(attrObstracle);
		obstacle_pawn = parse_target_pawn( tag, attrObstracle, mcu );		
	}

	const XMLCh* attrConsJoint = NULL;
	const char* consJointName = NULL;
	attrConsJoint = elem->getAttribute(ATTR_CONS_JOINT);		
	if( attrConsJoint && XMLString::stringLen( attrConsJoint ) ) 
	{
		consJointName = asciiString(attrConsJoint);				
	}

	const XMLCh* attrConsTarget = NULL;
	const char* consTargetName = NULL;
	attrConsTarget = elem->getAttribute(ATTR_CONS_TARGET);	
	const SkJoint* consTarget = NULL;
	if( attrConsTarget && XMLString::stringLen( attrConsTarget ) ) 
	{
		//consTargetName = asciiString(attrConsTarget);
		//consTarget = request->actor->skeleton_p->search_joint(consTargetName);		
		consTarget = parse_target(tag,attrConsTarget,mcu);
	}

// 	const XMLCh* attrRoot = NULL;
// 	const char* rootName = NULL;
// 	attrRoot = elem->getAttribute(ATTR_ROOT);		
// 	if( attrRoot && XMLString::stringLen( attrRoot ) ) 
// 	{
// 		rootName = asciiString(attrRoot);			
// 	}

	SrVec targetPos = SrVec();

	XMLCh* token;
	const XMLCh* attrTargetPos = elem->getAttribute( ATTR_TARGET_POS );		
	if (attrTargetPos && XMLString::stringLen( attrTargetPos ))
	{
		wistringstream in;		
		XMLStringTokenizer tokenizer( attrTargetPos );
		if (tokenizer.countTokens() == 3)
		{
			for (int i=0;i<3;i++)
			{
				token = tokenizer.nextToken();
				in.clear();
				in.str( token );
				in.seekg(0);
				in >> targetPos[i];
			}
		}								
	}

// 	if (effectorJoint == NULL && !bodyReachCt) {  // Invalid target.  Assume parse_target(..) printed error.
// 		return BehaviorRequestPtr();  // a.k.a., NULL
// 	}

	const XMLCh* attrReachDuration = elem->getAttribute( ATTR_REACH_DURATION );
	float reachDuration = -1.f;
	bool hasReachDuration = false;
	if(attrReachDuration != NULL && attrReachDuration[0] != '\0') 
	{
		if( !( wistringstream( attrReachDuration ) >> reachDuration) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse reach-duration interval attribute \""<< XMLString::transcode(attrReachDuration) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
		else
		{
			hasReachDuration = true;
		}
	}

	std::string reachType = xml_parse_string(ATTR_REACH_TYPE,elem,"none",false);

	const XMLCh* attrReachVelocity = elem->getAttribute( ATTR_REACH_VELOCITY );
	float reachVelocity = -1.f;
	if(attrReachVelocity != NULL && attrReachVelocity[0] != '\0') 
	{
		if( !( wistringstream( attrReachVelocity ) >> reachVelocity) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse reach-velocity interval attribute \""<< XMLString::transcode(attrReachVelocity) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}	

	float fadeOutTime = -1.f;
	float fadeInTime = -1.f;
// 	const XMLCh* attrFadeOut = elem->getAttribute( ATTR_FADE_OUT );
// 	if(attrFadeOut != NULL && attrFadeOut[0] != '\0') 
// 	{
// 		if( !( wistringstream( attrFadeOut ) >> fadeOutTime ) )
// 		{
// 			std::stringstream strstr;
// 			strstr << "WARNING: Failed to parse fade-out interval attribute \""<< XMLString::transcode(attrFadeOut) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
// 			LOG(strstr.str().c_str());
// 		}
// 	}
// 
// 	const XMLCh* attrFadeIn = elem->getAttribute( ATTR_FADE_IN );
// 	if(attrFadeIn != NULL && attrFadeIn[0] != '\0') 
// 	{
// 		if( !( wistringstream( attrFadeIn ) >> fadeInTime ) )
// 		{
// 			std::stringstream strstr;
// 			strstr << "WARNING: Failed to parse fade-in interval attribute \""<< XMLString::transcode(attrFadeIn) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
// 			LOG(strstr.str().c_str());
// 		}
// 	}

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	bool bCreateNewController = false;
	
	
	if (!bodyReachCt)
	{		
		bodyReachCt = new MeCtExampleBodyReach(curCharacter->getReachEngineMap());
		bodyReachCt->handle(handle);
		bodyReachCt->init(curCharacter);
		bCreateNewController = true;		
	}

	bodyReachCt->setDefaultReachType(reachType);

	const XMLCh* attrReachAction = NULL;
	attrReachAction = elem->getAttribute(ATTR_REACH_ACTION);
	if( attrReachAction && XMLString::stringLen( attrReachAction ) ) 
	{
		if( XMLString::compareIString( attrReachAction, L"pick-up" )==0 ) 
		{					
			bodyReachCt->setHandActionState(MeCtReachEngine::PICK_UP_OBJECT);
		}
		else if( XMLString::compareIString( attrReachAction, L"touch" )==0 )
		{				
			bodyReachCt->setHandActionState(MeCtReachEngine::TOUCH_OBJECT);
		}
		else if( XMLString::compareIString( attrReachAction, L"put-down" )==0 )
		{			
			bodyReachCt->setHandActionState(MeCtReachEngine::PUT_DOWN_OBJECT);
		}
	}	

	const XMLCh* attrReachFinish = NULL;
	attrReachFinish = elem->getAttribute(ATTR_REACH_FINISH);
	if( attrReachFinish && XMLString::stringLen( attrReachFinish ) ) 
	{
		if( XMLString::compareIString( attrReachFinish, L"true" )==0 ) 
		{			
			bodyReachCt->setFinishReaching(true);
		}
		else if( XMLString::compareIString( attrReachFinish, L"false" )==0 )
		{			
			bodyReachCt->setFinishReaching(false);
		}
	}	

	const XMLCh* attrFootIK = NULL;
	attrFootIK = elem->getAttribute(ATTR_FOOT_IK);
	if( attrFootIK && XMLString::stringLen( attrFootIK ) ) 
	{
		if( XMLString::compareIString( attrFootIK, L"true" )==0 ) 
		{		
			LOG("Foot IK = true");
			bodyReachCt->setFootIK(true);
		}
		else if( XMLString::compareIString( attrFootIK, L"false" )==0 )
		{			
			bodyReachCt->setFootIK(false);
		}
	}	

	if (reachVelocity > 0)
	{
		bodyReachCt->setLinearVelocity(reachVelocity);		
	}

// 	if (rootName)
// 	{
// 		bodyReachCt->setEndEffectorRoot(rootName);
// 	}	


	if( targetPawn )	{		
		bodyReachCt->setReachTargetPawn(const_cast<SbmPawn*>(targetPawn));
	}
	else if (targetJoint)
	{
		bodyReachCt->setReachTargetJoint(const_cast<SkJoint*>(targetJoint));
	}
	else if (attrTargetPos && XMLString::stringLen( attrTargetPos ))
	{
		bodyReachCt->setReachTargetPos(targetPos);
	}

	if (consTarget && consJointName)
	{
		bodyReachCt->addHandConstraint(const_cast<SkJoint*>(consTarget),consJointName);
	}
	
// 	if (obstacle_pawn && obstacleName)
// 	{
// 		if (obstacle_pawn->colObj_p)
// 			bodyReachCt->addObstacle(obstacleName, obstacle_pawn->colObj_p);
// 	}

	if (hasReachDuration)
	{
		bodyReachCt->setReachCompleteDuration(reachDuration);		
	}


	if (fadeInTime >= 0.0)
		bodyReachCt->setFadeIn(fadeInTime);

	if (fadeOutTime >= 0.0)
		bodyReachCt->setFadeOut(fadeOutTime);

	boost::shared_ptr<MeControllerRequest> ct_request;
	ct_request.reset();
	//if (bCreateNewController)
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, bodyReachCt, request->actor->reach_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}		

	return ct_request;
}
