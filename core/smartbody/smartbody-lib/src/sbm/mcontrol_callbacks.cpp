/*
 *  mcontrol_callbacks.cpp - part of SmartBody-lib
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
 *      Corne Versloot, USC
 *      Thomas Amundsen, USC
 *      
 */

#include "vhcl.h"

#include <stdlib.h>
#include <iostream>
#include <string>
#include <direct.h>

#include "sbm_audio.h"

#include "me_utilities.hpp"
#include "wsp.h"
#include "mcontrol_callbacks.h"
#include "sr/sr_model.h"
#include "sbm_pawn.hpp"

using namespace std;
using namespace WSP;

/////////////////////////////////////////////////////////////

int mcu_help_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		int level = args.read_int();

		if( level == 2 )	{
			LOG("API help...");

		}
		else
		if( level == 1 )	{
			LOG("Full help...");

		}
		else	{
			LOG("HELP: <sbm-command>");
			LOG("  <>		argument description");
			LOG("  |		or");
			LOG("  []		optional");
			LOG("  \\		deprecated");
			LOG("  #		planned");
			LOG("  !		needs attention");
			LOG("  ()		description");
			LOG("  sbm [id <pid>] <sbm-command>");
		}
	}
	return( CMD_SUCCESS );
}

/////////////////////////////////////////////////////////////

char * mcn_return_full_filename_func( const char * current_path, const char * file_name)
{
	if( file_name == NULL)	return NULL;
	char * currentPath = new char[_MAX_PATH];
	char * fileName = new char[_MAX_PATH];
	strcpy( currentPath, current_path );
	strcpy( fileName, file_name );
	char * full_filename = new char[_MAX_PATH];
	std::vector<std::string> filename_token;		// tokens for the full file name

	char * token;
	token = strtok( currentPath, "\\" );
	while(token != NULL)
	{
		std::stringstream	stream;
		stream << token;
		filename_token.push_back(stream.str());
		stream.clear();
		token = strtok( NULL, "\\" );
	}

	token = strtok( fileName, "/" );
	while(token!=NULL)
	{
		if( strcmp(token, "..") == 0 )
			filename_token.pop_back();
		if( token[0] != '.')
		{
			std::stringstream	stream;
			stream << token;
			filename_token.push_back(stream.str());
		}
		token = strtok( NULL, "/" );
	}	
	if( !filename_token.empty() )
	{
		strcpy( full_filename, filename_token[0].c_str()); 
		for(unsigned int i = 1 ; i < filename_token.size(); i++)
		{
			strcat( full_filename, "/" );
			strcat( full_filename, filename_token[i].c_str() );
		}
	}
	return full_filename;
}

/////////////////////////////////////////////////////////////

/*

	path seq|me|bp <file-path>

*/

int mcu_filepath_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

    if( mcu_p )	{
		char *path_tok = args.read_token();
		char* path = args.read_token();

		PathResource* pres = new PathResource();
		pres->setPath(path);
		if( strcmp( path_tok, "seq" ) == 0 )	{
			
			pres->setType("seq");
			mcu_p->seq_paths.insert( path );
		}
		else
		if(
			( strcmp( path_tok, "me" ) == 0 )||
			( strcmp( path_tok, "ME" ) == 0 )
		)	{
			pres->setType("me");
			mcu_p->me_paths.insert( path );
		}
		else
		if(
			( strcmp( path_tok, "bp" ) == 0 )||
			( strcmp( path_tok, "BP" ) == 0 )
		)	{
			pres->setType("bp");
			mcu_p->bp_paths.insert( path );
		}
		else	{
			delete pres;
			LOG( "mcu_filepath_func ERR: token '%s' NOT FOUND\n", path_tok );
			return( CMD_FAILURE );
		}
		
		mcu_p->resource_manager->addResource(pres);
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*

	PREPROCESSOR: make paths available immediately to inline seq preprocessor
	
		path seq|me|bp <file-path>
		seq <name> inline

*/

void mcu_preprocess_sequence( srCmdSeq *to_seq_p, srCmdSeq *fr_seq_p, mcuCBHandle *mcu_p )	{
	float t;
	char *cmd;
	
	fr_seq_p->reset();
	while( cmd = fr_seq_p->pull( & t ) )	{
		srArgBuffer args( cmd );
		srCmdSeq *inline_seq_p = NULL;

		char *tok = args.read_token();
		if( strcmp( tok, "path" ) == 0 )	{
			mcu_filepath_func( args, mcu_p );
			delete [] cmd;
			cmd = NULL;
		}
		else
		if( strcmp( tok, "seq" ) == 0 )	{
			char *name = args.read_token();
			tok = args.read_token();
			if( strcmp( tok, "inline" ) == 0 )	{

				inline_seq_p = mcu_p->lookup_seq( name );
				delete [] cmd;
				cmd = NULL;
				if( inline_seq_p == NULL )	{
					LOG( "mcu_preprocess_sequence ERR: inline seq '%s' NOT FOUND\n", name );
					return;
				}
			}
		}
		
		float absolute_offset = fr_seq_p->offset() + t;
		if( inline_seq_p )	{
			// iterate hierarchy
			inline_seq_p->offset( absolute_offset );
			mcu_preprocess_sequence( to_seq_p, inline_seq_p, mcu_p );
		}
		else
		if( cmd )	{
			// propagate un-consumed command
			to_seq_p->insert_ref( absolute_offset, cmd );
		}
	}
	delete fr_seq_p;
}

int begin_sequence( char* seq_name, mcuCBHandle *mcu_p )	{
	int err = CMD_FAILURE;
	
	srCmdSeq *seq_p = mcu_p->lookup_seq( seq_name );
	
	if( seq_p ) {
	
		// EXPAND INLINE SEQs HERE

		srCmdSeq *cp_seq_p = new srCmdSeq;
		mcu_preprocess_sequence( cp_seq_p, seq_p, mcu_p );

		cp_seq_p->offset( (float)( mcu_p->time ) );
		err = mcu_p->active_seq_map.insert( seq_name, cp_seq_p );

		if( err != CMD_SUCCESS )	{
			LOG( "begin_sequence ERR: insert active: '%s' FAILED\n", seq_name ); 
		}
	}

	return( err );
}

/*

	seq <name> at <time> <cmd...>
	seq <name> [begin|abort|print]
#	seq <name> write

*/

int mcu_sequence_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	int err;
	
	if( mcu_p )	{
		char *seq_name = args.read_token();
		char *seq_cmd = args.read_token();

		if( ( strcmp( seq_cmd, "begin" ) == 0 )||( strcmp( seq_cmd, EMPTY_STRING ) == 0 ) )	{
			int ret = begin_sequence( seq_name, mcu_p );
			return ret;
		}
		else	{
		if( strcmp( seq_cmd, "at" ) == 0 )	{
		
			srCmdSeq *seq_p = mcu_p->pending_seq_map.lookup( seq_name );
			if( seq_p == NULL )	{
				seq_p = new srCmdSeq;
				err = mcu_p->pending_seq_map.insert( seq_name, seq_p );
				if( err == CMD_FAILURE )	{
					LOG( "mcu_sequence_func ERR: insert pending '%s' FAILED\n", seq_name ); 
					return( err );
				}
			}
			
			float seq_time = args.read_float();
			char *seq_string = args.read_remainder_raw();
			int ret = seq_p->insert( seq_time, seq_string );
			return ret;
		}
		else
		if( strcmp( seq_cmd, "print" ) == 0 )	{
			
			srCmdSeq *seq_p = mcu_p->pending_seq_map.lookup( seq_name );
			if( seq_p == NULL )	{
				LOG( "mcu_sequence_func ERR: print: '%s' NOT FOUND\n", seq_name ); 
				return( CMD_FAILURE );
			}
			seq_p->print( stdout );
		}
		else
		if( strcmp( seq_cmd, "abort" ) == 0 )	{
			int result = mcu_p->abort_seq( seq_name );
			if( result == CMD_NOT_FOUND )	{
				LOG( "mcu_sequence_func ERR: abort: '%s' NOT FOUND\n", seq_name ); 
			}
			return( result );
		}
		else
		if( strcmp( seq_cmd, "delete" ) == 0 )	{
			int result = mcu_p->abort_seq( seq_name );
			if( result == CMD_NOT_FOUND )	{
				LOG( "mcu_sequence_func ERR: delete: '%s' NOT FOUND\n", seq_name ); 
			}
			return( result );
		}
		else
		{
			return( CMD_FAILURE );
		}
		}
		
		return( CMD_SUCCESS );
	}

	return( CMD_FAILURE );
}

/*
	seq-chain <seqname>*
*/

int mcu_sequence_chain_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	vector<string> seq_names;
	const char* token = args.read_token();
	while( token[0] != '\0' ) {
		seq_names.push_back( token );

		token = args.read_token();
	}

	if( seq_names.empty() ) {
		LOG("ERROR: seq-chain expected one or more .seq filenames.");
		return CMD_FAILURE;
	}

	return mcu_p->execute_seq_chain( seq_names, "ERROR: seq-chian: " );
}


/////////////////////////////////////////////////////////////

/*
	Executes a command to set a configuration variable.
*/

int mcu_set_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->set_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		// TODO: Differentiate between not finding this var and subargs
		LOG("SBM ERR: Unknown Variable, Cannot set: '%s'\n> ", arg );  // Clarify this as a set command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/*
	Executes a command to print to the console some debug data.
	See insert_print_cmd and its call in main
*/

int mcu_print_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->print_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		// TODO: Differentiate between not finding this var and subargs
		LOG("SBM ERR: Print command NOT FOUND: '%s'\n> ", arg );  // Clarify this as a print command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/*
	Executes a test sub-command.
	See insert_test_cmd and its call in main
*/

int mcu_test_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->test_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		LOG("SBM ERR: Test command NOT FOUND: '%s'\n> ", arg );  // Clarify this as a test command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/////////////////////////////////////////////////////////////

/*

	viewer open <width> <height> <px> <py> 
	viewer show|hide
	
*/

int mcu_viewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{

		char *view_cmd = args.read_token();
		if( strcmp( view_cmd, "open" ) == 0 )	{

			if( mcu_p->viewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				int width = 640;
				int height = 480;
				int px = 100;
				int py = 100;
				if( argc >= 4 )	{
					width = args.read_int();
					height = args.read_int();
					px = args.read_int();
					py = args.read_int();
				}
				int err = mcu_p->open_viewer( width, height, px, py );
				return( err );
			}
		}
		else
		if( strcmp( view_cmd, "show" ) == 0 )	{
			if( mcu_p->viewer_p )	{
				mcu_p->viewer_p->show_viewer();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( view_cmd, "hide" ) == 0 )	{
			if( mcu_p->viewer_p )	{
				mcu_p->viewer_p->hide_viewer();
				return( CMD_SUCCESS );
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/*

	bmlviewer open <width> <height> <px> <py> 
	bmlviewer show|hide
	
*/

int mcu_bmlviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{

		char *bmlview_cmd = args.read_token();
		if( strcmp( bmlview_cmd, "open" ) == 0 )	{

			if( mcu_p->bmlviewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->open_bml_viewer( width, height, px, py );
					return( err );
				} else {
					int err = mcu_p->open_bml_viewer( 800, 600, 100, 100 );
					return( err );
				}
			}
		}
		else
		if( strcmp( bmlview_cmd, "show" ) == 0 )	{
			if( mcu_p->bmlviewer_p )	{
				mcu_p->bmlviewer_p->show_bml_viewer();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( bmlview_cmd, "hide" ) == 0 )	{
			if( mcu_p->bmlviewer_p )	{
				mcu_p->bmlviewer_p->hide_bml_viewer();
				return( CMD_SUCCESS );
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*

	camera	eye <x y z>
	camera	center <x y z>
#	camera	up <x y z>
#	camera	fovy <degrees>
	camera	scale <factor>
	camera	default [<preset>]

*/

int mcu_camera_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		if( mcu_p->viewer_p )	{
			char *cam_cmd = args.read_token();
			if( strcmp( cam_cmd, "eye" ) == 0 )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				mcu_p->camera_p->eye.set( x, y, z );
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "center" ) == 0 )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				mcu_p->camera_p->center.set( x, y, z );
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "scale" ) == 0 )	{
				mcu_p->camera_p->scale = args.read_float();
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "track" ) == 0 )	{
				char* name = args.read_token();
				if (!name || strcmp(name, "") == 0)
				{
					if (mcu_p->cameraTracking.size() > 0)
					{
						for (std::vector<CameraTrack*>::iterator iter = mcu_p->cameraTracking.begin();
							 iter != mcu_p->cameraTracking.end();
							 iter++)
						{
							CameraTrack* cameraTrack = (*iter);
							delete cameraTrack;
						}
						mcu_p->cameraTracking.clear();
						LOG("Removing current tracked object.");
						return( CMD_SUCCESS );
					}
					LOG("Need to specify an object and a joint to track.");
					return( CMD_FAILURE );
				}
				SbmPawn* pawn = mcu_p->pawn_map.lookup(name);
				if (!pawn)
				{
					pawn = mcu_p->character_map.lookup(name);
					if (!pawn)
					{
						LOG("Object %s was not found, cannot track.", name);
						return( CMD_FAILURE );
					}
				}
				char* jointName = args.read_token();
				if (!jointName || strcmp(jointName, "") == 0)
				{
					LOG("Need to specify a joint to track.");
					return( CMD_FAILURE );
				}
				SkSkeleton* skeleton = NULL;
				skeleton = pawn->skeleton_p;

				SkJoint* joint = pawn->skeleton_p->search_joint(jointName);
				if (!joint)
				{
					LOG("Could not find joint %s on object %s.", jointName, name);
					return( CMD_FAILURE );
				}
	
				if (mcu_p->cameraTracking.size() > 0)
				{
					for (std::vector<CameraTrack*>::iterator iter = mcu_p->cameraTracking.begin();
							 iter != mcu_p->cameraTracking.end();
							 iter++)
					{
						CameraTrack* cameraTrack = (*iter);
						delete cameraTrack;
					}
					mcu_p->cameraTracking.clear();
					LOG("Removing current tracked object.");
				}
				joint->skeleton()->update_global_matrices();
				joint->update_gmat();
				const SrMat& jointMat = joint->gmat();
				SrVec jointPos(jointMat[12], jointMat[13], jointMat[14]);
				CameraTrack* cameraTrack = new CameraTrack();
				cameraTrack->joint = joint;
				cameraTrack->jointToCamera = mcu_p->camera_p->eye - jointPos;
				LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
				cameraTrack->targetToCamera = mcu_p->camera_p->eye - mcu_p->camera_p->center;
				LOG("Vector from target to eye is %f %f %f", cameraTrack->targetToCamera.x, cameraTrack->targetToCamera.y, cameraTrack->targetToCamera.z);				
				mcu_p->cameraTracking.push_back(cameraTrack);
				LOG("Object %s will now be tracked at joint %s.", name, jointName);
				
			}
			else
			if( strcmp( cam_cmd, "default" ) == 0 )	{
				int preset = args.read_int();
				
				if( preset == 1 )	{
					mcu_p->viewer_p->view_all();
				}
			}
			else if (strcmp( cam_cmd, "reset" ) == 0 ) {
				mcu_p->execute("camera eye 0 166 185");
				mcu_p->execute("camera center 0 92 0");
			}
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

int mcu_terrain_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *terr_cmd = args.read_token();

		if( strcmp( terr_cmd, "help" ) == 0 )	{
			LOG("HELP: terrain");	
			LOG("  load [<filename>]");
			LOG("  scale <X Y Z>");
			LOG("  origin <X Y Z>|auto");
			LOG("  delete");
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( terr_cmd, "load" ) == 0 )	{

			if( mcu_p->height_field_p == NULL )	{
				mcu_p->height_field_p = new Heightfield();
			}
			int n = args.calc_num_tokens();
			if( n == 0 )	{
				mcu_p->height_field_p->load( "../../../../data/terrain/range1.e.ppm" );
				mcu_p->height_field_p->set_scale( 5000.0f, 300.0f, 5000.0f );
				mcu_p->height_field_p->set_auto_origin();
			}
			else	{
				char *filename = args.read_token();
				mcu_p->height_field_p->load( filename );
			}
			return( CMD_SUCCESS );
		}
		else
		if( mcu_p->height_field_p == NULL ) {
			LOG( "mcu_terrain_func: ERR: no heightfield loaded" );
			return( CMD_FAILURE );
		}

		if( strcmp( terr_cmd, "scale" ) == 0 )	{
			
			float x = args.read_float();
			float y = args.read_float();
			float z = args.read_float();
			mcu_p->height_field_p->set_scale( x, y, z );
		}
		else
		if( strcmp( terr_cmd, "origin" ) == 0 )	{

			int n = args.calc_num_tokens();
			if( n == 1 )	{
				char *sub_cmd = args.read_token();
				if( strcmp( sub_cmd, "auto" ) == 0 )	{
					mcu_p->height_field_p->set_auto_origin();
				}
				else	{
					LOG( "mcu_terrain_func: ERR: token '%s' not recognized", sub_cmd );
					return( CMD_NOT_FOUND );
				}
			}
			else	{
				
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				mcu_p->height_field_p->set_origin( x, y, z );
			}
		}
		else
		if( strcmp( terr_cmd, "delete" ) == 0 )	{
			
			delete mcu_p->height_field_p;
			mcu_p->height_field_p = NULL;
		}
		else {
			return( CMD_NOT_FOUND );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

void mcu_print_timer_deprecation_warning( void )	{

	LOG("WARNING: fps/lockdt feature is deprecated");
	LOG("  - If you insist, be sure to set fps first, then lockdt...");
	LOG("  - Use 'time sleepfps <fps>' and 'time simfps <fps>' instead");
}

void mcu_print_timer_help( int level = 0 )	{
	
	if( level == 2 ) {
		LOG("API help...");
	
	}
	else
	if( level == 1 )	{
		LOG("Full help...");
	}
	else	{
		LOG("HELP: time");	
		LOG("  help | fullhelp");
		LOG("  maxfps | fps <desired-max-fps>	(DEPRECATED)");
		LOG("  lockdt [0|1]					(DEPRECATED)");
		LOG("  perf [0|1 [<interval>]]");
		LOG("  speed <real-time-factor>");
		LOG("  sleepfps | simfps | evalfps <fps>");
		LOG("  sleepdt | simdt | evaldt <dt>");
		LOG("  pause | resume");
		LOG("  step [num-steps]");
		LOG("  perf [<interval>]");
		LOG("  print");
	}
}

int mcu_time_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *time_cmd = args.read_token();

		if( strcmp( time_cmd, "help" ) == 0 )	{
			mcu_print_timer_help();
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( time_cmd, "fullhelp" ) == 0 )	{
			mcu_print_timer_help( 1 );
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( time_cmd, "api" ) == 0 )	{
			mcu_print_timer_help( 2 );
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( time_cmd, "test" ) == 0 )	{
//			void test_time_regulator( void );
//			test_time_regulator();
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( time_cmd, "print" ) == 0 )	{
			if( mcu_p->timer_p )	{
				mcu_p->timer_p->print();
			}
			else	{
				LOG( "TIME:%.3f ~ DT:%.3f %.2f:FPS\n",
					mcu_p->time,
					mcu_p->time_dt,
					1.0 / mcu_p->time_dt
				);
			}
			return( CMD_SUCCESS );
		}

		if( mcu_p->timer_p == NULL )	{
			LOG( "mcu_time_func NOTICE: %s: TimeRegulator was NOT REGISTERED\n", time_cmd );
			mcu_p->switch_internal_profiler(); 
		}
		TimeRegulator *timer_p = mcu_p->timer_p;
		
		if( strcmp( time_cmd, "reset" ) == 0 ) {
			timer_p->reset();
		}
		else 
		if( ( strcmp( time_cmd, "maxfps" ) == 0 ) || ( strcmp( time_cmd, "fps" ) == 0 ) )	{ // deprecate
			mcu_print_timer_deprecation_warning();
			timer_p->set_sleep_fps( args.read_float() );
		}
		else
		if( strcmp( time_cmd, "lockdt" ) == 0 )	{ // deprecate
			mcu_print_timer_deprecation_warning();
			int enable = true;
			int n = args.calc_num_tokens();
			if( n ) enable = args.read_int();
			timer_p->set_sleep_lock( enable != false );
		}
		else 
		if( strcmp( time_cmd, "speed" ) == 0 ) {
			timer_p->set_speed( args.read_float() );
		}
		else 
		if( strcmp( time_cmd, "sleepfps" ) == 0 ) {
			timer_p->set_sleep_fps( args.read_float() );
		}
		else 
		if( strcmp( time_cmd, "evalfps" ) == 0 ) {
			timer_p->set_eval_fps( args.read_float() );
		}
		else
		if( strcmp( time_cmd, "simfps" ) == 0 ) {
			timer_p->set_sim_fps( args.read_float() );
		}
		else 
		if( strcmp( time_cmd, "sleepdt" ) == 0 ) {
			timer_p->set_sleep_dt( args.read_float() );
		}
		else 
		if( strcmp( time_cmd, "evaldt" ) == 0 ) {
			timer_p->set_eval_dt( args.read_float() );
		}
		else 
		if( strcmp( time_cmd, "simdt" ) == 0 ) {
			timer_p->set_sim_dt( args.read_float() );
		}
		else
		if( strcmp( time_cmd, "pause" ) == 0 )	{
			timer_p->pause();
		}
		else 
		if( strcmp( time_cmd, "resume" ) == 0 )	{
			timer_p->resume();
		}
		else 
		if( strcmp( time_cmd, "step" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				timer_p->step( args.read_int() );
			}
			else	{
				timer_p->step( 1 );
			}
		}
		else 
		if( strcmp( time_cmd, "perf" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				timer_p->set_perf( args.read_float() );
			}
			else	{
				timer_p->set_perf( 10.0 );
			}
		}
		else {
			return( CMD_NOT_FOUND );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

void mcu_print_profiler_help( int level = 0 )	{
	
	if( level == 2 ) {
		LOG("API help...");
	}
	else
	if( level == 1 )	{
		LOG("Full help...");
	}
	else	{
		LOG( "HELP: tip" );	
		LOG( "  help | fullhelp | legend" );
		LOG( "  on | off | enable | disable | preload" );
		LOG( "  group <name> enable | disable | preload" );
		LOG( "  suppress | select [<level>]" );
		LOG( "  abs <delta> | 0" );
		LOG( "  rel <factor> | 0" );
		LOG( "  fix [abs|rel]" );
		LOG( "  dyn [abs|rel]" );
		LOG( "  sniff <factor:[0.0,1.0)>" );
		LOG( "  avoid <factor:(1.0,...)>" );
		LOG( "  decaying <factor:[0.0,1.0)>" );
		LOG( "  rolling <count>" );
		LOG( "  print | report" );
		LOG( "  erase | reset" );
		LOG( "  test [reps]" );
	}
}

int mcu_time_ival_prof_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *tip_cmd = args.read_token();

		if( strcmp( tip_cmd, "help" ) == 0 )	{
			mcu_print_profiler_help();
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( tip_cmd, "fullhelp" ) == 0 )	{
			mcu_print_profiler_help( 1 );
			return( CMD_SUCCESS );
		}
		else
		if( strcmp( tip_cmd, "api" ) == 0 )	{
			mcu_print_profiler_help( 2 );
			return( CMD_SUCCESS );
		}

		if( mcu_p->profiler_p == NULL )	{
			LOG( "mcu_time_ival_prof_func NOTICE: %s: TimeIntervalProfiler was NOT REGISTERED\n", tip_cmd ); 
			mcu_p->switch_internal_profiler();
		}
		TimeIntervalProfiler *prof_p = mcu_p->profiler_p;

		if( strcmp( tip_cmd, "legend" ) == 0 ) {
			prof_p->print_legend();
		}
		else
		if( strcmp( tip_cmd, "print" ) == 0 )	{
			prof_p->print();
		}
		else
		if( strcmp( tip_cmd, "report" ) == 0 )	{
			prof_p->report();
//			prof_p->full_report();
		}
		else
		if( strcmp( tip_cmd, "erase" ) == 0 )	{
			prof_p->erase();
		}
		else
		if( strcmp( tip_cmd, "reset" ) == 0 )	{
			prof_p->reset();
		}
		else 
		if( strcmp( tip_cmd, "on" ) == 0 )	{
			prof_p->bypass( false );
		}
		else
		if( strcmp( tip_cmd, "off" ) == 0 )	{
			prof_p->bypass( true );
		}
		else 
		if( strcmp( tip_cmd, "enable" ) == 0 )	{
			prof_p->enable( true );
		}
		else
		if( strcmp( tip_cmd, "disable" ) == 0 )  {
			prof_p->enable( false );
		}
		else
		if( strcmp( tip_cmd, "preload" ) == 0 )	{
			prof_p->preload();
		}
		else
		if( strcmp( tip_cmd, "group" ) == 0 )	{
			char *group_name = args.read_token();
			char *group_cmd = args.read_token();
			if( strcmp( group_cmd, "enable" ) == 0 )	{
				prof_p->enable( group_name, true );
			}
			else
			if( strcmp( group_cmd, "disable" ) == 0 )	{
				prof_p->enable( group_name, false );
			}
			else
			if( strcmp( group_cmd, "preload" ) == 0 )	{
				prof_p->preload( group_name );
			}
		}
		else
		if( strcmp( tip_cmd, "suppress" ) == 0 )	{
			int level = -1;
			int n = args.calc_num_tokens();
			if( n ) level = args.read_int();
			prof_p->set_suppression( level );
		}
		else
		if( strcmp( tip_cmd, "select" ) == 0 )	{
			int level = -1;
			int n = args.calc_num_tokens();
			if( n ) level = args.read_int();
			prof_p->set_selection( level );
		}
		else
		if( strcmp( tip_cmd, "abs" ) == 0 )	{
			float delta = args.read_float();
			prof_p->set_abs_threshold( (double)delta );
		}
		else
		if( strcmp( tip_cmd, "rel" ) == 0 )	{
			float factor = args.read_float();
			prof_p->set_rel_threshold( (double)factor );
		}
		else
		if( strcmp( tip_cmd, "fix" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				char *opt = args.read_token();
				if( strcmp( opt, "abs" ) == 0 )	{
					prof_p->set_dynamic_abs( false );
				}
				else
				if( strcmp( opt, "rel" ) == 0 )	{
					prof_p->set_dynamic_rel( false );
				}
				else	{
					LOG("ERROR: Unknown command option '%s'", opt );
					return( CMD_NOT_FOUND );
				}
			}
			else	{
				prof_p->set_dynamic_abs( false );
				prof_p->set_dynamic_rel( false );
			}
		}
		else
		if( strcmp( tip_cmd, "dyn" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				char *opt = args.read_token();
				if( strcmp( opt, "abs" ) == 0 )	{
					prof_p->set_dynamic_abs( true );
				}
				else
				if( strcmp( opt, "rel" ) == 0 )	{
					prof_p->set_dynamic_rel( true );
				}
				else	{
					LOG("ERROR: Unknown command option '%s'", opt );
					return( CMD_NOT_FOUND );
				}
			}
			else	{
				prof_p->set_dynamic_abs( true );
				prof_p->set_dynamic_rel( true );
			}
		}
		else
		if( strcmp( tip_cmd, "sniff" ) == 0 )	{
			float value = args.read_float();
			prof_p->set_sniff( (double)value );
		}
		else
		if( strcmp( tip_cmd, "avoid" ) == 0 )	{
			float value = args.read_float();
			prof_p->set_avoid( (double)value );
		}
		else
		if( strcmp( tip_cmd, "decaying" ) == 0 )	{
			float sm = args.read_float();
			prof_p->set_decaying( (double)sm );
		}
		else
		if( strcmp( tip_cmd, "rolling" ) == 0 )	{
			int c = args.read_int();
			prof_p->set_rolling( c );
		}
		else
		if( strcmp( tip_cmd, "test" ) == 0 )	{
			mcu_p->mark( "mcu_tip_func", 0, "test" );
			int reps = args.read_int();
			prof_p->test_clock( reps );
			mcu_p->mark( "mcu_tip_func" );
		}
		else
		if( strcmp( tip_cmd, "test2" ) == 0 )	{
			mcu_p->mark( "mcu_tip_func", 0, "test2" );
			int reps = args.read_int();
			prof_p->test_clock( reps );
			mcu_p->mark( "mcu_tip_func" );
		}
		else {
			return( CMD_NOT_FOUND );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

///////////////////////////////////////////////////////////////////

int mcu_character_load_mesh(const char* char_name, const char* obj_file, mcuCBHandle *mcu_p)
{
	SbmCharacter* char_p = mcu_p->character_map.lookup( char_name );
	if( !char_p )	
	{
		LOG( "mcu_character_load_mesh ERR: SbmCharacter '%s' NOT FOUND\n", char_name ); 
		return( CMD_FAILURE );
	}
	SrModel* objModel = new SrModel();
	if (!objModel->import_obj(obj_file))
	{
		LOG( "mcu_character_load_mesh ERR\n" );
		return( CMD_FAILURE );
	}
	
	SrSnModel* srSnModelDynamic = new SrSnModel();
	SrSnModel* srSnModelStatic = new SrSnModel();
	srSnModelDynamic->shape(*objModel);
	srSnModelStatic->shape(*objModel);
	srSnModelDynamic->changed(true);
	srSnModelDynamic->visible(false);
	const int char_max = 128;
	char file[char_max];
	char drive[char_max];
	char dir[char_max];
	char ext[char_max];
	_splitpath(obj_file, drive, dir, file, ext);
	srSnModelStatic->shape().name = std::string(file).c_str();
	srSnModelDynamic->shape().name = std::string(file).c_str();
	char_p->dMesh_p->dMeshDynamic_p.push_back(srSnModelDynamic);
	char_p->dMesh_p->dMeshStatic_p.push_back(srSnModelStatic);
	mcu_p->root_group_p->add(srSnModelDynamic);
	return( CMD_SUCCESS );
}

std::string nodeStr(const XMLCh* s)
{
	if (!s)	return "";
	std::string str = XMLString::transcode(s);
	return str;
}

std::string tokenize( std::string& str,
					  const std::string& delimiters = " ",
					  int mode = 1 )
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	if (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		std::string return_string = str.substr(lastPos, pos - lastPos);
		std::string::size_type lastPos = str.find_first_not_of(delimiters, pos);
		if (mode == 1)
		{
			if (std::string::npos == lastPos)	str = "";
			else								str = str.substr(lastPos, str.size() - lastPos);
		}
		return return_string;
	}
	else
		return "";
}


void parseLibraryControllers(xercesc_3_0::DOMNode* node, const char* char_name, mcuCBHandle* mcu_p)
{
	SbmCharacter* char_p = mcu_p->character_map.lookup( char_name );
	const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
	for (unsigned int c = 0; c < list->getLength(); c++)
	{
		xercesc_3_0::DOMNode* node = list->item(c);
		int type = node->getNodeType();
		std::string name = nodeStr(node->getNodeName());
		std::string value = nodeStr(node->getNodeValue());
		if (name == "controller")
		{
			xercesc_3_0::DOMNamedNodeMap* attributes = node->getAttributes();
			xercesc_3_0::DOMNode* idNode = attributes->getNamedItem(XMLString::transcode("id"));
			if (!idNode)	continue;
			std::string skinId = nodeStr(idNode->getNodeValue());
			if (node->hasChildNodes())
			{
				const xercesc_3_0::DOMNodeList* childrenList = node->getChildNodes();
				for (unsigned int cc = 0; cc < childrenList->getLength(); cc++)
				{
					xercesc_3_0::DOMNode* childNode = childrenList->item(cc);
					std::string childName = nodeStr(childNode->getNodeName());
					if (childName == "skin")	// parsing skinning weights
					{
						xercesc_3_0::DOMNamedNodeMap* skinAttributes = childNode->getAttributes();			
						xercesc_3_0::DOMNode* skinNode = skinAttributes->getNamedItem(XMLString::transcode("source"));	
						std::string skinSource = nodeStr(skinNode->getNodeValue());
						skinSource = skinSource.substr(1, skinSource.size() - 1);
						SkinWeight* skinWeight = new SkinWeight();
						skinWeight->sourceMesh = skinSource;

						// futhur for children
						const xercesc_3_0::DOMNodeList* childListOfSkin = childNode->getChildNodes();
						for (unsigned int cSkin = 0; cSkin < childListOfSkin->getLength(); cSkin++)
						{
							xercesc_3_0::DOMNode* childNodeOfSkin = childListOfSkin->item(cSkin);
							std::string childNameOfSkin = nodeStr(childNodeOfSkin->getNodeName());
							std::string bindJointName = skinSource + "-skin-joints";
							std::string bindWeightName = skinSource + "-skin-weights";
							std::string bindPoseMatName = skinSource + "-skin-bind_poses";

							if (childNameOfSkin == "bind_shape_matrix")
							{
								std::string tokenBlock = nodeStr(childNodeOfSkin->getTextContent());
								float* bindShapeMat = new float[16];
								for (int i = 0; i < 16; i++)
									bindShapeMat[i] = (float)atof(tokenize(tokenBlock).c_str());
								skinWeight->bindShapeMat.set(bindShapeMat);
								skinWeight->bindShapeMat.transpose();
							}
							if (childNameOfSkin == "source")
							{
								xercesc_3_0::DOMNamedNodeMap* sourceAttributes = childNodeOfSkin->getAttributes();
								xercesc_3_0::DOMNodeList* realContentNodeList = childNodeOfSkin->getChildNodes();
								std::string sourceId = nodeStr(sourceAttributes->getNamedItem(XMLString::transcode("id"))->getNodeValue());
								for (unsigned int cSource = 0; cSource < realContentNodeList->getLength(); cSource++)
								{
									xercesc_3_0::DOMNode* realContentNode = realContentNodeList->item(cSource);
									std::string realNodeName = nodeStr(realContentNode->getNodeName());		

									std::string tokenBlock = nodeStr(realContentNode->getTextContent());
									std::string content = tokenize(tokenBlock);
									int matCounter = 0;
									float* bindPosMat = new float[16];
									SrMat* newMat = new SrMat();
									while (content != "")
									{
										if ( sourceId == bindJointName && realNodeName == "Name_array")
											skinWeight->infJointName.push_back(content);
										if ( sourceId == bindWeightName && realNodeName == "float_array")
											skinWeight->bindWeight.push_back((float)atof(content.c_str()));
										if ( sourceId == bindPoseMatName && realNodeName == "float_array")
										{
											bindPosMat[matCounter] = (float)atof(content.c_str());
											matCounter ++;
											if (matCounter == 16)
											{
												matCounter = 0;
												newMat->set(bindPosMat);
												newMat->transpose();
												skinWeight->bindPoseMat.push_back(*newMat);
											}
										}
										content = tokenize(tokenBlock);
									}
								}								
							} // end of if (childNameOfSkin == "source")
							if (childNameOfSkin == "vertex_weights")
							{
								xercesc_3_0::DOMNodeList* indexNodeList = childNodeOfSkin->getChildNodes();
								for (unsigned int cVertexWeights = 0; cVertexWeights < indexNodeList->getLength(); cVertexWeights++)
								{
									xercesc_3_0::DOMNode* indexNode = indexNodeList->item(cVertexWeights);
									std::string indexNodeName = nodeStr(indexNode->getNodeName());
									std::string tokenBlock = nodeStr(indexNode->getTextContent());
									std::string content = tokenize(tokenBlock);
									if (indexNodeName == "vcount")
									{
										while (content != "")
										{
											skinWeight->numInfJoints.push_back(atoi(content.c_str()));
											content = tokenize(tokenBlock);
										}											
									}
									else if (indexNodeName == "v")
									{
										while (content != "")
										{
											skinWeight->jointNameIndex.push_back(atoi(content.c_str()));
											content = tokenize(tokenBlock);
											skinWeight->weightIndex.push_back(atoi(content.c_str()));
											content = tokenize(tokenBlock);
										}											
									}
									else continue;
								}
							}
						}
						if (char_p)
							char_p->dMesh_p->skinWeights.push_back(skinWeight);
					} // end of if (childName == "skin")
					if (childName == "morph")	// parsing morph targets
					{
						xercesc_3_0::DOMNamedNodeMap* morphAttributes = childNode->getAttributes();			
						xercesc_3_0::DOMNode* morphNode = morphAttributes->getNamedItem(XMLString::transcode("source"));	
						std::string morphName = nodeStr(morphNode->getNodeValue());
						morphName = morphName.substr(1, morphName.size() - 1);
						std::string morphFullName = morphName + "-morph";
						
						// futhur for children
						const xercesc_3_0::DOMNodeList* childListOfMorph = childNode->getChildNodes();
						for (unsigned int cMorph = 0; cMorph < childListOfMorph->getLength(); cMorph++)
						{
							xercesc_3_0::DOMNode* childNodeOfMorph = childListOfMorph->item(cMorph);
							std::string childNameOfMorph = nodeStr(childNodeOfMorph->getNodeName());
							if (childNameOfMorph == "source")
							{
								const xercesc_3_0::DOMNodeList* childListOfSource = childNodeOfMorph->getChildNodes();
								for (size_t cMorphSource = 0; cMorphSource < childListOfSource->getLength(); cMorphSource++)
								{
									xercesc_3_0::DOMNode* childNodeOfSource = childListOfSource->item(cMorphSource);
									std::string childNameOfSource = nodeStr(childNodeOfSource->getNodeName());
									if (childNameOfSource == "IDREF_array")
									{
										std::vector<std::string> refMesh;
										std::string tokenBlock = nodeStr(childNodeOfMorph->getTextContent());
										std::string content = tokenize(tokenBlock, " \n");
										while (content != "")
										{
											refMesh.push_back(content);
											content = tokenize(tokenBlock, " \n");
										}
										refMesh.push_back(morphName);
										char_p->dMesh_p->morphTargets.insert(make_pair(morphFullName, refMesh));
									}
								}
							}
						}
					} // end of if (childName == "morph")
				}
			}
		}
	}

	// cache the joint names for each skin weight
	for (size_t x = 0; x < char_p->dMesh_p->skinWeights.size(); x++)
	{
		SkinWeight* skinWeight = char_p->dMesh_p->skinWeights[x];
		for (size_t j = 0; j < skinWeight->infJointName.size(); j++)
		{
			std::string& jointName = skinWeight->infJointName[j];
			SkJoint* curJoint = char_p->skeleton_p->search_joint(jointName.c_str());
			skinWeight->infJoint.push_back(curJoint); // NOTE: If joints are added/removed during runtime, this list will contain stale data
		}
	}
}

void parseNode(xercesc_3_0::DOMNode* node, const char* char_name, mcuCBHandle* mcu_p)
{
	int type = node->getNodeType();
	std::string name = nodeStr(node->getNodeName());
	std::string value = nodeStr(node->getNodeValue());
	    

	if (name == "library_controllers" && node->getNodeType() ==  xercesc_3_0::DOMNode::ELEMENT_NODE)
	{
		  parseLibraryControllers(node, char_name, mcu_p);
	}
	if (node->hasChildNodes())
	{
		  const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
		  for (unsigned int c = 0; c < list->getLength(); c++)
		  {
				parseNode(list->item(c), char_name, mcu_p);
		  }
	}
}


int mcu_character_load_skinweights( const char* char_name, const char* skin_file, mcuCBHandle* mcu_p )
{
	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Error during initialization! :\n" << message << "\n";
		XMLString::release(&message);
		return( CMD_FAILURE );
	}

	XercesDOMParser* parser = new XercesDOMParser();
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true);    // optional

	ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
	parser->setErrorHandler(errHandler);

	try 
	{
		parser->parse(skin_file);
		xercesc_3_0::DOMDocument* doc = parser->getDocument();
		parseNode(doc, char_name, mcu_p);
	}
	catch (const XMLException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return( CMD_FAILURE );
	}
	catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return( CMD_FAILURE );
	}
		catch (...) {
		std::cout << "Unexpected Exception \n" ;
		return( CMD_FAILURE );
	}

	delete parser;
	delete errHandler;

	return( CMD_SUCCESS );	
}

int mcu_character_init( 
	const char* char_name, 
	const char* skel_file, 
	const char* unreal_class, 
	mcuCBHandle *mcu_p
)	{
	int err;
	
	if( strcmp(char_name, "*" )==0 ) {  // TODO: better character name valiadtion
		LOG( "init_character ERR: Invalid SbmCharacter name '%s'\n", char_name ); 
		return( CMD_FAILURE );
	}
	if( mcu_p->character_map.lookup( char_name ) )	{
		LOG( "init_character ERR: SbmCharacter '%s' EXISTS\n", char_name ); 
		return( CMD_FAILURE );
	}

	SbmCharacter *char_p = new SbmCharacter(char_name);
	SkSkeleton* skeleton_p = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager, mcu_p->skScale );
	if( !skeleton_p ) {
		LOG( "init_character ERR: Failed to load skeleton \"%s\"\n", skel_file ); 
		return CMD_FAILURE;
	}

	// Only initialize face_neutral if -facebone is enabled
	SkMotion* face_neutral_p = mcu_p->net_face_bones? mcu_p->face_neutral_p : NULL;
	err = char_p->init( skeleton_p, face_neutral_p, &mcu_p->au_motion_map, &mcu_p->viseme_map, &mcu_p->param_map, unreal_class, mcu_p->use_locomotion );
	if( err == CMD_SUCCESS ) {

		if (mcu_p->use_locomotion) 
		{
			int locoSuccess = char_p->init_locomotion_analyzer(skel_file, mcu_p);//temp init for analyzer Jingqiao Fu Aug/07/09
			if (locoSuccess != CMD_SUCCESS) {
				LOG("init_character ERR: Failed to init locomotion analyzer\n"); 
			}
		}

		char_p->ct_tree_p->set_evaluation_logger( mcu_p->logger_p );

		err = mcu_p->pawn_map.insert( char_name, char_p );
		if( err != CMD_SUCCESS )	{
			LOG( "init_character ERR: SbmCharacter pawn_map.insert(..) '%s' FAILED\n", char_name ); 
			delete char_p;
			return( err );
		}

		err = mcu_p->character_map.insert( char_name, char_p );
		if( err != CMD_SUCCESS )	{
			LOG( "init_character ERR: SbmCharacter character_map.insert(..) '%s' FAILED\n", char_name ); 
			mcu_p->pawn_map.remove( char_name );
			delete char_p;
			return( err );
		}

		err = mcu_p->add_scene( char_p->scene_p );
		if( err != CMD_SUCCESS )	{
			LOG( "init_character ERR: add_scene '%s' FAILED\n", char_name ); 
			return( err );
		}


		// register wsp data
		// first register world_offset position/rotation
		string wsp_world_offset = vhcl::Format( "%s:world_offset", char_name );

		err = mcu_p->theWSP->register_vector_3d_source( wsp_world_offset, "position", SbmPawn::wsp_world_position_accessor, char_p );
		if( err != CMD_SUCCESS )	{
			LOG( "WARNING: mcu_character_init \"%s\": Failed to register character position.\n", char_name ); 
		}

		err = mcu_p->theWSP->register_vector_4d_source( wsp_world_offset, "rotation", SbmPawn::wsp_world_rotation_accessor, char_p );
		if( err != CMD_SUCCESS )	{
			LOG( "WARNING: mcu_character_init \"%s\": Failed to register character rotation.\n", char_name ); 
		}


		// now register all joints.  wsp data isn't sent out until a request for it is received
		const SrArray<SkJoint *> & joints  = char_p->skeleton_p->joints();

		int i;
		for ( i = 0; i < joints.size(); i++ )
		{
			SkJoint * j = joints[ i ];

			string wsp_joint_name = vhcl::Format( "%s:%s", char_name, (const char *)j->name() );

			err = mcu_p->theWSP->register_vector_3d_source( wsp_joint_name, "position", SbmPawn::wsp_position_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				LOG( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" position.\n", char_name, wsp_joint_name ); 
			}

			err = mcu_p->theWSP->register_vector_4d_source( wsp_joint_name, "rotation", SbmPawn::wsp_rotation_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				LOG( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" rotation.\n", char_name, wsp_joint_name ); 
			}
		}
	}

	return( err );
}

int begin_controller( 
	const char *char_name, 
	const char *ctrl_name, 
	mcuCBHandle *mcu_p
)	{
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( ctrl_name );
		if( ctrl_p )	{
			// Use motion schedule by default
			MeCtScheduler2* sched_p = char_p->motion_sched_p;

			if( strcmp( ctrl_p->controller_type(), MeCtGaze::CONTROLLER_TYPE )==0 ) {
				sched_p = char_p->gaze_sched_p;
			}

			sched_p->schedule(
				ctrl_p, 
				mcu_p->time,
				mcu_p->time + ctrl_p->controller_duration(),
				ctrl_p->indt(), 
				ctrl_p->outdt()
			);
			return( CMD_SUCCESS );
		}
		LOG( "begin_controller ERR: ctrl '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	LOG( "begin_controller ERR: char '%s' NOT FOUND\n", char_name );
	return( CMD_FAILURE );
}

int begin_controller( 
	const char *char_name, 
	const char *ctrl_name, 
	float ease_in, 
	float ease_out, 
	mcuCBHandle *mcu_p
)	{
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( ctrl_name );
		if( ctrl_p )	{
			//char_p->scheduler_p->schedule( 
			char_p->motion_sched_p->schedule( // Regardless of type, controllers created via ctrl commands are treated as motions
				ctrl_p, 
				mcu_p->time, 
				mcu_p->time + ctrl_p->controller_duration(),
				ease_in, 
				ease_out
			);
			return( CMD_SUCCESS );
		}
		LOG( "begin_controller ERR: ctrl '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	LOG( "begin_controller ERR: char '%s' NOT FOUND\n", char_name );
	return( CMD_FAILURE );
}

#if 0  // Version replaced by SbmCharacter::character_cmd_func // WHY ???
/*

	char <> init <skel-file>
	char <> ctrl <> begin [<ease-in> [<ease-out>]]
	char <> viseme <viseme_name> <weight>
	char <> bone <bone_name> <w> <x> <y> <z>
	char <> remove

*/

int mcu_character_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *char_name = args.read_token();
		char *char_cmd = args.read_token();
	
		if( strcmp( char_cmd, "init" ) == 0 )	{
			char *skel_file = args.read_token();
			char *unreal_class = args.read_token();
			return(	
				mcu_character_init( char_name, skel_file, unreal_class, mcu_p )
			);
		}
		if( strcmp( char_cmd, "ctrl" ) == 0 )	{
			return mcu_character_ctrl_cmd( char_name, args, mcu_p );
		}
		if ( strcmp( char_cmd, "viseme" ) == 0 )
		{
			char * viseme = args.read_token();
			float  weight = args.read_float();
			float  duration = args.read_float();	// added for ramps but not used yet!
		 
			SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
			if ( !actor ) {
				LOG( "ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"%s\".\n", char_name );
				return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
			} else {
				actor->set_viseme( viseme, weight, duration );
				return CMD_SUCCESS;
			}
		}
		if ( strcmp( char_cmd, "bone" ) == 0 )
		{
			mcu_character_bone_cmd( char_name, args, mcu_p );
		}
		if ( strcmp( char_cmd, "remove" ) == 0 )
		{
			mcu_character_remove( char_name, mcu_p );

			return CMD_SUCCESS;
		}

		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}
#endif

/////////////////////////////////////////////////////////////

int mcu_character_ctrl_cmd(
	const char* char_name,
	srArgBuffer& args,
	mcuCBHandle *mcu_p 
) {
	const char *ctrl_name = args.read_token();
	const char *ctrl_cmd = args.read_token();
	
	if( strcmp( ctrl_cmd, "begin" ) == 0 )	{
		int n = args.calc_num_tokens();
		if( n )	{
			float ease_in = args.read_float();
			float ease_out = args.read_float();
			return(
				begin_controller( 
					char_name, 
					ctrl_name, 
					ease_in, 
					ease_out, 
					mcu_p
				)
			);
		}
		else	{
			return(
				begin_controller( 
					char_name, 
					ctrl_name, 
					mcu_p
				)
			);
		}
	}
	else
	if( strcmp( ctrl_cmd, "end" ) == 0 )	{
		
		//// TODO
		LOG( "ERROR: \"char <char id> ctrl <ctrl id> end ...\" Unimplemented.\n" );
		return( CMD_FAILURE );
	}
	else
		return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

// EDF - Hack!  This is currently used by Rapport for sending bone data straight through to Unreal.
//       It's only purpose is to be able to set a bone's rotation from an outside process (through VHMsg).

// "sbm char doctor bone base w x y z"
int mcu_character_bone_cmd(
	const char * char_name,
	srArgBuffer & args,
	mcuCBHandle *mcu_p 
) {
	char * bone = args.read_token();
	float  w    = args.read_float();
	float  x    = args.read_float();
	float  y    = args.read_float();
	float  z    = args.read_float();

	SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
	if ( !actor || !actor->skeleton_p )
	{
		return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
	}
	else
	{
		actor->bonebusCharacter->StartSendBoneRotations();

		for ( int i = 0; i < actor->skeleton_p->joints().size(); i++ )
		{
			SkJoint * j = actor->skeleton_p->joints()[ i ];

			if ( _stricmp( j->name(), bone ) == 0 )
			{
				actor->bonebusCharacter->AddBoneRotation( j->name(), w, x, y, z, mcu_p->time );

				//LOG( "%s %f %f %f %f\n", (const char *)j->name(), w, x, y, z );
			}
		}

		actor->bonebusCharacter->EndSendBoneRotations();


		actor->bonebusCharacter->StartSendBonePositions();

		for ( int i = 0; i < actor->skeleton_p->joints().size(); i++ )
		{
			SkJoint * j = actor->skeleton_p->joints()[ i ];

			if ( _stricmp( j->name(), bone ) == 0 )
			{
				float posx = j->pos()->value( 0 );
				float posy = j->pos()->value( 1 );
				float posz = j->pos()->value( 2 );

				actor->bonebusCharacter->AddBonePosition( j->name(), posx, posy, posz, mcu_p->time );
			}
		}

		actor->bonebusCharacter->EndSendBonePositions();
	}

	return CMD_SUCCESS;
}

// "sbm char doctor bonep base x y z"
int mcu_character_bone_position_cmd(
   const char* char_name,
   srArgBuffer& args,
   mcuCBHandle *mcu_p 
) {
   char * bone = args.read_token();
   float  x    = args.read_float();
   float  y    = args.read_float();
   float  z    = args.read_float();
   int i;

   SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
   if ( !actor || !actor->skeleton_p )
   {
      return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
   }
   else
   {
      actor->bonebusCharacter->StartSendBonePositions();

      for ( i = 0; i < actor->skeleton_p->joints().size(); i++ )
      {
         SkJoint * j	= actor->skeleton_p->joints()[ i ];

         if ( _stricmp( j->name(), bone ) == 0 )
         {
            float posx, posy, posz;

            /*
            if ( j->_ed == SrVec( 0, 0, 0 ) )
            {
               j->_ed = j->offset();
            }

            posx = j->_ed.x + x;
            posy = j->_ed.y + y;
            posz = j->_ed.z + z;
            */

            j->pos()->value( 0, x );
            j->pos()->value( 1, y );
            j->pos()->value( 2, z );

            //j->offset( SrVec( posx, posy, posz ) );

            posx = x;
            posy = y;
            posz = z;

            actor->bonebusCharacter->AddBonePosition( j->name(), posx, posy, posz, mcu_p->time );
         }
      }

      actor->bonebusCharacter->EndSendBonePositions();
   }

   return CMD_SUCCESS; 
}


/////////////////////////////////////////////////////////////

// Face pose mapping functions
const char* SET_FACE_AU_SYNTAX_HELP        = "set face au <unit-number> [left|right] <motion-name>";
const char* SET_FACE_VISEME_SYNTAX_HELP    = "set face viseme <viseme symbol> <motion-name>";
const char* PRINT_FACE_AU_SYNTAX1_HELP     = "print face au <unit number>";
const char* PRINT_FACE_AU_SYNTAX2_HELP     = "print face au *";
const char* PRINT_FACE_VISEME_SYNTAX1_HELP = "print face viseme <viseme name>";
const char* PRINT_FACE_VISEME_SYNTAX2_HELP = "print face viseme *";


int mcu_set_face_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string type = args.read_token();
	if( type.length() == 0 || type=="help" ) {
		// No arguments => help message
		LOG("Syntax:");
		LOG("\t%s", SET_FACE_AU_SYNTAX_HELP);
		LOG("\t%s", SET_FACE_VISEME_SYNTAX_HELP);
		return CMD_SUCCESS;
	}

	if( type=="au" ) {
		return mcu_set_face_au_func( args, mcu_p );
	} else if( type=="viseme" ) {
		return mcu_set_face_viseme_func( args, mcu_p );
	} else if( type=="neutral" ) {
		const string motion_name = args.read_token();
		if( motion_name.length()==0 ) {
			LOG("ERROR: Missing motion name.");
			return CMD_FAILURE;
		}

		std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(motion_name);
		if (motionIter != mcu_p->motion_map.end())
		{
			SkMotion* motion_p = (*motionIter).second;
			mcu_p->face_neutral_p = motion_p;
			return CMD_SUCCESS;
		} else {
			LOG("ERROR: Unknown motion \"%s\".", motion_name.c_str());
			return CMD_FAILURE;
		}
	} else {
		LOG("ERROR: Unknown command \"set face %s.", type.c_str());
		return CMD_NOT_FOUND;
	}
}

int mcu_print_face_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string type = args.read_token();
	if( type.length() == 0 || type=="help" ) {
		// No arguments => help message
		std::stringstream strstr;
		strstr << "Syntax:"<< endl
		     << "\t" << PRINT_FACE_AU_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX2_HELP << endl
		     << "\t" << PRINT_FACE_VISEME_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_VISEME_SYNTAX2_HELP;
		LOG("%s", strstr.str().c_str());
		return CMD_SUCCESS;
	}

	if( type=="au" ) {
		return mcu_print_face_au_func( args, mcu_p );
	} else if( type=="viseme" ) {
		return mcu_print_face_viseme_func( args, mcu_p );
	} else if( type=="neutral" ) {
		LOG("UNIMPLEMENTED");
		return CMD_FAILURE;
	} else {
		LOG("ERROR: Unknown command \"print face %s.", type.c_str());
		return CMD_NOT_FOUND;
	}
}

/**
 *  Command processor for "set face au ...".
 *  Syntax:
 *		set face au <unit-number> [left|right] <motion-name>
 */
int mcu_set_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string unit_str = args.read_token();
	if( unit_str.length()==0 ) {
		// No arguments => help message
		LOG("Syntax: %s", SET_FACE_AU_SYNTAX_HELP);
		return CMD_SUCCESS;
	}

	istringstream unit_iss( unit_str );
	int unit;
	if( !( unit_iss >> unit )
		|| ( unit < 1 ) )
	{
		LOG("ERROR: Invalid action unit number \"%s\".", unit_str);
		return CMD_FAILURE;
	}

	enum { UNIFIED, LEFT, RIGHT } side;
	string token = args.read_token();
	string face_pose_name;
	if( token=="left" || token=="LEFT" ) {
		side = LEFT;
		face_pose_name = args.read_token();
	} else if( token=="right" || token=="RIGHT" ) {
		side = RIGHT;
		face_pose_name = args.read_token();
	} else {
		side = UNIFIED;
		face_pose_name = token;
	}

	if( face_pose_name.length()==0 ) {
		LOG("ERROR: Missing viseme motion name.");
		return CMD_FAILURE;
	}

	// Currently we use the first frame of SkMotion because
	// of limitations in our exports (can't export direct to .skp).
	// TODO: use .skp and/or convert arbitrary frame number/time to SkPosture
	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(face_pose_name);
	if (motionIter == mcu_p->motion_map.end())
	{
		LOG("ERROR: Unknown facial pose \"%s\".", face_pose_name);
		return CMD_FAILURE;
	}
	SkMotion* motion =(*motionIter).second;

	AUMotionPtr au;
	AUMotionMap& au_map = mcu_p->au_motion_map;
	AUMotionMap::iterator pos = au_map.find(unit);
	if( pos == au_map.end() ) {
		switch( side ) {
			case UNIFIED:
				au = new AUMotion( motion );
				break;
			case LEFT:
				au = new AUMotion( motion, NULL );
				break;
			case RIGHT:
				au = new AUMotion( NULL, motion );
				break;
			default:
				// Invalid code.  Throw assert?
				LOG("ERROR: Invalid side \"%d\".", side);
				return CMD_FAILURE;
		}
		au_map.insert( make_pair( unit, au ) );
	} else {
		au = pos->second;  // value half of std::pair
		switch( side ) {
			case UNIFIED:
				if( au->left || au->right )
					LOG("WARNING: Overwritting au #%d", unit);
				au->set( motion );
				break;
			case LEFT:
				if( au->left )
					LOG("WARNING: Overwritting au #%d left", unit);
				au->set( motion, au->right );
				break;
			case RIGHT:
				if( au->right )
					LOG("WARNING: Overwritting au #%d right", unit);
				au->set( au->left, motion );
				break;
			default:
				// Invalid code.  Throw assert?
				LOG("ERROR: Invalid side \"%d\"", side);
				return CMD_FAILURE;
		}
	}

	return CMD_SUCCESS;
}

inline void print_au( const int unit, const AUMotionPtr au ) {
	if( au->is_bilateral() ) {
		std::stringstream strstr;
		strstr << "Action Unit #" << unit << ": Left SkMotion ";
		if( au->left ) {
			strstr << '\"' << au->left->name() << "\".";
		} else {
			strstr << "is NULL.";
		}
		LOG("%s", strstr.str().c_str());

		strstr.clear();
		strstr << "Action Unit #" << unit << ": Right SkMotion ";
		if( au->right ) {
			strstr << '\"' << au->right->name() << "\".";
		} else {
			strstr << "is NULL.";
		}
		LOG("%s", strstr.str().c_str());
	} else {
		std::stringstream strstr;
		strstr << "Action Unit #" << unit << ": SkMotion ";
		if( au->left ) {
			strstr << '\"' << au->left->name() << "\".";
		} else {
			strstr << "is NULL.";
		}
		LOG("%s", strstr.str().c_str());
	}
}

/**
 *  Implements the "print face au ..." command.
 *
 *  Syntax:
 *     print face au <unit-number>
 *     print face au *
 */
int mcu_print_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	const int ALL_ACTION_UNITS = 0;  // Marker

	string unit_str = args.read_token();
	if( unit_str.length()==0 ) {
		// No arguments => help message
		std::stringstream strstr;
		strstr << "Syntax:" << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX2_HELP;
		LOG("%s", strstr.str().c_str());
		return CMD_SUCCESS;
	}

	int unit;
	if( unit_str=="*" ) {
		unit = ALL_ACTION_UNITS;
	} else {
		istringstream unit_iss( unit_str );
		if( !( unit_iss >> unit )
			|| ( unit < 1 ) )
		{
			LOG("ERROR: Invalid action unit number \"%s\".", unit_str);
			return CMD_FAILURE;
		}
	}

	AUMotionPtr au;
	AUMotionMap& au_map = mcu_p->au_motion_map;
	AUMotionMap::iterator pos;
	AUMotionMap::iterator end = au_map.end();
	if( unit == ALL_ACTION_UNITS ) {
		for( pos = au_map.begin(); pos!=end; ++pos ) {
			unit = pos->first;
			au  = pos->second;

			print_au( unit, au );
		}
	} else {
		pos = au_map.find( unit );
		if( pos == end ) {
			LOG("Action Unit #%s is not set.", unit);
		} else {
			print_au( unit, pos->second );
		}
	}
	return CMD_SUCCESS;
}

/**
 *  Implements the "set face viseme ..." command.
 *
 *  Syntax: set face viseme <viseme symbol> <motion-name>
 */

int mcu_set_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		LOG("Syntax: %s", SET_FACE_VISEME_SYNTAX_HELP);
		return CMD_SUCCESS;
	}

	string motion_name = args.read_token();
	if( motion_name.length()==0 ) {
		LOG("ERROR: Missing viseme motion name.");
		return CMD_FAILURE;
	}

	// Currently we use the first frame of SkMotion because
	// of limitations in our exports (can't export direct to .skp).
	// TODO: use .skp and/or convert arbitrary frame number/time to SkPosture
	std::map<std::string, SkMotion*>::iterator motionIter = mcu_p->motion_map.find(motion_name);
	if (motionIter == mcu_p->motion_map.end())
	{
		LOG("ERROR: Unknown viseme pose \"%s\".", motion_name);
		return CMD_FAILURE;
	}
	SkMotion* motion = (*motionIter).second;
	VisemeMotionMap& viseme_map = mcu_p->viseme_map;
	VisemeMotionMap::iterator pos = viseme_map.find( viseme );
	if( pos != viseme_map.end() ) {
		LOG("WARNING: Overwriting viseme \"%s\" motion mapping.", viseme);
	}
	viseme_map.insert( make_pair( viseme, motion ) );
	
	return CMD_SUCCESS;
}

void print_viseme( const string& viseme, const SkMotion* motion ) {
	std::stringstream strstr;
	strstr << "Viseme \""<<viseme<<"\" pose: ";
	if( motion == NULL ) {
		strstr << "NULL";
	} else {
		strstr << '\"' << motion->name() << '\"';
	}
	LOG("%s", strstr.str().c_str());
}

/**
 *  Implements the "print face viseme ..." command.
 *
 *  Syntax:
 *     print face viseme <viseme-name>
 *     print face viseme *
 */

int mcu_print_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		cout << "Syntax:" << endl
		     << "\tprint face viseme <viseme name>" << endl
		     << "\tprint face viseme *" << endl;
		return CMD_SUCCESS;
	}

	VisemeMotionMap& viseme_map = mcu_p->viseme_map;
	VisemeMotionMap::iterator pos;
	VisemeMotionMap::iterator end = viseme_map.end();
	if( viseme=="*" ) {
		for( pos=viseme_map.begin(); pos!=end; ++pos ) {
			print_viseme( pos->first, pos->second );
		}
	} else {
		pos = viseme_map.find( viseme );
		if( pos == end ) {
			cout << "Viseme \""<<viseme<<"\" is unset." << endl;
		} else {
			print_viseme( viseme, pos->second );
		}
	}

	return CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////

int init_pose_controller( 
	char *ctrl_name, 
	char *pose_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkPosture*>::iterator postureIter = mcu_p->pose_map.find(std::string(pose_name));
	if (postureIter == mcu_p->pose_map.end())
	{
		LOG( "init_pose_controller ERR: SkPosture '%s' NOT FOUND in pose map\n", pose_name ); 
		return( CMD_FAILURE );
	}
	SkPosture *pose_p = (*postureIter).second;

	MeCtPose* ctrl_p = new MeCtPose;
	err = mcu_p->pose_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_pose_controller ERR: MeCtPose '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_pose_controller ERR: MeCtPose '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init( *pose_p );
	return( CMD_SUCCESS );
}

int init_motion_controller( 
	char *ctrl_name, 
	char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(mot_name);
	if( motionIter == mcu_p->motion_map.end() ) {
		LOG( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_p = (*motionIter).second;

	MeCtMotion* ctrl_p = new MeCtMotion;
	err = mcu_p->motion_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeCtMotion '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p );
	return( CMD_SUCCESS );
}

int init_stepturn_controller( 
	char *ctrl_name, 
	char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(mot_name);
	if( motionIter == mcu_p->motion_map.end() ) {
		LOG( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_p = (*motionIter).second;

	MeCtStepTurn* ctrl_p = new MeCtStepTurn;
	err = mcu_p->stepturn_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_stepturn_controller ERR: MeCtStepTurn '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_stepturn_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p );
	return( CMD_SUCCESS );
}

int init_quickdraw_controller( 
	char *ctrl_name, 
	char *mot_name, 
	char *alt_mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(std::string(mot_name));
	if( motionIter == mcu_p->motion_map.end() ) {
		LOG( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_p = (*motionIter).second;

	SkMotion *alt_mot_p = NULL;
	if (alt_mot_name)
	{
		std::map<std::string, SkMotion*>::iterator altMotionIter =  mcu_p->motion_map.find(std::string(alt_mot_name));
		if( altMotionIter == mcu_p->motion_map.end() ) {
			LOG( "init_quickdraw_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", alt_mot_name ); 
			return( CMD_FAILURE );
		}
		alt_mot_p = (*altMotionIter).second;
	}
	
	MeCtQuickDraw* ctrl_p = new MeCtQuickDraw;
	err = mcu_p->quickdraw_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_quickdraw_controller ERR: MeCtQuickDraw '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_quickdraw_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p, alt_mot_p );
	return( CMD_SUCCESS );
}

int init_gaze_controller(
	char *ctrl_name, 
	char *key_fr,
	char *key_to,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	MeCtGaze *ctrl_p;
	ctrl_p = mcu_p->gaze_ctrl_map.lookup( ctrl_name );
	if( ctrl_p )	{
		LOG( "init_gaze_controller ERR: MeCtGaze '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}

	ctrl_p = new MeCtGaze;
	err = mcu_p->gaze_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_gaze_controller ERR: MeCtGaze '%s' insertion FAILED\n", ctrl_name ); 
		delete ctrl_p;
		return( err );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_gaze_controller ERR: MeCtGaze '%s' EXISTS\n", ctrl_name ); 
		return( err );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init(
		MeCtGaze::key_index( strlen( key_fr ) ? key_fr : "back" ),  // WARN: does not handle NULL string
		MeCtGaze::key_index( strlen( key_to ) ? key_to : ( strlen( key_fr ) ? key_fr : "eyes" ) )
	);
	return( CMD_SUCCESS );
}

int init_simple_nod_controller(
	char *ctrl_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	MeCtSimpleNod* ctrl_p = new MeCtSimpleNod;
	err = mcu_p->snod_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_simple_nod_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( err );
	}
	ctrl_p->ref();
	
	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_simple_nod_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name ); 
		return( err );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init();
	return( CMD_SUCCESS );
}

int init_lilt_controller(
	char *ctrl_name,
	char *char_name,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p= mcu_p->character_map.lookup( char_name );
	if ( !char_p ) {
		LOG( "init_lilt_controller ERR: SbmCharacter '%s' NOT FOUND\n", char_name );
		return( CMD_FAILURE );
	}
	if ( !char_p->motion_sched_p ) {
		LOG( "init_lilt_controller ERR: SbmCharacter '%s' UNINITIALIZED\n", char_name );
		return( CMD_FAILURE );
	}
	MeCtAnkleLilt* ctrl_p = new MeCtAnkleLilt;
	err = mcu_p->lilt_ctrl_map.insert( ctrl_name, ctrl_p );
	if ( err == CMD_FAILURE ) {
		LOG( "init_lilt_controller ERR: MeCtAnkleLilt '%s' EXISTS\n", ctrl_name );
		delete ctrl_p;
		return( err );
	}
	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if (err == CMD_FAILURE){
		LOG( "init_lilt_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name);
		return( err );
	}
	ctrl_p->ref();
	ctrl_p->name( ctrl_name );
	ctrl_p->init( char_p->skeleton_p );
	return( CMD_SUCCESS );
}

int init_eyelid_controller(
	char *ctrl_name,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	MeCtEyeLid* ctrl_p = new MeCtEyeLid;
	err = mcu_p->eyelid_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_eyelid_controller ERR: MeCtEyeLid '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_eyelid_controller ERR: MeCtEyeLid '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
	ctrl_p->init();
	return( CMD_SUCCESS );
}

int init_lifecycle_controller( char *ctrl_name, char *child_name, mcuCBHandle *mcu_p ) {

	MeController* ct = mcu_p->controller_map.lookup( child_name );
	if( ct==NULL ) {
		LOG( "init_lifecycle_controller ERROR: MeController '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	MeCtLifecycleTest* lifecycle_ct = new MeCtLifecycleTest();
	if( mcu_p->controller_map.insert( ctrl_name, lifecycle_ct ) != CMD_SUCCESS ) {
		LOG( "init_lifecycle_controller ERROR: Failed to insert into controller_map\n" );
		return( CMD_FAILURE );
	}
	lifecycle_ct->ref();

	lifecycle_ct->name( ctrl_name );
	lifecycle_ct->init( ct );

	return CMD_SUCCESS;
}

int init_scheduler_controller( 
	char *ctrl_name, 
	char *char_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeCtScheduler2* sched_p = new MeCtScheduler2;
		err = mcu_p->sched_ctrl_map.insert( ctrl_name, sched_p );
		if( err == CMD_FAILURE )	{
			LOG( "init_scheduler_controller ERR: MeCtScheduler '%s' EXISTS\n", ctrl_name ); 
			delete sched_p;
			return( err );
		}
		sched_p->ref();
	
		err = mcu_p->controller_map.insert( ctrl_name, sched_p );
		if( err == CMD_FAILURE )	{
			LOG( "init_scheduler_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
			return( err );
		}
		sched_p->ref();
		
		sched_p->init();

		return( CMD_SUCCESS );
	}

	LOG( "init_scheduler_controller ERR: SbmCharacter '%s' NOT FOUND\n", char_name ); 
	return( CMD_FAILURE );
}

int set_controller_timing(
	MeController* ctrl_p, 
	float indt, 
	float outdt, 
	float empht
)	{
	ctrl_p->inoutdt( indt, outdt );
	ctrl_p->emphasist( empht );
	return( CMD_SUCCESS );
}

int query_controller(
	MeController* ctrl_p
)	{
	LOG( "MCU QUERY: MeController '%s':\n", ctrl_p->name() );
	LOG( "  type... %s\n", ctrl_p->controller_type() );
	LOG( "  indt... %.3f\n", ctrl_p->indt() );
	LOG( "  outdt.. %.3f\n", ctrl_p->outdt() );
	float emph = ctrl_p->emphasist();
	if( emph < 0.0 )	{
		LOG( "  emph... UNK\n");
	}
	else	{
		LOG( "  emph... %.3f\n", emph );
	}
	double dur = ctrl_p->controller_duration();
	if( dur < 0.0 )	{
		LOG( "  dur.... UNK\n" );
	}
	else	{
		LOG( "  dur.... %.3f\n", dur );
	}
//	LOG( "> " );
	return( CMD_SUCCESS );
}

/*

	ctrl <> query
	ctrl <> timing <ease-in> <ease-out> [<emph>]
	ctrl <> record motion <operation_type> [max <num-frames>]
	ctrl <> record bvh <operation_type> [max <num-frames>]
#	ctrl <> record pose <full-file-prefix>

	ctrl <> debugger on	
	ctrl <> debugger off

#	ctrl <> interp <dur> <ctrl-a> <ctrl-b> linear|sine
#	ctrl <> interp <dur> <ctrl-a> <ctrl-b> curve <curve-name>
#	ctrl <> point <x y z>

	ctrl <> gaze [<key-fr> <key-to>]

	ctrl <> motion <motion-file>
	ctrl <> snod <char-name>

	ctrl <> quickdraw <quickdraw-motion-name> [<reholster-motion>]

	ctrl <> lifecycle <child-controller-name>

#	ctrl <> sched <char-name> [<skel-subset>]
	ctrl <> sched <char-name>

*/

int mcu_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *ctrl_name = args.read_token();
		char *ctrl_cmd = args.read_token();
	
		if( strcmp(ctrl_cmd, "passthrough" ) == 0 )	{
			bool allControllers = false;
			if (strcmp(ctrl_name, "*") == 0)
				allControllers = true;

			char* value = args.read_token();
			bool passThroughValue = true;
			bool toggleValue = false;
			bool checkStatus = false;
			if (strlen(value) > 0)
			{
				if (strcmp("true", value) == 0)
				{
					passThroughValue = true;
				}
				else if (strcmp("false", value) == 0)
				{
					passThroughValue = false;
				}
				else if (strcmp("status", value) == 0)
				{
					checkStatus = true;
				}
				else
				{
					LOG("Use syntax: ctrl %s passthrough <true|false|status>, or ctrl %s passthrough ", ctrl_name, ctrl_name);
					return CMD_FAILURE;
				}
			}
			else
			{
				toggleValue = true;
			}

		
			srHashMap<SbmCharacter>& character_map = mcu_p->character_map;
			character_map.reset(); 
			SbmCharacter* character = character_map.next();
			int numControllersAffected = 0;
			while (character)
			{
				MeControllerTreeRoot* controllerTree = character->ct_tree_p;
				int numControllers = controllerTree->count_controllers();
			
				for (int c = 0; c < numControllers; c++)
				{
					if (checkStatus)
					{
						std::string passThroughStr = (controllerTree->controller(c)->is_pass_through())? " true " : " false";							
						LOG("[%s]=%s", character->name, controllerTree->controller(c)->name(), passThroughStr.c_str());
					}
					else if (allControllers)
					{
						if (toggleValue)
							controllerTree->controller(c)->set_pass_through(!controllerTree->controller(c)->is_pass_through());
						else
							controllerTree->controller(c)->set_pass_through(passThroughValue);
						numControllersAffected++;
					}
					else if (strcmp(controllerTree->controller(c)->name(), ctrl_name) == 0)
					{
						if (toggleValue)
							controllerTree->controller(c)->set_pass_through(!controllerTree->controller(c)->is_pass_through());
						else
							controllerTree->controller(c)->set_pass_through(passThroughValue);
						numControllersAffected++;
					}
				}
				character = character_map.next();
			}
			if (numControllersAffected > 0)
				return CMD_SUCCESS;
			else
				return CMD_FAILURE;
		}

		if( strcmp( ctrl_cmd, "pose" ) == 0 )	{
			char *pose_name = args.read_token();
			return(
				init_pose_controller( ctrl_name, pose_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "motion" ) == 0 )	{
			char *mot_name = args.read_token();
			return(
				init_motion_controller( ctrl_name, mot_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "stepturn" ) == 0 )	{
			char *mot_name = args.read_token();
			return(
				init_stepturn_controller( ctrl_name, mot_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "quickdraw" ) == 0 )	{
			char *mot_name = args.read_token();
			if( args.calc_num_tokens() > 0 )	{
				char *alt_mot_name = args.read_token();
				return(
					init_quickdraw_controller( ctrl_name, mot_name, alt_mot_name, mcu_p )
				);
			}
			return(
				init_quickdraw_controller( ctrl_name, mot_name, NULL, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "gaze" ) == 0 )	{
			char *key_fr = args.read_token();
			char *key_to = args.read_token();
			return(
				init_gaze_controller( ctrl_name, key_fr, key_to, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "snod" ) == 0 )	{
			return(
				init_simple_nod_controller( ctrl_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "lilt" )== 0) {
			char *char_name= args.read_token();
			return(
				init_lilt_controller( ctrl_name, char_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "eyelid" )== 0) {
			return(
				init_eyelid_controller( ctrl_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "lifecycle" )== 0) {
			char *child_name= args.read_token();
			return(
				init_lifecycle_controller( ctrl_name, child_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "sched" ) == 0 )	{
			char *char_name = args.read_token();
			return(
				init_scheduler_controller( ctrl_name, char_name, mcu_p )
			);
		}
		else
		{
			// Non-initializing controllers need an actual instance
			MeController* ctrl_p = mcu_p->lookup_ctrl( string( ctrl_name ), "ERROR: ctrl <controller name>: " );
			if( ctrl_p==NULL ) {
				// should have printed error from above function
				return CMD_FAILURE;
			}
			//example:	ctrl ~doctor/motion_sched record skm(or bvh) start [max 500]
			//			ctrl ~doctor/motion_sched record skm(or bvh) write file_prefix
			//			ctrl ~doctor/motion_sched record skm(or bvh) stop
			if( strcmp( ctrl_cmd, "record" ) == 0 )	{
				char *record_type = args.read_token();
				char *operation = args.read_token();
				int max_num_of_frames = 0;
				if( strcmp( operation, "start" ) == 0 )
				{
					char *optional = args.read_token();
					if( strcmp( optional, "max" ) == 0 )
						max_num_of_frames = args.read_int();
					if(max_num_of_frames<0)	max_num_of_frames = 0;
					
					if( strcmp( record_type, "skm" ) == 0 )	{
						ctrl_p->record_motion( max_num_of_frames );
						return( CMD_SUCCESS );
					}
					if( strcmp( record_type, "bvh" ) == 0 )	{
						if( mcu_p->timer_p )	{
							double sim_dt = mcu_p->timer_p->get_sim_dt();
							if( sim_dt > 0.0 )	{
								ctrl_p->record_bvh( max_num_of_frames, sim_dt );
								return( CMD_SUCCESS );
							}
							else	{
								LOG( "mcu_controller_func ERR: BVH recording requires 'time simfps|simdt' set\n" );
								return( CMD_FAILURE );
							}
						}
						else	{
							LOG( "mcu_controller_func ERR: BVH recording requires registered TimeRegulator\n" );
							return( CMD_FAILURE );
						}
					}
				}
				if( strcmp( operation, "write" ) == 0 )	{
					char *full_prefix = args.read_token();
					ctrl_p->record_write(full_prefix);
					return( CMD_SUCCESS );
				}
				if( strcmp( operation, "stop" ) == 0 )	{
					ctrl_p->record_stop();
					return( CMD_SUCCESS );
				}
				if( strcmp( operation, "clear" ) == 0 )	{
					ctrl_p->record_clear();
					return( CMD_SUCCESS );
				}
			}
			else
			if( strcmp( ctrl_cmd, "debugger" ) == 0 )	{
				char *operation = args.read_token();
				if( strcmp( operation, "on" ) == 0 ) {
					ctrl_p->record_buffer_changes(true);
					return( CMD_SUCCESS );
				} else if (strcmp( operation, "off" ) == 0 ) {
					ctrl_p->record_buffer_changes(false);
					return( CMD_SUCCESS );
				}
			}
			else
			if( strcmp( ctrl_cmd, "timing" ) == 0 )	{
				int n = args.calc_num_tokens();
				if( n )	{
					float indt = args.read_float();
					float outdt = args.read_float();
					float empht = -1.0;
					if( n > 2 )	{
						empht = args.read_float();
					}
					return(
						set_controller_timing( ctrl_p, indt, outdt, empht )
					);
				}
				return( CMD_FAILURE );
			}
			else
			if( strcmp( ctrl_cmd, "query" ) == 0 )	{
				return(
					query_controller( ctrl_p )
				);
			}
			else
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*
	motion <> speed <factor>
	motion <> dur <sec>
*/

int mcu_motion_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		char *warp_type = args.read_token(); // dur or speed
		float timing = args.read_float();
		MeCtMotion *mot_p= mcu_p->motion_ctrl_map.lookup( ctrl_name );
		if( mot_p ) {
			if( 
				( warp_type[ 0 ] == 'd' )||( warp_type[ 0 ] == 'D' )||
				( warp_type[ 0 ] == 't' )||( warp_type[ 0 ] == 'T' ) 
			)	{
				float out_dur = timing;
				timing = (float)( mot_p->phase_duration() ) / out_dur;
			}
			mot_p->warp_limits( timing, timing );
			mot_p->twarp( timing );
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/*
	stepturn <> dur|time|speed <sec|dps> local|world <heading-deg>
*/

int mcu_stepturn_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		char *time_type = args.read_token(); // dur or speed
		float timing = args.read_float();
		char *coord_type = args.read_token(); // local or world
		float deg = args.read_float();
		MeCtStepTurn *step_p= mcu_p->stepturn_ctrl_map.lookup( ctrl_name );
		if( step_p ) {
			if( 
				( time_type[ 0 ] == 'd' )||( time_type[ 0 ] == 'D' )||
				( time_type[ 0 ] == 't' )||( time_type[ 0 ] == 'T' ) 
			)	{
				step_p->set_duration( timing );
			}
			else	{
				step_p->set_speed( timing );
			}
			if( ( coord_type[ 0 ] == 'l' )||( coord_type[ 0 ] == 'L' ) )	{
				step_p->set_heading_local( deg );
			}
			else	{
				step_p->set_heading_world( deg );
			}
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/*
X	quickdraw <> <dur-sec> point <x y z>
X	quickdraw <> <dur-sec> point <x y z> [<joint>]

	quickdraw <> target point <x y z>
	quickdraw <> aimoff <p h r>
	quickdraw <> dur <draw-dur>
#	quickdraw <> dur <draw-dur> [<withdraw-dur>]
	quickdraw <> smooth <basis>
	quickdraw <> track <tracking-dur>
	quickdraw <> persist | reholster
*/

int mcu_quickdraw_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		MeCtQuickDraw *qdraw_p= mcu_p->quickdraw_ctrl_map.lookup( ctrl_name );
		if( qdraw_p ) {
#if 0
			float dur = args.read_float();
			qdraw_p->set_motion_duration( dur );

			char *target_type = args.read_token(); // local or world
			if( ( target_type[ 0 ] == 'p' )||( target_type[ 0 ] == 'P' ) )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				qdraw_p->set_target_point( x, y, z );
			}
#endif
			char *qdraw_cmd = args.read_token();
			if( strcmp( qdraw_cmd, "target" ) == 0 )	{
			
				char *target_type = args.read_token();
				if( strcmp( target_type, "point" ) == 0 )	{
					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					qdraw_p->set_target_point( x, y, z );
					return( CMD_SUCCESS );
				}
#if 0
				else
				if( strcmp( target_type, "joint" ) == 0 )	{
					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					char* joint_name = args.read_token();
					qdraw_p->set_target_joint( x, y, z, joint_name );
					return( CMD_SUCCESS );
				}
#endif
			}
			if( strcmp( qdraw_cmd, "aimoff" ) == 0 )	{
				float p = args.read_float();
				float h = args.read_float();
				float r = args.read_float();
				qdraw_p->set_aim_offset( p, h, r );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "dur" ) == 0 )	{
				float dur = args.read_float();
				if( args.calc_num_tokens() > 0 )	{
					float dur2 = args.read_float();
					qdraw_p->set_motion_duration( dur, dur2 );
				}
				else	{
					qdraw_p->set_motion_duration( dur );
				}
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "smooth" ) == 0 )	{
				float sm = args.read_float();
				qdraw_p->set_smooth( sm );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "track" ) == 0 )	{
				float dur = args.read_float();
				qdraw_p->set_track_duration( dur );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "persist" ) == 0 )	{
				qdraw_p->set_track_duration( -1.0 );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "reholster" ) == 0 )	{
				qdraw_p->set_track_duration( 0.0 );
				return( CMD_SUCCESS );
			}
			LOG( "mcu_quickdraw_controller_func ERR: command '%s' NOT RECOGNIZED\n", qdraw_cmd );
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

int mcu_gaze_limit_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )
	{
		int n = args.calc_num_tokens();
		int keyIndex = -1;
		if (n > 0)
		{
			char *key_label = args.read_token();
			keyIndex = MeCtGaze::key_index( key_label );
			if (keyIndex == -1)
			{
					LOG("%s is not a valid key.", key_label);
					return CMD_FAILURE;
			}
		}
		
		if( n > 3 )
		{
			float p_up = args.read_float();
			float p_dn = args.read_float();
			float h = args.read_float();
			float r = args.read_float();
			
			MeCtGaze::set_all_limits(keyIndex, p_up, p_dn, h, r );
		}
		else if (n == 1 || n == 0)
		{
			LOG("To set limits, use: gazelimits <lumbar|thorax|cervical|cranial|optical|back|chest|neck|head|eyes> pitchup pitchdown heading roll");
			int start = 0; 
			int finish = MeCtGaze::GAZE_KEY_EYES;
			if (n == 1)
				start = finish = keyIndex;
			for (int x = start; x <= finish; x++)
			{
				char* label = MeCtGaze::key_label(x);
				// show the limits for that particular key
				LOG("%s Pitch up  : %f ", label, MeCtGaze::DEFAULT_LIMIT_PITCH_UP[x]);
				LOG("%s Pitch down: %f", label, MeCtGaze::DEFAULT_LIMIT_PITCH_DOWN[x]);
				LOG("%s Heading   : %f", label, MeCtGaze::DEFAULT_LIMIT_HEADING[x]);
				LOG("%s Roll      : %f", label, MeCtGaze::DEFAULT_LIMIT_ROLL[x]);
			}
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/*
	gaze <> target point <x y z>
	gaze <> target euler <p h r>
	gaze <> offset euler <p h r>
	gaze <> smooth <basis>
	gaze <> speed <deg-per-sec>
	gaze <> bias <key> <p h r>
	gaze <> limit <key> <p h r>
	gaze <> limit <key> <p-up p-dn h r>
	gaze <> blend <key> <weight>
	gaze <> priority <key>
	gaze <> fadein|fadeout [<interval>]
*/

int mcu_gaze_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		MeCtGaze *gaze_p= mcu_p->gaze_ctrl_map.lookup( ctrl_name );
		if( gaze_p )	{
			
			char *gaze_cmd = args.read_token();
			if( strcmp( gaze_cmd, "target" ) == 0 )	{

				char *target_type = args.read_token();
				if( strcmp( target_type, "point" ) == 0 )	{

					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					gaze_p->set_target( x, y, z );
//					gaze_p->set_target_point( x, y, z );
					return( CMD_SUCCESS );
				}
				if( strcmp( target_type, "euler" ) == 0 )	{

					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_orient( p, h, r );
//					gaze_p->set_target_euler( p, h, r );
					return( CMD_SUCCESS );
				}
				LOG( "mcu_gaze_controller_func ERR: target '%s' NOT RECOGNIZED\n", target_type );
				return( CMD_NOT_FOUND );
			}
			if( strcmp( gaze_cmd, "offset" ) == 0 )	{

				char *offset_type = args.read_token();
				if( strcmp( offset_type, "euler" ) == 0 )	{

					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_offset_euler( p, h, r );
					return( CMD_SUCCESS );
				}
				LOG( "mcu_gaze_controller_func ERR: offset '%s' NOT RECOGNIZED\n", offset_type );
				return( CMD_NOT_FOUND );
			}
			if( strcmp( gaze_cmd, "smooth" ) == 0 )	{

				float sm = args.read_float();
				gaze_p->set_smooth( sm );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "speed" ) == 0 )	{

				float sp = args.read_float();
				gaze_p->set_speed( sp );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "bias" ) == 0 )	{

				char *key_label = args.read_token();
				float p = args.read_float();
				float h = args.read_float();
				float r = args.read_float();
				gaze_p->set_bias( MeCtGaze::key_index( key_label ), p, h, r );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "limit" ) == 0 )	{

				char *key_label = args.read_token();
				int n = args.calc_num_tokens();
				if( n > 3 ) {
					float p_up = args.read_float();
					float p_dn = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_limit( MeCtGaze::key_index( key_label ), p_up, p_dn, h, r );
				}
				else	{
					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_limit( MeCtGaze::key_index( key_label ), p, h, r );
				}
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "blend" ) == 0 )	{

				char *key_label = args.read_token();
				float w = args.read_float();
				gaze_p->set_blend( MeCtGaze::key_index( key_label ), w );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "priority" ) == 0 )	{

				char *key_label = args.read_token();
				gaze_p->set_task_priority( MeCtGaze::key_index( key_label ) );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "fadein" ) == 0 )	{

				float interval = args.read_float();
				gaze_p->set_fade_in( interval );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "fadeout" ) == 0 )	{

				float interval = args.read_float();
				gaze_p->set_fade_out( interval );
				return( CMD_SUCCESS );
			}
			LOG( "mcu_gaze_controller_func ERR: command '%s' NOT RECOGNIZED\n", gaze_cmd );
			return( CMD_NOT_FOUND );
		}
		LOG( "mcu_gaze_controller_func ERR: MeCtGaze '%s' NOT FOUND\n", ctrl_name );
	}
	return( CMD_FAILURE );
}

/*
	snod <> <dur-sec> <mag-deg> [<reps=1.0> [<affirm=1>]]
*/

int mcu_snod_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p )	{
		char *snod_ctrl_name = args.read_token();
		float dur = args.read_float();
		float mag = args.read_float();
		float rep = 1.0;
		int aff = TRUE;
		int n = args.calc_num_tokens();
		if( n )	{
			rep = args.read_float();
			if( n == 2 )	{
				aff = args.read_int();
			}
		}
		MeCtSimpleNod *snod_p = mcu_p->snod_ctrl_map.lookup( snod_ctrl_name );
		if( snod_p )	{
			snod_p->set_nod( dur, mag, rep, aff );
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

int mcu_lilt_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p) {

	if( mcu_p ) {
		char *lilt_ctrl_name = args.read_token();
		float dur = args.read_float();
		float mag = args.read_float();
		MeCtAnkleLilt *lilt_p= mcu_p->lilt_ctrl_map.lookup( lilt_ctrl_name );
		if( lilt_p ) {
			lilt_p->set_lilt( dur,mag,0);
			return(CMD_SUCCESS);
		}

	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

int add_controller_to_scheduler( 
	char *sched_ctrl_name, 
	char *add_ctrl_name, 
	float T_at, 
	mcuCBHandle *mcu_p
)	{

	MeCtScheduler2 *sched_p = mcu_p->sched_ctrl_map.lookup( sched_ctrl_name );
	if( sched_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( add_ctrl_name );
		if( ctrl_p )	{
			sched_p->schedule( 
				ctrl_p, 
				T_at, 
				T_at + ctrl_p->controller_duration(),
				ctrl_p->indt(), 
				ctrl_p->outdt()
			);
			return( CMD_SUCCESS );
		}
	}

	return( CMD_FAILURE );
}

int add_controller_to_scheduler( 
	char *sched_ctrl_name, 
	char *add_ctrl_name, 
	float T_at, 
	float ease_in, 
	float ease_out, 
	mcuCBHandle *mcu_p
)	{
	
	MeCtScheduler2 *sched_p = mcu_p->sched_ctrl_map.lookup( sched_ctrl_name );
	if( sched_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( add_ctrl_name );
		if( ctrl_p )	{
			sched_p->schedule( 
				ctrl_p, 
				T_at, 
				T_at + ctrl_p->controller_duration(),
				ease_in, 
				ease_out
			);
			return( CMD_SUCCESS );
		}
	}

	return( CMD_FAILURE );
}

/*

	sched <> add <ctrl-name> <at> [<ease-in> [<ease-out>]] 

*/

int mcu_sched_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p )	{
		char *sched_ctrl_name = args.read_token();
		char *sched_ctrl_cmd = args.read_token();
		
		if( strcmp( sched_ctrl_cmd, "add" ) == 0 )	{
			char *add_ctrl_name = args.read_token();
			float T_at = args.read_float();
			int n = args.calc_num_tokens();
			if( n )	{
				float ease_in = args.read_float();
				float ease_out = args.read_float();
				return(
					add_controller_to_scheduler( 
						sched_ctrl_name, 
						add_ctrl_name, 
						T_at, 
						ease_in, 
						ease_out, 
						mcu_p
					)
				);
			}
			else	{
				return(
					add_controller_to_scheduler( 
						sched_ctrl_name, 
						add_ctrl_name, 
						T_at, 
						mcu_p
					)
				);
			}
		}
		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*

	load ME | content <me-file>
	load motion <file-path> [-R]
	load pose <file-path> [-R]
	
*/

int mcu_load_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	if( mcu_p )	{
		char *load_cmd = args.read_token();
		
		if( strcmp( load_cmd, "motion" )==0 ||
		    strcmp( load_cmd, "motions" )==0 )
		{
			const char* token = args.read_token();

			bool recursive = false;
			if( strcmp( token, "-R" )==0 ) {
				recursive = true;
				token = args.read_token();
			}
			return mcu_p->load_motions( token, recursive );
		} else if( strcmp( load_cmd, "pose" )==0 ||
		           strcmp( load_cmd, "poses" )==0 )
		{
			const char* token = args.read_token();

			bool recursive = false;
			if( strcmp( token, "-R" )==0 ) {
				recursive = true;
				token = args.read_token();
			}
			return mcu_p->load_poses( token, recursive);
		}

		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}

/*

   net reset function -
   helpful in resyncing TCP socket connection to unreal if it breaks
	<EMF>
*/


int mcu_net_reset( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	bool ret;
	mcu_p->bonebus.CloseConnection();
	if (mcu_p->net_host)
		ret = mcu_p->bonebus.OpenConnection(mcu_p->net_host);
	
	if (!ret)
		return (CMD_SUCCESS);
	else
		return (CMD_FAILURE);
}
/*

   net boneupdates <0|1>
   net worldoffsetupdates <0|1>

*/

int mcu_net_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

   if( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "boneupdates" ) == 0 )
      {
         // turns on/off sending character bone information across the network
         // global setting that affects all characters and bones

         int enable = args.read_int();
         mcu_p->net_bone_updates = enable ? true : false;
         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "worldoffsetupdates" ) == 0 )
      {
         // turns on/off sending character world offset information across the network
         // global setting that affects all characters

         int enable = args.read_int();
         mcu_p->net_world_offset_updates = enable ? true : false;
         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*

   PlaySound <sound_file> <character_id>  // if sound_file starts with '<drive>:' - uses absolute path
                                          // if not - uses relative path, prepends absolute path of top-level saso directory to string
                                          // character_id can be used to position the sound where a character is in the world
*/

int mcu_play_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * remainder = args.read_remainder_raw();
      string sArgs = remainder;
      vector< string > splitArgs;
      vhcl::Tokenize( sArgs, splitArgs );

      if ( splitArgs.size() >= 2 )
      {
         string soundFile = splitArgs[ 0 ];
         string characterName = splitArgs[ 1 ];

         // check for double-quotes around sound file
         if ( soundFile.length() > 1 )
         {
            if ( soundFile[ 0 ] == '\"' )
            {
               size_t first = sArgs.find_first_of( "\"" );
               if ( first == string::npos )
               {
                  LOG( "Error parsing PlaySound message ''", sArgs.c_str() );
                  return CMD_FAILURE;
               }

               size_t second = sArgs.find_first_of( "\"", first + 1 );
               if ( second == string::npos )
               {
                  LOG( "Error parsing PlaySound message ''", sArgs.c_str() );
                  return CMD_FAILURE;
               }

               soundFile = sArgs.substr( first + 1, second - first - 1 );
               characterName = sArgs.substr( second + 2 );
            }
         }

         bool absolutePath = false;

         if ( soundFile.length() > 1 )
         {
            if ( soundFile[ 0 ] == '\\' ||
                 soundFile[ 0 ] == '/' ||
                 soundFile[ 1 ] == ':' )
            {
               absolutePath = true;
            }
         }

         if ( !absolutePath )
         {
            char full[ _MAX_PATH ];
            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            {
               soundFile = string( full ) + string( "\\" ) + soundFile;
            }
         }

         if ( mcu_p->play_internal_audio )
         {
            AUDIO_Play( soundFile.c_str() );
         }
         else
         {
            mcu_p->bonebus.SendPlaySound( soundFile.c_str(), characterName.c_str() );
         }

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*

   StopSound <sound_file>  // if sound_file starts with '<drive>:' - uses absolute path
                           // if not - uses relative path, prepends absolute path of top-level saso directory to string

*/

int mcu_stop_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * remainder = args.read_remainder_raw();
      string sArgs = remainder;
      vector< string > splitArgs;
      vhcl::Tokenize( sArgs, splitArgs );

      if ( splitArgs.size() >= 2 )
      {
         string soundFile = splitArgs[ 0 ];
         string characterName = splitArgs[ 1 ];

         // check for double-quotes around sound file
         if ( soundFile.length() > 1 )
         {
            if ( soundFile[ 0 ] == '\"' )
            {
               size_t first = sArgs.find_first_of( "\"" );
               if ( first == string::npos )
               {
                  LOG( "Error parsing StopSound message ''", sArgs.c_str() );
                  return CMD_FAILURE;
               }

               size_t second = sArgs.find_first_of( "\"", first + 1 );
               if ( second == string::npos )
               {
                  LOG( "Error parsing StopSound message ''", sArgs.c_str() );
                  return CMD_FAILURE;
               }

               soundFile = sArgs.substr( first + 1, second - first - 1 );
               characterName = sArgs.substr( second + 2 );
            }
         }

         bool absolutePath = false;

         if ( soundFile.length() > 1 )
         {
            if ( soundFile[ 0 ] == '\\' ||
                 soundFile[ 0 ] == '/' ||
                 soundFile[ 1 ] == ':' )
            {
               absolutePath = true;
            }
         }

         if ( !absolutePath )
         {
            char full[ _MAX_PATH ];
            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            {
               soundFile = string( full ) + string( "\\" ) + soundFile;
            }
         }

         mcu_p->bonebus.SendStopSound( soundFile.c_str() );

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*
   uscriptexec <uscript command>  - Passes command straight through to Unreal where it executes the given script command
                                  - This function existed in dimr, and is only supplied because other groups were using this command
                                  - and wish to keep using it, but don't want to have to run dimr (because of license issues).
*/

int mcu_uscriptexec_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      int num = args.calc_num_tokens();

      if ( num > 0 )
      {
         string command;

         command += args.read_token();

         int i = 0;
         while ( ++i < num )
         {
            command += " ";
            command += args.read_token();
         }

         //SendWinsockExecScript( command.c_str() );
         mcu_p->bonebus.ExecScript( command.c_str() );

         return CMD_SUCCESS;
      }
   }

   return CMD_FAILURE;
}


/*
   EDF - Temporary CommAPI hooks into the CommAPI.

   CommAPI setcameraposition <x> <y> <z>
      Sets the camera's position in the world.
      x,y,z = Unreal world coordinates to set the object to.

   CommAPI setcamerarotation <x> <y> <z>
      Sets the camera's rotation in the world.
      x,y,z = Orientation in degrees.  (default coord system would match x,y,z to r,h,p
*/

int mcu_commapi_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "setcameraposition" ) == 0 )
      {
         float x = args.read_float();
         float y = args.read_float();
         float z = args.read_float();

         mcu_p->bonebus.SetCameraPosition( x, y, z );

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setcamerarotation" ) == 0 )
      {
         float x = args.read_float();
         float y = args.read_float();
         float z = args.read_float();

         quat_t q = euler_t( x, y, z );

         mcu_p->bonebus.SetCameraRotation( (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() );

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}



/*
   vrKillComponent sbm
   vrKillComponent all
      Kills the sbm process
*/

int mcu_vrKillComponent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "sbm" ) == 0 ||
           _stricmp( command, "all" ) == 0 )
      {
	      mcu_p->loop = false;
	      return CMD_SUCCESS;
      }
   }

	return CMD_SUCCESS;
}


int mcu_vrQuery_func( srArgBuffer& args, mcuCBHandle* mcu_p )
{
	string command = args.read_token();
	if( strcmp(command.c_str(),"anims") && strcmp(command.c_str(),"poses") ) {
		LOG("ERROR: Invalid query command");
		return CMD_FAILURE;
	}

	if( !strcmp(command.c_str(),"anims") )
	{
		string message;
		message.append("vrQueryAnimReply ");

		for (std::map<std::string, SkMotion*>::iterator it = mcu_p->motion_map.begin(); it != mcu_p->motion_map.end(); ++it)
		{
			LOG("\n%s\n",(*it).first.c_str());
			message.append((*it).first);
			message.append(" ");
		}

		mcu_p->vhmsg_send(message.c_str());

		return (CMD_SUCCESS);
	}
	else if( !strcmp(command.c_str(),"poses") )
	{
		string message;
		message.append("vrQueryPoseReply ");

		for (std::map<std::string, SkPosture*>::iterator it = mcu_p->pose_map.begin(); it != mcu_p->pose_map.end(); ++it)
		{
			LOG("\n%s\n",(*it).first.c_str());
			message.append((*it).first);
			message.append(" ");
		}

		mcu_p->vhmsg_send(message.c_str());

		return (CMD_SUCCESS);
	}

	return CMD_FAILURE;

}


/*
   vrAllCall
     In response to this message, send out vrComponent to indicate that this component is running
*/

int mcu_vrAllCall_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
    if ( mcu_p )
    {
	    // Anm - All of the following need to identify mcu_p->process_id
		//       but that breaks the following syntax
        mcu_p->vhmsg_send( "vrComponent sbm" );
 
        // EDF - For our reply, we're going to send one vrComponent 
        //       message for each agent loaded
        SbmCharacter * char_p;
        mcu_p->character_map.reset();
        while ( char_p = mcu_p->character_map.next() )
        {
            string message = "sbm ";
            message += char_p->name;
            mcu_p->vhmsg_send( "vrComponent", message.c_str() );
        }
    }

	return CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////

/*
	Print contents of 
		srHashMap <SkPosture>		pose_map;
		srHashMap <SkMotion>		motion_map;
		srHashMap <MeCtPose>		pose_ctrl_map;
		srHashMap <MeCtMotion>		motion_ctrl_map;
		srHashMap <MeCtSimpleNod>	snod_ctrl_map;
		srHashMap <MeCtAnkleLilt>	lilt_ctrl_map;
		srHashMap <MeCtScheduler2>	sched_ctrl_map;
		srHashMap <MeController>	controller_map; 
*/

int mcu_divulge_content_func( srArgBuffer& args, mcuCBHandle* mcu_p ) {

	LOG( "POSES:\n" );
	for (std::map<std::string, SkPosture*>::iterator postureIter = mcu_p->pose_map.begin();
		postureIter != mcu_p->pose_map.end();
		postureIter++)
	{
		LOG( "  '%s'\n", (*postureIter).second->name() );
	}
	
	LOG( "MOTIONS:\n" );
	for (std::map<std::string, SkMotion*>::iterator motionIter = mcu_p->motion_map.begin();
		motionIter != mcu_p->motion_map.end();
		motionIter++)
	{
		LOG( "  '%s'\n", (*motionIter).second->name() );
	}
	
	LOG( "POSE CTRL:\n" );
	mcu_p->pose_ctrl_map.reset();
	MeCtPose * pose_ctrl_p;
	while( pose_ctrl_p = mcu_p->pose_ctrl_map.next() )	{
		LOG( "  '%s' : '%s'\n", pose_ctrl_p->name(), pose_ctrl_p->posture_name() );
	}
	
	LOG( "MOTION CTRL:\n" );
	mcu_p->motion_ctrl_map.reset();
	MeCtMotion * mot_ctrl_p;
	while( mot_ctrl_p = mcu_p->motion_ctrl_map.next() )	{
		LOG( "  '%s' : '%s'\n", mot_ctrl_p->name(), mot_ctrl_p->motion()->name() );
	}
	
	LOG( "SIMPLE-NOD:\n" );
	mcu_p->snod_ctrl_map.reset();
	MeCtSimpleNod * snod_p;
	while( snod_p = mcu_p->snod_ctrl_map.next() )	{
		LOG( "  '%s'\n", snod_p->name() );
	}
	
	LOG( "ANKLE-LILT:\n" );
	mcu_p->lilt_ctrl_map.reset();
	MeCtAnkleLilt * lilt_p;
	while( lilt_p = mcu_p->lilt_ctrl_map.next() )	{
		LOG( "  '%s'\n", lilt_p->name() );
	}
	
	LOG( "SCHEDULE:\n" );
	mcu_p->sched_ctrl_map.reset();
	MeCtScheduler2 * sched_p;

	while( sched_p = mcu_p->sched_ctrl_map.next() )	{
		LOG( "  '%s'\n", sched_p->name() );
	}
	
	LOG( "ALL CONTROLLERS:\n" );
	mcu_p->controller_map.reset();
	MeController * ctrl_p;
	while( ctrl_p = mcu_p->controller_map.next() )	{
		LOG( "  '%s'\n", ctrl_p->name() );
	}
	
	return (CMD_SUCCESS);
}

/////////////////////////////////////////////////////////////

int mcu_wsp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	mcu_p->theWSP->process_command( args.read_remainder_raw() );

	return( CMD_SUCCESS );
}

int mcu_syncpolicy_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      int num = args.calc_num_tokens();

      if ( num > 0 )
      {
         string command = args.read_token();
		 if (command == "delay")
		 {
			 mcu_p->delay_behaviors = true;
		 }
		 else if (command == "nodelay")
		 {
			 mcu_p->delay_behaviors = false;
		 }
		 else
		 {
			LOG("Usage: syncpolicy <delay|nodelay>");
			return CMD_FAILURE;
		 }
 
         return CMD_SUCCESS;
      }
	  else
	  {
		  if (mcu_p->delay_behaviors)
		  {
			 LOG("Behavior policy is 'delay'. Behaviors will be offset to a future time to make sure that all behaviors are executed in full.");
		  }
		  else
		  {
			 LOG("Behavior policy is 'nodelay'. Behaviors that are calculated to start in the past will be partially executed.");
		  }
		  return CMD_SUCCESS;
	  }
   }

   return CMD_FAILURE;
}

// Functions:
// 1. connecting the motion with the character, print out the channels inside the motion
// 2. examine whether these channels exist inside the skeleton
// 3. If frame number exist, this will output which channels are affected 
// check motion/skeleton <character name> <motion name> [frame number]
// frame number is optional
int mcu_check_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (mcu_p)
	{
		char* operation = args.read_token();
		char* charName = args.read_token();
		char* motionName = args.read_token();
		char* frameNumString = args.read_token();
		int frameNum = -1;
		if (frameNumString != "")	frameNum = atoi(frameNumString);
		int mode = -1;
		int chanSize;
		SkChannel chan;

		if (strcmp(operation, "motion") == 0)	mode = 1;
		if (strcmp(operation, "skeleton") == 0)	mode = 2;
		if (mode == -1)
		{
			LOG("mcu_check_func ERR: Mode %s not FOUND!", operation);
			return CMD_FAILURE;
		}

		SbmCharacter* character = mcu_p->character_map.lookup(charName);
		SkMotion* motion;
		std::map<std::string, SkMotion*>::iterator motionIter = mcu_p->motion_map.find(motionName);
		if (motionIter != mcu_p->motion_map.end())
			motion = motionIter->second;
		else
		{
			LOG("mcu_check_func ERR: Motion %s NOT EXIST!", motionName);
			return CMD_FAILURE;
		}
		if (character)
		{
			int numValidChannels = motion->connect(character->skeleton_p);	// connect and check for the joints
			SkChannelArray& mChanArray = motion->channels();
			int mChanSize = mChanArray.size();
			SkChannelArray& skelChanArray = character->skeleton_p->channels();
			int skelChanSize = skelChanArray.size();
			if (mode == 1)
			{
				chanSize = mChanSize;
				LOG("Channels in motion %s's channel matching %s's skeleton are preceeded with '+'", motionName, charName);
				LOG("motion %s's Channel Info:", motionName);
			}
			if (mode == 2)
			{
				chanSize = skelChanSize;
				LOG("Channels in skeleton %s's channel matching motion %s's channel are preceeded with '+'", charName, motionName);
				LOG("character %s's Channel Info:", charName);
			}
			LOG("Channel Size: %d", chanSize);
			for (int i = 0; i < chanSize; i++)
			{				
				std::stringstream outputInfo;
				if (mode == 1)	chan = mChanArray[i];
				if (mode == 2)	chan = skelChanArray[i];
				std::string jointName = chan.joint->name().get_string();
				int	chanType = chan.type;
				std::string chanTypeString;
				switch (chanType)
				{
					case 0:
						chanTypeString = "XPos";
						break;
					case 1:	
						chanTypeString = "YPos";
						break;
					case 2:
						chanTypeString = "ZPos";
						break;
					case 6:
						chanTypeString = "Quat";
						break;
					default:
						chanTypeString = "Others";
				}
				int pos;
				if (mode == 1)	pos = skelChanArray.search(chan.joint->name(), chan.type);
				if (mode == 2)	pos = mChanArray.search(chan.joint->name(), chan.type);
				if (pos != -1)
					outputInfo << "+ ";
				if (pos == -1)	
					outputInfo << "  ";
				outputInfo << i << ": " << jointName.c_str() << " (" << chanTypeString << ")";
				LOG("%s", outputInfo.str().c_str());
			}
			
			// check the non-zero channel
			if (frameNum >= 0 && frameNum < motion->frames())
			{
				float* frameData = motion->posture(frameNum);
				int channelCounter = 0;
				for (int c = 0; c < mChanArray.size(); c++)
				{
					int curIndex = channelCounter;
					SkChannel& channel = mChanArray.get(c);
					SkJointName& jointName = mChanArray.name(c);
					int chanSize = channel.size();
					int numZeroFrames = 0;
					int	chanType = channel.type;
					if (chanType == SkChannel::Quat)
					{
						if (fabs(frameData[channelCounter] - 1.0) > .0001 ||
							fabs(frameData[channelCounter + 1]) > .0001 ||
							fabs(frameData[channelCounter + 2]) > .0001 ||
							fabs(frameData[channelCounter + 3]) > .0001)
						{
							numZeroFrames = 0;
						}
						else
						{
							numZeroFrames = 1;
						}
						channelCounter += 4;
					}
					else
					{
						for (int x = 0; x < chanSize; x++)
						{
							if (fabs(frameData[channelCounter]) < .0001)
								numZeroFrames++;
							channelCounter++;
						}
							
					}
										
					if (numZeroFrames == 0)
					{
						if (chanType == SkChannel::XPos || chanType == SkChannel::YPos || chanType == SkChannel::ZPos ||
							chanType == SkChannel::XRot || chanType == SkChannel::YRot || chanType == SkChannel::ZRot || 
							chanType == SkChannel::Twist)
							LOG("Channel %s/%s has non-zero value: %f.", jointName.get_string(), channel.type_name(), frameData[curIndex]);
						else if (chanType == SkChannel::Swing)
							LOG("Channel %s/%s has non-zero value: %f %f.", jointName.get_string(), channel.type_name(), frameData[curIndex], frameData[curIndex + 1]);
						else if (chanType == SkChannel::Quat)
							LOG("Channel %s/%s has non-zero value: %f %f %f %f.", jointName.get_string(), channel.type_name(), frameData[curIndex], frameData[curIndex+ 1], frameData[curIndex + 2], frameData[curIndex + 3]);
					}
				}
				motion->disconnect();
			}
			else
			{
				if (frameNumString != "")
				{
					LOG("mcu_check_func ERR: Frame Number not Exist!");
					motion->disconnect();
					return CMD_FAILURE;			
				}
			}
		}
		else
		{
			LOG("mcu_check_func ERR: Character %s NOT EXIST!", charName);
			return CMD_FAILURE;
		}
	}
	return CMD_SUCCESS;
}
