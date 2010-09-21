/*
 *  bml_face.cpp - part of SmartBody-lib
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

#include <vhcl_log.h>

#include "bml_face.hpp"

#include "mcontrol_util.h"

#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


////// XML ATTRIBUTES
const XMLCh ATTR_AU[]     = L"au";
const XMLCh ATTR_AMOUNT[] = L"amount";
const XMLCh ATTR_SIDE[]   = L"side";	

using namespace std;
using namespace BML;
using namespace xml_utils;



BehaviorRequestPtr BML::parse_bml_face( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	std::string localId = "";
	const XMLCh* id = elem->getAttribute( ATTR_ID);
//	if (id &&  XMLString::stringLen( id ))
	if (id &&  *id != 0 )
		localId = XMLString::transcode(id);

	// Viseme transition hack until timing can support multiple sync points
	const XMLCh* str = elem->getAttribute( L"sbm:rampup" );
	float rampup = 0;
	if( str && *str != 0 ) {
		char* temp = XMLString::transcode(str);
		rampup =( float(atof(temp)));
	}

	str = elem->getAttribute( L"sbm:rampdown" );
	float rampdown = 0;
	if( str && *str != 0 ) {
		char* temp = XMLString::transcode(str);
		rampdown =( float(atof(temp)));
	}

	str = elem->getAttribute( L"sbm:duration" );
	float duration = 1.0;
	if( str && *str != 0 ) {
		char* temp = XMLString::transcode(str);
		duration =( float(atof(temp)));
	}
	
	const XMLCh* attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && *attrType != 0 ) {
        int type = -1;

        if( XMLString::compareIString( attrType, L"facs" )==0 ) {
            const XMLCh* attrAu = elem->getAttribute( ATTR_AU );
            if( attrAu && *attrAu != 0 ) {
                wistringstream inAu( attrAu );
                int au;
                if( inAu >> au ) {
                    float weight = 0.5f;
                    const XMLCh* attrAmount = elem->getAttribute( ATTR_AMOUNT );
                    if( attrAmount && *attrAmount != 0 ) {
                        if(LOG_BML_VISEMES) LOG( "LOG: BML::parse_bml_face(): FAC has specified weight!\n" );
                        wistringstream inAmount( attrAmount );
                        if( !( inAmount >> weight ) )
						{
							std::wstringstream wstrstr;
							wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_AMOUNT<<"=\""<<attrAmount<<"\" />: Illegal attribute value.";
							LOG(convertWStringToString(wstrstr.str()).c_str());
						}	
                         
                    }
                    if(LOG_BML_VISEMES) LOG( "LOG: BML::parse_bml_face(): FAC weight: %f\n", weight );
					boost::shared_ptr<VisemeRequest> viseme;
					viseme.reset( new VisemeRequest( unique_id, localId, "_", weight, duration, behav_syncs, rampup, rampdown ) );

					const XMLCh* attrSide = elem->getAttribute( ATTR_SIDE );
					std::string visemeSide = std::string(XMLString::transcode(attrSide));
					std::string auString = std::string(XMLString::transcode(attrAu));
					std::string visemeNameString;
					int sideSignal = 0;
					if (XMLString::compareIString( attrSide, L"left" )==0)	sideSignal = 1;
					if (XMLString::compareIString( attrSide, L"right" )==0)	sideSignal = 1;
					if (attrSide && *attrSide != 0)
					{
						if (sideSignal)
							visemeNameString = "au_" + auString + "_" + visemeSide;
						else
						{
							std::wstringstream wstrstr;
							wstrstr << "WARNING: BML::parse_bml_face(): Please check the side specification input";
							LOG(convertWStringToString(wstrstr.str()).c_str());
							return BehaviorRequestPtr();
						}
					}
					else
						visemeNameString = "au_" + auString;

					char* visemeName = new char [visemeNameString.size()+1];
					strcpy (visemeName, visemeNameString.c_str());
					viseme->setVisemeName(visemeName);
					return viseme;
                } else {
					std::wstringstream wstrstr;
                    wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_AU<<"=\""<<attrAu<<"\" />: Illegal attribute value.";
					LOG(convertWStringToString(wstrstr.str()).c_str());
					return BehaviorRequestPtr();  // a.k.a., NULL
                }
            } else {
				std::wstringstream wstrstr;
                wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<"> BML tag missing "<<ATTR_AU<<"= attribute.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return BehaviorRequestPtr();  // a.k.a., NULL
            }
        } else if( XMLString::compareIString( attrType, L"eyebrows" )==0 ) {
			std::wstringstream wstrstr;
            wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type.";
			LOG(convertWStringToString(wstrstr.str()).c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
        } else if( XMLString::compareIString( attrType, L"eyelids" )==0 ) {
			std::wstringstream wstrstr;
            wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type.";
			LOG(convertWStringToString(wstrstr.str()).c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
        } else if( XMLString::compareIString( attrType, L"mouth" )==0 ) {
			std::wstringstream wstrstr;
            wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type.";
			LOG(convertWStringToString(wstrstr.str()).c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
        } else {
			std::wstringstream wstrstr;
            wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unknown type value, ignore command";
			LOG(convertWStringToString(wstrstr.str()).c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
        }
    } else {
		std::wstringstream wstrstr;
        wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute.";
		LOG(convertWStringToString(wstrstr.str()).c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
    }
}
