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
const XMLCh ATTR_REACH_VELOCITY[] = L"sbm:reach-velocity";
const XMLCh ATTR_APEX_DURATION[] = L"sbm:apex-duration";



using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_bodyreach( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	std::wstringstream wstrstr;	

	MeCtExampleBodyReach* bodyReachCt = NULL; 

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
	

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !bodyReachCt && (!attrTarget || !XMLString::stringLen( attrTarget ) ) ) {		
        wstrstr << "WARNING: BML::parse_bml_reach(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute.";
		std::string str = convertWStringToString(wstrstr.str());
		LOG(str.c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
    }

	const SkJoint* target_joint = NULL;
	if (attrTarget && XMLString::stringLen( attrTarget ))
	{
		target_joint = parse_target( tag, attrTarget, mcu );
		
	}

	if (target_joint == NULL && !bodyReachCt) {  // Invalid target.  Assume parse_target(..) printed error.
		return BehaviorRequestPtr();  // a.k.a., NULL
	}

	const XMLCh* attrApexDuration = elem->getAttribute( ATTR_APEX_DURATION );
	float apexDuration = -1.f;
	if(attrApexDuration != NULL && attrApexDuration[0] != '\0') 
	{
		if( !( wistringstream( attrApexDuration ) >> apexDuration) )
		{
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse apex-duration interval attribute \""<< XMLString::transcode(attrApexDuration) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

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

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	bool bCreateNewController = false;
	if (!bodyReachCt)
	{
		bodyReachCt = new MeCtExampleBodyReach(request->actor->skeleton_p);		
		bodyReachCt->handle(handle);
		bodyReachCt->init();		
		bodyReachCt->reachVelocity = reachVelocity > 0 ? reachVelocity : 50.f;
		bodyReachCt->reachCompleteDuration = apexDuration > 0 ? apexDuration : 2.f;

		const MotionDataSet& motionData = request->actor->getReachMotionDataSet();
		bodyReachCt->updateMotionExamples(motionData);
		bCreateNewController = true;
	}

	if (reachVelocity > 0)
		bodyReachCt->reachVelocity = reachVelocity;
	if (apexDuration > 0)
		bodyReachCt->reachCompleteDuration = apexDuration;


	if( target_joint )	{
		SrVec reachPos = target_joint->gmat().get_translation();
		bodyReachCt->setReachTarget(reachPos);	
		bodyReachCt->setReachTargetJoint(const_cast<SkJoint*>(target_joint));
	}

	boost::shared_ptr<MeControllerRequest> ct_request;
	ct_request.reset();
	if (bCreateNewController)
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, bodyReachCt, request->actor->reach_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}		

	return ct_request;
}
