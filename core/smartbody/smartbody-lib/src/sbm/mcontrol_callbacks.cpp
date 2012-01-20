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
#include "mcontrol_callbacks.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <sbm/SBSkeleton.h>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "sbm_audio.h"

#include "me_utilities.hpp"

#if USE_WSP
#include "wsp.h"
#endif

#include "sr/sr_model.h"
#include "sbm_pawn.hpp"
#include "sbm/Event.h"
#include "sbm/ParserOpenCOLLADA.h"

#include "SteeringAgent.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include "ParserFBX.h"
#include "SBCharacter.h"
#include <sbm/BMLDefs.h>
#include <math.h>


#ifdef USE_GOOGLE_PROFILER
#include <google/profiler.h>
#endif

using namespace std;

#if USE_WSP
using namespace WSP;
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

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
	boost::filesystem::path fullpath = current_path;
	fullpath /= std::string(file_name);
	printf("%s", fullpath.string().c_str());
	return (char*) fullpath.native_file_string().c_str();

	if( file_name == NULL)	return NULL;
	printf("CURRENT PATH=%s, file_name=%s", current_path, file_name);
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
		if( strcmp( path_tok, "seq" ) == 0 )
		{	
			pres->setType("seq");
			mcu_p->seq_paths.insert( path );
		}
		else if( (strcmp( path_tok, "me" ) == 0 ) ||
			   ( strcmp( path_tok, "ME" ) == 0 ))
		{
			pres->setType("me");
			mcu_p->me_paths.insert( path );
		}
		else if(strcmp( path_tok, "audio") == 0 )
		{
			pres->setType("audio");
			// remove the old paths 
			mcu_p->audio_paths = srPathList();
			mcu_p->audio_paths.setPathPrefix(mcu_p->getMediaPath());
			mcu_p->audio_paths.insert( path );
		}
		else if(strcmp( path_tok, "mesh") == 0 )
		{
			pres->setType("mesh");
			mcu_p->mesh_paths.insert( path );
		}
		else
		{
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
		if( strcmp( tok, "mediapath" ) == 0 )	{
			mcu_mediapath_func( args, mcu_p );
			delete [] cmd;
			cmd = NULL;
		}
		else
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

int begin_sequence( char* name, mcuCBHandle* mcu_p )	{
	
	srCmdSeq *seq = mcu_p->lookup_seq( name );
	
	if( seq )
	{
		srCmdSeq* copySeq = new srCmdSeq;
		mcu_preprocess_sequence( copySeq, seq, mcu_p );

		copySeq->offset( (float)( mcu_p->time ) );
		bool success = mcu_p->activeSequences.addSequence(name, copySeq );

		if( !success )
		{
			LOG( "begin_sequence ERR: insert active: '%s' FAILED\n", name ); 
			return CMD_FAILURE;
		}
		
	}

	return( CMD_SUCCESS );
}

/*

	seq <name> at <time> <cmd...>	
	seq <name> [begin|abort|print]
#	seq <name> write

*/

int mcu_sequence_func( srArgBuffer& args, mcuCBHandle* mcu )
{
	if( mcu )	{
		char *seqName = args.read_token();
		char *seqCmd = args.read_token();
		//std::cout << "SEQUENCE LOADED: " << seq_name << " " << seq_cmd << std::endl;
		//std::string seqStr = seq_cmd;
		//LOG("mcu_sequence_func : seq_name = %s, seqStr = %s",seq_cmd, seqStr.c_str());
		if( ( strcmp( seqCmd, "begin" ) == 0 )||( strcmp( seqCmd, EMPTY_STRING ) == 0 ) )	{
			int ret = begin_sequence( seqName, mcu );
			return ret;
		}
		else
		if( strcmp( seqCmd, "at" ) == 0 )	{
		
			srCmdSeq* seq = mcu->pendingSequences.getSequence( seqName );
			if (!seq)
			{
				seq = new srCmdSeq;
				bool success = mcu->pendingSequences.addSequence( seqName, seq );
				if( !success )
				{
					LOG( "mcu_sequence_func ERR: insert pending '%s' FAILED\n", seqName ); 
					return( CMD_FAILURE );
				}
			}
			
			float seqTime = args.read_float();
			char* seqStr = args.read_remainder_raw();
			int ret = seq->insert( seqTime, seqStr );
			return ret;
		}
		else
		if( strcmp( seqCmd, "print" ) == 0 )	{
			
			srCmdSeq* seq = mcu->pendingSequences.getSequence( seqName );
			if (!seq)
			{
				LOG( "mcu_sequence_func ERR: print: '%s' NOT FOUND\n", seqName ); 
				return( CMD_FAILURE );
			}
			seq->print( stdout );
		}
		else
		if( ( strcmp( seqCmd, "abort" ) == 0 )||( strcmp( seqCmd, "delete" ) == 0 ) )	{
			int result = mcu->abortSequence( seqName );
			if( result == CMD_NOT_FOUND )	{
				LOG( "mcu_sequence_func ERR: abort|delete: '%s' NOT FOUND\n", seqName ); 
			}
			return( result );
		}
		else {
			return( CMD_NOT_FOUND );
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
			else {
				mcu_p->viewer_p ->show_viewer();
			}
		}		
		else
		if( strcmp( view_cmd, "close" ) == 0 )	{
			if( mcu_p->viewer_p )	{
				mcu_p->close_viewer();
				return( CMD_SUCCESS );
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
				mcu_p->bmlviewer_p->show_viewer();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( bmlview_cmd, "hide" ) == 0 )	{
			if( mcu_p->bmlviewer_p )	{
				mcu_p->bmlviewer_p->hide_viewer();
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

	panimviewer open <width> <height> <px> <py> 
	panimviewer show|hide
	
*/

int mcu_panimationviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )	{
		char *panimview_cmd = args.read_token();
		if( strcmp( panimview_cmd, "open" ) == 0 )	{

			if( mcu_p->panimationviewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->open_panimation_viewer( width, height, px, py );
					return( err );
				} else {
					int err = mcu_p->open_panimation_viewer( 800, 740, 50, 20 );
					return( err );
				}
			}
		}
		else
		if( strcmp( panimview_cmd, "show" ) == 0 )	{
			if( mcu_p->panimationviewer_p )	{
				mcu_p->panimationviewer_p->show_viewer();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( panimview_cmd, "hide" ) == 0 )	{
			if( mcu_p->panimationviewer_p )	{
				mcu_p->panimationviewer_p->hide_viewer();
				return( CMD_SUCCESS );
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

int mcu_channelbufferviewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )	{
		char *cbufferview_cmd = args.read_token();
		if( strcmp( cbufferview_cmd, "open" ) == 0 )	{

			if( mcu_p->channelbufferviewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->open_channelbuffer_viewer( width, height, px, py );
					return( err );
				} else {
					int err = mcu_p->open_channelbuffer_viewer( 800, 600, 50, 50 );
					return( err );
				}
			}
		}
		else
		if( strcmp( cbufferview_cmd, "show" ) == 0 )	{
			if( mcu_p->channelbufferviewer_p )	{
				mcu_p->channelbufferviewer_p->show_viewer();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( cbufferview_cmd, "hide" ) == 0 )	{
			if( mcu_p->channelbufferviewer_p )	{
				mcu_p->channelbufferviewer_p->hide_viewer();
				return( CMD_SUCCESS );
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}


int mcu_resourceViewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )	{
		char *resourceViewerCmd = args.read_token();
		if( strcmp( resourceViewerCmd, "open" ) == 0 )	{

			if( mcu_p->resourceViewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->openResourceViewer( width, height, px, py );
					return( err );
				} else {
					int err = mcu_p->openResourceViewer( 800, 600, 50, 50 );
					return( err );
				}
			}
		}
		else
			if( strcmp( resourceViewerCmd, "show" ) == 0 )	{
				if( mcu_p->resourceViewer_p )	{
					mcu_p->resourceViewer_p->show_viewer();
					return( CMD_SUCCESS );
				}
			}
			else
				if( strcmp( resourceViewerCmd, "hide" ) == 0 )	{
					if( mcu_p->resourceViewer_p )	{
						mcu_p->resourceViewer_p->hide_viewer();
						return( CMD_SUCCESS );
					}
				}
				else	{
					return( CMD_NOT_FOUND );
				}
	}
	return( CMD_FAILURE );
}


int mcu_faceViewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )	{
		char *faceViewerCmd = args.read_token();
		if( strcmp( faceViewerCmd, "open" ) == 0 )	{

			if( mcu_p->faceViewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->openFaceViewer( width, height, px, py );
					return( err );
				} else {
					int err = mcu_p->openFaceViewer( 480, 650, 50, 50 );
					return( err );
				}
			}
		}
		else
			if( strcmp( faceViewerCmd, "show" ) == 0 )	{
				if( mcu_p->faceViewer_p )	{
					mcu_p->faceViewer_p->show_viewer();
					return( CMD_SUCCESS );
				}
			}
			else
				if( strcmp( faceViewerCmd, "hide" ) == 0 )	{
					if( mcu_p->faceViewer_p )	{
						mcu_p->faceViewer_p->hide_viewer();
						return( CMD_SUCCESS );
					}
				}
				else	{
					return( CMD_NOT_FOUND );
				}
	}
	return( CMD_FAILURE );
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

/**
	panim state <state-name> cycle <cycle> <number of motions> <motion1> <motion2>... <motionN> <number of keys per motion> <keys for motion1> <keys for motion2> <keys for motion3>
	panim state <state-name> parameter <type> <number of motions that have parameters> <motion1> <parameter1> <motion2> <parameter2>
	panim schedule char <char-name> state <state-name> loop <loop> <weight list>
	panim update char <char-name> state <weight list>
	panim transition fromstate <state-name> tostate <state-name> <motion-name1 in first state> <2 keys for motion-name1> <motion-name2 in second state> <2 keys for motion-name2>
**/
double parseMotionParameters(std::string m, std::string parameter, double min, double max)
{
	std::string skeletonName = tokenize(parameter, "|");
	SkSkeleton* sk = NULL;
	if (parameter != "")
	{
		std::map<std::string, SkSkeleton*>::iterator iter = mcuCBHandle::singleton().skeleton_map.find(skeletonName);
		if (iter != mcuCBHandle::singleton().skeleton_map.end())
			sk = iter->second;
		else
			LOG("parseMotionParameters ERR: skeleton %s not found! Parameter won't be setup properly", skeletonName.c_str());
	}
	int type = 0;
	if (parameter == "speed1")
		type = 0;
	if (parameter == "speed2")
		type = 1;
	if (parameter == "angular1")
		type = 2;
	if (parameter == "angular2")
		type = 3;
	if (parameter == "transitionx")
		type = 4;
	if (parameter == "transitiony")
		type = 5;
	if (parameter == "transitionz")
		type = 6;
	if (parameter == "avgrooty")
		type = 7;
	if (!sk) return -9999;
	MotionParameters mParam(mcuCBHandle::singleton().lookUpMotion(m.c_str()), sk, "base");
	mParam.setFrameId(min, max);
	return mParam.getParameter(type);
}

int mcu_motion_mirror_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p )
{
	if (mcu_p)
	{
		std::string refMotionName = args.read_token();
		std::string mirrorMotionName = args.read_token();
		std::string skeletonName = args.read_token();
		if (refMotionName == "" || skeletonName == "")
		{
			LOG("Usage: mirror <from> <to> <skeleton>");
			return CMD_FAILURE;
		}
		
		SkSkeleton* skeleton = mcu_p->_scene->getSkeleton(skeletonName);	
		if (!skeleton)
		{
			LOG("No skeleton named '%s' found. Cannot mirror motion %s.", skeletonName.c_str(), refMotionName.c_str());
			return CMD_FAILURE;
		}
		SkMotion* refMotion = mcu_p->lookUpMotion(refMotionName.c_str());
		if (!refMotion)
		{
			LOG("No motion named '%s' found. Cannot mirror motion %s.", refMotionName.c_str(), refMotionName.c_str());
			return CMD_FAILURE;
		}
		if (refMotion && skeleton)
		{
#if 1
			std::map<std::string, SkMotion*>& map = mcu_p->motion_map;
			SkMotion* mirrorMotion = refMotion->buildMirrorMotion(skeleton);
			if (mirrorMotionName == EMPTY_STRING)
				mirrorMotionName = refMotionName + "_mirror";
			mirrorMotion->setName(mirrorMotionName.c_str());
			map.insert(std::pair<std::string, SkMotion*>(mirrorMotionName, mirrorMotion));			
#endif
			return CMD_SUCCESS;
		}		
	}
	return CMD_SUCCESS;
}


int mcu_physics_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (mcu_p)
	{
		std::string operation = args.read_token();
		if (operation == "enable")
		{
			//mcu_p->setPhysicsEngine(true);
			mcu_p->_scene->getPhysicsManager()->setEnable(true);
			return CMD_SUCCESS;
		}
		else if (operation == "disable")
		{
			//mcu_p->setPhysicsEngine(false);	
			mcu_p->_scene->getPhysicsManager()->setEnable(false);
			return CMD_SUCCESS;
		}
		else if (operation == "gravity")
		{
			float gravity = args.read_float();
			//if (gravity > 0.f)
			//	mcu_p->physicsEngine->setGravity(gravity);		
			mcu_p->_scene->getPhysicsManager()->getPhysicsEngine()->setGravity(gravity);
		}
	}
	return CMD_SUCCESS;
}

int mcu_panim_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (mcu_p)
	{
		std::string operation = args.read_token();
		if (operation == "enable")
		{
			LOG("Command: 'panim enable' has been deprecated.");
			return CMD_FAILURE;
		}
		if (operation == "state")
		{
			std::string stateName = args.read_token();
			PAStateData* newState = new PAStateData(stateName);
			std::string nextString = args.read_token();
			if (nextString == "cycle")
			{
				std::string cycle = args.read_token();
				if (cycle == "true")
					newState->cycle = true;
				if (cycle == "false")
					newState->cycle = false;
				int numMotions = args.read_int();
				for (int i = 0; i < numMotions; i++)
				{
					std::string motionName = args.read_token();
					std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionName);
					if (iter == mcu_p->motion_map.end())
						return CMD_FAILURE;
					newState->motions.push_back(iter->second);
				}
				int numKeys = args.read_int();
				for (int i = 0; i < numMotions; i++)
				{
					std::vector<double> keysForOneMotion;
					if (numKeys == 0)
					{
						keysForOneMotion.push_back(0.0);
						keysForOneMotion.push_back(newState->motions[i]->duration());
					}
					else
					{
						for (int j = 0; j < numKeys; j++)
						{
							double key = args.read_double();
							keysForOneMotion.push_back(key);
						}
						for (int j = 0; j < numKeys - 1; j++)
						{
							if (keysForOneMotion[j] > keysForOneMotion[j + 1])
								keysForOneMotion[j + 1] += newState->motions[i]->duration();
						}
					}
					newState->keys.push_back(keysForOneMotion);
				}
				for (int i = 0; i < numMotions; i++)
				{
					if (i == 0)
						newState->weights.push_back(1.0);
					else
						newState->weights.push_back(0.0);
				}
				mcu_p->addPAState(newState);
			}
			else if (nextString == "parameter")
			{
				PAStateData* state = mcu_p->lookUpPAState(stateName);
				if (!state) return CMD_FAILURE;
				std::string type = args.read_token();
				if (type == "1D") state->paramManager->setType(0);
				else if (type == "2D") state->paramManager->setType(1);
				else if (type == "3D") state->paramManager->setType(2);
				else return CMD_FAILURE;
				int num = args.read_int();
				for (int i = 0; i < num; i++)
				{
					std::string m = args.read_token();
					if (type == "1D")
					{
						std::string parameter = args.read_token();
						int motionId = state->getMotionId(m);
						if (motionId < 0) return CMD_FAILURE;
						double param = parseMotionParameters(m, parameter, state->keys[motionId][0], state->keys[motionId][state->getNumKeys() - 1]);
						if (param < -9000) param = atof(parameter.c_str());
						state->paramManager->setParameter(m, param);
					}
					else if (type == "2D")
					{
						std::string parameterX = args.read_token();
						std::string parameterY = args.read_token();
						double paramX = parseMotionParameters(m, parameterX, state->keys[state->getMotionId(m)][0], state->keys[state->getMotionId(m)][state->getNumKeys() - 1]);
						double paramY = parseMotionParameters(m, parameterY, state->keys[state->getMotionId(m)][0], state->keys[state->getMotionId(m)][state->getNumKeys() - 1]);
						if (paramX < -9000) paramX = atof(parameterX.c_str());
						if (paramY < -9000) paramY = atof(parameterY.c_str());
						state->paramManager->setParameter(m, paramX, paramY);
					}
					else if (type == "3D")
					{
						double param[3];
						for (int pc = 0; pc < 3; pc++)
						{
							std::string para = args.read_token();
							param[pc] = parseMotionParameters(m, para, state->keys[state->getMotionId(m)][0], state->keys[state->getMotionId(m)][state->getNumKeys() - 1]);
							if (param[pc] < -9000) param[pc] = atof(para.c_str());
						}
						state->paramManager->setParameter(m, param[0], param[1], param[2]);
					}
				}
//				if (type == "3D")
//					state->paramManager->buildTetrahedron();
			}
			else if (nextString == "triangle")
			{
				PAStateData* state = mcu_p->lookUpPAState(stateName);
				if (!state) return CMD_FAILURE;
				int numTriangles = args.read_int();
				for (int i = 0; i < numTriangles; i++)
				{
					std::string motion1 = args.read_token();
					std::string motion2 = args.read_token();
					std::string motion3 = args.read_token();
					state->paramManager->addTriangle(motion1, motion2, motion3);
				}
			}
			else if (nextString == "tetrahedron")
			{ 
				PAStateData* state = mcu_p->lookUpPAState(stateName);
				if (!state) return CMD_FAILURE;
				int numTetrahedrons = args.read_int();
				for (int i = 0; i < numTetrahedrons; i++)
				{
					std::string motion1 = args.read_token();
					std::string motion2 = args.read_token();
					std::string motion3 = args.read_token();
					std::string motion4 = args.read_token();
					state->paramManager->addTetrahedron(motion1, motion2, motion3, motion4);
				}				
			}
			else
				return CMD_FAILURE;
		}
		else if (operation == "schedule" || operation == "unschedule" || operation == "update" ||  operation == "updatestate" || operation == "basename")
		{
			std::string charString = args.read_token();
			if (charString != "char")
				return CMD_FAILURE;
			SbmCharacter* character = mcu_p->getCharacter(args.read_token());
			if (!character)
				return CMD_FAILURE;
			if (!character->param_animation_ct)
			{
				LOG("Parameterized Animation Not Enabled!");
				return CMD_SUCCESS;
			}

			if (operation == "schedule")
			{
				std::string stateString = args.read_token();
				if (stateString != "state")
					return CMD_FAILURE;
				std::string stateName = args.read_token();
				PAStateData* state = mcu_p->lookUpPAState(stateName);
				if (!state)
					LOG("State %s not exist, schedule Idle State.", stateName.c_str());
				std::string loopString = args.read_token();
				if (loopString != "loop")
					return CMD_FAILURE;
				std::string loop = args.read_token();
				bool l = true;
				if (loop == "true") l = true;
				if (loop == "false") l = false;
				std::string playNowString = args.read_token();
				if (playNowString != "playnow")
					return CMD_FAILURE;
				bool pn = false;
				std::string playNow = args.read_token();
				if (playNow == "true") pn = true;
				else if (playNow == "false") pn = false;
				else 
					return CMD_FAILURE;
				int numWeights = args.calc_num_tokens();
				if (numWeights > 0)
				{
					if (state)
					{
						for (int i = 0; i < state->getNumMotions(); i++)
							state->weights[i] = args.read_double();
					}
				}
				character->param_animation_ct->schedule(state, l, pn);
			}

			if (operation == "unschedule")
				character->param_animation_ct->unschedule();

			if (operation == "update")
			{
				std::vector<double> w;
				if (args.calc_num_tokens() == character->param_animation_ct->getNumWeights())
				{
					for (int i = 0; i < character->param_animation_ct->getNumWeights(); i++)
						w.push_back(args.read_double());
					character->param_animation_ct->updateWeights(w);
				}
			}
			if (operation == "updatestate")
			{
				if (!character->param_animation_ct->getCurrentPAStateData())
				{
					LOG("No state available for character %s.", character->getName().c_str());
					return CMD_FAILURE;
				}

				int type = character->param_animation_ct->getCurrentPAStateData()->paramManager->getType();
				if (args.calc_num_tokens() != type + 1)
				{
					LOG("Cannot update state %s for character %s which has %d parameters, you sent %d.", character->param_animation_ct->getName().c_str(), character->getName().c_str(), (type + 1), args.calc_num_tokens());
					return CMD_FAILURE;
				}
				std::vector<double> w;
				for (int i = 0; i < (type + 1); i++)
					w.push_back(args.read_double());
				if (type == 0)
					character->param_animation_ct->getCurrentPAStateData()->paramManager->setWeight(w[0]);
				else if (type == 1)
					character->param_animation_ct->getCurrentPAStateData()->paramManager->setWeight(w[0], w[1]);
				else if (type == 2)
					character->param_animation_ct->getCurrentPAStateData()->paramManager->setWeight(w[0], w[1], w[2]);
				character->param_animation_ct->updateWeights();
				return CMD_SUCCESS;
			}


			if (operation == "basename")
				character->param_animation_ct->setBaseJointName(args.read_token());
		}
		else if (operation == "transition")
		{
			PATransitionData* newTransition = new PATransitionData();
			std::string fromStateString = args.read_token();
			if (fromStateString != "fromstate")
				return CMD_FAILURE;
			PAStateData* fromState = mcu_p->lookUpPAState(args.read_token());
			if (!fromState)
				return CMD_FAILURE;
			newTransition->fromState = fromState;
			std::string toStateString = args.read_token();
			if (toStateString != "tostate")
				return CMD_FAILURE;
			PAStateData* toState = mcu_p->lookUpPAState(args.read_token());
			if (!toState)
				return CMD_FAILURE;
			newTransition->toState = toState;
			if (args.calc_num_tokens() == 2)
			{
				newTransition->fromMotionName = args.read_token();
				newTransition->easeOutStart.push_back(mcu_p->lookUpMotion(newTransition->fromMotionName.c_str())->duration() - defaultTransition);
				newTransition->easeOutEnd.push_back(mcu_p->lookUpMotion(newTransition->fromMotionName.c_str())->duration());
				newTransition->toMotionName = args.read_token();
				newTransition->easeInStart = 0.0;
				newTransition->easeInEnd = defaultTransition;
			}
			else
			{
				newTransition->fromMotionName = args.read_token();
				int numOfEaseOut = args.read_int();
				for (int i = 0; i < numOfEaseOut; i++)
				{
					newTransition->easeOutStart.push_back(args.read_double());
					newTransition->easeOutEnd.push_back(args.read_double());
				}
				newTransition->toMotionName = args.read_token();
				newTransition->easeInStart = args.read_double();
				newTransition->easeInEnd = args.read_double();
				mcu_p->addPATransition(newTransition);
			}
			fromState->toStates.push_back(toState);
			toState->fromStates.push_back(fromState);
		}
		else
		{
			LOG("mcu_panim_cmd_func ERR: operation not valid.");
			return CMD_FAILURE;
		}
	}
	return CMD_SUCCESS;
}

int mcu_motion_player_func(srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (mcu_p)
	{
		std::string charName = args.read_token();
		SbmCharacter* character = mcu_p->getCharacter(charName);
		if (character)
		{
			std::string next = args.read_token();
			if (next == "on")
				character->motionplayer_ct->setActive(true);
			else if (next == "off")
				character->motionplayer_ct->setActive(false);
			else
			{
				double frameNumber = args.read_double();
				character->motionplayer_ct->init(character,next, frameNumber);
			}
			return CMD_SUCCESS;
		}
		return CMD_FAILURE;
	}
	return CMD_FAILURE;
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
				SbmPawn* pawn = mcu_p->getPawn(name);
				if (!pawn)
				{
					pawn = mcu_p->getCharacter(name);
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
				skeleton = pawn->getSkeleton();

				SkJoint* joint = pawn->getSkeleton()->search_joint(jointName);
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
				cameraTrack->yPos = jointMat[13];
				cameraTrack->threshold = 100.0;
				SrCamera cam;
				cameraTrack->jointToCamera = mcu_p->viewer_p->get_camera()->eye - jointPos;
				LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
				cameraTrack->targetToCamera = mcu_p->viewer_p->get_camera()->eye - mcu_p->viewer_p->get_camera()->center;
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
				mcu_p->execute((char*)"camera eye 0 166 185");
				mcu_p->execute((char*)"camera center 0 92 0");
			}
			else if (strcmp( cam_cmd, "frame" ) == 0 ) {
				SrBox sceneBox;
				SrCamera* camera = mcu_p->viewer_p->get_camera();
				for (std::map<std::string, SbmPawn*>::iterator iter = mcu_p->getPawnMap().begin();
					iter != mcu_p->getPawnMap().end();
					iter++)
				{
					SrBox box = (*iter).second->getSkeleton()->getBoundingBox();
					sceneBox.extend(box);
				}

				camera->view_all(sceneBox, camera->fovy);
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
				mcu_p->height_field_p->load( (char*)"../../../../data/terrain/range1.e.ppm" );
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
			mcu_p->switch_internal_timer(); 
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

int mcu_character_load_mesh(const char* char_name, const char* obj_file, mcuCBHandle *mcu_p, const char* option)
{
	SbmCharacter* char_p = mcu_p->getCharacter( char_name );
	if( !char_p )	
	{
		LOG( "mcu_character_load_mesh ERR: SbmCharacter '%s' NOT FOUND\n", char_name ); 
		return( CMD_FAILURE );
	}
	float factor = 1.f;
	if (option)
	{
		if (strcmp(option,"-m") == 0)
		{
			factor = 0.01f;
		}
	}

	// Here, detect which type of file it is
	std::string ext = boost::filesystem2::extension(obj_file);
	std::string file = boost::filesystem::basename(obj_file);	
	std::vector<SrModel*> meshModelVec;
	if (ext == ".obj" || ext == ".OBJ")
	{
		SrModel* objModel = new SrModel();
		if (!objModel->import_obj(obj_file))
		{
			LOG( "Could not load mesh from file '%s'", obj_file);
			delete objModel;
			return( CMD_FAILURE );
		}
		meshModelVec.push_back(objModel);
	}
	if (ext == ".dae" || ext == ".DAE" || ext == ".xml" || ext == ".XML")
	{
		DOMNode* geometryNode = ParserOpenCOLLADA::getNode("library_geometries", obj_file);
		if (geometryNode)
		{
			// first from library visual scene retrieve the material id to name mapping (TODO: needs reorganizing the assets management)
			std::map<std::string, std::string> materialId2Name;
			DOMNode* visualSceneNode = ParserOpenCOLLADA::getNode("library_visual_scenes", obj_file);
			if (!visualSceneNode)
				LOG("mcu_character_load_mesh ERR: .dae file doesn't contain correct geometry information.");
			SkSkeleton skeleton;
			SkMotion motion;
			int order;
			ParserOpenCOLLADA::parseLibraryVisualScenes(visualSceneNode, skeleton, motion, 1.0, order, materialId2Name);

			// get picture id to file mapping
			std::map<std::string, std::string> pictureId2File;
			DOMNode* imageNode = ParserOpenCOLLADA::getNode("library_images", obj_file);
			if (imageNode)
				ParserOpenCOLLADA::parseLibraryImages(imageNode, pictureId2File);

			// start parsing mateiral
			std::map<std::string, std::string> effectId2MaterialId;
			DOMNode* materialNode = ParserOpenCOLLADA::getNode("library_materials", obj_file);
			if (materialNode)
				ParserOpenCOLLADA::parseLibraryMaterials(materialNode, effectId2MaterialId);

			// start parsing effect
			SrArray<SrMaterial> M;
			SrStringArray mnames;
			std::map<std::string,std::string> mtlTextMap;
			std::map<std::string,std::string> mtlTextBumpMap;
			DOMNode* effectNode = ParserOpenCOLLADA::getNode("library_effects", obj_file);
			if (effectNode)
			{
				ParserOpenCOLLADA::parseLibraryEffects(effectNode, effectId2MaterialId, materialId2Name, pictureId2File, M, mnames, mtlTextMap, mtlTextBumpMap);
			}

			// parsing geometry
			ParserOpenCOLLADA::parseLibraryGeometries(geometryNode, obj_file, M, mnames, mtlTextMap, mtlTextBumpMap, meshModelVec, 1.0f);
		}
		else
		{
			LOG( "Could not load mesh from file '%s'", obj_file);
			return CMD_FAILURE;
		}

		// below code is to adjust the mesh if there's orientation in the joints. potential bug too because here only detect the first joint
		SkSkeleton* skel = char_p->getSkeleton();
		SkJoint* firstJ = skel->root()->child(0);
		if (firstJ)
		{
			SrQuat orient = skel->root()->child(0)->quat()->orientation();
			SrMat rotMat;
			orient.get_mat(rotMat);
			for (unsigned int i = 0; i < meshModelVec.size(); i++)
			{
				for (int j = 0; j < meshModelVec[i]->V.size(); j++)
				{
					SrVec pt = SrVec(meshModelVec[i]->V[j].x, meshModelVec[i]->V[j].y, meshModelVec[i]->V[j].z);
					SrVec ptp = pt * rotMat;
					meshModelVec[i]->V[j].x = ptp.x;
					meshModelVec[i]->V[j].y = ptp.y;
					meshModelVec[i]->V[j].z = ptp.z;
				}
				for (int j = 0; j < meshModelVec[i]->N.size(); j++)
				{
					SrVec pt = SrVec(meshModelVec[i]->N[j].x, meshModelVec[i]->N[j].y, meshModelVec[i]->N[j].z);
					SrVec ptp = pt * rotMat;
					meshModelVec[i]->N[j].x = ptp.x;
					meshModelVec[i]->N[j].y = ptp.y;
					meshModelVec[i]->N[j].z = ptp.z;
				}	
			}
		}
//		delete geometryNode;
	}	
	for (unsigned int i = 0; i < meshModelVec.size(); i++)
	{
		for (int j = 0; j < meshModelVec[i]->V.size(); j++)
		{
			meshModelVec[i]->V[j] *= factor;
		}
		SrSnModel* srSnModelDynamic = new SrSnModel();
		SrSnModel* srSnModelStatic = new SrSnModel();
		srSnModelDynamic->shape(*meshModelVec[i]);
		srSnModelStatic->shape(*meshModelVec[i]);
		srSnModelDynamic->changed(true);
		srSnModelDynamic->visible(false);
		srSnModelStatic->shape().name = meshModelVec[i]->name;
		srSnModelDynamic->shape().name = meshModelVec[i]->name;
		if (char_p->dMesh_p)
		{
			char_p->dMesh_p->dMeshDynamic_p.push_back(srSnModelDynamic);
			char_p->dMesh_p->dMeshStatic_p.push_back(srSnModelStatic);
			srSnModelDynamic->ref();
			srSnModelStatic->ref();
		}
		mcu_p->root_group_p->add(srSnModelDynamic);
	
		delete meshModelVec[i];
	}

	return( CMD_SUCCESS );
}

void nodeStr(const XMLCh* s, std::string& out)
{
	xml_utils::xml_translate(&out, s);
}

void parseLibraryControllers(DOMNode* node, const char* char_name, float scaleFactor, std::string jointPrefix, mcuCBHandle* mcu_p)
{
	boost::char_separator<char> sep(" \n");

	SbmCharacter* char_p = mcu_p->getCharacter( char_name );
	const DOMNodeList* list = node->getChildNodes();
	for (unsigned int c = 0; c < list->getLength(); c++)
	{
		DOMNode* node = list->item(c);
		int type = node->getNodeType();
		std::string name;
		nodeStr(node->getNodeName(), name);
		std::string value;
		nodeStr(node->getNodeValue(), value);
		boost::algorithm::to_lower(name);
		if (name == "controller")
		{
			DOMNamedNodeMap* attributes = node->getAttributes();
			DOMNode* idNode = attributes->getNamedItem(BML::BMLDefs::ATTR_ID);
			if (!idNode)	continue;
			std::string skinId;
			nodeStr(idNode->getNodeValue(), skinId);
			if (node->hasChildNodes())
			{
				const DOMNodeList* childrenList = node->getChildNodes();
				for (unsigned int cc = 0; cc < childrenList->getLength(); cc++)
				{
					DOMNode* childNode = childrenList->item(cc);
					std::string childName;
					nodeStr(childNode->getNodeName(), childName);
					if (childName == "skin")	// parsing skinning weights
					{
						DOMNamedNodeMap* skinAttributes = childNode->getAttributes();			
						DOMNode* skinNode = skinAttributes->getNamedItem(BML::BMLDefs::ATTR_SOURCE);	
						std::string skinSource;
						nodeStr(skinNode->getNodeValue(), skinSource);
						skinSource = skinSource.substr(1, skinSource.size() - 1);
						SkinWeight* skinWeight = new SkinWeight();
						skinWeight->sourceMesh = skinSource;

						// futhur for children
						const DOMNodeList* childListOfSkin = childNode->getChildNodes();
						for (unsigned int cSkin = 0; cSkin < childListOfSkin->getLength(); cSkin++)
						{
							DOMNode* childNodeOfSkin = childListOfSkin->item(cSkin);
							std::string childNameOfSkin;
							nodeStr(childNodeOfSkin->getNodeName(), childNameOfSkin);
							std::string bindJointName = skinSource + "-skin-joints";
							std::string bindWeightName = skinSource + "-skin-weights";
							std::string bindPoseMatName = skinSource + "-skin-bind_poses";

							if (childNameOfSkin == "bind_shape_matrix")
							{
								std::string tokenBlock;
								nodeStr(childNodeOfSkin->getTextContent(), tokenBlock);
								float bindShapeMat[16];
								
								boost::tokenizer<boost::char_separator<char> > tokens(tokenBlock, sep);
								int i = 0;
								for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
									 it != tokens.end();
									 ++it)
								{
									bindShapeMat[i] = (float)atof((*it).c_str());
									i++;
									if (i >= 16)
										break;
							    }
								//for (int i = 0; i < 16; i++)
								//	bindShapeMat[i] = (float)atof(tokenize(tokenBlock).c_str());
								
								skinWeight->bindShapeMat.set(bindShapeMat);
								skinWeight->bindShapeMat.transpose();
								skinWeight->bindShapeMat.setl4(skinWeight->bindShapeMat.get_translation()*scaleFactor);
							}
							if (childNameOfSkin == "source")
							{
								DOMNamedNodeMap* sourceAttributes = childNodeOfSkin->getAttributes();
								DOMNodeList* realContentNodeList = childNodeOfSkin->getChildNodes();
								std::string sourceId;
								nodeStr(sourceAttributes->getNamedItem(BML::BMLDefs::ATTR_ID)->getNodeValue(), sourceId);
								boost::algorithm::to_lower(sourceId);
								bool isBindJointName = (sourceId.find("joints") != std::string::npos);
								bool isBindWeights = (sourceId.find("weights") != std::string::npos);
								bool isBindPoseMatrices = (sourceId.find("bind_poses") != std::string::npos || sourceId.find("matrices") != std::string::npos);

								for (unsigned int cSource = 0; cSource < realContentNodeList->getLength(); cSource++)
								{
									DOMNode* realContentNode = realContentNodeList->item(cSource);
									std::string realNodeName;
									nodeStr(realContentNode->getNodeName(), realNodeName);		

									std::string tokenBlock;
									nodeStr(realContentNode->getTextContent(), tokenBlock);
									boost::tokenizer<boost::char_separator<char> > tokens(tokenBlock, sep);
									int matCounter = 0;
									float bindPosMat[16];
									SrMat newMat;

									for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
										 it != tokens.end();
										 ++it)
									{
										//if ( sourceId == bindJointName && realNodeName == "Name_array") // joint name
										if ( isBindJointName && realNodeName == "Name_array") // joint name
										{
											std::string jointName = (*it);
											// check if the joint name start with the pre-fix and remove the prefix
											if ((*it).compare(0, jointPrefix.size(), jointPrefix) == 0)
											{
												jointName.erase(0, jointPrefix.size());
											}
											//cout << "joint name = " << jointName << endl;	
											SBJoint* joint = char_p->getSkeleton()->getJointByName(jointName);
											if (joint)
												skinWeight->infJointName.push_back(joint->getName());
											else
												skinWeight->infJointName.push_back(jointName);
											
										}
										//if ( sourceId == bindWeightName && realNodeName == "float_array") // joint weights
										if ( isBindWeights && realNodeName == "float_array") // joint weights
											skinWeight->bindWeight.push_back((float)atof((*it).c_str()));
										//if ( sourceId == bindPoseMatName && realNodeName == "float_array") // bind pose matrices
										if ( isBindPoseMatrices && realNodeName == "float_array") // bind pose matrices
										{
											bindPosMat[matCounter] = (float)atof((*it).c_str());
											matCounter ++;
											if (matCounter == 16)
											{
												matCounter = 0;
												newMat.set(bindPosMat);													
												newMat.transpose();
												SrVec newTran = newMat.get_translation()*scaleFactor;
												newMat.setl4(newTran);
												skinWeight->bindPoseMat.push_back(newMat);
											}
										}
									}
								}								
							} // end of if (childNameOfSkin == "source")
							if (childNameOfSkin == "vertex_weights")
							{
								DOMNodeList* indexNodeList = childNodeOfSkin->getChildNodes();
								for (unsigned int cVertexWeights = 0; cVertexWeights < indexNodeList->getLength(); cVertexWeights++)
								{
									DOMNode* indexNode = indexNodeList->item(cVertexWeights);
									std::string indexNodeName;
									nodeStr(indexNode->getNodeName(), indexNodeName);
									std::string tokenBlock;
									nodeStr(indexNode->getTextContent(), tokenBlock);

									boost::tokenizer<boost::char_separator<char> > tokens(tokenBlock, sep);

									if (indexNodeName == "vcount")
									{
										for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
											 it != tokens.end();
										     ++it)
										{
											skinWeight->numInfJoints.push_back(atoi((*it).c_str()));
										}
									}
									else if (indexNodeName == "v")
									{
										for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
											 it != tokens.end();
										     ++it)
										{
											skinWeight->jointNameIndex.push_back(atoi((*it).c_str()));
											it++;
											skinWeight->weightIndex.push_back(atoi((*it).c_str()));
										}
									}
									else
									{
										continue;
									}
								}
							}
						}
						if (char_p && char_p->dMesh_p)
							char_p->dMesh_p->skinWeights.push_back(skinWeight);
					} // end of if (childName == "skin")
					if (childName == "morph")	// parsing morph targets
					{
						DOMNamedNodeMap* morphAttributes = childNode->getAttributes();			
						DOMNode* morphNode = morphAttributes->getNamedItem(BML::BMLDefs::ATTR_SOURCE);	
						std::string morphName;
						nodeStr(morphNode->getNodeValue(), morphName);
						morphName = morphName.substr(1, morphName.size() - 1);
						std::string morphFullName = morphName + "-morph";
						
						// futhur for children
						const DOMNodeList* childListOfMorph = childNode->getChildNodes();
						for (unsigned int cMorph = 0; cMorph < childListOfMorph->getLength(); cMorph++)
						{
							DOMNode* childNodeOfMorph = childListOfMorph->item(cMorph);
							std::string childNameOfMorph;
							nodeStr(childNodeOfMorph->getNodeName(), childNameOfMorph);
							if (childNameOfMorph == "source")
							{
								const DOMNodeList* childListOfSource = childNodeOfMorph->getChildNodes();
								for (size_t cMorphSource = 0; cMorphSource < childListOfSource->getLength(); cMorphSource++)
								{
									DOMNode* childNodeOfSource = childListOfSource->item(cMorphSource);
									std::string childNameOfSource;
									nodeStr(childNodeOfSource->getNodeName(), childNameOfSource);
									if (childNameOfSource == "IDREF_array")
									{
										std::vector<std::string> refMesh;
										std::string tokenBlock;
										nodeStr(childNodeOfMorph->getTextContent(), tokenBlock);
										boost::char_separator<char> sep2(" \n");
										boost::tokenizer<boost::char_separator<char> > tokens(tokenBlock, sep2);
										for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
											 it != tokens.end();
											 ++it)
										{
											refMesh.push_back((*it));
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
	if (char_p && char_p->dMesh_p)
	{
		for (size_t x = 0; x < char_p->dMesh_p->skinWeights.size(); x++)
		{
			SkinWeight* skinWeight = char_p->dMesh_p->skinWeights[x];
			for (size_t j = 0; j < skinWeight->infJointName.size(); j++)
			{
				std::string& jointName = skinWeight->infJointName[j];
				SkJoint* curJoint = char_p->getSkeleton()->search_joint(jointName.c_str());
				skinWeight->infJoint.push_back(curJoint); // NOTE: If joints are added/removed during runtime, this list will contain stale data
			}
		}
	}
}


int mcu_character_load_skinweights( const char* char_name, const char* skin_file, mcuCBHandle* mcu_p, float scaleFactor, const char* prefix )
{

#if defined(__ANDROID__) || defined(SBM_IPHONE)
	return ( CMD_SUCCESS );	
#endif

   std::string jointNamePrefix;
	if (prefix)
		jointNamePrefix = prefix;

#if ENABLE_FBX_PARSER
   if (strstr(skin_file, ".fbx") || strstr(skin_file, ".FBX"))
   {
      ParserFBX::parseSkin(skin_file, char_name, scaleFactor, jointNamePrefix, mcu_p);
      return( CMD_SUCCESS );
   }
#endif

	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		std::string message;
		xml_utils::xml_translate(&message, toCatch.getMessage());
		std::cout << "Error during initialization! :\n" << message << "\n";
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
		DOMDocument* doc = parser->getDocument();
		DOMNode* controllerNode = ParserOpenCOLLADA::getNode("library_controllers", doc);
		if (!controllerNode)
		{
			LOG("mcu_character_load_skinweights ERR: no binding info contained");
			return CMD_FAILURE;
		}
		parseLibraryControllers(controllerNode, char_name, scaleFactor, jointNamePrefix, mcu_p);
	}
	catch (const XMLException& toCatch) 
	{
		std::string message;
		xml_utils::xml_translate(&message, toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		return( CMD_FAILURE );
	}
	catch (const DOMException& toCatch) {
		std::string message;
		xml_utils::xml_translate(&message, toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
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
	const char* className, 
	mcuCBHandle *mcu_p)
{
	int err;
	
	if( strcmp(char_name, "*" )==0 ) {  // TODO: better character name valiadtion
		LOG( "init_character ERR: Invalid SbmCharacter name '%s'\n", char_name ); 
		return( CMD_FAILURE );
	}
	 SbmCharacter* existingCharacter = mcu_p->getCharacter(char_name);
	if (existingCharacter)
	{
		LOG( "init_character ERR: SbmCharacter '%s' EXISTS\n", char_name ); 
		return( CMD_FAILURE );
	}

	//SbmCharacter *char_p = new SbmCharacter(char_name);
	SmartBody::SBCharacter *char_p = new SmartBody::SBCharacter(char_name, className);
	mcu_p->registerCharacter(char_p);
	SkSkeleton* skeleton_p = NULL;
	// does the skeleton already exist in the skeleton map?
	std::map<std::string, SkSkeleton*>::iterator skelIter = mcu_p->skeleton_map.find(skel_file);
	if (skelIter ==  mcu_p->skeleton_map.end())
	{
		SkSkeleton* cachedSkeleton = load_skeleton( skel_file, mcu_p->me_paths, mcu_p->resource_manager, mcu_p->skScale );
		if( !cachedSkeleton ) {
			LOG( "init_character ERR: Failed to load skeleton \"%s\"\n", skel_file ); 
			mcu_p->unregisterCharacter(char_p);
			delete char_p;

			return CMD_FAILURE;
		}
		cachedSkeleton->ref();
		mcu_p->skeleton_map.insert(std::pair<std::string, SkSkeleton*>(skel_file, cachedSkeleton));
		skelIter = mcu_p->skeleton_map.find(skel_file);
	}

	//skeleton_p = new SkSkeleton((*skelIter).second);
	SmartBody::SBSkeleton* sbSkeleton = dynamic_cast<SmartBody::SBSkeleton*>((*skelIter).second);
	skeleton_p = new SmartBody::SBSkeleton(sbSkeleton);
	skeleton_p->ref();

	SmartBody::SBFaceDefinition* faceDefinition = NULL;

	// get the face motion mapping per character
	std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceIter = mcu_p->face_map.find(std::string(char_name));
	if (faceIter !=  mcu_p->face_map.end())
	{
		faceDefinition = (*faceIter).second;
	}
	else
	{
		// get the default face motion mapping
		faceIter = mcu_p->face_map.find("_default_");
		if (faceIter !=  mcu_p->face_map.end())
		{
			faceDefinition = (*faceIter).second;
		}
		else
		{
			LOG("Couldn't find _default_ face motion mappings! Check code.");
		}
	}

	err = char_p->init( skeleton_p, faceDefinition, &mcu_p->param_map, className );
	if( err == CMD_SUCCESS ) {

		char_p->locomotion_ct->init_skeleton(skeleton_p, skeleton_p);

		char_p->ct_tree_p->set_evaluation_logger( mcu_p->logger_p );
	}


#if USE_WSP
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
#endif


		// now register all joints.  wsp data isn't sent out until a request for it is received
		const std::vector<SkJoint *> & joints  = char_p->getSkeleton()->joints();

		for (size_t i = 0; i < joints.size(); i++ )
		{
			SkJoint * j = joints[ i ];


#if USE_WSP
			string wsp_joint_name = vhcl::Format( "%s:%s", char_name, j->name().c_str() );

			err = mcu_p->theWSP->register_vector_3d_source( wsp_joint_name, "position", SbmPawn::wsp_position_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				LOG( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" position.\n", char_name, wsp_joint_name.c_str() ); 
			}

			err = mcu_p->theWSP->register_vector_4d_source( wsp_joint_name, "rotation", SbmPawn::wsp_rotation_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				LOG( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" rotation.\n", char_name, wsp_joint_name.c_str() ); 
			}
#endif

		}
	

	return( err );
}


int begin_controller( 
	const char *char_name, 
	const char *ctrl_name, 
	mcuCBHandle *mcu_p
)	{
	
	SbmCharacter *char_p = mcu_p->getCharacter(char_name );
	if( char_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( ctrl_name );
		if( ctrl_p )	{
			// Use motion schedule by default
			MeCtScheduler2* sched_p = char_p->motion_sched_p;

			if( ctrl_p->controller_type() == MeCtGaze::CONTROLLER_TYPE) {
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
	
	SbmCharacter *char_p = mcu_p->getCharacter(char_name );
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

	string charName = "";

	// check to see if we are loading character-specific face mappings
	std::string characterName = "_default_";
	if (type == "char")
	{
		// now get the character name
		characterName= args.read_token();
		if (characterName == "")
		{
			LOG("Character name needed. Use: set face char <name> <au|viseme|neutral> <skmfile>");
			return CMD_FAILURE;
		}

		// restore the type
		type = args.read_token();
	}

	if( type=="au" ) {
		return mcu_set_face_au_func( args, mcu_p, characterName );
	} else if( type=="viseme" ) {
		return mcu_set_face_viseme_func( args, mcu_p, characterName );
	} else if( type=="neutral" ) {
		const string motion_name = args.read_token();

		SmartBody::SBFaceDefinition* faceDefinition = NULL;
		std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceMotionIter = mcu_p->face_map.find(characterName);
		if (faceMotionIter == mcu_p->face_map.end())
		{
			// face motion mappings for character do not yet exist - create them
			faceDefinition = new SmartBody::SBFaceDefinition();
			faceDefinition->setName(characterName);
			mcu_p->face_map.insert(std::pair<std::string, SmartBody::SBFaceDefinition*>(characterName, faceDefinition));
		}
		else
		{
			faceDefinition = (*faceMotionIter).second;
		}

		if (motion_name.size() > 0)
		{
			std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(motion_name);
			if (motionIter != mcu_p->motion_map.end())
			{
				SkMotion* motion_p = (*motionIter).second;
				faceDefinition->setFaceNeutral(motion_p->getName());
				return CMD_SUCCESS;
			} else {
				LOG("ERROR: Unknown motion \"%s\".", motion_name.c_str());
				return CMD_FAILURE;
			}
		}
		else
		{
			// no motion specified, create the channel without mapping it to a motion
			return CMD_SUCCESS;
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

	// check to see if we are loading character-specific face mappings
	std::string characterName = "_default_";
	if (type == "char")
	{
		// now get the character name
		characterName= args.read_token();
		if (characterName == "")
		{
			LOG("Character name needed. Use: set face char <name> <au|viseme|neutral> <skmfile>");
			return CMD_FAILURE;
		}

		// restore the type
		type = args.read_token();
	}

	if( type=="au" ) {
		return mcu_print_face_au_func( args, mcu_p, characterName );
	} else if( type=="viseme" ) {
		return mcu_print_face_viseme_func( args, mcu_p, characterName );
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
int mcu_set_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string characterName ) {
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
		LOG("ERROR: Invalid action unit number \"%s\".", unit_str.c_str());
		return CMD_FAILURE;
	}

	string token = args.read_token();
	std::string side = token;

	std::string motion = args.read_token();

	if (side != "left" && 
		side != "right" &&
		side != "LEFT" &&
		side != "RIGHT")
	{
		motion = side;
		side = "";
	}
	
	SmartBody::SBFaceDefinition* faceDefinition = NULL;
	std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceMotionIter = mcu_p->face_map.find(characterName);
	if (faceMotionIter == mcu_p->face_map.end())
	{
		// face motion mappings for character do not yet exist - create them
		faceDefinition = new SmartBody::SBFaceDefinition();
		faceDefinition->setName(characterName);
		mcu_p->face_map.insert(std::pair<std::string, SmartBody::SBFaceDefinition*>(characterName, faceDefinition));
	}
	else
	{
		faceDefinition = (*faceMotionIter).second;
	}

	faceDefinition->setAU(unit, side, motion);

	return CMD_SUCCESS;
}


inline void print_au( const int unit, const ActionUnit* au ) {
	if( au->is_bilateral() ) {
		std::stringstream strstr;
		strstr << "Action Unit #" << unit << ": Left SkMotion ";
		if( au->left ) {
			strstr << '\"' << au->left->getName() << "\".";
		} else {
			strstr << "is NULL.";
		}
		LOG("%s", strstr.str().c_str());

		strstr.clear();
		strstr << "Action Unit #" << unit << ": Right SkMotion ";
		if( au->right ) {
			strstr << '\"' << au->right->getName() << "\".";
		} else {
			strstr << "is NULL.";
		}
		LOG("%s", strstr.str().c_str());
	} else {
		std::stringstream strstr;
		strstr << "Action Unit #" << unit << ": SkMotion ";
		if( au->left ) {
			strstr << '\"' << au->left->getName() << "\".";
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
int mcu_print_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string characterName ) {
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
			LOG("ERROR: Invalid action unit number \"%s\".", unit_str.c_str());
			return CMD_FAILURE;
		}
	}

	SmartBody::SBFaceDefinition* faceDefinition = NULL;
	std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceMotionIter = mcu_p->face_map.find(characterName);
	if (faceMotionIter == mcu_p->face_map.end())
	{
		// face motion mappings for character do not yet exist
		LOG("Character %s does not yet have any AU mappings for the face.", characterName.c_str());
		return CMD_FAILURE;
	}
	else
	{
		faceDefinition = (*faceMotionIter).second;
	}

	if( unit == ALL_ACTION_UNITS )
	{
		int numAUs = faceDefinition->getNumAUs();
		for (int a = 0; a < numAUs; a++)
		{
			int unit = faceDefinition->getAUNum(a);
			ActionUnit* au = faceDefinition->getAU(unit);
			print_au(unit, au);
		}
	} 
	else
	{
		ActionUnit* au = faceDefinition->getAU(unit);
		if (!au)
		{
			LOG("Action Unit #%s is not set.", unit);
		} 
		else
		{
			print_au(unit, au);
		}
	}
	return CMD_SUCCESS;
}


/**
 *  Implements the "set face viseme ..." command.
 *
 *  Syntax: set face viseme <viseme symbol> <motion-name>
 */

int mcu_set_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p, std::string characterName ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		LOG("Syntax: %s", SET_FACE_VISEME_SYNTAX_HELP);
		return CMD_SUCCESS;
	}

	SmartBody::SBFaceDefinition* faceDefinition = NULL;
	std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceMotionIter = mcu_p->face_map.find(characterName);
	if (faceMotionIter == mcu_p->face_map.end())
	{
		// face motion mappings for character do not yet exist - create them
		faceDefinition = new SmartBody::SBFaceDefinition();
		faceDefinition->setName(characterName);
		mcu_p->face_map.insert(std::pair<std::string, SmartBody::SBFaceDefinition*>(characterName, faceDefinition));
	}
	else
	{
		faceDefinition = (*faceMotionIter).second;
	}

	std::string motionName = args.read_token();

	faceDefinition->setViseme(viseme, motionName);

	return CMD_SUCCESS;
}

void print_viseme( const string& viseme, const SkMotion* motion ) {
	std::stringstream strstr;
	strstr << "Viseme \""<<viseme<<"\" pose: ";
	if( motion == NULL ) {
		strstr << "NULL";
	} else {
		SkMotion* m2 = const_cast<SkMotion*>(motion);
		strstr << '\"' << m2->getName() << '\"';
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

int mcu_print_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p,std::string characterName ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		cout << "Syntax:" << endl
		     << "\tprint face viseme <viseme name>" << endl
		     << "\tprint face viseme *" << endl;
		return CMD_SUCCESS;
	}

	SmartBody::SBFaceDefinition* faceDefinition = NULL;
	std::map<std::string, SmartBody::SBFaceDefinition*>::iterator faceMotionIter = mcu_p->face_map.find(characterName);
	if (faceMotionIter == mcu_p->face_map.end())
	{
		// face motion mappings for character do not yet exist
		LOG("Character %s does not yet have any viseme mappings for the face.", characterName.c_str());
		return CMD_FAILURE;
	}
	faceDefinition = (*faceMotionIter).second;

	if (viseme == "*")
	{
		int numVisemes = faceDefinition->getNumVisemes();
		for (int v = 0; v < numVisemes; v++)
		{
			std::string visemeName = faceDefinition->getVisemeName(v);
			SkMotion* motion = faceDefinition->getVisemeMotion(visemeName);
			print_viseme(visemeName, motion);
		}
	}
	else
	{
		SkMotion* motion = faceDefinition->getVisemeMotion(viseme);
		print_viseme(viseme, motion);
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

	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, *pose_p );
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

	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, mot_p );
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
	
	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, mot_p );
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
	
	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, mot_p, alt_mot_p );
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

	ctrl_p->setName( ctrl_name );
	ctrl_p->init(
		NULL,
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

	ctrl_p->setName( ctrl_name );
	ctrl_p->init(NULL);
	return( CMD_SUCCESS );
}

int init_lilt_controller(
	char *ctrl_name,
	char *char_name,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p= mcu_p->getCharacter(char_name );
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
	ctrl_p->setName( ctrl_name );
	ctrl_p->init( char_p, char_p->getSkeleton() );
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
	
	ctrl_p->setName( ctrl_name );
	ctrl_p->init(NULL);
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

	lifecycle_ct->setName( ctrl_name );
	lifecycle_ct->init( ct );

	return CMD_SUCCESS;
}

int init_scheduler_controller( 
	char *ctrl_name, 
	char *char_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p = mcu_p->getCharacter(char_name );
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
		
		sched_p->init(char_p);

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
	LOG( "MCU QUERY: MeController '%s':\n", ctrl_p->getName().c_str());
	LOG( "  type... %s\n", ctrl_p->controller_type().c_str() );
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

	ctrl <> handle <handle-name>
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

		
			int numControllersAffected = 0;
			for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
				iter != mcu_p->getCharacterMap().end();
				iter++)
			{
				SbmCharacter* character = (*iter).second;
				MeControllerTreeRoot* controllerTree = character->ct_tree_p;
				int numControllers = controllerTree->count_controllers();
			
				for (int c = 0; c < numControllers; c++)
				{
					if (checkStatus)
					{
						std::string passThroughStr = (controllerTree->controller(c)->is_pass_through())? " true " : " false";							
						LOG("[%s] %s = %s", character->getName().c_str(), controllerTree->controller(c)->getName().c_str(), passThroughStr.c_str());
						numControllersAffected = numControllers;	// just so it won't generate error
					}
					else if (allControllers)
					{
						if (toggleValue)
							controllerTree->controller(c)->set_pass_through(!controllerTree->controller(c)->is_pass_through());
						else
							controllerTree->controller(c)->set_pass_through(passThroughValue);
						numControllersAffected++;
					}
					else if (controllerTree->controller(c)->getName() == ctrl_name)
					{
						if (toggleValue)
							controllerTree->controller(c)->set_pass_through(!controllerTree->controller(c)->is_pass_through());
						else
							controllerTree->controller(c)->set_pass_through(passThroughValue);
						numControllersAffected++;
					}
				}
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
			if( strcmp( ctrl_cmd, "handle" ) == 0 )	{
				char *handle = args.read_token();
				ctrl_p->handle( handle );
				return( CMD_SUCCESS );
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

#if 0
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
#else
	LOG( "mcu_motion_controller_func ERR: deprecated" );
#endif
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
			LOG("To set limits, use: gazelimit <lumbar|thorax|cervical|cranial|optical|back|chest|neck|head|eyes> pitchup pitchdown heading roll");
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

/////////////////////////////////////////////////////////////

/*
	gaze <> target point <x y z>
	gaze <> target euler <p h r>
	gaze <> offset euler <p h r>
	gaze <> smooth <basis>
	gaze <> speed <deg-per-sec>
	gaze <> timehint <sec>
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
			if( strcmp( gaze_cmd, "timehint" ) == 0 )	{

				float sec = args.read_float();
				gaze_p->set_time_hint( sec );
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
	load skeleton <file-path> [-R]
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
		} else if( strcmp( load_cmd, "skeleton" )==0 ||
		           strcmp( load_cmd, "skeletons" )==0 )
		{
			const char* token = args.read_token();

			bool recursive = false;
			if( strcmp( token, "-R" )==0 ) {
				recursive = true;
				token = args.read_token();
			}
			return mcu_p->load_skeletons( token, recursive );
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
	bool ret = CMD_SUCCESS;
	mcu_p->bonebus.CloseConnection();
	if (mcu_p->net_host)
	{
		ret = mcu_p->bonebus.OpenConnection(mcu_p->net_host);
		mcu_p->bonebus.UpdateAllCharacters();
	}
	
	if (ret)
		return (CMD_SUCCESS);
	else
		return (CMD_FAILURE);
}

int mcu_net_check( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	if (!mcu_p->bonebus.IsOpen())
	{
		if (!mcu_p->net_host)
			mcu_p->net_host = "localhost";
		return mcu_net_reset(args, mcu_p);
	}
	else
		return CMD_SUCCESS;
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
			boost::filesystem::path p( "../../../../.." );
			boost::filesystem::path abs_p = boost::filesystem::complete( p );

//            char full[ _MAX_PATH ];
//            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            if ( boost::filesystem2::exists( abs_p ) )
            {
//               soundFile = string( full ) + string( "/" ) + soundFile;
				p  /= soundFile;
               soundFile = abs_p.string();
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
			boost::filesystem::path p( "../../../../.." );
			boost::filesystem::path abs_p = boost::filesystem::complete( p );
            if ( boost::filesystem2::exists( abs_p ) )
            {
				p  /= soundFile;
               soundFile = abs_p.string();
            }
         }

#if 0
         if ( !absolutePath )
         {
            char full[ _MAX_PATH ];
            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            {
               soundFile = string( full ) + string( "\\" ) + soundFile;
            }
         }
#endif

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

         gwiz::quat_t q = gwiz::euler_t( x, y, z );

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
		  mcu_p->vhmsg_send( "vrProcEnd sbm" );
	      return CMD_SUCCESS;
      }
   }

	return CMD_SUCCESS;
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
        for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
			iter != mcu_p->getCharacterMap().end();
			iter++)
		{
            string message = "sbm ";
			message += (*iter).second->getName();
            mcu_p->vhmsg_send( "vrComponent", message.c_str() );
        }
    }

	return CMD_SUCCESS;
}
/*
* Callback function for vrPerception message
*/
int mcu_vrPerception_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	LOG("coordinates received");
	if (mcu_p)
	{
		char * command = args.read_token();

		//only considering kinect or gavam or both

      if ( _stricmp( command, "kinect" ) == 0 ||
           _stricmp( command, "gavam" ) == 0 )
      {
		  //if the message contains kinect data
		  if (_stricmp( command, "kinect" ) == 0)
		  {
			  string stringId = args.read_token();
			  int kinectId = args.read_int();
			  string stringFailed = args.read_token();
			  int kinectFailed = args.read_int();
			  string stringUsers = args.read_token();
			  int kinectUsers = args.read_int();

			  for (int indexUsers = 0; indexUsers < kinectUsers; ++indexUsers)
			  {
				string stringPosition = args.read_token();
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
			  }

			  //currently not doing anything with the Kinect data

			  command = args.read_token();

			  //if message contains gavam after kinect
			  //ideally  this parsing should be a separate method so that the message can contain gavam / kinect in any order
			  if (_stricmp( command, "gavam" ) == 0)
			  {
				  PerceptionData* data = new PerceptionData();

				  //parse float data
				  data->pos[0] = args.read_float();
				  data->pos[1] = args.read_float();
				  data->pos[2] = args.read_float();
				  data->rot[0] = args.read_float();
				  data->rot[1] = args.read_float();
				  data->rot[2] = args.read_float();

				  //For converting data from euler to quaternion

				  float cos_z_2 = cosf((float)0.5*data->rot[2]);
				  float cos_y_2 = cosf((float)0.5*data->rot[1]);
				  float cos_x_2 = cosf((float)0.5*data->rot[0]);

				  float sin_z_2 = sinf((float)0.5*data->rot[2]);
				  float sin_y_2 = sinf((float)0.5*data->rot[1]);
				  float sin_x_2 = sinf((float)0.5*data->rot[0]);

				  // and now compute quaternion
				  float quatW = cos_z_2*cos_y_2*cos_x_2 + sin_z_2*sin_y_2*sin_x_2;
				  float quatX = cos_z_2*cos_y_2*sin_x_2 - sin_z_2*sin_y_2*cos_x_2;
				  float quatY = cos_z_2*sin_y_2*cos_x_2 + sin_z_2*cos_y_2*sin_x_2;
				  float quatZ = sin_z_2*cos_y_2*cos_x_2 - cos_z_2*sin_y_2*sin_x_2;

				  // Log the Gavam data
				  LOG("Gavam Coordinates - %f %f %f %f %f %f", data->pos[0], data->pos[1], data->pos[2], data->rot[0], data->rot[1], data->rot[2]);
				  char *messg = new char[1024];
				  #ifdef WIN32
				  sprintf_s(messg, 1024, "receiver skeleton brad generic rotation skullbase %f %f %f %f", quatW, quatX, quatY, quatZ);
				  #else
				  snprintf(messg, 1024, "receiver skeleton brad generic rotation skullbase %f %f %f %f", quatW, quatX, quatY, quatZ);
				  #endif
				  //log message before sending it
				  //LOG(messg);
				  //sending temp mesg just to make sure it works 
				  //mcu_p->vhmsg_send("test", messg);
				  //send the receiver message to orient the skullbase of brad as per the user's head orientation as detected by Gavam
				  mcu_p->vhmsg_send("sbm", messg);
				  //after sending the message, send a test message as confirmation
				  //mcu_p->vhmsg_send("testconfirmed", messg);
			  }
		  }
		  
		  else if (_stricmp( command, "gavam" ) == 0)
		  {
			  //this is only when only gavam is detected (which is not the case right now)
			  //right now vrPerception send kinect and the gavam coordinates.
		  }
	  }
	}

	return CMD_SUCCESS;
}

int mcu_sbmdebugger_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string instanceId = args.read_token();
	// make sure this instance id matches
	// ...
	// ...

	std::string messageId = args.read_token();
	std::string type = args.read_token();
	if (type == "response")
		return CMD_SUCCESS;

	if (type != "request")
		return CMD_SUCCESS;

	std::string returnType = args.read_token();
	std::string code = args.read_token();
	if (code.size() == 0)
	{
		std::stringstream strstr;
		strstr << instanceId << " " << messageId << " response-fail ";
		strstr << "No Python code sent.";
		mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
		return CMD_FAILURE;
	}
	
	PyObject* pyResult = NULL;
	if (returnType == "void")
	{
		try {
			boost::python::exec(code.c_str(), *mcu_p->mainDict);
		} catch (...) {
			PyErr_Print();
		}
		return CMD_SUCCESS;
	}
	else if (returnType == "int")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			int result = boost::python::extract<int>(mcu_p->mainDict["ret"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			strstr << result;
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else if (returnType == "float")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			float result = boost::python::extract<float>(mcu_p->mainDict["ret"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			strstr << result;
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else if (returnType == "string")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			std::string result = boost::python::extract<std::string>(mcu_p->mainDict["ret"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			strstr << result;
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else if (returnType == "int-array")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			boost::python::object obj2 = boost::python::exec("size = len(ret)", mcu_p->mainDict);
			int size =  boost::python::extract<int>(mcu_p->mainDict["size"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			for (int x = 0; x < size; x++)
			{
				int val =  boost::python::extract<int>(mcu_p->mainDict["ret"][x]);
				strstr << " " << val;
			}
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else if (returnType == "float-array")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			boost::python::object obj2 = boost::python::exec("size = len(ret)", mcu_p->mainDict);
			int size =  boost::python::extract<int>(mcu_p->mainDict["size"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			for (int x = 0; x < size; x++)
			{
				float val =  boost::python::extract<float>(mcu_p->mainDict["ret"][x]);
				strstr << " " << val;
			}
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else if (returnType == "string-array")
	{
		try {
			boost::python::object obj = boost::python::exec(code.c_str(), mcu_p->mainDict);
			boost::python::object obj2 = boost::python::exec("size = len(ret)", mcu_p->mainDict);
			int size =  boost::python::extract<int>(mcu_p->mainDict["size"]);
			std::stringstream strstr;
			strstr << instanceId << " " << messageId << " response ";
			for (int x = 0; x < size; x++)
			{
				std::string val =  boost::python::extract<std::string>(mcu_p->mainDict["ret"][x]);
				strstr << " " << val;
			}
			mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
		}
	}
	else
	{
		std::stringstream strstr;
		strstr << instanceId << " " << messageId << " response-fail ";
		strstr << "Unknown return type: " << returnType;
		mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
		return CMD_FAILURE;
	}

	std::stringstream strstr;
	strstr << instanceId << " " << messageId << " response-fail ";
	strstr << "Problem executing code." << returnType;
	mcu_p->vhmsg_send( "sbmdebugger", strstr.str().c_str() );
	return CMD_FAILURE;
	
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
		LOG( "  '%s'\n", (*motionIter).second->getName().c_str() );
	}
	
	LOG( "POSE CTRL:\n" );
	mcu_p->pose_ctrl_map.reset();
	MeCtPose * pose_ctrl_p;
	while( pose_ctrl_p = mcu_p->pose_ctrl_map.next() )	{
		LOG( "  '%s' : '%s'\n", pose_ctrl_p->getName().c_str(), pose_ctrl_p->posture_name() );
	}
	
	LOG( "MOTION CTRL:\n" );
	mcu_p->motion_ctrl_map.reset();
	MeCtMotion * mot_ctrl_p;
	while( mot_ctrl_p = mcu_p->motion_ctrl_map.next() )	{
		LOG( "  '%s' : '%s'\n", mot_ctrl_p->getName().c_str(), mot_ctrl_p->motion()->getName().c_str() );
	}
	
	LOG( "SIMPLE-NOD:\n" );
	mcu_p->snod_ctrl_map.reset();
	MeCtSimpleNod * snod_p;
	while( snod_p = mcu_p->snod_ctrl_map.next() )	{
		LOG( "  '%s'\n", snod_p->getName().c_str() );
	}
	
	LOG( "ANKLE-LILT:\n" );
	mcu_p->lilt_ctrl_map.reset();
	MeCtAnkleLilt * lilt_p;
	while( lilt_p = mcu_p->lilt_ctrl_map.next() )	{
		LOG( "  '%s'\n", lilt_p->getName().c_str() );
	}
	
	LOG( "SCHEDULE:\n" );
	mcu_p->sched_ctrl_map.reset();
	MeCtScheduler2 * sched_p;

	while( sched_p = mcu_p->sched_ctrl_map.next() )	{
		LOG( "  '%s'\n", sched_p->getName().c_str());
	}
	
	LOG( "ALL CONTROLLERS:\n" );
	mcu_p->controller_map.reset();
	MeController * ctrl_p;
	while( ctrl_p = mcu_p->controller_map.next() )	{
		LOG( "  '%s'\n", ctrl_p->getName().c_str() );
	}
	
	return (CMD_SUCCESS);
}

/////////////////////////////////////////////////////////////

int mcu_wsp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

#if USE_WSP
	mcu_p->theWSP->process_command( args.read_remainder_raw() );
#endif

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

		SbmCharacter* character = mcu_p->getCharacter(charName);
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
			int numValidChannels = motion->connect(character->getSkeleton());	// connect and check for the joints
			SkChannelArray& mChanArray = motion->channels();
			int mChanSize = mChanArray.size();
			SkChannelArray& skelChanArray = character->getSkeleton()->channels();
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
				std::string channelName = "";
				if (mode == 1)
				{
					chan = mChanArray[i];
					channelName = mChanArray.name(i);
				}
				if (mode == 2)
				{
					chan = skelChanArray[i];
					channelName = skelChanArray.name(i);
				}

				if (!chan.joint)
				{
					LOG("  %d: (No mathing joint)", i);
					continue;
				}

				std::string jointName = chan.joint->name();
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
				if (mode == 1)
				{
					pos = skelChanArray.search(chan.joint->name(), chan.type);
					//pos = skelChanArray.linear_search(chan.joint->name(), chan.type);
				}
				if (mode == 2)	
				{
					pos = mChanArray.search(chan.joint->name(), chan.type);
					//pos = mChanArray.linear_search(chan.joint->name(), chan.type);
				}
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
					std::string jointName = mChanArray.name(c);
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
							LOG("Channel %s/%s has non-zero value: %f.", jointName.c_str(), channel.type_name(), frameData[curIndex]);
						else if (chanType == SkChannel::Swing)
							LOG("Channel %s/%s has non-zero value: %f %f.", jointName.c_str(), channel.type_name(), frameData[curIndex], frameData[curIndex + 1]);
						else if (chanType == SkChannel::Quat)
							LOG("Channel %s/%s has non-zero value: %f %f %f %f.", jointName.c_str(), channel.type_name(), frameData[curIndex], frameData[curIndex+ 1], frameData[curIndex + 2], frameData[curIndex + 3]);
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


// command: adjustmotion <motion name> <character name>
// this command adjust the motion with character skeleton's prerotation for once
int mcu_adjust_motion_function( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (mcu_p)
	{
		std::string motionName = args.read_token();
		std::string charName = args.read_token();
		SkMotion* motion = mcu_p->lookUpMotion(motionName.c_str());
		if (!motion)
		{
			LOG("mcu_adjust_motion_function ERR: motion %s not found!", motionName.c_str());
			return CMD_FAILURE;
		}
		SbmCharacter* character = mcu_p->getCharacter(charName);
		if (!character)
		{
			LOG("mcu_adjust_motion_function ERR: character %s not found!", charName.c_str());
			return CMD_FAILURE;
		}
		ParserOpenCOLLADA::animationPostProcess(*character->getSkeleton(), *motion);
	}
	return CMD_SUCCESS;
}

int mcu_mediapath_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	int num = args.calc_num_tokens();
	if ( num > 1)
	{
		LOG("Use: mediapath <path>");
		return CMD_FAILURE;
	}
	if ( num == 1 )
	{
		char* path = args.read_token();
		if (strcmp(path, "help") == 0)
		{
			LOG("Use: mediapath <path>");
			return CMD_SUCCESS;
		}

		if (mcu_p)
		{
			mcu_p->setMediaPath(path);
		}
		return CMD_SUCCESS;
	}

	LOG("mediapath is '%s'", mcu_p->getMediaPath().c_str());
	return CMD_SUCCESS;
}

int triggerevent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* eventType = args.read_token();
	if (!eventType || strcmp(eventType, "help") == 0)
	{
		LOG("Use: triggerevent <event> <parameters>");
		return CMD_SUCCESS;
	}

	char* params = args.read_token();

	EventManager* eventManager = EventManager::getEventManager();
	std::string parameters = params;
	Event e;
	e.setType(eventType);
	e.setParameters(parameters);
	eventManager->handleEvent(&e, mcu_p->time);
		
	return CMD_SUCCESS;
}

int mcu_python_func( srArgBuffer& args, mcuCBHandle* mcu_p )
{
	// executes Python code
	std::string command = args.read_remainder_raw();
	return mcu_p->executePython(command.c_str());
}

int mcu_interp_func( srArgBuffer& args, mcuCBHandle* mcu_p )
{
	if (mcu_p->use_python)
	{
		mcu_p->use_python = false;
		LOG("Python interpreter is now active.");
	}
	else
	{
		mcu_p->use_python = true;
		LOG("Command interpreter is now active.");
	}


	return CMD_SUCCESS;
}

int mcu_pythonscript_func( srArgBuffer& args, mcuCBHandle* mcu_p )
{
	std::string scriptName = args.read_token();	
	return mcu_p->executePythonFile(scriptName.c_str());
}


int addevent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* tok = args.read_token();
	if (!tok || strcmp(tok, "help") == 0)
	{
		LOG("Use: addevent <motion> <time> <event> <parameters> [once]");
		return CMD_SUCCESS;
	}

	// find the motion
	std::string motionName = tok;
	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionName);
	if (iter == mcu_p->motion_map.end())
	{
		LOG("Motion %s not found, no event added.", motionName.c_str());
		return CMD_FAILURE;
	}

	SkMotion* motion = (*iter).second;
	float time = args.read_float();
	char* type = args.read_token();
	if (!type)
	{
		LOG("Event type was not found, no event added for motion %s.", motionName.c_str());
		return CMD_FAILURE;
	}

	char* parameters = args.read_token();
	char* once = args.read_token();
	bool isOnce = false;
	if (strlen(once) > 0 )
	{
		if (strcmp(once, "once") == 0)
		{
			isOnce = true;
		}
		else
		{
			LOG("Event option '%s' was not understood, expected 'once'. No event type '%s' with parameters '%s' added for motion %s at time %f.", motionName.c_str(), type, parameters, time);
			return CMD_FAILURE;
		}
	}

	MotionEvent* motionEvent = new MotionEvent();
	if (isOnce)
		motionEvent->setIsOnceOnly(true);
	motionEvent->setTime(time);
	motionEvent->setType(type);
	if (parameters)
		motionEvent->setParameters(parameters);
	motion->addMotionEvent(motionEvent);
	LOG("Event '%s/%s' added to motion '%s' at time %f", type, parameters, motionName.c_str(), time);
		
	return CMD_SUCCESS;
}

int removeevent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* tok = args.read_token();
	if (!tok || strcmp(tok, "help") == 0)
	{
		LOG("Use: removeevent <motion|*>");
		return CMD_SUCCESS;
	}

	// find the motion
	std::string motionName = tok;
	if (motionName == "*") // clear events in all motions
	{
		int numMotions = 0;
		int numEvents = 0;
		for (std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.begin();
			 iter != mcu_p->motion_map.end();
			 iter++)
		{
			SkMotion* motion = (*iter).second;
			numMotions++;
			std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
			for (size_t x = 0; x < motionEvents.size(); x++)
			{
				delete motionEvents[x];
				numEvents++;
			}
			motionEvents.clear();
			LOG("%d motion events in %d motions have been removed.", numEvents, numMotions);
			return CMD_SUCCESS;
		}
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionName);
	if (iter == mcu_p->motion_map.end())
	{
		LOG("Motion %s not found, no events removed.", motionName.c_str());
		return CMD_FAILURE;
	}

	SkMotion* motion = (*iter).second;
	std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
	int numEvents = motionEvents.size();
	for (size_t x = 0; x < motionEvents.size(); x++)
	{
		delete motionEvents[x];
	}
	motionEvents.clear();
	LOG("%d motion events removed from motion %s.", numEvents, motion->getName().c_str());
		
	return CMD_SUCCESS;
}

int disableevents_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* tok = args.read_token();
	if (!tok || strcmp(tok, "help") == 0)
	{
		LOG("Use: disableevents <motion|*>");
		return CMD_SUCCESS;
	}

	// find the motion
	std::string motionName = tok;
	if (motionName == "*") // clear events in all motions
	{
		int numMotions = 0;
		int numEvents = 0;
		for (std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.begin();
			 iter != mcu_p->motion_map.end();
			 iter++)
		{
			SkMotion* motion = (*iter).second;
			numMotions++;
			std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
			for (size_t x = 0; x < motionEvents.size(); x++)
			{
				motionEvents[x]->setEnabled(false);
				numEvents++;
			}
			LOG("%d motion events in %d motions have been disabled.", numEvents, numMotions);
			return CMD_SUCCESS;
		}
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionName);
	if (iter == mcu_p->motion_map.end())
	{
		LOG("Motion %s not found, no events disabled.", motionName.c_str());
		return CMD_FAILURE;
	}

	SkMotion* motion = (*iter).second;
	std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
	int numEvents = motionEvents.size();
	for (size_t x = 0; x < motionEvents.size(); x++)
	{
		motionEvents[x]->setEnabled(false);
	}
	LOG("%d motion events have been disabled from motion %s.", numEvents, motion->getName().c_str());
		
	return CMD_SUCCESS;
}

int enableevents_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* tok = args.read_token();
	if (!tok || strcmp(tok, "help") == 0)
	{
		LOG("Use: enableevents <motion|*>");
		return CMD_SUCCESS;
	}

	// find the motion
	std::string motionName = tok;
	if (motionName == "*") // clear events in all motions
	{
		int numMotions = 0;
		int numEvents = 0;
		for (std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.begin();
			 iter != mcu_p->motion_map.end();
			 iter++)
		{
			SkMotion* motion = (*iter).second;
			numMotions++;
			std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
			for (size_t x = 0; x < motionEvents.size(); x++)
			{
				motionEvents[x]->setEnabled(true);
				numEvents++;
			}
			LOG("%d motion events in %d motions have been enabled.", numEvents, numMotions);
			return CMD_SUCCESS;
		}
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionName);
	if (iter == mcu_p->motion_map.end())
	{
		LOG("Motion %s not found, no events enabled.", motionName.c_str());
		return CMD_FAILURE;
	}

	SkMotion* motion = (*iter).second;
	std::vector<MotionEvent*>& motionEvents = motion->getMotionEvents();
	int numEvents = motionEvents.size();
	for (size_t x = 0; x < motionEvents.size(); x++)
	{
		motionEvents[x]->setEnabled(true);
	}
	LOG("%d motion events have been enabled from motion %s.", numEvents, motion->getName().c_str());
		
	return CMD_SUCCESS;
}


int registerevent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* type = args.read_token();
	if (!type || strcmp(type, "help") == 0)
	{
		LOG("Use: registerevent <eventtype> <action>");
		return CMD_SUCCESS;
	}

	char* action = args.read_token();

	EventManager* eventManager = EventManager::getEventManager();
	BasicHandler* handler = new BasicHandler();
	handler->setAction(action);
	eventManager->addEventHandler(type, handler);

	return CMD_SUCCESS;
}


int unregisterevent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* type = args.read_token();
	if (!type || strcmp(type, "help") == 0)
	{
		LOG("Use: unregisterevent <eventtype>");
		return CMD_SUCCESS;
	}

	EventManager* eventManager = EventManager::getEventManager();
	eventManager->removeEventHandler(type);

	return CMD_SUCCESS;
}

int setmap_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	// set bone mapping on a specifically named map
	char* mapname = args.read_token();
	if (!mapname || strcmp(mapname, "help") == 0)
	{
		LOG("Use: setmap <mapname> <from> <to>");
		return CMD_SUCCESS;
	}

	char* from =  args.read_token();
	if (strlen(from) == 0)
	{
		LOG("Use: setmap <mapname> <from> <to>");
		return CMD_FAILURE;
	}

	char* to =  args.read_token();
	if (strlen(to) == 0)
	{
		LOG("Use: setmap <mapname> <from> <to>");
		return CMD_FAILURE;
	}

	BoneMap* boneMap = NULL;
	std::map<std::string, BoneMap*>::iterator iter = mcu_p->boneMaps.find(mapname);
	if (iter != mcu_p->boneMaps.end())
	{
		boneMap = (*iter).second;
	}
	else
	{
		boneMap = new BoneMap();
		mcu_p->boneMaps.insert(std::pair<std::string, BoneMap*>(mapname, boneMap));
	}

	bool found = false;
	for (size_t x = 0; x < boneMap->map.size(); x++)
	{
		if (from == boneMap->map[x].first)
		{
			boneMap->map[x].second = to;
			found = true;
		}
	}
	if (!found)
	{
		boneMap->map.push_back(std::pair<std::string, std::string>(from, to));
	}

	LOG("Mapping %s -> %s on bone map %s", from, to, mapname);
	return CMD_SUCCESS;
}

int motionmapdir_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string directory = args.read_token();
	if (directory.size() == 0 || directory == "help")
	{
		LOG("Use: motionmapdir <directory> <map>");
		return CMD_SUCCESS;
	}

	std::string mapName = args.read_token();
	if (mapName.size() == 0)
	{
		LOG("Use: motionmapdir <directory> <map>");
		return CMD_SUCCESS;
	}

	BoneMap* boneMap = NULL;
	std::map<std::string, BoneMap*>::iterator boneMapIter = mcu_p->boneMaps.find(mapName);
	if (boneMapIter == mcu_p->boneMaps.end())
	{
		LOG("Cannot find bone map name '%s'.", mapName.c_str());
		return CMD_FAILURE;
	}

	std::string mediaPath = mcuCBHandle::singleton().getMediaPath();

	boost::filesystem::path path(directory);
	
	boost::filesystem::path finalPath;
	// include the media path in the pathname if applicable
	std::string rootDir = path.root_directory();
	if (rootDir.size() == 0)
	{	
		finalPath = operator/(mediaPath, path);
	}
	else
	{
		finalPath = path;
	}

    if (!boost::filesystem::is_directory(finalPath))
	{
		LOG("Path %s is not a directory, so motion mapping will not occur.", finalPath.string().c_str());
		return CMD_FAILURE;
	}
	SBScene* scene = mcu_p->_scene;

	std::vector<std::string> motionNames = scene->getMotionNames();
	for (std::vector<std::string>::iterator iter = motionNames.begin();
		 iter != motionNames.end();
		 iter++)
	{
		SBMotion* motion = scene->getMotion((*iter));
		const std::string& fileName = motion->getMotionFileName();
		boost::filesystem::path motionPath(fileName);
		std::string motionRootDir = motionPath.root_directory();
		boost::filesystem::path finalMotionPath;
//		if (motionRootDir.size() == 0)
//		{
//			finalMotionPath = operator/(mediaPath, motionPath);
//		}
//		else
//		{
			finalMotionPath = motionPath;
//		}
		boost::filesystem::path currentPath = finalMotionPath;
		while (currentPath.has_parent_path())
		{
			boost::filesystem::path parentPath = currentPath.parent_path();
			if (boost::filesystem2::equivalent(parentPath, finalPath))
			{
				std::stringstream strstr;
				strstr << "motionmap " << motion->getName() << " " << mapName;
				mcu_p->execute((char*) strstr.str().c_str());
				break;
			}
			else
			{
				currentPath = currentPath.parent_path();
			}
		}

	}

	return CMD_SUCCESS;

}

int motionmap_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* motion = args.read_token();
	if (!motion || strcmp(motion, "help") == 0)
	{
		LOG("Use: motionmap <motion> <mapname>");
		return CMD_SUCCESS;
	}

	SkMotion* skmotion = NULL;
	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motion);
	if (iter == mcu_p->motion_map.end())
	{
		LOG("Cannot find motion name %s.", motion);
		return CMD_FAILURE;
	}
	else
	{
		 skmotion = (*iter).second;
	}

	char* mapname =  args.read_token();
	if (strlen(mapname) == 0)
	{
		LOG("Use: motionmap <motion> <mapname>");
		return CMD_SUCCESS;
	}

	// find the bone map name
	BoneMap* boneMap = NULL;
	std::map<std::string, BoneMap*>::iterator boneMapIter = mcu_p->boneMaps.find(mapname);
	if (boneMapIter == mcu_p->boneMaps.end())
	{
		LOG("Cannot find bone map name '%s'.", mapname);
		return CMD_FAILURE;
	}
	else
	{
		boneMap = (*boneMapIter).second;
	}

	// apply the map
	boneMap->apply(skmotion);

	LOG("Applied bone map %s to motion %s.", mapname, motion);
	return CMD_SUCCESS;
}

int skeletonmap_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* skeleton = args.read_token();
	if (!skeleton || strcmp(skeleton, "help") == 0)
	{
		LOG("Use: skeletonmap <skeleton> <mapname>");
		return CMD_SUCCESS;
	}

	SkSkeleton* skskeleton = NULL;
	std::map<std::string, SkSkeleton*>::iterator iter = mcu_p->skeleton_map.find(skeleton);
	if (iter == mcu_p->skeleton_map.end())
	{
		LOG("Cannot find skeleton named %s.", skeleton);
		return CMD_FAILURE;
	}
	else
	{
		 skskeleton = (*iter).second;
	}

	char* mapname =  args.read_token();
	if (strlen(mapname) == 0)
	{
		LOG("Use: skeletonmap <skeleton> <mapname>");
		return CMD_SUCCESS;
	}

	// find the bone map name
	BoneMap* boneMap = NULL;
	std::map<std::string, BoneMap*>::iterator boneMapIter = mcu_p->boneMaps.find(mapname);
	if (boneMapIter == mcu_p->boneMaps.end())
	{
		LOG("Cannot find bone map name '%s'.", mapname);
		return CMD_FAILURE;
	}
	else
	{
		boneMap = (*boneMapIter).second;
	}

	// apply the map
	boneMap->apply(skskeleton);

	LOG("Applied bone map %s to skeleton %s.", mapname, skeleton);
	return CMD_SUCCESS;
}


int mcu_steer_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if ( mcu_p )
	{
		std::string command = args.read_token();
		if (command == "help")
		{
			LOG("Use: steer <start|stop>");
			LOG("     steer move <character> <mode> <x> <y> <z>");
			return CMD_SUCCESS;
		}
		else if (command == "unit")
		{
			std::string unit = args.read_token();
			if (unit == "meter")
				mcu_p->steeringScale = 1.0f;
			else if (unit == "centimeter")
				mcu_p->steeringScale = 0.01f;
			else
				LOG("Unit %s not supported yet", unit.c_str());
			return CMD_SUCCESS;
		}
		else if (command == "start")
		{
			mcu_p->_scene->getSteerManager()->setEnable(true);
			return CMD_SUCCESS;
		}
		else if (command == "stop")
		{
			mcu_p->_scene->getSteerManager()->setEnable(false);
			return CMD_SUCCESS;
		}
		else if (command == "move")
		{
			std::string characterName = args.read_token();
			std::string mode = args.read_token();
			int num = args.calc_num_tokens();
			if (num % 3 != 0)
			{
				LOG("Syntax: steer move <character> <mode> <x1> <y1> <z1> <x2> <y2> <z2> ...");
				return CMD_FAILURE;
			}
			if (mcu_p->_scene->getSteerManager()->getEngineDriver()->isInitialized())
			{
				SbmCharacter* character = mcu_p->getCharacter(characterName);
				if (character)
				{
					if (character->steeringAgent)
					{
						character->tranjectoryGoalList.clear();
						float x, y, z;
						float yaw, pitch, roll;
						character->get_world_offset(x, y, z, yaw, pitch, roll);
						character->tranjectoryGoalList.push_back(x);
						character->tranjectoryGoalList.push_back(y);
						character->tranjectoryGoalList.push_back(z);

						if (mode == "normal")
						{
							if (character->steeringAgent->getAgent())
								character->steeringAgent->getAgent()->clearGoals();
							character->steeringAgent->goalList.clear();
							for (int i = 0; i < num; i++)
							{
								float v = args.read_float();
								character->steeringAgent->goalList.push_back(v);
								character->tranjectoryGoalList.push_back(v);
							}
						}
						else if (mode == "additive")
						{
							for (int i = 0; i < num; i++)
							{
								float v = args.read_float();
								character->steeringAgent->goalList.push_back(v);
								character->tranjectoryGoalList.push_back(v);
							}
						}
						else
							LOG("steer move mode not recognized!");
					}
				}
			}
			return CMD_SUCCESS;
		}
		else if (command == "proximity")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				character->steeringAgent->distThreshold = (float)args.read_double() / mcu_p->steeringScale;
				return CMD_SUCCESS;
			}
		}
		else if (command == "fastinitial")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				std::string fastinitialString = args.read_token();
				if (fastinitialString == "true")
					character->steeringAgent->fastInitial = true;
				else
					character->steeringAgent->fastInitial = false;
				return CMD_SUCCESS;
			}
		}
		else if (command == "speed")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				if (character->locomotion_type != character->Procedural)
				{
					character->steeringAgent->desiredSpeed = (float)args.read_double();
					return CMD_SUCCESS;
				}
			}		
		}
		else if (command == "stateprefix")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				character->statePrefix = args.read_token();
				return CMD_SUCCESS;
			}
			else
				return CMD_FAILURE;
		}
		else if (command == "type")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				std::string type = args.read_token();
				if (type == "example")
				{
					if (character->checkExamples())
						character->locomotion_type = character->Example;
					else
						character->locomotion_type = character->Basic;
					/*
					if (character->param_animation_ct)
						character->param_animation_ct->set_pass_through(false);
					if (character->locomotion_ct)
						character->locomotion_ct->set_pass_through(true);
					if (character->basic_locomotion_ct)
						character->basic_locomotion_ct->set_pass_through(true);
					*/
					return CMD_SUCCESS;
				}
				if (type == "procedural")
				{
					character->locomotion_type = character->Procedural;
					if (character->steeringAgent)
						character->steeringAgent->desiredSpeed = 1.6f;
					/*
					if (character->param_animation_ct)
						character->param_animation_ct->set_pass_through(true);
					if (character->locomotion_ct)
						character->locomotion_ct->set_pass_through(false);
					if (character->basic_locomotion_ct)
						character->basic_locomotion_ct->set_pass_through(true);
					*/

					return CMD_SUCCESS;
				}
				if (type == "basic")
				{
					character->locomotion_type = character->Basic;
					/*
					if (character->param_animation_ct)
						character->param_animation_ct->set_pass_through(true);
					if (character->locomotion_ct)
						character->locomotion_ct->set_pass_through(true);
					if (character->basic_locomotion_ct)
						character->basic_locomotion_ct->set_pass_through(false);
					*/
					return CMD_SUCCESS;
				}
			}
		}
		else if (command == "facing")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				character->steeringAgent->facingAngle = (float)args.read_double();
				return CMD_SUCCESS;
			}				
		}
		else if (command == "braking")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				character->steeringAgent->brakingGain = (float)args.read_double();
				if (character->steeringAgent->brakingGain < 1.0f)
					character->steeringAgent->brakingGain = 1.0f;
				if (character->steeringAgent->brakingGain > 4.5f)
					character->steeringAgent->brakingGain = 4.5f;
				return CMD_SUCCESS;
			}				
		}
		else if (command == "test")
		{
			std::string characterName = args.read_token();
			SbmCharacter* character = mcu_p->getCharacter(characterName);
			if (character)
			{
				character->steeringAgent->startParameterTesting();
				return CMD_SUCCESS;
			}
		}
	}
	return CMD_FAILURE;
}


int showcharacters_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
		iter != mcu_p->getCharacterMap().end();
		iter++)
	{
		LOG("%s", (*iter).second->getName().c_str());
	}
	return CMD_SUCCESS;
}


int showpawns_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu_p->getPawnMap().begin();
		iter != mcu_p->getPawnMap().end();
		iter++)
	{
		SbmCharacter* character = dynamic_cast<SbmCharacter*>((*iter).second);
		if (!character)
			LOG("%s", (*iter).second->getName().c_str());
	}

	return CMD_SUCCESS;
}

int syncpoint_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string motionStr = args.read_token();

	if (motionStr == "")
	{
		LOG("Usage: syncpoint <motion>");
		LOG("Usage: syncpoint <motion> <start> <ready> <strokestart> <stroke> <strokeend> <relax> <end>");
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu_p->motion_map.find(motionStr);
	if (iter != mcu_p->motion_map.end())
	{
		SkMotion* motion = (*iter).second;
		int num = args.calc_num_tokens();
		if (num == 0)
		{
			// show the current sync point timings
			double start = motion->time_start();
			double ready = motion->time_ready();
			double strokeStart = motion->time_stroke_start();
			double stroke = motion->time_stroke_emphasis();
			double strokeEnd = motion->time_stroke_end();
			double relax = motion->time_relax();
			double end = motion->time_stop();
			LOG("%f %f %f %f %f %f %f", start, ready, strokeStart, stroke, strokeEnd, relax, end);
			return CMD_SUCCESS;
		}
		else if (num != 7)
		{
			LOG("Usage: syncpoint <motion> <start> <ready> <strokestart> <stroke> <strokeend> <relax> <end>");
			return CMD_FAILURE;
		}
		else
		{
			double start = args.read_float();
			double ready = args.read_float();
			double strokeStart = args.read_float();
			double stroke = args.read_float();
			double strokeEnd = args.read_float();
			double relax = args.read_float();
			double end = args.read_float();
			motion->synch_points.set_time(start, ready, strokeStart, stroke, strokeEnd, relax, end);
			return CMD_SUCCESS;
		}

	}

	return CMD_SUCCESS;
}

int pawnbonebus_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
		std::string next = args.read_token();
		if (next == "")
		{
			LOG("Pawn bonebus is %s", mcu_p->sendPawnUpdates? "on" : "off");
			return CMD_SUCCESS;
		}

		if (next == "on")
		{
			mcu_p->sendPawnUpdates = true;
			return CMD_SUCCESS;
		}
		else if (next == "off")
		{
			mcu_p->sendPawnUpdates = false;
			return CMD_SUCCESS;
		}
		else
		{
			LOG("Usage: pawnbonebus <on|off>");
			return CMD_FAILURE;
		}
}

#ifdef USE_GOOGLE_PROFILER
int startprofile_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	LOG("Starting the CPU Profiler...");
	ProfilerStart("/tmp/smartbodyprofile");
	return CMD_SUCCESS;
}

int stopprofile_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	LOG("Stopping the CPU Profiler...");
	ProfilerStop();
	return CMD_SUCCESS;
}
#endif

// Usage:
// receiver echo <content>
// receiver enable
// receiver skeleton <skeletonName> <emitterType> position <joint-index/joint-name> <x> <y> <z>	
// receiver skeleton <skeletonName> <emitterType> positions <x1> <y1> <z1> <x2> <y2> <z2> ...							20 Joints in total if emitterType == "kinect"
// receiver skeleton <skeletonName> <emitterType> rotation <joint-index/joint-name> <q.w> <q.x> <q.y> <q.z>
// receiver skeleton <skeletonName> <emitterType> rotations <q1.w> <q1.x> <q1.y> <q1.z> <q2.w> <q2.x> <q2.y> <q2.z>...	20 Joints in total if emitterType == "kinect"
// 
// p.s. 
// currently position is for global and rotation is for local
int mcu_joint_datareceiver_func( srArgBuffer& args, mcuCBHandle *mcu )
{
	std::string operation = args.read_token();
	if (operation == "echo")
		LOG("%s", args.read_remainder_raw());
	if (operation == "skeleton")
	{
		std::string skelName = args.read_token();
		std::string emitterName = args.read_token();
		float scale = 1.0f;
		if (emitterName == "kinect")
			scale = 0.1f;

		std::string skeletonType = args.read_token();
		if (skeletonType == "position")
		{
			std::string jName;
			if (emitterName == "kinect")
				jName = mcu->kinectProcessor->getSBJointName(args.read_int());
			else
				jName = args.read_token();
			float x = args.read_float() * scale;
			float y = args.read_float() * scale;
			float z = args.read_float() * scale;
			SbmCharacter* character = mcu->getCharacter(skelName);	
			SrVec vec(x, y, z);
			character->datareceiver_ct->setGlobalPosition(jName, vec);
		}
		if (skeletonType == "positions")
		{
			;	// TODO: add support to joint global positions
		}
		if (skeletonType == "rotation")
		{
			std::string jName;
			if (emitterName == "kinect")
				jName = mcu->kinectProcessor->getSBJointName(args.read_int());
			else
				jName = args.read_token();

			SrQuat quat;
			quat.w = args.read_float();
			quat.x = args.read_float();
			quat.y = args.read_float();
			quat.z = args.read_float();
			SbmCharacter* character = mcu->getCharacter(skelName);	
			character->datareceiver_ct->setLocalRotation(jName, quat);
		}
		if (skeletonType == "rotations")
		{
			if (emitterName == "kinect")
			{
				int numRemainTokens = args.calc_num_tokens();
				if (numRemainTokens < 80)
				{
					LOG("Kinect skeleton %s rotation data is not valid.", skelName.c_str());
					return CMD_FAILURE;
				}
				std::vector<SrQuat> quats;
				for (int i = 0; i < 20; i++)
				{
					SrQuat quat;
					quat.w = args.read_float();
					quat.x = args.read_float();
					quat.y = args.read_float();
					quat.z = args.read_float();
					quats.push_back(quat);
				}
				KinectProcessor::processGlobalRotation(quats);
	//			mcu->kinectProcessor->filterRotation(quats);
				SbmCharacter* character = mcu->getCharacter(skelName);	
				for (int i = 0; i < 20; i++)
				{
					if (quats[i].w != 0)
						character->datareceiver_ct->setLocalRotation(mcu->kinectProcessor->getSBJointName(i), quats[i]);
				}
			}
		}
	}
	return CMD_SUCCESS;
}

int mcu_character_breathing( const char* name, srArgBuffer& args, mcuCBHandle *mcu_p) //Celso: Summer 2008
{
	SbmCharacter* char_p = mcu_p->getCharacter( name );
	if( !char_p )	
	{
		LOG( "mcu_character_breathing ERR: Character '%s' NOT FOUND\n", name ); 
		return( CMD_FAILURE );
	}

	MeCtBreathing* breathing_p = char_p->breathing_p;
	if (!breathing_p)
	{
		LOG("Character '%s' has no breathing controller.", name);
		return CMD_FAILURE;
	}

	char *breathing_cmd = args.read_token();
	
	if( strcmp( breathing_cmd, "useblendchannels" ) == 0 )	
	{
		std::string token = args.read_token();
		if (token == "true" || token == "")
		{
			breathing_p->setUseBlendChannels(true);
		}
		else if (token == "false")
		{
			breathing_p->setUseBlendChannels(false);
		}
		else
		{
			LOG("Usage: char %s breathing usechannels true|false", char_p->getName().c_str());
			return CMD_FAILURE;
		}
		return CMD_SUCCESS;
	}
	else if( strcmp( breathing_cmd, "bpm" ) == 0 )	
	{
		float bpm = args.read_float();
		bool smooth = true;
		if(args.calc_num_tokens() > 0)
		{
			char *smooth_str = args.read_token();
			smooth = (strcmp(smooth_str, "true") == 0);
		}
		breathing_p->breaths_per_minute(bpm, smooth);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "min" ) == 0 )	
	{
		float min = args.read_float();
		float transition = 1.0f;
		if(args.calc_num_tokens() > 0)
			transition = args.read_float();
		if((strcmp(breathing_p->current_breath_layer()->cycle->type(), "LinearBreathCycle") == 0)
			|| (strcmp(breathing_p->current_breath_layer()->cycle->type(), "SineBreathCycle") == 0))
			((MinMaxBreathCycle*)breathing_p->current_breath_layer()->cycle)->min(min, transition);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "max" ) == 0 )	
	{
		float max = args.read_float();
		float transition = 1.0f;
		if(args.calc_num_tokens() > 0)
			transition = args.read_float();
		if((strcmp(breathing_p->current_breath_layer()->cycle->type(), "LinearBreathCycle") == 0)
			|| (strcmp(breathing_p->current_breath_layer()->cycle->type(), "SineBreathCycle") == 0))
			((MinMaxBreathCycle*)breathing_p->current_breath_layer()->cycle)->max(max, transition);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "motion" ) == 0 )	
	{
		char *motion = args.read_token();
		SkMotion *mot_p = mcu_p->getMotion( motion );
		if( mot_p == NULL ) {
			printf( "Breathing motion '%s' NOT FOUND in motion map\n", motion ); 
			return( CMD_FAILURE );
		}
		breathing_p->motion(mot_p);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "incremental" ) == 0 )
	{
		char *incremental_str = args.read_token();
		bool incremental = (strcmp(incremental_str, "true") == 0);
		breathing_p->incremental(incremental);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "push" ) == 0 )	
	{
		char *type = args.read_token();

		if( strcmp(type, "linear") == 0)
			breathing_p->push_breath_layer(breathing_p->default_breath_cycle());
		else if( strcmp(type, "sine") == 0)
			breathing_p->push_breath_layer(new SineBreathCycle(
			breathing_p->default_breath_cycle()->min(), breathing_p->default_breath_cycle()->max()));
		else if( strcmp(type, "keyframe") == 0)
		{
			int args_count = args.calc_num_tokens();
			if((args_count % 2) != 0)
			{
				printf( "Number of arguments should be even. Try 'breathing <name> help'."); 
				return( CMD_FAILURE );
			}
			
			KeyframeBreathCycle* cycle = new KeyframeBreathCycle();
			for(int i=0; i<args_count; i = i+2)
			{
				float time = args.read_float();
				float value = args.read_float();
				cycle->keyframes.push_back(new KeyframeBreathCycle::Keyframe(value, time));
			}
			cycle->update();

			breathing_p->push_breath_layer(cycle);
		}
		else if( strcmp(type, "spline") == 0)
		{
#ifndef __ANDROID__
			int args_count = args.calc_num_tokens();
			if((args_count % 2) != 0)
			{
				printf( "Number of arguments should be even. Try 'breathing <name> help'."); 
				return( CMD_FAILURE );
			}

			MeSpline1D* spline = new MeSpline1D();
			for(int i=0; i<args_count; i = i+2)
			{
				float x = args.read_float();
				float y = args.read_float();
				spline->make_smooth(x, y, 0, 0, 0);
			}
			breathing_p->push_breath_layer(new SplineBreathCycle(spline));
#endif
		}
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "pop" ) == 0 )
	{
		breathing_p->pop_breath_layer();
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "print" ) == 0 )
	{
		breathing_p->print_state(0);
		return( CMD_SUCCESS );
	}
	else if( strcmp( breathing_cmd, "help" ) == 0 )
	{
		printf("Sintax: breathing <controller_name> <options>\n"
			"Options:\n"
			" help\n"
			"\tSets the breaths-per-minute rate.\n"
			"\tSet 'isSmooth' to 'true' or 'false' to define whether to apply a\n"
			"\tsmooth transition. Defaults to 'true'.\n"

			" min <value> [transition]\n"
			"\tSets the minimum respiratory volume. Use with the linear and sine\n"
			"\tbreath cycles only.\n"
			"\tSet 'transition' to the transition duration in seconds. Default\n"
			"\tvalue is 1.0s.\n"
			
			" max <value> [transition]\n"
			"\tSets the maximum respiratory volume. Use with the linear and sine\n"
			"\tbreath cycles only.\n"
			"\tSet 'transition' to the transition duration in seconds. Default\n"
			"\tvalue is 1.0s.\n"
			
			" motion <name> [isSmooth]\n"
			"\tSets the breathing motion.\n"

			" incremental <bool>\n"
			"\tSets whether the breathing motion is additive.\n"

			" push linear\n"
			"\tPushes to the top of the stack the (default) linear breathing cycle.\n"

			" push sine\n"
			"\tPushes to the top of the stack a sine breathing cycle.\n"
			
			" push keyframe <time1> <value1> <time2> <value2> ...\n"
			"\tPushes to the top of the stack a custom keyframe breathing cycle.\n"
			
			" push spline knot1_x knot1_y knot2_x knot2_y ...\n"
			"\tPushes to the top of the stack a spline breathing cycle.\n"
			
			" pop\n"
			"\tPops the breathing cycle from the top of the stack.\n"
			
			" print\n"
			"\tPrints the controller's state\n"
			);
					
		return( CMD_SUCCESS );
	}


	return( CMD_FAILURE );
}


int mcu_vrExpress_func( srArgBuffer& args, mcuCBHandle *mcu )
{
	if (args.calc_num_tokens() < 4)
	{
		return CMD_SUCCESS;
	}

	std::string actor = args.read_token();
	std::string to = args.read_token();
	std::string messageId = args.read_token();
	std::string xml = args.read_token();

	// get the NVBG process for the character, if available
	SbmCharacter* character = mcu->getCharacter(actor);
	if (!character)
		return CMD_SUCCESS;

	Nvbg* nvbg = character->getNvbg();
	if (!nvbg)
		return CMD_SUCCESS;

	bool ok = nvbg->execute(actor, to, messageId, xml);

	if (!ok)
	{
		LOG("NVBG for character %s to %s did not handle message %s.", actor.c_str(), to.c_str(), messageId.c_str());
	}
	return CMD_SUCCESS;
}


int mcu_vhmsg_connect_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	char* vhmsgServerVar = getenv( "VHMSG_SERVER" );
	char* vhmsgPortVar = getenv("VHMSG_PORT");
	char* vhmsgScopeVar =  getenv("VHMSG_SCOPE");

	std::string vhmsgServer = "localhost";
	if (vhmsgServerVar)
		vhmsgServer = vhmsgServerVar;

	std::string vhmsgPort = "61616";
	if (vhmsgPortVar)
		vhmsgPort = vhmsgPortVar;

	std::string vhmsgScope = "DEFAULT_SCOPE";

	if (args.calc_num_tokens() > 0)
	{
		vhmsgServer = args.read_token();
	}

	if (args.calc_num_tokens() > 0)
	{
		vhmsgPort = args.read_token();
	}

	if (args.calc_num_tokens() > 0)
	{
		vhmsgScope = args.read_token();
	}

	// disconnect first in case we are already connected

	vhmsg::ttu_close();

	if (vhmsg::ttu_open(vhmsgServer.c_str(), vhmsgScope.c_str(), vhmsgPort.c_str())==vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( mcu_vhmsg_callback );
		int err = vhmsg::TTU_SUCCESS;
		err = vhmsg::ttu_register( "sbm" );
		err = vhmsg::ttu_register( "vrAgentBML" );
		err = vhmsg::ttu_register( "vrExpress" );
		err = vhmsg::ttu_register( "vrSpeak" );
		err = vhmsg::ttu_register( "RemoteSpeechReply" );
		err = vhmsg::ttu_register( "PlaySound" );
		err = vhmsg::ttu_register( "StopSound" );
		err = vhmsg::ttu_register( "CommAPI" );
		err = vhmsg::ttu_register( "object-data" );
		err = vhmsg::ttu_register( "vrAllCall" );
		err = vhmsg::ttu_register( "vrKillComponent" );
		err = vhmsg::ttu_register( "wsp" );
		err = vhmsg::ttu_register( "receiver" );
		err = vhmsg::ttu_register( "sbmdebugger" );
		err = vhmsg::ttu_register( "vrPerception" );
		mcu_p->vhmsg_enabled = true;
		return CMD_SUCCESS;
	} 
	else
	{
		LOG("Could not connect to %s:%s", vhmsgServer.c_str(), vhmsgPort.c_str());
		mcu_p->vhmsg_enabled = false;
		return CMD_FAILURE;
	}

}

int mcu_vhmsg_disconnect_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (!mcu_p->vhmsg_enabled)
	{
		LOG("VHMSG is not connected.");
		return CMD_FAILURE;
	}
	int ret = vhmsg::ttu_close();
	if (ret == vhmsg::TTU_ERROR)
	{
		LOG("Problem disconnecting VHMSG.");
		return CMD_FAILURE;
	}
	else
	{
		mcu_p->vhmsg_enabled = false;
		LOG("VHMSG has been disconnected.");
		return CMD_SUCCESS;
	}
	
}

void mcu_vhmsg_callback( const char *op, const char *args, void * user_data )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	switch( mcu.execute( op, (char*) args ) )
	{
        case CMD_NOT_FOUND:
            LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
            break;
        case CMD_FAILURE:
            LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
            break;
    }
}

int mcuFestivalRemoteSpeechCmd_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	FestivalSpeechRelayLocal* speechRelay = mcu_p->festivalRelay();
	const char* message = args.read_remainder_raw();
	speechRelay->processSpeechMessage(message);
	//processSpeechMessage(
	return 0;
}

int register_animation_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	SkMotion* motion = mcu_p->getMotion("ChrRio_Entry001");
	if (!motion)
		return CMD_FAILURE;

	motion->registerAnimation();
	return CMD_SUCCESS;

}

int unregister_animation_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	SkMotion* motion = mcu_p->getMotion("ChrRio_Entry001");
	if (!motion)
		return CMD_FAILURE;

	motion->unregisterAnimation();
	return CMD_SUCCESS;

}

int resetanim_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string motionName;
	std::string outputFile;

	if (args.calc_num_tokens() > 0)
	{
		motionName = args.read_token();
	}
	else
	{
		LOG("No motion named %s found. Usage: resetanim <motion> <outputfile>", motionName.c_str());
		return CMD_FAILURE;
	}

	if (args.calc_num_tokens() > 0)
	{
		outputFile = args.read_token();
	}
	else
	{
		LOG("No output file name entered. Usage: resetanim <motion> <outputfile>");
		return CMD_FAILURE;
	}

	SrOutput* out = new SrOutput(outputFile.c_str(), "w");
	if (!out->valid())
	{
		LOG("Cannot write to file %s", outputFile.c_str());
		delete out;
		return CMD_FAILURE;
	}

	SkMotion* motion = mcu_p->getMotion(motionName);
	if (!motion)
	{
		LOG("Motion %s not found. Cannot reset motion to origin.", motionName.c_str());
		return CMD_FAILURE;
	}
	
	// make sure that the motion ends at the origin (0,0,0)
	int numFrames = motion->frames();

	float* lastPosture = motion->posture(numFrames - 1);
	// get the base x, y, z
	SkChannelArray& channels = motion->channels();
	int pos[3];
	pos[0] = channels.search("base", SkChannel::XPos);
	pos[1] = channels.search("base", SkChannel::YPos);
	pos[2] = channels.search("base", SkChannel::ZPos);

	if (pos[0] == -1 || pos[1] == -1 || pos[2] == -1)
	{
		LOG("No base position found in motion %s, cannot register.", motionName.c_str());
		return CMD_FAILURE;
	}

	SrVec last(lastPosture[pos[0]], lastPosture[pos[1]], lastPosture[pos[2]]);

	for (int f = 0; f < numFrames; f++)
	{
		float* curFrame = motion->posture(f);
		for (int p = 0; p < 3; p++)
		{
			curFrame[pos[p]] = curFrame[pos[p]] - last[p];
		}

	}

	motion->save(*out);

	LOG("Motion saved to %s", outputFile.c_str());

	delete out;
	return CMD_SUCCESS;
}

int animation_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string motionName;

	if (args.calc_num_tokens() > 0)
	{
		motionName = args.read_token();
	}
	else
	{
		LOG("Usage: animation <motion> save");
		LOG("       animation <motion> frame <framenum>");
		LOG("       animation <motion> translate <x> <y> <z>");
		LOG("       animation <motion> rotate <x> <y> <z>");
		LOG("       animation <motion> scale <factor>");
		//LOG("       animation <motion> trim <from> <to>");

		return CMD_SUCCESS;
	}

	SkMotion* motion = mcu_p->getMotion(motionName);
	if (!motion)
	{
		LOG("Motion %s not found. Cannot reset motion to origin.", motionName.c_str());
		return CMD_FAILURE;
	}

	std::string command = "";

	if (args.calc_num_tokens() == 0)
	{
		// dump information about the motion
		LOG("Motion name:   %s", motion->getName().c_str());
		LOG("Motion source: %s", motion->filename().c_str());
		LOG("Num Frames:    %d", motion->frames());
		LOG("Num Channels:  %d", motion->channels().size());

		return CMD_SUCCESS;
	}
	else
	{
		command = args.read_token();
	}

	if (command == "save")
	{
		if (args.calc_num_tokens() == 0)
		{
			LOG("Usage: animation <motion> save");
			return CMD_FAILURE;
		}
		std::string outputFile = args.read_token();
		SrOutput* out = new SrOutput(outputFile.c_str(), "w");
		if (!out->valid())
		{
			LOG("Cannot write to file %s", outputFile.c_str());
			delete out;
			return CMD_FAILURE;
		}

		motion->save(*out);

		LOG("Motion %s saved to %s", motionName.c_str(), outputFile.c_str());

		delete out;
		return CMD_SUCCESS;
	}
	else if (command == "frame")
	{
		if (args.calc_num_tokens() == 0)
		{
			LOG("Usage: animation <motion> frame <framenum>");
			return CMD_FAILURE;
		}
		int index = args.read_int();

		if (index >=0 && index < motion->frames())
		{
			std::stringstream strstr;
			float* posture = motion->posture(index);
			for (int i = 0; i < motion->posture_size(); i++)
			{
				strstr << posture[i] << " ";
			}
			LOG("Motion %s frame %d:", motionName.c_str(), index);
			LOG(strstr.str().c_str());
			return CMD_SUCCESS;
		}
	}
	else if (command == "scale")
	{
		if (args.calc_num_tokens() == 0)
		{
			LOG("Usage: animation <motion> scale <factor>");
			return CMD_FAILURE;
		}
		float factor = args.read_float();

		SBMotion* sbMotion = dynamic_cast<SBMotion*>(motion);
		bool result = sbMotion->scale(factor);
		if (result)
			return CMD_SUCCESS;
		else
			return CMD_FAILURE;
	}
	else if (command == "retime")
	{
		if (args.calc_num_tokens() == 0)
		{
			LOG("Usage: animation <motion> retime <factor>");
			return CMD_FAILURE;
		}
		float factor = args.read_float();

		SBMotion* sbMotion = dynamic_cast<SBMotion*>(motion);
		bool result = sbMotion->retime(factor);
		if (result)
			return CMD_SUCCESS;
		else
			return CMD_FAILURE;
	}
	else if (command == "translate")
	{
		if (args.calc_num_tokens() < 3)
		{
			LOG("Usage: animation <motion> translate <x> <y> <z>");
			return CMD_FAILURE;
		}
		SrVec offset;
		offset[0] = args.read_float();
		offset[1] = args.read_float();
		offset[2] = args.read_float();

		SBMotion* sbMotion = dynamic_cast<SBMotion*>(motion);
		bool result = sbMotion->translate(offset[0], offset[1], offset[2], "base");
		if (result)
			return CMD_SUCCESS;
		else
			return CMD_FAILURE;
	}	
	else if (command == "rotate")
	{
		if (args.calc_num_tokens() < 3)
		{
			LOG("Usage: animation <motion> rotate <x> <y> <z>");
			return CMD_FAILURE;
		}
		SrVec rotation;
		rotation[0] = args.read_float();
		rotation[1] = args.read_float();
		rotation[2] = args.read_float();

		SBMotion* sbMotion = dynamic_cast<SBMotion*>(motion);
		bool result = sbMotion->rotate(rotation[0], rotation[1], rotation[2], "base");
		if (result)
			return CMD_SUCCESS;
		else
			return CMD_FAILURE;
	}
	else
	{
		LOG("Usage: animation <motion> save");
		LOG("       animation <motion> frame <framenum>");
		LOG("       animation <motion> translate <x> <y> <z>"); 
		LOG("       animation <motion> rotate <x> <y> <z>");
		LOG("       animation <motion> scale <factor>");
		LOG("       animation <motion> retime <factor>");
		//LOG("       animation <motion> trim <from> <to>");

		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

int vhmsglog_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if (args.calc_num_tokens() == 0)
	{
		if (mcu_p->logListener)
		{
			LOG("VHMSG logging is on");
		}
		else
		{
			LOG("VHMSG logging is off");
		}
		return CMD_SUCCESS;
	}

	std::string token = args.read_token();
	if (token == "on")
	{
		if (mcu_p->logListener)
		{
			LOG("VHMSG logging is already on");
			return CMD_SUCCESS;
		}
		else
		{
			mcu_p->logListener = new VHMsgLogger();
			vhcl::Log::g_log.AddListener(mcu_p->logListener);
			LOG("VHMSG logging is now on");
			return CMD_SUCCESS;
		}
	}
	else if (token == "off")
	{
		if (!mcu_p->logListener)
		{
			LOG("VHMSG logging is already off");
			return CMD_SUCCESS;
		}
		else
		{
			vhcl::Log::g_log.RemoveListener(mcu_p->logListener);
			delete mcu_p->logListener;
			mcu_p->logListener = NULL;
			LOG("VHMSG logging is now off");
			return CMD_SUCCESS;
		}
	}
	else
	{
		LOG("Usage: vhmsglog <on|off>");
		return CMD_FAILURE;
	}

}

int skscale_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	int num = args.calc_num_tokens();
	if ( num > 1 )
	{
		LOG("Use: skscale <value>");
		return CMD_FAILURE;
	}

	if ( num == 1 )
	{
		double value = args.read_double();

		if (mcu_p)
		{
			mcu_p->skScale = value;
		}
		return CMD_SUCCESS;
	}

	return CMD_FAILURE;
}

int skmscale_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	int num = args.calc_num_tokens();
	if ( num > 1 )
	{
		LOG("Use: skmscale <value>");
		return CMD_FAILURE;
	}

	if ( num == 1 )
	{
		double value = args.read_double();

		if (mcu_p)
		{
			mcu_p->skmScale = value;
		}
		return CMD_SUCCESS;
	}

	return CMD_FAILURE;
}

int mcu_reset_func( srArgBuffer& args, mcuCBHandle *mcu_p  )
{
	// TODO: If arg, call as init, else call previous init
	mcu_p->reset();
	return( CMD_SUCCESS );
}

int mcu_echo_func( srArgBuffer& args, mcuCBHandle *mcu_p  )
{
	std::stringstream timeStr;
	timeStr << mcu_p->time;
	std::string echoStr = args.read_remainder_raw();
	int pos = echoStr.find("$time");
	if (pos != std::string::npos)
		boost::replace_all(echoStr, "$time", timeStr.str());
	
    LOG("%s ", echoStr.c_str() );
	return( CMD_SUCCESS );
}

int sbm_main_func( srArgBuffer & args, mcuCBHandle * mcu_p )
{
   const char * token = args.read_token();
   if ( strcmp( token, "id" ) == 0 )
   {  // Process specific
      token = args.read_token(); // Process id
      const char * process_id = mcu_p->process_id.c_str();
      if( ( mcu_p->process_id == "" )         // If process id unassigned
         || strcmp( token, process_id ) !=0 ) // or doesn't match
         return CMD_SUCCESS;                  // Ignore.
      token = args.read_token(); // Sub-command
   }

   const char * args_raw = args.read_remainder_raw();
   srArgBuffer arg_buf( args_raw );
   int result = mcu_p->execute( token, arg_buf );
   switch( result )
   {
      case CMD_NOT_FOUND:
         LOG( "SBM ERR: command NOT FOUND: '%s %s'> ", token, args_raw );
         break;
      case CMD_FAILURE:
         LOG( "SBM ERR: command FAILED: '%s %s'> ", token, args_raw );
         break;
      case CMD_SUCCESS:
         break;
      default:
         break;
   }

   return CMD_SUCCESS;
}

int sbm_vhmsg_send_func( srArgBuffer& args, mcuCBHandle *mcu_p  )
{
	char* cmdName = args.read_token();
	char* cmdArgs = args.read_remainder_raw();
	return mcu_p->vhmsg_send( cmdName, cmdArgs );
}

int xmlcachedir_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	return CMD_SUCCESS;
}

int xmlcache_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::string token = args.read_token();
	if (token == "on")
	{
		mcu_p->useXmlCache = true;
		LOG("XML caching is now on");
		return CMD_SUCCESS;
	}
	else if (token == "off")
	{
		mcu_p->useXmlCache = false;
		LOG("XML caching is now off");
		return CMD_SUCCESS;
	}
	else if (token == "auto")
	{
		std::string token = args.read_token();
		if (token == "on")
		{
			mcu_p->useXmlCacheAuto = true;
			LOG("XML automatic caching is now on");
			return CMD_SUCCESS;
		}
		else if (token == "off")
		{
			mcu_p->useXmlCacheAuto = false;
			LOG("XML automatic caching is now off");
			return CMD_SUCCESS;
		}
		else
		{
			LOG("XML automatic caching is %s", mcu_p->useXmlCacheAuto? "on" : "off");
			return CMD_SUCCESS;
		}
	}
	else if (token == "reset")
	{
		// delete the cache
		for (std::map<std::string, DOMDocument*>::iterator iter = mcu_p->xmlCache.begin();
			 iter != mcu_p->xmlCache.end();
			 iter++)
		{
			(*iter).second->release();
		}
		LOG("XML cache cleared.");
		return CMD_SUCCESS;
	}
	else
	{
		LOG("Current cache:");
		for (std::map<std::string, DOMDocument*>::iterator iter = mcu_p->xmlCache.begin();
			 iter != mcu_p->xmlCache.end();
			 iter++)
		{
			LOG("%s", (*iter).first.c_str());
		}
		return CMD_SUCCESS;
	}
}
