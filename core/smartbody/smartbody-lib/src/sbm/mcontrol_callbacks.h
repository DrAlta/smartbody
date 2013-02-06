/*
 *  mcontrol_callbacks.h - part of SmartBody-lib
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

#ifndef MCONTROL_CALLBACKS_H
#define MCONTROL_CALLBACKS_H

#include <sbm/sr_arg_buff.h>
#include <sb/SBCommandManager.h>

//////////////////////////////////////////////////////////////////

int mcu_help_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

// parser which would return a specified file name with full path
char * mcn_return_full_filename_func( const char * current_path, const char * file_name);	



int mcu_filepath_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_sequence_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_sequence_chain_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_viewer_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_camera_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_terrain_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_time_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_time_ival_prof_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

// panimation commands
int mcu_panim_cmd_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_panim_test_cmd_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_motion_player_func(srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_panim_schedule_func(std::string motion1, std::string charName, bool loop, SmartBody::SBCommandManager* cmdMgr);
int mcu_panim_schedule_func(std::string motion1, std::string motion2, std::string characterName, float weight, bool loop, SmartBody::SBCommandManager* cmdMgr);

int mcu_physics_cmd_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_motion_mirror_cmd_func( srArgBuffer& args,SmartBody::SBCommandManager* cmdMgr );

//int mcu_character_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );  // Old version... See SbmCharacter::character_cmd_func

int mcu_character_init( const char* char_name, const char *skel_file, const char *unreal_class, SmartBody::SBCommandManager* cmdMgr );

int mcu_load_mesh( const char* pawnName, const char* obj_file, SmartBody::SBCommandManager* cmdMgr, const char* option = NULL );	// support obj file for now
int mcu_character_load_mesh( const char* char_name, const char* obj_file, SmartBody::SBCommandManager* cmdMgr, const char* option = NULL );	// support obj file for now
int mcu_character_load_skinweights( const char* char_name, const char* skin_file, SmartBody::SBCommandManager* cmdMgr, float scaleFactor, const char* prefix = NULL );	// support colladda for now
int mcu_character_breathing(const char* char_name, srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr);


//  The following have been replace by SbmCharacter::remove_from_scene( const char* )
//int mcu_character_remove( const char * char_name, mcuCBHandle * mcu_p );
//int mcu_removeallcharacters_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_set_face_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_print_face_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_set_face_au_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr, std::string name );
int mcu_print_face_au_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr, std::string name  );
int mcu_set_face_viseme_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr, std::string name );
int mcu_print_face_viseme_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr, std::string name  );

int mcu_controller_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_net_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_net_reset( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_net_check( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_play_sound_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_stop_sound_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_uscriptexec_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_commapi_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_vrKillComponent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_vrAllCall_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_vrQuery_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr);

//perception
int mcu_vrPerception_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

// listening feedback
int mcu_vrBCFeedback_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

//speech
int mcu_vrSpeech_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_sbmdebugger_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_wsp_cmd_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_syncpolicy_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

// examine the motion for specific skeleton
int mcu_check_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_adjust_motion_function( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_mediapath_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int mcu_vhmsg_connect_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_vhmsg_disconnect_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int register_animation_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int unregister_animation_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int resetanim_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int animation_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int triggerevent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_python_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  );
int mcu_pythonscript_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  );
int addevent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int removeevent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int disableevents_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int enableevents_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int registerevent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int unregisterevent_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int setmap_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int motionmap_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int motionmapdir_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int skeletonmap_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int mcu_steer_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int showcharacters_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int showpawns_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int syncpoint_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
#ifdef USE_GOOGLE_PROFILER
int startprofile_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int stopprofile_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int startheapprofile_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int stopheapprofile_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
#endif

int mcu_joint_datareceiver_func(srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr);
int mcu_vrExpress_func(srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr);
int vhmsglog_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int skscale_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int skmscale_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

int xmlcachedir_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );
int xmlcache_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr );

void mcu_vhmsg_callback( const char *op, const char *args, void * user_data );
int mcuFestivalRemoteSpeechCmd_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr);

int mcu_reset_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  );
int mcu_echo_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  );
int sb_main_func( srArgBuffer & args, SmartBody::SBCommandManager* cmdMgr  );
int sbm_main_func( srArgBuffer & args, SmartBody::SBCommandManager* cmdMgr  );
int sbm_vhmsg_send_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  );

//////////////////////////////////////////////////////////////////
#endif
