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


#define DEBUG_DESCRIPTION_LEVELS	(0)


////// XML ATTRIBUTES
const XMLCh ATTR_ANIM[]         = L"anim";
const XMLCh ATTR_SMOOTH[]       = L"smooth";
const XMLCh ATTR_TRACK_DUR[]    = L"track-duration";
const XMLCh ATTR_GUNDRAW_DUR[]   = L"gundraw-duration";
const XMLCh ATTR_HOLSTER_DUR[]   = L"holster-duration";
const XMLCh ATTR_AIM_OFFSET[]    = L"aim-offset";



char* DEFAULT_QUICKDRAW_ANIM	= "AdultM_FastDraw001";


using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_quickdraw( DOMElement* elem,
											 std::string& unique_id,
											 BehaviorSyncPoints& behav_syncs,
											 bool required,
											 BmlRequestPtr request,
											 mcuCBHandle *mcu )
{
    const XMLCh* tag      = elem->getTagName();

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !attrTarget || !XMLString::stringLen( attrTarget ) ) {
        wcerr << "WARNING: BML::parse_bml_quickdraw(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute." << endl;
		return BehaviorRequestPtr();  // a.k.a., NULL
    }

	const SkJoint* joint = parse_target( tag, attrTarget, mcu );
	if( joint == NULL ) {  // invalid target (parse_target should have printed something)
		return BehaviorRequestPtr();  // a.k.a., NULL
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
		return BehaviorRequestPtr();  // a.k.a., NULL
	}

	float track_duration = -1;  // indefinite tracking by default
	const XMLCh* attrTrackDur = elem->getAttribute( ATTR_TRACK_DUR );
	if( attrTrackDur && XMLString::stringLen( attrTrackDur )>0 ) {
		char* temp_ascii = XMLString::transcode( attrTrackDur );
		string parse_buffer( temp_ascii );
		istringstream parser( parse_buffer );
		if( !( parser >> track_duration ) ) {
			wcerr << "WARNING: BML::parse_bml_quickdraw(): Attribute "<<ATTR_TRACK_DUR<<"=\""<<attrTrackDur<<"\" is not a valid number." << endl;
		}
		XMLString::release( &temp_ascii );
	}

	bool set_gundraw_dur_param = false;
	float gundraw_dur = 0.0;
	const XMLCh* attrGundrawDur = elem->getAttribute( ATTR_GUNDRAW_DUR );
	if( attrGundrawDur && XMLString::stringLen( attrGundrawDur )>0 ) {
		char* temp_ascii = XMLString::transcode( attrGundrawDur );
		string parse_buffer( temp_ascii );
		istringstream parser( parse_buffer );
		if( !( parser >> gundraw_dur ) ) {
			wcerr << "WARNING: BML::parse_bml_quickdraw(): Attribute "<<ATTR_GUNDRAW_DUR<<"=\""<<attrGundrawDur<<"\" is not a valid number." << endl;
		}
		else	{
			set_gundraw_dur_param = true;
		}
		XMLString::release( &temp_ascii );
	}

	bool set_holster_dur_param = false;
	float holster_dur = 0.0;
	const XMLCh* attrHolsterDur = elem->getAttribute( ATTR_HOLSTER_DUR );
	if( attrHolsterDur && XMLString::stringLen( attrHolsterDur )>0 ) {
		char* temp_ascii = XMLString::transcode( attrHolsterDur );
		string parse_buffer( temp_ascii );
		istringstream parser( parse_buffer );
		if( !( parser >> gundraw_dur ) ) {
			wcerr << "WARNING: BML::parse_bml_quickdraw(): Attribute "<<ATTR_HOLSTER_DUR<<"=\""<<attrHolsterDur<<"\" is not a valid number." << endl;
		}
		else	{
			set_holster_dur_param = true;
		}
		XMLString::release( &temp_ascii );
	}

	bool set_aim_offset_param = false;
	float aim_offset_p = 0.0;
	float aim_offset_h = 0.0;
	float aim_offset_r = 0.0;
	const XMLCh* attrAimOffset = elem->getAttribute( ATTR_AIM_OFFSET );
	if( attrAimOffset && XMLString::stringLen( attrAimOffset )>0 ) {
		char* temp_ascii = XMLString::transcode( attrAimOffset );
		string parse_buffer( temp_ascii );
		istringstream parser( parse_buffer );
		if( !( parser >> aim_offset_p >> aim_offset_h >> aim_offset_r ) ) {
			wcerr << "WARNING: BML::parse_bml_quickdraw(): Attribute "<<ATTR_AIM_OFFSET<<"=\""<<attrAimOffset<<"\" is not valid." << endl;
		}
		else	{
			set_aim_offset_param = true;
		}
		XMLString::release( &temp_ascii );
	}

	bool set_smooth_param = false;
	float smooth_factor = 0.0;
	const XMLCh* attrSmooth = elem->getAttribute( ATTR_SMOOTH );
	if( attrSmooth && XMLString::stringLen( attrSmooth )>0 ) {
		char* temp_ascii = XMLString::transcode( attrSmooth );
		string parse_buffer( temp_ascii );
		istringstream parser( parse_buffer );
		if( !( parser >> smooth_factor ) ) {
			wcerr << "WARNING: BML::parse_bml_quickdraw(): Attribute "<<ATTR_SMOOTH<<"=\""<<attrSmooth<<"\" is not a valid number." << endl;
		}
		else	{
			set_smooth_param = true;
		}
		XMLString::release( &temp_ascii );
	}

	MeCtQuickDraw* qdraw_ct = new MeCtQuickDraw();
	qdraw_ct->init( anim );
	qdraw_ct->set_target_joint( 0, 0, 0, const_cast<SkJoint*>(joint) );
	qdraw_ct->set_track_duration( track_duration );
	if( set_gundraw_dur_param ) qdraw_ct->set_gundraw_duration( gundraw_dur );
	if( set_holster_dur_param ) qdraw_ct->set_holster_duration( holster_dur );
	if( set_aim_offset_param ) qdraw_ct->set_aim_offset( aim_offset_p, aim_offset_h, aim_offset_r );
	if( set_smooth_param ) qdraw_ct->set_smooth( smooth_factor );

	return BehaviorRequestPtr( new MeControllerRequest( unique_id, qdraw_ct, request->actor->motion_sched_p, behav_syncs ) );
}
