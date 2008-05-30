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

#include "mcontrol_util.h"
#include "bml_face.hpp"
#include "xercesc_utils.hpp"


////// XML ATTRIBUTES
const XMLCh ATTR_AU[]     = L"au";
const XMLCh ATTR_AMOUNT[] = L"amount";



using namespace std;
using namespace BML;
using namespace xml_utils;



BehaviorRequest* BML::parse_bml_face( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	// Viseme transition hack until timing can support multiple sync points
	const XMLCh* str = elem->getAttribute( L"sbm:rampup" );
	float rampup = -1;
	if( str && XMLString::stringLen( str ) ) {
		char* temp = XMLString::transcode(str);
		rampup =( float(atof(temp)));
	}

	str = elem->getAttribute( L"sbm:rampdown" );
	float rampdown = -1;
	if( str && XMLString::stringLen( str ) ) {
		char* temp = XMLString::transcode(str);
		rampdown =( float(atof(temp)));
	}

	
	const XMLCh* attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && XMLString::stringLen( attrType ) ) {
        int type = -1;

        if( XMLString::compareIString( attrType, L"facs" )==0 ) {
            const XMLCh* attrAu = elem->getAttribute( ATTR_AU );
            if( attrAu && XMLString::stringLen( attrAu )>0 ) {
                wistringstream inAu( attrAu );
                int au;
                if( inAu >> au ) {
                    float weight = 0.5f;
                    const XMLCh* attrAmount = elem->getAttribute( ATTR_AMOUNT );
                    if( attrAmount && XMLString::stringLen( attrAmount )>0 ) {
                        if(LOG_BML_VISEMES) printf( "LOG: BML::parse_bml_face(): Viseme has specified weight!\n" );
                        wistringstream inAmount( attrAmount );
                        if( !( inAmount >> weight ) )
                            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_AMOUNT<<"=\""<<attrAmount<<"\" />: Illegal attribute value."<<endl;
                    }
                    if(LOG_BML_VISEMES) printf( "LOG: BodyPlannerImpl::parseBML(): Viseme weight: %f\n", weight );
                    VisemeRequest* viseme;
					if (rampup != -1 && rampdown != -1) viseme = new VisemeRequest( "_", weight, 1, tms.start, tms.ready, tms.stroke, tms.relax, tms.end, rampup, rampdown );
					else viseme = new VisemeRequest( "_", weight, 1, tms.start, tms.ready, tms.stroke, tms.relax, tms.end );

                    switch( au ) {
                        case 1:
                            viseme->setVisemeName( "unit1_inner_brow_raiser" );
                            break;
                        case 2:
                            viseme->setVisemeName( "unit2_outer_brow_raiser" );
                            break;
                        case 4:
                            viseme->setVisemeName( "unit4_inner_brow_lowerer" );
                            break;
                        case 5:
                            viseme->setVisemeName( "unit5_upper_lid_raiser" );
                            break;
                        case 6:
                            viseme->setVisemeName( "unit6_eye_squint" );
                            break;
                        case 7:
                            viseme->setVisemeName( "unit7_lid_tightener" );
                            break;
                        case 9:
                            viseme->setVisemeName( "unit9_nose_wrinkle" );
                            break;
                        case 10:
                            viseme->setVisemeName( "unit10_upper_lip_raiser" );
                            break;
                        case 12:
                            viseme->setVisemeName( "unit12_smile_mouth" );
                            break;
                        case 15:
                            viseme->setVisemeName( "unit15_lip_corner_depressor" );
                            break;
                        case 20:
                            viseme->setVisemeName( "unit20_mouth_stretch" );
                            break;
                        case 23:
                            viseme->setVisemeName( "unit23_lip_tightener" );
                            break;
                        case 25:
                            viseme->setVisemeName( "unit25_lip_parser" );
                            break;
                        case 26:
                            viseme->setVisemeName( "unit26_jaw_drop" );
                            break;
                        case 27:
                            viseme->setVisemeName( "unit27_jaw_stretch_open" );
                            break;
                        case 38:
                            viseme->setVisemeName( "unit38_nostril_dilator" );
                            break;
                        case 39:
                            viseme->setVisemeName( "unit39_nostril_compressor" );
                            break;

                        default:
                            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_AU<<".. />: Unknown action unit #"<<au<<"."<<endl;
                            delete viseme;
                            viseme = NULL;
                    }
					return viseme;
                } else {
                    wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_AU<<"=\""<<attrAu<<"\" />: Illegal attribute value."<<endl;
					return NULL;
                }
            } else {
                wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_AU<<"= attribute." << endl;
				return NULL;
            }
        } else if( XMLString::compareIString( attrType, L"eyebrows" )==0 ) {
            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type." << endl;
			return NULL;
        } else if( XMLString::compareIString( attrType, L"eyelids" )==0 ) {
            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type." << endl;
			return NULL;
        } else if( XMLString::compareIString( attrType, L"mouth" )==0 ) {
            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unimplemented type." << endl;
			return NULL;
        } else {
            wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unknown type value, ignore command" << endl;
			return NULL;
        }
    } else {
        wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute." << endl;
		return NULL;
    }
}
