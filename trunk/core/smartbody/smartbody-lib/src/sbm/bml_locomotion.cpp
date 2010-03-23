/*
 *  bml_locomotion.cpp - part of SmartBody-lib
 *  Copyright (C) 2009  University of Southern California
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

#include "bml_locomotion.hpp"

#include "mcontrol_util.h"

#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


////// XML ATTRIBUTES
const XMLCh ATTR_TARGET[]     = L"target";
const XMLCh ATTR_VELOCITY[]     = L"velocity";


using namespace std;
using namespace BML;
using namespace xml_utils;



BehaviorRequestPtr BML::parse_bml_locomotion( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& sync_seq, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	// Viseme transition hack until timing can support multiple sync points
	const XMLCh* attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && XMLString::stringLen( attrType ) ) {
        int type = -1;

        if( XMLString::compareIString( attrType, L"TARGET" )==0 ) {
			// Locomotion target is a static target

			const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );

			wcerr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type." << endl;
			return BehaviorRequestPtr();  // a.k.a., NULL
        } else if( XMLString::compareIString( attrType, L"VELOCITY" )==0 ) {
			// Locomotion specified by a velocity vector

			const XMLCh* attrVelocity = elem->getAttribute( ATTR_VELOCITY );
			
			wcerr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type." << endl;
			return BehaviorRequestPtr();  // a.k.a., NULL
        } else {
            wcerr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unknown type value, ignore command" << endl;
			return BehaviorRequestPtr();  // a.k.a., NULL
        }
    } else {
        wcerr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute." << endl;
		return BehaviorRequestPtr();  // a.k.a., NULL
    }
}
