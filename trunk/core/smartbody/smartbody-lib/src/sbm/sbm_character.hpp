/*
 *  sbm_character.hpp - part of SmartBody-lib
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
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Andrew n marshall, USC
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 *      Ashok Basawapatna, USC (no longer)
 */

#ifndef SBM_CHARACTER_HPP
#define SBM_CHARACTER_HPP

#include <float.h>
#include <iostream>
#include <string>
#include <set>
#include <map>

#include "bonebus.h"

#include <SK/sk_motion.h>

#include "sbm_constants.h"
#include "sbm_speech.hpp"
#include <sbm/sr_linear_curve.h>

#include <ME/me_ct_scheduler2.h>
#include <sbm/me_ct_face.h>
#include <sbm/me_ct_eyelid.h>
#include <sbm/me_ct_gaze.h>
#define MeCtSchedulerClass MeCtScheduler2

#if(1) // Use primary locomotion controller
#include "me_ct_navigation_circle.hpp"
#include "me_ct_locomotion.hpp"
#include "me_ct_locomotion_analysis.hpp"
#define  MeCtLocomotionClass MeCtLocomotion
#else
// "Ghost-walking" implementation, useful for test just the navigation code
#include "me_ct_locomotion_simple.hpp"
#define MeCtLocomotionClass MeCtLocomotionSimple
#endif

#include "sr_path_list.h"

#include "sbm_pawn.hpp"

#include <sbm/action_unit.hpp>
#include <sbm/viseme_map.hpp>
#include <sbm/general_param_setting.h>
#include <me/me_spline_1d.hpp>

class SbmCharacter : public SbmPawn	{
public:
	// Static Constants

	static const char* LOCOMOTION_ROTATION;
	
	//! Channel name for instantaneous locomotion rotation, uses YPos
	static const char* LOCOMOTION_GLOBAL_ROTATION;

	static const char* LOCOMOTION_LOCAL_ROTATION;

	static const char* LOCOMOTION_LOCAL_ROTATION_ANGLE;

	static const char* LOCOMOTION_ID;

	//! Channel name for immediate locomotion speed and trajectory, stored in the joint position channels
	static const char* LOCOMOTION_VELOCITY;
	
	//! Channel name for the body orientation target, stored in the joint position channels
	static const char* ORIENTATION_TARGET;

	MeCtLocomotionClass* locomotion_ct;

		// Face data temporary storage to pass reference into init_skeleton()
	AUMotionMap*     au_motion_map;
	VisemeMotionMap* viseme_motion_map;
	GeneralParamMap*   param_map;


protected:
	// Private Data

	// The implementation to be used for speech (NULL if unset) 
	SmartBody::SpeechInterface* speech_impl;
	// The voice code used by the implementation (empty string if unset) 
	std::string		           voice_code;

	SmartBody::SpeechInterface* speech_impl_backup;
	std::string		           voice_code_backup;

	// Evaluation time face data
	SkMotion*         face_neutral;
	AUChannelMap      au_channel_map;
	VisemeMotionMap   viseme_map;
	MeCtLocomotionAnalysis* locomotion_ct_analysis;
	
	MeCtEyeLidRegulator*	eyelid_reg_ct_p;
	MeCtFace*				face_ct;
	MeCtEyeLid*				eyelid_ct;

	// The term "viseme" in the following variables is a misnomer,
	// and may also refer to an action unit or other face shape.
	// They are all controlled by the "character .. viseme .." command.

	// Following patch can be removed after the other parts are fullfilled
	// viseme name patch
	// in case there's a mis-match between audio file&remote speech viseme name and Bonebus&SBM viseme name
	// e.g. Oh <-> oh, OW <-> oh
	// also doing this for bilateral au, e.g. mapping au_1 to au_1_left and au_1_right
	// in this way, au_1 can control both au_1_left and au_1_right
	std::map<std::string, std::vector<std::string>> viseme_name_patch;

	// Viseme Curve Info
	std::map <std::string, srLinearCurve*> visemeCurve;
	bool			use_viseme_curve;
	float			time_delay;

public:
	//  Methods
	SbmCharacter( const char * char_name );
	virtual ~SbmCharacter();
	
	int init( SkSkeleton* skeleton_p,
	          SkMotion* face_neutral,
	          AUMotionMap* fac_map,
			  VisemeMotionMap* viseme_map,
			  GeneralParamMap* param_map,
			  const char* unreal_class,
			  bool use_locomotion );

	//* Overrides SbmPawn::prune_controller_tree()
	virtual int prune_controller_tree( mcuCBHandle *mcu_p );

	//* Overrides SbmPawn::remove_from_scene()
	virtual void remove_from_scene();

	/**
	 *  Sets the character's speech implementation.
	 */
	int set_speech_impl(SmartBody::SpeechInterface *speech_impl); //set speech returns CMD_SUCCESS  

	/**
	 *  Sets the character's backup speech implementation.
	 */
	int set_speech_impl_backup(SmartBody::SpeechInterface *speech_impl); //set speech returns CMD_SUCCESS  


	/**
	 *  Gets the character's speech implementation.
	 */
	SmartBody::SpeechInterface* get_speech_impl() const; //returns speech implementation if set or NULL if not
	
	/**
	 *  Gets the character's backup speech implementation.
	 */
	SmartBody::SpeechInterface* get_speech_impl_backup() const; //returns speech implementation if set or NULL if not
	
	/**
	 *  Sets the character's voice code string.
	 */
	int set_voice_code(std::string& voice_code); //set Voice returns CMD_SUCCESS  

	/**
	 *  Sets the character's backup voice code string which will be activated if the primary voice fails.
	 */
	int set_voice_code_backup(std::string& voice_code); //set Voice returns CMD_SUCCESS  


	/**
	 *  Gets the character's voice code string.
	 */
	const std::string& get_voice_code() const; //returns voice if exists or NULL if not

	/**
	 *  Gets the character's backup voice code string.
	 */
	const std::string& get_voice_code_backup() const; //returns voice if exists or NULL if not



	// Prioritized Schedules for behaviors (known as "blocking" in manual animation)
	// TODO: Rename by body part, rather than controller type
	MeCtSchedulerClass*	posture_sched_p; // legs / stance / posture
	MeCtSchedulerClass*	motion_sched_p;  // full body motions
	MeCtSchedulerClass*	gaze_sched_p;    // back / chest / spine

//	MeCtPeriodicReplay* blink_ct_p; 
	// TODO: Arms
	// TODO: Hands
	MeCtSchedulerClass*	head_sched_p; // neck / head orientation
	MeCtSchedulerClass*	param_sched_p; // general parameters

	BoneBusCharacter * bonebusCharacter;

#define SWITCH_TO_SET_VISEME_FUNC	0
	int set_viseme( const char* viseme, float weight , double start_time, float rampin_duration, float* curve_info, int numKeys );
	void reset_viseme_bonebus(double curTime);

	void eye_blink_update( const double frame_time );

public:

	void inspect_skeleton( SkJoint* joint_p, int depth = 0 );
	void inspect_skeleton_local_transform( SkJoint* joint_p, int depth = 0 );
	void inspect_skeleton_world_transform( SkJoint* joint_p, int depth = 0 );

	int reholster_quickdraw( mcuCBHandle *mcu_p );  // HACK to initiate reholster on all QuickDraw controllers

	int print_controller_schedules();

	/** Returns true if face controller is active on this character. */
	bool is_face_controller_enabled();

	//////////////////////////////////////////
	// Static command handlers

	/**
	 *  Handles commands beginning with "char ..." or "character ...".
	 *
	 *  char <> init <skel-file>
	 *  char <> ctrl <> begin [<ease-in> [<ease-out>]]
	 *  char <> viseme <viseme_name> <weight>
	 *  char <> bone <bone_name> <w> <x> <y> <z>
	 *  char <> remove
	 */
	static int character_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	static int character_init_cmd( srArgBuffer& args, mcuCBHandle *mcu_p );

	static int character_ctrl_cmd( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Removes a character from the scene by name.
	 *  Using "*" as a character name will remove all pawns.
	 */
	static int remove_from_scene( const char* char_name );

	/**
	 *  Handles commands beginning with "set character <character id> ...".
	 */
	static int set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Handles commands beginning with "print character <character id> ...".
	 */
	static int print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	int init_locomotion_analyzer(const char* skel_file, mcuCBHandle *mcu_p);
	void automate_locomotion(bool automate);

	//temp command process.............................
	bool is_locomotion_controller_initialized();
	bool is_locomotion_controller_enabled();
	void locomotion_reset();
	void locomotion_set_turning_speed(float radians);
	void locomotion_set_turning_mode(int mode);
	void locomotion_ik_enable(bool enable);
	MeCtLocomotionClass* get_locomotion_ct();
	MeCtLocomotionAnalysis* get_locomotion_ct_analysis();

	AUChannelMap& get_au_channel_map();

public:
	// reset the buffer channel data to be zero before blending
	// hope we have cleverer way later
//	void reset_viseme_channels();

	// viseme curve related functions
	bool is_viseme_curve() const {return use_viseme_curve;}
	void set_viseme_curve(bool mode) {use_viseme_curve = mode;}
	void set_viseme_time_delay(float timeDelay) {time_delay = timeDelay;}
	float get_viseme_time_delay() {return time_delay;}

	void setSoftEyes(bool val);
	bool isSoftEyes();

	void setHeight(float height);
	float getHeight();

private:
	int viseme_channel_start_pos;
	int viseme_channel_end_pos;
	bool softEyes;
	float _height;

protected:
	/*!
	 *   Modify skeleton, if necessary.
	 *
	 *   SbmPawn inserts world_offset joint above the existing root.
	 */
	virtual int init_skeleton();

#if 0
// DEPRECATED
	/*!
	 *  Initialize the controllers that drive the viseme, AU, and face bones
	 *  Only called when the neutral face pose is defined (used by MeCtFace).
	 */
	virtual void init_face_controllers();
#endif

//	int set_world_offset_cmd( srArgBuffer& args ); // NOT DEFINED

	/*!
	 *  Adds a single float channel, bounded by lower and upper limits.
	 */

	// NOTE: called for shader params, bound ( 0, 2 )
	void add_bounded_float_channel( const std::string & name, float lower, float upper, const int wo_index );


	/*!
	 *   Adds a joint of given name with XPos activated
	 *   for the channel to control an aspect of the face.
	 */

	// NOTE: called for A-units and visemes; bound ( 0, 1 )
	void add_face_channel( const std::string& name, const int wo_index );

};

/////////////////////////////////////////////////////////////
#endif // SBM_CHARACTER_HPP

