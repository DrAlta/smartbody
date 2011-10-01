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

#include "mcontrol_util.h"

//////////////////////////////////////////////////////////////////

int mcu_help_func( srArgBuffer& args, mcuCBHandle *mcu_p );

// parser which would return a specified file name with full path
char * mcn_return_full_filename_func( const char * current_path, const char * file_name);	

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
int mcu_bmlviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_panimationviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_channelbufferviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_resourceViewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_faceViewer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_camera_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_terrain_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_time_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_time_ival_prof_func( srArgBuffer& args, mcuCBHandle *mcu_p );

// panimation commands
int mcu_panim_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_panim_test_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_motion_player_func(srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_panim_schedule_func(std::string motion1, std::string charName, bool loop, mcuCBHandle *mcu_p);
int mcu_panim_schedule_func(std::string motion1, std::string motion2, std::string characterName, float weight, bool loop, mcuCBHandle *mcu_p);

int mcu_physics_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_motion_mirror_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p );

//int mcu_character_func( srArgBuffer& args, mcuCBHandle *mcu_p );  // Old version... See SbmCharacter::character_cmd_func

int mcu_character_init( const char* char_name, const char *skel_file, const char *unreal_class, mcuCBHandle *mcu_p );
int mcu_character_ctrl_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_character_bone_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_character_bone_position_cmd( const char* char_name, srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_character_load_mesh( const char* char_name, const char* obj_file, mcuCBHandle* mcu_p, const char* option = NULL );	// support obj file for now
int mcu_character_load_skinweights( const char* char_name, const char* skin_file, mcuCBHandle* mcu_p, float scaleFactor, const char* prefix = NULL );	// support colladda for now
int mcu_character_breathing(const char* char_name, srArgBuffer& args, mcuCBHandle* mcu_p);


//  The following have been replace by SbmCharacter::remove_from_scene( const char* )
//int mcu_character_remove( const char * char_name, mcuCBHandle * mcu_p );
//int mcu_removeallcharacters_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_set_face_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_print_face_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_set_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string name );
int mcu_print_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string name  );
int mcu_set_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string name );
int mcu_print_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string name  );

int mcu_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_sched_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_motion_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_stepturn_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_quickdraw_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_gaze_limit_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_gaze_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_snod_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_lilt_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_load_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_net_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_net_reset( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_net_check( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_play_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_stop_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_uscriptexec_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_commapi_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_vrKillComponent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_vrAllCall_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_vrQuery_func( srArgBuffer& args, mcuCBHandle* mcu_p );

int mcu_divulge_content_func( srArgBuffer& args, mcuCBHandle* mcu_p );
int mcu_wsp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_syncpolicy_func( srArgBuffer& args, mcuCBHandle *mcu_p );

// examine the motion for specific skeleton
int mcu_check_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_adjust_motion_function( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_mediapath_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int mcu_vhmsg_connect_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_vhmsg_disconnect_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int register_animation_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int unregister_animation_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int resetanim_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int animation_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int triggerevent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_python_func( srArgBuffer& args, mcuCBHandle* mcu_p );
int mcu_pythonscript_func( srArgBuffer& args, mcuCBHandle* mcu_p );
int addevent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int removeevent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int disableevents_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int enableevents_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int registerevent_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int unregisterevent_func( srArgBuffer& args, mcuCBHandle *mcu_p );

int setmap_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int motionmap_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int skeletonmap_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int mcu_steer_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int showcharacters_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int showpawns_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int syncpoint_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int pawnbonebus_func( srArgBuffer& args, mcuCBHandle *mcu_p );
#ifdef USE_GOOGLE_PROFILER
int startprofile_func( srArgBuffer& args, mcuCBHandle *mcu_p );
int stopprofile_func( srArgBuffer& args, mcuCBHandle *mcu_p );
#endif

int mcu_joint_datareceiver_func(srArgBuffer& args, mcuCBHandle *mcu_p);
int mcu_vrExpress_func(srArgBuffer& args, mcuCBHandle *mcu_p);
int vhmsglog_func( srArgBuffer& args, mcuCBHandle *mcu_p );

void mcu_vhmsg_callback( const char *op, const char *args, void * user_data );

int mcu_reset_func( srArgBuffer& args, mcuCBHandle *mcu_p  );
int mcu_echo_func( srArgBuffer& args, mcuCBHandle *mcu_p  );
int sbm_main_func( srArgBuffer & args, mcuCBHandle * mcu_p );
int sbm_vhmsg_send_func( srArgBuffer& args, mcuCBHandle *mcu_p  );

//////////////////////////////////////////////////////////////////
#endif
