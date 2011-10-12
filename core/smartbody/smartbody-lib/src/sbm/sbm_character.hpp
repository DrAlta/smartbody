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


#include <sk/sk_motion.h>

#include "sbm_constants.h"
#include "sbm_speech.hpp"
#include <sbm/sr_linear_curve.h>

#include <me/me_ct_scheduler2.h>
#include <sbm/me_ct_face.h>
#include <sbm/me_ct_eyelid.h>
#include <sbm/me_ct_gaze.h>
#include <sbm/me_ct_reach.hpp>
#include <sbm/me_ct_constraint.hpp>
#include <sbm/me_ct_example_body_reach.hpp>
#include <sbm/me_ct_hand.hpp>
#include <sbm/MeCtReachEngine.h>
#include <sbm/me_ct_breathing_interface.h>
#include <sbm/me_ct_breathing.h>
#include <sbm/VisemeMap.hpp>
#define MeCtSchedulerClass MeCtScheduler2

#if(1) // Use primary locomotion controller
#include "me_ct_navigation_circle.hpp"
#include "me_ct_locomotion.hpp"
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

#include <sbm/me_ct_param_animation.h>
#include <sbm/me_ct_saccade.h>
#include <sbm/me_ct_basic_locomotion.h>
#include "SteeringAgent.h"

#include <sbm/me_ct_data_receiver.h>
#include <sbm/nvbg.h>
#include <sbm/MiniBrain.h>

//#include <me/me_spline_1d.hpp>
class MeCtMotionPlayer;
class MeCtPAnimation;
class MeCtParamAnimation;
class MeCtSaccade;
class SteeringAgent;
class MeCtBasicLocomotion;
class MeCtDataReceiver;

class SbmCharacter : public SbmPawn	{

public:
	std::list<SrVec> trajectoryBuffer;
	static const int trajectoryLength = 1000;
	SteeringAgent* steeringAgent;
	int	_numSteeringGoal;
	bool _reachTarget;
	bool _lastReachStatus;

public:
	// Static Constants
	MeCtLocomotionClass* locomotion_ct;
	GeneralParamMap*   param_map;

protected:
	// reach motion database for example-based IK reaching
	MotionDataSet      reachMotionData;

	MotionDataSet      reachHandData;
	MotionDataSet      grabHandData;
	MotionDataSet      releaseHandData;

	// The implementation to be used for speech (NULL if unset) 
	SmartBody::SpeechInterface* speech_impl;
	// The voice code used by the implementation (empty string if unset) 
	std::string 			voice_code;

	SmartBody::SpeechInterface* speech_impl_backup;
	std::string 			voice_code_backup;

	// Evaluation time face data
	SkMotion*				face_neutral;

	VisemeMotionMap 		viseme_map;

	//MeCtLocomotionAnalysis* locomotion_ct_analysis;
	
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
	std::map<std::string, std::vector<std::string> > viseme_name_patch;

	// associate each joint with a physics object
	// currently, we assume the vis-geo as the geometry ( or an capsule to the child if vis-geo is not available )
	std::map<std::string, SbmPhysicsObj*> jointPhyObjMap; 	

	// reach engine map
	ReachEngineMap reachEngineMap;
	int            currentReachType;

	// Viseme Curve Info
	bool	use_viseme_curve;
	float	viseme_time_offset;
	float	viseme_sound_offset;
	float	viseme_magnitude;
	int 	viseme_channel_count;

	float	*viseme_history_arr;

	FaceDefinition* _faceDefinition;

	bool _isControllerPruning;

	Nvbg* _nvbg;
	MiniBrain* _miniBrain;
public:
	//  Methods
	SbmCharacter();
	SbmCharacter( const char * char_name );
	SbmCharacter( const char* character_name, std::string type);
	virtual ~SbmCharacter();
	
	int init( SkSkeleton* skeleton_p,
				FaceDefinition* faceDefinition,
			  GeneralParamMap* param_map,
			  const char* classType,
			  bool use_locomotion,
			  bool use_param_animation,
			  bool use_data_receiver);

	virtual int setup();

	virtual void createStandardControllers();

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
	// TODO: Arms
	MeCtSchedulerClass* constraint_sched_p;
	MeCtSchedulerClass*	reach_sched_p; // reaching
	MeCtSchedulerClass*	grab_sched_p; // grabbing (temp)
	MeCtSchedulerClass*	head_sched_p; // neck / head orientation
	MeCtSchedulerClass*	param_sched_p; // general parameters

	MeCtDataReceiver*	datareceiver_ct;
	MeCtParamAnimation* param_animation_ct;
	MeCtMotionPlayer*	motionplayer_ct;
	MeCtSaccade*		saccade_ct;
	//MeCtReachEngine*	reachEngine[2];	
	MeCtBreathing*			breathing_p;
	MeCtBasicLocomotion*	basic_locomotion_ct;

	int 	viseme_channel_start_pos;
	int 	viseme_channel_end_pos;

	void schedule_viseme_curve( 
		const char* viseme, 
		double start_time, 
		float* curve_info, int num_keys, int num_key_params, 
		float ramp_in, float ramp_out 
	);
	void schedule_viseme_trapezoid( 
		const char* viseme,
		double start_time,
		float weight,
		float duration,
		float ramp_in, 
		float ramp_out 
	);
	void schedule_viseme_blend_curve( 
		const char* viseme, 
		double start_time, 
		float weight, 
		float* curve_info, int num_keys, int num_key_params 
	);
	void schedule_viseme_blend_ramp( 
		const char* viseme, 
		double start_time, 
		float weight, 
		float rampin_duration
	);
	void forward_visemes( double curTime);
	void forward_parameters( double curTime);

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
	int parse_character_command( std::string cmd, srArgBuffer& args, mcuCBHandle *mcu_p, bool all_characters );

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

	int init_locomotion_skeleton(const char* skel_file, mcuCBHandle *mcu_p);
	void automate_locomotion(bool automate);

	//temp command process.............................
	bool is_locomotion_controller_initialized();
	bool is_locomotion_controller_enabled();
	void locomotion_reset();
	void locomotion_set_turning_speed(float radians);
	void locomotion_set_turning_mode(int mode);
	void locomotion_ik_enable(bool enable);
	MeCtLocomotionClass* get_locomotion_ct();

public:
	bool addReachMotion(int tag, SkMotion* motion);
	SkMotion* getReachMotion(int index);
	const MotionDataSet& getReachMotionDataSet() const { return reachMotionData;}

	const MotionDataSet& getGrabHandData() const { return grabHandData;}
	const MotionDataSet& getReachHandData() const { return reachHandData;}
	const MotionDataSet& getReleaseHandData() const { return releaseHandData;}
	std::map<int,MeCtReachEngine*>& getReachEngineMap() { return reachEngineMap; }
	int   getCurrentReachType() { return currentReachType; }
	void   setCurrentReachType(int type) { currentReachType = type; }

	std::map<std::string,SbmPhysicsObj*>& getJointPhyObjs() { return jointPhyObjMap; }
	void buildJointPhyObjs();
	void setJointCollider(std::string jointName, float size);
	void updateJointPhyObjs();
	void setJointPhyCollision(bool useCollision);
	void setJointCollider(std::string jointName, float len, float radius);
		
	// viseme curve related functions
	void set_viseme_curve_mode( bool mode )		{ use_viseme_curve = mode; }
	bool get_viseme_curve_mode( void ) const	{ return( use_viseme_curve ); }
	void set_viseme_time_delay( float timeDelay ) { viseme_time_offset = timeDelay; }
	float get_viseme_time_delay( void ) const	{ return( viseme_time_offset ); }
	void set_viseme_sound_delay( float timeDelay ) { viseme_sound_offset = timeDelay; }
	float get_viseme_sound_delay( void ) const	{ return( viseme_sound_offset ); }
	void set_viseme_magnitude( float magnitude ) { viseme_magnitude = magnitude; }
	float get_viseme_magnitude( void ) const		{ return( viseme_magnitude ); }

	void setSoftEyes( bool val )	{
		_soft_eyes_enabled = val;
		if( eyelid_ct )	{
			eyelid_ct->set_pass_through(!val);
		}
	}
	bool isSoftEyes( void ) const { return _soft_eyes_enabled; }
	bool isVisemePlateau( void ) const { return _visemePlateau; }
	void setVisemePlateau( bool val ) { _visemePlateau = val; }

	void setHeight( float height )	{ _height = height; }
	float getHeight( void ) 		{ return _height; }
	SrVec getFacingDirection() ;

	void setMinVisemeTime(float minTime);
	float getMinVisemeTime() const;
	void notify(DSubject* subject);

	FaceDefinition* getFaceDefinition();
	void setFaceDefinition(FaceDefinition* face);
	void updateFaceDefinition();
	void removeAllFaceChannels();

	virtual void setNvbg(Nvbg* nvbg);
	virtual Nvbg* getNvbg();

	virtual void setMiniBrain(MiniBrain* mini);
	virtual MiniBrain* getMiniBrain();

private:

	bool	_soft_eyes_enabled;
	bool	_visemePlateau;
	float	_height;
	float	_minVisemeTime;

protected:
	/*!
	 *   Modify skeleton, if necessary.
	 *
	 *   SbmPawn inserts world_offset joint above the existing root.
	 */
	virtual int init_skeleton();

	/*!
	 *  Adds a single float channel, bounded by lower and upper limits.
	 */

	// NOTE: called for shader params, bound ( 0, 2 )
	SkJoint* add_bounded_float_channel( const std::string & name, float lower, float upper, const int wo_index );


	/*!
	 *   Adds a joint of given name with XPos activated
	 *   for the channel to control an aspect of the face.
	 */

	// NOTE: called for A-units and visemes; bound ( 0, 1 )
	void add_face_channel( const std::string& name, const int wo_index );

	
	int writeSkeletonHierarchy(std::string file, double scale);
	void writeSkeletonHierarchyRecurse(SkJoint* joint, std::ofstream& ostream, double scale, int indentLevel);
	void indent(int num, std::ofstream& ostream);

	void addVisemeChannel(std::string visemeName, SkMotion* motion);
	void addVisemeChannel(std::string visemeName, std::string motionName);
	void addActionUnitChannel(int auNum, ActionUnit* au);

	void initData();

};

/////////////////////////////////////////////////////////////
#endif // SBM_CHARACTER_HPP

