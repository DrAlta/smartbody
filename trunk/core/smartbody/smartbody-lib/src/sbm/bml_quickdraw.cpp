/*
 *  bml_quickdraw.cpp - part of SmartBody-lib
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
 */

#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include "mcontrol_util.h"
#include "bml_quickdraw.hpp"
#include "me_ct_quick_draw.h"
#include "bml_target.hpp"
#include "xercesc_utils.hpp"


#define LOG_GAZE_PARAMS				(0)
#define DEBUG_BML_GAZE				(0)
#define DEBUG_JOINT_RANGE			(0)
#define DEBUG_GAZE_KEYS				(0)
#define DEBUG_DESCRIPTION_LEVELS	(0)


////// XML ATTRIBUTES
const XMLCh ATTR_ROLL[]         = L"roll";
const XMLCh ATTR_ANIM[]         = L"anim";


char* DEFAULT_QUICKDRAW_ANIM	= "AdultM_FastDraw001";


using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequest* BML::parse_bml_quickdraw( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !attrTarget || !XMLString::stringLen( attrTarget ) ) {
        wcerr << "WARNING: BML::parse_bml_quickdraw(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute." << endl;
		return NULL;
    }

	const SkJoint* joint = parse_target( tag, attrTarget, mcu );
	if( joint == NULL ) {  // invalid target (parse_target should have printed something)
		return NULL;
	}

	const XMLCh* attrRoll = elem->getAttribute( ATTR_ROLL );
	if( !attrTarget || !XMLString::stringLen( attrTarget ) ) {
	}

	string anim_name( DEFAULT_QUICKDRAW_ANIM );
	const XMLCh* attrAnim = elem->getAttribute( ATTR_ANIM );
	if( attrTarget && XMLString::stringLen( attrAnim )>0 ) {
		char* temp_ascii = XMLString::transcode( attrAnim );
		anim_name = temp_ascii;
		XMLString::release( &temp_ascii );
	}

	SkMotion* anim = mcu->motion_map.lookup( anim_name.c_str() );
	if( !anim ) {
        cerr << "WARNING: BML::parse_bml_quickdraw(): Unknown source animation \"" << anim_name << "\"." << endl;
		return NULL;
	}

	MeCtQuickDraw* qdraw_ct = new MeCtQuickDraw();
	qdraw_ct->init( anim );
	qdraw_ct->set_target_joint( 0, 0, 0, const_cast<SkJoint*>(joint) );

	return new MeControllerRequest( MeControllerRequest::MOTION, qdraw_ct, tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
}
