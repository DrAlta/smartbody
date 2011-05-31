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
#include "gwiz_math.h"

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
const XMLCh ATTR_ROOT[] = L"sbm:root-joint";
const XMLCh ATTR_EFFECTOR[] = L"effector";
const XMLCh ATTR_CONSTRAINT_TYPE[] = L"sbm:constraint-type";
const XMLCh ATTR_EFFECTOR_ROOT[] = L"sbm:effector-root";
const XMLCh ATTR_FADE_OUT[]		= L"sbm:fade-out";
const XMLCh ATTR_FADE_IN[]		= L"sbm:fade-in";

const XMLCh ATTR_OFFSET_ROTX[]        = L"rot-x";
const XMLCh ATTR_OFFSET_ROTY[]      = L"rot-y";
const XMLCh ATTR_OFFSET_ROTZ[]         = L"rot-z";
//const XMLCh ATTR_OFFSET_POS[]         = L"offset-pos";

const XMLCh ATTR_X[]     = L"pos-x";
const XMLCh ATTR_Y[]     = L"pos-y";
const XMLCh ATTR_Z[]     = L"pos-z";


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

	const XMLCh* attrEffectorRoot = NULL;
	const char* effectorRootName = "";
	attrEffectorRoot = elem->getAttribute(ATTR_EFFECTOR_ROOT);	
	if( attrEffector && XMLString::stringLen( attrEffectorRoot ) ) 
	{
		effectorRootName = asciiString(attrEffectorRoot);
	}

	const XMLCh* attrRootJoint = NULL;
	const char* rootJointName = NULL;
	attrRootJoint = elem->getAttribute(ATTR_ROOT);	
	if( attrEffector && XMLString::stringLen( attrRootJoint ) ) 
	{
		rootJointName = asciiString(attrRootJoint);
	}

	const XMLCh* attrConstraintType = NULL;
	const char* typeName = NULL;
	attrConstraintType = elem->getAttribute(ATTR_CONSTRAINT_TYPE);	
	if( attrConstraintType && XMLString::stringLen( attrConstraintType ) ) 
	{
		typeName = asciiString(attrConstraintType);
	}

	if (target_joint == NULL && !constraintCt) {  // Invalid target.  Assume parse_target(..) printed error.
		return BehaviorRequestPtr();  // a.k.a., NULL
	}

	// get offset rotation
	SrVec posOffset = SrVec(0.f,0.f,0.f);
	SrQuat rotOffset = SrQuat();
	float rotX = 0.f, rotY = 0.f, rotZ = 0.f;
	const XMLCh* value = elem->getAttribute( ATTR_OFFSET_ROTX );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> rotX ) ) {
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse pitch attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element.";
			LOG(strstr.str().c_str());
		}
	}

	//euler_t e = euler_t()
	value = elem->getAttribute( ATTR_OFFSET_ROTY );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> rotY ) ) {
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse heading attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

	value = elem->getAttribute( ATTR_OFFSET_ROTZ );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> rotZ ) ) {
			std::stringstream strstr;
			strstr << "WARNING: Failed to parse roll attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
			LOG(strstr.str().c_str());
		}
	}

	XMLCh* token;
	//if(XMLString::compareIString( tag, ATTR_OFFSET_POS )==0)
	{		
		const XMLCh* attrTarget[3];		
		attrTarget[0] = elem->getAttribute( ATTR_X );
		attrTarget[1] = elem->getAttribute( ATTR_Y );
		attrTarget[2] = elem->getAttribute( ATTR_Z );
		
		wistringstream in;		
		for(int i = 0; i < 3; ++i)
		{
			XMLStringTokenizer tokenizer( attrTarget[i] );
			switch( tokenizer.countTokens() ) 
			{
			case 1:
				token = tokenizer.nextToken();
				in.clear();
				in.str( token );
				in.seekg(0);
				in >> posOffset[i];
				break;
			default:
				break;
			}
		}				
	}	

	gwiz::quat_t q = gwiz::quat_t(gwiz::euler_t( rotX, rotY, rotZ ));
	rotOffset = SrQuat((float)q.w(),(float)q.x(),(float)q.y(),(float)q.z());	

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
		SbmCharacter* chr = const_cast<SbmCharacter*>(request->actor);
		float characterHeight = chr->getHeight();
		constraintCt->characterHeight = characterHeight;
		constraintCt->handle(handle);
		constraintCt->init(rootJointName);
		bCreateNewController = true;
	}

	if( target_joint && effectorName)	{
		//constraintCt->set_target_joint(const_cast<SkJoint*>( target_joint ) );
		MeCtConstraint::ConstraintType cType = MeCtConstraint::CONSTRAINT_POS;
		if (typeName)
		{
			if (strcmp(typeName,"pos") == 0)
				cType = MeCtConstraint::CONSTRAINT_POS;
			else if (strcmp(typeName,"rot") == 0)
				cType = MeCtConstraint::CONSTRAINT_ROT;
		}
		
		constraintCt->addEffectorJointPair(const_cast<SkJoint*>(target_joint),effectorName,effectorRootName,posOffset,rotOffset,cType);
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
	//if (bCreateNewController)
	{
		ct_request.reset( new MeControllerRequest( unique_id, localId, constraintCt, request->actor->constraint_sched_p, behav_syncs ) );
		ct_request->set_persistent( true );
	}		

	return ct_request;
}
