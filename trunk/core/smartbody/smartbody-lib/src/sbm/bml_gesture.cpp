#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <string>

#include "bml_gesture.hpp"
#include "mcontrol_util.h"
#include "sb/SBScene.h"
#include "bml_xml_consts.hpp"
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>

using namespace std;
using namespace BML;
using namespace xml_utils;

BML::BehaviorRequestPtr BML::parse_bml_gesture( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) 
{
	const XMLCh* animName = elem->getAttribute( BMLDefs::ATTR_NAME );
	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	const XMLCh* typeAttr = elem->getAttribute(BMLDefs::ATTR_LEXEME);
	const XMLCh* modeAttr = elem->getAttribute(BMLDefs::ATTR_MODE);
	const XMLCh* styleAttr = elem->getAttribute(BMLDefs::ATTR_STYLE);
	std::string animationName;
	std::string localId;
	std::string type;
	std::string mode;
	std::string style;
	xml_utils::xml_translate(&localId, id);
	xml_utils::xml_translate(&animationName, animName);
	xml_utils::xml_translate(&type, typeAttr);
	xml_utils::xml_translate(&mode, modeAttr);
	xml_utils::xml_translate(&style, styleAttr);

	if (animationName == "")	// If you have assigned the animation name, do not look for the map
	{
		SmartBody::SBGestureMap* gestureMap = mcu->_scene->getGestureMapManager()->getGestureMap(request->actor->getName());
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

		animationName = gestureMap->getGestureByInfo(type, posture, mode, style);
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
		motionCt->init(const_cast<SbmCharacter*>(request->actor), motion, 0.0, 1.0);

		BehaviorRequestPtr behavPtr(new MotionRequest( unique_id, localId, motionCt, request->actor->motion_sched_p, behav_syncs ) );
		return behavPtr; 
	} 
	else 
	{
		LOG("WARNING: BML::parse_bml_gesture(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), animationName.c_str());
		return BehaviorRequestPtr();
	}
}
