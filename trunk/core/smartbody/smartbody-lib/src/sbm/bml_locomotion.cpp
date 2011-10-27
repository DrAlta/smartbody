/*
 *  bml_locomotion.cpp - part of SmartBody-lib
 *  Copyright (C) 2011  University of Southern California
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
 *		Yuyu Xu, ICT USC
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
#include "BMLDefs.h"

////// XML ATTRIBUTES

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

	MeCtLocomotion* ct_locomotion = request->actor->locomotion_ct;

	// Viseme transition hack until timing can support multiple sync points

	attrType = elem->getAttribute( BMLDefs::ATTR_TYPE );
	if( attrType && *attrType != 0 ) 
	{
		if( XMLString::compareIString( attrType, BMLDefs::ATTR_TARGET )==0 ) 
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
		else if( XMLString::compareIString( attrType, BMLDefs::ATTR_DIRECTION )==0 ) 
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
	attrEnable = elem->getAttribute( BMLDefs::ATTR_ENABLE );
	if( attrEnable && *attrEnable != 0 ) 
	{
		if( XMLString::compareIString( attrEnable, BMLDefs::ATTR_TRUE )==0 ) 
		{
			ct_locomotion->set_enabled(true);
		}
		else if( XMLString::compareIString( attrEnable, BMLDefs::ATTR_FALSE )==0 )
		{
			ct_locomotion->set_enabled(false);
		}
	}

	int id = xml_parse_int( BMLDefs::ATTR_ID, elem, -1 );

	child = getFirstChildElement( elem );

//	Locomotion::parse_routine(child, request, type, id);
	//-------------starting  from here, it's BML Spec 1.0
	std::string localId;
	xml_utils::xml_translate(&localId, attrID);

	if (!mcu->steerEngine.isInitialized())
	{
		LOG("Steering Engine not started. Call \"steer start\" first");
		return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
	}
	if (!mcu->steerEngine._engine)
	{
		LOG("Steering Engine not started. Call \"steer start\" first");
		return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
	}
	std::stringstream command;
	SbmCharacter* c = mcu->getCharacter(request->actor->getName());

	c->steeringAgent->steppingMode = false;
	bool stepMode = false;
	bool stepTargetMode = false;
	std::string stepDirection = "";
	float stepTargetX = 0.0f;
	float stepTargetZ = 0.0f;


	std::string locotype = xml_parse_string(BMLDefs::ATTR_TYPE, elem);
	std::string steerTypeCommand = "steer type " + locotype;
	mcu->execute((char*) steerTypeCommand.c_str());
	float proximity = xml_parse_float(BMLDefs::ATTR_PROXIMITY, elem, c->steeringAgent->distThreshold);
	c->steeringAgent->distThreshold = proximity;
	c->steeringAgent->acceleration = xml_parse_float(BMLDefs::ATTR_STEERACCEL, elem, c->steeringAgent->acceleration);
	c->steeringAgent->scootAcceleration = xml_parse_float(BMLDefs::ATTR_STEERSCOOTACCEL, elem, c->steeringAgent->scootAcceleration);
	c->steeringAgent->angleAcceleration = xml_parse_float(BMLDefs::ATTR_STEERANGLEACCEL, elem, c->steeringAgent->angleAcceleration);
	std::string manner = xml_parse_string(BMLDefs::ATTR_MANNER, elem);
	if (manner != "")
	{
		if (c->locomotion_type == c->Procedural)
		{
			LOG("This mode does not support Procedural Locomotion currently!");
			return BehaviorRequestPtr();
		}
		if (manner == "walk")
			c->steeringAgent->desiredSpeed = 1.2f;
		else if (manner == "jog")
			c->steeringAgent->desiredSpeed = 2.5f;
		else if (manner == "run")
			c->steeringAgent->desiredSpeed = 3.5f;
		else if (manner == "sbm:step")
			stepMode = true;
		else if (manner == "sbm:jump")
		{
			if (c->param_animation_ct)
			{
				if (c->param_animation_ct->hasPAState("UtahJump"))
					return BehaviorRequestPtr( new EventRequest(unique_id, localId, command.str().c_str(), behav_syncs, ""));
				std::stringstream command1;
				if (c->param_animation_ct->getCurrentStateName() == "UtahLocomotion")
				{
					command1 << "bml char " << c->getName() << " <sbm:states loop=\"false\" name=\"UtahJump\" sbm:startnow=\"true\"/>";
					command1 << "<sbm:states loop=\"true\" name=\"UtahLocomotion\" sbm:startnow=\"false\"/>";
				}
				else
				{
					command1 << "bml char " << c->getName() << " <sbm:states loop=\"false\" name=\"UtahJump\" sbm:startnow=\"true\"/>";
				}
				mcu->execute((char*)command1.str().c_str());
				return BehaviorRequestPtr( new EventRequest(unique_id, localId, command.str().c_str(), behav_syncs, ""));
			}
		}
		else 
			return BehaviorRequestPtr();
		// also has to update state weight
		PAStateData* locoData = mcu->lookUpPAState("UtahLocomotion");
		if (locoData)
			locoData->paramManager->setWeight(c->steeringAgent->desiredSpeed * 100.0f, 0.0);
	}

	// for facing angle, we need to execute with some delay
	const char* facingAngle = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_FACING));
	if (strcmp(facingAngle, "") != 0)
	{
		std::stringstream command;
		command << "steer facing " << c->getName() << " " << (float)atof(facingAngle);
		srCmdSeq *seq = new srCmdSeq();
		seq->insert(float(mcu->time + mcu->time_dt), command.str().c_str());
		mcu->execute_seq(seq);
	}
	else
	{
		std::stringstream command;
		command << "steer facing " << c->getName() << " " << "-200"; // set to fabs() > 180 to cancel old facing value
		srCmdSeq *seq = new srCmdSeq();
		seq->insert(float(mcu->time + mcu->time_dt), command.str().c_str());
		mcu->execute_seq(seq);
	}
	std::string following = xml_parse_string(BMLDefs::ATTR_FOLLOW, elem);
	SbmCharacter* followingC = mcu->getCharacter(following);
	c->steeringAgent->setTargetAgent(followingC);
	float* targetFloat = new float[2];
	int targetParsing = xml_parse_float(targetFloat, 2, BMLDefs::ATTR_TARGET, elem);
	if (targetParsing == 2)
	{
		if (stepMode)
		{
			stepTargetX = targetFloat[0];
			stepTargetZ = targetFloat[1];
			stepTargetMode = true;
		}
		else
			command << "sbm steer move " << c->getName() << " " << targetFloat[0] << " 0 " << targetFloat[1];
	}
	else
	{
		if (targetParsing != 0)
			LOG("Warning: target's format is X Z.");

		std::string targetString = xml_parse_string(BMLDefs::ATTR_TARGET, elem);
		if ((targetString == "forward" || targetString == "backward" || targetString == "left" || targetString == "right") && stepMode)
		{
			stepTargetMode = false;
			stepDirection = targetString;
		}
		else
		{
			SbmPawn* pawn = mcu->getPawn(targetString);
			if (pawn)
			{
				// get the world offset x & z
				float x, y, z, yaw, pitch, roll;
				pawn->get_world_offset(x, y, z, yaw, pitch, roll);
				if (stepMode)
				{
					stepTargetMode = true;
					stepTargetX = x;
					stepTargetZ = z;
				}
				else
					command << "sbm steer move " << c->getName() << " " << x << " 0 " << z;
			}
		}
	}
	delete [] targetFloat;

	// step mode attributes
	int numSteps = xml_parse_int(BMLDefs::ATTR_NUM_STEPS, elem);
	if (numSteps <= 0) numSteps = 1;

	// execute steps
	if (stepMode)
	{
		if (!c->param_animation_ct)
		{
			LOG("Parameterized Animation Engine not setup, cannot use step control.");
			return BehaviorRequestPtr();
		}
		if (c->param_animation_ct->hasPAState("UtahStep") || !c->param_animation_ct->isIdle())
			return BehaviorRequestPtr();
		c->steeringAgent->stepAdjust = false;
		if (stepTargetMode)		// given target
		{
			c->steeringAgent->steppingMode = true;
			c->steeringAgent->stepTargetX = stepTargetX;
			c->steeringAgent->stepTargetZ = stepTargetZ;
		}
		else					// given direction and number of steps
		{
			for (int i = 0; i < numSteps; i++)
			{
				std::stringstream command1;
				command1 << "panim schedule char " << c->getName();
				command1 << " state UtahStep loop false playnow false ";
				if (stepDirection == "forward")
					command1 << "0 0 1 0 0 0 0";
				if (stepDirection == "backward")
					command1 << "0 1 0 0 0 0 0";
				if (stepDirection == "left")
					command1 << "0 0 0 0 0 0 1";
				if (stepDirection == "right")
					command1 << "0 0 0 1 0 0 0";
				mcu->execute((char*)command1.str().c_str());
			}
		}
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

	while(child)
	{
		tag = child->getTagName();
		if(XMLString::compareIString( tag, BMLDefs::ATTR_TARGET )==0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_TARGET)
			{
				LOG("WARNING: BML::parse_routine(): locomotion routine type unmatched");
				return;  // a.k.a., NULL
			}

			pos[ 0 ] = xml_parse_float( BMLDefs::ATTR_X, child );
			pos[ 1 ] = xml_parse_float( BMLDefs::ATTR_Y, child );
			pos[ 2 ] = xml_parse_float( BMLDefs::ATTR_Z, child );
			speed = xml_parse_float( BMLDefs::ATTR_SPEED, child );

			ct_locomotion->get_navigator()->clear_destination_list();
			SrVec dest(pos[0], pos[1], pos[2]);
			ct_locomotion->get_navigator()->add_destination(&dest);
			ct_locomotion->get_navigator()->has_destination = true;
			ct_locomotion->get_navigator()->add_speed(speed);
		}
		else if(XMLString::compareIString( tag, BMLDefs::ATTR_DIRECTION) == 0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_DIRECTION)
			{
				LOG("WARNING: BML::parse_routine(): locomotion routine type unmatched");
				return;  // a.k.a., NULL
			}

			pos[ 0 ] = xml_parse_float( BMLDefs::ATTR_X, child );
			pos[ 1 ] = xml_parse_float( BMLDefs::ATTR_Y, child );
			pos[ 2 ] = xml_parse_float( BMLDefs::ATTR_Z, child );
			speed = xml_parse_float( BMLDefs::ATTR_SPEED, child );

			SrVec dir(pos[0], pos[1], pos[2]);
			dir.normalize();
			dir *= speed;
			pos[0] = dir.x;
			pos[1] = dir.y;
			pos[2] = dir.z;
			ct_locomotion->get_navigator()->has_destination = false;
		}
		else if(XMLString::compareIString( tag, BMLDefs::ATTR_VELOCITY) == 0)
		{
			if(type != BML_LOCOMOTION_TARGET_TYPE_DIRECTION)
			{
				LOG("WARNING: BML::parse_routine(): locomotion routine type unmatched.");
				return;  // a.k.a., NULL
			}

			pos[ 0 ] = xml_parse_float( BMLDefs::ATTR_X, child );
			pos[ 1 ] = xml_parse_float( BMLDefs::ATTR_Y, child );
			pos[ 2 ] = xml_parse_float( BMLDefs::ATTR_Z, child );
		}

		else if(XMLString::compareIString( tag, BMLDefs::ATTR_ROTATION) == 0)
		{
			const XMLCh* attrType = child->getAttribute( BMLDefs::ATTR_TYPE );
			
			float rotation_speed = xml_parse_float( BMLDefs::ATTR_SPEED, child );
			
			if(XMLString::compareIString( attrType, BMLDefs::ATTR_LRPS) == 0)
			{
				l_angular_speed = rotation_speed;
			}

			else if(XMLString::compareIString( attrType, BMLDefs::ATTR_GRPS) == 0)
			{
				g_angular_speed = rotation_speed;
			}

			else if(XMLString::compareIString( attrType, BMLDefs::ATTR_RPS) == 0)
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
	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	std::string localId;
	xml_utils::xml_translate(&localId, id);
	SbmCharacter* c = mcu->getCharacter(request->actor->getName());
	if (!c->param_animation_ct)
	{
		LOG("Parameterized Animation not enabled!");
		return BehaviorRequestPtr();
	}
	const XMLCh* forwardSpd = elem->getAttribute(BMLDefs::ATTR_SPD);
	const XMLCh* turningSpd = elem->getAttribute(BMLDefs::ATTR_RPS);
	double spd = 0.0;
	double rps = 0.0;
	if (forwardSpd)
		spd = xml_utils::xml_translate_float(forwardSpd);
	if (turningSpd)
		rps = xml_utils::xml_translate_float(turningSpd);

	if (spd == 0 && rps == 0)
	{
		if (c->param_animation_ct->getCurrentStateName() == "UtahLocomotion")
		{
			std::stringstream command;
			command << "panim schedule char " << c->getName();
			command << " state UtahWalkToStop loop false playnow false";	
			mcu->execute((char*) command.str().c_str());
		}
	}
	if (c->param_animation_ct->getCurrentPAStateData() == NULL)
	{
		std::stringstream command;
		command << "panim schedule char " << c->getName();
		command << " state UtahStopToWalk loop false playnow false";
		mcu->execute((char*) command.str().c_str());
		std::stringstream command1;
		command1 << "panim schedule char " << c->getName();
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