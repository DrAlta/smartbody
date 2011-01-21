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
 *      Andrew n marshall, USC
 *      Ed Fast, USC
 */

#include "vhcl.h"
#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include <sr/sr_vec.h>
#include <sr/sr_alg.h>

#include "bml_reach.hpp"

#include "mcontrol_util.h"
#include "me_ct_reach.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


#define TEST_GAZE_LOCOMOTION 1 // set to 1 if want to test gaze+locomotion control when reaching

////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";

////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ICT.SBM";

////// XML ATTRIBUTES
const XMLCh ATTR_REACH_ARM[] = L"reach-arm";

using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_reach( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
	
	// attach the skeleton to the reach controller
	MeCtReach* reachCt = new MeCtReach(request->actor->skeleton_p);

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !reachCt && (!attrTarget || !XMLString::stringLen( attrTarget ) ) ) {
		std::wstringstream wstrstr;
        wstrstr << "WARNING: BML::parse_bml_gaze(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute.";
		std::string str = convertWStringToString(wstrstr.str());
		LOG(str.c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
    }

	const XMLCh* attrReachArm = NULL;
	attrReachArm = elem->getAttribute( ATTR_REACH_ARM );
	if( attrReachArm && *attrReachArm != 0 ) 
	{
		if( XMLString::compareIString( attrReachArm, L"left" )==0 ) 
		{
			reachCt->setReachArm(MeCtReach::REACH_LEFT_ARM);
		}
		else if( XMLString::compareIString( attrReachArm, L"right" )==0 )
		{
			reachCt->setReachArm(MeCtReach::REACH_RIGHT_ARM);
		}
	}

	const SkJoint* target_joint = NULL;
	if (attrTarget && XMLString::stringLen( attrTarget ))
	{
		target_joint = parse_target( tag, attrTarget, mcu );
		
	}
	if (target_joint == NULL && !reachCt) {  // Invalid target.  Assume parse_target(..) printed error.
		return BehaviorRequestPtr();  // a.k.a., NULL
	}

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	
	reachCt->init();
	if( target_joint )	{
		reachCt->set_target_joint(const_cast<SkJoint*>( target_joint ) );
	}

	BehaviorSyncPoints::iterator end = behav_syncs.sync_end();	
	
	BML::NamedSyncPointPtr syncPtr = (*end);
	
 	if( isTimeSet( syncPtr.sync()->offset) ) {
 		//reachCt->set_duration(syncPtr.sync()->offset);	
 	}
	
	boost::shared_ptr<MeControllerRequest> ct_request( new MeControllerRequest( unique_id, localId, reachCt, request->actor->reach_sched_p, behav_syncs ) );
	ct_request->set_persistent( true );
	BehaviorSyncPoints::iterator start =behav_syncs.sync_start();
	BML::NamedSyncPointPtr syncStartPtr = (*start);

	
	float startTime = 0.f;
	if( isTimeSet( syncStartPtr.sync()->offset) ) {		
		startTime = syncStartPtr.sync()->offset;
	}	



#if TEST_GAZE_LOCOMOTION
	// make the character gaze at the target
	char gazeCmd[256];	
	if (target_joint)
	{
		std::wstringstream wstrstr;
		wstrstr << attrTarget;
		std::string targetName = convertWStringToString(wstrstr.str());		
		sprintf(gazeCmd,"bml char %s <gaze target=\"%s\" sbm:joint-range=\"EYES HEAD\"/>",request->actor->name,targetName.c_str());
		mcu->execute_later(gazeCmd,startTime);		
	}	

	// make the character walk toward the target if it is out of reach
	if (target_joint)
	{
		char locoCmd[256];	
		const SrMat& mat = target_joint->gmat();
		const SrMat& rootMat = reachCt->getRootJoint()->gmat();
		// get target position
		SrVec target = SrVec(mat.get(12),mat.get(13),mat.get(14));
		SrVec targetGround = SrVec(mat.get(12),0,mat.get(14));
		SrVec reachRootPos = SrVec(rootMat.get(12),rootMat.get(13),rootMat.get(14));
		SrVec actorPos; 	
		float walkLength = 0.0;

		SbmCharacter* actor = (SbmCharacter*)request->actor;	
		actorPos = actor->get_locomotion_ct()->get_navigator()->get_world_pos();
		actorPos.y = 0.f;
		SrVec offset = reachRootPos - target;	
		SrVec dir = targetGround - actorPos; 
		double pathLength = dir.len();
		double limbLength = reachCt->getLimbLength();	
		dir.normalize();	
		// solve a qudratic equation to get suitable offsets	
#define QUADRATIC_SOLVE_PATH 0
#if QUADRATIC_SOLVE_PATH
		double c[3];
		double sol[2];		 
		c[0] = dot(offset,offset) - limbLength*limbLength*0.9;
		c[1] = dot(offset,dir)*2.f;
		c[2] = dot(dir,dir);
		// this heuristic is currently buggy ( didn't account for character facing directions ) , so use the simplest heuristic of limb length.
		int success = sr_solve_quadric_polynomial(c,sol);
		walkLength = success != 0 ? min(sol[0],sol[1]) : pathLength - limbLength*0.8; 
#else
		// if the object is not reachable, walk toward the object
		walkLength = pathLength > limbLength ? (float)(pathLength - limbLength*0.8) : 0.f;  
#endif
		SrVec newTarget = actorPos + dir*walkLength;

		if (walkLength != 0.0)
		{
			sprintf(locoCmd,"test loco char %s tx %f tz %f spd 30 lrps 1.5",request->actor->name,newTarget.x,newTarget.z);
			mcu->execute_later(locoCmd,startTime);  
		}
	}	
#endif
	return ct_request;
}
