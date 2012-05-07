/*
 *  me_ct_motion_player.cpp - part of Motion Engine and SmartBody-lib
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
 *      Yuyu Xu, USC
 */

#include <sbm/me_ct_motion_player.h>
#include <sb/SBSkeleton.h>

std::string MeCtMotionPlayer::Context::CONTEXT_TYPE = "MeCtMotionPlayer::Context";
std::string MeCtMotionPlayer::CONTROLLER_TYPE = "MeCtMotionPlayer";

void MeCtMotionPlayer::Context::child_channels_updated( MeController* child )
{
}

MeCtMotionPlayer::MeCtMotionPlayer(SbmCharacter* c) : MeCtContainer(new MeCtMotionPlayer::Context(this)), character(c)
{
	motionName = "";
	controller = NULL;
	frameNum = 0;
	isActive = false;
}

MeCtMotionPlayer::~MeCtMotionPlayer()
{
	if (controller)
		controller->unref();
	controller = NULL;
}

void MeCtMotionPlayer::init(SbmPawn* pawn, std::string name, double n)
{
	if (motionName == name)
	{
		frameNum = n;
		return;
	}
	if (controller != NULL)
	{
		_sub_context->remove_controller(controller);
		controller->unref();
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(name);
	if (iter == mcu.motion_map.end())
		return;

	motionName = name;
	SkMotion* motion = (*iter).second;
	motion->connect(character->getSkeleton());
	controller = new MeCtMotion();
	MeCtMotion* mController = dynamic_cast<MeCtMotion*> (controller);
	mController->init(pawn,motion);
	std::string controllerName;
	controllerName = "motion player for " + motionName;
	controller->setName(controllerName.c_str());


	_sub_context->add_controller(controller);
	controller->ref();
	controller_map_updated();
	this->remap();
	frameNum = n;
}

void MeCtMotionPlayer::setFrameNum(double n)
{
	frameNum = n;
}

double MeCtMotionPlayer::getFrameNum()
{
	return frameNum;
}

void MeCtMotionPlayer::setMotionName(const std::string& name)
{
	motionName = name;
}

const std::string& MeCtMotionPlayer::getMotionName()
{
	return motionName;
}

void MeCtMotionPlayer::setActive(bool a)
{
	isActive = a;
}

bool MeCtMotionPlayer::getActive()
{
	return isActive;
}

void MeCtMotionPlayer::controller_map_updated()
{
	channels.init();
	if (controller == NULL)
		return;
	channels.merge(controller->controller_channels());
	controller->remap();
}

SkChannelArray& MeCtMotionPlayer::controller_channels()
{
	return channels;
}

double MeCtMotionPlayer::controller_duration()
{
	return -1;
}

bool MeCtMotionPlayer::controller_evaluate(double t, MeFrameData& frame)
{
	if (controller == NULL)
		return false;

	if (!isActive)
		return false;

	MeCtMotion* mController = dynamic_cast<MeCtMotion*> (controller);
	SkMotion* motion = mController->motion();

	double deltaT = motion->duration() / double(motion->frames() - 1);
	double time = deltaT * frameNum;
	int lastFrame = int(frameNum);
	motion->apply((float)time, &frame.buffer()[0], &mController->get_context_map(), SkMotion::Linear, &lastFrame);
	return true;
}