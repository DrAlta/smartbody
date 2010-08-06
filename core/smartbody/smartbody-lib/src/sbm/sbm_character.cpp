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

#include <stdio.h>

#include <iostream>
#include <string>
#include <cstring>


#include <SK/sk_skeleton.h>
#include <ME/me_ct_adshr_envelope.hpp>
#include <ME/me_ct_blend.hpp>
#include <ME/me_ct_time_shift_warp.hpp>
#include "mcontrol_util.h"
#include "mcontrol_callbacks.h"
#include "me_utilities.hpp"
#include <ME/me_spline_1d.hpp>


const bool LOG_PRUNE_CMD_TIME                        = false;
const bool LOG_CONTROLLER_TREE_PRUNING               = false;
const bool LOG_PRUNE_TRACK_WITHOUT_BLEND_SPLIE_KNOTS = false;

const bool ENABLE_EYELID_CORRECTIVE_CT = false;

const float BLINK_SHUTTING_DURATION    = 0.05f;
const float BLINK_OPENING_DURATION     = 0.2f;
const float BLINK_MIN_REPEAT_DURATION  = 4.0f;  // how long to wait until the next blink
const float BLINK_MAX_REPEAT_DURATION  = 8.0f;  // will pick a random number between these min/max



using namespace std;

// Predeclare private functions defined below
static int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );
static inline bool parse_float_or_error( float& var, const char* str, const string& var_name );


/////////////////////////////////////////////////////////////
//  Static Data
const char* SbmCharacter::LOCOMOTION_VELOCITY = "locomotion_velocity";
const char* SbmCharacter::LOCOMOTION_ROTATION = "locomotion_rotation";
const char* SbmCharacter::LOCOMOTION_GLOBAL_ROTATION = "locomotion_global_rotation";
const char* SbmCharacter::LOCOMOTION_LOCAL_ROTATION = "locomotion_local_rotation";
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
	posture_sched_p( CreateSchedulerCt( character_name, "posture" ) ),
	motion_sched_p( CreateSchedulerCt( character_name, "motion" ) ),
	gaze_sched_p( CreateSchedulerCt( character_name, "gaze" ) ),
	locomotion_ct_analysis( NULL ),
	locomotion_ct( NULL ),
	blink_ct_p( NULL ),
	head_sched_p( CreateSchedulerCt( character_name, "head" ) ),
	param_sched_p( CreateSchedulerCt( character_name, "param" ) ),
	face_ct( NULL ),
	eyelid_ct( new MeCtEyeLid() ),
	face_neutral( NULL )
{
	posture_sched_p->ref();
	motion_sched_p->ref();
	gaze_sched_p->ref();
	head_sched_p->ref();
	param_sched_p->ref();
	eyelid_ct->ref();

	bonebusCharacter = NULL;

	eye_blink_closed = false;
	eye_blink_last_time = 0;
	eye_blink_repeat_time = 0;
}

//  Destructor
SbmCharacter::~SbmCharacter( void )	{
	posture_sched_p->unref();
	motion_sched_p->unref();
	gaze_sched_p->unref();
	if( blink_ct_p )
		blink_ct_p->unref();
	head_sched_p->unref();
	param_sched_p->unref();
	if( face_ct )
		face_ct->unref();
	eyelid_ct->unref();

	if (locomotion_ct_analysis)
		delete locomotion_ct_analysis;
	if (locomotion_ct)
		delete locomotion_ct;

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
	std::string walkForwardMotion = "Step_WalkForward";
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
		std::cout << "No " << walkForwardMotion << " animation." << std::endl;
		motionsNotLoaded = true;
	}
	if (!walking2_p)
	{
		std::cout << "No " << strafeMotion << " animation." << std::endl;
		motionsNotLoaded = true;
	}
	if (!standing_p)
	{
		std::cout << "No " << standingMotion << " animation." << std::endl;
		motionsNotLoaded = true;
	}
	if (motionsNotLoaded)
	{
		return CMD_FAILURE;
	}

	SkSkeleton* walking_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );
	SkSkeleton* standing_skeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager );

	locomotion_ct->init_skeleton(standing_skeleton, walking_skeleton);
	locomotion_ct_analysis->set_ct(locomotion_ct);

	locomotion_ct->add_locomotion_anim(walking1_p);
	locomotion_ct->add_locomotion_anim(walking2_p);
	
	locomotion_ct_analysis->init(standing_p, mcu_p->me_paths);
	locomotion_ct_analysis->add_locomotion(walking1_p);
	locomotion_ct_analysis->add_locomotion(walking2_p);
	locomotion_ct_analysis->init_blended_anim();

	return CMD_SUCCESS;
}

void SbmCharacter::automate_locomotion(bool automate)
{
	locomotion_ct->automate = automate;
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
                        const AUMotionMap* au_motion_map,
                        const VisemeMotionMap* viseme_motion_map,
						const GeneralParamMap* param_map,
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

	init_face_controllers();  // Should I pass in the viseme_motion_map and au_motion_map here so face_ct can be initialized in here?


	//if (use_locomotion) 
	{
		this->locomotion_ct_analysis = new MeCtLocomotionAnalysis();
		this->locomotion_ct =  new MeCtLocomotionClass();
		this->locomotion_ct->name("locomotion");
		locomotion_ct->ref();
	}

	// Clear pointer data no longer used after this point in initialization.
	this->viseme_motion_map = NULL;
	this->au_motion_map     = NULL;
	if( face_neutral ) {
		face_neutral->unref();
		this->face_neutral      = NULL;
	}

	posture_sched_p->init();
	motion_sched_p->init();
	if( locomotion_ct != NULL )
		locomotion_ct->init();
	gaze_sched_p->init();

	// Blink controller before head group (where visemes are controlled)
	head_sched_p->init();

	param_sched_p->init();

	// Add Prioritized Schedule Controllers to the Controller Tree
	ct_tree_p->add_controller( posture_sched_p );
	ct_tree_p->add_controller( motion_sched_p );
	ct_tree_p->add_controller( gaze_sched_p );
	ct_tree_p->add_controller( blink_ct_p );
	ct_tree_p->add_controller( head_sched_p );
	ct_tree_p->add_controller( param_sched_p );
	ct_tree_p->name( std::string(name)+"'s ct_tree" );

	// Locomotion controller
	if( locomotion_ct)
	{
		// add after the motion scheduler
		int numControllers = ct_tree_p->count_controllers();
		for (int c = 0; c < numControllers; c++)
		{
			MeController* controller = ct_tree_p->controller(c);
			if (controller == motion_sched_p)
			{
				ct_tree_p->add_controller( locomotion_ct, c);
				break;
			}
		}
	}
		

	// Face controller
	if( face_neutral ) {
		ct_tree_p->add_controller( face_ct );
#if ENABLE_EYELID_CORRECTIVE_CT
		eyelid_ct->init();
		ct_tree_p->add_controller( eyelid_ct );
#endif
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

	init_visemes_left_right_channels( "au_1", "unit1_inner_brow_raiser" );
	init_visemes_left_right_channels( "au_2", "unit2_outer_brow_raiser" );
	init_visemes_left_right_channels( "au_4", "unit4_inner_brow_lowerer" );

	init_viseme_simple( "au_5",        "unit5_upper_lid_raiser");
	init_viseme_simple( "au_6",        "unit6_eye_squint");
	init_viseme_simple( "au_7",        "unit7_lid_tightener");
	init_viseme_simple( "au_9",        "unit9_nose_wrinkle");
	init_viseme_simple( "au_10",       "unit10_upper_lip_raiser");
	init_viseme_simple( "au_12",       "unit12_smile_mouth");
	init_viseme_simple( "au_15",       "unit15_lip_corner_depressor");
	init_viseme_simple( "au_20",       "unit20_lip_stretcher");
	init_viseme_simple( "au_23",       "unit23_lip_tightener");
	init_viseme_simple( "au_25",       "unit25_lip_parser");
	init_viseme_simple( "au_26",       "unit26_jaw_drop");
	init_viseme_simple( "au_27",       "unit27_jaw_stretch_open");
	{
		//init_viseme_simple( "au_45_left",  "unit45_left_blink");
		//init_viseme_simple( "au_45_right", "unit45_right_blink");
		VisemeImplDataPtr viseme_blink = init_visemes_left_right_channels( "au_45", "unit45_blink" );
		viseme_impl_data.insert( make_pair( "blink", viseme_blink ) );  // Synomyn
	}
	init_viseme_simple( "au_38",       "unit38_nostril_dilator");
	init_viseme_simple( "au_39",       "unit39_nostril_compressor");

	VecVisemeImplData visemes;
	visemes.push_back( init_viseme_simple( "viseme_ao",   "Ao") );
	visemes.push_back( init_viseme_simple( "viseme_d",    "D") );
	visemes.push_back( init_viseme_simple( "viseme_ee",   "EE") );
	visemes.push_back( init_viseme_simple( "viseme_er",   "Er") );
	{
		VisemeImplDataPtr viseme_f = init_viseme_simple( "viseme_f", "f");
		viseme_impl_data.insert( make_pair( "F", viseme_f ) );  // Compatibility patch...
		visemes.push_back( viseme_f );
	}
	visemes.push_back( init_viseme_simple( "viseme_ih",   "Ih") );
	visemes.push_back( init_viseme_simple( "viseme_j",    "j") );
	visemes.push_back( init_viseme_simple( "viseme_kg",   "KG") );
	visemes.push_back( init_viseme_simple( "viseme_oh",   "oh") );
	{
		VisemeImplDataPtr viseme_oh = init_viseme_simple( "viseme_oh", "oh");
		viseme_impl_data.insert( make_pair( "Oh", viseme_oh ) );  // Compatibility patch...
		viseme_impl_data.insert( make_pair( "OW", viseme_oh ) );  // Compatibility patch...  TODO: Fix in RVoiceRelay lines 164 & 214 (and others?)
		visemes.push_back( viseme_oh );
	}
	visemes.push_back( init_viseme_simple( "viseme_oo",   "OO") );
	visemes.push_back( init_viseme_simple( "viseme_ng",   "NG") );
	visemes.push_back( init_viseme_simple( "viseme_r",    "R") );
	visemes.push_back( init_viseme_simple( "viseme_th",   "Th") );
	visemes.push_back( init_viseme_simple( "viseme_z",    "Z") );

	viseme_impl_data.insert( make_pair( "ALL", composite_visemes( visemes ) ) );

	VisemeImplDataPtr closed_mouth( new VisemeImplData() );  // Does nothing, but signals as recognized
	viseme_impl_data.insert( make_pair( "_", closed_mouth ) );
	viseme_impl_data.insert( make_pair( "BMP", closed_mouth ) );
	
/*
	viseme_to_channel.insert(pair<const char*, const char*>("unit1_left_inner_brow_raiser","au_1_left"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit1_right_inner_brow_raiser","au_1_right"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit2_left_outer_brow_raiser","au_2_left"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit2_right_outer_brow_raiser","au_2_right"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit4_left_brow_raiser","au_4_left"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit4_right_brow_raiser","au_4_right"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit5_upper_lid_raiser","au_5"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit6_eye_squint","au_6"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit7_lid_tightener","au_7"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit9_nose_wrinkle","au_9"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit10_upper_lip_raiser","au_10"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit12_smile_mouth","au_12"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit15_lip_corner_depressor","au_15"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit20_lip_stretcher","au_20"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit23_lip_tightener","au_23"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit25_lip_parser","au_25"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit26_jaw_drop","au_26"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit27_jaw_stretch_open","au_27"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit45_left_blink","au_45_left"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit45_right_blink","au_45_right"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit38_nostril_dilator","au_38"));
	viseme_to_channel.insert(pair<const char*, const char*>("unit39_nostril_compressor","au_39"));

	viseme_to_channel.insert(pair<const char*, const char*>("Ao","viseme_ao"));
	viseme_to_channel.insert(pair<const char*, const char*>("D","viseme_d"));
	viseme_to_channel.insert(pair<const char*, const char*>("EE","viseme_ee"));
	viseme_to_channel.insert(pair<const char*, const char*>("Er","viseme_er"));
	viseme_to_channel.insert(pair<const char*, const char*>("f","viseme_f"));
	viseme_to_channel.insert(pair<const char*, const char*>("kg","viseme_kg"));
	viseme_to_channel.insert(pair<const char*, const char*>("Ih","viseme_ih"));
	viseme_to_channel.insert(pair<const char*, const char*>("NG","viseme_ng"));
	viseme_to_channel.insert(pair<const char*, const char*>("oh","viseme_oh"));
	viseme_to_channel.insert(pair<const char*, const char*>("OO","viseme_oo"));
	viseme_to_channel.insert(pair<const char*, const char*>("R","viseme_r"));
	viseme_to_channel.insert(pair<const char*, const char*>("Th","viseme_th"));
	viseme_to_channel.insert(pair<const char*, const char*>("Z","viseme_z"));
*/

	return( CMD_SUCCESS ); 
}

SbmCharacter::VisemeImplDataPtr SbmCharacter::init_viseme_simple( const char* channel_name, const char* bonebus_name ) {
	VisemeImplDataPtr data( new VisemeImplData() );
	if( bonebus_name ) {
		data->bonebus_names.push_back( bonebus_name );
		viseme_impl_data.insert( make_pair( bonebus_name, data ) );
	}
	if( channel_name ) {
		data->channel_names.push_back( channel_name );
		viseme_impl_data.insert( make_pair( channel_name, data ) );
	}

	return data;
}

/*!
 *   Initializes a set of visemes for relate left and right components.
 *   Bonebus does not implement independent left and right variants, but the face controller often does.
 *   This method registers the independent channel implementations, as well as a unified viseme of both channels
 *   and the single bonebus name.
 */
SbmCharacter::VisemeImplDataPtr SbmCharacter::init_visemes_left_right_channels( const char* channel_name, const char* bonebus_name ) {
	VisemeImplDataPtr left_right;
	if( channel_name ) {
		int len = strlen( channel_name );
		VecVisemeImplData vec;

		string channel_left( channel_name );
		channel_left.append( "_left" );
		vec.push_back( init_viseme_simple( channel_left.c_str(), NULL ) );
		
		string channel_right( channel_name );
		channel_right.append( "_right" );
		vec.push_back( init_viseme_simple( channel_right.c_str(), NULL ) );

		left_right = composite_visemes( vec );
	} else {
		left_right.reset( new VisemeImplData() );
	}

	if( bonebus_name ) {
		left_right->bonebus_names.push_back( bonebus_name );
		viseme_impl_data.insert( make_pair( bonebus_name, left_right ) );
	}
	if( channel_name ) {
		viseme_impl_data.insert( make_pair( channel_name, left_right ) );
	}

	return left_right;
}

SbmCharacter::VisemeImplDataPtr SbmCharacter::composite_visemes( VecVisemeImplData visemes ) {
	// Temporarily store names in a set to prevent duplicates
	set<string> bonebus_names, channel_names;

	{
		VecVisemeImplData::iterator it = visemes.begin();
		VecVisemeImplData::iterator end = visemes.end();
		for( ; it != end; ++it ) {
			// Add all bonebus names
			bonebus_names.insert( (*it)->bonebus_names.begin(), (*it)->bonebus_names.end() );
			// Add all channel names
			channel_names.insert( (*it)->channel_names.begin(), (*it)->channel_names.end() );
		}
	}

	VisemeImplDataPtr data( new VisemeImplData() );
	// Add all bonebus and channel names in set into the new VisemeImplData
	data->bonebus_names.insert(
		data->bonebus_names.end(),
		bonebus_names.begin(), bonebus_names.end()
	);
	data->channel_names.insert(
		data->channel_names.end(),
		channel_names.begin(), channel_names.end()
	);

	return data;
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
		cerr << "ERROR: SbmCharacter lacks world_offset joint after SbmPawn::init_skeleton." << endl;
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
		SkJoint* id_joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
		g_angular_velocity_joint_p->name( SkJointName( LOCOMOTION_GLOBAL_ROTATION ) );
		l_angular_velocity_joint_p->name( SkJointName( LOCOMOTION_LOCAL_ROTATION ) );
		id_joint_p->name( SkJointName( LOCOMOTION_ID ) );

		// Activate positional channels, unlimited
		g_angular_velocity_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		l_angular_velocity_joint_p->pos()->limits( 1, false ); // Unlimit YPos
		id_joint_p->pos()->limits( 1, false ); // Unlimit YPos

		g_angular_velocity_joint_p->euler()->activate();
		l_angular_velocity_joint_p->euler()->activate();
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
				string name = "viseme_";
				name += id;

				// Create the Viseme control channel
				add_face_channel( name, wo_index );
				
				// Register control channel with face controller
				if( face_ct )
					face_ct->add_key( name.c_str(), motion );
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

void SbmCharacter::init_face_controllers() {
	// face_ct is actually initialized in init_skeleton to reduce 
	// because it's initialization depends on the au_motion_map and viseme_motion_map.

	{	// drive the blink via action units
		MeCtAdshrEnvelope* adshr_ct = new MeCtAdshrEnvelope();
		ostringstream adshr_name;
		adshr_name << name << "'s blink envelope";
		adshr_ct->name( adshr_name.str().c_str() );
		adshr_ct->envelope( (float)0.9, BLINK_SHUTTING_DURATION, 0, (float)0.9, 0, BLINK_OPENING_DURATION );  // I miss the Java 'f' syntax
		SkChannelArray blink_channels;
		blink_channels.add( "au_45_left", SkChannel::XPos );
		blink_channels.add( "au_45_right", SkChannel::XPos );
		adshr_ct->init( blink_channels );

		blink_ct_p = new MeCtPeriodicReplay( adshr_ct );  // TODO: Replace with aperiodic controller, with proper min/max durations
		blink_ct_p->ref();
		ostringstream replay_name;
		replay_name << name << "'s blink replay";
		blink_ct_p->name( replay_name.str().c_str() );
		blink_ct_p->init( ( BLINK_MIN_REPEAT_DURATION + BLINK_MAX_REPEAT_DURATION ) /2 );  // duration between blinks
	}
}

bool test_ct_for_pruning( MeCtScheduler2::TrackPtr track ) {
	bool prune_ok = true;

	MeController* ct = track->animation_ct();
	if( ct != NULL ) {
		MePrunePolicy* prune_policy = ct->prune_policy();
		if( prune_policy != NULL ) {
			prune_ok = prune_policy->shouldPrune( ct, track->animation_parent_ct() );

			if( LOG_CONTROLLER_TREE_PRUNING && !prune_ok )
				cout << "DEBUG: "<<ct->controller_type()<<" \""<<ct->name()<<"\" withheld from pruning by MePrunePolicy."<<endl;
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
	if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: sbm_character.cpp prune_schedule(..): Pruning schedule \""<<sched->name()<<"\":"<<endl;

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

				MeSpline1D& spline = blend_ct->blend_curve();
				MeSpline1D::Knot* knot = spline.knot_last();
				if( knot != NULL ) {
					// Has at least one knot
					MeSpline1D::domain x = knot->get_x();
					MeSpline1D::range  y = knot->get_y();

					if( LOG_CONTROLLER_TREE_PRUNING )
						cout << "\tblend_Ct \""<<blend_ct->name()<<"\": blend curve last knot: x = "<<x<<", y = "<< y <<endl;
					if( x < time ) {
						flat_blend_curve = true;
						if( y == 0 ) {
							in_use = false;
						}
					} else {
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
					}
				} else {
					if( LOG_PRUNE_TRACK_WITHOUT_BLEND_SPLIE_KNOTS ) {
						cout << "DEBUG: prune_schedule(..): sched \""<<sched->name()<<"\", anim_source \""<<anim_source->name()<<"\": blend_ct without spline knots." <<endl;
						blend_ct->print_state(1);  // Prints controller type, name, and blend curve
					}

					// A spline with no knots evaluates to 0
					in_use = false;
				}
			}

			const char* anim_ct_type = anim_source->controller_type();
			if( LOG_CONTROLLER_TREE_PRUNING )
				cout << '\t' << anim_ct_type << " \"" << anim_source->name() << "\": in_use = "<<in_use<<", flat_blend_curve = "<<flat_blend_curve<<endl;
			if( !in_use ) {
				if( LOG_CONTROLLER_TREE_PRUNING )
					cout << "\t- Pruned (not in use)!!" << endl;
			} else if( flat_blend_curve ) {  // Ignore tracks with future blend activity or are already not in use
				// Determine if the animation will be occluded by
				// (previously visited) higher priority controllers
				if( anim_ct_type == MeCtScheduler2::type_name ) {

					double time_offset = time;
					if( timing_ct ) {
						time_offset = timing_ct->time_func().eval( time );
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
				} else if( anim_ct_type == MeCtRawWriter::TYPE ) {
					const SkChannelArray& ct_channels = anim_source->controller_channels();
					vector<int> new_channels;  // list of indices to channels in use
					
					const int total_channels = ct_channels.size();
					for( int i=0; i<total_channels; ++i ) {
						int index = raw_channels.search( ct_channels.name(i), ct_channels.type(i) );
						if( index != -1 ) {
							new_channels.push_back( index );
						}
					}

					if( new_channels.empty() ) {
						in_use = false;
					} else {
						for( vector<int>::iterator it = new_channels.begin(); it!=new_channels.end(); ++it ) {
							raw_channels.add( ct_channels.name(*it), ct_channels.type(*it) );
						}
					}
				} else {
					//  TODO: Throttle warnings....
					cerr << "WARNING: Cannot prune unknown controller type \"" << anim_source->controller_type() << "\"" << endl;
				}
				if( LOG_CONTROLLER_TREE_PRUNING )
					cout << ( in_use? "\t- Not Pruned (primary ct of type)." : "\t- Pruned (occluded)!!" ) << endl;
			} else {
				if( LOG_CONTROLLER_TREE_PRUNING )
					cout << "\t- Not Pruned (future activity)." << endl;
			}
		} else {
			// No animation source
			in_use = false;

			if( LOG_CONTROLLER_TREE_PRUNING )
				cout << "\t- Pruned (no anim ct)!!" << endl;
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
		cout << "SbmCharacter \""<<name<<"\" prune_controller_tree(..) @ time "<<time<<endl;

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
		cout << endl;
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

//returns speech implementation if set or NULL if not
SmartBody::SpeechInterface* SbmCharacter::get_speech_impl() const {
	return speech_impl;
}

int SbmCharacter::set_voice_code( std::string& voice_code ) //allows you to set the voice-- made different from the init because of non Rhetoric might not have voice codes
{
	//TODO: LOOK AND SEE IF THIS VOICE EXISTS AND IF IT DOESN'T PRINT ERROR MESSAGE AND RETURN FAILURE
	this->voice_code = voice_code; //sets voice 
	return (CMD_SUCCESS);
}

const std::string& SbmCharacter::get_voice_code() const
{
	return voice_code; //if voice isn't NULL-- no error message; just returns the string
}


int SbmCharacter::set_viseme( char* viseme,
							  float weight,
							  double start_time,
							  float rampin_duration )
{
    //LOG("Recieved: SbmCharacter(\"%s\")::set_viseme( \"%s\", %f, %f )\n", name, viseme, weight, rampin_duration );

	VisemeToDataMap::const_iterator it;

	// tolower(viseme) removed until BoneBone visemes are also lowercased

	it = viseme_impl_data.find( viseme );
	if( it != viseme_impl_data.end() ) {
		VisemeImplDataPtr data( it->second );

		std::vector<string>::const_iterator it, end;

		if ( bonebusCharacter ) {
			// iterate over bonebus_names
			it  = data->bonebus_names.begin();
			end = data->bonebus_names.end();

			for( ; it!= end; ++it )
				bonebusCharacter->SetViseme( it->c_str(), weight, rampin_duration );
		}

		if ( mcuCBHandle::singleton().sbm_character_listener )
		{
			// iterate over bonebus_names
			it  = data->bonebus_names.begin();
			end = data->bonebus_names.end();

			for( ; it!= end; ++it )
				mcuCBHandle::singleton().sbm_character_listener->OnViseme( name, it->c_str(), weight, rampin_duration );
		}

		{	// Viseme/AU channel activation

			// iterate over channel_names
			it  = data->channel_names.begin();
			end = data->channel_names.end();

			for( ; it!= end; ++it ) {
				// Add controllers to drive the face channels
				const char* channel = it->c_str();

				ostringstream ct_name;
				ct_name << "Viseme \"" << viseme << "\", Channel \"" << channel << "\"";

				SkChannelArray channels;
				channels.add( SkJointName(channel), SkChannel::XPos );

				MeCtRawWriter* ct = new MeCtRawWriter();
				ct->name( ct_name.str().c_str() );
				ct->init( channels, true );
				SrBuffer<float> value;
				value.size(1);
				value[0] = (float)weight;
				ct->set_data(value);
				head_sched_p->schedule( ct, start_time, start_time + ct->controller_duration(), rampin_duration, 0 );
			}
		}
		return CMD_SUCCESS;
	} else {
		cerr << "WARNING: Unknown viseme \"" << viseme << "\" for character \"" << name << "\"." << endl;
		return CMD_FAILURE;
	}
}


void SbmCharacter::eye_blink_update( const double frame_time )
{
   // automatic blinking routine using bonebus viseme commands
   mcuCBHandle& mcu = mcuCBHandle::singleton();

   if ( !eye_blink_closed )
   {
      if ( frame_time - eye_blink_last_time > eye_blink_repeat_time )
      {
         // close the eyes
         if ( bonebusCharacter )
         {
            bonebusCharacter->SetViseme( "blink", 0.9f, BLINK_SHUTTING_DURATION );
         }

         if ( mcu.sbm_character_listener )
         {
            mcu.sbm_character_listener->OnViseme( name, string( "blink" ), 0.9f, BLINK_SHUTTING_DURATION );
         }

         eye_blink_last_time = frame_time;
         eye_blink_closed = true;
      }
   }
   else
   {
      if ( frame_time - eye_blink_last_time > BLINK_SHUTTING_DURATION )
      {
         // open the eyes
         if ( bonebusCharacter )
         {
            bonebusCharacter->SetViseme( "blink", 0, BLINK_OPENING_DURATION );
         }

         if ( mcu.sbm_character_listener )
         {
            mcu.sbm_character_listener->OnViseme( name, "blink", 0, BLINK_OPENING_DURATION );
         }

         eye_blink_last_time = frame_time;
         eye_blink_closed = false;

         // compute when to close them again
         double fraction = (double)rand() / (double)RAND_MAX;
         eye_blink_repeat_time = ( fraction * ( BLINK_MAX_REPEAT_DURATION - BLINK_MIN_REPEAT_DURATION ) ) + BLINK_MIN_REPEAT_DURATION;
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
		cout << "Character " << name << "'s schedule:" << endl;
		cout << "POSTURE Schedule:" << endl;
		posture_sched_p->print_state( 0 );
		cout << "MOTION Schedule:" << endl;
		motion_sched_p->print_state( 0 );
		cout << "GAZE Schedule:" << endl;
		gaze_sched_p->print_state( 0 );
		cout << "HEAD Schedule:" << endl;
		head_sched_p->print_state( 0 );
		// Print Face?

		return CMD_SUCCESS;
}

bool SbmCharacter::is_face_controller_enabled() {
	return (face_ct!=NULL && face_ct->context()!=NULL);
}

set<string> SbmCharacter::get_face_names() {
	set<string> names;

	bool using_facebone = is_face_controller_enabled();

	VisemeToDataMap::const_iterator vid_it  = viseme_impl_data.begin();
	VisemeToDataMap::const_iterator vid_end = viseme_impl_data.end();
	for( ; vid_it!=vid_end; ++vid_it ) {
		VisemeImplDataPtr data = vid_it->second;

		if( using_facebone ) {
			// Valid if at least one input channel exists 
			if( !data->channel_names.empty() )
				names.insert( vid_it->first );
		} else {
			// Valid if at least one bonebus name exists
			if( !data->bonebus_names.empty() )
				names.insert( vid_it->first );
		}
	}

	return names;
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
					MeSpline1D& spline = blend_ct->blend_curve();
					// TODO: Don't assume we're starting at 1, may already be less than and already blending out.
					spline.make_cusp( blend_out_start,1,  0,1, blend_spline_tanget,1 );
					MeSpline1D::Knot* knot = spline.make_cusp( blend_out_end,  0,  blend_spline_tanget,1, 0,1 );

					// TODO: delete following knots

					if( blend_out_dur > max_blend_dur )
						max_blend_dur = blend_out_end;
				}
			}
		}

		++it;
	}

	if( !found_quickdraw ) {
		cout << "WARNING: Character \""<<name<<"\" reholster(): No quickdraw controller found." << endl;
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
		cerr << "ERROR: Expected character name." << endl;
		return CMD_FAILURE;
	}

	string char_cmd  = args.read_token();
	if( char_cmd.length()==0 ) {
		cerr << "ERROR: Expected character command." << endl;
		return CMD_FAILURE;
	}

	bool all_characters = false;
	SbmCharacter* character = NULL;
	if( char_name=="*" ) {
		all_characters = true;
	} else {
		character = mcu_p->character_map.lookup( char_name.c_str() );
	}

	if( char_cmd=="param") {
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
					std::cout << c << " ";
					SkJoint* joint = channels.joint(c);
					if (joint)
					{
						std::cout << joint->name().get_string() << " ";
					}
					SkChannel& channel = channels[c];
					int channelSize = channel.size();
					// get the channel index
					int channelIndex = controllerTree->toBufferIndex(c);
					std::cout << channelIndex << " (" << channelSize << ") ";
					std::cout << std::endl;

				}
			}
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
					cerr << "ERROR: Failed to prune controller tree of character \""<<character->name<<"\"."<<endl;
					result = CMD_FAILURE;
				}
			}
		} else if( character ) {
			int result = character->prune_controller_tree( mcu_p );
			if( result != CMD_SUCCESS ) {
				cerr << "ERROR: Failed to prune controller tree of character \""<<char_name<<"\"."<<endl;
			}
		} else {
			cerr<<"ERROR: Unknown character \""<<char_name<<"\" for prune command."<<endl;
			result = CMD_FAILURE;
		}
		return result;
	}
	else 
	if( char_cmd=="viseme" ) {
        char * viseme = args.read_token();
        float  weight = args.read_float();
		float  rampin_duration = args.read_float();
		 
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				character->set_viseme( viseme, weight, mcu_p->time, rampin_duration );
			}
			return CMD_SUCCESS;
		} else {
			if ( !character ) {
				cerr << "ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"" << char_name << "\"." << endl;
				return CMD_FAILURE;  // ignore/out-of-domain? But it's not a standard network message.
			} else {
				return character->set_viseme( viseme, weight, mcu_p->time, rampin_duration );
			}
		}
	} else if( char_cmd=="bone" ) {
		return mcu_character_bone_cmd( char_name.c_str(), args, mcu_p );
	} else if( char_cmd=="bonep" ) {
		return mcu_character_bone_position_cmd( char_name.c_str(), args, mcu_p );
	} else if( char_cmd=="remove" ) {
		return SbmCharacter::remove_from_scene( char_name.c_str() );
	} else if( char_cmd=="reholster" ) {
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				character->reholster_quickdraw( mcu_p );
			}
			return CMD_SUCCESS;
		} else {
			if ( !character ) {
				cerr << "ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"" << char_name << "\"." << endl;
				return CMD_FAILURE;  // ignore/out-of-domain? But it's not a standard network message.
			} else {
				return character->reholster_quickdraw( mcu_p );
			}
		}
	} else {
		return CMD_NOT_FOUND;
	}
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
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Missing character id." << endl;
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
	if( character==NULL ) {
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Unknown character \""<<character_id<<"\" to set." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Missing attribute to set." << endl;
		return CMD_FAILURE;
	}

	//  voice_code and voice-code are backward compatible patches
	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		return set_voice_cmd_func( character, args, mcu_p );
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
		cout << "Unset " << character->name << "'s voice." << endl;
	} else if( _strcmpi( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			cerr << "ERROR: Expected remote voice id." << endl;
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_rvoice() );
		character->set_voice_code( string( voice_id ) );
	} else if( _strcmpi( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			cerr << "ERROR: Expected audiofile voice path." << endl;
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else if( _strcmpi( impl_id, "text" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			cerr << "ERROR: Expected id." << endl;
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_text() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else {
		cerr << "ERROR: Unknown speech implementation \"" << impl_id << "\"." << endl;
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

int SbmCharacter::print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Missing character id." << endl;
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
	if( character==NULL ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Unknown character \""<<character<<"\"." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Missing attribute to print." << endl;
		return CMD_FAILURE;
	}

	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		//  Command: print character <character id> voice_code
		//  Print out the character's voice_id
		cout << "character " << character_id << "'s voice_code: " << character->get_voice_code() << endl;
		return CMD_SUCCESS;
	} else if( attribute=="schedule" ) {
		return character->print_controller_schedules();
	} else if( attribute=="face" ) {
		string sub_attribute = args.read_token();
		if( sub_attribute=="names" ) {
			ostringstream output;
			if( character->is_face_controller_enabled() ) {
				output << "Using bone based face control:" << endl;
			} else {
				output << "Using renderer face control via BoneBus (implementation of the following will depend upon the render):" << endl;
			}

			set<string> names = character->get_face_names();

			set<string>::iterator it = names.begin();
			set<string>::iterator end = names.end();
			for( ; it!=end; ++it )
				output << '\t' << *it << endl;

			cout << output.str();
			return CMD_SUCCESS;
		} else if( sub_attribute == "channels" ) {
			// TODO: print list of face activating channels (and associated visemes?)
			cout << "Pending implementation" << endl;
			return CMD_SUCCESS;
		} else {
			cerr << "ERROR: Expected viseme subattribute \"names\".  Found \""<<sub_attribute<<"\"." << endl;
			return CMD_FAILURE;
		}
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
	cerr << "ERROR: Invalid value for " << var_name << ": " << str << endl;
	return false;
}
