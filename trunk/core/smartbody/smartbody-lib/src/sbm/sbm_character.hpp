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

#include "bonebus.h"

#include <SK/sk_motion.h>

#include "sbm_constants.h"
#include "sbm_speech.hpp"

#include <ME/me_ct_scheduler2.h>
#include <sbm/me_ct_face.h>
#include <sbm/me_ct_eyelid.h>
#define MeCtSchedulerClass MeCtScheduler2

#include "sr_path_list.h"

#include "sbm_pawn.hpp"


#include <sbm/action_unit.hpp>
#include <sbm/viseme_map.hpp>



class SbmCharacter : public SbmPawn	{
public:
	// Static Constants

	//! Channel name for immediate locomotion speed and trajectory, stored in the joint position channels
	static const char* LOCOMOTION_VELOCITY;
	
	//! Channel name for the body orientation target, stored in the joint position channels
	static const char* ORIENTATION_TARGET;

protected:
	// Private Data

	/** The implementation to be used for speech (NULL if unset) */
	SmartBody::SpeechInterface *speech_impl;
	/** The voice code used by the implementation (empty string if unset) */
	std::string		           voice_code;

	// Face data temporary storage to pass reference into init_skeleton()
	const AUMotionMap*     au_motion_map;
	const VisemeMotionMap* viseme_motion_map;

	// Evaluation time face data
	SkMotion*         face_neutral;
	AUChannelMap      au_channel_map;
	VisemeMotionMap   viseme_map;
	MeCtFace*         face_ct;
	MeCtEyeLid*       eyelid_ct;

	// The term "viseme" in the following variables is a misnomer,
	// and may also refer to an action unit or other face shape.
	// They are all controlled by the "character .. viseme .." command.

	// Strings used to control a viseme.
	struct VisemeImplData {
		// Names used by the BoneBus code (renderer-side implementation)
		std::vector<std::string> bonebus_names;
		// Channel names used by Motion Engine controllers (i.e., -facebone mode)
		std::vector<std::string> channel_names;
	};
	typedef boost::shared_ptr<VisemeImplData> VisemeImplDataPtr;
	// Mapping of viseme name to its implementation data. 
	typedef std::vector<VisemeImplDataPtr> VecVisemeImplData;
	typedef std::map<std::string,VisemeImplDataPtr> VisemeToDataMap;
	VisemeToDataMap viseme_impl_data;

public:
	//  Methods
	SbmCharacter( const char * char_name );
	virtual ~SbmCharacter();
	
	int init( SkSkeleton* skeleton_p,
	          SkMotion* face_neutral,
	          const AUMotionMap* fac_map,
			  const VisemeMotionMap* viseme_map,
			  const char* unreal_class );

	//  Convience method
	int init( SkSkeleton* skeleton_p,
			  const char* unreal_class )
	{	init( skeleton_p, NULL, NULL, NULL, unreal_class );
	}

	//* Overrides SbmPawn::prune_controller_tree()
	virtual int prune_controller_tree( mcuCBHandle *mcu_p );

	//* Overrides SbmPawn::remove_from_scene()
	virtual void remove_from_scene();

	/**
	 *  Sets the character's speech implementation.
	 */
	int set_speech_impl(SmartBody::SpeechInterface *speech_impl); //set speech returns CMD_SUCCESS  

	/**
	 *  Gets the character's speech implementation.
	 */
	SmartBody::SpeechInterface* get_speech_impl() const; //returns speech implementation if set or NULL if not
	
	/**
	 *  Sets the character's voice code string.
	 */
	int set_voice_code(std::string& voice_code); //set Voice returns CMD_SUCCESS  

	/**
	 *  Gets the character's voice code string.
	 */
	const std::string& get_voice_code() const; //returns voice if exists or NULL if not

	// Prioritized Schedules for behaviors (known as "blocking" in manual animation)
	// TODO: Rename by body part, rather than controller type
	MeCtSchedulerClass*	posture_sched_p; // legs / stance / posture
	MeCtSchedulerClass*	motion_sched_p;  // full body motions
	MeCtSchedulerClass*	gaze_sched_p;    // back / chest / spine
	// TODO: Arms
	// TODO: Hands
	MeCtSchedulerClass*	head_sched_p; // neck / head orientation

	BoneBusCharacter * bonebusCharacter;
	
	int set_viseme( char* viseme, float weight , double start_time, float rampin_duration );

	bool   eye_blink_closed;
	double eye_blink_last_time;
	double eye_blink_repeat_time;

	void eye_blink_update( const double frame_time );

public:

	void inspect_skeleton( SkJoint* joint_p, int depth = 0 );
	void inspect_skeleton_local_transform( SkJoint* joint_p, int depth = 0 );
	void inspect_skeleton_world_transform( SkJoint* joint_p, int depth = 0 );

	int reholster_quickdraw( mcuCBHandle *mcu_p );  // HACK to initiate reholster on all QuickDraw controllers


	int print_controller_schedules();


	/** Returns true if face controller is active on this character. */
	bool is_face_controller_enabled();

	/** Return a list of named faceposes implemented in this character. */
	std::set<std::string> get_face_names();


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

protected:
	/*!
	 *   Modify skeleton, if necessary.
	 *
	 *   SbmPawn inserts world_offset joint above the existing root.
	 */
	virtual int init_skeleton();

	/*!
	 *   Initializes a viseme that is implemented by one channel and/or one bonebus command.
	 *   Either parameter may be NULL.
	 */
	VisemeImplDataPtr init_viseme_simple( const char* channel_name, const char* bonebus_name );

	/*!
	 *   Initializes a set of visemes for relate left and right components.
	 *   Bonebus does not implement independent left and right variants, but the face controller often does.
	 *   This method registers the independent channel implementations, as well as a unified viseme of both channels
	 *   and the single bonebus name.
	 */
	VisemeImplDataPtr init_visemes_left_right_channels( const char* channel_name, const char* bonebus_name );

	/*!
	 */
	VisemeImplDataPtr composite_visemes( std::vector<VisemeImplDataPtr> visemes );

	int set_world_offset_cmd( srArgBuffer& args );

	/*!
	 *   Adds a joint of given name with XPos activated
	 *   for the channel to control an aspect of the face.
	 */
	void add_face_channel( const std::string& name, const int wo_index );
};

/////////////////////////////////////////////////////////////
#endif // SBM_CHARACTER_HPP

