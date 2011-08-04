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
#include <map>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include <sr/sr_vec.h>
#include <sr/sr_alg.h>

#include "bml_bodyreach.hpp"

#include "mcontrol_util.h"
#include "me_ct_example_body_reach.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"
#include "BMLDefs.h"

#define TEST_GAZE_LOCOMOTION 0 // set to 1 if want to test gaze+locomotion control when reaching

/*
////// XML ATTRIBUTES
//const XMLCh ATTR_EFFECTOR[] = L"effector";
//const XMLCh ATTR_ROOT[] = L"sbm:root";
const XMLCh ATTR_CONS_JOINT[] = L"sbm:cons-joint";
const XMLCh ATTR_CONS_TARGET[] = L"sbm:cons-target";
const XMLCh ATTR_TARGET_POS[] = L"sbm:target-pos";
//const XMLCh ATTR_TARGET_PAWN[] = L"sbm:target-pawn";
const XMLCh ATTR_REACH_VELOCITY[] = L"sbm:reach-velocity";
const XMLCh ATTR_REACH_DURATION[] = L"sbm:reach-duration";
const XMLCh ATTR_REACH_FINISH[] = L"sbm:reach-finish";
const XMLCh ATTR_FOOT_IK[] = L"sbm:foot-ik";
const XMLCh ATTR_REACH_ACTION[] = L"sbm:action";
const XMLCh ATTR_FADE_OUT[]		= L"sbm:fade-out";
const XMLCh ATTR_FADE_IN[]		= L"sbm:fade-in";
const XMLCh ATTR_OBSTACLE[] = L"sbm:obstacle";
//const XMLCh ATTR_APEX_DURATION[] = L"sbm:apex-duration";
*/


using namespace std;
using namespace BML;
using namespace xml_utils;

#define REQUIRED_ATTR	(false)

BehaviorRequestPtr BML::parse_bml_bodyreach( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	std::wstringstream wstrstr;	

	MeCtExampleBodyReach* bodyReachCt = NULL; 

	

	SbmCharacter* curCharacter = const_cast<SbmCharacter*>(request->actor);
	std::map<int,MeCtReachEngine*>& reMap = curCharacter->getReachEngineMap();
	if (reMap.size() == 0)
	{
		LOG("Character : %s, no reach engine initialized.", request->actor->getName().c_str());
		return BehaviorRequestPtr();
	}

	const XMLCh* attrHandle = elem->getAttribute( BMLDefs::ATTR_HANDLE );

	std::string handle = xml_parse_string(BMLDefs::ATTR_HANDLE, elem, "", REQUIRED_ATTR);//"";
	if( !handle.empty() ) {		
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

	const XMLCh* attrTarget = elem->getAttribute( BMLDefs::ATTR_TARGET );
	const SbmPawn* targetPawn = NULL;
	const SkJoint* targetJoint = NULL;
	if (attrTarget && XMLString::stringLen( attrTarget ))
	{
		targetPawn = parse_target_pawn(tag,attrTarget,mcu);
		if (!targetPawn)
			targetJoint = parse_target(tag,attrTarget,mcu);
	}

// 	const XMLCh* attrObstracle = elem->getAttribute(BMLDefs::ATTR_OBSTACLE );
// 	const char* obstacleName = NULL;
// 	const SbmPawn* obstacle_pawn = NULL;
// 	if (attrObstracle && XMLString::stringLen( attrObstracle ))
// 	{
// 		obstacleName = asciiString(attrObstracle);
// 		obstacle_pawn = parse_target_pawn( tag, attrObstracle, mcu );		
// 	}
	
	std::string consJointName = xml_parse_string(BMLDefs::ATTR_CONS_JOINT, elem, "", REQUIRED_ATTR);
	const XMLCh* attrConsTarget = elem->getAttribute( BMLDefs::ATTR_CONS_TARGET );
	std::string consTargetName = xml_parse_string(BMLDefs::ATTR_CONS_TARGET, elem, "", REQUIRED_ATTR);	
	SkJoint* consTarget = const_cast<SkJoint*>(parse_target(tag,attrConsTarget,mcu));

	const XMLCh* attrTargetPos = elem->getAttribute( BMLDefs::ATTR_TARGET_POS );
	SrVec targetPos = SrVec();
	xml_parse_float((float*)&targetPos,3,BMLDefs::ATTR_TARGET_POS,elem,REQUIRED_ATTR);

	const XMLCh* attrReachDuration = elem->getAttribute( BMLDefs::ATTR_REACH_DURATION );
	float reachDuration = xml_parse_float(BMLDefs::ATTR_REACH_DURATION,elem,-1.f,REQUIRED_ATTR);//-1.f;	
	float reachVelocity = xml_parse_float(BMLDefs::ATTR_REACH_VELOCITY,elem,-1.f,REQUIRED_ATTR);

	std::string reachType = xml_parse_string(BMLDefs::ATTR_REACH_TYPE,elem,"none",false);

	float fadeOutTime = -1.f;
	float fadeInTime = -1.f;	
	std::string localId = xml_parse_string( BMLDefs::ATTR_ID, elem, "", REQUIRED_ATTR );

	bool bCreateNewController = false;	
	if (!bodyReachCt)
	{		
		bodyReachCt = new MeCtExampleBodyReach(curCharacter->getReachEngineMap());
		bodyReachCt->handle(handle);
		bodyReachCt->init(curCharacter);
		bCreateNewController = true;		
	}

	bodyReachCt->setDefaultReachType(reachType);

	std::string attrReachAction = xml_parse_string( BMLDefs::ATTR_REACH_ACTION, elem, "", REQUIRED_ATTR );	
	if(  stringICompare(attrReachAction,"pick-up" )) 
	{					
		bodyReachCt->setHandActionState(MeCtReachEngine::PICK_UP_OBJECT);
	}
	else if( stringICompare(attrReachAction,"touch") )
	{				
		bodyReachCt->setHandActionState(MeCtReachEngine::TOUCH_OBJECT);
	}
	else if( stringICompare(attrReachAction,"put-down") )
	{			
		bodyReachCt->setHandActionState(MeCtReachEngine::PUT_DOWN_OBJECT);
	}

	std::string attrReachFinish = xml_parse_string( BMLDefs::ATTR_REACH_FINISH, elem, "", REQUIRED_ATTR );	
	if( stringICompare(attrReachFinish,"true") ) 
	{			
		bodyReachCt->setFinishReaching(true);
	}
	else if( stringICompare(attrReachFinish,"false") )
	{			
		bodyReachCt->setFinishReaching(false);
	}

	std::string attrFootIK = xml_parse_string( BMLDefs::ATTR_FOOT_IK, elem, "", REQUIRED_ATTR );	
	if( stringICompare(attrFootIK,"true") ) 
	{			
		bodyReachCt->setFootIK(true);
	}
	else if( stringICompare(attrFootIK,"false") )
	{			
		bodyReachCt->setFootIK(false);
	}

	if (reachVelocity > 0)
	{
		bodyReachCt->setLinearVelocity(reachVelocity);		
	}

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

	if (!consTargetName.empty() && !consJointName.empty())
	{
		bodyReachCt->addHandConstraint(const_cast<SkJoint*>(consTarget),consJointName.c_str());
	}
	
	if (attrReachDuration && XMLString::stringLen( attrReachDuration ))
	{
		bodyReachCt->setReachCompleteDuration(reachDuration);		
	}	

// 	if (fadeInTime >= 0.0)
// 		bodyReachCt->setFadeIn(fadeInTime);
// 
// 	if (fadeOutTime >= 0.0)
// 		bodyReachCt->setFadeOut(fadeOutTime);

	boost::shared_ptr<MeControllerRequest> ct_request;
	ct_request.reset();
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, bodyReachCt, request->actor->reach_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}		

	return ct_request;
}
