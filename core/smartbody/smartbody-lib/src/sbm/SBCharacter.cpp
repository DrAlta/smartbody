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


void SBCharacter::addController(SBController* controller)
{
	if (!controller)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	MeControllerTreeRoot* controllerTree = ct_tree_p;
	controllerTree->add_controller(controller);

	linkControllers(controller);
}

bool SBCharacter::isFaceNeutral()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* face_neutral_p = NULL;
	std::map<std::string, FaceDefinition*>::iterator faceIter = mcu.face_map.find(std::string(getName()));
	if (faceIter !=  mcu.face_map.end())
	{
		FaceDefinition* faceDefinition = (*faceIter).second;
		if (mcu.net_face_bones)
			face_neutral_p = faceDefinition->getFaceNeutral();
	}
	else
	{
		// get the default face motion mapping
		faceIter = mcu.face_map.find("_default_");
		if (faceIter !=  mcu.face_map.end())
		{
			FaceDefinition* faceDefinition = (*faceIter).second;
			if (mcu.net_face_bones)
				face_neutral_p = faceDefinition->getFaceNeutral();
		}
		else
		{
			LOG("Couldn't find _default_ face motion mappings! Check code.");
		}
	}	
	if (face_neutral_p)
		return true;
	else
		return false;
}

bool SBCharacter::initFaceController(MeCtFace* faceCtrl)
{/*
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	FaceMotion* faceDefinition = NULL;
		// get the face motion mapping per character
	std::map<std::string, FaceDefinition*>::iterator faceIter = mcu.face_map.find(std::string(getName()));
	if (faceIter !=  mcu.face_map.end())
	{
		faceDefinition = (*faceIter).second;
	}
	else
	{
		// get the default face motion mapping
		faceIter = mcu.face_map.find("_default_");
		if (faceIter !=  mcu.face_map.end())
		{
			faceDefinition = (*faceIter).second;
		}
		else
		{
			LOG("Couldn't find _default_ face motion mappings! Check code.");
		}
	}

	if (mcu.net_face_bones)
	{
		SkMotion* faceNeutralMotion = faceDefinition->getFaceNeutral();
		faceCtrl->ref();
		faceCtrl->init(faceNeutralMotion);
	}

	// adding other key poses
	int wo_index = get_world_offset_joint()->index();
	std::string viseme_start_name;
	int visemeChannelCount = 0;
	{	// Generate AU and viseme activation channels.
		AUMotionMap::const_iterator i   = auMotionMap->begin();
		AUMotionMap::const_iterator end = auMotionMap->end();
		
		for(; i != end; ++i ) {
			//const int   id = i->first;
			std::stringstream id;
			id << i->first;
			AUMotionPtr au( i->second );

			if( au->is_left() ) {
				std::string name = "au_";
				//name += id;
				name += id.str();
				name += "_left";

				// Create the AU control channel
//				character->add_face_channel( name, wo_index );
				if (visemeChannelCount == 0)	viseme_start_name = name;
				visemeChannelCount ++;

				// TODO: Add to au_channel_map (?)

				// Register control channel with face controller
				if( faceCtrl )
					faceCtrl->add_key( name.c_str(), au->left.get() );
			}
			if( au->is_right()) {
				std::string name = "au_";
				//name += id;
				name += id.str();
				name += "_right";

				// Create the AU control channel
//				character->add_face_channel( name, wo_index );
				if (visemeChannelCount == 0)	viseme_start_name = name;
				visemeChannelCount ++;

				// Register control channel with face controller
				if( faceCtrl )
					faceCtrl->add_key( name.c_str(), au->right.get() );
			}
			if (au->is_bilateral())
			{
				std::string name = "au_";
				//name += id;
				name += id.str();

				// Create the AU control channel
//				character->add_face_channel( name, wo_index );
				if (visemeChannelCount == 0)	viseme_start_name = name;
				visemeChannelCount ++;

				// Register control channel with face controller
				if( faceCtrl )
					faceCtrl->add_key( name.c_str(), au->left.get() );
			}
		}

		int numVisemes = faceDefinition->getNumVisemes();
		for (int v = 0; v < numVisemes; v++)
		{
			const std::string&    id     = vi->first;
			SkMotion* motion = vi->second;

			if( motion ) {
				// Create the Viseme control channel
//				character->add_face_channel( id, wo_index );
				if (visemeChannelCount == 0)	viseme_start_name = id;
				visemeChannelCount ++;
				
				// Register control channel with face controller
				if (face_neutral_p)
					faceCtrl->add_key( id.c_str(), motion );
			}
		}
	}
	if (face_neutral_p)
		faceCtrl->finish_adding();	

	// activate the channels
	getSkeleton()->make_active_channels();

	//int skel, visgeo, colgeo, axis;
	//character->scene_p->get_visibility(skel, visgeo, colgeo, axis);
	int deformgeo = dMesh_p->get_visibility();
	//character->scene_p->init(character->getSkeleton());
	//character->scene_p->set_visibility(skel, visgeo, colgeo, axis);
	dMesh_p->set_visibility(deformgeo);
	init_viseme_channel_index(viseme_start_name, visemeChannelCount);

	if (face_neutral_p)	
		return true;
	else
		return false;
		*/
return true;
}

void SBCharacter::initLocomotion(MeCtLocomotion* locoCtrl)
{
	/*
	mcuCBHandle& mcu = mcuCBHandle::singleton();	

	const std::string& wholeName = getSkeleton()->skfilename();
	char* skFileName = strrchr(const_cast<char*> (wholeName), (int)'/');
	skFileName++;
	SkSkeleton* walking_skeleton = load_skeleton( skFileName, mcu.me_paths, mcu.resource_manager );
	SkSkeleton* standing_skeleton = load_skeleton(skFileName, mcu.me_paths, mcu.resource_manager );
	locoCtrl->init_skeleton(walking_skeleton, standing_skeleton);

	locoCtrl->get_navigator()->setWordOffsetController(get_world_offset_writer_p());
	*/
}

void SBCharacter::linkControllers(SBController* controller)
{
	/*
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	MeCtScheduler2* scheduler = dynamic_cast<MeCtScheduler2*>(controller);
	if (scheduler)
	{
		if (strcmp(scheduler->name(), "posture") == 0)
		{
			if (posture_sched_p)
				posture_sched_p->unref();
			posture_sched_p = scheduler;	
			posture_sched_p->ref();
		}
		else if (strcmp(scheduler->name(), "motion") == 0)
		{
			if (motion_sched_p)
				motion_sched_p->unref();
			motion_sched_p = scheduler;	
			motion_sched_p->ref();
		}
		else if (strcmp(scheduler->name(), "gaze") == 0)
		{
			if (gaze_sched_p)
				gaze_sched_p->unref();
			gaze_sched_p = scheduler;	
			gaze_sched_p->ref();
		}
		else if (strcmp(scheduler->name(), "head") == 0)
		{
			if (head_sched_p)
				head_sched_p->unref();
			head_sched_p = scheduler;	
			head_sched_p->ref();
		}
		else
			return;
	}

	MeCtLocomotion* locoCtrl = dynamic_cast<MeCtLocomotion*> (controller);
	if (locoCtrl)
	{
		if (strcmp(locoCtrl->name(), "locomotion") == 0)
		{
			initLocomotion(locoCtrl);
			locoCtrl->set_enabled(true);
			if (locomotion_ct)
				locomotion_ct->unref();
			locomotion_ct = locoCtrl;
			locomotion_ct->ref();
		}
	}

	MeCtEyeLidRegulator* blinkCtrl = dynamic_cast<MeCtEyeLidRegulator*>(controller);
	if (blinkCtrl) //init blinking controller
	{
		if (strcmp(blinkCtrl->name(), "blink") == 0)
		{
			if (eyelid_reg_ct_p)
				eyelid_reg_ct_p->unref();
			eyelid_reg_ct_p = blinkCtrl;
			eyelid_reg_ct_p->ref();
		}
	}

	MeCtFace* faceCtrl = dynamic_cast<MeCtFace*> (controller);
	if (faceCtrl)
	{
		if (strcmp(faceCtrl->name(), "face") == 0)
		{
			bool faceNeutral = initFaceController(faceCtrl);
			if (faceNeutral)
			{
				if (face_ct)
					face_ct->unref();
				face_ct = faceCtrl;
				face_ct->ref();
			}
		}
	}

	MeCtEyeLid* eyeLidCtrl = dynamic_cast<MeCtEyeLid*> (controller);
	if (eyeLidCtrl)
	{
		if (strcmp(eyeLidCtrl->name(), "eyelid") == 0)
		{
			if (isFaceNeutral())
			{
				// adjust according to the characters
				float dfl_hgt = 175.0f;
				float rel_scale = getHeight() / dfl_hgt;
				float lo, up;
				eyeLidCtrl->get_upper_lid_range( lo, up );
				eyeLidCtrl->set_upper_lid_range( lo * rel_scale, up * rel_scale );
				eyeLidCtrl->get_lower_lid_range( lo, up );
				eyeLidCtrl->set_lower_lid_range( lo * rel_scale, up * rel_scale );
			}
		}
	}
	*/
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
						_curBehaviors.push_back(gazeBehavior);
						break;
					}
				}
			}
		}
	}

	return _curBehaviors;
}


};
