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

#include <SK/sk_skeleton.h>
#include <ME/me_ct_blend.hpp>
#include <ME/me_ct_time_shift_warp.hpp>
#include "mcontrol_util.h"
#include "mcontrol_callbacks.h"
#include "me_utilities.hpp"
#include <ME/me_spline_1d.hpp>
#include <ME/me_ct_interpolator.h>
#include "sr_curve_builder.h"

#define USE_REACH 1
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
	sched_p->name( sched_name.c_str() );
	return sched_p;
}

//  Constructor
SbmCharacter::SbmCharacter( const char* character_name )
:	SbmPawn( character_name ),
	speech_impl( NULL ),
	speech_impl_backup( NULL ),
	posture_sched_p( CreateSchedulerCt( character_name, "posture" ) ),
	motion_sched_p( CreateSchedulerCt( character_name, "motion" ) ),
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
	face_neutral( NULL ),
	_soft_eyes_enabled( ENABLE_EYELID_CORRECTIVE_CT ),
	_height(1.0f), 
	_visemePlateau(true)
{
	posture_sched_p->ref();
	motion_sched_p->ref();
	gaze_sched_p->ref();
	head_sched_p->ref();
	param_sched_p->ref();
	eyelid_ct->ref();

	bonebusCharacter = NULL;

	use_viseme_curve = false;
	viseme_time_offset = 0.0;
	viseme_magnitude = 1.0;
	viseme_channel_count = 0;
	viseme_channel_start_pos = 0;
	viseme_channel_end_pos = 0;
	viseme_history_arr = NULL;
	_minVisemeTime = 0.0f;

	_numSteeringGoals = 0;
}

//  Destructor
SbmCharacter::~SbmCharacter( void )	{

	printf("delete character %s\n",this->name);

	if (posture_sched_p)
		posture_sched_p->unref();
	if (motion_sched_p)
		motion_sched_p->unref();
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
	if (eyelid_ct)
		eyelid_ct->unref();

	if (locomotion_ct)
		locomotion_ct->unref();
	if (param_animation_ct)
		param_animation_ct->unref();

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterDelete( name );
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

int SbmCharacter::init( SkSkeleton* new_skeleton_p,
					    SkMotion* face_neutral,
                        AUMotionMap* au_motion_map,
                        VisemeMotionMap* viseme_motion_map,
						GeneralParamMap* param_map,
                        const char* unreal_class,
						bool use_locomotion,
						bool use_param_animation)
{
	// Store pointers for access via init_skeleton()
	if( face_neutral ) {
		this->face_neutral      = face_neutral;
		face_neutral->ref();
	}
	this->au_motion_map     = au_motion_map;
	this->viseme_motion_map = viseme_motion_map;
	this->param_map = param_map;

	int init_result = SbmPawn::init( new_skeleton_p );  // Indirectly calls init_skeleton 
	if( init_result!=CMD_SUCCESS ) {
		return( init_result ); 
	}

	float height = new_skeleton_p->getCurrentHeight();
	setHeight(height);

	eyelid_reg_ct_p = new MeCtEyeLidRegulator();
	if( eyelid_reg_ct_p )	{
		eyelid_reg_ct_p->ref();
		if (!face_neutral)
			eyelid_reg_ct_p->set_use_blink_viseme( true );

		eyelid_reg_ct_p->init(true);
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
		this->param_animation_ct->ref();
	}

	//if (use_locomotion) 
	{
		this->locomotion_ct =  new MeCtLocomotionClass();
		std::string locomotionname = std::string(name)+ "'s locomotion controller";
		this->locomotion_ct->name( locomotionname.c_str() );
		locomotion_ct->get_navigator()->setWordOffsetController(world_offset_writer_p);
		locomotion_ct->ref();
	}
	// Clear pointer data no longer used after this point in initialization.
	this->viseme_motion_map = NULL;
	this->au_motion_map     = NULL;
	if( face_neutral ) {
		face_neutral->unref();
		this->face_neutral = NULL; // MLT: What is the purpose of this?
	}

	posture_sched_p->init();
	motion_sched_p->init();
	if( locomotion_ct != NULL )
		locomotion_ct->init();
	gaze_sched_p->init();

#ifdef USE_REACH
	reach_sched_p->init();
	grab_sched_p->init();
	constraint_sched_p->init();
#endif


	// Blink controller before head group (where visemes are controlled)
	head_sched_p->init();

	param_sched_p->init();

	ct_tree_p->name( std::string(name)+"'s ct_tree" );

	// Add Prioritized Schedule Controllers to the Controller Tree
	ct_tree_p->add_controller( posture_sched_p );
	ct_tree_p->add_controller( motion_sched_p );
	
	if (locomotion_ct)
		ct_tree_p->add_controller( locomotion_ct );

	if (param_animation_ct)
		ct_tree_p->add_controller(param_animation_ct);

	ct_tree_p->add_controller( reach_sched_p );	
	ct_tree_p->add_controller( grab_sched_p );
	
	ct_tree_p->add_controller( gaze_sched_p );		
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
			
			eyelid_ct->init();
			ct_tree_p->add_controller( eyelid_ct );
		}
	}

	// motion player
	motionplayer_ct = new MeCtMotionPlayer(this);
	std::string mpName(name);
	mpName += "'s motion player";
	motionplayer_ct->name(mpName.c_str());
	motionplayer_ct->setActive(false);
	ct_tree_p->add_controller(motionplayer_ct);

	bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( name, unreal_class, mcuCBHandle::singleton().net_face_bones );

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterCreate( name, unreal_class );
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
	
	return( CMD_SUCCESS ); 
}


void SbmCharacter::add_face_channel( const string& name, const int wo_index ) {
	add_bounded_float_channel( name, 0, 2, wo_index );
}


void SbmCharacter::add_bounded_float_channel( const string& name, float lower, float upper, const int wo_index ) {

	SkJoint* joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
	joint_p->name( SkJointName( name.c_str() ) );
	// Activate channel with lower limit != upper.
	joint_p->pos()->limits( SkJointPos::X, lower, upper );  // Setting upper bound to 2 allows some exageration
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
		SkJoint* loc_vector_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		loc_vector_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_VELOCITY ) );
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
		SkJoint* g_angular_velocity_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* l_angular_velocity_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* l_angular_angle_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* time_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		SkJoint* id_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		g_angular_velocity_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_GLOBAL_ROTATION ) );
		l_angular_velocity_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_LOCAL_ROTATION ) );
		l_angular_angle_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_LOCAL_ROTATION_ANGLE ) );
		time_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_TIME ) );
		id_joint_p->name( SkJointName( MeCtLocomotionPawn::LOCOMOTION_ID ) );

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

	if( face_neutral ) {
		// Unlike other controllers, face_ct is constructed and initialized in
		// init_skelton because it initialized by data in the au_motion_map
		// and viseme_motion_map that is not available later.
		face_ct = new MeCtFace();
		face_ct->ref();

		string face_ct_name( name );
		face_ct_name += "'s face_ct";
		face_ct->name( face_ct_name.c_str() );

		face_ct->init( face_neutral );
	}

	std::string viseme_start_name;
	viseme_channel_count = 0;

	{	// Generate AU and viseme activation channels.
		AUMotionMap::const_iterator i   = au_motion_map->begin();
		AUMotionMap::const_iterator end = au_motion_map->end();
		
		for(; i != end; ++i ) {
			//const int   id = i->first;
			stringstream id;
			id << i->first;
			AUMotionPtr au( i->second );

			if( au->is_left() ) {
				string name = "au_";
				//name += id;
				name += id.str();
				name += "_left";

				// Create the AU control channel
				add_face_channel( name, wo_index );
				if (viseme_channel_count == 0)	viseme_start_name = name;
				viseme_channel_count ++;

				// TODO: Add to au_channel_map (?)

				// Register control channel with face controller
				if( face_ct )
					face_ct->add_key( name.c_str(), au->left.get() );
			}
			if( au->is_right()) {
				string name = "au_";
				//name += id;
				name += id.str();
				name += "_right";

				// Create the AU control channel
				add_face_channel( name, wo_index );
				if (viseme_channel_count == 0)	viseme_start_name = name;
				viseme_channel_count ++;

				// Register control channel with face controller
				if( face_ct )
					face_ct->add_key( name.c_str(), au->right.get() );
			}
			if (au->is_bilateral())
			{
				string name = "au_";
					//name += id;
					name += id.str();

					// Create the AU control channel
					add_face_channel( name, wo_index );
					if (viseme_channel_count == 0)	viseme_start_name = name;
					viseme_channel_count ++;

					// Register control channel with face controller
					if( face_ct )
						face_ct->add_key( name.c_str(), au->left.get() );
			}
		}

		VisemeMotionMap::const_iterator vi   = viseme_motion_map->begin();
		VisemeMotionMap::const_iterator vend = viseme_motion_map->end();
		for(; vi != vend; ++vi ) {
			const string&    id     = vi->first;
			SkMotion* motion = vi->second;

				/* get rid of the "viseme_" prefix */ 
//				string name = "viseme_";
//				name += id;

				// Create the Viseme control channel
				add_face_channel( id, wo_index );
				if (viseme_channel_count == 0)	viseme_start_name = id;
				viseme_channel_count ++;
				
				// Register control channel with face controller
				if( face_ct )
					if( motion ) {
						face_ct->add_key( id.c_str(), motion );
			}
		}
	}

	if( face_ct ) {
		face_ct->finish_adding();
	}

	// Adding general parameter channels using a format of <char_name>_1_1, <char_name>_2_1, <char_name>_2_2, <char_name>_2_3...(for joint name)
	int Index = 0;
	for( GeneralParamMap::const_iterator pos = param_map->begin(); pos != param_map->end(); pos++ )
	{
		for( int m = 0; m < (int)pos->second->char_names.size(); m++ )
		{
			if( pos->second->char_names[m] == string(this->name) )
			{
				Index ++;
				for(int i = 0; i< pos->second->size; i++)
				{
					std::stringstream joint_name;
					joint_name << this->name << "_" << Index << "_" << ( i + 1 );
					add_bounded_float_channel( joint_name.str(), 0 , 1, wo_index );
				}
			}
		}
	}	

	// Rebuild the active channels to include new joints
	skeleton_p->make_active_channels();

	// keep record of viseme channel start index
	if (viseme_start_name != "")
	{
		viseme_channel_start_pos = skeleton_p->channels().search(SkJointName(viseme_start_name.c_str()), SkChannel::XPos);
		viseme_channel_end_pos = viseme_channel_start_pos + viseme_channel_count;
		viseme_history_arr = new float[ viseme_channel_count ];
	}
	else	// no map exist
	{
		viseme_channel_start_pos = 0;
		viseme_channel_end_pos = 0;
	}

	for( int i=0; i<viseme_channel_count; i++ ) {
		viseme_history_arr[ i ] = -1.0;
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
				LOG("DEBUG: %s \"%s\" withheld from pruning by MePrunePolicy.", ct->controller_type(), ct->name());
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
		LOG("DEBUG: sbm_character.cpp prune_schedule(..): Pruning schedule \"%s\":", sched->name());

	typedef MeCtScheduler2::TrackPtr   TrackPtr;
	typedef MeCtScheduler2::VecOfTrack VecOfTrack;

	VecOfTrack tracks = sched->tracks();  // copy of tracks
	VecOfTrack tracks_to_remove;  // don't remove during iteration

	VecOfTrack::iterator first = tracks.begin();
	VecOfTrack::iterator it    = tracks.end();

	bool hasReach = false;
	bool hasConstraint = false;
	bool hasBodyReach = false;
	bool hasReachLeft = false, hasReachRight = false;
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
					double t = blend_curve.get_tail_param();
					double v = blend_curve.get_tail_value();

					if( LOG_CONTROLLER_TREE_PRUNING )
						LOG("\tblend_Ct \"%s\": blend curve last knot: t = %f v = %f", blend_ct->name(), t, v );
					if( t <= time ) {
						flat_blend_curve = true;
						if( v == 0.0 ) {
							in_use = false;
						}
					} else {
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
						strstr << "DEBUG: prune_schedule(..): sched \""<<sched->name()<<"\", anim_source \""<<anim_source->name()<<"\": blend_ct without spline knots.";
						LOG(strstr.str().c_str());
						blend_ct->print_state(1);  // Prints controller type, name, and blend curve
					}
					in_use = false; // A spline with no knots evaluates to 0
				}
			}

			const char* anim_ct_type = anim_source->controller_type();
			if( LOG_CONTROLLER_TREE_PRUNING )
			{
				std::stringstream strstr;
				strstr << '\t' << anim_ct_type << " \"" << anim_source->name() << "\": in_use = "<<in_use<<", flat_blend_curve = "<<flat_blend_curve<<endl;
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
				} else if( anim_ct_type == MeCtSimpleNod::_type_name ) {
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
				} else if (dynamic_cast<MeCtReach*>(anim_source)) {
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
				else if (dynamic_cast<MeCtExampleBodyReach*>(anim_source)) {
					MeCtExampleBodyReach* ct_bodyReach = dynamic_cast<MeCtExampleBodyReach*>(anim_source);
					if (hasBodyReach)
					{
						in_use = false;
					}
					else
					{
						hasBodyReach = true;
					}
				}
				else if( anim_ct_type == MeCtMotion::type_name || anim_ct_type == MeCtQuickDraw::type_name ) {
					if( motion_ct || pose_ct ) {
						in_use = false;
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
					LOG("WARNING: Cannot prune unknown controller type \"%s\"", anim_source->controller_type());
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
		strstr << "SbmCharacter \""<<name<<"\" prune_controller_tree(..) @ time "<<time<<endl;
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
	vrProcEnd_msg += name;
	mcu.vhmsg_send( vrProcEnd_msg.c_str() );

	mcu.character_map.remove( name );

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
	std::map<std::string, std::vector<std::string>>::iterator iter;
	
	iter = viseme_name_patch.find(viseme);
	if (iter != viseme_name_patch.end())
	{
		for (size_t nCount = 0; nCount < iter->second.size(); nCount++)
			visemeNames.push_back(iter->second[nCount]);
	}
	else
		visemeNames.push_back(viseme);

	for( size_t nCount = 0; nCount < visemeNames.size(); nCount++ )
	{
		if( num_keys > 0 )
		{
			float timeDelay = this->get_viseme_time_delay();
			
			ostringstream ct_name;
			ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

			SkChannelArray channels;
			channels.add( SkJointName(visemeNames[nCount].c_str()), SkChannel::XPos );

			MeCtCurveWriter* ct_p = new MeCtCurveWriter();
			ct_p->name( ct_name.str().c_str() );
			ct_p->init( channels ); // CROP, CROP, true

			if (num_keys <= 2)
			{
				for (int i = 0; i < num_keys; i++)	{
					float t = curve_info[ i * num_key_params + 0 ];
					float w = curve_info[ i * num_key_params + 1 ];
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
					float w = curve_info[ i * num_key_params + 1 ];

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
	curve_info[ 1 ] = weight;
	curve_info[ 2 ] = duration;
	curve_info[ 3 ] = weight;
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
	std::map<std::string, std::vector<std::string>>::iterator iter;
	
	iter = viseme_name_patch.find(viseme);
	if (iter != viseme_name_patch.end())
	{
		for (size_t nCount = 0; nCount < iter->second.size(); nCount++)
			visemeNames.push_back(iter->second[nCount]);
	}
	else
		visemeNames.push_back(viseme);

	for( size_t nCount = 0; nCount < visemeNames.size(); nCount++ )
	{
		if( num_keys > 0 )
		{
			float timeDelay = this->get_viseme_time_delay();
			
			ostringstream ct_name;
			ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

			SkChannelArray channels;
			channels.add( SkJointName(visemeNames[nCount].c_str()), SkChannel::XPos );

			MeCtChannelWriter* ct_p = new MeCtChannelWriter();
			ct_p->name( ct_name.str().c_str() );
			ct_p->init( channels, true );
			SrBuffer<float> value;
			value.size( 1 );
			value[ 0 ] = weight;
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
	curve_info[ 2 ] = rampin_duration;
	schedule_viseme_blend_curve( viseme, start_time, weight, curve_info, 2, 2 );
}

void SbmCharacter::forward_visemes( double curTime )
{
	SBMCharacterListener *listener_p = mcuCBHandle::singleton().sbm_character_listener;

	if( bonebusCharacter || listener_p )
	{
		SkChannelArray& channels = skeleton_p->channels();
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
						bonebusCharacter->SetViseme( channels.name(c).get_string(), value, 0 );
					}
					if( listener_p )
					{
						listener_p->OnViseme( name, channels.name(c).get_string(), value, 0 );
					}
					viseme_history_arr[ i ] = value;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

void SbmCharacter::inspect_skeleton( SkJoint* joint_p, int depth )	{
	int i, j, n;
	
	if( joint_p )	{
		const char *name = joint_p->name();
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
		const char *name = joint_p->name();
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
		const char *name = joint_p->name();
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
		LOG("Character %s's schedule:", name);
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
					&& strcmp(blending_ct->controller_type(), MeCtBlend::CONTROLLER_TYPE ) )
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
		strstr << "WARNING: Character \""<<name<<"\" reholster(): No quickdraw controller found.";
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

	if( cmd == "smoothbindmesh" ) {
		char* obj_file = args.read_token();
		return mcu_character_load_mesh( name, obj_file, mcu_p );
	} 
	else 
	if( cmd == "smoothbindweight" ) {
		char* skin_file = args.read_token();
		return mcu_character_load_skinweights( name, skin_file, mcu_p );
	} 
	else 
	if( cmd == "ctrl" ) {
		return mcu_character_ctrl_cmd( name, args, mcu_p );
	} 
	else 
	if( cmd == "bone" ) {
		return mcu_character_bone_cmd( name, args, mcu_p );
	} 
	else 
	if( cmd == "bonep" ) {
		return mcu_character_bone_position_cmd( name, args, mcu_p );
	} 
	else 
	if( cmd == "remove" ) {
		return SbmCharacter::remove_from_scene( name );
	} 
	else 
	if( cmd == "inspect" ) {
	
		if( skeleton_p ) {
			SkJoint* joint_p = skeleton_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
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

		if( skeleton_p )
		{
			if( ct_tree_p ) 
			{
				SkChannelArray channels = skeleton_p->channels();
				int numChannels = channels.size();
				for (int c = 0; c < numChannels; c++)
				{
					std::stringstream strstr;
					strstr << c << " ";
					SkJoint* joint = channels.joint(c);
					if (joint)
					{
						strstr << joint->name().get_string() << " ";
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
				LOG( "%s", ct_tree_p->controller(c)->name() );
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
				std::string charName = this->name;
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
				
				std::string charName = this->name;
				charName.append("|");
				int index = requestWithName.find(charName);
				if (index == 0)
				{
					std::string request = requestWithName.substr(charName.size());
					std::stringstream strstr;
					strstr << "bp interrupt " << this->name << " " << request << " .5"; 
					mcu_p->execute((char*) strstr.str().c_str());
					numRequestsInterrupted++;
				}
			}
			LOG("%d requests interrupted on character %s.", numRequestsInterrupted, this->name);
			return CMD_SUCCESS;
		}
		return CMD_SUCCESS;
	}
	else
	if( cmd == "prune" ) {
		return( prune_controller_tree( mcu_p ) );
	}
	else 
	if( cmd == "viseme" ) { 
		char* viseme = args.read_token();
		char* next = args.read_token();
//		float* curveInfo = NULL;
//		float weight = 0.0f;
//		float rampin_duration = 0.0;
//		int numKeys = 0;
//		int numKeyParams = 0;

		if( _strcmpi( viseme, "curveon" ) == 0 )
		{
			set_viseme_curve_mode(true);
			return CMD_SUCCESS;
		}
		else 
		if( _strcmpi( viseme, "curveoff" ) == 0 )
		{
			set_viseme_curve_mode(false);
			return CMD_SUCCESS;
		}
		else
		if( _strcmpi( viseme, "timedelay" ) == 0 )
		{
			float timeDelay = (float)atof( next );
			set_viseme_time_delay( timeDelay );
			return CMD_SUCCESS;
		}
		if( _strcmpi( viseme, "magnitude" ) == 0 )
		{
			float magnitude = (float)atof( next );
			set_viseme_magnitude( magnitude );
			return CMD_SUCCESS;
		}
		if( _strcmpi( viseme, "plateau" ) == 0 )
		{
			if (!next)
			{
				LOG("Character %s viseme plateau setting is %s", this->name, this->isVisemePlateau()? "on" : "off");
				return CMD_SUCCESS;
			}
			if (_strcmpi(next, "on") == 0)
			{
				this->setVisemePlateau(true);
				LOG("Character %s viseme plateau setting is now on.", this->name);
			}
			else if (_strcmpi(next, "off") == 0)
			{
				this->setVisemePlateau(false);
				LOG("Character %s viseme plateau setting is now off.", this->name);
			}
			else
			{
				LOG("use: char %s viseme plateau <on|off>", this->name);
			}
			return CMD_SUCCESS;
		}

		if( _strcmpi( viseme, "minvisemetime" ) == 0 )
		{
			if (!next)
			{
				LOG("Character %s min viseme time is %f", this->name, this->getMinVisemeTime());
				return CMD_SUCCESS;
			}
			float minTime = (float)atof( next );
			setMinVisemeTime( minTime );
			return CMD_SUCCESS;
		}

		// keyword next to viseme
		if( _strcmpi( viseme, "clear" ) == 0 ) // removes all head controllers
		{
			if (head_sched_p)
			{
				std::vector<MeCtScheduler2::TrackPtr> tracks = head_sched_p->tracks();
				head_sched_p->remove_tracks(tracks);
			}
		}
		else
		if( _strcmpi( next, "curve" ) == 0 )
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
		else
		if( _strcmpi( next, "trap" ) == 0 )
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
	else 
	if( cmd == "viewer" ) {
		int mode = args.read_int();
		switch (mode)
		{
			case 0:
				scene_p->set_visibility(1,0,0,0);
				dMesh_p->set_visibility(0);
				break;
			case 1:
				scene_p->set_visibility(0,1,0,0);
				dMesh_p->set_visibility(0);
				break;
			case 2:
				scene_p->set_visibility(0,0,1,0);
				dMesh_p->set_visibility(0);
				break;
			case 3:
				scene_p->set_visibility(0,0,0,1);
				dMesh_p->set_visibility(0);
				break;
			case 4:
				scene_p->set_visibility(0,0,0,0);
				dMesh_p->set_visibility(1);
				break;
			default:	
				break;
		}
		return CMD_SUCCESS;
	} 
	else 
	if( cmd == "gazefade" ) {

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
			LOG( "char '%s' gaze tracks:", name );
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
					LOG( " %s", gaze_p->name() );
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
			LOG("ERROR: SbmCharacter::parse_character_command(..): character \"%s\" has no eyelid_ct.", name );
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
	else if ( cmd == "reachmotion" )
	{
		string reach_cmd = args.read_token();		
		bool print_track = false;
		if (reach_cmd == "add")
		{			
			string motion_name = args.read_token();
			SkMotion* motion = mcu_p->lookUpMotion(motion_name.c_str());
			//LOG("SbmCharacter::parse_character_command LOG: add motion name : %s ", motion_name.c_str());
			if (motion)
			{
				addReachMotion(motion);
				return CMD_SUCCESS;
			}
			else
			{
				LOG( "SbmCharacter::parse_character_command ERR: motion '%s' not found", motion_name.c_str());
				return CMD_NOT_FOUND;
			}
		}

		else if (reach_cmd == "grabhand" || reach_cmd == "reachhand" || reach_cmd == "releasehand")
		{
			string motion_name = args.read_token();
			SkMotion* motion = mcu_p->lookUpMotion(motion_name.c_str());
			//LOG("SbmCharacter::parse_character_command LOG: add motion name : %s ", motion_name.c_str());
			if (motion)
			{
				//addReachMotion(motion);
				if (reach_cmd == "grabhand")
					this->grabHandData.insert(motion);
				else if (reach_cmd == "reachhand")
					this->reachHandData.insert(motion);
				else if (reach_cmd == "releasehand")
					this->releaseHandData.insert(motion);

				return CMD_SUCCESS;
			}
			else
			{
				LOG( "SbmCharacter::parse_character_command ERR: motion '%s' not found", motion_name.c_str());
				return CMD_NOT_FOUND;
			}
		}
		else if (reach_cmd == "play")
		{
			int motion_num = args.read_int();
			SkMotion* motion = getReachMotion(motion_num);
			if (motion)
			{
				//motion->name()
				char cmd[256];
				sprintf(cmd,"bml char %s <body posture=\"%s\"/>",name,motion->name());
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
		mcu_p->character_map.reset();
		while( character = mcu_p->character_map.next() ) {
			
			srArgBuffer copy_args( args.peek_string() );
			int err = character->parse_character_command( char_cmd, copy_args, mcu_p, true );
			if( err != CMD_SUCCESS )	{
				return( err );
			}
		}
		return( CMD_SUCCESS );
	} 

	character = mcu_p->character_map.lookup( char_name.c_str() );
	if( character ) {
	
		int err = character->parse_character_command( char_cmd, args, mcu_p, false );
		if( err != CMD_NOT_FOUND )	{
			return( err );
		}
	}
	
	// Commands for uninitialized characters:
	if( char_cmd == "init" ) {

		char* skel_file = args.read_token();
		char* unreal_class = args.read_token();
		return(	
			mcu_character_init( char_name.c_str(), skel_file, unreal_class, mcu_p )
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
		SbmCharacter * char_p;
		mcu.character_map.reset();
		while( char_p = mcu.character_map.pull() ) {
			char_p->remove_from_scene();
			delete char_p;
		}
		return CMD_SUCCESS;
	} else {
		SbmCharacter* char_p = mcu.character_map.lookup( char_name );

		if ( char_p ) {
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

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
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
		character->set_voice_code( string("") );
		
		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->name);
	} else if( _strcmpi( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_rvoice() );
		character->set_voice_code( string( voice_id ) );
	} else if( _strcmpi( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else if( _strcmpi( impl_id, "text" )==0 ) {
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
		character->set_voice_code_backup( string("") );
		
		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->name);
	} else if( _strcmpi( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_rvoice() );
		character->set_voice_code_backup( string( voice_id ) );
	} else if( _strcmpi( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code_backup( voice_path_str );
	} else if( _strcmpi( impl_id, "text" )==0 ) {
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

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
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

bool SbmCharacter::addReachMotion( SkMotion* motion )
{
	if (reachMotionData.find(motion) == reachMotionData.end()) 
	{
		reachMotionData.insert(motion);
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
			return *vi;
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