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

					int sideSign = -1;
					const XMLCh* attrSide = elem->getAttribute( ATTR_SIDE );
					if (XMLString::compareIString( attrSide, L"left" )==0)	sideSign = 1;
					if (XMLString::compareIString( attrSide, L"right" )==0)	sideSign = 2;

					std::wstringstream wstrstr;
                    switch( au ) {
                        case 1:
                            viseme->setVisemeName( "unit1_inner_brow_raiser" );
							//viseme->setVisemeName( "au1_inner_brow_raiser" );
							if (sideSign == 1)
								viseme->setVisemeName( "au1_inner_brow_raiser_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au1_inner_brow_raiser_right" );
                            break;
                        case 2:
                            viseme->setVisemeName( "unit2_outer_brow_raiser" );
							//viseme->setVisemeName( "au2_outer_brow_raiser" );
							if (sideSign == 1)
								viseme->setVisemeName( "au2_outer_brow_raiser_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au2_outer_brow_raiser_right" );
                            break;
                        case 4:
                            viseme->setVisemeName( "unit4_inner_brow_lowerer" );
							//viseme->setVisemeName( "au4_brow_lowerer" );
							if (sideSign == 1)
								viseme->setVisemeName( "au4_brow_lowerer_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au4_brow_lowerer_right" );
                            break;
                        case 5:
                            viseme->setVisemeName( "unit5_upper_lid_raiser" );
							//viseme->setVisemeName( "au5_upper_lid_raiser" );
							if (sideSign == 1)
								viseme->setVisemeName( "au5_upper_lid_raiser_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au5_upper_lid_raiser_right" );
                            break;
                        case 6:
                            viseme->setVisemeName( "unit6_eye_squint" );
							//viseme->setVisemeName( "au6_cheek_raiser_and_lid_compressor" );
							if (sideSign == 1)
								viseme->setVisemeName( "au6_cheek_raiser_and_lid_compressor_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au6_cheek_raiser_and_lid_compressor_right" );
                            break;
                        case 7:
                            viseme->setVisemeName( "unit7_lid_tightener" );
							//viseme->setVisemeName( "au7_lid_tightener" );
							if (sideSign == 1)
								viseme->setVisemeName( "au7_lid_tightener_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au7_lid_tightener_right" );
                            break;
						case 8:
                            viseme->setVisemeName( "au8_lips_toward_each_other" );
							if (sideSign == 1)
								viseme->setVisemeName( "au8_lips_toward_each_other_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au8_lips_toward_each_other_left" );
                            break;
                        case 9:
                            viseme->setVisemeName( "unit9_nose_wrinkle" );
							//viseme->setVisemeName( "au9_nose_wrinkler" );
							if (sideSign == 1)
								viseme->setVisemeName( "au9_nose_wrinkler_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au9_nose_wrinkler_right" );
                            break;
                        case 10:
                            viseme->setVisemeName( "unit10_upper_lip_raiser" );
							//viseme->setVisemeName( "au10_upper_lip_raiser" );
							if (sideSign == 1)
								viseme->setVisemeName( "au10_upper_lip_raiser_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au10_upper_lip_raiser_right" );
                            break;
						case 11:
                            viseme->setVisemeName( "au11_nasbolabial_furrow_deepener" );
							if (sideSign == 1)
								viseme->setVisemeName( "au11_nasbolabial_furrow_deepener_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au11_nasbolabial_furrow_deepener_right" );
                            break;
                        case 12:
                            viseme->setVisemeName( "unit12_smile_mouth" );
							//viseme->setVisemeName( "au12_lip_corner_puller" );
							if (sideSign == 1)
								viseme->setVisemeName( "au12_lip_corner_puller_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au12_lip_corner_puller_right" );
                            break;
						case 13:
                            viseme->setVisemeName( "au13_cheek_puffer" );
							if (sideSign == 1)
								viseme->setVisemeName( "au13_cheek_puffer_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au13_cheek_puffer_right" );
                            break;
						case 14:
                            viseme->setVisemeName( "au14_dimpler" );
							if (sideSign == 1)
								viseme->setVisemeName( "au14_dimpler_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au14_dimpler_right" );
                            break;
                        case 15:
                            viseme->setVisemeName( "unit15_lip_corner_depressor" );
							//viseme->setVisemeName( "au15_lip_corner_depressor" );
							if (sideSign == 1)
								viseme->setVisemeName( "au15_lip_corner_depressor_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au15_lip_corner_depressor_right" );
                            break;
						case 16:
                            viseme->setVisemeName( "au16_lower_lip_depressor" );
							if (sideSign == 1)
								viseme->setVisemeName( "au16_lower_lip_depressor_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au16_lower_lip_depressor_right" );
                            break;
						case 17:
                            viseme->setVisemeName( "au17_chin_raiser" );
							if (sideSign == 1)
								viseme->setVisemeName( "au17_chin_raiser_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au17_chin_raiser_right" );
                            break;
						case 18:
                            viseme->setVisemeName( "au18_lip_puckerer" );
							if (sideSign == 1)
								viseme->setVisemeName( "au18_lip_puckerer_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au18_lip_pucker_right" );
                            break;
						case 19:
                            viseme->setVisemeName( "au19_tongue_out" );
							if (sideSign == 1)
								viseme->setVisemeName( "au19_tongue_out_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au19_tongue_out_right" );
                            break;
                        case 20:
                            viseme->setVisemeName( "unit20_mouth_stretch" );
							//viseme->setVisemeName( "au20_lip_stretcher" );
							if (sideSign == 1)
								viseme->setVisemeName( "au20_lip_stretcher_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au20_lip_stretcher_right" );
                            break;
						case 21:
                            viseme->setVisemeName( "au21_neck_tightner" );
							if (sideSign == 1)
								viseme->setVisemeName( "au21_neck_tightner_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au21_neck_tightner_right" );
                            break;
						case 22:
                            viseme->setVisemeName( "au22_lip_funneler" );
							if (sideSign == 1)
								viseme->setVisemeName( "au22_lip_funneler_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au22_lip_funneler_right" );
                            break;
                        case 23:
                            viseme->setVisemeName( "unit23_lip_tightener" );
							//viseme->setVisemeName( "au23_lip_tightener" );
							if (sideSign == 1)
								viseme->setVisemeName( "au23_lip_tightener_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au23_lip_tightener_right" );
                            break;
						case 24:
                            viseme->setVisemeName( "au24_lip_pressor" );
							if (sideSign == 1)
								viseme->setVisemeName( "au24_lip_pressor_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au24_lip_pressor_right" );
                            break;
                        case 25:
                            viseme->setVisemeName( "unit25_lip_parser" );
							//viseme->setVisemeName( "au25_lips_part" );
							if (sideSign == 1)
								viseme->setVisemeName( "au25_lips_part_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au25_lips_part_right" );
                            break;
                        case 26:
                            viseme->setVisemeName( "unit26_jaw_drop" );
							//viseme->setVisemeName( "au26_jaw_drop" );
                            break;
                        case 27:
                            viseme->setVisemeName( "unit27_jaw_stretch_open" );
							//viseme->setVisemeName( "au27_mouth_stretch" );
							if (sideSign == 1)
								viseme->setVisemeName( "au27_mouth_stretch_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au27_mouth_stretch_right" );
                            break;
						case 28:
                            viseme->setVisemeName( "au28_lip_suck" );
							if (sideSign == 1)
								viseme->setVisemeName( "au28_lip_suck_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au28_lip_suck_right" );
                            break;
						case 29:
                            viseme->setVisemeName( "au29_jaw_thrust" );
							if (sideSign == 1)
								viseme->setVisemeName( "au29_jaw_thrust_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au29_jaw_thrust_right" );
                            break;
						case 30:
                            viseme->setVisemeName( "au30_jaw_sideways" );
							if (sideSign == 1)
								viseme->setVisemeName( "au30_jaw_sideways_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au30_jaw_sideways_right" );
                            break;
						case 31:
                            viseme->setVisemeName( "au31_jaw_clencher" );
							if (sideSign == 1)
								viseme->setVisemeName( "au31_jaw_clencher_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au31_jaw_clencher_right" );
                            break;
						case 32:
                            viseme->setVisemeName( "au32_lip_bite" );
							if (sideSign == 1)
								viseme->setVisemeName( "au32_lip_bite_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au32_lip_bite_right" );
                            break;
						case 33:
                            viseme->setVisemeName( "au33_cheek_blow" );
							if (sideSign == 1)
								viseme->setVisemeName( "au33_cheek_blow_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au33_cheek_blow_left" );
                            break;
						case 34:
                            viseme->setVisemeName( "au34_cheek_puff" );
							if (sideSign == 1)
								viseme->setVisemeName( "au34_cheek_puff_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au34_cheek_puff_right" );
                            break;
						case 35:
                            viseme->setVisemeName( "au35_cheek_suck" );
							if (sideSign == 1)
								viseme->setVisemeName( "au35_cheek_suck_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au35_cheek_suck_right" );
                            break;
						case 36:
                            viseme->setVisemeName( "au36_tongue_buldge" );
							if (sideSign == 1)
								viseme->setVisemeName( "au36_tongue_buldge_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au36_tongue_buldge_right" );
                            break;
						case 37:
                            viseme->setVisemeName( "au37_lip_wipe" );
							if (sideSign == 1)
								viseme->setVisemeName( "au37_lip_wipe_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au37_lip_wipe_right" );
                            break;
                        case 38:
                            viseme->setVisemeName( "unit38_nostril_dilator" );
							//viseme->setVisemeName( "au38_nostril_dialator" );
							if (sideSign == 1)
								viseme->setVisemeName( "au38_nostril_dialator_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au38_nostril_dialator_right" );
                            break;
                        case 39:
                            viseme->setVisemeName( "unit39_nostril_compressor" );
							//viseme->setVisemeName( "au39_nostril_compressor" );
							if (sideSign == 1)
								viseme->setVisemeName( "au39_nostril_compressor_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au39_nostril_compressor_right" );
                            break;
						case 41:
                            viseme->setVisemeName( "au41_lid_droop" );
							if (sideSign == 1)
								viseme->setVisemeName( "au41_lid_droop_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au41_lid_droop_right" );
                            break;
						case 42:
                            viseme->setVisemeName( "au42_slit" );
							if (sideSign == 1)
								viseme->setVisemeName( "au42_slit_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au42_slit_right" );
                            break;
						case 43:
                            viseme->setVisemeName( "au43_eyes_closed" );
							if (sideSign == 1)
								viseme->setVisemeName( "au43_eyes_closed_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au43_eyes_closed_right" );
                            break;
						case 44:
                            viseme->setVisemeName( "au44_squint" );
							if (sideSign == 1)
								viseme->setVisemeName( "au44_squint_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au44_squint_right" );
                            break;
						case 45:
                            viseme->setVisemeName( "au45_blink" );
							if (sideSign == 1)
								viseme->setVisemeName( "au45_blink_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au45_blink_right" );
                            break;
						case 46:
                            viseme->setVisemeName( "au46_wink" );
							if (sideSign == 1)
								viseme->setVisemeName( "au46_wink_left" );
							if (sideSign == 2)
								viseme->setVisemeName( "au46_wink_right" );
                            break;

                        default:
                            wstrstr << "WARNING: BML::parse_bml_face(): <"<<tag<<" "<<ATTR_AU<<".. />: Unknown action unit #"<<au<<".";
							LOG(convertWStringToString(wstrstr.str()).c_str());
                            viseme.reset();
                    }
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
