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

#include "vhcl.h"
#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>
#include <sbm/me_ct_navigation_circle.hpp>
#include <me/me_ct_channel_writer.hpp>
#include <sbm/me_ct_param_animation.h>

#include "bml_locomotion.hpp"
#include "bml_event.hpp"

#include "mcontrol_util.h"
#include "me_ct_locomotion.hpp"

#include "bml_xml_consts.hpp"
#include "xercesc_utils.hpp"


////// XML ATTRIBUTES
const XMLCh ATTR_TARGET[]     = L"target";
const XMLCh ATTR_TYPE[]     = L"type";
const XMLCh ATTR_DIRECTION[]     = L"direction";
const XMLCh ATTR_ROTATION[]     = L"rotation";
const XMLCh ATTR_ENABLE[]     = L"enable";
const XMLCh ATTR_VELOCITY[]     = L"velocity";
const XMLCh ATTR_SPEED[]     = L"speed";
const XMLCh ATTR_ID[]     = L"id";

const XMLCh ATTR_X[]     = L"x";
const XMLCh ATTR_Y[]     = L"y";
const XMLCh ATTR_Z[]     = L"z";

using namespace std;
using namespace BML;
using namespace xml_utils;



BehaviorRequestPtr BML::parse_bml_locomotion( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	DOMElement* child = NULL;
	const XMLCh* attrType = NULL;
	const XMLCh* attrEnable = NULL;
	const XMLCh* attrID = NULL;
	const char * ascii_enable = NULL;
	string enable;
	int type = BML_LOCOMOTION_TARGET_TYPE_UNKNOWN;
	int id = -1;

	MeCtLocomotion* ct_locomotion = request->actor->locomotion_ct;

	// Viseme transition hack until timing can support multiple sync points

	attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && *attrType != 0 ) 
	{
		if( XMLString::compareIString( attrType, ATTR_TARGET )==0 ) 
		{
			// Locomotion target is a static target
			if(type == BML_LOCOMOTION_TARGET_TYPE_UNKNOWN || type == BML_LOCOMOTION_TARGET_TYPE_TARGET)
			{
				type = BML_LOCOMOTION_TARGET_TYPE_TARGET;
			}
			else 
			{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<"> BML target type conflicts with previous setting ";
				LOG(convertWStringToString(wstrstr.str()).c_str());
			}

		} 
		else if( XMLString::compareIString( attrType, ATTR_DIRECTION )==0 ) 
		{
			// Locomotion target is a direction.
			if(type == BML_LOCOMOTION_TARGET_TYPE_UNKNOWN || type == BML_LOCOMOTION_TARGET_TYPE_DIRECTION)
			{
				type = BML_LOCOMOTION_TARGET_TYPE_DIRECTION;
			}
			else
			{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<"> BML target type conflicts with previous setting ";
				LOG(convertWStringToString(wstrstr.str()).c_str());
			}
		} 
	}

	// Enable/disable locomotion controller
	attrEnable = elem->getAttribute( ATTR_ENABLE );
	if( attrEnable && *attrEnable != 0 ) 
	{
		if( XMLString::compareIString( attrEnable, L"true" )==0 ) 
		{
			ct_locomotion->set_enabled(true);
		}
		else if( XMLString::compareIString( attrEnable, L"false" )==0 )
		{
			ct_locomotion->set_enabled(false);
		}
	}

	attrID = elem->getAttribute( ATTR_ID );
	if( attrType && *attrID != 0 ) 
	{
		// ID of the locomotion routine
		XMLStringTokenizer tokenizer( attrID );
		wistringstream in;
		XMLCh* token;
		switch( tokenizer.countTokens() ) 
		{
		case 1:
			token = tokenizer.nextToken();
			in.clear();
			in.str( token );
			in.seekg(0);
			in >> id;
			break;
		default:
			break;
		}
		
	} 

	if(0)
	{
		std::wstringstream wstrstr;
		wstrstr << "WARNING: BML::parse_bml_locomotion(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute.";
		LOG(convertWStringToString(wstrstr.str()).c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
	
	child = getFirstChildElement( elem );

//	Locomotion::parse_routine(child, request, type, id);
	//-------------starting  from here, it's BML Spec 1.0
	std::string localId;
	if (attrID)
		localId = XMLString::transcode(attrID);
	std::stringstream command;
	SbmCharacter* c = mcu->character_map.lookup(request->actor->name);
	const XMLCh* locotype = elem->getAttribute(L"sbm:type");
	if (XMLString::compareIString(locotype, L"") != 0)
	{
		std::string typeString = XMLString::transcode(locotype);
		if (typeString == "procedural")
		{
			mcu->steering_use_procedural = true;
			c->steeringAgent->desiredSpeed = 1.6f;
		}
		else if (typeString == "example")
			mcu->steering_use_procedural = false;
		else
			return BehaviorRequestPtr();
	}
	const XMLCh* proximity = elem->getAttribute(L"proximity");
	if (XMLString::compareIString(proximity, L"") != 0)
		c->steeringAgent->distThreshold = (float)atof(XMLString::transcode(proximity)) * 100.0f;
	const XMLCh* manner = elem->getAttribute(L"manner");
	if (XMLString::compareIString(manner, L"") != 0)
	{
		if (mcu->steering_use_procedural)
		{
			LOG("This mode does not support Procedural Locomotion currently!");
			return BehaviorRequestPtr();
		}
		std::string mannerString = XMLString::transcode(manner);
		if (mannerString == "walk")
			c->steeringAgent->desiredSpeed = 1.6f;
		else if (mannerString == "jog")
			c->steeringAgent->desiredSpeed = 2.5f;
		else if (mannerString == "run")
			c->steeringAgent->desiredSpeed = 3.5f;
		else 
			return BehaviorRequestPtr();
		// also has to update state weight
		PAStateData* locoData = mcu->lookUpPAState("UtahLocomotion");
		if (locoData)
			locoData->paramManager->setWeight(c->steeringAgent->desiredSpeed * 100.0f, 0.0);
	}
	const XMLCh* facingAngle = elem->getAttribute(L"facing");
	if (XMLString::compareIString(facingAngle, L"") != 0)
	{
		std::stringstream command;
		command << "steer facing " << c->name << " " << (float)atof(XMLString::transcode(facingAngle));
		srCmdSeq *seq = new srCmdSeq(); //sequence that holds the startup feedback
		seq->insert(float(mcu->time + mcu->time_dt), command.str().c_str());
		mcu->execute_seq(seq);
	//	c->steeringAgent->facingAngle = (float)atof(XMLString::transcode(facingAngle));
	}
	const XMLCh* following = elem->getAttribute(L"sbm:follow");
	if (XMLString::compareIString(following, L"") != 0)
	{
		std::string character = XMLString::transcode(following);
		SbmCharacter* followingC = mcu->character_map.lookup(character.c_str());
		if (!followingC)
			c->steeringAgent->setTargetAgent(NULL);
		c->steeringAgent->setTargetAgent(followingC);
	}
	const XMLCh* target = elem->getAttribute(L"target");
	if (XMLString::compareIString(target, L"") != 0)
	{
		XMLStringTokenizer tokenizer(target);
		if (tokenizer.countTokens() != 2)
			return BehaviorRequestPtr();
		XMLCh* x = tokenizer.nextToken();
		XMLCh* z = tokenizer.nextToken();
		command << "sbm steer move " << c->name << " " << XMLString::transcode(x) << " 0 " << XMLString::transcode(z);
	}
	return BehaviorRequestPtr( new EventRequest(unique_id, localId, command.str().c_str(), behav_syncs, ""));
}

void BML::Locomotion::parse_routine(DOMElement* elem, BmlRequestPtr request, int type, int id)
{
	MeCtNavigationCircle* nav_circle = new MeCtNavigationCircle();
	nav_circle->ref();
	nav_circle->set( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
	request->actor->posture_sched_p->create_track( NULL, NULL, nav_circle);
	nav_circle->unref();

	float speed = 0.0f;
	float g_angular_speed = 0.0f;
	float l_angular_speed = 0.0f;
	float pos[3];
	memset(pos, 0, sizeof(float)*3);

	MeCtLocomotion* ct_locomotion = request->actor->locomotion_ct;

	const XMLCh* tag = NULL;
	DOMElement* child = elem;

	wistringstream in;
	XMLCh* token;

	while(child)
	{
		tag = child->getTagName();
		if(XMLString::compareIString( tag, ATTR_TARGET )==0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_TARGET)
			{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::parse_routine(): locomotion routine type unmatched";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return;  // a.k.a., NULL
			}
			const XMLCh* attrTarget[3];
			const XMLCh* attrSpeed;

			attrTarget[0] = child->getAttribute( ATTR_X );
			attrTarget[1] = child->getAttribute( ATTR_Y );
			attrTarget[2] = child->getAttribute( ATTR_Z );
			attrSpeed = child->getAttribute( ATTR_SPEED );

			XMLStringTokenizer tokenizer( attrSpeed );
			switch( tokenizer.countTokens() ) 
			{
			case 1:
				token = tokenizer.nextToken();
				in.clear();
				in.str( token );
				in.seekg(0);
				in >> speed;
				break;
			default:
				break;
			}
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
					in >> pos[i];
					break;
				default:
					break;
				}
			}
			ct_locomotion->get_navigator()->clear_destination_list();
			SrVec dest(pos[0], pos[1], pos[2]);
			ct_locomotion->get_navigator()->add_destination(&dest);
			ct_locomotion->get_navigator()->has_destination = true;
			ct_locomotion->get_navigator()->add_speed(speed);
		}
		else if(XMLString::compareIString( tag, ATTR_DIRECTION) == 0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_DIRECTION)
			{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::parse_routine(): locomotion routine type unmatched";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return;  // a.k.a., NULL
			}
			const XMLCh* attrDir[3];
			const XMLCh* attrSpeed;

			attrDir[0] = child->getAttribute( ATTR_X );
			attrDir[1] = child->getAttribute( ATTR_Y );
			attrDir[2] = child->getAttribute( ATTR_Z );
			attrSpeed = child->getAttribute( ATTR_SPEED );

			XMLStringTokenizer tokenizer( attrSpeed );
			switch( tokenizer.countTokens() ) 
			{
			case 1:
				token = tokenizer.nextToken();
				in.clear();
				in.str( token );
				in.seekg(0);
				in >> speed;
				break;
			default:
				break;
			}

			for(int i = 0; i < 3; ++i)
			{
				XMLStringTokenizer tokenizer( attrDir[i] );
				switch( tokenizer.countTokens() ) 
				{
				case 1:
					token = tokenizer.nextToken();
					in.clear();
					in.str( token );
					in.seekg(0);
					in >> pos[i];
					break;
				default:
					break;
				}
			}
			SrVec dir(pos[0], pos[1], pos[2]);
			dir.normalize();
			dir *= speed;
			pos[0] = dir.x;
			pos[1] = dir.y;
			pos[2] = dir.z;
			ct_locomotion->get_navigator()->has_destination = false;
		}
		else if(XMLString::compareIString( tag, ATTR_VELOCITY) == 0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_DIRECTION)
			{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::parse_routine(): locomotion routine type unmatched.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return;  // a.k.a., NULL
			}
			const XMLCh* attrVel[3];

			attrVel[0] = child->getAttribute( ATTR_X );
			attrVel[1] = child->getAttribute( ATTR_Y );
			attrVel[2] = child->getAttribute( ATTR_Z );

			for(int i = 0; i < 3; ++i)
			{
				XMLStringTokenizer tokenizer( attrVel[i] );
				switch( tokenizer.countTokens() ) 
				{
				case 1:
					token = tokenizer.nextToken();
					in.clear();
					in.str( token );
					in.seekg(0);
					in >> pos[i];
					break;
				default:
					break;
				}
			}
		}

		else if(XMLString::compareIString( tag, ATTR_ROTATION) == 0)
		{
			const XMLCh* attrType;
			const XMLCh* attrSpeed;
			float rotation_speed = 0.0f;

			attrType = child->getAttribute( ATTR_TYPE );
			attrSpeed = child->getAttribute( ATTR_SPEED );

			XMLStringTokenizer tokenizer( attrSpeed );
			XMLCh* token;
			switch( tokenizer.countTokens() ) 
			{
			case 1:
				token = tokenizer.nextToken();
				in.clear();
				in.str( token );
				in.seekg(0);
				in >> rotation_speed;
				break;
			default:
				break;
			}
			
			if(XMLString::compareIString( attrType, L"lrps") == 0)
			{
				l_angular_speed = rotation_speed;
			}

			else if(XMLString::compareIString( attrType, L"grps") == 0)
			{
				g_angular_speed = rotation_speed;
			}

			else if(XMLString::compareIString( attrType, L"rps") == 0)
			{
				l_angular_speed = rotation_speed;
				g_angular_speed = rotation_speed;
			}
			
		}

		child = getNextElement( child );
	}


	nav_circle->set( pos[0], pos[1], pos[2], g_angular_speed, l_angular_speed, 0, id, 0, 0, 0, -1 );
}

BehaviorRequestPtr BML::parse_bml_example_locomotion( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu )
{
	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	SbmCharacter* c = mcu->character_map.lookup(request->actor->name);
	if (!c->param_animation_ct)
	{
		LOG("Parameterized Animation not enabled!");
		return BehaviorRequestPtr();
	}
	const XMLCh* forwardSpd = elem->getAttribute(L"spd");
	const XMLCh* turningSpd = elem->getAttribute(L"rps");
	double spd = 0.0;
	double rps = 0.0;
	if (forwardSpd)
		spd = atof(XMLString::transcode(forwardSpd));
	if (turningSpd)
		rps = atof(XMLString::transcode(turningSpd));

	if (spd == 0 && rps == 0)
	{
		if (c->param_animation_ct->getCurrentStateName() == "UtahLocomotion")
		{
			std::stringstream command;
			command << "panim schedule char " << c->name;
			command << " state UtahWalkToStop loop false playnow false";	
			mcu->execute((char*) command.str().c_str());
		}
	}
	if (c->param_animation_ct->getCurrentPAStateData() == NULL)
	{
		std::stringstream command;
		command << "panim schedule char " << c->name;
		command << " state UtahStopToWalk loop false playnow false";
		mcu->execute((char*) command.str().c_str());
		std::stringstream command1;
		command1 << "panim schedule char " << c->name;
		command1 << " state UtahLocomotion loop true playnow false";
		mcu->execute((char*) command1.str().c_str());
	}
	else
	{
		if (c->param_animation_ct->getCurrentStateName() == "UtahLocomotion")
		{
			c->param_animation_ct->getCurrentPAStateData()->paramManager->setWeight(spd, rps);
			c->param_animation_ct->updateWeights();			
		}
	}
	return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
}
