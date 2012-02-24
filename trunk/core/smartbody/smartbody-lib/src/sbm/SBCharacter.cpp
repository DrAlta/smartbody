#include "SBCharacter.h"
#include <sbm/SBSkeleton.h>
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include "sbm/SBController.h"
#include "sbm/me_utilities.hpp"
#include "sbm/SBBehavior.h"

namespace SmartBody {

SBCharacter::SBCharacter() : SbmCharacter()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
}

SBCharacter::SBCharacter(std::string name, std::string type) : SbmCharacter(name.c_str(), type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
}

int SBCharacter::setup()
{
	return SbmCharacter::setup();
}

const std::string& SBCharacter::getName()
{
	return SbmCharacter::getName();
}

void SBCharacter::setName(std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::string oldName = getName();
	mcu.unregisterCharacter(this);

	SbmCharacter::setName(name);
	mcu.registerCharacter(this);
}

void SBCharacter::setMeshMap(std::string filename)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu_character_load_skinweights( getName().c_str(), filename.c_str(), &mcu, 1.0f );
}

void SBCharacter::addMesh(std::string mesh)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu_character_load_mesh( getName().c_str(), mesh.c_str(), &mcu );
}

int SBCharacter::getNumControllers()
{
	MeControllerTreeRoot* controllerTree = ct_tree_p;
	if (controllerTree)
		return controllerTree->count_controllers();
	else
		return 0;
}



void SBCharacter::setAutomaticPruning(bool val)
{
	_isControllerPruning = val;
}

bool SBCharacter::isAutomaticPruning()
{
	return _isControllerPruning;
}

void SBCharacter::pruneControllers()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	prune_controller_tree( &mcu );
}

void SBCharacter::setUseVisemeCurves(bool val)
{
	set_viseme_curve_mode(val);
}

bool SBCharacter::isUseVisemeCurves()
{
	return get_viseme_curve_mode();
}

float SBCharacter::getVisemeTimeOffset()
{
	return get_viseme_time_delay();
}

void SBCharacter::setVisemeTimeOffset(float val)
{
	set_viseme_time_delay(val);
}

SBController* SBCharacter::getControllerByIndex(int index)
{
	if (!ct_tree_p)
		return NULL;

	if (index < 0 || index >= (int)ct_tree_p->count_controllers())
	{
		LOG("Index %d out of range.", index);
		return NULL;
	}

	SBController* controller = getControllerByName(ct_tree_p->controller(index)->getName());
	return controller;
}

SBController* SBCharacter::getControllerByName(std::string name)
{
	if (!ct_tree_p)
		return NULL;
	
	for (int i = 0; i < (int)ct_tree_p->count_controllers(); i++)
	{
		SBController* controller = dynamic_cast<SBController*>(ct_tree_p->controller(i));
		const std::string& cName = controller->getName();
		if (name == cName)
		{
			return controller;
		}
	}
	return NULL;
}

std::vector<std::string> SBCharacter::getControllerNames()
{
	std::vector<std::string> ret;
	for (int i = 0; i < (int)ct_tree_p->count_controllers(); i++)
	{
		const std::string& cName = ct_tree_p->controller(i)->getName();
		ret.push_back(cName);
	}	
	return ret;
}

void SBCharacter::setVoice(std::string type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (type == "")
	{
		set_speech_impl(NULL);
	}
	else if (type == "remote")
	{
		set_speech_impl(mcu.speech_rvoice());
	}
	else if (type == "audiofile")
	{
		set_speech_impl(mcu.speech_audiofile());
	}
	else if (type == "text")
	{
		set_speech_impl(mcu.speech_text());
	}
}

const std::string& SBCharacter::getVoice()
{
	return get_voice_code();
}

void SBCharacter::setVoiceCode(std::string param)
{
	set_voice_code(param);
}

const std::string& SBCharacter::getVoiceCode()
{
	return get_voice_code();
}

void SBCharacter::setVoiceBackup(std::string type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (type == "")
	{
		set_speech_impl_backup(NULL);
	}
	else if (type == "remote")
	{
		set_speech_impl_backup(mcu.speech_rvoice());
	}
	else if (type == "audiofile")
	{
		set_speech_impl_backup(mcu.speech_audiofile());
	}
	else if (type == "text")
	{
		set_speech_impl_backup(mcu.speech_text());
	}
}

const std::string& SBCharacter::getVoiceBackup()
{
	return get_voice_code_backup();
}

void SBCharacter::setVoiceBackupCode(std::string param)
{
	set_voice_code_backup(param);
}

const std::string& SBCharacter::getVoiceBackupCode()
{
	return get_voice_code_backup();
}

int SBCharacter::getNumBehaviors()
{
	std::vector<SBBehavior*>& behaviors = getBehaviors();
	return behaviors.size();
}

SBBehavior* SBCharacter::getBehavior(int num)
{
	if (num < (int) _curBehaviors.size())
		return _curBehaviors[num];
	else
		return NULL;
}

std::vector<SBBehavior*>& SBCharacter::getBehaviors()
{
	for (size_t b = 0; b < _curBehaviors.size(); b++)
	{
		delete _curBehaviors[b];
	}
	_curBehaviors.clear();

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// posture
	if (this->posture_sched_p)
	{
		MeCtScheduler2::VecOfTrack tracks = gaze_sched_p->tracks();
		for (size_t t = 0; t < tracks.size(); t++)
		{
			MeCtMotion* motionCt = dynamic_cast<MeCtMotion*>(tracks[t]->animation_ct());
			if (motionCt)
			{
				const std::string& motionName = motionCt->motion()->getName();
				PostureBehavior* postureBehavior = new PostureBehavior();
				postureBehavior->setPosture(motionName);
				_curBehaviors.push_back(postureBehavior);
			}
		}
	}

	// locomotion
	if (this->steeringAgent)
	{
		const SteerLib::AgentGoalInfo& goal = this->steeringAgent->getAgent()->currentGoal();
		Util::Point goalTarget = goal.targetLocation;
		LocomotionBehavior* locoBehavior = new LocomotionBehavior();
		SrVec target(goalTarget.x, 0.f, goalTarget.z);
		locoBehavior->setLocomotionTarget(target);
		_curBehaviors.push_back(locoBehavior);
	}

	// gaze
	if (this->gaze_sched_p)
	{
		MeCtScheduler2::VecOfTrack tracks = gaze_sched_p->tracks();
		for (size_t t = 0; t < tracks.size(); t++)
		{
			MeCtGaze* gazeCt = dynamic_cast<MeCtGaze*>(tracks[t]->animation_ct());
			if (gazeCt)
			{
				float x, y, z;
				SkJoint* joint = gazeCt->get_target_joint(x, y, z);
				SkSkeleton* skeleton = joint->skeleton();
				// who's skeleton is this? Very inefficient!
				std::map<std::string, SbmPawn*>& pawns = mcu.getPawnMap();
				for (std::map<std::string, SbmPawn*>::iterator iter = pawns.begin();
					iter != pawns.end();
					iter++)
				{
					SbmPawn* pawn = (*iter).second;
					if (pawn->getSkeleton() == skeleton)
					{
						GazeBehavior* gazeBehavior = new GazeBehavior();
						gazeBehavior->setGazeTarget(pawn->getName());
						gazeBehavior->setFadingIn(gazeCt->isFadingIn());
						gazeBehavior->setFadingOut(gazeCt->isFadingOut());
						gazeBehavior->setFadedOut(gazeCt->isFadedOut());
						gazeBehavior->setHandle(gazeCt->handle());

						_curBehaviors.push_back(gazeBehavior);
						break;
					}
				}
			}
		}
	}

	return _curBehaviors;
}


SBFaceDefinition* SBCharacter::getFaceDefinition()
{
	return SbmCharacter::getFaceDefinition();
}

void SBCharacter::setFaceDefinition(SBFaceDefinition* face)
{
		SbmCharacter::setFaceDefinition(face);
}

void SBCharacter::setSteerAgent(SBSteerAgent* sbAgent)
{
	sbAgent->setCurrentSBCharacter(this);
}

};
