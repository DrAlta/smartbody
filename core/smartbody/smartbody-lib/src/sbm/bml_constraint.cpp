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

#include "bml_constraint.hpp"

#include "mcontrol_util.h"
#include "me_ct_constraint.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


#define TEST_GAZE_LOCOMOTION 0 // set to 1 if want to test gaze+locomotion control when reaching

////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";

////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ICT.SBM";

////// XML ATTRIBUTES
const XMLCh ATTR_EFFECTOR[] = L"effector";
const XMLCh ATTR_FADE_OUT[]		= L"sbm:fade-out";
const XMLCh ATTR_FADE_IN[]		= L"sbm:fade-in";


using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_constraint( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	std::wstringstream wstrstr;	

	MeCtConstraint* constraintCt = NULL; 

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

			constraintCt = dynamic_cast<MeCtConstraint*>(controller);
		}

		if (!constraintCt)
		{
			LOG("Handle : %s, controller not found.",handle.c_str());
		}
	}
	

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !constraintCt && (!attrTarget || !XMLString::stringLen( attrTarget ) ) ) {		
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

	const XMLCh* attrEffector = NULL;
	const char* effectorName = NULL;
	attrEffector = elem->getAttribute(ATTR_EFFECTOR);	
	if( attrEffector && XMLString::stringLen( attrEffector ) ) 
	{
		effectorName = asciiString(attrEffector);
	}

	if (target_joint == NULL && !constraintCt) {  // Invalid target.  Assume parse_target(..) printed error.
		return BehaviorRequestPtr();  // a.k.a., NULL
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
	if (!constraintCt)
	{
		constraintCt = new MeCtConstraint(request->actor->skeleton_p);		
		constraintCt->handle(handle);
		constraintCt->init();
		bCreateNewController = true;
	}

	if( target_joint && effectorName)	{
		//constraintCt->set_target_joint(const_cast<SkJoint*>( target_joint ) );
		constraintCt->addJointEndEffectorPair(const_cast<SkJoint*>(target_joint),effectorName);
	}

	if (fadeInTime >= 0.0)
		constraintCt->setFadeIn(fadeInTime);

	if (fadeOutTime >= 0.0)
		constraintCt->setFadeOut(fadeOutTime);

	BehaviorSyncPoints::iterator end = behav_syncs.sync_end();	
	
	BML::NamedSyncPointPtr syncPtr = (*end);
	
 	if( isTimeSet( syncPtr.sync()->offset) ) {
 		//reachCt->set_duration(syncPtr.sync()->offset);	
 	}

	boost::shared_ptr<MeControllerRequest> ct_request;
	ct_request.reset();
	if (bCreateNewController)
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, constraintCt, request->actor->reach_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}		

	return ct_request;
}
