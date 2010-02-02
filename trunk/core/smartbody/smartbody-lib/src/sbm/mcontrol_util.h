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


#define LINK_VHMSG_CLIENT		(1)


#if LINK_VHMSG_CLIENT
#include "vhmsg-tt.h"
#endif

#include "bonebus.h"


#include <SR/sr_viewer.h>
#include <SR/sr_camera.h>
#include <ME/me_ct_pose.h>
#include <ME/me_ct_motion.h>
#include <ME/me_ct_lifecycle_test.hpp>
#include "me_ct_step_turn.h"
#include "me_ct_quick_draw.h"
#include "me_ct_gaze.h"
#include "me_ct_tether.h"
#include "me_ct_eyelid.h"

#include "sbm_constants.h"

#include <ME/me_ct_scheduler2.h>

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
#include "sbm_perf.h"

#include "joint_logger.hpp"

#include <sbm/action_unit.hpp>
#include <sbm/viseme_map.hpp>

#include BML_PROCESSOR_INCLUDE


namespace WSP
{
    class Manager;
};


//////////////////////////////////////////////////////////////////


// This class is meant for listening to specific events that could be handled externally from smartbody
// Currently being used by smartbody-dll
class SBMCharacterListener
{
   public:
      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass ) {}
      virtual void OnCharacterDelete( const std::string & name ) {}
      virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime ) {}
};


// Motion Controller Utility Callback Handle (Yes, seriously.)
class mcuCBHandle	{
	protected:
		// Data
		remote_speech				_speech_rvoice;
		SmartBody::AudioFileSpeech	_speech_audiofile;
		text_speech					_speech_text; // [BMLR]
		unsigned int				queued_cmds;

	public:
		// Data
		bool		loop;
		bool		vhmsg_enabled;
		bool		net_bone_updates;
		bool		net_world_offset_updates;
		bool		net_face_bones;
		const char* net_host;
		BoneBusClient bonebus;
		SBMCharacterListener * sbm_character_listener;   // only one listener possible, must be set manually
		const char* process_id;
		bool		play_internal_audio;
		bool		lock_dt; // if true: report fixed dt to animation system
		double		desired_max_fps;
		double		time;

		SbmPerfReport perf;

		SrViewer	*viewer_p;
		SrCamera	*camera_p;
		SrSnGroup	*root_group_p;
		
		srPathList	seq_paths;
		srPathList	me_paths;
		srPathList	bp_paths;

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

		srHashMap <SkPosture>		pose_map;
		srHashMap <SkMotion>		motion_map;

		SkMotion*                   face_neutral_p;
		AUMotionMap					au_motion_map;
		VisemeMotionMap				viseme_map;

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

		srHashMap <SbmPawn>			pawn_map;
		srHashMap <SbmCharacter>	character_map;

		BML_PROCESSOR				bml_processor;
		WSP::Manager*				theWSP;

		joint_logger::EvaluationLogger* logger_p;


	private:
		// Constant
		static mcuCBHandle* _singleton;

		//  Constructor
		mcuCBHandle( void );
		virtual ~mcuCBHandle( void );
		void clear();

		// Private access prevents calls to the following
		mcuCBHandle( mcuCBHandle& );
		mcuCBHandle& operator= (const mcuCBHandle&);

//		void test_map();

	public:
		static mcuCBHandle& singleton() {
			if( !_singleton )
				_singleton = new mcuCBHandle();
			return *_singleton;
		}
		static void destroy_singleton() {
			if( _singleton )
				delete _singleton;
			_singleton = NULL;
		}

		void reset();
		void set_time( double real_time );

		int open_viewer( int width, int height, int px, int py );
		void close_viewer( void );
		int add_scene( SrSnGroup *scene_p );
		int remove_scene( SrSnGroup *scene_p );
		void render( void )	{ if( viewer_p ) { viewer_p->render(); } }
		
		void update( void );
		int insert( char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( cmd_map.insert( key, fp ) );
		}

		int insert_set_cmd( char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( set_cmd_map.insert( key, fp ) );
		}

		int insert_print_cmd( char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( print_cmd_map.insert( key, fp ) );
		}

		int insert_test_cmd( char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( test_cmd_map.insert( key, fp ) );
		}

		void set_net_host( const char * net_host );
		void set_process_id( const char * process_id );

		int vhmsg_send( const char *op, const char* message );

		int vhmsg_send( const char* message );

		int load_motions( const char* pathname, bool recursive );
		int load_poses( const char* pathname, bool recursive );


		MeController* lookup_ctrl( const std::string& ctrl_name, const char* print_error_prefix=NULL );

		srCmdSeq* lookup_seq( const char* );

		int execute( const char *key, srArgBuffer& args ) { 
			return( cmd_map.execute( key, args, this ) ); 
		}

		int execute( const char *key, char* strArgs ) { 
            srArgBuffer args( strArgs );
			return( cmd_map.execute( key, args, this ) ); 
		}

		int execute( char *cmd ) { 
			return( cmd_map.execute( cmd, this ) ); 
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

		int abort_seq( const char* command );
		int delete_seq( const char* command );

		remote_speech* speech_rvoice() { return &_speech_rvoice; }
		SmartBody::AudioFileSpeech* speech_audiofile() { return &_speech_audiofile; }
		text_speech* speech_text() { return &_speech_text; } // [BMLR]

		void NetworkSendSkeleton( BoneBusCharacter * character, SkSkeleton * skeleton );

	protected:
		FILE* open_sequence_file( const char *seq_name );
};

/*! Executes a variable setting sub-command.   See mcuCBHandle::insert_set_cmd(..). */
int mcu_set_func( srArgBuffer& args, mcuCBHandle *mcu_p );
/*! Executes a variable printing/data debugging sub-command.   See mcuCBHandle::insert_print_cmd(..). */
int mcu_print_func( srArgBuffer& args, mcuCBHandle *mcu_p );
/*! Executes a test sub-command.   See mcuCBHandle::insert_test_cmd(..). */
int mcu_test_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_filepath_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_sequence_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_sequence_chain_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_viewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_camera_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_time_func( srArgBuffer& args, mcuCBHandle *mcu_p );

//int mcu_character_func( srArgBuffer& args, mcuCBHandle *mcu_p );  // Old version... See SbmCharacter::character_cmd_func
int mcu_character_init( const char* char_name, const char *skel_file, const char *unreal_class, mcuCBHandle *mcu_p );
int mcu_character_ctrl_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_character_bone_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_character_bone_position_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );
//  The following have been replace by SbmCharacter::remove_from_scene( const char* )
//int mcu_character_remove( const char * char_name, mcuCBHandle * mcu_p );
//int mcu_removeallcharacters_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_set_face_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_print_face_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_set_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_print_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_set_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_print_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_sched_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_motion_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_stepturn_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_quickdraw_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_gaze_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_snod_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_lilt_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_load_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_net_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_net_reset( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_play_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_stop_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_uscriptexec_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_commapi_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_vrKillComponent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_vrAllCall_func( srArgBuffer& args, mcuCBHandle *mcu_p );

//////////////////////////////////////////////////////////////////

int mcu_wsp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

//////////////////////////////////////////////////////////////////

int mcu_divulge_content_func( srArgBuffer& args, mcuCBHandle* mcu_p );

//////////////////////////////////////////////////////////////////
#endif
