#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <string>

#include "bml_gesture.hpp"
#include "sbm/mcontrol_util.h"
#include "sb/SBScene.h"
#include "bml_xml_consts.hpp"
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBBehavior.h>

using namespace std;
using namespace BML;
using namespace xml_utils;

/*	Usage of bml gestures
 *	- if stroke and stroke_end is defined, poststroke_hold time would be calculated automatically even if there's poststroke_hold given by user
 *	- you can specify idle animation used for gesture holding period
 *	- you can specify joints, scale, frequency for perlin noise added to gesture holding period. premise is no idle animation is specified.
 */

BML::BehaviorRequestPtr BML::parse_bml_gesture( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) 
{
	const XMLCh* animName = elem->getAttribute( BMLDefs::ATTR_NAME );
	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	const XMLCh* lexemeAttr = elem->getAttribute(BMLDefs::ATTR_LEXEME);
	const XMLCh* typeAttr = elem->getAttribute(BMLDefs::ATTR_TYPE);
	const XMLCh* modeAttr = elem->getAttribute(BMLDefs::ATTR_MODE);
	const XMLCh* styleAttr = elem->getAttribute(BMLDefs::ATTR_STYLE);
	std::string animationName;
	std::string localId;
	std::string lexeme;
	std::string type;
	std::string mode;
	std::string style;
	xml_utils::xml_translate(&localId, id);
	xml_utils::xml_translate(&animationName, animName);
	xml_utils::xml_translate(&lexeme, lexemeAttr);
	xml_utils::xml_translate(&type, typeAttr);
	xml_utils::xml_translate(&mode, modeAttr);
	xml_utils::xml_translate(&style, styleAttr);

	SBCharacter* character = dynamic_cast<SBCharacter*>(request->actor);
	std::string gestureMapName = character->getStringAttribute("gestureMap");
	if (animationName == "")	// If you have assigned the animation name, do not look for the map
	{
		SmartBody::SBGestureMap* gestureMap = mcu->_scene->getGestureMapManager()->getGestureMap(gestureMapName);
		if (!gestureMap)
		{
			LOG("WARNING: BML::parse_bml_gesture(): gesture map for character %s doesn't exist.", request->actor->getName().c_str());
			return BehaviorRequestPtr();		
		}

		// Get current posture
		std::string posture = "";
		if (request->actor->posture_sched_p)
		{
			MeCtScheduler2::VecOfTrack tracks = request->actor->posture_sched_p->tracks();
			for (size_t t = 0; t < tracks.size(); t++)
			{
				MeCtMotion* motionCt = dynamic_cast<MeCtMotion*>(tracks[t]->animation_ct());
				if (motionCt)
				{
					posture = motionCt->motion()->getName();
					break;
				}
			}
		}
		animationName = gestureMap->getGestureByInfo(lexeme, type, mode, style, posture);
	}

	if (animationName == "")
	{
		LOG("WARNING: BML::parse_bml_gesture(): invalid animation name");
		return BehaviorRequestPtr();
	}

	// play the animation
	if (!request->actor->motion_sched_p)
	{
		LOG("Character %s does not have a motion scheduler, so cannot schedule motion.", request->actor->getName().c_str());
		return BehaviorRequestPtr();
	}
	std::map<std::string, SkMotion*>::iterator motionIter = mcu->motion_map.find(animationName);
	if (motionIter != mcu->motion_map.end())
	{
		SkMotion* motion = (*motionIter).second;
		MeCtMotion* motionCt = new MeCtMotion();

		// Name controller with behavior unique_id
		ostringstream name;
		name << unique_id << ' ' << motion->getName();
		motionCt->setName(name.str().c_str());  // TODO: include BML act and behavior ids
		// Handle stroke hold
		SkMotion* mForCt = motion;
		float prestrokehold = (float)xml_utils::xml_parse_double(BMLDefs::ATTR_PRESTROKE_HOLD, elem, -1.0);
		std::string prestrokehold_idlemotion = xml_utils::xml_parse_string(BMLDefs::ATTR_PRESTROKE_HOLD_IDLEMOTION, elem);
		SkMotion* preIdleMotion = (SkMotion*)mcu->_scene->getMotion(prestrokehold_idlemotion);
		if (prestrokehold > 0)
			mForCt = motion->buildPrestrokeHoldMotion(prestrokehold, preIdleMotion);
		float poststrokehold = (float)xml_utils::xml_parse_double(BMLDefs::ATTR_POSTSTROKE_HOLD, elem, -1.0);
		std::string poststrokehold_idlemotion = xml_utils::xml_parse_string(BMLDefs::ATTR_POSTSTROKE_HOLD_IDLEMOTION, elem);
		SkMotion* postIdleMotion = (SkMotion*)mcu->_scene->getMotion(poststrokehold_idlemotion);
		
		std::string joints = xml_utils::xml_parse_string(BMLDefs::ATTR_JOINT_RANGE, elem);
		float scale = (float)xml_utils::xml_parse_double(BMLDefs::ATTR_SCALE, elem, 0.02f);
		float freq = (float)xml_utils::xml_parse_double(BMLDefs::ATTR_FREQUENCY, elem, 0.03f);
		std::string strokeString = xml_utils::xml_parse_string(BMLDefs::ATTR_STROKE, elem);
		std::string strokeEndString = xml_utils::xml_parse_string(BMLDefs::ATTR_STROKE_END, elem);
		if (poststrokehold > 0 && strokeString == "" && strokeEndString == "")
		{
			std::vector<std::string> jointVec;
			vhcl::Tokenize(joints, jointVec);
			mForCt = mForCt->buildPoststrokeHoldMotion(poststrokehold, jointVec, scale, freq, postIdleMotion);
		}
		//motionCt->init(const_cast<SbmCharacter*>(request->actor), motion, 0.0, 1.0);
		SBCharacter* sbCharacter = dynamic_cast<SBCharacter*>(request->actor);
		bool isInLocomotion = false;
		if (sbCharacter->steeringAgent)
		{
			for (size_t i = 0; i < request->behaviors.size(); ++i)
			{
				if ((request->behaviors)[i]->local_id == "locomotion")
				{
					isInLocomotion = true;
					break;
				}
			}
			if (sbCharacter->steeringAgent->isInLocomotion())
				isInLocomotion = true;
		}
		if (isInLocomotion)
			motionCt->init( const_cast<SbmCharacter*>(request->actor), mForCt, sbCharacter->getSkeleton()->getUpperBodyJointNames());
		else
			motionCt->init( const_cast<SbmCharacter*>(request->actor), mForCt, 0.0, 1.0);
		BehaviorRequestPtr behavPtr(new GestureRequest( unique_id, localId, motionCt, request->actor->motion_sched_p, behav_syncs, joints, scale, freq) );
		return behavPtr; 
	} 
	else 
	{
		LOG("WARNING: BML::parse_bml_gesture(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), animationName.c_str());
		return BehaviorRequestPtr();
	}
}
