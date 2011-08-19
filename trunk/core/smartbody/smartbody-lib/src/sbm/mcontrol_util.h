/*
 *  mcontrol_util.h - part of SmartBody-lib
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
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 *      Andrew n marshall, USC
 *      Ashok Basawapatna, USC (no longer)
 *      Eric Forbell, USC
 *      Thomas Amundsen, USC
 */
#ifndef MCONTROL_UTIL_H
#define MCONTROL_UTIL_H
//  Declare classes defined by this file
//  (prevents include recursion)
class mcuCBHandle;

#include <map>

#ifdef __ANDROID__
#define LINK_VHMSG_CLIENT		(0)
#define USE_WSP 1
#else
#define LINK_VHMSG_CLIENT		(1)
#define USE_WSP 1
#endif


#if LINK_VHMSG_CLIENT
#include "vhmsg-tt.h"
#endif

#include "bonebus.h"

#include <sbm/GenericViewer.h>
#include <sr/sr_viewer.h>
#include <sr/sr_camera.h>
#include <me/me_ct_pose.h>
#include <me/me_ct_motion.h>
#include <me/me_ct_lifecycle_test.hpp>
#include "me_ct_step_turn.h"
#include "me_ct_quick_draw.h"
#include "me_ct_gaze.h"
#include "me_ct_tether.h"
#include "me_ct_eyelid.h"
#include "me_ct_data_driven_reach.hpp"

#include "sbm_constants.h"

#include <me/me_ct_scheduler2.h>
#include <sbm/me_ct_motion_player.h>

#include "sr_hash_map.h"
#include "sr_cmd_map.h"
#include "sr_cmd_seq.h"
#include "sr_path_list.h"
#include "sbm_pawn.hpp"
#include "sbm_character.hpp"
#include "remote_speech.h"
#include "text_speech.h" // [BMLR]
#include "sbm_speech_audiofile.hpp"
#include "me_ct_examples.h"
#include "me_ct_lilt_try.h"
#include "time_regulator.h"
#include "time_profiler.h"
#include "Heightfield.h"

#include "joint_logger.hpp"
#include "ResourceManager.h"
#include "sbm/Event.h"
#include <sbm/action_unit.hpp>
#include <sbm/viseme_map.hpp>
#include <sbm/general_param_setting.h>
#include <sbm/SBScene.h>
#include "sbm/BoneMap.h"
#include <sbm/nvbg.h>

#include <me/me_ct_interpolator.h>
#include <sbm/me_ct_param_animation_utilities.h>
#include <sbm/me_ct_param_animation_data.h>

#include <sbm/SteerSuiteEngineDriver.h>
#include <sbm/Physics/SbmPhysicsSim.h>
#include <sbm/KinectProcessor.h>

#include BML_PROCESSOR_INCLUDE


#if USE_WSP
namespace WSP
{
    class Manager;
};
#endif


//////////////////////////////////////////////////////////////////

// This class is meant for listening to specific events that could be handled externally from smartbody
// Currently being used by smartbody-dll
class SBMCharacterListener
{
   public:
      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass ) {}
      virtual void OnCharacterDelete( const std::string & name ) {}
	  virtual void OnCharacterUpdate( const std::string & name, const std::string & objectClass ) {}
	  virtual void OnCharacterChanged( const std::string& name ) {}
      virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime ) {}
};

class CameraTrack
{
	public:
		SkJoint* joint;
		SrVec jointToCamera;
		SrVec targetToCamera;
		double yPos;
		double threshold;
};

class FaceMotion
{
	public:
		SkMotion* face_neutral_p;
	
		FaceMotion() { face_neutral_p = NULL; }
};

typedef std::map<std::string, FaceMotion*> FaceMotionMap;
typedef std::map<std::string, SkPosture*> SkPostureMap;
typedef std::map<std::string, SkMotion*> SkMotionMap;
typedef std::map<std::string, SkSkeleton*> SkSkeletonMap;


// Motion Controller Utility Callback Handle (Yes, seriously.)
class mcuCBHandle {
	protected:
		// Data
		remote_speech				_speech_rvoice;
		SmartBody::AudioFileSpeech	_speech_audiofile;
		text_speech					_speech_text; // [BMLR]
		unsigned int				queued_cmds;

	public:
		enum LocomotionType {Basic, Example, Procedural};
		// Data
		bool		loop;
		bool		vhmsg_enabled;
		bool		net_bone_updates;
		bool		net_world_offset_updates;
		bool		net_face_bones;
		bool		use_locomotion;
		bool		use_param_animation;
		bool		use_data_receiver;
		bool        updatePhysics;
		bool		sendPawnUpdates; // if true, sends the pawn information over bonebus in the same wasy as the characters
		bool        resourceDataChanged;
		const char* net_host;
		bonebus::BoneBusClient bonebus;
		SBMCharacterListener * sbm_character_listener;   // only one listener possible, must be set manually
		std::string speech_audiofile_base_path;
		std::string process_id;
		bool		play_internal_audio;	
		SBScene     scene;

		
		
		// scale factor (used for SmartBody to handle unit convert, both sk and skm)
		double		skScale;
		double		skmScale;
		double      physicsTime;

		KinectProcessor*							kinectProcessor;

		std::vector<PAStateData*>					param_anim_states;
		std::vector<PATransitionData*>				param_anim_transitions;
		LocomotionType								locomotion_type;

		TimeRegulator	*internal_timer_p;
		TimeRegulator	*external_timer_p;
		TimeRegulator	*timer_p;
		double			time; // AKA sim_time
		double			time_dt;

		TimeIntervalProfiler	*internal_profiler_p;
		TimeIntervalProfiler	*external_profiler_p;
		TimeIntervalProfiler	*profiler_p;

		bool		delay_behaviors;
		int			snapshot_counter;
		bool		use_python;

		SrViewerFactory *viewer_factory;
		SrViewer	*viewer_p;
		GenericViewer	*bmlviewer_p;
		GenericViewer	*panimationviewer_p;
		GenericViewer	*channelbufferviewer_p;
		GenericViewer   *resourceViewer_p;
		GenericViewer   *velocityViewer_p;
		GenericViewer   *faceViewer_p;

		GenericViewerFactory *bmlviewer_factory;
		GenericViewerFactory *panimationviewer_factory;
		GenericViewerFactory *channelbufferviewer_factory;
		GenericViewerFactory *commandviewer_factory;
		GenericViewerFactory *resourceViewerFactory;
		GenericViewerFactory *velocityViewerFactory;
		GenericViewerFactory *faceViewerFactory;
		
		SrCamera	*camera_p;
		SrSnGroup	*root_group_p;
		
		Heightfield *height_field_p;
		
		srPathList	seq_paths;
		srPathList	me_paths;
		srPathList	audio_paths;
		srPathList	mesh_paths;

		std::string media_path;

		/** Character id for test commands to use when required but not specified. */
		std::string test_character_default;
		/** Character id for test commands to use as recipient when required but not specified. */
		std::string test_recipient_default;

		srCmdMap <mcuCBHandle>		cmd_map;
		srCmdMap <mcuCBHandle>		set_cmd_map;
		srCmdMap <mcuCBHandle>		print_cmd_map;
		srCmdMap <mcuCBHandle>		test_cmd_map;

		srHashMap <srCmdSeq>		pending_seq_map;
		srHashMap <srCmdSeq>		active_seq_map;

		std::map<std::string, FaceDefinition*> face_map;
		std::map<std::string, SkPosture*> pose_map;
		std::map<std::string, SkMotion*> motion_map;
		std::map<std::string, SkSkeleton*> skeleton_map;
		std::map<std::string, BoneMap*> boneMaps;

		GeneralParamMap				param_map;			// map that contains the information of shader parameters

		srHashMap <MeCtPose>		pose_ctrl_map;
		srHashMap <MeCtMotion>		motion_ctrl_map;
		srHashMap <MeCtStepTurn>	stepturn_ctrl_map;
		srHashMap <MeCtQuickDraw>	quickdraw_ctrl_map;
		srHashMap <MeCtGaze>		gaze_ctrl_map;
		srHashMap <MeCtSimpleNod>	snod_ctrl_map;
		srHashMap <MeCtAnkleLilt>	lilt_ctrl_map;
		srHashMap <MeCtEyeLid>		eyelid_ctrl_map;
		srHashMap <MeCtScheduler2>	sched_ctrl_map;
		
		srHashMap <MeController>	controller_map;


		std::string getValidName(const std::string& name);
		int registerCharacter(SbmCharacter* character);
		int unregisterCharacter(SbmCharacter* character);

		std::map<std::string, SbmPawn*>& getPawnMap();
		bool addPawn(SbmPawn* pawn);
		void removePawn(std::string name);
		SbmPawn* getPawn(std::string name);
		int getNumPawns();

		std::map<std::string, SbmCharacter*>& getCharacterMap();
		bool addCharacter(SbmCharacter* character);
		void removeCharacter(std::string name);
		SbmCharacter* getCharacter(std::string name);
		int getNumCharacters();

		SkMotion* getMotion(std::string motionName);

protected:
		std::map<std::string, SbmPawn*>	pawn_map;
		std::map<std::string, SbmCharacter*> character_map;
		std::map<std::string, Nvbg*> nvbgMap;
public:

		BML_PROCESSOR				bml_processor;

#if USE_WSP
		WSP::Manager*				theWSP;
#endif

		joint_logger::EvaluationLogger* logger_p;
		ResourceManager*			resource_manager;
		std::vector<CameraTrack*>	cameraTracking;

		SteerSuiteEngineDriver		steerEngine;
		SbmPhysicsSim*              physicsEngine;

	private:
		// Constant
		static mcuCBHandle* _singleton;

        bool _interactive;
		std::vector<MeController*> _defaultControllers;

		//  Constructor
		mcuCBHandle( void );
		virtual ~mcuCBHandle( void );
		void clear();

		// Private access prevents calls to the following
		mcuCBHandle( mcuCBHandle& );
		mcuCBHandle& operator= (const mcuCBHandle&);

	public:
		static mcuCBHandle& singleton() {
			//LOG("Begin Singleton\n");
			//if (!_singleton)
			//	LOG("Singleton is NULL\n");
			if( !_singleton )
				_singleton = new mcuCBHandle();
			//LOG("End Singleton\n");
			return *_singleton;
		}
		static void destroy_singleton() {
			if( _singleton )
				delete _singleton;
			_singleton = NULL;
		}		

		void reset();

		void register_profiler( TimeIntervalProfiler& time_prof )	{
			external_profiler_p = &( time_prof );
			profiler_p = external_profiler_p;
		}
		void switch_internal_profiler( void )	{
			if( internal_profiler_p == NULL ) internal_profiler_p = new TimeIntervalProfiler;
			profiler_p = internal_profiler_p;
		}
		void mark( const char* group_name, int level, const char* label )	{
			if( profiler_p ) profiler_p->mark( group_name, level, label );
		}
		int mark( const char* group_name )	{
			if( profiler_p ) return( profiler_p->mark( group_name ) );
			return( 0 );
		}

		void register_timer( TimeRegulator& time_reg )	{
			external_timer_p = &( time_reg );
			timer_p = external_timer_p;
		}
		void switch_internal_timer( void )	{
			if( internal_timer_p == NULL ) internal_timer_p = new TimeRegulator;
			timer_p = internal_timer_p;
		}
		void update_profiler( double in_time = -1.0 )	{
			if( profiler_p )	{
				profiler_p->update();
			}
		}
		bool update_timer( double in_time = -1.0 )	{
			
			if( timer_p )	{
				bool ret = timer_p->update( in_time );
				time = timer_p->get_time();
				time_dt = timer_p->get_dt();
				return( ret );
			}
			double prev = time;
			time = in_time;
			time_dt = time - prev;
			return( true );
		}

		static std::string cmdl_tab_callback( std::string str );

		int open_viewer( int width, int height, int px, int py );
		void close_viewer( void );
		int open_bml_viewer( int width, int height, int px, int py );
		void close_bml_viewer( void );
		int open_panimation_viewer( int width, int height, int px, int py );
		void close_panimation_viewer( void );
		int open_channelbuffer_viewer( int width, int height, int px, int py );
		void close_channelbuffer_viewer( void );
		int openResourceViewer( int width, int height, int px, int py );
		void closeResourceViewer( void );
		int openVelocityViewer( int width, int height, int px, int py );
		void closeVelocityViewer( void );
		int openFaceViewer( int width, int height, int px, int py );
		void closeFaceViewer( void );


		int add_scene( SrSnGroup *scene_p );
		int remove_scene( SrSnGroup *scene_p );
		void render()	{ if( viewer_p ) { viewer_p->render(); } }
		
		void render_terrain( int renderMode ) {
			if( height_field_p )	{
				height_field_p->render(renderMode);
			}
		}
		float query_terrain( float x, float z, float *normal_p )	{
			if( height_field_p )	{
				return( height_field_p->get_elevation( x, z, normal_p ) );
			}
			if( normal_p )	{
				normal_p[ 0 ] = 0.0;
				normal_p[ 1 ] = 1.0;
				normal_p[ 2 ] = 0.0;
			}
			return( 0.0 );
		}
		
		void update( void );
		int insert( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp, char* description = NULL )	{
			return( cmd_map.insert( key, fp ) );
		}

		int insert_set_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( set_cmd_map.insert( key, fp ) );
		}

		int insert_print_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( print_cmd_map.insert( key, fp ) );
		}

		int insert_test_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( test_cmd_map.insert( key, fp ) );
		}

		void set_net_host( const char * net_host );
		void set_process_id( const char * process_id );

		int vhmsg_send( const char *op, const char* message );

		int vhmsg_send( const char* message );

		int load_motions( const char* pathname, bool recursive );
		int load_skeletons( const char* pathname, bool recursive );
		int load_poses( const char* pathname, bool recursive );


		MeController* lookup_ctrl( const std::string& ctrl_name, const char* print_error_prefix=NULL );

		srCmdSeq* lookup_seq( const char* );

		SkMotion* lookUpMotion(const char* motionName);

		PAStateData* lookUpPAState(std::string stateName);

		SkMotion* addMirrorMotion(SkMotion* motion);
		void addPAState(PAStateData* state);
		PATransitionData* lookUpPATransition(std::string fromStateName, std::string toStateName);
		void addPATransition(PATransitionData* transition);
		bool checkExamples();
		void setPhysicsEngine(bool start);


		int execute( const char *key, srArgBuffer& args ) { 
			std::stringstream strstr;
			strstr << key << " " << args.peek_string();
			CmdResource* resource = new CmdResource();
			resource->setChildrenLimit(resource_manager->getLimit());	// assuming the limit of total resources( path, motion, file, command) is the same with the limit of children ( command resource only) number
			resource->setCommand(strstr.str());
			resource->setTime(time);
			resource_manager->addCommandResource(resource);

			int ret = ( cmd_map.execute( key, args, this ) );
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret; 
		}

		int execute( const char *key, char* strArgs ) {
			std::stringstream strstr;
			strstr << key << " " << strArgs;
			CmdResource* resource = new CmdResource();
			resource->setChildrenLimit(resource_manager->getLimit());	// assuming the limit of total resources( path, motion, file, command) is the same with the limit of children ( command resource only) number
			resource->setCommand(strstr.str());
			resource->setTime(time);
			resource_manager->addCommandResource(resource);

            srArgBuffer args( strArgs );
			
			int ret = ( cmd_map.execute( key, args, this ) );
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret; 
		}

		int execute( char *cmd ) { 
			CmdResource* resource = new CmdResource();
			resource->setChildrenLimit(resource_manager->getLimit());	// assuming the limit of total resources( path, motion, file, command) is the same with the limit of children ( command resource only) number						
			resource->setCommand(cmd);
			resource->setTime(time);
			resource_manager->addCommandResource(resource);

			// check to see if this is a sequence command
			// if so, save the command id
			std::string checkCmd = cmd;
			size_t startpos = checkCmd.find_first_not_of(" \t");
			if( std::string::npos != startpos )
				checkCmd = checkCmd.substr( startpos );
			unsigned int seqPos = checkCmd.find("seq");
			if (seqPos == 0)
			{
				std::string remainderCmd = checkCmd.substr(3, checkCmd.size() - 3);
				size_t startpos = remainderCmd.find_first_not_of(" \t");
				if( std::string::npos != startpos )
					remainderCmd = remainderCmd.substr( startpos );
				size_t endpos = remainderCmd.find_first_of(" \t");
				remainderCmd = remainderCmd.substr(0, endpos);
				resource->setId(remainderCmd);
			}

			int ret = ( cmd_map.execute( cmd, this ) ); 
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret;
		}

		int execute_seq( srCmdSeq *seq );
		int execute_seq( srCmdSeq *seq, const char* seq_name );
		int execute_seq_chain( const std::vector<std::string>& seq_names, const char* error_prefix = NULL );

		//  Schedule command in some seconds
		int execute_later( const char* command, float seconds );

		//  Queue command for next frame
		int execute_later( const char* command ) { 
			return( execute_later( command, 0 ) ); 
		}

		int executePython(const char* command);
		int executePythonFile(const char* filename);


		int abort_seq( const char* command );
		int delete_seq( const char* command );

		remote_speech* speech_rvoice() { return &_speech_rvoice; }
		SmartBody::AudioFileSpeech* speech_audiofile() { return &_speech_audiofile; }
		text_speech* speech_text() { return &_speech_text; } // [BMLR]

		void NetworkSendSkeleton( bonebus::BoneBusCharacter * character, SkSkeleton * skeleton, GeneralParamMap * param_map );

		void register_viewer_factory(SrViewerFactory* factory) { 
				if (viewer_factory != NULL) delete viewer_factory;
				viewer_factory = factory;
		}
		void register_bmlviewer_factory(GenericViewerFactory* factory) { 
				if (bmlviewer_factory != NULL) delete bmlviewer_factory;
				bmlviewer_factory = factory;
		}
		void register_panimationviewer_factory(GenericViewerFactory* factory) { 
				if (panimationviewer_factory != NULL) delete panimationviewer_factory;
				panimationviewer_factory = factory;
		}
		void register_channelbufferviewer_factory(GenericViewerFactory* factory) { 
				if (channelbufferviewer_factory != NULL) delete channelbufferviewer_factory;
				channelbufferviewer_factory = factory;
		}

		void register_commandviewer_factory(GenericViewerFactory* factory) { 
				if (commandviewer_factory != NULL) delete commandviewer_factory;
				commandviewer_factory = factory;
		}

		void register_ResourceViewer_factory(GenericViewerFactory* factory) { 
			if (resourceViewerFactory != NULL) delete resourceViewerFactory;
			resourceViewerFactory = factory;
		}

		void register_VelocityViewer_factory(GenericViewerFactory* factory) { 
			if (velocityViewerFactory != NULL) delete velocityViewerFactory;
			velocityViewerFactory = factory;
		}

		void register_FaceViewer_factory(GenericViewerFactory* factory) { 
			if (faceViewerFactory != NULL) delete faceViewerFactory;
			faceViewerFactory = factory;
		}		

		void setMediaPath(std::string path);
		const std::string& getMediaPath();

                void setInteractive(bool val);
                bool getInteractive();


		void createDefaultControllers();
		std::vector<MeController*>& getDefaultControllers();

		void addNvbg(std::string id, Nvbg* nvbg);
		void removeNvbg(std::string id);
		Nvbg* getNvbg(std::string id);

	public:
		FILE* open_sequence_file( const char *seq_name, std::string& fullPath );
};

//////////////////////////////////////////////////////////////////


#endif
