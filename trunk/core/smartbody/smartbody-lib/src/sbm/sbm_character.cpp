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


const bool LOG_PRUNE_CMD_TIME                        = false;
const bool LOG_CONTROLLER_TREE_PRUNING               = false;
const bool LOG_PRUNE_TRACK_WITHOUT_BLEND_SPLIE_KNOTS = false;

const bool ENABLE_EYELID_CORRECTIVE_CT = true;

using namespace std;

// Predeclare private functions defined below
static int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );
static int set_voicebackup_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );
static inline bool parse_float_or_error( float& var, const char* str, const string& var_name );


/////////////////////////////////////////////////////////////
//  Static Data
const char* SbmCharacter::LOCOMOTION_VELOCITY = "locomotion_velocity";
const char* SbmCharacter::LOCOMOTION_ROTATION = "locomotion_rotation";
const char* SbmCharacter::LOCOMOTION_GLOBAL_ROTATION = "locomotion_global_rotation";
const char* SbmCharacter::LOCOMOTION_LOCAL_ROTATION = "locomotion_local_rotation";
const char* SbmCharacter::LOCOMOTION_LOCAL_ROTATION_ANGLE = "locomotion_local_rotation_angle";
const char* SbmCharacter::LOCOMOTION_TIME = "locomotion_time";
const char* SbmCharacter::LOCOMOTION_ID = "locomotion_id";

/////////////////////////////////////////////////////////////
const char* SbmCharacter::ORIENTATION_TARGET  = "orientation_target";



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
	locomotion_ct_analysis( NULL ),
	locomotion_ct( NULL ),
	eyelid_reg_ct_p( NULL ),
	head_sched_p( CreateSchedulerCt( character_name, "head" ) ),
	param_sched_p( CreateSchedulerCt( character_name, "param" ) ),
	face_ct( NULL ),
	eyelid_ct( new MeCtEyeLid() ),
	face_neutral( NULL ),
	use_viseme_curve( false ),
	softEyes( ENABLE_EYELID_CORRECTIVE_CT ),
	_height(1.0f)
{
	posture_sched_p->ref();
	motion_sched_p->ref();
	gaze_sched_p->ref();
	head_sched_p->ref();
	param_sched_p->ref();
	eyelid_ct->ref();

	bonebusCharacter = NULL;

	viseme_time_offset = 0.0;
}

//  Destructor
SbmCharacter::~SbmCharacter( void )	{

	posture_sched_p->unref();
	motion_sched_p->unref();
	gaze_sched_p->unref();

	if( eyelid_reg_ct_p )
		eyelid_reg_ct_p->unref();

	head_sched_p->unref();
	param_sched_p->unref();
	if( face_ct )
		face_ct->unref();
	eyelid_ct->unref();

	if (locomotion_ct_analysis)
		delete locomotion_ct_analysis;
	locomotion_ct->unref();
		
	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterDelete( name );
	}

    if ( bonebusCharacter )
    {
       mcuCBHandle::singleton().bonebus.DeleteCharacter( bonebusCharacter );
       bonebusCharacter = NULL;
    }
}

int SbmCharacter::init_locomotion_analyzer(const char* skel_file, mcuCBHandle *mcu_p)
{
	/*std::string walkForwardMotion = "Step_WalkForward";
	std::string strafeMotion = "Step_StrafeRight";
	std::string standingMotion = "HandsAtSide_Motex_Softened";

	SkMotion* walking1_p = NULL;
	SkMotion* walking2_p = NULL;
	SkMotion* standing_p = NULL;
	
	std::map<std::string, SkMotion*>::iterator walkIter = mcu_p->motion_map.find(walkForwardMotion);
	if (walkIter != mcu_p->motion_map.end())
		walking1_p = (*walkIter).second;

	std::map<std::string, SkMotion*>::iterator strafeIter = mcu_p->motion_map.find(strafeMotion);
	if (strafeIter != mcu_p->motion_map.end())
		walking2_p = (*strafeIter).second;

	std::map<std::string, SkMotion*>::iterator standIter = mcu_p->motion_map.find(standingMotion);
	if (standIter != mcu_p->motion_map.end())
		standing_p = (*standIter).second;

	// need better error checking here
	bool motionsNotLoaded = false;
	if (!walking1_p)
	{
		LOG("No %s animation", walkForwardMotion.c_str());
		motionsNotLoaded = true;
	}
	if (!walking2_p)
	{
		LOG("No %s animation", strafeMotion.c_str());
		motionsNotLoaded = true;
	}
	if (!standing_p)
	{
		LOG("No %s animation", standingMotion.c_str());
		motionsNotLoaded = true;
	}
	if (motionsNotLoaded)
	{
		return CMD_FAILURE;
	}*/

	SkSkeleton* walking_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );
	SkSkeleton* standing_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );

	locomotion_ct->init_skeleton(standing_skeleton, walking_skeleton);
	locomotion_ct_analysis->set_ct(locomotion_ct);

	/*locomotion_ct_analysis->init(standing_p, mcu_p->me_paths);
	locomotion_ct_analysis->init_blended_anim();

	locomotion_ct->add_locomotion_anim(walking1_p);
	//locomotion_ct_analysis->add_locomotion(walking1_p, 1, 0);
	locomotion_ct_analysis->add_locomotion(walking1_p, 29, 45, 6, 1, 17, 33);

	locomotion_ct->add_locomotion_anim(walking2_p);
	//locomotion_ct_analysis->add_locomotion(walking2_p, 2, 0);
	locomotion_ct_analysis->add_locomotion(walking2_p, 50, 2, 34, 24, 51, 3);*/
	
	//locomotion_ct_analysis->print_info();

	return CMD_SUCCESS;
}

/*void SbmCharacter::automate_locomotion(bool automate)
{
	locomotion_ct->automate = automate;
}*/

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

MeCtLocomotionAnalysis* SbmCharacter::get_locomotion_ct_analysis()
{
	return this->locomotion_ct_analysis;
}

AUChannelMap& SbmCharacter::get_au_channel_map()
{
	return au_channel_map;
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
						bool use_locomotion)
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
	eyelid_reg_ct_p->ref();
	eyelid_reg_ct_p->init(true);
	eyelid_reg_ct_p->set_upper_range( -30.0, 30.0 );
	eyelid_reg_ct_p->set_close_angle( 30.0 );
	ostringstream ct_name;
	ct_name << name << "'s eyelid controller";
	eyelid_reg_ct_p->name( ct_name.str().c_str() );

	//if (use_locomotion) 
	{
		this->locomotion_ct_analysis = new MeCtLocomotionAnalysis();
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
		this->face_neutral      = NULL; // MLT: What is the purpose of this?
	}

	posture_sched_p->init();
	motion_sched_p->init();
	if( locomotion_ct != NULL )
		locomotion_ct->init();
	gaze_sched_p->init();

	// Blink controller before head group (where visemes are controlled)
	head_sched_p->init();

	param_sched_p->init();

	ct_tree_p->name( std::string(name)+"'s ct_tree" );

	// Add Prioritized Schedule Controllers to the Controller Tree
	ct_tree_p->add_controller( posture_sched_p );
	ct_tree_p->add_controller( motion_sched_p );
	if (locomotion_ct)
		ct_tree_p->add_controller( locomotion_ct );
	ct_tree_p->add_controller( gaze_sched_p );

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
	all_viseme.push_back("base");
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
		const float max_speed = 1000000;   // TODO: set max speed value to some reasonable value for the current scale

		// 3D vector for current speed and trajectory of the body
		SkJoint* loc_vector_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		loc_vector_joint_p->name( SkJointName( LOCOMOTION_VELOCITY ) );
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
		g_angular_velocity_joint_p->name( SkJointName( LOCOMOTION_GLOBAL_ROTATION ) );
		l_angular_velocity_joint_p->name( SkJointName( LOCOMOTION_LOCAL_ROTATION ) );
		l_angular_angle_joint_p->name( SkJointName( LOCOMOTION_LOCAL_ROTATION_ANGLE ) );
		time_joint_p->name( SkJointName( LOCOMOTION_TIME ) );
		id_joint_p->name( SkJointName( LOCOMOTION_ID ) );

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
	int visemeChannelCounter = 0;

	{	// Generate AU and viseme activation channels.
		AUMotionMap::const_iterator i   = au_motion_map->begin();
		AUMotionMap::const_iterator end = au_motion_map->end();
		
		for(; i != end; ++i ) {
			//const int   id = i->first;
			stringstream id;
			id << i->first;
			AUMotionPtr au( i->second );

			if( au->is_bilateral() ) {
				if( au->left ) {
					string name = "au_";
					//name += id;
					name += id.str();
					name += "_left";

					// Create the AU control channel
					add_face_channel( name, wo_index );
					if (visemeChannelCounter == 0)	viseme_start_name = name;
					visemeChannelCounter ++;

					// TODO: Add to au_channel_map (?)

					// Register control channel with face controller
					if( face_ct )
						face_ct->add_key( name.c_str(), au->left.get() );
				}
				if( au->right ) {
					string name = "au_";
					//name += id;
					name += id.str();
					name += "_right";

					// Create the AU control channel
					add_face_channel( name, wo_index );
					if (visemeChannelCounter == 0)	viseme_start_name = name;
					visemeChannelCounter ++;

					// Register control channel with face controller
					if( face_ct )
						face_ct->add_key( name.c_str(), au->right.get() );
				}
			} else {
				if( au->left ) {
					string name = "au_";
					//name += id;
					name += id.str();

					// Create the AU control channel
					add_face_channel( name, wo_index );
					if (visemeChannelCounter == 0)	viseme_start_name = name;
					visemeChannelCounter ++;

					// Register control channel with face controller
					if( face_ct )
						face_ct->add_key( name.c_str(), au->left.get() );
				}
			}
		}

		VisemeMotionMap::const_iterator vi   = viseme_motion_map->begin();
		VisemeMotionMap::const_iterator vend = viseme_motion_map->end();
		for(; vi != vend; ++vi ) {
			const string&    id     = vi->first;
			SkMotion* motion = vi->second;

			if( motion ) {
				/* get rid of the "viseme_" prefix */ 
//				string name = "viseme_";
//				name += id;

				// Create the Viseme control channel
				add_face_channel( id, wo_index );
				if (visemeChannelCounter == 0)	viseme_start_name = id;
				visemeChannelCounter ++;
				
				// Register control channel with face controller
				if( face_ct )
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
					if( t < time ) {
						flat_blend_curve = true;
						if( v == 0.0 ) {
							in_use = false;
						}
					} else {
						LOG("sbm_character.cpp prune_schedule(): ERR: this pruning path not implemented");
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
				} else if( anim_ct_type == MeCtMotion::type_name || anim_ct_type == MeCtQuickDraw::type_name ) {
					if( motion_ct || pose_ct ) {
						in_use = false;
					} else {
						motion_ct = anim_source;
					}
				} else if( anim_ct_type == MeCtPose::type_name ) {
					if( pose_ct ) {
						in_use = false;
					} else {
						pose_ct = (MeCtPose*)anim_source;
					}
				} else if( anim_ct_type == MeCtChannelWriter::TYPE ) {
					const SkChannelArray& ct_channels = anim_source->controller_channels();
					vector<int> new_channels;  // list of indices to channels in use
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

					if(foundChannelMatch)
					{
						in_use = false;
					}
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
	prune_schedule( this, gaze_sched_p, mcu_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct, raw_channels );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
void SbmCharacter::update_viseme( const char* viseme,
								  float weight )
{
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

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (size_t nCount = 0; nCount < visemeNames.size(); nCount++)
	{
		if (bonebusCharacter)
		{
			bonebusCharacter->SetViseme( visemeNames[nCount].c_str(), weight, 0 );
		}
		if ( mcuCBHandle::singleton().sbm_character_listener )
		{
			mcuCBHandle::singleton().sbm_character_listener->OnViseme( name, visemeNames[nCount].c_str(), weight, 0 );
		}
		if (is_face_controller_enabled()) 
		{
			// Viseme/AU channel activation
			ostringstream ct_name;
			ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

			SkChannelArray channels;
			channels.add( SkJointName(visemeNames[nCount].c_str()), SkChannel::XPos );

			MeCtChannelWriter* ct = new MeCtChannelWriter();
			ct->name( ct_name.str().c_str() );
			ct->init( channels, true );
			SrBuffer<float> value;
			value.size( 1 );
			value[ 0 ] = (float)weight;
			ct->set_data(value);

			head_sched_p->schedule( ct, mcu.time, mcu.time + ct->controller_duration(), 0, 0 );
		}
	}
}
#endif

void SbmCharacter::set_viseme_curve(
	const char* viseme, 
	double start_time, 
	float* curve_info, 
	int numKeys, 
	int numKeyParams 
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
		if( numKeys > 0 )
		{
			std::map<std::string, srLinearCurve*>::iterator iter = visemeCurveMap.find( visemeNames[ nCount ] );
			if (iter != visemeCurveMap.end())
			{
				visemeCurveMap.erase(iter);
			}

			float timeDelay = this->get_viseme_time_delay();

			if (is_face_controller_enabled()) 
			{
				ostringstream ct_name;
				ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

				SkChannelArray channels;
				channels.add( SkJointName(visemeNames[nCount].c_str()), SkChannel::XPos );

				MeCtChannelWriter* ct = new MeCtChannelWriter();
				ct->name( ct_name.str().c_str() );
				ct->init( channels, true );
				SrBuffer<float> value;
				value.size( 1 );
				value[ 0 ] = 1.0f;
				ct->set_data(value);

				head_sched_p->schedule( ct, start_time + timeDelay, curve_info, numKeys, numKeyParams );
			}

			LOG( "start: %f val: %f ramp: %f weight:%f", 
				start_time + curve_info[ 0 ],
				curve_info[ 1 ],
				curve_info[ 2 ],
				curve_info[ 3 ]
			);

#if 1 // for forwarding...
			srLinearCurve* curve = new srLinearCurve();
			for (int i = 0; i < numKeys; i++)
			{
				float time = curve_info[ i * numKeyParams + 0 ];
				float weight = curve_info[ i * numKeyParams + 1 ];
				curve->insert( start_time + time + timeDelay, weight );
			}
			visemeCurveMap.insert( make_pair( visemeNames[ nCount ], curve ) );
#endif
		}
	}
}

#if 1
void SbmCharacter::set_viseme_ramp( const char* viseme,
							  float weight,
							  double start_time,
							  float rampin_duration )
{
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

	for (size_t nCount = 0; nCount < visemeNames.size(); nCount++)
	{
		if (is_face_controller_enabled()) 
		{
			// Viseme/AU channel activation
			ostringstream ct_name;
			ct_name << "Viseme \"" << visemeNames[nCount] << "\", Channel \"" << visemeNames[nCount] << "\"";

			SkChannelArray channels;
			channels.add( SkJointName(visemeNames[nCount].c_str()), SkChannel::XPos );

			MeCtChannelWriter* ct = new MeCtChannelWriter();
			ct->name( ct_name.str().c_str() );
			ct->init( channels, true );
			SrBuffer<float> value;
			value.size( 1 );
			value[ 0 ] = (float)weight;
			ct->set_data(value);

			head_sched_p->schedule( ct, start_time, start_time + ct->controller_duration(), rampin_duration, 0 );
		}

		LOG( "start: %f ramp: %f", start_time, rampin_duration );
#if 1 // for forwarding...
		srLinearCurve* curve = new srLinearCurve();
		curve->insert( start_time, 0.0f );
		curve->insert( start_time + rampin_duration, weight );
		visemeCurveMap.insert( make_pair( visemeNames[ nCount ], curve ) );
#endif
	}
}
#else

void SbmCharacter::set_viseme_ramp( 
	const char* viseme,
	float weight,
	double start_time,
	float rampin_duration
)	{

	float curve_info[ 4 ];
	curve_info[ 0 ] = 0.0f;
	curve_info[ 1 ] = 0.0f;
	curve_info[ 2 ] = rampin_duration;
	curve_info[ 3 ] = weight;
	
	set_viseme_curve( viseme, start_time, curve_info, 2, 2 );
}
#endif

#if 0
void SbmCharacter::build_viseme_curve(
	const char* viseme, 
	double start_time, 
	float* curve_info, 
	int numKeys, 
	int numKeyParams 
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
		if( numKeys > 0 )
		{
			std::map<std::string, srLinearCurve*>::iterator iter = visemeCurveMap.find( visemeNames[ nCount ] );
			if (iter != visemeCurveMap.end())
			{
				visemeCurveMap.erase(iter);
			}

			srLinearCurve* curve = new srLinearCurve();
//			curve->insert( start_time, 0 );

			float timeDelay = this->get_viseme_time_delay();
			for (int i = 0; i < numKeys; i++)
			{
				float time = curve_info[ i * numKeyParams + 0 ];
				float weight = curve_info[ i * numKeyParams + 1 ];
				curve->insert( start_time + time + timeDelay, weight );
			}
			visemeCurveMap.insert( make_pair( visemeNames[ nCount ], curve ) );
		}
	}
}
#endif

void SbmCharacter::forward_viseme( const char* viseme,
								  float weight )
{
	std::vector<std::string> visemeNames;
	std::map<std::string, std::vector<std::string>>::iterator iter;
	iter = viseme_name_patch.find(viseme);
	if (iter != viseme_name_patch.end())
	{
		for( size_t nCount = 0; nCount < iter->second.size(); nCount++ )
			visemeNames.push_back( iter->second[ nCount ] );
	}
	else
		visemeNames.push_back( viseme );

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (size_t nCount = 0; nCount < visemeNames.size(); nCount++)
	{
		if (bonebusCharacter)
		{
			bonebusCharacter->SetViseme( visemeNames[ nCount ].c_str(), weight, 0 );
		}
		if ( mcuCBHandle::singleton().sbm_character_listener )
		{
			mcuCBHandle::singleton().sbm_character_listener->OnViseme( name, visemeNames[ nCount ].c_str(), weight, 0 );
		}
	}
}

void SbmCharacter::forward_all_visemes( double curTime )
{
	if( eyelid_reg_ct_p )	{

		bool left_changed;
		bool right_changed;
		float left_weight = eyelid_reg_ct_p->get_upper_left( &left_changed );
		float right_weight = eyelid_reg_ct_p->get_upper_right( &right_changed );

		if( left_weight == right_weight )	{ // ensured with float granularity...
			if( left_changed )	{
				forward_viseme( "blink", left_weight );
			}
		}
		else	{
			if( left_changed )	{
				forward_viseme( "blink_lf", left_weight );
			}
			if( right_changed )	{
				forward_viseme( "blink_rt", right_weight );
			}		
		}
	}

	std::map<std::string, srLinearCurve*>::iterator curveIter;
	std::vector<std::string> visemesToDelete;
	for (curveIter = visemeCurveMap.begin(); curveIter != visemeCurveMap.end(); curveIter++)
	{

		float weight = (float)curveIter->second->evaluate( curTime );
		std::map<std::string, std::vector<std::string>>::iterator namePatchIter;
		namePatchIter = viseme_name_patch.find(curveIter->first);

		if (namePatchIter != viseme_name_patch.end())
		{
			for (size_t nCount = 0; nCount < namePatchIter->second.size(); nCount++)
			{
				forward_viseme( namePatchIter->second[nCount].c_str(), weight );
			}
		}
		else
		{
			forward_viseme( curveIter->first.c_str(), weight );
		}

		if (weight == 0 && curveIter->second->get_tail_param() <= curTime)
		{
			visemesToDelete.push_back(curveIter->first);
		}
	}

	for (size_t x = 0; x < visemesToDelete.size(); x++)
	{
		std::map<std::string, srLinearCurve*>::iterator iter = visemeCurveMap.find(visemesToDelete[x]);
		if (iter != visemeCurveMap.end())
		{
			visemeCurveMap.erase(iter);
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
		matrix_t M;
		int i, j;

		SrMat sr_M = joint_p->lmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

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
		matrix_t M;
		int i, j;

		joint_p->update_gmat_up();
		SrMat sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

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
					float  blend_spline_tanget = -1/blend_out_dur;

					MeCtBlend* blend_ct = (MeCtBlend*)blending_ct;
#if 0
					MeSpline1D& spline = blend_ct->blend_curve();
					// TODO: Don't assume we're starting at 1, may already be less than and already blending out.
					spline.make_cusp( blend_out_start,1,  0,1, blend_spline_tanget,1 );
					MeSpline1D::Knot* knot = spline.make_cusp( blend_out_end,  0,  blend_spline_tanget,1, 0,1 );
#else
					srLinearCurve& blend_curve = blend_ct->get_curve();
					blend_curve.insert( blend_out_start, 1.0 );
					blend_curve.insert( blend_out_end, 0.0 );
#endif
					// TODO: delete following knots

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

int SbmCharacter::character_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	string char_name = args.read_token();
	if( char_name.length()==0 ) {
#if 0
		LOG("ERROR: Expected character name.");
		return CMD_FAILURE;
#else
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
		LOG( "  viseme <viseme name> <weight> <ramp in>" );
		LOG( "  viseme <viseme name> curve <number of keys> <curve information>" );
		LOG( "  viseme curve" );
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
#endif
	}

	string char_cmd  = args.read_token();
	if( char_cmd.length()==0 ) {
		LOG("ERROR: Expected character command.");
		return CMD_FAILURE;
	}

	bool all_characters = false;
	SbmCharacter* character = NULL;
	if( char_name=="*" ) {
		all_characters = true;
	} else {
		character = mcu_p->character_map.lookup( char_name.c_str() );
	}

	if( char_cmd=="param" ) {
		char* param_name = args.read_token();
		GeneralParam * new_param = new GeneralParam;
		new_param->size = args.read_int();
		
		if( new_param->size == 0 )
		{
			LOG("param_registeration ERR: parameter size not defined!\n");
			delete new_param;
			return( CMD_FAILURE );
		}
		for(int i = 0 ; i < (int)new_param->char_names.size(); i++)
		{
			if(char_name == new_param->char_names[i])
			{
				LOG("param_registeration ERR: parameter redefinition!\n");
				
				delete new_param;
				return( CMD_FAILURE );				
			}
		}
		new_param->char_names.push_back(char_name);
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
	else
	if( char_cmd=="init" ) {
		// Original style creation:
		char* skel_file = args.read_token();
		char* unreal_class = args.read_token();
		return(	
			mcu_character_init( char_name.c_str(), skel_file, unreal_class, mcu_p )
		);
	} 
	else 
	if( char_cmd=="smoothbindmesh" ) {
		char* obj_file = args.read_token();
		return mcu_character_load_mesh( char_name.c_str(), obj_file, mcu_p );
	} 
	else 
	if( char_cmd=="smoothbindweight" ) {
		char* skin_file = args.read_token();
		return mcu_character_load_skinweights( char_name.c_str(), skin_file, mcu_p );
	} 
	else 
	if( char_cmd=="ctrl" ) {
		return mcu_character_ctrl_cmd( char_name.c_str(), args, mcu_p );
	} 
	else 
	if( char_cmd=="inspect" ) {
		if( character ) {
			if( character->skeleton_p ) {
				SkJoint* joint_p = character->skeleton_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
				if( joint_p )	{
					character->inspect_skeleton( joint_p );
//					inspect_skeleton_local_transform( joint_p );
//					inspect_skeleton_world_transform( joint_p );
				}
			}
		}
		return CMD_SUCCESS;
	}
	else
	if( char_cmd=="channels" ) {
		if( character ) {
			if( character->skeleton_p ) {
				MeControllerTreeRoot* controllerTree = character->ct_tree_p;

				SkChannelArray channels = character->skeleton_p->channels();
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
					int channelIndex = controllerTree->toBufferIndex(c);
					strstr << channelIndex << " (" << channelSize << ") ";
					LOG("%s", strstr.str().c_str());

				}
			}
		}
		return CMD_SUCCESS;
	}
	else if (char_cmd == "controllers")
	{
		if (!character)
			return CMD_FAILURE;

		MeControllerTreeRoot* controllerTree = character->ct_tree_p;
		int numControllers = controllerTree->count_controllers();
	
		for (int c = 0; c < numControllers; c++)
		{
			LOG("%s", controllerTree->controller(c)->name());
		}
		return CMD_SUCCESS;
	}
	else
	if( char_cmd=="prune" ) {
		int result = CMD_SUCCESS;
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				if( character->prune_controller_tree( mcu_p ) != CMD_SUCCESS ) {
					LOG("ERROR: Failed to prune controller tree of character \"%s\".", character->name);
					result = CMD_FAILURE;
				}
			}
		} else if( character ) {
			int result = character->prune_controller_tree( mcu_p );
			if( result != CMD_SUCCESS ) {
				LOG("ERROR: Failed to prune controller tree of character \"%s\".", character->name);
			}
		} else {
			LOG("ERROR: Unknown character  \"%s\"  for prune command.", char_name.c_str());
			result = CMD_FAILURE;
		}
		return result;
	}
	else 
	if( char_cmd=="viseme" ) {
	//  1) the time delay function is used to postpone viseme curve which now is a little bit mismatch with the sound
	//	2) need to modify the curve slope in and out later, now they are always 0
 
		char* viseme = args.read_token();		// viseme name
		char* next = args.read_token();			// two modes: original add-on or curve
		float* curveInfo = NULL;
		float weight = 0.0;
		float rampin_duration = 0.0;
		int numKeys = 0;
		int numKeyParams = 0;

		// viseme
		if (_strcmpi(viseme, "curveon") == 0)
		{
			character->set_viseme_curve_mode(true);
			return CMD_SUCCESS;
		}
		else if(_strcmpi(viseme, "curveoff") == 0)
		{
			character->set_viseme_curve_mode(false);
			return CMD_SUCCESS;
		}
		else if(_strcmpi(viseme, "timedelay") == 0)
		{
			float timeDelay = (float)atof(next);
			character->set_viseme_time_delay(timeDelay);
			return CMD_SUCCESS;
		}

		// keyword next to viseme
		if( _strcmpi( next, "curve" ) == 0 )
		{
			numKeys = args.read_int();
			if( numKeys <= 0 )	
			{
				LOG( "Viseme data is missing" );
				return CMD_FAILURE;
			}
			int num_remaining = args.calc_num_tokens();
			numKeyParams = num_remaining / numKeys;
			if( num_remaining != numKeys * numKeyParams )	{
				LOG( "Viseme data is malformed" );
				return CMD_FAILURE;
			}
			curveInfo = new float[ num_remaining ];
			args.read_float_vect( curveInfo, num_remaining );
		}
		else
		{
			weight = (float)atof(next);
			rampin_duration = args.read_float();
		}

		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() )
			{
				if( curveInfo == NULL )
				{
//					character->set_viseme( viseme, weight, mcu_p->time, rampin_duration );
					character->set_viseme_ramp( viseme, weight, mcu_p->time, rampin_duration );
				}
				else
				{
//					character->build_viseme_curve( viseme, mcu_p->time, curveInfo, numKeys, numKeyParams );
					character->set_viseme_curve( viseme, mcu_p->time, curveInfo, numKeys, numKeyParams );
				}
			}
		} 
		else
		if( character ) 
		{
			if( curveInfo == NULL )
			{
//				character->set_viseme( viseme, weight, mcu_p->time, rampin_duration );
				character->set_viseme_ramp( viseme, weight, mcu_p->time, rampin_duration );
			}
			else
			{
//				character->build_viseme_curve(viseme, mcu_p->time, curveInfo, numKeys, numKeyParams );
				character->set_viseme_curve( viseme, mcu_p->time, curveInfo, numKeys, numKeyParams );
			}
		}
		
		if( curveInfo ) delete [] curveInfo;
		return CMD_SUCCESS;
	} 
	else 
	if( char_cmd=="bone" ) {
		return mcu_character_bone_cmd( char_name.c_str(), args, mcu_p );
	} 
	else 
	if( char_cmd=="bonep" ) {
		return mcu_character_bone_position_cmd( char_name.c_str(), args, mcu_p );
	} 
	else 
	if( char_cmd=="remove" ) {
		return SbmCharacter::remove_from_scene( char_name.c_str() );
	} else if( char_cmd=="viewer" ) {
		int mode = args.read_int();
		if (character)
		{
			switch (mode)
			{
				case 0:
					character->scene_p->set_visibility(1,0,0,0);
					character->dMesh_p->set_visibility(0);
					break;
				case 1:
					character->scene_p->set_visibility(0,1,0,0);
					character->dMesh_p->set_visibility(0);
					break;
				case 2:
					character->scene_p->set_visibility(0,0,1,0);
					character->dMesh_p->set_visibility(0);
					break;
				case 3:
					character->scene_p->set_visibility(0,0,0,1);
					character->dMesh_p->set_visibility(0);
					break;
				case 4:
					character->scene_p->set_visibility(0,0,0,0);
					character->dMesh_p->set_visibility(1);
					break;
				default:	
					break;
			}
		}
		return CMD_SUCCESS;
	} 
	else 
	if( char_cmd=="gazefade" ) {

		string fade_cmd = args.read_token();
		bool fade_in;
		bool print_track = false;
		if( fade_cmd=="in" ) {
			fade_in = true;
		}
		else
		if( fade_cmd=="out" ) {
			fade_in = false;
		}
		else
		if( fade_cmd=="print" ) {
			print_track = true;
		}
		else	{
			return( CMD_NOT_FOUND );
		}
		float interval = args.read_float();
		if( print_track )	{
			LOG( "char '%s' gaze tracks:", character->name );
		}
		MeCtScheduler2::VecOfTrack track_vec = character->gaze_sched_p->tracks();
		int n = track_vec.size();
		for( int i = 0; i < n; i++ )	{
			MeCtScheduler2::TrackPtr t_p = track_vec[ i ];
			MeController* ct_p = t_p->animation_ct();
			MeCtGaze* gaze_p = dynamic_cast<MeCtGaze*> (ct_p);
			if( gaze_p )	{	
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
	if( char_cmd=="reholster" ) {
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				character->reholster_quickdraw( mcu_p );
			}
			return CMD_SUCCESS;
		} else {
			if ( !character ) {
				LOG("ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"%s\".", char_name.c_str());
				return CMD_FAILURE;  // ignore/out-of-domain? But it's not a standard network message.
			} else {
				return character->reholster_quickdraw( mcu_p );
			}
		}
	}
	else
	if( char_cmd == "blink" )
	{
		MeCtEyeLidRegulator *eye_reg_ct = character->eyelid_reg_ct_p;
		if( eye_reg_ct )	{
			eye_reg_ct->blink_now();
			return( CMD_SUCCESS );
		}
		return( CMD_FAILURE );
	}
	else
	if( char_cmd == "eyelid" )
	{
		MeCtEyeLidRegulator *eye_reg_ct = character->eyelid_reg_ct_p;
		if( eye_reg_ct )	{

			string eyelid_cmd  = args.read_token();
			if( eyelid_cmd.length()==0 ) {
				LOG("ERROR: Expected eyelid command.");
				return( CMD_FAILURE );
			}

			int n = args.calc_num_tokens();
			if( eyelid_cmd == "pitch" )
			{
				if( n > 0 )	{
					bool enable = args.read_int() != 0;
					eye_reg_ct->set_eyeball_tracking( enable );
				}
				else	{

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
				eye_reg_ct->set_upper_range( upper_min, upper_max );
				if( n >= 4 )	{
					float lower_min = args.read_float();
					float lower_max = args.read_float();
					eye_reg_ct->set_lower_range( lower_min, lower_max );
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
				eye_reg_ct->set_close_angle( close_angle );
				return( CMD_SUCCESS );
			}
			else
			if( eyelid_cmd == "tight" )
			{
				float upper_mag = args.read_float();
				eye_reg_ct->set_upper_tighten( upper_mag );
				if( n > 1 ) {
					float lower_mag = args.read_float();
					eye_reg_ct->set_lower_tighten( lower_mag );
				}
				return( CMD_SUCCESS );
			}
			if( eyelid_cmd == "print" )
			{
				eye_reg_ct->print();
				return( CMD_SUCCESS );
			}
			return( CMD_NOT_FOUND );
		}
		return( CMD_FAILURE );
	}
	else
	if (char_cmd == "softeyes")
	{
		if (!character)
		{
			LOG("ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"%s\".", char_name.c_str());
			return CMD_FAILURE;
		}
		MeCtEyeLid *eyelid_ct = character->eyelid_ct;
		if (!eyelid_ct)
		{
			LOG("ERROR: SbmCharacter::character_cmd_func(..): character \"%s\" has no eyelid_ct.", char_name.c_str());
			return CMD_FAILURE;
		}
		
		if (args.calc_num_tokens() == 0)
		{
			LOG( "softeyes params: %s", character->isSoftEyes() ? "ENABLED" : "DISABLED" );
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
		if (softEyesCommand == "on")
		{
			character->setSoftEyes(true);
		}
		else 
		if (softEyesCommand == "off")
		{
			character->setSoftEyes(false);
		}
		else	{
			
			float lo = args.read_float();
			float up = args.read_float();
			
			if (softEyesCommand == "weight")
			{
				eyelid_ct->set_weight( lo, up );
			}
			else 
			if (softEyesCommand == "upperlid")
			{
				eyelid_ct->set_upper_lid_range( lo, up );
			}
			else 
			if (softEyesCommand == "lowerlid")
			{
				eyelid_ct->set_lower_lid_range( lo, up );
			}
			else 
			if (softEyesCommand == "eyepitch")
			{
				eyelid_ct->set_eye_pitch_range( lo, up );
			}
			else
			{
				LOG( "SbmCharacter::character_cmd_func ERR: command '%s' not recognized", softEyesCommand.c_str());
				return CMD_NOT_FOUND;
			}
			return CMD_SUCCESS;
		}
		return CMD_SUCCESS;
	}
	return CMD_NOT_FOUND;
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


///////////////////////////////////////////////////////////////////////////
//  Private sbm_character functions

// Because I don't like c style error checking, I'm avoiding srArgBuffer::read_float
bool parse_float_or_error( float& var, const char* str, const string& var_name ) {
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	LOG("ERROR: Invalid value for %s: %s", var_name.c_str(), str);
	return false;
}

void SbmCharacter::setSoftEyes(bool val)
{
	softEyes = val;
	// disable the eyelid controller if available
	if (eyelid_ct)
	{
		eyelid_ct->set_pass_through(!val);
	}
}

bool SbmCharacter::isSoftEyes()
{
	return softEyes;
}

void SbmCharacter::setHeight(float height)
{
	_height = height;
}

float SbmCharacter::getHeight()
{
	return _height;
}
