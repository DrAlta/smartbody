/*
*  sbm_character.cpp - part of SmartBody-lib
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
*      Marcus Thiebaux, USC
*      Ed Fast, USC
*      Ashok Basawapatna, USC (no longer)
*/

#include "vhcl.h"
#include "sbm_character.hpp"

#include <stdio.h>

#include <iostream>
#include <string>
#include <cstring>

#include <sk/sk_skeleton.h>
#include <me/me_ct_blend.hpp>
#include <me/me_ct_time_shift_warp.hpp>
#include "mcontrol_util.h"
#include "mcontrol_callbacks.h"
#include "me_utilities.hpp"
#include <me/me_spline_1d.hpp>
#include <me/me_ct_interpolator.h>
#include "sr_curve_builder.h"
#include "lin_win.h"
#include <boost/filesystem/operations.hpp>
#include <sbm/SBSkeleton.h>
#include <sbm/SBJoint.h>

#define USE_REACH 1
#define USE_PHYSICS_CHARACTER 0
//#define USE_REACH_TEST 0
const bool LOG_PRUNE_CMD_TIME							= false;
const bool LOG_CONTROLLER_TREE_PRUNING					= false;
const bool LOG_PRUNE_TRACK_WITHOUT_BLEND_SPLIE_KNOTS	= false;
const bool ENABLE_EYELID_CORRECTIVE_CT					= false;

using namespace std;

// Predeclare private functions defined below
static int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );
static int set_voicebackup_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );

#if 0
static inline bool parse_float_or_error( float& var, const char* str, const string& var_name );
//  Private sbm_character functions
// Because I don't like c style error checking, I'm avoiding srArgBuffer::read_float
bool parse_float_or_error( float& var, const char* str, const string& var_name ) {
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	LOG("ERROR: Invalid value for %s: %s", var_name.c_str(), str);
	return false;
}
#endif

/////////////////////////////////////////////////////////////
//  Static Data

/////////////////////////////////////////////////////////////
//  Method Definitions

MeCtSchedulerClass* CreateSchedulerCt( const char* character_name, const char* sched_type_name ) {

	MeCtSchedulerClass* sched_p = new MeCtSchedulerClass();
	sched_p->active_when_empty( true );
	string sched_name( character_name );
	sched_name += "'s ";
	sched_name += sched_type_name;
	sched_name += " schedule";
	sched_p->setName( sched_name.c_str() );
	return sched_p;
}

SbmCharacter::SbmCharacter() : SBPawn()
{
	SbmCharacter::initData();
	setClassType("");
}

SbmCharacter::SbmCharacter( const char* character_name, std::string type)
:	SBPawn( character_name )
{
	SbmCharacter::initData();
	setClassType(type);
}

//  Constructor
SbmCharacter::SbmCharacter( const char* character_name )
:	SBPawn( character_name ),

posture_sched_p( CreateSchedulerCt( character_name, "posture" ) ),
motion_sched_p( CreateSchedulerCt( character_name, "motion" ) ),
breathing_p( ),
gaze_sched_p( CreateSchedulerCt( character_name, "gaze" ) ),
locomotion_ct( NULL ),
eyelid_reg_ct_p( NULL ),
#ifdef USE_REACH
constraint_sched_p( CreateSchedulerCt( character_name, "constraint" ) ),
reach_sched_p( CreateSchedulerCt( character_name, "reach" ) ),
grab_sched_p( CreateSchedulerCt( character_name, "grab" ) ),
#else
reach_sched_p( NULL ),
#endif
head_sched_p( CreateSchedulerCt( character_name, "head" ) ),
param_sched_p( CreateSchedulerCt( character_name, "param" ) ),
param_animation_ct( NULL ),
face_ct( NULL ),
eyelid_ct( new MeCtEyeLid() ),
motionplayer_ct( NULL ),	
face_neutral( NULL ),
_soft_eyes_enabled( ENABLE_EYELID_CORRECTIVE_CT )
{
	posture_sched_p->ref();
	motion_sched_p->ref();
	breathing_p->ref();
	gaze_sched_p->ref();
	head_sched_p->ref();
	param_sched_p->ref();
	eyelid_ct->ref();
	phyChar = NULL;
}


//  Destructor
SbmCharacter::~SbmCharacter( void )	{

	printf("delete character %s\n",this->getName().c_str());

	if (_faceDefinition)
		delete _faceDefinition;

	if (posture_sched_p)
		posture_sched_p->unref();
	if (motion_sched_p)
		motion_sched_p->unref();
	if (breathing_p)
		breathing_p->unref();
	if (gaze_sched_p)
		gaze_sched_p->unref();
	if (reach_sched_p)
		reach_sched_p->unref();

	if (grab_sched_p)
		grab_sched_p->unref();

	if (constraint_sched_p)
		constraint_sched_p->unref();

	if( eyelid_reg_ct_p )
		eyelid_reg_ct_p->unref();

	if (head_sched_p)
		head_sched_p->unref();
	if (param_sched_p)
		param_sched_p->unref();
	if( face_ct )
		face_ct->unref();
	/*
	if (eyelid_ct)
	eyelid_ct->unref();
	*/

	if (locomotion_ct)
		locomotion_ct->unref();
	if (param_animation_ct)
		param_animation_ct->unref();
	if (motionplayer_ct)
		motionplayer_ct->unref();
	if (saccade_ct)
		saccade_ct->unref();

	std::map<int,MeCtReachEngine*>::iterator mi;
	for ( mi  = reachEngineMap.begin();
		mi != reachEngineMap.end();
		mi++)
	{
		MeCtReachEngine* re = mi->second;
		delete re;
	}
	reachEngineMap.clear();

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterDelete( getName() );
	}

	if ( bonebusCharacter )
	{
		mcuCBHandle::singleton().bonebus.DeleteCharacter( bonebusCharacter );
		bonebusCharacter = NULL;
	}

	if( viseme_history_arr )	{
		delete [] viseme_history_arr;
		viseme_history_arr = NULL;
	}

	delete steeringAgent;
	if (_miniBrain)
		delete _miniBrain;
}

void SbmCharacter::createStandardControllers()
{
	posture_sched_p = CreateSchedulerCt( getName().c_str(), "posture" );
	motion_sched_p = CreateSchedulerCt( getName().c_str(), "motion" );

	// procedural locomotion
	this->locomotion_ct =  new MeCtLocomotionClass();
	std::string locomotionname = getName() + "'s locomotion controller";
	this->locomotion_ct->setName( locomotionname.c_str() );
	locomotion_ct->get_navigator()->setWordOffsetController(world_offset_writer_p);
	locomotion_ct->init_skeleton(this->getSkeleton(), this->getSkeleton());
	locomotion_ct->ref();

	// example-based locomotion
	this->param_animation_ct = new MeCtParamAnimation(this, world_offset_writer_p);
	std::string paramAnimationName = getName() + "'s param animation controller";
	this->param_animation_ct->setName(paramAnimationName.c_str());
	param_sched_p = CreateSchedulerCt( getName().c_str(), "param" );

	// basic locomotion
	this->basic_locomotion_ct = new MeCtBasicLocomotion(this);
	std::string bLocoName = getName() + "'s basic locomotion controller";
	this->basic_locomotion_ct->setName(bLocoName.c_str());
	//this->basic_locomotion_ct->set_pass_through(false);



	SkJoint* effector = this->_skeleton->search_joint("r_middle1");
	if (effector)
	{
		MeCtReachEngine* rengine = new MeCtReachEngine(this,this->_skeleton);
		rengine->init(MeCtReachEngine::RIGHT_ARM,effector);
		this->reachEngineMap[MeCtReachEngine::RIGHT_ARM] = rengine;		
	}	

	SkJoint* leftEffector = this->_skeleton->search_joint("l_middle1");
	if (leftEffector)
	{
		MeCtReachEngine* rengine = new MeCtReachEngine(this,this->_skeleton);
		rengine->init(MeCtReachEngine::LEFT_ARM,leftEffector);
		this->reachEngineMap[MeCtReachEngine::LEFT_ARM] = rengine;		
	}

	

	constraint_sched_p = CreateSchedulerCt( getName().c_str(), "constraint" );
	reach_sched_p = CreateSchedulerCt( getName().c_str(), "reach" );
	grab_sched_p = CreateSchedulerCt( getName().c_str(), "grab" );

	breathing_p = new MeCtBreathing();
	breathing_p->setName(getName() + "'s breathing controller");
	// add two channels for blendshape-based breathing
	SmartBody::SBSkeleton* sbSkel = dynamic_cast<SmartBody::SBSkeleton*>(getSkeleton());
	SmartBody::SBJoint* rootJoint = dynamic_cast<SmartBody::SBJoint*>(sbSkel->root());

	if (rootJoint)
	{
		SmartBody::SBJoint* breathingJointX = new SmartBody::SBJoint();
		std::string breathNameX = "breathX"; // parametric breath time
		breathingJointX->setName(breathNameX);
		breathingJointX->setJointType(SkJoint::TypeOther);
		breathingJointX->setUsePosition(0, true);
		breathingJointX->pos()->limits(SkJointPos::X, -1000, 1000);  // Setting upper bound to 2 allows some exageration
		rootJoint->addChild(breathingJointX);

		SmartBody::SBJoint* breathingJointY = new SmartBody::SBJoint();
		std::string breathNameY = "breathY"; // breathing intensity
		breathingJointY->setName(breathNameY);
		breathingJointY->setJointType(SkJoint::TypeOther);
		breathingJointY->setUsePosition(0, true);
		breathingJointY->pos()->limits(SkJointPos::X, -1000, 1000);  // Setting upper bound to 2 allows some exageration
		rootJoint->addChild(breathingJointY);
	}

	gaze_sched_p = CreateSchedulerCt( getName().c_str(), "gaze" );

	head_sched_p = CreateSchedulerCt( getName().c_str(), "head" );
	face_ct = new MeCtFace();
	string faceCtName( getName() );
	faceCtName += "'s face_ct";
	face_ct->setName(faceCtName);

	eyelid_reg_ct_p = new MeCtEyeLidRegulator();
	eyelid_reg_ct_p->ref();
	if (!_faceDefinition || !_faceDefinition->getFaceNeutral())
	{
		eyelid_reg_ct_p->set_use_blink_viseme( true );
		LOG("Character %s will use 'blink' viseme to control blinking.", getName().c_str());
	}
	else
	{
		LOG("Character %s will use FAC 45 left and right to control blinking.", getName().c_str());
	}
	eyelid_reg_ct_p->init(this, true);
	eyelid_reg_ct_p->set_upper_range( -30.0, 30.0 );
	eyelid_reg_ct_p->set_close_angle( 30.0 );
	ostringstream ct_name;
	ct_name << getName() << "'s eyelid controller";
	eyelid_reg_ct_p->setName( ct_name.str().c_str() );

	this->saccade_ct = new MeCtSaccade(this->_skeleton);
	this->saccade_ct->init(this);
	std::string saccadeCtName = getName() + "'s eye saccade controller";
	this->saccade_ct->setName(saccadeCtName.c_str());

	// motion player
	motionplayer_ct = new MeCtMotionPlayer(this);
	//	motionplayer_ct->ref();
	std::string mpName = getName();
	mpName += "'s motion player";
	motionplayer_ct->setName(mpName.c_str());
	motionplayer_ct->setActive(false);

	this->datareceiver_ct = new MeCtDataReceiver(this->_skeleton);
	std::string datareceiverCtName = getName() + "'s data receiver controller";
	this->datareceiver_ct->setName(datareceiverCtName.c_str());

	this->physics_ct = new MeCtPhysicsController(this);
	std::string physicsCtName = getName() + "'s physics controller";
	this->physics_ct->setName(physicsCtName.c_str());

	

	posture_sched_p->ref();
	motion_sched_p->ref();
	gaze_sched_p->ref();
	head_sched_p->ref();
	face_ct->ref();
	param_sched_p->ref();

	posture_sched_p->init(this);
	motion_sched_p->init(this);
	param_sched_p->init(this);
	locomotion_ct->init(this);
	breathing_p->init(this);
	gaze_sched_p->init(this);

	reach_sched_p->init(this);
	grab_sched_p->init(this);
	constraint_sched_p->init(this);

	head_sched_p->init(this);
	face_ct->init( getFaceDefinition() );

	ct_tree_p->add_controller( posture_sched_p );
	ct_tree_p->add_controller( motion_sched_p );
	ct_tree_p->add_controller( locomotion_ct );
	ct_tree_p->add_controller( param_animation_ct );
	ct_tree_p->add_controller( basic_locomotion_ct );

	ct_tree_p->add_controller( reach_sched_p );	
	ct_tree_p->add_controller( grab_sched_p );
	ct_tree_p->add_controller( breathing_p );
	ct_tree_p->add_controller( gaze_sched_p );
	ct_tree_p->add_controller( saccade_ct );
	ct_tree_p->add_controller( constraint_sched_p );	
	ct_tree_p->add_controller( eyelid_reg_ct_p );
	ct_tree_p->add_controller( head_sched_p );
	ct_tree_p->add_controller( face_ct );
	ct_tree_p->add_controller( param_sched_p );
#if USE_PHYSICS_CHARACTER
	ct_tree_p->add_controller( physics_ct );
#endif
	ct_tree_p->add_controller( motionplayer_ct );
	ct_tree_p->add_controller( datareceiver_ct );

}
void SbmCharacter::initData()
{
	posture_sched_p = NULL;
	motion_sched_p = NULL;
	gaze_sched_p = NULL;
	reach_sched_p = NULL;
	head_sched_p = NULL;
	param_sched_p = NULL;
	breathing_p = NULL;
	grab_sched_p = NULL;
	constraint_sched_p = NULL;
	param_animation_ct = NULL;
	saccade_ct = NULL;

	phyChar = NULL;

	speech_impl = NULL;
	speech_impl_backup = NULL;
	locomotion_ct = NULL;
	eyelid_reg_ct_p = NULL;
	face_ct = NULL;
	motionplayer_ct = NULL;
	use_viseme_curve = false;
	_soft_eyes_enabled = ENABLE_EYELID_CORRECTIVE_CT;
	_height = 1.0f;
	bonebusCharacter = NULL;

	param_map = new GeneralParamMap;

	use_viseme_curve = false;
	viseme_time_offset = 0.0;
	viseme_sound_offset = 0.0;
	viseme_magnitude = 1.0;
	viseme_channel_count = 0;
	viseme_channel_start_pos = 0;
	viseme_channel_end_pos = 0;
	viseme_history_arr = NULL;
	_minVisemeTime = 0.0f;
	_isControllerPruning = true;
	_classType = "";
	_minVisemeTime = 0.0f;
	_faceDefinition = NULL;
	steeringAgent = NULL;
	_numSteeringGoal = 0;
	_reachTarget = false;
	_lastReachStatus = true;
	_height = 1.0f; 
	_visemePlateau = true;
	_nvbg = NULL;
	_miniBrain = NULL;

	locomotion_type = Basic;
	statePrefix = "";

	createBoolAttribute("facebone", false, true, "Basic", 200, false, false, false, "Use bones for facial animation. If false, blendshapes will be used. If true, face will be animated via face definitions.");
	createBoolAttribute("visemecurve", false, true, "Basic", 200, false, false, false, "Use curve-based visemes instead of discrete visemes.");
	SmartBody::DoubleAttribute* timeDelayAttr = createDoubleAttribute("visemetimedelay", 0.0, true, "Basic", 210, false, false, false, "Delay visemes by a fixed amount.");
	timeDelayAttr->setMin(0.0);
	createStringAttribute("mesh", "", true, "Basic", 220, false, false, false, "Directory that contains mesh information.");
}

int SbmCharacter::init_locomotion_skeleton(const char* skel_file, mcuCBHandle *mcu_p)
{
	SkSkeleton* walking_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );
	SkSkeleton* standing_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );

	locomotion_ct->init_skeleton(standing_skeleton, walking_skeleton);

	return CMD_SUCCESS;
}

void SbmCharacter::locomotion_reset()
{
	locomotion_ct->reset = true;
}

void SbmCharacter::locomotion_set_turning_speed(float radians)
{
	//locomotion_ct->set_turning_speed(radians);
	for(int i = 0; i < locomotion_ct->limb_list.size(); ++i)
	{
		locomotion_ct->limb_list.get(i)->direction_planner.set_turning_speed(radians);
	}
}

MeCtLocomotionClass* SbmCharacter::get_locomotion_ct()
{
	return this->locomotion_ct;
}

void SbmCharacter::locomotion_ik_enable(bool enable)
{
	locomotion_ct->ik_enabled = enable;
}

bool SbmCharacter::is_locomotion_controller_initialized()
{
	if(locomotion_ct == NULL) return false;
	return locomotion_ct->is_initialized();
}

bool SbmCharacter::is_locomotion_controller_enabled()
{
	if(locomotion_ct == NULL) return false;
	return locomotion_ct->is_enabled();
}

void SbmCharacter::locomotion_set_turning_mode(int mode)
{
	for(int i = 0; i < locomotion_ct->limb_list.size(); ++i)
	{
		locomotion_ct->limb_list.get(i)->direction_planner.set_turning_mode(mode);
	}
}

void SbmCharacter::updateJointPhyObjs(bool phySim)
{
	//return;
	if (!_skeleton)
		return;
	if (!phyChar) return;

	bool charPhySim = phySim && phyChar->getBoolAttribute("enable");
	std::map<std::string,SbmJointObj*> jointPhyObjMap = phyChar->getJointObjMap();
	const std::vector<SkJoint*>& joints = _skeleton->joints();	
	_skeleton->update_global_matrices();
	for (size_t i=0;i<joints.size();i++)
	{
		SkJoint* curJoint = joints[i]; 
		const std::string& jointName = curJoint->name();
		std::map<std::string, SbmJointObj*>::iterator iter = jointPhyObjMap.find(jointName);
		bool kinematicRoot = (curJoint->name() == "base" || curJoint->name() == "JtPelvis") && phyChar->getBoolAttribute("kinematicRoot");
		if (iter != jointPhyObjMap.end())
		{
			SbmJointObj* phyObj = (*iter).second;

#if USE_PHYSICS_CHARACTER			
			if (charPhySim && !kinematicRoot)
			{
				phyObj->enablePhysicsSim(true);
				phyObj->updateSbmObj();
			}
			else
			{				
				SBJoint* curSBJoint = dynamic_cast<SBJoint*>(curJoint);
				SrMat tranMat; tranMat.translation(curSBJoint->getLocalCenter());	
				phyObj->enablePhysicsSim(false);
				//if (joint->parent()) 
				SrMat gmat = tranMat*curSBJoint->gmat();		
				phyObj->setRefTransform(phyObj->getGlobalTransform()); // save previous transform
				phyObj->setGlobalTransform(gmat);				
				phyObj->setAngularVel(phyObj->getPhyJoint()->getRefAngularVel());
				phyObj->updatePhySim();
			}		
#else
			{
				SBJoint* curSBJoint = dynamic_cast<SBJoint*>(curJoint);
				SrMat tranMat; tranMat.translation(curSBJoint->getLocalCenter());					
				SrMat gmat = tranMat*curSBJoint->gmat();		
				phyObj->setGlobalTransform(gmat);
			}			
#endif
		}
	}	
}

void SbmCharacter::setJointPhyCollision( bool useCollision )
{	
	if (!phyChar) return;
	const std::vector<SkJoint*>& joints = _skeleton->joints();	
	std::map<std::string,SbmJointObj*> jointPhyObjMap = phyChar->getJointObjMap();
	for (size_t i=0;i<joints.size();i++)
	{
		SkJoint* curJoint = joints[i];
		std::string jointName = curJoint->name();
		if (jointPhyObjMap.find(jointName) != jointPhyObjMap.end())
		{
			SbmPhysicsObj* phyObj = jointPhyObjMap[jointName];
			phyObj->enableCollisionSim(useCollision);
		}
	}	
}

void SbmCharacter::buildJointPhyObjs()
{
	const std::vector<SkJoint*>& joints = _skeleton->joints();
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	if (!phySim)
		return;
	//printf("init physics obj\n");	
	phyChar = new SbmPhysicsCharacter();
	std::queue<SkJoint*> tempJointList;
	std::vector<std::string> jointNameList;
	std::set<std::string> excludeNameList; 
	excludeNameList.insert("r_wrist");
	excludeNameList.insert("l_wrist");
	excludeNameList.insert("spine5");
	excludeNameList.insert("l_forefoot");
	excludeNameList.insert("r_forefoot");

	excludeNameList.insert("l_ankle");
	excludeNameList.insert("r_ankle");

	//excludeNameList.insert("r_sternoclavicular");
	//excludeNameList.insert("l_sternoclavicular");
	//excludeNameList.insert("r_shoulder");
	//excludeNameList.insert("l_shoulder");
	//excludeNameList.insert("r_acromioclavicular");
	//excludeNameList.insert("l_acromioclavicular");

	//excludeNameList.insert("l_hip");
	//excludeNameList.insert("r_hip");	
	//excludeNameList.insert("spine3");
	//excludeNameList.insert("r_elbow");
	//excludeNameList.insert("l_elbow");
	SkJoint* rootJoint = _skeleton->root();
	//jointNameList.push_back(rootJoint->name());
	tempJointList.push(rootJoint->child(0));
	//tempJointList.push(_skeleton->search_joint("spine2"));

	
	while (!tempJointList.empty())
	{
		SkJoint* joint = tempJointList.front(); tempJointList.pop();
		std::string jName = joint->name();
		if (joint->num_children() == 0) // don't process leaves
			continue;
		jointNameList.push_back(jName);
		if (excludeNameList.find(jName) != excludeNameList.end())
			continue;
		for (int i=0;i<joint->num_children();i++)
		{
			SkJoint* cj = joint->child(i);	
			if (std::find(joints.begin(),joints.end(),cj) != joints.end())
				tempJointList.push(cj);
		}
	}
		
	/*
	for (size_t i=0;i<joints.size();i++)
	{
		SkJoint* curJoint = joints[i];
		std::string jointName = curJoint->name();
		jointNameList.push_back(jointName);				
	}*/
	

	std::string charName = getName();
	phyChar->initPhysicsCharacter(charName,jointNameList,true);
#if USE_PHYSICS_CHARACTER
	phySim->addPhysicsCharacter(phyChar);
#endif
}
/*
void SbmCharacter::setJointCollider( std::string jointName, float len, float radius )
{
	SkJoint* joint = _skeleton->search_joint(jointName.c_str());
	if (!joint || joint->parent() == 0)
		return;
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	if (!phySim)
		return;

	std::map<std::string, SbmPhysicsObj*>::iterator mi = jointPhyObjMap.find(jointName);
	// clean up first
	if (mi != jointPhyObjMap.end())
	{
		SbmPhysicsObj* phyObj = mi->second;		
		SbmGeomObject* geomObj = phyObj->getColObj();		
		phySim->removePhysicsObj(phyObj);
		delete geomObj;
		delete phyObj;
	}

	//SkJoint* child = joint->child(0);
	SkJoint* parent = joint->parent();
	SrVec offset = joint->offset(); 
	SrVec center = offset*0.5f;
	SrVec dir = offset; dir.normalize();
	float boneLen = offset.len();	
	if (len <= 0.f)
		len = boneLen+0.001f;
	if (radius <= 0.f)
		radius = len*0.2f;	

	// generate new geometry
	SbmGeomObject* newGeomObj = new SbmGeomCapsule(center-dir*len*0.5f, center+dir*len*0.5f,radius);
	SbmPhysicsObj* jointPhy = phySim->createPhyObj();	
	jointPhy->setGeometry(newGeomObj,10.f);	
	phySim->addPhysicsObj(jointPhy);
	phySim->updatePhyObjGeometry(jointPhy);		
	jointPhy->enablePhysicsSim(false);	
	jointPhyObjMap[jointName] = jointPhy;
}
*/




int SbmCharacter::init(SkSkeleton* new_skeleton_p,
					   SmartBody::SBFaceDefinition* faceDefinition,
					   GeneralParamMap* param_map,
					   const char* classType,
					   bool use_locomotion,
					   bool use_param_animation,
					   bool use_data_receiver)
{

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// Store pointers for access via init_skeleton()

	this->param_map = param_map;

	if (!scene_p)
	{
		scene_p = new SkScene();
		scene_p->ref();
	}

	int init_result = SbmPawn::init( new_skeleton_p );  // Indirectly calls init_skeleton 
	if( init_result!=CMD_SUCCESS ) {
		return( init_result ); 
	}

	setClassType(classType);
	float height = new_skeleton_p->getCurrentHeight();
	setHeight(height);

	setFaceDefinition(faceDefinition);
	createStandardControllers();

	/*
	eyelid_reg_ct_p = new MeCtEyeLidRegulator();
	if( eyelid_reg_ct_p )	{
	eyelid_reg_ct_p->ref();
	if (!face_neutral)
	eyelid_reg_ct_p->set_use_blink_viseme( true );

	eyelid_reg_ct_p->init(this,true);
	eyelid_reg_ct_p->set_upper_range( -30.0, 30.0 );
	eyelid_reg_ct_p->set_close_angle( 30.0 );
	ostringstream ct_name;
	ct_name << name << "'s eyelid controller";
	eyelid_reg_ct_p->name( ct_name.str().c_str() );
	}

	if (use_param_animation)
	{
	this->param_animation_ct = new MeCtParamAnimation(this, world_offset_writer_p);
	std::string paramAnimationName = std::string(name)+ "'s param animation controller";
	this->param_animation_ct->name(paramAnimationName.c_str());
	//this->param_animation_ct->set_pass_through(true);
	}

	this->basic_locomotion_ct = new MeCtBasicLocomotion(this);
	std::string bLocoName = std::string(name)+ "'s basic locomotion controller";
	this->basic_locomotion_ct->name(bLocoName.c_str());
	//this->basic_locomotion_ct->set_pass_through(false);

	// init reach engine
	{
	SkJoint* effector = this->_skeleton->search_joint("r_middle1");
	if (effector)
	{
	MeCtReachEngine* rengine = new MeCtReachEngine(this,this->_skeleton);
	rengine->init(MeCtReachEngine::RIGHT_ARM,effector);
	this->reachEngineMap[MeCtReachEngine::RIGHT_ARM] = rengine;		
	}	

	SkJoint* leftEffector = this->_skeleton->search_joint("l_middle1");
	if (leftEffector)
	{
	MeCtReachEngine* rengine = new MeCtReachEngine(this,this->_skeleton);
	rengine->init(MeCtReachEngine::LEFT_ARM,leftEffector);
	this->reachEngineMap[MeCtReachEngine::LEFT_ARM] = rengine;		
	}	
	}
	//if (use_locomotion) 
	{
	this->locomotion_ct =  new MeCtLocomotionClass();
	std::string locomotionname = std::string(name)+ "'s locomotion controller";
	this->locomotion_ct->name( locomotionname.c_str() );
	locomotion_ct->get_navigator()->setWordOffsetController(world_offset_writer_p);
	//locomotion_ct->set_pass_through(true);
	}

	{
	this->saccade_ct = new MeCtSaccade(this->_skeleton);
	this->saccade_ct->init(this);
	std::string saccadeCtName = std::string(name)+ "'s eye saccade controller";
	this->saccade_ct->name(saccadeCtName.c_str());
	}

	{
	this->datareceiver_ct = new MeCtDataReceiver(this->_skeleton);
	std::string datareceiverCtName = std::string(name) + "'s data receiver controller";
	this->datareceiver_ct->name(datareceiverCtName.c_str());
	}

	posture_sched_p->init(this);
	motion_sched_p->init(this);
	if( locomotion_ct != NULL )
	locomotion_ct->init(this);
	gaze_sched_p->init(this);

	#ifdef USE_REACH
	reach_sched_p->init(this);
	grab_sched_p->init(this);
	constraint_sched_p->init(this);
	#endif


	// Blink controller before head group (where visemes are controlled)
	head_sched_p->init(this);

	param_sched_p->init(this);

	ct_tree_p->name( std::string(name)+"'s ct_tree" );

	// Add Prioritized Schedule Controllers to the Controller Tree
	ct_tree_p->add_controller( posture_sched_p );
	ct_tree_p->add_controller( motion_sched_p );

	if (locomotion_ct)
	ct_tree_p->add_controller( locomotion_ct );

	if (param_animation_ct)
	ct_tree_p->add_controller(param_animation_ct);

	ct_tree_p->add_controller(basic_locomotion_ct);

	ct_tree_p->add_controller( reach_sched_p );	
	ct_tree_p->add_controller( grab_sched_p );	
	ct_tree_p->add_controller( gaze_sched_p );	
	ct_tree_p->add_controller( saccade_ct );
	ct_tree_p->add_controller( constraint_sched_p );	

	if( eyelid_reg_ct_p )
	ct_tree_p->add_controller( eyelid_reg_ct_p );

	ct_tree_p->add_controller( head_sched_p );
	ct_tree_p->add_controller( param_sched_p );


	// Face controller and softeyes control
	if( face_neutral ) {
	ct_tree_p->add_controller( face_ct );
	if  (ENABLE_EYELID_CORRECTIVE_CT) {
	std::string eyelidCtName( name );
	eyelidCtName += "'s eyelid_ct";
	eyelid_ct->name( eyelidCtName.c_str() );

	// determine the size of the character to set the 
	// appropriate scaling factor for the eyelids. 

	float dfl_hgt = 175.0f;
	float rel_scale = getHeight() / dfl_hgt;
	float lo, up;

	eyelid_ct->get_upper_lid_range( lo, up );
	eyelid_ct->set_upper_lid_range( lo * rel_scale, up * rel_scale );

	eyelid_ct->get_lower_lid_range( lo, up );
	eyelid_ct->set_lower_lid_range( lo * rel_scale, up * rel_scale );

	eyelid_ct->init(this);
	ct_tree_p->add_controller( eyelid_ct );
	}
	}

	// data receiver player
	if (use_data_receiver)
	ct_tree_p->add_controller(datareceiver_ct);

	*/

	scene_p->init( _skeleton ); 

	steeringAgent = new SteeringAgent(this);

	if (mcuCBHandle::singleton().net_bone_updates)
		bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( getName().c_str(), classType, mcuCBHandle::singleton().net_face_bones );

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{		
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterCreate( getName(), classType );
	}

	// This needs to be tested
	if( bonebusCharacter )
	{
		int index = 0;
		GeneralParamMap::const_iterator pos = param_map->begin();
		for ( ; pos != param_map->end(); pos++ )
		{

			bonebusCharacter->SetParams( pos->first.c_str(), index );
		}
	}

	// Do the sbm viseme name patch here
	std::vector<std::string> au_1_patch;
	au_1_patch.push_back("au_1_left");
	au_1_patch.push_back("au_1_right");
	std::vector<std::string> au_2_patch;
	au_2_patch.push_back("au_2_left");
	au_2_patch.push_back("au_2_right");
	std::vector<std::string> au_4_patch;
	au_4_patch.push_back("au_4_left");
	au_4_patch.push_back("au_4_right");
	std::vector<std::string> au_45_patch;
	au_45_patch.push_back("au_45_left");
	au_45_patch.push_back("au_45_right");
	viseme_name_patch.insert(make_pair("au_1", au_1_patch));
	viseme_name_patch.insert(make_pair("au_2", au_2_patch));
	viseme_name_patch.insert(make_pair("au_4", au_4_patch));
	viseme_name_patch.insert(make_pair("au_45", au_45_patch));

	// Do the bone bus viseme name patch here
	// hard coded, can be removed if these are added to bone bus and rendering side
	std::vector<std::string> closed_mouth;							// this should is an empty vector
	viseme_name_patch.insert(make_pair("_", closed_mouth));	// when receiving this two viseme, do not send signal to bone bus
	//viseme_name_patch.insert(make_pair("BMP", closed_mouth));

	std::vector<std::string> f_patch;								// "F" patch (match to "f")
	f_patch.push_back("f");
	viseme_name_patch.insert(make_pair("F",f_patch));

	std::vector<std::string> oh_patch;								// "OW" patch (match to "oh")
	oh_patch.push_back("oh");
	viseme_name_patch.insert(make_pair("OW",oh_patch));

	std::vector<std::string> all_viseme;							// "ALL" patch (match to all the viseme)
	all_viseme.push_back("Ao");
	all_viseme.push_back("D");
	all_viseme.push_back("EE");
	all_viseme.push_back("Er");
	all_viseme.push_back("f");
	all_viseme.push_back("Ih");
	all_viseme.push_back("j");
	all_viseme.push_back("KG");
	all_viseme.push_back("oh");
	all_viseme.push_back("OO");
	all_viseme.push_back("NG");
	all_viseme.push_back("R");
	all_viseme.push_back("Th");
	all_viseme.push_back("Z");
	// new added visemes: here viseme needs a better name, because following is really facial expressions
	//all_viseme.push_back("base"); // Removed by A. Shapiro 3/8/11 - was causing a conflict with joint name 'base'
	all_viseme.push_back("base_lower_face");
	all_viseme.push_back("base_upper_face");
	all_viseme.push_back("fe103_effort");
	all_viseme.push_back("fe107_surprised");
	all_viseme.push_back("fe112_happy");
	all_viseme.push_back("fe113_sad");
	all_viseme.push_back("fe119_bored");
	all_viseme.push_back("fe124_dislike");
	all_viseme.push_back("fe7_worried");
	all_viseme.push_back("fe8_scared");
	all_viseme.push_back("fe9_thinking");
	all_viseme.push_back("fe127_yawn");
	all_viseme.push_back("fe129_angry");
	viseme_name_patch.insert(make_pair("ALL",all_viseme));

#ifdef USE_REACH_TEST	
	// init left and right arm IKs for the character	
	string r_effector_name, l_effector_name;
	r_effector_name = std::string(name)+"_right_effector";
	l_effector_name = std::string(name)+"_left_effector";	
	// initialize two pawns as end effector
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	char pawnInitCmd[256];
	sprintf(pawnInitCmd,"pawn %s init",r_effector_name.c_str());
	mcu.execute(pawnInitCmd);
	sprintf(pawnInitCmd,"pawn %s init",l_effector_name.c_str());
	mcu.execute(pawnInitCmd);

	char reachCmd[256];
	sprintf(reachCmd,"bml char %s <reach target=\"%s\" reach-arm=\"right\" end=\"1000\"/>",name,r_effector_name.c_str());
	mcu.execute(reachCmd);
	sprintf(reachCmd,"bml char %s <reach target=\"%s\" reach-arm=\"left\" end=\"1000\"/>",name,l_effector_name.c_str());
	mcu.execute(reachCmd);
#endif

	//buildJointPhyObjs();


	// get the default attributes from the default controllers
	std::vector<MeController*>& defaultControllers = mcu.getDefaultControllers();
	for (size_t x = 0; x < defaultControllers.size(); x++)
	{
		MeController* controller = defaultControllers[x];
		const std::vector<AttributeVarPair>& defaultAttributes = controller->getDefaultAttributes();
		std::string groupName = controller->getName();		
		for (size_t a = 0; a < defaultAttributes.size(); a++)
		{
			SmartBody::SBAttribute* attribute = defaultAttributes[a].first;
			SmartBody::SBAttribute* attributeCopy = attribute->copy();
			this->addAttribute(attributeCopy);
		}
	}

	return( CMD_SUCCESS ); 
}


void SbmCharacter::add_face_channel( const string& name, const int wo_index ) {
	SkJoint* joint = add_bounded_float_channel( name, 0, 2, wo_index );
	joint->setJointType(SkJoint::TypeViseme);
}


SkJoint*  SbmCharacter::add_bounded_float_channel( const string& name, float lower, float upper, const int wo_index ) {

	SkJoint* joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
	joint_p->name( name );
	// Activate channel with lower limit != upper.
	joint_p->pos()->limits( SkJointPos::X, lower, upper );  // Setting upper bound to 2 allows some exageration
	return joint_p;
}

int SbmCharacter::init_skeleton() {

	if( SbmPawn::init_skeleton() != CMD_SUCCESS ) {
		return CMD_FAILURE;
	}

	// Adding viseme and FAC control channels
	//
	// Because the channels at the root are based on
	// the chanels of a skeleton, we need to use joints to add
	// these channels.  We need to reimplement the tree to
	// use raw channels or channel arrays.
	const SkJoint* wo_joint_p = get_world_offset_joint();
	if( !wo_joint_p ) {
		LOG("ERROR: SbmCharacter lacks world_offset joint after SbmPawn::init_skeleton.");
		return CMD_FAILURE;
	}
	const int wo_index = wo_joint_p->index();  // World offest joint index

	// Add channels for locomotion control...
	{
		const float max_speed = 1000000.0f;   // TODO: set max speed value to some reasonable value for the current scale

		// 3D vector for current speed and trajectory of the body
		SkJoint* loc_vector_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		loc_vector_joint_p->name( MeCtLocomotionPawn::LOCOMOTION_VELOCITY );
		// Activate positional channels
		loc_vector_joint_p->pos()->limits( 0, -max_speed, max_speed );
		loc_vector_joint_p->pos()->limits( 1, -max_speed, max_speed );
		loc_vector_joint_p->pos()->limits( 2, -max_speed, max_speed );

		//delete after test
		//SkJoint* velocity_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		//velocity_joint_p->name( SkJointName( LOCOMOTION_ROTATION ) );
		//velocity_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		//velocity_joint_p->euler()->activate();

		// 3D position for angular velocity
		SkJoint* g_angular_velocity_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* l_angular_velocity_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* l_angular_angle_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* time_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* id_joint_p = _skeleton->add_joint( SkJoint::TypeEuler, wo_index );
		g_angular_velocity_joint_p->name( MeCtLocomotionPawn::LOCOMOTION_GLOBAL_ROTATION  );
		l_angular_velocity_joint_p->name( MeCtLocomotionPawn::LOCOMOTION_LOCAL_ROTATION  );
		l_angular_angle_joint_p->name( MeCtLocomotionPawn::LOCOMOTION_LOCAL_ROTATION_ANGLE  );
		time_joint_p->name(  MeCtLocomotionPawn::LOCOMOTION_TIME  );
		id_joint_p->name( MeCtLocomotionPawn::LOCOMOTION_ID  );

		// Activate positional channels, unlimited
		g_angular_velocity_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		l_angular_velocity_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		l_angular_angle_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		time_joint_p->pos()->limits( 1, false ); // Unlimit XPos
		id_joint_p->pos()->limits( 1, false ); // Unlimit YPos

		g_angular_velocity_joint_p->euler()->activate();
		l_angular_velocity_joint_p->euler()->activate();
		l_angular_angle_joint_p->euler()->activate();
		time_joint_p->euler()->activate();
		id_joint_p->euler()->activate();
	}

	// Adding general parameter channels using a format of <char_name>_1_1, <char_name>_2_1, <char_name>_2_2, <char_name>_2_3...(for joint name)
	int Index = 0;
	for( GeneralParamMap::const_iterator pos = param_map->begin(); pos != param_map->end(); pos++ )
	{
		for( int m = 0; m < (int)pos->second->char_names.size(); m++ )
		{
			if( pos->second->char_names[m] == this->getName() )
			{
				Index ++;
				for(int i = 0; i< pos->second->size; i++)
				{
					std::stringstream joint_name;
					joint_name << this->getName() << "_" << Index << "_" << ( i + 1 );
					SkJoint* joint = add_bounded_float_channel( joint_name.str(), 0 , 1, wo_index );
					joint->setJointType(SkJoint::TypeOther);
				}
			}
		}
	}	

	// Rebuild the active channels to include new joints
	_skeleton->make_active_channels();

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.sbm_character_listener)
	{
		mcu.sbm_character_listener->OnCharacterChanged(getName());
	}

	for( int i=0; i<viseme_channel_count; i++ ) {
		viseme_history_arr[ i ] = -1.0;
	}

	return CMD_SUCCESS;
}

int SbmCharacter::setup() {

	if( SbmPawn::setup() != CMD_SUCCESS ) {
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

bool test_ct_for_pruning( MeCtScheduler2::TrackPtr track ) {
	bool prune_ok = true;

	MeController* ct = track->animation_ct();
	if( ct != NULL ) {
		MePrunePolicy* prune_policy = ct->prune_policy();
		if( prune_policy != NULL ) {
			prune_ok = prune_policy->shouldPrune( ct, track->animation_parent_ct() );

			if( LOG_CONTROLLER_TREE_PRUNING && !prune_ok )
				LOG("DEBUG: %s \"%s\" withheld from pruning by MePrunePolicy.", ct->controller_type().c_str(), ct->getName().c_str());
		}
	}

	return prune_ok;
}

// Recursive portion of SbmCharacter::prune_controller_tree
void prune_schedule( SbmCharacter*   actor,
					MeCtScheduler2* sched,
					mcuCBHandle*    mcu_p,
					double          time,
					MeCtScheduler2* posture_sched_p,
					//////  Higher priority controllers....
					MeCtGaze**      &gaze_key_cts,
					MeCtSimpleNod*  &nod_ct,
					MeController*   &motion_ct,
					MeCtPose*       &pose_ct,
					SkChannelArray  &raw_channels
					) {
						if( LOG_CONTROLLER_TREE_PRUNING ) 
							LOG("DEBUG: sbm_character.cpp prune_schedule(..): Pruning schedule \"%s\":", sched->getName().c_str());

						typedef MeCtScheduler2::TrackPtr   TrackPtr;
						typedef MeCtScheduler2::VecOfTrack VecOfTrack;

						VecOfTrack tracks = sched->tracks();  // copy of tracks
						VecOfTrack tracks_to_remove;  // don't remove during iteration

						VecOfTrack::iterator first = tracks.begin();
						VecOfTrack::iterator it    = tracks.end();

						bool hasReach = false;
						bool hasConstraint = false;
						bool hasBodyReach = false;
						bool hasHand = false;
						bool hasReachLeft = false, hasReachRight = false;
						bool hasGrabLeft = false, hasGrabRight = false;
						bool finishedBlending = false;

						while( it != first ) {
							// Decrement track iterator (remember, we started at end)
							--it;

							TrackPtr track = (*it);

							// Start with the assumption the controller is in use
							bool in_use     = true;
							bool flat_blend_curve = true;  // No blend controller means the blend is always 1, thus flat

							MeController* anim_source = track->animation_ct();
							if( anim_source ) {

#if 0 // DYNAMIC_CASTS_ACTUALLY_WORK?
								// These don't seem to work, even with Runtime Type Inspection enabled
								MeCtBlend*         blend_ct = dynamic_cast<MeCtBlend*>( track->blending_ct() );
								MeCtTimeShiftWarp* timing_ct = dynamic_cast<MeCtTimeShiftWarp*>( track->timing_ct() );
#else // Trying using manual runtime typing
								MeCtUnary* unary_blend_ct = track->blending_ct();
								MeCtBlend* blend_ct = NULL;
								if( unary_blend_ct && unary_blend_ct->controller_type() == MeCtBlend::CONTROLLER_TYPE )
									blend_ct = (MeCtBlend*)unary_blend_ct;

								MeCtUnary*         unary_timing_ct = track->timing_ct();
								MeCtTimeShiftWarp* timing_ct = NULL;
								if( unary_timing_ct && unary_timing_ct->controller_type() == MeCtTimeShiftWarp::CONTROLLER_TYPE )
									timing_ct = (MeCtTimeShiftWarp*)unary_timing_ct;
#endif

								if( blend_ct ) {
									// Determine if the blend is still active,
									// or will ever be in the future

									srLinearCurve& blend_curve = blend_ct->get_curve();
									int n = blend_curve.get_num_keys();
									if( n > 0 )	{

										double timeBegin = blend_curve.get_head_param();
										double valueBegin = blend_curve.get_head_value();
										// motions that haven't been started yet shouldn't be pruned
										if (timeBegin > time && valueBegin == 0.0)
										{ 
											// skip this track since it hasn't started yet
											continue;
										}

										double t = blend_curve.get_tail_param();
										double v = blend_curve.get_tail_value();


										if( LOG_CONTROLLER_TREE_PRUNING )
											LOG("\tblend_Ct \"%s\": blend curve last knot: t = %f v = %f", blend_ct->getName().c_str(), t, v );
										if( t <= time )
										{
											flat_blend_curve = true;
											if( v == 0.0 ) {
												in_use = false;
											}
										} 
										else 
										{
											//						LOG( "sbm_character.cpp prune_schedule(): ERR: this pruning path not implemented" );

											v = blend_curve.evaluate( time );
											if( v == 0.0 )	{
												t = blend_curve.get_next_nonzero_value( time );
												if( t < time )	{
													flat_blend_curve = true;
													in_use = false;
												}
											}
											if (v < 1.0 && sched == posture_sched_p)
											{  // special case handling for postures
												finishedBlending = false;
											}

											if( !flat_blend_curve )	{
												if( blend_curve.get_next_nonzero_slope( time ) < 0.0 )	{
													flat_blend_curve = true;
												}
											}
#if 0
											// NOTE: UNUSED CODE PATH...
											// Has knots beyond current time
											static const double END_OF_TIME = MeCtScheduler2::MAX_TRACK_DURATION * 0.999;  // Edge of acceptable precision

											// Last knots are far in the future, beyond reasonable values of time
											MeSpline1D::Knot* prev_knot = knot->get_prev();
											while( prev_knot!=NULL && prev_knot->get_x()>END_OF_TIME ) {
												knot = prev_knot;
												prev_knot = knot->get_prev();
											}

											if( knot->get_x()>END_OF_TIME || knot->get_left_y() == knot->get_y() ) {
												// This knot is flat, time to check others...
												flat_blend_curve = true;

												while( flat_blend_curve && prev_knot!=NULL && prev_knot->get_x() > time ) {
													flat_blend_curve = prev_knot->get_y()==y && prev_knot->get_left_y()==y;
													prev_knot = prev_knot->get_prev();
												}
												if( flat_blend_curve && prev_knot!=NULL ) {
													// prev_knot is knot just before time
													flat_blend_curve = prev_knot->get_y()==y;
												}
											}

											// Only consider the most recent end-of-time knot and its left value
											y = knot->get_left_y();
											in_use = flat_blend_curve ? ( y <=0 ) : true;
#endif
										}
									} 
									else	{
										if( LOG_PRUNE_TRACK_WITHOUT_BLEND_SPLIE_KNOTS ) {
											std::stringstream strstr;
											strstr << "DEBUG: prune_schedule(..): sched \""<<sched->getName()<<"\", anim_source \""<<anim_source->getName()<<"\": blend_ct without spline knots.";
											LOG(strstr.str().c_str());
											blend_ct->print_state(1);  // Prints controller type, name, and blend curve
										}
										in_use = false; // A spline with no knots evaluates to 0
									}
								}

								const std::string& anim_ct_type = anim_source->controller_type();
								if( LOG_CONTROLLER_TREE_PRUNING )
								{
									std::stringstream strstr;
									strstr << '\t' << anim_ct_type << " \"" << anim_source->getName() << "\": in_use = "<<in_use<<", flat_blend_curve = "<<flat_blend_curve<<endl;
									LOG(strstr.str().c_str());
								}
								if( !in_use ) {
									if( LOG_CONTROLLER_TREE_PRUNING )
										LOG("\t- Pruned (not in use)!!");
								} else if( flat_blend_curve ) {  // Ignore tracks with future blend activity or are already not in use
									// Determine if the animation will be occluded by
									// (previously visited) higher priority controllers
									if( anim_ct_type == MeCtScheduler2::type_name ) {

										double time_offset = time;
										if( timing_ct ) {
											time_offset = timing_ct->get_curve().evaluate( time );
										}

										MeCtScheduler2* sched_ct = (MeCtScheduler2*)anim_source;

										if( sched_ct==posture_sched_p ) {
											//ostringstream oss;
											//oss << sched_ct->print_state( "1", oss );

											//  Don't let higher priority controller occlude ALL pose controllers
											//  by pretending there wasn't a higher priority controller
											MeCtGaze**     gaze_key2_cts = new MeCtGaze*[ MeCtGaze::NUM_GAZE_KEYS ];
											for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
												gaze_key2_cts[key] = NULL;

											MeCtSimpleNod* nod2_ct = NULL;
											MeController*  motion2_ct = NULL;
											MeCtPose*      pose2_ct = NULL;
											prune_schedule( actor, sched_ct, mcu_p, time_offset, posture_sched_p, gaze_key2_cts, nod2_ct, motion2_ct, pose2_ct, raw_channels );

											delete[] gaze_key2_cts;
											//if( sched_ct->count_children()==0 ) {
											//	cerr<< "ERROR!!  Invalid posture track: "<<oss.str()<<endl;
											//}

											in_use = true;
										} else {
											prune_schedule( actor, sched_ct, mcu_p, time_offset, posture_sched_p, gaze_key_cts, nod_ct, motion_ct, pose_ct, raw_channels );
											in_use = sched_ct->count_children()>0;
										}
									} else if( anim_ct_type == MeCtSimpleNod::_type_name )
									{
										if(    nod_ct
											|| (    (gaze_key_cts[MeCtGaze::GAZE_KEY_HEAD]!=NULL)
											&& (gaze_key_cts[MeCtGaze::GAZE_KEY_NECK]!=NULL) ) )
										{
											in_use = false;
										} else {
											nod_ct = (MeCtSimpleNod*)anim_source;
										}
									} else if( anim_ct_type == MeCtGaze::CONTROLLER_TYPE ) {
										if( motion_ct || pose_ct ) {
											in_use = false;
										} else {
											MeCtGaze* gaze_ct = (MeCtGaze*)anim_source;

											bool is_occluded = true;
											for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key ) {
												if( gaze_ct->get_blend( key ) > 0 ) {  // gaze_ct has output for this key
													if( gaze_key_cts[ key ]==NULL ) {
														is_occluded = false;
														if( gaze_ct->is_full_blend( key ) )
															// Occludes lower priority controllers
															gaze_key_cts[ key ] = gaze_ct;
													}
												}
											}

											// If still ocluded (after testing each key) then it is not in use
											in_use = !is_occluded;

											// don't remove gazes that have handles
											if (!in_use)
											{
												if (gaze_ct->handle() != "")
													in_use = true;
											}
										}
									} 
#if 0
									else if (dynamic_cast<MeCtReach*>(anim_source)) {
										MeCtReach* ct_reach = dynamic_cast<MeCtReach*>(anim_source);

										if (ct_reach->getReachArm() == MeCtReach::REACH_LEFT_ARM)
										{
											if (hasReachLeft)
											{
												in_use = false;
											}
											else
											{
												hasReachLeft = true;
											}
										}

										if (ct_reach->getReachArm() == MeCtReach::REACH_RIGHT_ARM)
										{
											if (hasReachRight)
											{
												in_use = false;
											}
											else
											{
												hasReachRight = true;
											}
										}
									} 
#endif
									else if (dynamic_cast<MeCtConstraint*>(anim_source)) {
										MeCtConstraint* ct_constraint = dynamic_cast<MeCtConstraint*>(anim_source);
										if (hasConstraint)
										{
											in_use = false;
										}
										else
										{
											hasConstraint = true;
										}
									}
									else if (dynamic_cast<MeCtHand*>(anim_source)) {
										MeCtHand* ct_hand = dynamic_cast<MeCtHand*>(anim_source);

										if (ct_hand->getGrabType() == MeCtReachEngine::RIGHT_ARM)
										{
											if (hasGrabRight)
											{
												in_use = false;
											}
											else
											{
												hasGrabRight = true;
											}
										}

										if (ct_hand->getGrabType() == MeCtReachEngine::LEFT_ARM)
										{
											if (hasGrabLeft)
											{
												in_use = false;
											}
											else
											{
												hasGrabLeft = true;
											}
										}
										/*
										if (hasHand)
										{
											in_use = false;
										}
										else
										{
											hasHand = true;
										}
										*/
									}
									else if (dynamic_cast<MeCtExampleBodyReach*>(anim_source)) {
										MeCtExampleBodyReach* ct_bodyReach = dynamic_cast<MeCtExampleBodyReach*>(anim_source);
										if (hasBodyReach)
										{
											//LOG("Prune Reach Controller!\n");
											in_use = false;
										}
										else
										{
											hasBodyReach = true;
										}
									}
									else if( anim_ct_type == MeCtMotion::type_name || anim_ct_type == MeCtQuickDraw::type_name ) {
										if( motion_ct || pose_ct )
										{
											// if a motion is already present, other animations should be pruned
											// however, if the other tracks consist of animations that are not yet scheduled to be played,
											// then don't prune them
											//
											// Check to see if the motion is scheduled for the future
											srLinearCurve& blend_curve = blend_ct->get_curve();
											int n = blend_curve.get_num_keys();
											if( n > 0 )
											{
												double timeBegin = blend_curve.get_head_param();
												double valueBegin = blend_curve.get_head_value();
												// motions that haven't been started yet shouldn't be pruned
												if (timeBegin > time && valueBegin == 0.0)
												{ 
													in_use = true;
												}
												else
												{
													motion_ct = anim_source;
												}
											}
											else
											{
												motion_ct = anim_source;
											}
										} else {
											motion_ct = anim_source;
										}
									} 
									else 
										if( anim_ct_type == MeCtPose::type_name ) {
											if( pose_ct ) {
												in_use = false;
											} else {
												pose_ct = (MeCtPose*)anim_source;
											}
										} 
										else 
											if( ( anim_ct_type == MeCtChannelWriter::TYPE )||( anim_ct_type == MeCtCurveWriter::TYPE ) ) {
#if 1
												const SkChannelArray& ct_channels = anim_source->controller_channels();
												//					vector<int> new_channels;  // list of indices to channels in use
												bool foundChannelMatch = false;
												const int total_channels = ct_channels.size();
												for( int i=0; i<total_channels; ++i ) {
													int index = raw_channels.search( ct_channels.name(i), ct_channels.type(i) );
													if( index != -1 ) {
														foundChannelMatch = true;
													} else {
														raw_channels.add(ct_channels.name(i), ct_channels.type(i));
													}
												}

												if(foundChannelMatch && anim_ct_type == MeCtChannelWriter::TYPE)
												{
													in_use = false;
												}
#endif
											} 
											else if(anim_ct_type == MeCtNavigationCircle::TYPE)
											{
											}
											else if(anim_ct_type == MeCtInterpolator::CONTROLLER_TYPE)
											{
											}
											else {
												//  TODO: Throttle warnings....
												LOG("WARNING: Cannot prune unknown controller type \"%s\"", anim_source->controller_type().c_str());
											}
											if( LOG_CONTROLLER_TREE_PRUNING )
												if (in_use)
													LOG("\t- Not Pruned (primary ct of type).");
												else
													LOG("\t- Pruned (occluded)!!");
								} else {
									if( LOG_CONTROLLER_TREE_PRUNING )
										LOG("\t- Not Pruned (future activity).");
								}
							} else {
								// No animation source
								in_use = false;

								if( LOG_CONTROLLER_TREE_PRUNING )
									LOG("\t- Pruned (no anim ct)!!");
							}

							if (sched == posture_sched_p && !finishedBlending)
							{ // make sure that we don't remove old postures before the new postures have finished blending in
								in_use = true;
							}

							if( !in_use && test_ct_for_pruning( track ) ) {

								// insert at front, because we are iterating end->begin
								// and we prefer the final list order matches order within schedule
								tracks_to_remove.insert( tracks_to_remove.begin(), track );
							}
						}

						if( !tracks_to_remove.empty() ) {
#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
							vec_tracks::iterator it = tracks_to_remove.begin();
							vec_tracks::iterator end = tracks_to_remove.begin();
							for( ; it != end; ++it ) {
								MeController* anim_ct = (*it)->animation_ct();
								if( anim_ct != NULL ) {
									// Inform character about the to-be-removed controller
									actor->exec_controller_cleanup( anim_ct, mcu_p );
								}
							}
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

							sched->remove_tracks( tracks_to_remove );
						}
}

/**
*  Prunes the controller tree by making wild assumptions about
*  what types of controllers will overwrite the results of
*  of other types of controllers. Fails to recognize partial
*  body motions and partial spine gazes.
*/
int SbmCharacter::prune_controller_tree( mcuCBHandle* mcu_p ) {

	double time = mcu_p->time;  // current time

	if( LOG_PRUNE_CMD_TIME || LOG_CONTROLLER_TREE_PRUNING )
	{
		std::stringstream strstr;
		strstr << "SbmCharacter \""<<getName()<<"\" prune_controller_tree(..) @ time "<<time<<endl;
		LOG(strstr.str().c_str());
	}

	// Pointers to the most active controllers of each type.
	MeCtGaze**     gaze_key_cts = new MeCtGaze*[ MeCtGaze::NUM_GAZE_KEYS ];
	for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
		gaze_key_cts[key] = NULL;
	MeCtSimpleNod* nod_ct    = NULL;
	MeController*  motion_ct = NULL;  // also covers quickdraw
	MeCtPose*      pose_ct   = NULL;
	SkChannelArray raw_channels;

	// Traverse the controller tree from highest priority down, most recent to earliest
	prune_schedule( this, head_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
	prune_schedule( this, reach_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
	prune_schedule( this, grab_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
	prune_schedule( this, gaze_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
	prune_schedule( this, constraint_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
	prune_schedule( this, motion_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );

	// For the posture track, ignore prior controllers, as they should never be used to mark a posture as unused
	for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
		gaze_key_cts[key] = NULL;
	nod_ct    = NULL;
	motion_ct = NULL;  // also covers quickdraw
	pose_ct   = NULL;
	raw_channels = SkChannelArray::empty_channel_array();
	prune_schedule( this, posture_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );

	if( LOG_CONTROLLER_TREE_PRUNING ) {
		LOG("");
		print_controller_schedules();
	}

	delete[] gaze_key_cts;

	return CMD_SUCCESS;
}


void SbmCharacter::remove_from_scene() {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	string vrProcEnd_msg = "vrProcEnd sbm ";
	vrProcEnd_msg += getName();
	mcu.vhmsg_send( vrProcEnd_msg.c_str() );

	mcu.removeCharacter(getName());

	SbmPawn::remove_from_scene();
}

int SbmCharacter::set_speech_impl( SmartBody::SpeechInterface *speech_impl ) {
	this->speech_impl = speech_impl;

	return CMD_SUCCESS;
}

int SbmCharacter::set_speech_impl_backup( SmartBody::SpeechInterface *speech_impl ) {
	this->speech_impl_backup = speech_impl;

	return CMD_SUCCESS;
}

//returns speech implementation if set or NULL if not
SmartBody::SpeechInterface* SbmCharacter::get_speech_impl() const {
	return speech_impl;
}

//returns speech implementation if set or NULL if not
SmartBody::SpeechInterface* SbmCharacter::get_speech_impl_backup() const {
	return speech_impl_backup;
}


int SbmCharacter::set_voice_code( std::string& voice_code ) //allows you to set the voice-- made different from the init because of non Rhetoric might not have voice codes
{
	//TODO: LOOK AND SEE IF THIS VOICE EXISTS AND IF IT DOESN'T PRINT ERROR MESSAGE AND RETURN FAILURE
	this->voice_code = voice_code; //sets voice 
	return (CMD_SUCCESS);
}

int SbmCharacter::set_voice_code_backup( std::string& voice_code ) //allows you to set the voice-- made different from the init because of non Rhetoric might not have voice codes
{
	//TODO: LOOK AND SEE IF THIS VOICE EXISTS AND IF IT DOESN'T PRINT ERROR MESSAGE AND RETURN FAILURE
	this->voice_code_backup = voice_code; //sets voice 
	return (CMD_SUCCESS);
}


const std::string& SbmCharacter::get_voice_code() const
{
	return voice_code; //if voice isn't NULL-- no error message; just returns the string
}

const std::string& SbmCharacter::get_voice_code_backup() const
{
	return voice_code_backup; //if voice isn't NULL-- no error message; just returns the string
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void SbmCharacter::schedule_viseme_curve(
	const char* viseme, 
	double start_time, 
	float* curve_info, 
	int num_keys, 
	int num_key_params, 
	float ramp_in, 
	float ramp_out 
	) {

		std::vector<std::string> visemeNames;
		std::map<std::string, std::vector<std::string> >::iterator iter;

		iter = viseme_name_patch.find(viseme);
		if (iter != viseme_name_patch.end())
		{
			for (size_t nCount = 0; nCount < iter->second.size(); nCount++)
				visemeNames.push_back(iter->second[nCount]);
		}
		else
			visemeNames.push_back(viseme);

		SmartBody::SBFaceDefinition* faceDefinition = getFaceDefinition();

		for( size_t nCount = 0; nCount < visemeNames.size(); nCount++ )
		{
			if( num_keys > 0 )
			{
				float visemeWeight = 1.0f;
				float timeDelay = this->get_viseme_time_delay();

				ostringstream ct_name;
				ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";
				if (faceDefinition)
				{
					if (faceDefinition->hasViseme(visemeNames[nCount]))
					{
						visemeWeight = faceDefinition->getVisemeWeight(visemeNames[nCount]);
					}
				}


				SkChannelArray channels;
				channels.add( visemeNames[nCount], SkChannel::XPos );

				MeCtCurveWriter* ct_p = new MeCtCurveWriter();
				ct_p->setName( ct_name.str().c_str() );
				ct_p->init(this, channels ); // CROP, CROP, true

				if (num_keys <= 2)
				{
					for (int i = 0; i < num_keys; i++)	{
						float t = curve_info[ i * num_key_params + 0 ];
						float w = curve_info[ i * num_key_params + 1 ] * visemeWeight;
						ct_p->insert_key( t, w );
					}
				}
				else
				{
					srSplineCurve spline( srSplineCurve::INSERT_NODES );
					//				srSplineCurve spline( srSplineCurve::INSERT_KEYS );
					spline.set_extensions( srSplineCurve::EXTEND_DECEL, srSplineCurve::EXTEND_DECEL );
					spline.set_algorithm( srSplineCurve::ALG_HALTING );

					for (int i = 0; i < num_keys; i++)	{

						float t = curve_info[ i * num_key_params + 0 ];
						float w = curve_info[ i * num_key_params + 1 ] * visemeWeight;

						//					if (i == 0) spline.insert(t - .001, w);
						spline.insert( t, w );
						//					if (i == num_keys - 1) spline.insert(t + .001, w);
					}
					spline.apply_extensions();

#define LINEAR_SPLINE_SEGS_PER_SEC 30.0
#if 1
					ct_p->insert_spline( spline, LINEAR_SPLINE_SEGS_PER_SEC );
#else
					double fr, to;
					spline.query_span( &fr, &to );
					int num_segs = (int)( ( to - fr ) * LINEAR_SPLINE_SEGS_PER_SEC );

					// MeCtCurveWriter: ct_p must handle the build internally...
					// ct_p->sample_spline( int curve_index, spline, num_segs, true );
					srCurveBuilder builder;
					srLinearCurve linear;
					builder.get_spline_curve( &linear, spline, num_segs, true );

					linear.query_reset();
					double t, v;
					while( linear.query_next( &t, &v, true ) )	{
						ct_p->insert_key( t, v );
					}
#endif
				}
				double ct_dur = ct_p->controller_duration();
				double tin = start_time + timeDelay;
				double tout = tin + ct_dur;
				head_sched_p->schedule( ct_p, tin, tout, ramp_in, ramp_out );
			}
		}
}

void SbmCharacter::schedule_viseme_trapezoid( 
	const char* viseme,
	double start_time,
	float weight,
	float duration,
	float ramp_in, 
	float ramp_out 
	)	{

		static float curve_info[ 4 ] = {
			0.0f, 0.0f, 
			0.0f, 0.0f
		};

		SmartBody::SBFaceDefinition* faceDefinition = getFaceDefinition();
		float visemeWeight = 1.0f;
		if (faceDefinition)
		{
			if (faceDefinition->hasViseme(viseme))
			{
				visemeWeight = faceDefinition->getVisemeWeight(viseme);
			}
		}

		// make sure that we never have a zero duration
		if (duration <= 0.0)
			duration = .05f;
		curve_info[ 1 ] = weight * visemeWeight;
		curve_info[ 2 ] = duration;
		curve_info[ 3 ] = weight * visemeWeight;
		schedule_viseme_curve( viseme, start_time, curve_info, 2, 2, ramp_in, ramp_out );
}

void SbmCharacter::schedule_viseme_blend_curve(
	const char* viseme, 
	double start_time, 
	float weight,
	float* curve_info, 
	int num_keys, 
	int num_key_params
	) {

		std::vector<std::string> visemeNames;
		std::map<std::string, std::vector<std::string> >::iterator iter;

		iter = viseme_name_patch.find(viseme);
		if (iter != viseme_name_patch.end())
		{
			for (size_t nCount = 0; nCount < iter->second.size(); nCount++)
				visemeNames.push_back(iter->second[nCount]);
		}
		else
			visemeNames.push_back(viseme);

		SmartBody::SBFaceDefinition* faceDefinition = getFaceDefinition();
		float visemeWeight = 1.0f;

		for( size_t nCount = 0; nCount < visemeNames.size(); nCount++ )
		{
			if( num_keys > 0 )
			{
				float timeDelay = this->get_viseme_time_delay();

				ostringstream ct_name;
				ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

				if (faceDefinition)
				{
					if (faceDefinition->hasViseme(visemeNames[nCount]))
					{
						visemeWeight = faceDefinition->getVisemeWeight(visemeNames[nCount]);
					}
				}
				SkChannelArray channels;
				channels.add( visemeNames[nCount], SkChannel::XPos );

				MeCtChannelWriter* ct_p = new MeCtChannelWriter();
				ct_p->setName( ct_name.str().c_str() );
				ct_p->init(this, channels, true );
				SrBuffer<float> value;
				value.size( 1 );
				value[ 0 ] = weight * visemeWeight;
				ct_p->set_data(value);

				head_sched_p->schedule( ct_p, start_time + timeDelay, curve_info, num_keys, num_key_params );
			}
		}
}

void SbmCharacter::schedule_viseme_blend_ramp( 
	const char* viseme,
	double start_time,
	float weight,
	float rampin_duration
	)	{

		static float curve_info[ 4 ] = {
			0.0f, 0.0f, 
			0.0f, 1.0f
		};
		SmartBody::SBFaceDefinition* faceDefinition = getFaceDefinition();
		float visemeWeight = 1.0f;
		if (faceDefinition)
		{
			if (faceDefinition->hasViseme(viseme))
			{
				visemeWeight = faceDefinition->getVisemeWeight(viseme);
			}
		}
		curve_info[ 2 ] = rampin_duration;
		schedule_viseme_blend_curve( viseme, start_time, weight * visemeWeight, curve_info, 2, 2 );
}

void SbmCharacter::forward_visemes( double curTime )
{
	SBMCharacterListener *listener_p = mcuCBHandle::singleton().sbm_character_listener;

	if( bonebusCharacter || listener_p )
	{
		SkChannelArray& channels = _skeleton->channels();
		MeFrameData& frameData = ct_tree_p->getLastFrame();

		int i = 0;
		for( int c = viseme_channel_start_pos; c < viseme_channel_end_pos; c++, i++ )
		{
			SkChannel& chan = channels[c];
			int buffIndex = ct_tree_p->toBufferIndex(c);

			if( buffIndex > -1 )	
			{
				float value = frameData.buffer()[ buffIndex ];
				if( value != viseme_history_arr[ i ] )	{

					if( bonebusCharacter )
					{
						bonebusCharacter->SetViseme( channels.name(c).c_str(), value, 0 );
					}
					if( listener_p )
					{
						listener_p->OnViseme( getName(), channels.name(c), value, 0 );
					}
					viseme_history_arr[ i ] = value;
				}
			}
		}
	}
}

void SbmCharacter::forward_parameters( double curTime )
{
	SBMCharacterListener *listener_p = mcuCBHandle::singleton().sbm_character_listener;

	if( listener_p )
	{
		const std::vector<SkJoint*>& joints = _skeleton->joints();
		MeFrameData& frameData = ct_tree_p->getLastFrame();

		for (size_t j = 0; j < joints.size(); j++)
		{
			SkJoint* joint = joints[j];
			if (joint->getJointType() != SkJoint::TypeOther)
				continue;
			listener_p->OnChannel(getName(), joint->name(), joint->pos()->value(0)); 
		}
	}
}

///////////////////////////////////////////////////////////////////////////

void SbmCharacter::inspect_skeleton( SkJoint* joint_p, int depth )	{
	int i, j, n;

	if( joint_p )	{
		const char *name = joint_p->name().c_str();
		for( j=0; j<depth; j++ ) { LOG( " " ); }
		LOG( "%s\n", name );
		n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton( joint_p->child( i ), depth + 1 );
		}
	}
}

void SbmCharacter::inspect_skeleton_local_transform( SkJoint* joint_p, int depth )	{

	if( joint_p )	{
		const char *name = joint_p->name().c_str();
		gwiz::matrix_t M;
		int i, j;

		SrMat sr_M = joint_p->lmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		gwiz::vector_t pos = M.translation( gwiz::COMP_M_TR );
		gwiz::euler_t rot = M.euler( gwiz::COMP_M_TR );

		for( j=0; j<depth; j++ ) { LOG( " " ); }
		LOG( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
			);

		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_local_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

void SbmCharacter::inspect_skeleton_world_transform( SkJoint* joint_p, int depth )	{

	if( joint_p )	{
		const char *name = joint_p->name().c_str();
		gwiz::matrix_t M;
		int i, j;

		joint_p->update_gmat_up();
		SrMat sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		gwiz::vector_t pos = M.translation( gwiz::COMP_M_TR );
		gwiz::euler_t rot = M.euler( gwiz::COMP_M_TR );

		for( j=0; j<depth; j++ ) { LOG( " " ); }
		LOG( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
			);

		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_world_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

int SbmCharacter::print_controller_schedules() {
	//  Command: print character <character id> schedule
	//  Print out the current state of the character's schedule
	LOG("Character %s's schedule:", getName().c_str());
	LOG("POSTURE Schedule:");
	posture_sched_p->print_state( 0 );
	LOG("MOTION Schedule");
	motion_sched_p->print_state( 0 );
	LOG("GAZE Schedule:");
	gaze_sched_p->print_state( 0 );
	LOG("HEAD Schedule:");
	head_sched_p->print_state( 0 );
	LOG("REACH Schedule:");
	reach_sched_p->print_state( 0 );
	LOG("Grab Schedule:");
	grab_sched_p->print_state( 0 );
	// Print Face?

	return CMD_SUCCESS;
}

bool SbmCharacter::is_face_controller_enabled() {
	return (face_ct!=NULL && face_ct->context()!=NULL);
}

// HACK to initiate reholster on all QuickDraw controllers
int SbmCharacter::reholster_quickdraw( mcuCBHandle *mcu_p ) {
	const double now = mcu_p->time;

	double max_blend_dur = -1;

	// Local convience typedefs
	typedef MeCtScheduler2::TrackPtr   TrackPtr;
	typedef MeCtScheduler2::VecOfTrack VecOfTrack;

	VecOfTrack tracks = motion_sched_p->tracks();

	VecOfTrack::iterator it = tracks.begin();
	VecOfTrack::iterator end = tracks.end();

	bool found_quickdraw = false;

	while( it != end ) {
		TrackPtr track = (*it);

		MeController* anim_ct = track->animation_ct();
		if( anim_ct ) {
			string anim_ct_type( anim_ct->controller_type() );
			if( anim_ct_type==MeCtQuickDraw::type_name ) {
				found_quickdraw = true;

				MeCtQuickDraw* qdraw_ct = (MeCtQuickDraw*)anim_ct;

				// Initiate reholster
				qdraw_ct->set_reholster();

				// Attempt to schedule blend out
				MeCtUnary* blending_ct = track->blending_ct();
				if(    blending_ct
					&& blending_ct->controller_type() == MeCtBlend::CONTROLLER_TYPE)
				{
					// TODO: account for time scaling of motion_duration
					double blend_out_start = now + qdraw_ct->get_holster_duration();
					float  blend_out_dur   = qdraw_ct->outdt();
					double blend_out_end   = blend_out_start + blend_out_dur;
					//					float  blend_spline_tanget = -1/blend_out_dur;

					MeCtBlend* blend_ct = (MeCtBlend*)blending_ct;
					srLinearCurve& blend_curve = blend_ct->get_curve();
					blend_curve.clear_after( blend_out_start );
					blend_curve.insert( blend_out_start, 1.0 );
					blend_curve.insert( blend_out_end, 0.0 );

					if( blend_out_dur > max_blend_dur )
						max_blend_dur = blend_out_end;
				}
			}
		}

		++it;
	}

	if( !found_quickdraw ) {
		std::stringstream strstr;
		strstr << "WARNING: Character \""<<getName()<<"\" reholster(): No quickdraw controller found.";
		LOG(strstr.str().c_str());
	}

	////  Won't compile, and I'm tired:
	////  Error	1	error C2296: '<<' : illegal, left operand has type 'std::ostringstream (__cdecl *)(void)'
	//	if( max_blend_dur >= 0 ) {
	//		// schedule prune
	//		max_blend_dur += 1;  // small buffer period
	//
	//		ostringstream out();
	//		out << "char " << name << " prune";
	//		mcu_p->execute_later( out.str().c_str(), max_blend_dur );
	//	}

	return CMD_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////

int SbmCharacter::parse_character_command( std::string cmd, srArgBuffer& args, mcuCBHandle *mcu_p, bool all_characters ) {

	if (cmd == "mesh")
	{
		if (!this->dMesh_p)
		{
			LOG("Character %s has no dynamic mesh, cannot perform mesh operations.", this->getName().c_str());
			return CMD_FAILURE;
		}
		char* meshdir = args.read_token();
		if (!meshdir)
		{
			LOG("Usage: mesh <meshdirectory> |-prefix|");
			return CMD_FAILURE;
		}

		std::string prefix = "";
		int numRemaining  = args.calc_num_tokens();
		if (numRemaining > 0)
		{
			std::string prefixCommand = args.read_token();
			if (prefixCommand == "-prefix")
			{
				prefix = args.read_token();
			}
		}
		// remove any existing mesh for this character
		// how do we do this???
		// ...
		// ... FIXME!
		// ...
		mcu_p->mesh_paths.reset();
		std::string path = "";
		while ((path = mcu_p->mesh_paths.next_path()) != "")
		{
			boost::filesystem2::path curpath( path );
			if (!boost::filesystem2::is_directory(curpath))
				continue;
			curpath /= std::string(meshdir);			
			bool isDir = boost::filesystem2::is_directory(curpath);  
			if (!isDir)
			{
				LOG("%s is not a directory.", curpath.directory_string().c_str());
				return CMD_FAILURE;
			}
			else
			{
				// set the mesh directory
				std::string meshFullDir = curpath.string();
				//this->setStringAttribute("mesh",meshFullDir);
			}

			std::vector<std::string> xmlFileList;
			std::vector<std::string> objFileList;
			boost::filesystem2::directory_iterator end;
			for (boost::filesystem2::directory_iterator iter(curpath); iter != end; iter++)
			{
				if (boost::filesystem2::is_regular(*iter))
				{
					std::string fileName = (*iter).string();
					if (fileName.size() < 4)
						continue;
					std::string ext = fileName.substr(fileName.size() - 4);
					if (ext == ".xml" || ext == ".XML" ||
						ext == ".dae" || ext == ".DAE")
					{
						xmlFileList.push_back(fileName);
					}
					if (ext == ".obj" || ext == ".OBJ")
					{
						objFileList.push_back(fileName);
					}
				}
			}

			bool hasXmlMesh = true;
			if (xmlFileList.size() == 0)
			{
				LOG("No XML file is found inside %s", meshdir);
			}
			else
			{
				
				std::string fileName = xmlFileList[0];
				std::stringstream strstr;
				
				strstr << "char " << getName() << " smoothbindweight " << fileName;
				if (prefix != "")
					strstr << " -prefix " << prefix;
				int successWeight = mcu_p->execute((char*) strstr.str().c_str());
				if (successWeight == CMD_SUCCESS)
				{
					LOG("Successfully read skin weights from file %s", fileName.c_str());
				}
				
				// this could also be a file containing a mesh, so run that command as well
				std::stringstream strstr2;
				strstr2 << "char " << getName() << " smoothbindmesh " << fileName;
				int successMesh = mcu_p->execute((char*) strstr2.str().c_str());
				if (successMesh == CMD_SUCCESS)
				{
					LOG("Successfully read mesh from file %s", fileName.c_str());
				}
				else
				{
					hasXmlMesh = false;
					LOG("Could not read skin mesh from %s, searching for OBJ files now", fileName.c_str());
				}
				if (successMesh == CMD_FAILURE && successWeight == CMD_FAILURE)
				{
					LOG("Could not read skin weights or mesh from %s", fileName.c_str());
				}				
			}
			if (!hasXmlMesh)
			{
				for (size_t i = 0; i < objFileList.size(); i++)
				{
					std::string fileName = objFileList[i];
					std::stringstream strstr;
					strstr << "char " << getName() << " smoothbindmesh " << fileName;
					int success = mcu_p->execute((char*) strstr.str().c_str());
					if (success != CMD_SUCCESS)
					{
						LOG("Problem running: %s", strstr.str().c_str());
					}
					else
					{
						LOG("Successfully read mesh from file %s", fileName.c_str());
					}
				}
			}
		}
		return CMD_SUCCESS;

	}
	else if (cmd == "meshstatus")
	{
		if (!this->dMesh_p)
		{
			LOG("Character %s has no dynamic mesh, cannot perform mesh operations.", this->getName().c_str());
			return CMD_FAILURE;
		}
		LOG("Number of skinned meshes: %d", this->dMesh_p->skinWeights.size());
		for (size_t i = 0; i < this->dMesh_p->skinWeights.size(); i++)
		{
			SkinWeight* skinWeight = this->dMesh_p->skinWeights[i];
			LOG("%s", skinWeight->sourceMesh.c_str());
		}

		for (size_t m = 0; m < this->dMesh_p->dMeshStatic_p.size(); m++)
		{
			SrSnModel* srsnModel = this->dMesh_p->dMeshStatic_p[m];
			SrModel& model = srsnModel->shape();
			LOG("Name: %s  Verts: %d  Faces: %d  Materials: %d", (const char*) model.name, model.V.size(), model.F.size(), model.M.size());
		}

		return CMD_SUCCESS;
	}
	else if( cmd == "smoothbindmesh" )
	{
		if (!this->dMesh_p)
		{
			LOG("Character %s has no dynamic mesh, cannot perform mesh operations.", this->getName().c_str());
			return CMD_FAILURE;
		}
		char* obj_file = args.read_token();
		char* option = args.read_token();
		return mcu_character_load_mesh( getName().c_str(), obj_file, mcu_p, option );
	} 
	else 
		if( cmd == "smoothbindweight" ) {
			if (!this->dMesh_p)
			{
				LOG("Character %s has no dynamic mesh, cannot perform mesh operations.", this->getName().c_str());
				return CMD_FAILURE;
			}
			char* skin_file = args.read_token();
			char* option = args.read_token();
			char* prefixName = NULL;
			float scaleFactor = 1.f;

			while (strcmp(option,EMPTY_STRING) != 0)
			{
				if (option && strcmp(option,"-prefix") == 0)
				{
					prefixName = args.read_token();
				}
				else if (strcmp(option,"-m") == 0)
				{
					scaleFactor = 0.01f;
				}
				else if (strcmp(option,"-scale") == 0)
				{
					scaleFactor =  args.read_float();
				}
				option = args.read_token();
			}

			//printf("prefix name = %s\n",prefixName);
			return mcu_character_load_skinweights( getName().c_str(), skin_file, mcu_p, scaleFactor,prefixName);
		} 
		else 
			if( cmd == "ctrl" ) {
				return mcu_character_ctrl_cmd( getName().c_str(), args, mcu_p );
			} 
		else if( cmd == "remove" ) {
				int ret = SbmCharacter::remove_from_scene( getName().c_str() );
				mcu_p->unregisterCharacter(this);
				return ret;

			} 
			else if( cmd == "inspect" ) {
					if( _skeleton ) {
						SkJoint* joint_p = _skeleton->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
						if( joint_p )	{
							inspect_skeleton( joint_p );
							//				inspect_skeleton_local_transform( joint_p );
							//				inspect_skeleton_world_transform( joint_p );
						}
					}
					return CMD_SUCCESS;
				}
				else
					if( cmd == "channels" ) {

						if( _skeleton )
						{
							if( ct_tree_p ) 
							{
								SkChannelArray channels = _skeleton->channels();
								int numChannels = channels.size();
								for (int c = 0; c < numChannels; c++)
								{
									std::stringstream strstr;
									strstr << c << " ";
									SkJoint* joint = channels.joint(c);
									if (joint)
									{
										strstr << joint->name() << " ";
									}
									SkChannel& channel = channels[c];
									int channelSize = channel.size();
									// get the channel index
									int channelIndex = ct_tree_p->toBufferIndex(c);
									strstr << channelIndex << " (" << channelSize << ") ";
									LOG( "%s", strstr.str().c_str() );
								}
							}
						}
						return CMD_SUCCESS;
					}
					else 
						if( cmd == "controllers" )
						{
							if( ct_tree_p )
							{
								int n = ct_tree_p->count_controllers();
								for (int c = 0; c < n; c++)
								{
									LOG( "%s", ct_tree_p->controller(c)->getName().c_str() );
								}
							}
							return CMD_SUCCESS;
						}
						else 
							if (cmd == "requests")
							{
								BML::Processor& bp = mcu_p->bml_processor;
								for (std::map<std::string, BML::BmlRequestPtr >::iterator iter = bp.getBMLRequestMap().begin();
									iter != bp.getBMLRequestMap().end();
									iter++)
								{
									if (all_characters)
									{
										LOG("%s", (*iter).second->requestId.c_str());
									}
									else
									{			
										// make sure the requests is for this character
										std::string requestWithName = (*iter).second->requestId;
										std::string charName = this->getName();
										charName.append("|");
										int index = requestWithName.find(charName);
										if (index == 0)
										{
											LOG("%s", (*iter).second->requestId.c_str());
										}
									}
								}
								return CMD_SUCCESS;
							}
							if (cmd == "interrupt")
							{
								int numRequestsInterrupted = 0;
								BML::Processor& bp = mcu_p->bml_processor;
								for (std::map<std::string, BML::BmlRequestPtr >::iterator iter = bp.getBMLRequestMap().begin();
									iter != bp.getBMLRequestMap().end();
									iter++)
								{
									std::string requestWithName = (*iter).second->requestId;
									if (all_characters)
									{
										int pipeLocation = requestWithName.find("|");
										std::string charName = requestWithName.substr(0, pipeLocation);
										std::string request = requestWithName.substr(pipeLocation + 1);
										std::stringstream strstr;
										strstr << "bp interrupt " << charName << " " << request << " .5"; 
										mcu_p->execute((char*) strstr.str().c_str());
										numRequestsInterrupted++;
									}
									else
									{			
										// make sure the requests is for this character

										std::string charName = this->getName();
										charName.append("|");
										int index = requestWithName.find(charName);
										if (index == 0)
										{
											std::string request = requestWithName.substr(charName.size());
											std::stringstream strstr;
											strstr << "bp interrupt " << this->getName() << " " << request << " .5"; 
											mcu_p->execute((char*) strstr.str().c_str());
											numRequestsInterrupted++;
										}
									}
									std::string name = this->getName();
									LOG("%d requests interrupted on character %s.", numRequestsInterrupted, name.c_str());
								}
								return CMD_SUCCESS;
							}
							else if( cmd == "prune" ) {
						return( prune_controller_tree( mcu_p ) );
					}
					else if( cmd == "viseme" )
					{ 
							char* viseme = args.read_token();
							char* next = args.read_token();
							//		float* curveInfo = NULL;
							//		float weight = 0.0f;
							//		float rampin_duration = 0.0;
							//		int numKeys = 0;
							//		int numKeyParams = 0;

							if( _stricmp( viseme, "curveon" ) == 0 )
							{
								set_viseme_curve_mode(true);
								return CMD_SUCCESS;
							}
							else if ( _stricmp( viseme, "reset") == 0)
							{
								// reset all of the visemes and action units to zero
								// clear away all the controllers on the head schedule
								MeCtScheduler2* scheduler = this->head_sched_p;
								if (!scheduler)
								{
									LOG("No scheduler available");
									return CMD_SUCCESS;
								}
								std::vector<MeCtScheduler2::TrackPtr> tracksToRemove;
								MeCtScheduler2::VecOfTrack tracks = scheduler->tracks();
								for (size_t t = 0; t < tracks.size(); t++)
								{
									MeCtScheduler2::TrackPtr track = tracks[t];
									MeController* controller = track->animation_ct();
									MeCtChannelWriter* channelWriter = dynamic_cast<MeCtChannelWriter*>(controller);
									if (channelWriter)
									{
										tracksToRemove.push_back(track);
									}
								}
								scheduler->remove_tracks(tracksToRemove);
								LOG("Removed %d visemes/Action Units", tracksToRemove.size());
								return CMD_SUCCESS;
							}

							else if( _stricmp( viseme, "curveoff" ) == 0 )
							{
								set_viseme_curve_mode(false);
								return CMD_SUCCESS;
							}
							else if( _stricmp( viseme, "timedelay" ) == 0 )
							{
								float timeDelay = (float)atof( next );
								set_viseme_time_delay( timeDelay );
								return CMD_SUCCESS;
							}
							if( _stricmp( viseme, "sounddelay" ) == 0 )
							{
								float soundDelay = (float)atof( next );
								set_viseme_sound_delay( soundDelay );
								return CMD_SUCCESS;
							}
							if( _stricmp( viseme, "magnitude" ) == 0 )
							{
								float magnitude = (float)atof( next );
								set_viseme_magnitude( magnitude );
								return CMD_SUCCESS;
							}
							if( _stricmp( viseme, "plateau" ) == 0 )
							{
								if (!next)
								{
									LOG("Character %s viseme plateau setting is %s", this->getName().c_str(), this->isVisemePlateau()? "on" : "off");
									return CMD_SUCCESS;
								}
								if (_stricmp(next, "on") == 0)
								{
									this->setVisemePlateau(true);
									LOG("Character %s viseme plateau setting is now on.", this->getName().c_str());
								}
								else if (_stricmp(next, "off") == 0)
								{
									this->setVisemePlateau(false);
									LOG("Character %s viseme plateau setting is now off.", this->getName().c_str());
								}
								else
								{
									LOG("use: char %s viseme plateau <on|off>", this->getName().c_str());
								}
								return CMD_SUCCESS;
							}

							if( strcmp( viseme, "minvisemetime" ) == 0 )
							{
								if (!next)
								{
									LOG("Character %s min viseme time is %f", this->getName().c_str(), this->getMinVisemeTime());
									return CMD_SUCCESS;
								}
								float minTime = (float)atof( next );
								setMinVisemeTime( minTime );
								return CMD_SUCCESS;
							}

							// keyword next to viseme
							if( strcmp( viseme, "clear" ) == 0 ) // removes all head controllers
							{
								if (head_sched_p)
								{
									std::vector<MeCtScheduler2::TrackPtr> tracks = head_sched_p->tracks();
									head_sched_p->remove_tracks(tracks);
								}
							}
							else if( strcmp( next, "curve" ) == 0 )
							{
								int numKeys = args.read_int();
								if( numKeys <= 0 )	
								{
									LOG( "Viseme data is missing" );
									return CMD_FAILURE;
								}
								int num_remaining = args.calc_num_tokens();
								int numKeyParams = num_remaining / numKeys;
								if( num_remaining != numKeys * numKeyParams )	{
									LOG( "Viseme data is malformed" );
									return CMD_FAILURE;
								}
								float* curveInfo = new float[ num_remaining ];
								args.read_float_vect( curveInfo, num_remaining );											

								//			schedule_viseme_blend_curve( viseme, mcu_p->time, 1.0f, curveInfo, numKeys, numKeyParams );
								schedule_viseme_curve( viseme, mcu_p->time, curveInfo, numKeys, numKeyParams, 0.1f, 0.1f );
								delete [] curveInfo;
							}
							else if( _stricmp( next, "trap" ) == 0 )
							{
								// trap <weight> <dur> [<ramp-in> <ramp-out>]
								float weight = args.read_float();
								float dur = args.read_float();
								float ramp_in = 0.1f;
								float ramp_out = 0.1f;
								if( args.calc_num_tokens() > 0 )
									ramp_in = args.read_float();
								if( args.calc_num_tokens() > 0 )
									ramp_out = args.read_float();
								schedule_viseme_trapezoid( viseme, mcu_p->time, weight, dur, ramp_in, ramp_out );
							}
							else
							{
								float weight = (float)atof(next);
								float rampin_duration = args.read_float();
								schedule_viseme_blend_ramp( viseme, mcu_p->time, weight, rampin_duration );
							}
							return CMD_SUCCESS;
						}
						else if (cmd == "visemeweight")
						{
							SmartBody::SBFaceDefinition* faceDefinition = this->getFaceDefinition();
							if (!faceDefinition)
							{
								LOG("Character %s does not have any visemes defined.", getName().c_str());
								return CMD_FAILURE;
							}
							int numRemaining = args.calc_num_tokens();
							if (numRemaining == 0)
							{
								// dump all of the existing viseme weights
								
								int numVisemes = faceDefinition->getNumVisemes();
								for (int v = 0; v < numVisemes; v++)
								{
									const std::string& visemeName = faceDefinition->getVisemeName(v);
									float weight = faceDefinition->getVisemeWeight(visemeName);
									LOG("%s %f", visemeName.c_str(), weight);

								}
								return CMD_SUCCESS;
							}
							if (numRemaining == 1)
							{
								std::string visemeName = args.read_token();
								if (!faceDefinition->hasViseme(visemeName))
								{
									LOG("Character %s does not have viseme %s defined.", getName().c_str(), visemeName.c_str());
									return CMD_FAILURE;
								}
								float weight = faceDefinition->getVisemeWeight(visemeName);
								LOG("%s %f", visemeName.c_str(), weight);
								return CMD_SUCCESS;
							}
							if (numRemaining == 2)
							{
								std::string visemeName = args.read_token();
								float weight = args.read_float();
								if (visemeName == "*")
								{
									// change all of the visemes
									int numVisemes = faceDefinition->getNumVisemes();
									for (int v = 0; v < numVisemes; v++)
									{
										std::string viseme = faceDefinition->getVisemeName(v);
										faceDefinition->setVisemeWeight(viseme, weight);
									}
									LOG("Set all visemes to weight %f", visemeName.c_str(), weight);
									return CMD_SUCCESS;
								}
								if (!faceDefinition->hasViseme(visemeName))
								{
									LOG("Character %s does not have viseme %s defined.", getName().c_str(), visemeName.c_str());
									return CMD_FAILURE;
								}
								faceDefinition->setVisemeWeight(visemeName, weight);
								LOG("%s %f", visemeName.c_str(), weight);
								return CMD_SUCCESS;
							}
							if (numRemaining > 2)
							{
								LOG("Usage:\nchar %s visemeweight\nchar %s visemeweight <visemename>\nchar %s visemeweight <visemename> <weight>", getName().c_str(), getName().c_str(), getName().c_str());
								return CMD_FAILURE;
							}

						}
					else 	if( cmd == "viewer" ) {
							std::string viewType = args.read_token();

							if (viewType == "0" || viewType == "bones")
							{
								if (scene_p)
									scene_p->set_visibility(1,0,0,0);
								dMesh_p->set_visibility(0);
							}
							else if (viewType == "1" || viewType == "visgeo")
							{
								if (scene_p)
									scene_p->set_visibility(0,1,0,0);
								dMesh_p->set_visibility(0);
							}
							else if (viewType == "2" || viewType == "colgeo")
							{
								if (scene_p)
									scene_p->set_visibility(0,0,1,0);
								dMesh_p->set_visibility(0);
							}
							else if (viewType == "3" || viewType == "axis")
							{
								if (scene_p)
									scene_p->set_visibility(0,0,0,1);
								dMesh_p->set_visibility(0);
							}
							else if (viewType == "4" || viewType == "deformable")
							{
								if (scene_p)
									scene_p->set_visibility(0,0,0,0);
								dMesh_p->set_visibility(1);
							}
							else
							{
								LOG("Usage: char <name> viewer <bones|visgeo|colgeo|axis|deformable>");
							}

							return CMD_SUCCESS;
						} 
			else if( cmd == "gazefade" ) {

					string fade_cmd = args.read_token();
					bool fade_in;
					bool print_track = false;
					if( fade_cmd == "in" ) {
						fade_in = true;
					}
					else
						if( fade_cmd == "out" ) {
							fade_in = false;
						}
						else
							if( fade_cmd == "print" ) {
								print_track = true;
							}
							else	{
								return( CMD_NOT_FOUND );
							}
							float interval = args.read_float();
							if( print_track )	{
								LOG( "char '%s' gaze tracks:", getName().c_str() );
							}
							mcuCBHandle& mcu = mcuCBHandle::singleton();
							double curTime = mcu.time;
							MeCtScheduler2::VecOfTrack track_vec = gaze_sched_p->tracks();
							int n = track_vec.size();
							for( int i = 0; i < n; i++ )	{
								MeCtScheduler2::TrackPtr t_p = track_vec[ i ];
								MeCtBlend* blend = dynamic_cast<MeCtBlend*>(t_p->blending_ct()); 
								MeController* ct_p = t_p->animation_ct();
								MeCtGaze* gaze_p = dynamic_cast<MeCtGaze*> (ct_p);
								if( gaze_p )	{	
									if (blend) {
										// don't fade gaze controllers that are scheduled 
										// but have not yet been started

										srLinearCurve& blend_curve = blend->get_curve();
										int n = blend_curve.get_num_keys();
										if( n > 0 )	{
											double h = blend_curve.get_head_param();
											double v = blend_curve.get_head_value();
											if (h > curTime) // controller hasn't started yet
											{
												continue;
											}
										}
									}
									if( print_track )	{
										LOG( " %s", gaze_p->getName().c_str() );
									}
									else
										if( fade_in )	{
											gaze_p->set_fade_in( interval );
										}
										else	{
											gaze_p->set_fade_out( interval );
										}
								}
							}
							return CMD_SUCCESS;
				} 	
				else 
					if( cmd == "reholster" ) {
						return( reholster_quickdraw( mcu_p ) );
					}
					else
						if( cmd == "blink" )
						{
							if( eyelid_reg_ct_p )	{
								eyelid_reg_ct_p->blink_now();
								return( CMD_SUCCESS );
							}
							return( CMD_FAILURE );
						}
						else
							if (cmd == "breathing")
							{
								if (breathing_p)
								{
									return mcu_character_breathing(getName().c_str(), args, mcu_p);
								}
								return CMD_FAILURE;
							}
							else
								if( cmd == "eyelid" )
								{
									if( eyelid_reg_ct_p )	{

										string eyelid_cmd  = args.read_token();
										if( eyelid_cmd.length()==0 ) {

											LOG( "char <> eyelid <command>:" );
											LOG( " eyelid print" );
											LOG( " eyelid pitch 0|1" );
											LOG( " eyelid range <upper-min> <upper-max> [<lower-min> <lower-max>]" );
											LOG( " eyelid close <closed-angle>" );
											LOG( " eyelid tight <upper-weight> [<lower-weight>]" );
											LOG( " eyelid delay <upper-delay> [<upper-delay>]" );


											//				eyelid_reg_ct_p->test();
											return( CMD_SUCCESS );
										}

										int n = args.calc_num_tokens();
										if( eyelid_cmd == "pitch" )
										{
											if( n > 0 )	{
												bool enable = args.read_int() != 0;
												eyelid_reg_ct_p->set_eyeball_tracking( enable );
											}
											else	{
												LOG( "MeCtEyeLidRegulator: pitch tracking %s", 
													eyelid_reg_ct_p->get_eyeball_tracking() ?
													"ENABLED" : "DISABLED"
													);
											}
											return( CMD_SUCCESS );
										}
										else
											if( eyelid_cmd == "range" )
											{
												if( n < 2 )	{
													return( CMD_FAILURE );
												}
												float upper_min = args.read_float();
												float upper_max = args.read_float();
												eyelid_reg_ct_p->set_upper_range( upper_min, upper_max );
												if( n >= 4 )	{
													float lower_min = args.read_float();
													float lower_max = args.read_float();
													eyelid_reg_ct_p->set_lower_range( lower_min, lower_max );
												}
												return( CMD_SUCCESS );
											}
											else
												if( eyelid_cmd == "close" )
												{
													if( n < 1 ) {
														return( CMD_FAILURE );
													}
													float close_angle = args.read_float();
													eyelid_reg_ct_p->set_close_angle( close_angle );
													return( CMD_SUCCESS );
												}
												else
													if( eyelid_cmd == "tight" )
													{
														float upper_mag = args.read_float();
														eyelid_reg_ct_p->set_upper_tighten( upper_mag );
														if( n > 1 ) {
															float lower_mag = args.read_float();
															eyelid_reg_ct_p->set_lower_tighten( lower_mag );
														}
														return( CMD_SUCCESS );
													}
													if( eyelid_cmd == "delay" )
													{
														float upper_delay = args.read_float();
														eyelid_reg_ct_p->set_upper_delay( upper_delay );
														if( n > 1 ) {
															float lower_delay = args.read_float();
															eyelid_reg_ct_p->set_lower_delay( lower_delay );
														}
														return( CMD_SUCCESS );
													}
													if( eyelid_cmd == "print" )
													{
														eyelid_reg_ct_p->print();
														return( CMD_SUCCESS );
													}
													return( CMD_NOT_FOUND );
									}
									return( CMD_FAILURE );
								}
								else
									if( cmd == "softeyes" )
									{
										if( eyelid_ct == NULL )
										{
											LOG("ERROR: SbmCharacter::parse_character_command(..): character \"%s\" has no eyelid_ct.", getName().c_str() );
											return CMD_FAILURE;
										}

										if( args.calc_num_tokens() == 0 )
										{
											LOG( "softeyes params: %s", isSoftEyes() ? "ENABLED" : "DISABLED" );
											float lo, up;

											eyelid_ct->get_weight( lo, up );
											LOG( " eyelid weight: { %f, %f }", lo, up );

											eyelid_ct->get_upper_lid_range( lo, up );
											LOG( " eyelid upper trans: { %f, %f }", lo, up );

											eyelid_ct->get_lower_lid_range( lo, up );
											LOG( " eyelid lower trans: { %f, %f }", lo, up );

											eyelid_ct->get_eye_pitch_range( lo, up );
											LOG( " eyeball pitch: { %f, %f }", lo, up );

											LOG( "commmands:" );
											LOG( " char <> softeyes [on|off] " );
											LOG( " char <> softeyes weight <lower> <upper>" );
											LOG( " char <> softeyes upperlid|lowerlid|eyepitch <lower-lim> <upper-lim>" );
											return CMD_SUCCESS;
										}

										std::string softEyesCommand = args.read_token();
										if( softEyesCommand == "on")
										{
											setSoftEyes( true );
										}
										else 
											if( softEyesCommand == "off")
											{
												setSoftEyes( false );
											}
											else	{

												float lo = args.read_float();
												float up = args.read_float();

												if( softEyesCommand == "weight" )
												{
													eyelid_ct->set_weight( lo, up );
												}
												else
													if( softEyesCommand == "upperlid" )
													{
														eyelid_ct->set_upper_lid_range( lo, up );
													}
													else 
														if( softEyesCommand == "lowerlid" )
														{
															eyelid_ct->set_lower_lid_range( lo, up );
														}
														else 
															if( softEyesCommand == "eyepitch" )
															{
																eyelid_ct->set_eye_pitch_range( lo, up );
															}
															else
															{
																LOG( "SbmCharacter::parse_character_command ERR: command '%s' not recognized", softEyesCommand.c_str());
																return CMD_NOT_FOUND;
															}
															return CMD_SUCCESS;
											}
											return CMD_SUCCESS;
									}
									else if (cmd == "sk")
									{
										string file = args.read_token();
										string scaleStr = args.read_token();
										double scale = atof(scaleStr.c_str());
										return writeSkeletonHierarchy(file, scale);		
									}
									else if (cmd == "minibrain")
									{
										if (args.calc_num_tokens() == 0)
										{
											MiniBrain* miniBrain = this->getMiniBrain();
											if (miniBrain)
											{
												LOG("Character %s has an active minibrain.", this->getName().c_str());
											}
											else
											{
												LOG("Character %s has an inactive minibrain.", this->getName().c_str());
											}
											return CMD_SUCCESS;
										}

										std::string tok = args.read_token();
										if (tok == "on")
										{
											MiniBrain* miniBrain = this->getMiniBrain();
											if (miniBrain)
											{
												delete miniBrain;
											}
											miniBrain = new MiniBrain();
											this->setMiniBrain(miniBrain);
											LOG("Minibrain for character %s is now on", this->getName().c_str());
										}
										else if (tok == "off")
										{
											MiniBrain* miniBrain = this->getMiniBrain();
											if (miniBrain)
											{
												delete miniBrain;
											}
											
											this->setMiniBrain(NULL);
											LOG("Minibrain for character %s is now off", this->getName().c_str());
										}
										else
										{
											LOG("Usage: char %s minibrain <on|off>", this->getName().c_str());
										}
										return CMD_SUCCESS;
									}
									else if ( cmd == "collision")
									{
										string phyCmd = args.read_token();
										if (phyCmd == "on" || phyCmd == "ON")
										{
											this->setJointPhyCollision(true);
											return CMD_SUCCESS;
										}
										else if (phyCmd == "off" || phyCmd == "OFF")
										{
											this->setJointPhyCollision(false);
											return CMD_SUCCESS;
										}
										else
										{
											LOG( "SbmCharacter::parse_character_command ERR: incorrect parameter for collision = %s",phyCmd.c_str());
											return CMD_FAILURE;
										}
									}
									else if ( cmd == "collider" )
									{
										string colCmd = args.read_token();
										if (colCmd == "build") // build all joint colliders automatically
										{
											this->buildJointPhyObjs();
											return CMD_SUCCESS;
										}
										/*
										else if (colCmd == "joint") // build 
										{
											string jointName = args.read_token();
											float length = -1.f, width = -1.f;			
											string nextCmd = args.read_token();
											while (nextCmd != "")
											{
												float nextFloat = args.read_float();
												if (nextCmd == "length")
													length = nextFloat;
												else if (nextCmd == "width")
													width = nextFloat;
												nextCmd = args.read_token();
											}
											this->setJointCollider(jointName,length,width);
											return CMD_SUCCESS;
										}	
										*/
									}
									else if ( cmd == "handmotion")
									{
										string hand_cmd = args.read_token();	
										if (hand_cmd == "grabhand" || hand_cmd == "reachhand" || hand_cmd == "releasehand")
										{
											string motion_name = args.read_token();
											string tagName = args.read_token();
											SkMotion* motion = mcu_p->lookUpMotion(motion_name.c_str());
											//LOG("SbmCharacter::parse_character_command LOG: add motion name : %s ", motion_name.c_str());
											int reachType = MeCtReachEngine::getReachType(tagName);//
											if (reachType == -1)
												reachType = MeCtReachEngine::RIGHT_ARM;
											if (motion)
											{
												//addReachMotion(motion);
												TagMotion tagMotion = TagMotion(reachType,motion);
												if (hand_cmd == "grabhand")
													this->grabHandData.insert(tagMotion);
												else if (hand_cmd == "reachhand")
													this->reachHandData.insert(tagMotion);
												else if (hand_cmd == "releasehand")
													this->releaseHandData.insert(tagMotion);

												return CMD_SUCCESS;
											}
											else
											{
												LOG( "SbmCharacter::parse_character_command ERR: motion '%s' not found", motion_name.c_str());
												return CMD_FAILURE;
											}
										}
									}
									else if ( cmd == "reachmotion" )
									{
										string reach_cmd = args.read_token();		
										bool print_track = false;
										if (reach_cmd == "add")
										{			
											string motion_name = args.read_token();		
											string tagName = args.read_token();
											int reachType = MeCtReachEngine::getReachType(tagName);//
											if (reachType == -1)
												reachType = MeCtReachEngine::RIGHT_ARM;
											SkMotion* motion = mcu_p->lookUpMotion(motion_name.c_str());
											//LOG("SbmCharacter::parse_character_command LOG: add motion name : %s ", motion_name.c_str());
											if (motion)
											{
												// assume the right hand motion and mirror the left hand motion
												addReachMotion(reachType,motion);
												return CMD_SUCCESS;
											}
											else
											{
												LOG( "SbmCharacter::parse_character_command ERR: motion '%s' not found", motion_name.c_str());
												return CMD_NOT_FOUND;
											}
										}
										else if (reach_cmd == "list")
										{
											int motion_num = this->reachMotionData.size();
											//SkMotion* motion = getReachMotion(motion_num);
											for (int c = 0; c < motion_num; c++)
											{
												LOG( "%s", getReachMotion(c)->getName().c_str() );
											}
											return CMD_SUCCESS;
										}
										else if (reach_cmd == "build")
										{			
											if (reachEngineMap.size() == 0)
											{
												LOG("character %s, reach engine is not initialized.", this->getName().c_str());
												return CMD_FAILURE;
											}				
											ReachEngineMap::iterator mi;
											for ( mi  = reachEngineMap.begin();
												mi != reachEngineMap.end();
												mi++)
											{
												MeCtReachEngine* re = mi->second;
												if (re)
												{
													re->updateMotionExamples(getReachMotionDataSet());
												}
											}
											return (CMD_SUCCESS);
										}			
										else if (reach_cmd == "play")
										{
											int motion_num = args.read_int();
											SkMotion* motion = getReachMotion(motion_num);
											if (motion)
											{
												//motion->name()
												char cmd[256];
												sprintf(cmd,"bml char %s <body posture=\"%s\"/>",getName().c_str(),motion->getName().c_str());
												mcuCBHandle::singleton().execute(cmd);
											}			
											return CMD_SUCCESS;
										}
										return CMD_FAILURE;
									}
									return( CMD_NOT_FOUND );
}
int SbmCharacter::character_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	string char_name = args.read_token();
	if( char_name.length()==0 ) {
		LOG( "HELP: char <> <command>" );
		LOG( "  param" );
		LOG( "  init" );
		LOG( "  smoothbindmesh" );
		LOG( "  smoothbindweight" );
		LOG( "  mesh");
		LOG( "  ctrl" );
		LOG( "  inspect" );
		LOG( "  channels" );
		LOG( "  controllers" );
		LOG( "  prune" );
		LOG( "  viseme curveon|curveoff" );
		LOG( "  viseme timedelay <timedelay>" );
		LOG( "  viseme magnitude <amount>" );
		LOG( "  viseme <viseme name> <weight> <ramp in>" );
		LOG( "  viseme <viseme name> trap <weight> <dur> [<ramp-in> [<ramp-out>]]" );
		LOG( "  viseme <viseme name> curve <number of keys> <curve information>" );
		LOG( "  viseme curve" );
		LOG( "  viseme plateau on|off" );
		LOG( "  clampvisemes on|off" );
		LOG( "  minvisemetime <amount>" );
		LOG( "  bone" );
		LOG( "  bonep" );
		LOG( "  remove" );
		LOG( "  viewer" );
		LOG( "  gazefade in|out [<interval>]" );
		LOG( "  gazefade print" );
		LOG( "  reholster" );
		LOG( "  blink" );
		LOG( "  eyelid pitch <enable>" );
		LOG( "  eyelid range <min-angle> <max-angle> [<lower-min> <lower-max>]" );
		LOG( "  eyelid close <closed-angle>" );
		LOG( "  eyelid tight <upper-norm> [<lower-norm>]" );
		LOG( "  softeyes" );
		LOG( "  sk <file> <scale>");
		LOG( "  minibrain <on|off>");
		return( CMD_SUCCESS );
	}

	string char_cmd = args.read_token();
	if( char_cmd.length()==0 ) {
		LOG( "SbmCharacter::character_cmd_func: ERR: Expected character command." );
		return CMD_FAILURE;
	}

	bool all_characters = false;
	SbmCharacter* character = NULL;
	if( char_name == "*" ) {

		all_characters = true;
		std::vector<std::string> characters;
		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
			iter != mcu_p->getCharacterMap().end();
			iter++)
		{
			characters.push_back((*iter).second->getName());
		}

		for (std::vector<std::string>::iterator citer = characters.begin();
			citer != characters.end();
			citer++)
		{
			srArgBuffer copy_args( args.peek_string() );
			character = mcu_p->getCharacter( *citer );
			int err = character->parse_character_command( char_cmd, copy_args, mcu_p, true );
			if( err != CMD_SUCCESS )
				return( err );
		}
		return( CMD_SUCCESS );
	} 

	character = mcu_p->getCharacter( char_name );
	if( character ) {

		int err = character->parse_character_command( char_cmd, args, mcu_p, false );
		if( err != CMD_NOT_FOUND )	{
			return( err );
		}
	}

	// Commands for uninitialized characters:
	if( char_cmd == "init" ) {

		char* skel_file = args.read_token();
		char* type = args.read_token();
		return(	
			mcu_character_init( char_name.c_str(), skel_file, type, mcu_p )
			);
	} 
	else
		if( char_cmd == "param" ) {

			char* param_name = args.read_token();
			GeneralParam * new_param = new GeneralParam;
			new_param->size = args.read_int();

			if( new_param->size == 0 )
			{
				LOG("SbmCharacter::parse_character_command: param_registeration ERR: parameter size not defined!\n");
				delete new_param;
				return( CMD_FAILURE );
			}
			for(int i = 0 ; i < (int)new_param->char_names.size(); i++)
			{
				if(char_name == new_param->char_names[i])
				{
					LOG("SbmCharacter::parse_character_command: param_registeration ERR: parameter redefinition!\n");
					delete new_param;
					return( CMD_FAILURE );	
				}
			}
			new_param->char_names.push_back( char_name );
			GeneralParamMap::iterator it; 
			if( (it = mcu_p->param_map.find(param_name)) != mcu_p->param_map.end())
			{
				it->second->char_names.push_back( char_name );
				delete new_param;
			}
			else
			{
				mcu_p->param_map.insert(make_pair(string(param_name),new_param));
			}
			return( CMD_SUCCESS );
		}

		LOG( "SbmCharacter::character_cmd_func ERR: char '%s' or cmd '%s' NOT FOUND", char_name.c_str(), char_cmd.c_str() );
		return( CMD_FAILURE );
}

int SbmCharacter::remove_from_scene( const char* char_name ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if( strcmp( char_name, "*" )==0 ) {
		std::vector<SbmCharacter*> charsToDelete;
		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			iter != mcu.getCharacterMap().end();
			iter++)
		{
			charsToDelete.push_back(iter->second);
		}
		for (size_t c = 0; c < charsToDelete.size(); c++)
		{
			charsToDelete[c]->remove_from_scene();	
			delete charsToDelete[c];
		}
		return CMD_SUCCESS;
	} else {
		SbmCharacter* char_p = mcu.getCharacter( char_name );

		if ( char_p ) {
			char_p->notifyObservers();
			char_p->remove_from_scene();
			delete char_p;

			return CMD_SUCCESS;
		} else {
			LOG( "ERROR: Unknown character \"%s\".\n", char_name );
			return CMD_FAILURE;
		}
	}
}

int SbmCharacter::set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Missing character id.");
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->getCharacter( character_id );
	if( character==NULL ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Unknown character \"%s\" to set.", character_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Missing attribute to set.");
		return CMD_FAILURE;
	}

	//  voice_code and voice-code are backward compatible patches
	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		return set_voice_cmd_func( character, args, mcu_p );
	} else if( attribute == "voicebackup") {
		return set_voicebackup_cmd_func( character, args, mcu_p );
	} else {
		return SbmPawn::set_attribute( character, attribute, args, mcu_p );
	}
}

int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	//  Command: set character voice <speech_impl> <character id> voice <implementation-id> <voice code>
	//  Where <implementation-id> is "remote" or "audiofile"
	//  Sets character's voice code
	const char* impl_id = args.read_token();

	if( strlen( impl_id )==0 ) {
		character->set_speech_impl( NULL );
		string s( "" );
		character->set_voice_code( s );

		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->getName().c_str());
	} else if( _stricmp( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_rvoice() );
		string s( voice_id );
		character->set_voice_code( s );
	} else if( _stricmp( impl_id, "local" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected local voice id.");
			return CMD_FAILURE;
		}
		LOG("set local voice");
		character->set_speech_impl( mcu_p->speech_localvoice() );
		FestivalSpeechRelayLocal* relay = mcu_p->festivalRelay();
		relay->setVoice(voice_id);
		string s( voice_id );
		character->set_voice_code( s );
	} else if( _stricmp( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else if( _stricmp( impl_id, "text" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_text() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else {
		LOG("ERROR: Unknown speech implementation \"%s\".", impl_id);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

int set_voicebackup_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	//  Command: set character voice <speech_impl> <character id> voice <implementation-id> <voice code>
	//  Where <implementation-id> is "remote" or "audiofile"
	//  Sets character's voice code
	const char* impl_id = args.read_token();

	if( strlen( impl_id )==0 ) {
		character->set_speech_impl_backup( NULL );
		string s("");
		character->set_voice_code_backup( s );

		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->getName().c_str());
	} else if( _stricmp( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_rvoice() );
		string s( voice_id );
		character->set_voice_code_backup( s );
	} else if( _stricmp( impl_id, "local" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected local voice id.");
			return CMD_FAILURE;
		}
		LOG("set local voice");
		character->set_speech_impl_backup( mcu_p->speech_localvoice() );
		FestivalSpeechRelayLocal* relay = mcu_p->festivalRelay();
		relay->setVoice(voice_id);
		string s( voice_id );
		character->set_voice_code_backup( s );
	} else if( _stricmp( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code_backup( voice_path_str );
	} else if( _stricmp( impl_id, "text" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_text() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code_backup( voice_path_str );
	} else {
		LOG("ERROR: Unknown speech implementation \"%s\".", impl_id);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

int SbmCharacter::print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Missing character id.");
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->getCharacter( character_id );
	if( character==NULL ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Unknown character \"%s\".", character_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Missing attribute to print.");
		return CMD_FAILURE;
	}

	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		//  Command: print character <character id> voice_code
		//  Print out the character's voice_id
		std::stringstream strstr;
		strstr << "character " << character_id << "'s voice_code: " << character->get_voice_code();
		LOG(strstr.str().c_str());
		return CMD_SUCCESS;
	} else if( attribute=="schedule" ) {
		return character->print_controller_schedules();
	} else {
		return SbmPawn::print_attribute( character, attribute, args, mcu_p );
	}
}


bool SbmCharacter::removeReachMotion( int tag, SkMotion* motion )
{
	TagMotion tagMotion = TagMotion(tag, motion);
	if (reachMotionData.find(tagMotion) != reachMotionData.end()) 
	{
		reachMotionData.erase(tagMotion);
		return true;
	}
	return false;
}

bool SbmCharacter::addReachMotion( int tag, SkMotion* motion )
{
	TagMotion tagMotion = TagMotion(tag, motion);
	if (reachMotionData.find(tagMotion) == reachMotionData.end()) 
	{

		reachMotionData.insert(tagMotion);
		return true;
	}
	return false;
}

SkMotion* SbmCharacter::getReachMotion( int index )
{
	MotionDataSet::iterator vi;
	int icount = 0;
	for (vi  = reachMotionData.begin();
		vi != reachMotionData.end();
		vi++)
	{
		if (icount == index)
			return vi->second;
		icount++;
	}
	return NULL;
}

void SbmCharacter::setMinVisemeTime(float minTime)
{
	_minVisemeTime = minTime;
}

float SbmCharacter::getMinVisemeTime() const
{
	return _minVisemeTime;
}

void SbmCharacter::notify(SBSubject* subject)
{
	SBPawn::notify(subject);

	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (attribute)
	{
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
		else if (attrName == "mesh")
		{
			SmartBody::StringAttribute* meshAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			std::stringstream strstr;
			strstr << "char " << getName() << " mesh " << meshAttribute->getValue();
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			int success = mcu.execute((char*) strstr.str().c_str());
			if (success != CMD_SUCCESS)
			{
				LOG("Problem setting attribute 'mesh' on character %s", getName().c_str());
			}
		}
		if (attrName == "facebone")
		{
			SmartBody::BoolAttribute* faceBoneAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			// deprecated - face bones are now defined by the face definition
		}
		if (attrName.find("steering.") == 0)
		{
			// update the steering params on the next evaluation cycle
			if (steeringAgent)
				steeringAgent->setSteerParamsDirty(true);
		}
	}
}

SrVec SbmCharacter::getFacingDirection()
{
	float x,y,z,h,p,r;
	get_world_offset(x,y,z,h,p,r);		
	SrMat mat;
	mat.roty(h*(float)M_PI/180.f);
	SrVec charDir(0.0, 0.0, 1.0f);
	charDir = charDir*mat;
	charDir.normalize();
	return charDir;
}

void SbmCharacter::setJointCollider( std::string jointName, float size )
{

}

int SbmCharacter::writeSkeletonHierarchy(std::string file, double scale)
{
	std::ofstream ostream(file.c_str());
	if (!ostream.good())
	{
		LOG("Cannot open file '%s' for writing .sk file.", file.c_str());
		return CMD_FAILURE;
	}

	SkJoint* root = _skeleton->root();
	if (!root)
		return CMD_SUCCESS;

	ostream << "set_name " << this->getName().c_str() << "\n";
	ostream << "\n";
	ostream << "skeleton\n";
	ostream << "root " << root->name() << "\n";
	writeSkeletonHierarchyRecurse(root, ostream, scale, 0);
	ostream << "\n";
	ostream << "end\n";

	LOG("Wrote file '%s'.", file.c_str());

	return CMD_SUCCESS;

}

void SbmCharacter::indent(int num, std::ofstream& ostream)
{
	for (int x = 0; x < num; x++)
	{
		ostream << "\t";
	}
}

void SbmCharacter::writeSkeletonHierarchyRecurse(SkJoint* joint, std::ofstream& ostream, double scale, int indentLevel)
{
	SrVec offset = joint->offset();
	indent(indentLevel, ostream);
	ostream << "{\n";
	indentLevel++;
	indent(indentLevel, ostream);
	ostream << "offset " << offset[0] * scale << " " << offset[1] * scale << " " << offset[2] * scale << "\n";
	SkJointPos* pos = joint->pos();
	if (pos)
	{
		if (!pos->frozen(0))
		{
			indent(indentLevel, ostream);
			ostream << "channel XPos 0 free\n";
		}
		if (!pos->frozen(1))
		{
			indent(indentLevel, ostream);
			ostream << "channel YPos 0 free\n";
		}
		if (!pos->frozen(2))
		{
			indent(indentLevel, ostream);
			ostream << "channel ZPos 0 free\n";
		}
	}
	SkJointQuat* quat = joint->quat();
	if (quat)
	{
		indent(indentLevel, ostream);
		ostream << "channel Quat\n";
	}
	ostream << "\n";

	for (int n = 0; n < joint->num_children(); n++)
	{
		SkJoint* child = joint->child(n);
		// make sure that 
		indent(indentLevel, ostream);
		ostream << "joint " << child->name() << "\n";
		writeSkeletonHierarchyRecurse(child, ostream, scale, indentLevel);	
	}

	indentLevel--;
	indent(indentLevel, ostream);
	ostream << "}\n";
}

SmartBody::SBFaceDefinition* SbmCharacter::getFaceDefinition()
{
	return _faceDefinition;
}

void SbmCharacter::setFaceDefinition(SmartBody::SBFaceDefinition* faceDefinition)
{
	if (!faceDefinition)
		return;

	if (_faceDefinition)
	{
		delete _faceDefinition;
		this->removeAllFaceChannels();
	}
	_faceDefinition = new SmartBody::SBFaceDefinition(faceDefinition);
	_faceDefinition->setName(faceDefinition->getName() + "_copy");

	SkSkeleton* skeleton = getSkeleton();
	if (!skeleton)
		return;

	updateFaceDefinition();
}

void SbmCharacter::updateFaceDefinition()
{
	if (!_faceDefinition)
		return;

	if (!getSkeleton())
		return;

	getSkeleton()->make_active_channels();

	SkSkeleton* skeleton = getSkeleton();
	SkChannelArray& skelChannelArray = skeleton->channels();


	// add the action units (AUs)
	int numAUs = _faceDefinition->getNumAUs();
	for (int a = 0; a < numAUs; a++)
	{
		int auNum = _faceDefinition->getAUNum(a);
		ActionUnit* au = _faceDefinition->getAU(auNum);
		this->addActionUnitChannel(auNum, au);
	}

	// add the visemes
	int numVisemes = _faceDefinition->getNumVisemes();
	for (int v = 0; v < numVisemes; v++)
	{
		std::string visemeName = _faceDefinition->getVisemeName(v);
		SkMotion* motion = _faceDefinition->getVisemeMotion(visemeName);
		this->addVisemeChannel(visemeName, motion);
	}

	// look for the start and end position of the viseme channels
	std::vector<SkJoint*>& joints = skeleton->get_joint_array();
	viseme_channel_start_pos = -1;
	viseme_channel_end_pos = -1;

	for (int c = 0; c < skelChannelArray.size(); c++)
	{
		SkChannel& channel = skelChannelArray[c];
		SkJoint* joint = channel.joint;
		if (joint && joint->getJointType() == SkJoint::TypeViseme)
		{
			if (viseme_channel_start_pos == -1)
			{
				viseme_channel_start_pos = c;
				viseme_channel_end_pos = c;
			}
			else
			{
				viseme_channel_end_pos = c;
			}
		}

	}

	int numAUChannels = _faceDefinition->getNumAUChannels();
	int numVisemeChannels = numVisemes;

//	viseme_channel_count = numAUs + numVisemes;
//	viseme_channel_count = viseme_channel_end_pos - viseme_channel_start_pos;
	viseme_channel_count = numAUChannels + numVisemeChannels;

	viseme_history_arr = new float[viseme_channel_count];
	for( int i=0; i<viseme_channel_count; i++ ) {
		viseme_history_arr[ i ] = -1.0;
	}

	// Do the sbm viseme name patch here
	viseme_name_patch.clear();
	std::vector<std::string> au_1_patch;
	au_1_patch.push_back("au_1_left");
	au_1_patch.push_back("au_1_right");
	std::vector<std::string> au_2_patch;
	au_2_patch.push_back("au_2_left");
	au_2_patch.push_back("au_2_right");
	std::vector<std::string> au_4_patch;
	au_4_patch.push_back("au_4_left");
	au_4_patch.push_back("au_4_right");
	std::vector<std::string> au_45_patch;
	au_45_patch.push_back("au_45_left");
	au_45_patch.push_back("au_45_right");
	viseme_name_patch.insert(make_pair("au_1", au_1_patch));
	viseme_name_patch.insert(make_pair("au_2", au_2_patch));
	viseme_name_patch.insert(make_pair("au_4", au_4_patch));
	viseme_name_patch.insert(make_pair("au_45", au_45_patch));

	// Do the bone bus viseme name patch here
	// hard coded, can be removed if these are added to bone bus and rendering side
	std::vector<std::string> closed_mouth;							// this should is an empty vector
	viseme_name_patch.insert(make_pair("_", closed_mouth));	// when receiving this two viseme, do not send signal to bone bus
	//viseme_name_patch.insert(make_pair("BMP", closed_mouth));

	std::vector<std::string> f_patch;								// "F" patch (match to "f")
	f_patch.push_back("f");
	viseme_name_patch.insert(make_pair("F",f_patch));

	std::vector<std::string> oh_patch;								// "OW" patch (match to "oh")
	oh_patch.push_back("oh");
	viseme_name_patch.insert(make_pair("OW",oh_patch));

	std::vector<std::string> all_viseme;							// "ALL" patch (match to all the viseme)
	all_viseme.push_back("Ao");
	all_viseme.push_back("D");
	all_viseme.push_back("EE");
	all_viseme.push_back("Er");
	all_viseme.push_back("f");
	all_viseme.push_back("Ih");
	all_viseme.push_back("j");
	all_viseme.push_back("KG");
	all_viseme.push_back("oh");
	all_viseme.push_back("OO");
	all_viseme.push_back("NG");
	all_viseme.push_back("R");
	all_viseme.push_back("Th");
	all_viseme.push_back("Z");
	// new added visemes: here viseme needs a better name, because following is really facial expressions
	//all_viseme.push_back("base"); // Removed by A. Shapiro 3/8/11 - was causing a conflict with joint name 'base'
	all_viseme.push_back("base_lower_face");
	all_viseme.push_back("base_upper_face");
	all_viseme.push_back("fe103_effort");
	all_viseme.push_back("fe107_surprised");
	all_viseme.push_back("fe112_happy");
	all_viseme.push_back("fe113_sad");
	all_viseme.push_back("fe119_bored");
	all_viseme.push_back("fe124_dislike");
	all_viseme.push_back("fe7_worried");
	all_viseme.push_back("fe8_scared");
	all_viseme.push_back("fe9_thinking");
	all_viseme.push_back("fe127_yawn");
	all_viseme.push_back("fe129_angry");
	viseme_name_patch.insert(make_pair("ALL",all_viseme));

	// make sure that the face controller has been updated
	if (face_ct)
	{
		face_ct->init( _faceDefinition );
	}
}


void SbmCharacter::removeAllFaceChannels()
{
	if (viseme_channel_start_pos == 0 && viseme_channel_end_pos == 0)
		return;

	SkSkeleton* skeleton = getSkeleton();
	SkChannelArray& channels = skeleton->channels();
	for (int i = viseme_channel_end_pos; i >= viseme_channel_start_pos; i++)
	{
		SkJoint* joint = channels.joint(i);
		if (joint)
		{
			// remove the joint
			// function does not exist - add it
		}
	}
}

void SbmCharacter::addVisemeChannel(std::string visemeName, SkMotion* motion)
{
	// add a corresponding channel for this viseme
	SmartBody::SBSkeleton* sbSkel = dynamic_cast<SmartBody::SBSkeleton*>(getSkeleton());
	if (!sbSkel)
	{
		LOG("No skeleton for character %s. Viseme %s cannot be added.", this->getName().c_str(), visemeName.c_str());
		return;
	}
	SmartBody::SBJoint* rootJoint = dynamic_cast<SmartBody::SBJoint*>(sbSkel->root());
	if (!rootJoint)
	{
		LOG("No root joint for character %s. Viseme %s cannot be added.", this->getName().c_str(), visemeName.c_str());
		return;
	}

	SmartBody::SBJoint* visemeJoint = new SmartBody::SBJoint();
	visemeJoint->setName(visemeName);
	visemeJoint->setJointType(SkJoint::TypeViseme);
	visemeJoint->setUsePosition(0, true);
	visemeJoint->pos()->limits(SkJointPos::X, 0, 2);  // Setting upper bound to 2 allows some exageration
	rootJoint->addChild(visemeJoint);
	viseme_channel_count++;
}

void SbmCharacter::addVisemeChannel(std::string visemeName, std::string motionName)
{
	// find the motion
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (motionName != "")
	{
		std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName);
		if (iter != mcu.motion_map.end())
		{
			addVisemeChannel(visemeName, (*iter).second);
		}
		else
		{
			LOG("Could not find motion named '%s' - no viseme added.", motionName.c_str());
		}
	}
	else
	{
		addVisemeChannel(visemeName, NULL);
	}
}

void SbmCharacter::addActionUnitChannel(int auNum, ActionUnit* au)
{
	// add a corresponding channel for this viseme
	std::stringstream strstr;
	strstr << auNum;
	std::string id = strstr.str();

	std::vector<std::string> allUnits;
	if( au->is_left() ) {
		std::string name = "au_";
		name += id;
		name += "_";
		name += "left";

		allUnits.push_back(name);
	}
	if( au->is_right()) {
		std::string name = "au_";
		name += id;
		name += "_";
		name += "right";

		allUnits.push_back(name);
	}
	if (au->is_bilateral())
	{
		std::string name = "au_";
		name += id;

		allUnits.push_back(name);
	}

	// add a corresponding channel for this action unit
	SmartBody::SBSkeleton* sbSkel = dynamic_cast<SmartBody::SBSkeleton*>(getSkeleton());
	if (!sbSkel)
	{
		LOG("No skeleton for character %s. Action unit %d cannot be added.", this->getName().c_str(), auNum);
		return;
	}
	SmartBody::SBJoint* rootJoint = dynamic_cast<SmartBody::SBJoint*>(sbSkel->root());
	if (!rootJoint)
	{
		LOG("No root joint for character %s. Action unit %d cannot be added.", this->getName().c_str(), auNum);
		return;
	}
	for (size_t a = 0; a < allUnits.size(); a++)
	{
		SmartBody::SBJoint* auJoint = new SmartBody::SBJoint();
		auJoint->setJointType(SkJoint::TypeViseme);
		auJoint->setName(allUnits[a]);
		auJoint->setUsePosition(0, true);
		auJoint->pos()->limits(SkJointPos::X, 0, 2);  // Setting upper bound to 2 allows some exageration
		rootJoint->addChild(auJoint);
		viseme_channel_count++;
	}
}

void SbmCharacter::setNvbg(Nvbg* nvbg)
{
	_nvbg = nvbg;
}

Nvbg* SbmCharacter::getNvbg()
{
	return _nvbg;
}

void SbmCharacter::setMiniBrain(MiniBrain* mini)
{
	_miniBrain = mini;
}

MiniBrain* SbmCharacter::getMiniBrain()
{
	return _miniBrain;
}


bool SbmCharacter::checkExamples()
{
	if (steeringAgent)
		steeringAgent->updateSteerStateName();

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string prefix = this->getName();
	if (this->statePrefix != "")
	{
		prefix = this->statePrefix;
	}
	std::string locomotionName = prefix + "Locomotion";
	std::string stepName = prefix + "Step";
	std::string startingLName = prefix + "StartingLeft";
	std::string startingRName = prefix + "StartingRight";
	std::string idleTurnName = prefix + "IdleTurn";
	std::vector<std::string> standardRequiredStates;
	standardRequiredStates.push_back(locomotionName);
	standardRequiredStates.push_back(stepName);
	standardRequiredStates.push_back(startingLName);
	standardRequiredStates.push_back(startingRName);
	standardRequiredStates.push_back(idleTurnName);

	int numMissing = 0;
	for (size_t x = 0; x < standardRequiredStates.size(); x++)
	{
		PAStateData* state = mcu.lookUpPAState(standardRequiredStates[x]);
		if (!state)
		{
			numMissing++;
//			LOG("SteeringAgent::checkExamples() standard config: Could not find state '%s' needed for example-based locomotion.", standardRequiredStates[x].c_str());
		}
	}
	if (numMissing == 0)
	{
		LOG("%s: Steering works under standard config.", this->getName().c_str());
		this->steeringConfig = STANDARD;
		return true;
	}

	std::vector<std::string> minimalRequiredStates;
	minimalRequiredStates.push_back(locomotionName);
	minimalRequiredStates.push_back(startingLName);
	minimalRequiredStates.push_back(startingRName);
	minimalRequiredStates.push_back(stepName);

	int numMissing1 = 0;
	for (size_t x = 0; x < minimalRequiredStates.size(); x++)
	{
		PAStateData* state = mcu.lookUpPAState(minimalRequiredStates[x]);
		if (!state)
		{
			numMissing1++;
//			LOG("SteeringAgent::checkExamples() minimal config: Could not find state '%s' needed for example-based locomotion.", minimalRequiredStates[x].c_str());
		}
	}
	if (numMissing1 == 0)
	{
		LOG("%s: Steering works under minimal config.", this->getName().c_str());
		this->steeringConfig = MINIMAL;
		return true;
	}
	LOG("%s: Steering cannot work under example mode, reverting back to basic mode", this->getName().c_str());
	return false;
}

SkMotion* SbmCharacter::findTagSkMotion( int tag, const MotionDataSet& motionSet )
{
	MotionDataSet::const_iterator vi;
	for ( vi  = motionSet.begin();
		vi != motionSet.end();
		vi++)
	{
		TagMotion tagMotion = *vi;
		if (tagMotion.first == tag)
			return tagMotion.second;
	}
	return NULL;
}