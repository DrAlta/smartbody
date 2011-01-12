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

#include "bml_reach.hpp"

#include "mcontrol_util.h"
#include "me_ct_reach.hpp"

#include "bml_target.hpp"
#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";

////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ICT.SBM";

////// XML ATTRIBUTES

using namespace std;
using namespace BML;
using namespace xml_utils;


BehaviorRequestPtr BML::parse_bml_reach( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
	////////////////////////////////////////////////////////////////
	//  GAZE BEHAVIORS


	// determine if the requestor wants to use an existing gaze controller
	// identified by the 'handle' attribute


	MeCtReach* reachCt = new MeCtReach();

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !reachCt && (!attrTarget || !XMLString::stringLen( attrTarget ) ) ) {
		std::wstringstream wstrstr;
        wstrstr << "WARNING: BML::parse_bml_gaze(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute.";
		std::string str = convertWStringToString(wstrstr.str());
		LOG(str.c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
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

	boost::shared_ptr<MeControllerRequest> ct_request( new MeControllerRequest( unique_id, localId, reachCt, request->actor->reach_sched_p, behav_syncs ) );
	ct_request->set_persistent( true );
	return ct_request;
}
