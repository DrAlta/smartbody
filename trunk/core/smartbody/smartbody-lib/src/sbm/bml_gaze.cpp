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

#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include "mcontrol_util.h"
#include "bml_gaze.hpp"
#include "me_ct_gaze.h"
#include "bml_target.hpp"
#include "xercesc_utils.hpp"


#define LOG_GAZE_PARAMS				(0)
#define DEBUG_BML_GAZE				(0)
#define DEBUG_JOINT_RANGE			(0)
#define DEBUG_GAZE_KEYS				(0)
#define DEBUG_DESCRIPTION_LEVELS	(0)


////// XML Tags
const XMLCh TAG_DESCRIPTION[] = L"description";


////// BML Description Type
const XMLCh DTYPE_SBM[]  = L"ISI.SBM";


////// XML ATTRIBUTES
const XMLCh ATTR_ANGLE[]        = L"angle";
const XMLCh ATTR_DIRECTION[]    = L"direction";
const XMLCh ATTR_SBM_ROLL[]     = L"sbm:roll";
const XMLCh ATTR_JOINT_RANGE[]  = L"sbm:joint-range";
const XMLCh ATTR_JOINT_SPEED[]  = L"sbm:joint-speed";
const XMLCh ATTR_JOINT_SMOOTH[] = L"sbm:speed-smoothing";
const XMLCh ATTR_PITCH[]        = L"pitch";
const XMLCh ATTR_HEADING[]      = L"heading";
const XMLCh ATTR_ROLL[]         = L"roll";
const XMLCh ATTR_BLEND[]        = L"blend";
const XMLCh ATTR_INTERPOLATE_BIAS[] = L"interpolate-bias";

const XMLCh ATTR_PRIORITY_JOINT[] = L"sbm:priority-joint";
const XMLCh ATTR_PITCH_MIN[] = L"pitch-min";
const XMLCh ATTR_PITCH_MAX[] = L"pitch-max";

////// XML Direction constants
// Angular (gaze) and orienting (head)
const XMLCh DIR_RIGHT[]        = L"RIGHT";
const XMLCh DIR_LEFT[]         = L"LEFT";
const XMLCh DIR_UP[]           = L"UP";
const XMLCh DIR_DOWN[]         = L"DOWN";
// Angular only
const XMLCh DIR_UPRIGHT[]      = L"UPRIGHT";
const XMLCh DIR_UPLEFT[]       = L"UPLEFT";
const XMLCh DIR_DOWNRIGHT[]    = L"DOWNRIGHT";
const XMLCh DIR_DOWNLEFT[]     = L"DOWNLEFT";
const XMLCh DIR_POLAR[]        = L"POLAR";




namespace BML {
	namespace Gaze {
		//  Old Hard-Coded Defaults 
		//const float DEFAULT_SPEED_LUMBAR    = (float)1000;
		//const float DEFAULT_SPEED_CERVICAL  = (float)1500;
		//const float DEFAULT_SPEED_EYEBALL   = (float)2000;
		//const float DEFAULT_SMOOTH_LUMBAR   = (float)0.8;
		//const float DEFAULT_SMOOTH_CERVICAL = (float)0.8;
		//const float DEFAULT_SMOOTH_EYEBALL  = (float)0.1;

		// Declare and initialize variable runtime defaults
		// New Defaults imported from storage in me_ct_gaze.h

		float speed_lumbar    = (float)(MeCtGaze::gaze_defaults->speed_head / 2.0);  // See void MeCtGaze::set_speed( float back_dps, float neck_dps, float eyes_dps ) in me_ct_gaze.cpp
		float speed_cervical  = (float)(MeCtGaze::gaze_defaults->speed_head / 2.0);  // As of 10/2008 speed is defined with two values:  head & eyes
		float speed_eyeball   = MeCtGaze::gaze_defaults->speed_eyes;
		float smooth_lumbar   = MeCtGaze::gaze_defaults->smooth_back;
		float smooth_cervical = MeCtGaze::gaze_defaults->smooth_neck;
		float smooth_eyeball  = MeCtGaze::gaze_defaults->smooth_eyes;


		/**
		 *  Contains the possible values for a gaze key
		 */
		struct KeyData {
			SkJoint* target;

			float bias_pitch;
			float pitch_min;
			float pitch_max;

			float bias_heading;
			float bias_roll;

			bool interpolate_bias;

			float blend_weight;

			KeyData()
			:	bias_pitch( 0.0 ),
				bias_heading( 0.0 ),
				bias_roll( 0.0 ),
				interpolate_bias( true ),

				blend_weight( 1.0 ),
				pitch_min( -1.0 ),
				pitch_max( -1.0 )  //used as flags to indicate non-initialization.
			{}
		};


		void parse_gaze_key_element( DOMElement* elem, Gaze::KeyData* key_data );

		bool parse_children( DOMElement* elem, Gaze::KeyData* key_data[] );
	};
};

using namespace std;
using namespace BML;
using namespace xml_utils;


ostream& operator<<( ostream& os, const Gaze::KeyData key_data ) {
	os <<"KeyData: heading="<<key_data.bias_heading
		      <<", pitch="<<key_data.bias_pitch
			  <<", roll="<<key_data.bias_roll
			  <<", interpolate-bias="<<(key_data.interpolate_bias?"true":"false")
			  <<", blend="<<key_data.blend_weight;
	return os;
}

int check_gaze_speed( float lumbar, float cervical, float eyeball ) {
	if( lumbar <= 0 || cervical <= 0 || eyeball <= 0 ) {
		cerr << "ERROR: Gaze joint speed cannot be <= 0." << endl;
		return false;
	}
	// TODO: Print warning on extremely slow / fast speeds.
	return true;
}

int BML::Gaze::set_gaze_speed( float lumbar, float cervical, float eyeball ) {
	if( check_gaze_speed( lumbar, cervical, eyeball ) ) {
		BML::Gaze::speed_lumbar   = lumbar;
		BML::Gaze::speed_cervical = cervical;
		BML::Gaze::speed_eyeball  = eyeball;

		return CMD_SUCCESS;
	} else {
		return CMD_FAILURE;
	}
}

void BML::Gaze::print_gaze_speed() {
	cout << "BML Processor default gaze joint speed (degrees per second):" << endl
		 << "\tlumbar = " << BML::Gaze::speed_lumbar << endl
		 << "\tcervical = " << BML::Gaze::speed_cervical << endl
		 << "\teyeballs = " << BML::Gaze::speed_eyeball << endl;
}

bool check_gaze_smoothing( float lumbar, float cervical, float eyeball ) {
	if( lumbar < 0 || cervical < 0 || eyeball < 0 ) {
		cerr << "ERROR: Gaze speed smoothing cannot be < 0." << endl;
		return false;
	}
	if( lumbar > 1 || cervical > 1 || eyeball > 1 ) {
		cerr << "ERROR: Gaze speed smoothing cannot be > 1." << endl;
		return false;
	}
	return true;
}

int BML::Gaze::set_gaze_smoothing( float lumbar, float cervical, float eyeball ) {
	if( check_gaze_speed( lumbar, cervical, eyeball ) ) {
		BML::Gaze::smooth_lumbar   = lumbar;
		BML::Gaze::smooth_cervical = cervical;
		BML::Gaze::smooth_eyeball  = eyeball;

		return CMD_SUCCESS;
	} else {
		return CMD_FAILURE;
	}
}

void BML::Gaze::print_gaze_smoothing() {
	cout << "BML Processor default gaze speed smoothing (0 to 1):" << endl
		 << "\tlumbar = " << BML::Gaze::smooth_lumbar << endl
		 << "\tcervical = " << BML::Gaze::smooth_cervical << endl
		 << "\teyeballs = " << BML::Gaze::smooth_eyeball << endl;
}

void BML::Gaze::parse_gaze_key_element( DOMElement* elem, Gaze::KeyData* key_data ) {
	const XMLCh* value = elem->getAttribute( ATTR_PITCH );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> key_data->bias_pitch ) ) {
			cerr << "WARNING: Failed to parse pitch attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_HEADING );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> key_data->bias_heading ) ) {
			cerr << "WARNING: Failed to parse heading attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_ROLL );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> key_data->bias_roll ) ) {
			cerr << "WARNING: Failed to parse roll attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_BLEND );
	if( value!=NULL && value[0]!='\0' ) {
		if( !( wistringstream( value ) >> key_data->blend_weight ) ) {
			cerr << "WARNING: Failed to parse blend attribute \""<<XMLString::transcode(value)<<"\" of <"<<XMLString::transcode( elem->getTagName() )<<" .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_PITCH_MIN );
	if(value != NULL && value[0] != '\0') 
	{
		if( !( wistringstream( value ) >> key_data->pitch_min ) )
		{
			cerr << "WARNING: Failed to parse minimum pitch attribute \""<< XMLString::transcode(value) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_PITCH_MAX );
	if(value != NULL && value[0] != '\0') 
	{
		if( !( wistringstream( value ) >> key_data->pitch_max ) )
		{
			cerr << "WARNING: Failed to parse maximum pitch attribute \""<< XMLString::transcode(value) <<"\" of <"<< XMLString::transcode(elem->getTagName()) << " .../> element." << endl;
		}
	}

	value = elem->getAttribute( ATTR_INTERPOLATE_BIAS );
	if( value!=NULL && value[0]!='\0' ) {
		string s( XMLString::transcode( value ) );
		if( DEBUG_GAZE_KEYS ) cout << "interpolate-bias=\"" << s << "\"" << endl;
		for( string::size_type i=0; i<s.length(); ++i ) {  // Isn't there an easier way in std:: to convert strings to uppercase?
			s[i] = toupper( s[i] );
		}
		if( DEBUG_GAZE_KEYS ) cout << "interpolate-bias (uppercase) =\"" << s << "\"" << endl;
		key_data->interpolate_bias = ( s.find( "TRUE" ) != string::npos );
		if( DEBUG_GAZE_KEYS ) cout << "key_data->interpolate_bias=" << (key_data->interpolate_bias? "true": "false") << endl;
	}
}


bool BML::Gaze::parse_children( DOMElement* elem, Gaze::KeyData* key_data[] ) {
	bool has_data = false;

	DOMElement* description = NULL;
	int description_level = -1;
	DOMElement* child = getFirstChildElement( elem );
	if( DEBUG_DESCRIPTION_LEVELS && child!=NULL ) cout << "BML::Gaze::parse_children(..): <gaze ../> has child elements." << endl;
	while( child != NULL ) {  // TODO: Need BML function to order all description levels of a behavior tag.
		wstring child_tag = child->getTagName();
		if( DEBUG_DESCRIPTION_LEVELS ) wcout << "\tchild_tag = \""<< child_tag << "\"" << endl;
		if( child_tag == TAG_DESCRIPTION ) {
			wstring description_type = child->getAttribute( ATTR_TYPE );
			if( DEBUG_DESCRIPTION_LEVELS ) wcout << "\tdescription_type = \""<< description_type << "\"" << endl;
			if( description_type == DTYPE_SBM ) {
				string level_str = XMLString::transcode( child->getAttribute( ATTR_LEVEL ) );
				int child_level = 0;
				if( level_str.length() > 0 ) {
					if( DEBUG_DESCRIPTION_LEVELS ) cout << "\tlevel_str = \""<< level_str << "\"" << endl;
					istringstream iss;
					iss.str( level_str );
					if( iss >> child_level ) {  // if level_str is a valid integer
						if( child_level > description_level ) {
							if( DEBUG_DESCRIPTION_LEVELS ) wcout << "\tFound higher-leveled description." << endl;

							description = child;
							description_level = child_level;
						}
					} else {
						cout << "WARNING: Invalid level number \""<<level_str<<"\" in gaze description." << endl;
					}
				} else {
					if( DEBUG_DESCRIPTION_LEVELS ) wcout << "\tMissing level attribute." << endl;
				}
			} // end type == DTYPE_SBM
		} else if( child_tag.find( L"sbm:" ) == 0 ) {
			// Begins with SBM:: namespace prefix
			const char* gaze_key_name = XMLString::transcode( child_tag.substr( 4 ).c_str() );  // transcode element's local name
			int key = MeCtGaze::key_index( gaze_key_name );

			if( key != -1 ) {
				has_data = true;
				if( key_data[key] == NULL ) {
					key_data[key] = new Gaze::KeyData();
				} else {
					wcerr << "WARNING: BML::Gaze::parse_children(..): Gaze joint element \""<<child_tag<<"\" overwriting existing KeyData." << endl;
				}

				parse_gaze_key_element( child, key_data[key] );
			} else {
				wcerr << "WARNING: Unrecognized <"<<child_tag<<" ../> element inside gaze behavior.  Ignoring element." << endl;
			}
		} else {
			wcerr << "WARNING: Unrecognized <"<<child_tag<<" ../> inside element gaze behavior.  Ignoring element." << endl;
		}
		
		child = getNextElement( child );
	}

	if( description != NULL ) {
		if( DEBUG_DESCRIPTION_LEVELS ) cout << "\tFound description" << endl;
		
		child = getFirstChildElement( description );
		while( child != NULL ) {
			// Lazily parse this by directly treating the tag name as the gaze key,
			// regardless of capitalization, etc.
			wstring child_tag = child->getTagName();
			const char* gaze_key_name = XMLString::transcode( child_tag.c_str() );  // transcode tag name
			int key = MeCtGaze::key_index( gaze_key_name );

			if( key != -1 ) {
				has_data = true;
				if( key_data[key] == NULL ) {
					key_data[key] = new Gaze::KeyData();
				} else {
					wcerr << "WARNING: BML::Gaze::parse_children(..): Gaze description \""<<child_tag<<"\" overwriting existing KeyData." << endl;
				}

				parse_gaze_key_element( child, key_data[key] );
			} else {
				wcerr << "WARNING: Unrecognized <" << child_tag << " ../> element inside \"isi:sbm\" typed gaze description level.  Ignoring element." << endl;
			}

			child = getNextElement( child );
		}
	}

	return has_data;
}



BehaviorRequest* BML::parse_bml_gaze( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
	////////////////////////////////////////////////////////////////
	//  GAZE BEHAVIORS

	const XMLCh* attrTarget = elem->getAttribute( ATTR_TARGET );
	if( !attrTarget || !XMLString::stringLen( attrTarget ) ) {
        wcerr << "WARNING: BML::parse_bml_gaze(): <"<<tag<<"> BML tag missing "<<ATTR_TARGET<<"= attribute." << endl;
		return NULL;
    }

	const SkJoint* joint = parse_target( tag, attrTarget, mcu );
	if( joint == NULL ) {  // Invalid target.  Assume parse_target(..) printed error.
		return NULL;
	}


	/////////////////////////////////////////////////////////////
	//  Parse <description type="ISI.SBM"> and gaze key elements if present
	Gaze::KeyData* key_data[ MeCtGaze::NUM_GAZE_KEYS ];
	for( int i=0; i<MeCtGaze::NUM_GAZE_KEYS; ++i )  // necessary?
		key_data[i] = NULL;
	bool has_key_data = Gaze::parse_children( elem, key_data );

	if( DEBUG_GAZE_KEYS ) {
		for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key ) {
			if( key_data[key] != NULL ) {
				if( DEBUG_GAZE_KEYS ) cout << "BML::parse_bml_gaze(..): Gaze key "<<key<<": " << *( key_data[key] ) << endl;
			}
		}
	}


	/////////////////////////////////////////////////////////////
	// Identify the low and high gaze key indices
	int low_key_index  = MeCtGaze::GAZE_KEY_LUMBAR;
	int high_key_index = MeCtGaze::GAZE_KEY_EYES;

	const XMLCh* attrJointRange = elem->getAttribute( ATTR_JOINT_RANGE );
	if( attrJointRange && XMLString::stringLen( attrJointRange ) ) {
		//  Parse sbm:joint-range
		XMLStringTokenizer tokenizer( attrJointRange, L" \r\n\t\f" );  // include the dash to delimit ranges
		if( tokenizer.countTokens()==0 ) {
			cerr << "ERROR: No valid tokens in <gaze ../> behavior attribute "<<ATTR_JOINT_RANGE<<endl;
		} else {
			const char* key_name = asciiString( tokenizer.nextToken() );
			int key_index = MeCtGaze::key_index( key_name );
			if( key_index == -1 ) {
				cerr << "WARNING: BML::parse_bml_gaze(..): Invalid joint range token \""<<key_name<<"\"."<<endl;
			}
			delete [] key_name;

			while( key_index == -1 && tokenizer.countTokens() > 0 ) {  // find first valid key
				key_name = asciiString( tokenizer.nextToken() );
				key_index = MeCtGaze::key_index( key_name );
				if( key_index == -1 ) {
					cerr << "WARNING: BML::parse_bml_gaze(..): Invalid joint range token \""<<key_name<<"\"."<<endl;
				}
				delete [] key_name;
			}
			if( key_index != -1 ) {  // found a valid key
				low_key_index = high_key_index = key_index;

				while( tokenizer.countTokens() > 0 ) {
					key_name = asciiString( tokenizer.nextToken() );
					key_index = MeCtGaze::key_index( key_name );
					if( key_index != -1 ) {
						if( key_index < low_key_index )
							low_key_index = key_index;
						else if( key_index > high_key_index ) 
							high_key_index = key_index;
					} else {
						cerr << "WARNING: BML::parse_bml_gaze(..): Invalid joint range token \""<<key_name<<"\"."<<endl;
					}
					delete [] key_name;
				}
			}

		}
	}
	if( DEBUG_JOINT_RANGE ) {
		cout << "DEBUG: BML::parse_bml_gaze(..): "
			<< "low_key_index = "<<low_key_index<<",\t"
			<< "high_key_index = "<<high_key_index<<endl;
	}
	
	/////////////////////////////////////////////////////////////
	//  Parse sbm:priority-joint attribute
	int priority_key_index = high_key_index;

	const XMLCh* attrPriority = elem->getAttribute( ATTR_PRIORITY_JOINT );
	if( attrPriority && XMLString::stringLen(attrPriority) > 0 ) 
	{
		const char* priority_key_name = asciiString(attrPriority);
		priority_key_index = MeCtGaze::key_index(priority_key_name);
		if(priority_key_index < low_key_index)
		{
			priority_key_index = low_key_index;
			cerr << "WARNING: BML::parse_bml_gaze(..): Invalid priority key attribute\"" << priority_key_name << "\"." << endl;
		}
		if(priority_key_index > high_key_index)
		{
			priority_key_index = high_key_index;
			cerr << "WARNING: BML::parse_bml_gaze(..): Invalid priority key attribute\"" << priority_key_name << "\"." << endl;
		}
	}

	/////////////////////////////////////////////////////////////
	//  Parse attributes sbm:joint-speed and sbm:speed-smoothing

	// default gaze values...
	
	float gaze_speed_lumbar   = BML::Gaze::speed_lumbar;
	float gaze_speed_cervical = BML::Gaze::speed_cervical;
	float gaze_speed_eyeball  = BML::Gaze::speed_eyeball;
	float gaze_smooth_lumbar   = BML::Gaze::smooth_lumbar;
	float gaze_smooth_cervical = BML::Gaze::smooth_cervical;
	float gaze_smooth_eyeball  = BML::Gaze::smooth_eyeball;
	

	// parse sbm:joint-speed
	const XMLCh* attrSpeed = elem->getAttribute( ATTR_JOINT_SPEED );
	if( attrSpeed && XMLString::stringLen( attrSpeed ) ) {
		// Ugly mix of XMLStringTokenizer and streams to get a token count before parsing
		XMLStringTokenizer tokenizer( attrSpeed );
		switch( tokenizer.countTokens() ) {
			case 3: {
				float values[3];
				wistringstream in;
				bool  valid = !in.fail();

				int i=0;
				XMLCh* token;
				for( ; valid && i<3; ++i ) {
					token = tokenizer.nextToken();
					in.clear();
					in.str( token );
					in.seekg(0);
					valid = !( in >> values[i] ).fail();
				}
				if( valid ) {
					if( check_gaze_speed( values[0], values[1], values[2] ) ) {
						gaze_speed_lumbar   = values[0];
						gaze_speed_cervical = values[1];
						gaze_speed_eyeball  = values[2];
					}
				} else {
					wcerr << "WARNING: Expected three numerical tokens in gaze behavior attribute " << ATTR_JOINT_SPEED << "=\"" << attrSpeed << "\"."
						<< "  Unable to parse token "<<i<<" \""<<token<<"\" ("<<in.rdstate()<<": "<<(in.bad()?"BAD ":"")<<(in.fail()?"FAIL ":"")<<(in.eof()?"EOF":"")<<").  Ignoring attribute." << endl;
				}

				break;
			}
			default:
				wcerr << "WARNING: Expected three numerical tokens in gaze behavior attribute " << ATTR_JOINT_SPEED << "=\"" << attrSpeed << "\".  Found " << tokenizer.countTokens() << ".  Ignoring attribute." << endl;
				break;						
		}
	}

	// parse sbm:speed-smoothing
	const XMLCh* attrSmooth = elem->getAttribute( ATTR_JOINT_SMOOTH );
	if( attrSmooth && XMLString::stringLen( attrSmooth ) ) {
		// Ugly mix of XMLStringTokenizer and streams to get a token count before parsing
		XMLStringTokenizer tokenizer( attrSmooth );
		switch( tokenizer.countTokens() ) {
			case 3: {
				float values[3];
				wistringstream in;

				bool valid = !in.fail();
				XMLCh* token = NULL;
				int i=0;
				for( ; valid && i<3; ++i ) {
					token = tokenizer.nextToken();
					in.clear();
					in.str( token );
					in.seekg(0);
					valid = !( in >> values[i] ).fail();
				}
				if( valid ) {
					if( check_gaze_smoothing( values[0], values[1], values[2] ) ) {
						gaze_smooth_lumbar   = values[0];
						gaze_smooth_cervical = values[1];
						gaze_smooth_eyeball  = values[2];
					}
				} else {
					wcerr << "WARNING: Expected three numerical tokens in gaze behavior attribute " << ATTR_JOINT_SPEED << "=\"" << attrSpeed << "\"."
						<< "  Unable to parse token "<<i<<" \""<<token<<"\" ("<<in.rdstate()<<": "<<(in.bad()?"BAD ":"")<<(in.fail()?"FAIL ":"")<<(in.eof()?"EOF":"")<<").  Ignoring attribute." << endl;
				}

				break;
			}
			default:
				wcerr << "WARNING: Expected three numerical tokens in gaze behavior attribute " << ATTR_JOINT_SMOOTH << "=\"" << attrSpeed << "\".  Found " << tokenizer.countTokens() << ".  Ignoring attribute." << endl;
				break;						
		}
	}

	if( LOG_GAZE_PARAMS ) {
		cout << "DEBUG: Gaze parameters:" << endl
				<< "\tgaze_speed_lumbar = " << gaze_speed_lumbar << endl
				<< "\tgaze_speed_cervical = " << gaze_speed_cervical << endl
				<< "\tgaze_speed_eyeball = " << gaze_speed_eyeball << endl
				<< "\tgaze_smooth_lumbar = " << gaze_smooth_lumbar << endl
				<< "\tgaze_smooth_cervical = " << gaze_smooth_cervical << endl
				<< "\tgaze_smooth_eyeball = " << gaze_smooth_eyeball << endl;
	}

	MeCtGaze* gaze_ct = new MeCtGaze();
	gaze_ct->init( low_key_index, high_key_index );
	gaze_ct->set_target_joint( 0, 0, 0, const_cast<SkJoint*>(joint) );
	gaze_ct->set_task_priority( priority_key_index );
	gaze_ct->set_speed( gaze_speed_lumbar, gaze_speed_cervical, gaze_speed_eyeball );
	gaze_ct->set_smooth( gaze_smooth_lumbar, gaze_smooth_cervical, gaze_smooth_eyeball );
	float pitch_minimum, pitch_maximum;

	if( has_key_data ) {    //if there is key data
		if( key_data[ low_key_index ]==NULL ) {  
			key_data[ low_key_index ] = new Gaze::KeyData();  
		}
		if( key_data[ high_key_index ]==NULL ) { 
			key_data[ high_key_index ] = new Gaze::KeyData(); 
		} else if( high_key_index == MeCtGaze::GAZE_KEY_EYES ) {  
			key_data[ MeCtGaze::GAZE_KEY_EYES ]->bias_roll = 0;  
		}

		int key = low_key_index;   //start at the low key index
		Gaze::KeyData* data = key_data[key];
		if( low_key_index < high_key_index ) 
		{  
			int next_key = key + 1;  
			Gaze::KeyData* next_data; 
			while( next_key < high_key_index ) 
			{  
				if( key_data[next_key] != NULL ) 
				{  
					next_data = key_data[next_key];

					if( data->interpolate_bias && next_data->interpolate_bias ) 
					{
						gaze_ct->set_bias_pitch(   key, next_key, data->bias_pitch,   next_data->bias_pitch );
						gaze_ct->set_bias_heading( key, next_key, data->bias_heading, next_data->bias_heading );
						gaze_ct->set_bias_roll(    key, next_key, data->bias_roll,    next_data->bias_roll );
					} else { 
						gaze_ct->set_bias( key, data->bias_pitch,
												data->bias_heading,
							  					data->bias_roll );
					}
					gaze_ct->set_blend( key, data->blend_weight );

					pitch_minimum = (data->pitch_min < 0) ? gaze_ct->gaze_defaults->pitch_dn[key] : data->pitch_min;
					pitch_maximum = (data->pitch_max < 0) ? gaze_ct->gaze_defaults->pitch_up[key] : data->pitch_max;
					gaze_ct->set_limit( key, pitch_maximum, pitch_minimum, gaze_ct->gaze_defaults->heading[key], gaze_ct->gaze_defaults->roll[key]);

					key  = next_key;
					data = next_data;
				} // if( key_data[next_key] != NULL )
				++next_key;
			} // while( next_key < high_key_index ) 

			next_data = key_data[next_key];  // last key
			if( data->interpolate_bias && next_data->interpolate_bias ) {
				gaze_ct->set_bias_pitch(   key, next_key, data->bias_pitch,   next_data->bias_pitch );
				gaze_ct->set_bias_heading( key, next_key, data->bias_heading, next_data->bias_heading );
				gaze_ct->set_bias_roll(    key, next_key, data->bias_roll,    next_data->bias_roll );
			} else {
				gaze_ct->set_bias( key, data->bias_pitch,
										data->bias_heading,
										data->bias_roll );
				gaze_ct->set_bias( next_key, next_data->bias_pitch,
										     next_data->bias_heading,
										     next_data->bias_roll );
			}
			gaze_ct->set_blend( key, data->blend_weight );
			gaze_ct->set_blend( next_key, next_data->blend_weight );

			pitch_minimum = (data->pitch_min < 0) ? gaze_ct->gaze_defaults->pitch_dn[key] : data->pitch_min;
			pitch_maximum = (data->pitch_max < 0) ? gaze_ct->gaze_defaults->pitch_up[key] : data->pitch_max;
			gaze_ct->set_limit( key, pitch_maximum, pitch_minimum, gaze_ct->gaze_defaults->heading[key], gaze_ct->gaze_defaults->roll[key]);

			pitch_minimum = (next_data->pitch_min < 0) ? gaze_ct->gaze_defaults->pitch_dn[key] : next_data->pitch_min;
			pitch_maximum = (next_data->pitch_max < 0) ? gaze_ct->gaze_defaults->pitch_up[key] : next_data->pitch_max;
			gaze_ct->set_limit( next_key, pitch_maximum, pitch_minimum, gaze_ct->gaze_defaults->heading[key], gaze_ct->gaze_defaults->roll[key]);

		} else { // if( low_key_index < high_key_index )
			// Only one gaze key
			gaze_ct->set_bias( key, data->bias_pitch,
									data->bias_heading,
									data->bias_roll );
			gaze_ct->set_blend( key, data->blend_weight );

			pitch_minimum = (data->pitch_min < 0) ? gaze_ct->gaze_defaults->pitch_dn[key] : data->pitch_min;
			pitch_maximum = (data->pitch_max < 0) ? gaze_ct->gaze_defaults->pitch_up[key] : data->pitch_max;
			gaze_ct->set_limit( key, pitch_maximum, pitch_minimum, gaze_ct->gaze_defaults->heading[key], gaze_ct->gaze_defaults->roll[key]);


		}
	}

	float roll = 0;
	const XMLCh* attrRoll = elem->getAttribute( ATTR_SBM_ROLL );
	if( attrRoll && XMLString::stringLen(attrRoll)>0 ) {
		if( !( wistringstream( attrRoll ) >> roll ) ) {
			wcerr << "WARNING: BML::parse_bml_gaze(): Expected float for "<<ATTR_SBM_ROLL<<" attribute \"" << attrRoll << "\"." << endl;
		}
	}

	// New code: uses set_offset_polar
	const XMLCh* attrDirection = elem->getAttribute( ATTR_DIRECTION );
	if( attrDirection && XMLString::stringLen(attrDirection)>0 ) {
		wistringstream dir_in( attrDirection );
		wstring token;
		if( dir_in >> token ) {
			float angle = 45;  // Over-exagerated default until further analysis

			const XMLCh* attrAngle = elem->getAttribute( ATTR_ANGLE );
			if( attrAngle && XMLString::stringLen(attrAngle)>0 ) {
				if( !( wistringstream( attrAngle ) >> angle ) ) {
					wcerr << "WARNING: BML::parse_bml_gaze(): Expected float for angle attribute \"" << attrAngle << "\"." << endl;
				}
			} else {
				cerr << "WARNING: BML::parse_bml_gaze(): Found direction attribute, but no angle attribute.  Assuming angle " << angle << "\"." << endl;
			}

			float dir_angle;
			bool parse_gaze_direction = true;  // future function name and success value
			if( token.compare( 0, XMLString::stringLen(DIR_POLAR), DIR_POLAR )==0 ) {
				if( !( dir_in >> dir_angle ) ) {
					wcerr << "WARNING: BML::parse_bml_gaze(): Expected float for \"POLAR direction attribute \"" << attrAngle << "\"." << endl;
				}
			} else if( token==DIR_UP ) {
				dir_angle = 0;
			} else if( token==DIR_UPRIGHT ) {
				dir_angle = 45;
			} else if( token==DIR_RIGHT ) {
				dir_angle = 90;
			} else if( token==DIR_DOWNRIGHT ) {
				dir_angle = 135;
			} else if( token==DIR_DOWN ) {
				dir_angle = 180;
			} else if( token==DIR_DOWNLEFT ) {
				dir_angle = 225;
			} else if( token==DIR_LEFT ) {
				dir_angle = 270;
			} else if( token==DIR_UPLEFT ) {
				dir_angle = 315;
			} else {
				wcerr << "WARNING: BML::parse_bml_gaze(): Unrecognized gaze direction \""<<attrDirection<<"\".  Direction ignored."<< endl;
				parse_gaze_direction = false;
			}

			if( parse_gaze_direction ) {  // parsed successfully?
				gaze_ct->set_offset_polar( dir_angle, angle, roll );
			}
		} else {
			wcerr << "WARNING: BML::parse_bml_gaze(): Failed to parse direction attribute " << attrDirection << "\"." << endl;
		}
	} else if( roll != 0 ) {
		gaze_ct->set_offset_polar( 0, 0, roll );
	}

	return new MeControllerRequest( MeControllerRequest::GAZE, gaze_ct, tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
}
