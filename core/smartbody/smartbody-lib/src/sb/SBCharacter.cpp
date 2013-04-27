#include "SBCharacter.h"
#include <sb/SBSkeleton.h>

#include <sbm/mcontrol_callbacks.h>
#include "sb/SBController.h"
#include "bml/bml_types.hpp"
#include "bml/bml_speech.hpp"
#include "sb/SBBehavior.h"
#include <sbm/PPRAISteeringAgent.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBScene.h>
#include <sb/SBSpeechManager.h>
#include <sb/SBBmlProcessor.h>
#include <controllers/me_ct_motion_recorder.h>
#include <controllers/me_ct_scheduler2.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_controller_tree_root.hpp>
#include <sbm/remote_speech.h>
#include <sbm/local_speech.h>
#include <sbm/text_speech.h>
#include <sbm/sbm_speech_audiofile.hpp>
#include <bml/bml_processor.hpp>

namespace SmartBody {

SBCharacter::SBCharacter() : SbmCharacter()
{
	
}

SBCharacter::SBCharacter(std::string name, std::string type) : SbmCharacter(name.c_str(), type)
{
	
	
	createBoolAttribute("gestureRequest.autoGestureTransition", true, true, "Basic", 89, false, false, false, "Whether SmartBody should filter gestures behaviors according to priority."); 
	createBoolAttribute("gestureRequest.matchingHandness", true, true, "Basic", 90, false, false, false, "Whether SmartBody should filter gestures behaviors according to priority."); 
	createBoolAttribute("gestureRequest.enableTransitionToStroke", false, true, "Basic", 91, false, false, false, "Enable Transition to stroke posture directly if time is too limited."); 
	createBoolAttribute("gestureRequest.gestureLog", false, true, "Basic", 92, false, false, false, "Toggle for logging the gesture request process."); 
	createBoolAttribute("gestureRequest.matchingSpeed", true, true, "Basic", 93, false, false, false, "Holding previous gesture if necessary to match up with current gesture stroke speed."); 
	createDoubleAttribute("gestureRequest.gestureSpeedThreshold", 1.5, true, "Basic", 95, false, false, false, "The speed threshold used to determine whether it's suitable to transition from one gesture to another"); 
	createDoubleAttribute("gestureRequest.gestureWristActiveThreshold", 0.15, true, "Basic", 96, false, false, false, "The speed threshold used to determine if this hand moving."); 
	createStringAttribute("gestureMap", "", true, "Basic", 50, false, false, false, "Name of the gesture map to use.");
	createStringAttribute("gestureMapMeek", "", true, "Basic", 51, false, false, false, "Name of the gesture map to use that is meek.");
	createStringAttribute("gestureMapEmphatic", "", true, "Basic", 52, false, false, false, "Name of the gesture map to use that is emphatic.");


	std::vector<std::string> gesturePolicyVec;
	gesturePolicyVec.push_back("random");
	gesturePolicyVec.push_back("first");
	StringAttribute* gesturePolicyAttr = createStringAttribute("gesturePolicy", "random", true, "Basic", 60, false, false, false, "Gesture policy to be used");
	gesturePolicyAttr->setValidValues(gesturePolicyVec);

	createBoolAttribute("visemecurve", false, true, "Basic", 100, false, false, false, "Use curve-based visemes instead of discrete visemes.");
	createBoolAttribute("reach.useLocomotion", false, true, "Basic", 110, false, false, false, "Whether to use locomotion for reach by default.");
	createBoolAttribute("useDiphone", true, true, "Basic", 150, false, false, false, "Use diphones.");
	createDoubleAttribute("diphoneScale", 1, true, "Basic", 155, false, false, false, "Scale factor for diphone curves.");
	createStringAttribute("diphoneSetName", "", true, "Basic", 155, false, false, false, "Name of the diphone set to be used when using diphone-based lip-syncing.");
	createBoolAttribute("diphoneSplineCurve", true, true, "Basic", 156, false, false, false, "Use diphones spline/linear curve.");
	createDoubleAttribute("diphoneSmoothWindow", .15, true, "Basic", 157, false, false, false, "Smooth window size. If it's less than 0, don't do smooth.");
	createDoubleAttribute("diphoneSpeedLimit", 8.0f, true, "Basic", 158, false, false, false, "Speed Limit of mouth movement");
	
#if 0	// Dom
	createDoubleAttribute("jaw_rot_min", -4.0, true, "Basic", 159, false, false, false, "");
	createDoubleAttribute("jaw_rot_max", 13, true, "Basic", 160, false, false, false, "");
	createDoubleAttribute("jaw_rot_default", 0.21, true, "Basic", 161, false, false, false, "");

	createDoubleAttribute("lower_lip_ftuck_min", -15, true, "Basic", 162, false, false, false, "");
	createDoubleAttribute("lower_lip_ftuck_max", 10, true, "Basic", 163, false, false, false, "");

	createDoubleAttribute("upper_lip_raise_min", -30, true, "Basic", 164, false, false, false, "");
	createDoubleAttribute("upper_lip_raise_max", 20, true, "Basic", 165, false, false, false, "");
	createDoubleAttribute("upper_lip_raise_default", -2.96, true, "Basic", 166, false, false, false, "");

	createDoubleAttribute("cheek_hollow_min", -1.5, true, "Basic", 167, false, false, false, "");
	createDoubleAttribute("cheek_hollow_max", 1.5, true, "Basic", 168, false, false, false, "");

	createDoubleAttribute("lower_lip_roll_min", -20, true, "Basic", 169, false, false, false, "");
	createDoubleAttribute("lower_lip_roll_max", 40, true, "Basic", 170, false, false, false, "");

	createDoubleAttribute("jaw_thrust_min", -10, true, "Basic", 171, false, false, false, "");
	createDoubleAttribute("jaw_thrust_max", 30, true, "Basic", 172, false, false, false, "");

	createDoubleAttribute("lip_corner_zip_min", -20, true, "Basic", 173, false, false, false, "");
	createDoubleAttribute("lip_corner_zip_max", 30, true, "Basic", 174, false, false, false, "");

	createDoubleAttribute("lower_lip_raise_min", -25, true, "Basic", 175, false, false, false, "");
	createDoubleAttribute("lower_lip_raise_max", 15, true, "Basic", 176, false, false, false, "");

	createDoubleAttribute("lip_rounding_min", 0, true, "Basic", 177, false, false, false, "");
	createDoubleAttribute("lip_rounding_max", 20, true, "Basic", 178, false, false, false, "");
	createDoubleAttribute("lip_rounding_default", 6.106, true, "Basic", 179, false, false, false, "");

	createDoubleAttribute("lip_retraction_min", 0, true, "Basic", 180, false, false, false, "");
	createDoubleAttribute("lip_retraction_max", 15, true, "Basic", 181, false, false, false, "");
	createDoubleAttribute("lip_retraction_default", 2.809, true, "Basic", 182, false, false, false, "");

	createDoubleAttribute("tongue_tip_y_min", -2, true, "Basic", 180, false, false, false, "");
	createDoubleAttribute("tongue_tip_y_max", 2, true, "Basic", 181, false, false, false, "");

	createDoubleAttribute("tongue_tip_z_min", -2, true, "Basic", 182, false, false, false, "");
	createDoubleAttribute("tongue_tip_z_max", 2, true, "Basic", 183, false, false, false, "");

	createBoolAttribute("handtune", true, true, "Basic", 184, false, false, false, "");
#endif

	createBoolAttribute("ikPostFix", false, true, "Basic", 200, false, false, false, "Post-Processing IK to fix foot sliding.");
	createBoolAttribute("terrainWalk", false, true, "Basic", 200, false, false, false, "Post-Processing to adjust the character for different terrain height. ikPostFix must be on for this to work.");
	createBoolAttribute("onlineRetarget", true, true, "Basic", 200, false, false, false, "Use on-line retargeting to adjust joint angles when playing animation blend.");


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


void SBCharacter::startMotionRecord( double frameRate )
{
	if (record_ct)
	{
		record_ct->startRecording(frameRate);
	}
}

void SBCharacter::stopMotionRecord( const std::string& motionName )
{
	if (record_ct)
	{
		record_ct->stopRecording(motionName);
	}

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
	SBPawn::setName(name);

	SmartBody::SBScene::getScene()->updateCharacterNames();

}

void SBCharacter::setType(const std::string& type)
{
	setClassType(type);
}

std::string SBCharacter::getType()
{
	return getClassType();
}

void SBCharacter::setMeshMap(std::string filename)
{
	mcu_character_load_skinweights( getName().c_str(), filename.c_str(), SmartBody::SBScene::getScene()->getCommandManager(), 1.0f );
}

void SBCharacter::addMesh(std::string mesh)
{
	mcu_character_load_mesh( getName().c_str(), mesh.c_str(), SmartBody::SBScene::getScene()->getCommandManager() );
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
	prune_controller_tree();
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
	if (type == "")
	{
		set_speech_impl(NULL);
	}
	else if (type == "remote")
	{
		set_speech_impl(SmartBody::SBScene::getScene()->getSpeechManager()->speech_rvoice());
	}
	else if (type == "audiofile")
	{
		set_speech_impl(SmartBody::SBScene::getScene()->getSpeechManager()->speech_audiofile());
	}
	else if (type == "text")
	{
		set_speech_impl(SmartBody::SBScene::getScene()->getSpeechManager()->speech_text());
	}
	else if (type == "local")
	{
		set_speech_impl(SmartBody::SBScene::getScene()->getSpeechManager()->speech_localvoice());
	}
	else
	{
		LOG("Unknown voice setting '%s'.", type.c_str());
	}

	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voice"));
	if (attr)
		attr->setValueFast(type);

}

const std::string SBCharacter::getVoice()
{
	/*
	
	std::string type = "";
	if (mcu.speech_rvoice())
		type = "remote";
	if (mcu.speech_audiofile())
		type = "audiofile";
	if (mcu.speech_text())
		type = "text";
	if (mcu.speech_localvoice())
		type = "local";
	*/
	// through attribute system...
	std::string type = "";
	StringAttribute* attr = dynamic_cast<StringAttribute*>(getAttribute("voice"));
	if (attr)
		type = attr->getValue();
	return type;
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
	

	if (type == "")
	{
		set_speech_impl_backup(NULL);
	}
	else if (type == "remote")
	{
		set_speech_impl_backup(SmartBody::SBScene::getScene()->getSpeechManager()->speech_rvoice());
	}
	else if (type == "audiofile")
	{
		set_speech_impl_backup(SmartBody::SBScene::getScene()->getSpeechManager()->speech_audiofile());
	}
	else if (type == "text")
	{
		set_speech_impl_backup(SmartBody::SBScene::getScene()->getSpeechManager()->speech_text());
	}
	else if (type == "local")
	{
		set_speech_impl_backup(SmartBody::SBScene::getScene()->getSpeechManager()->speech_localvoice());
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

	BML::MapOfBmlRequest bmlRequestMap = SmartBody::SBScene::getScene()->getBmlProcessor()->getBMLProcessor()->getBMLRequestMap();
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
	BML::MapOfBmlRequest bmlRequestMap = SmartBody::SBScene::getScene()->getBmlProcessor()->getBMLProcessor()->getBMLRequestMap();
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

	// speech
	BML::MapOfBmlRequest bmlRequestMap = SmartBody::SBScene::getScene()->getBmlProcessor()->getBMLProcessor()->getBMLRequestMap();
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
	SmartBody::SBSteerManager* manager = SmartBody::SBScene::getScene()->getSteerManager();
	SmartBody::SBSteerAgent* steerAgent = manager->getSteerAgent(this->getName());
	if (steerAgent)
	{
		LocomotionBehavior* locoBehavior = new LocomotionBehavior();
		PPRAISteeringAgent* ppraiAgent = dynamic_cast<PPRAISteeringAgent*>(steerAgent);
		if (ppraiAgent->getAgent())
		{
			const SteerLib::AgentGoalInfo& goal = ppraiAgent->getAgent()->currentGoal();
			Util::Point goalTarget = goal.targetLocation;
		
			SrVec target(goalTarget.x, 0.f, goalTarget.z);
			bool reachTarget = _reachTarget && !_lastReachStatus;
			locoBehavior->setLocomotionTarget(target);
			locoBehavior->setReachTarget(reachTarget);
		}
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
				if (joint)
				{
					SkSkeleton* skeleton = joint->skeleton();
					SBSkeleton* sbSkel = dynamic_cast<SBSkeleton*> (skeleton);
					SBPawn* sbPawn = sbSkel->getPawn();
					
					GazeBehavior* gazeBehavior = new GazeBehavior();
					gazeBehavior->setGazeTarget(sbPawn->getName());
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

void SBCharacter::notify(SBSubject* subject)
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
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
		else if (attrName == "diphoneScale")
		{
			SmartBody::DoubleAttribute* diphoneScaleAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			this->setDiphoneScale((float)diphoneScaleAttribute->getValue());
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
		else if (attrName == "diphoneSpeedLimit")
		{
			SmartBody::DoubleAttribute* speedLimitAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			setDiphoneSpeedLimit((float)speedLimitAttribute->getValue());
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

			int success = SmartBody::SBScene::getScene()->getCommandManager()->execute((char*) strstr.str().c_str());
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
			SmartBody::SBSteerManager* steerManager = SmartBody::SBScene::getScene()->getSteerManager();
			SmartBody::SBSteerAgent* steerAgent = steerManager->getSteerAgent(getName());
			if (steerAgent)
			{
				PPRAISteeringAgent* ppraiAgent = dynamic_cast<PPRAISteeringAgent*>(steerAgent);
				ppraiAgent->setSteerParamsDirty(true);
			}
		}
#if 1 // rocket box test		
		if (attrName == "_1testHead")
		{
			scene->run("testHead('"+getName()+"',0.0)");		
		}
		else if (attrName == "_2testGaze")
		{
			scene->run("testGaze('"+getName()+"',0.0)");		
		}
		else if (attrName == "_3testGesture")
		{
			scene->run("testGesture('"+getName()+"',0.0)");		
		}
		else if (attrName == "_4testReach")
		{
			scene->run("testReach('"+getName()+"',0.0)");		
		}
		else if (attrName == "_5testLocomotion")
		{
			scene->run("testLocomotion('"+getName()+"',0.0)");		
		}
		
#endif
	}

	SbmCharacter::notify(subject);
	SBPawn::notify(subject);
}

};
