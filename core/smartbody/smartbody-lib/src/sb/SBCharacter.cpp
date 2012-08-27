#include "SBCharacter.h"
#include <sb/SBSkeleton.h>
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include "sb/SBController.h"
#include "sbm/me_utilities.hpp"
#include "bml/bml_types.hpp"
#include "bml/bml_speech.hpp"
#include "sb/SBBehavior.h"
#include <sb/SBSteerAgent.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>

namespace SmartBody {

SBCharacter::SBCharacter() : SbmCharacter()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
}

SBCharacter::SBCharacter(std::string name, std::string type) : SbmCharacter(name.c_str(), type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	createBoolAttribute("visemecurve", false, true, "Basic", 100, false, false, false, "Use curve-based visemes instead of discrete visemes.");
	createBoolAttribute("reach.useLocomotion", false, true, "Basic", 110, false, false, false, "Whether to use locomotion for reach by default.");
	createBoolAttribute("useDiphone", false, true, "Basic", 150, false, false, false, "Use diphones.");
	createStringAttribute("diphoneSetName", "", true, "Basic", 160, false, false, false, "Name of the diphone set to be used when using diphone-based lip-syncing.");
	createBoolAttribute("diphoneSplineCurve", false, true, "Basic", 170, false, false, false, "Use diphones spline/linear curve.");
	SmartBody::DoubleAttribute* diphoneSmoothWindow = createDoubleAttribute("diphoneSmoothWindow", -1.0, true, "Basic", 180, false, false, false, "Smooth window size. If it's less than 0, don't do smooth.");

	SmartBody::DoubleAttribute* timeDelayAttr = createDoubleAttribute("visemetimedelay", 0.0, true, "Basic", 210, false, false, false, "Delay visemes by a fixed amount.");
	timeDelayAttr->setMin(0.0);
	createStringAttribute("deformableMesh", "", true, "Basic", 220, false, false, false, "Directory that contains mesh information.");
	createDoubleAttribute("deformableMeshScale", 1, true, "Basic", 230, false, false, false, "Scale factor when loading mesh.");
	createStringAttribute("receiverName", "kinect1", true, "Basic", 300, false, false, false, "Name to respond to when receiving joint positions and orientations remotely.");

	std::vector<std::string> voiceTypes;
	voiceTypes.push_back("");
	voiceTypes.push_back("remote");
	voiceTypes.push_back("audiofile");	
	voiceTypes.push_back("local");	
	voiceTypes.push_back("text");

	StringAttribute* voiceAttribute = createStringAttribute("voice", "remote", true, "Basic", 400, false, false, false, "How the voice is created - local (uses local festival voice), remote (uses a speech relay), or audiofile (voice generated from prerecorded audio).");
	voiceAttribute->setValidValues(voiceTypes);

	createStringAttribute("voiceCode", "voice_kal_diphone", true, "Basic", 410, false, false, false, "For local and remote voices, the name of the voice to be used. For audiofile, the path to the audiofile when combined with the media path.");
	StringAttribute* voiceBackupAttribute = createStringAttribute("voiceBackup", "audiofile", true, "Basic", 420, false, false, false, "How the voice is created if the primary voice fails. local (uses local festival voice), remote (uses a speech relay), or audiofile (voice generated from prerecorded audio).");
	voiceBackupAttribute->setValidValues(voiceTypes);
	createStringAttribute("voiceBackupCode", ".", true, "Basic", 430, false, false, false, "For local and remote voices, the name of the backup voice to be used. For audiofile, the path to the audiofile when combined with the media path.");
	
	std::vector<std::string> utterancePolicyTypes;
	utterancePolicyTypes.push_back("none");
	utterancePolicyTypes.push_back("ignore");
	utterancePolicyTypes.push_back("queue");
	utterancePolicyTypes.push_back("interrupt");	
	StringAttribute* utterancePolicyAttribute = createStringAttribute("utterancePolicy", "none", true, "Basic", 500, false, false, false, "How utterances are handled when the character is already performing an utterance. Valid values are: ignore (ignores the new utterance and any associated behaviors), queue (plays the new utterance and associated behavior when the old utterance has finished), interrupt (stops the existing utterance and associated behaviors and plays the new one)");
	utterancePolicyAttribute->setValidValues(utterancePolicyTypes);


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
	else if (type == "local")
	{
		set_speech_impl(mcu.speech_localvoice());
	}
	else
	{
		LOG("Unknown voice setting '%s'.", type.c_str());
	}

	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voice"));
	if (attr)
		attr->setValueFast(type);

}

const std::string& SBCharacter::getVoice()
{
	return get_voice_code();
}

void SBCharacter::setVoiceCode(std::string param)
{
	set_voice_code(param);
	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voiceCode"));
	if (attr)
		attr->setValueFast(param);
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
	else if (type == "local")
	{
		set_speech_impl_backup(mcu.speech_localvoice());
	}
	else
	{
		LOG("Unknown voice backup setting '%s'.", type.c_str());
	}

	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voiceBackup"));
	if (attr)
		attr->setValueFast(type);
}

const std::string& SBCharacter::getVoiceBackup()
{
	return get_voice_code_backup();
}

void SBCharacter::setVoiceBackupCode(std::string param)
{
	set_voice_code_backup(param);
	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voiceCodeBackup"));
	if (attr)
		attr->setValueFast(param);
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

double SBCharacter::getLastScheduledSpeechBehavior()
{
	double lastTime =-1.0;

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	BML::MapOfBmlRequest bmlRequestMap = mcu.bml_processor.getBMLRequestMap();
	for (BML::MapOfBmlRequest::iterator iter = bmlRequestMap.begin(); 
		 iter != bmlRequestMap.end();
		 iter ++
		)
	{
		std::string requestName = iter->first;
		BML::BmlRequestPtr bmlRequestPtr = iter->second;
		if (bmlRequestPtr->actor->getName() == this->getName())
		{
			if (bmlRequestPtr->speech_request)
			{
				if (lastTime < bmlRequestPtr->speech_request.get()->behav_syncs.sync_end()->time())
					lastTime = bmlRequestPtr->speech_request.get()->behav_syncs.sync_end()->time();

			}
		}
	}
	return lastTime;
}

std::string SBCharacter::hasSpeechBehavior()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	BML::MapOfBmlRequest bmlRequestMap = mcu.bml_processor.getBMLRequestMap();
	for (BML::MapOfBmlRequest::iterator iter = bmlRequestMap.begin(); 
		 iter != bmlRequestMap.end();
		 iter ++
		)
	{
		std::string requestName = iter->first;
		BML::BmlRequestPtr bmlRequestPtr = iter->second;
		if (bmlRequestPtr->actor->getName() == this->getName())
		{
			if (bmlRequestPtr->speech_request)
			{
				return (*bmlRequestPtr).msgId;
			}
		}
	}

	return "";
}

std::vector<SBBehavior*>& SBCharacter::getBehaviors()
{
	for (size_t b = 0; b < _curBehaviors.size(); b++)
	{
		delete _curBehaviors[b];
	}
	_curBehaviors.clear();

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// speech
	BML::MapOfBmlRequest bmlRequestMap = mcu.bml_processor.getBMLRequestMap();
	for (BML::MapOfBmlRequest::iterator iter = bmlRequestMap.begin(); 
		 iter != bmlRequestMap.end();
		 iter ++
		)
	{
		std::string requestName = iter->first;
		BML::BmlRequestPtr bmlRequestPtr = iter->second;
		if (bmlRequestPtr->actor->getName() == this->getName())
		{
			if (bmlRequestPtr->speech_request)
			{
				SpeechBehavior* speechBehavior = new SpeechBehavior();
				// what information do we need here?
				speechBehavior->setId((*bmlRequestPtr).msgId);

				_curBehaviors.push_back(speechBehavior);
			}
		}
	}

	// posture
	if (this->posture_sched_p)
	{
		MeCtScheduler2::VecOfTrack tracks = posture_sched_p->tracks();
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

void SBCharacter::notify(SBSubject* subject)
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "createPhysics")
		{
			SmartBody::SBPhysicsManager* manager = SmartBody::SBScene::getScene()->getPhysicsManager();
			manager->createPhysicsCharacter(this->getName());
			return;
		}

		const std::string& attrName = attribute->getName();
		if (attrName == "visemecurve")
		{
			SmartBody::BoolAttribute* curveAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			set_viseme_curve_mode(curveAttribute->getValue());
		}
		else if (attrName == "visemetimedelay")
		{
			SmartBody::DoubleAttribute* timeDelayAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			set_viseme_time_delay((float) timeDelayAttribute->getValue());
		}
		else if (attrName == "useDiphone")
		{
			SmartBody::BoolAttribute* diphoneAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			this->setDiphone(diphoneAttribute->getValue());
		}
		else if (attrName == "diphoneSplineCurve")
		{
			SmartBody::BoolAttribute* splineCurveAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			this->setDiphoneSplineCurve(splineCurveAttribute->getValue());
		}
		else if (attrName == "diphoneSmoothWindow")
		{
			SmartBody::DoubleAttribute* smoothWindowAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			setDiphoneSmoothWindow((float)smoothWindowAttribute->getValue());
		}
		else if (attrName == "deformableMesh")
		{
			SmartBody::StringAttribute* meshAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			std::stringstream strstr;
			strstr << "char " << getName() << " mesh " << meshAttribute->getValue();
			SmartBody::DoubleAttribute* meshScaleAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(getAttribute("deformableMeshScale"));
			if (meshScaleAttribute && meshScaleAttribute->getValue() !=  1.0)
			{
				strstr << " -scale " << meshScaleAttribute->getValue();
			}

			mcuCBHandle& mcu = mcuCBHandle::singleton();
			int success = mcu.execute((char*) strstr.str().c_str());
			if (success != CMD_SUCCESS)
			{
				LOG("Problem setting attribute 'mesh' on character %s", getName().c_str());
			}
		}
		else if (attrName == "voice")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			this->setVoice(strAttribute->getValue());
		}
		else if (attrName == "voiceCode")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			this->setVoiceCode(strAttribute->getValue());
		}
		else if (attrName == "voiceBackup")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			this->setVoiceBackup(strAttribute->getValue());
		}
		else if (attrName == "voiceBackupCode")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			this->setVoiceBackupCode(strAttribute->getValue());
		}
		if (attrName.find("steering.") == 0)
		{
			// update the steering params on the next evaluation cycle
			if (steeringAgent)
				steeringAgent->setSteerParamsDirty(true);
		}
	}

	SbmCharacter::notify(subject);
	SBPawn::notify(subject);
}
};
