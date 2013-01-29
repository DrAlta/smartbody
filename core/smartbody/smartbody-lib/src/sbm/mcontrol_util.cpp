/*
 *  mcontrol_util.cpp - part of SmartBody-lib
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

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#include "sbm/lin_win.h"
#include "vhcl.h"
#include "sbm/mcontrol_util.h"

#include <stdlib.h>
#include <iostream>
#include <string>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif
#endif

#include "sbm_audio.h"
#include "sb/SBScene.h"
#include "me_utilities.hpp"
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>

#if USE_WSP
#include "wsp.h"
#endif

#ifndef __native_client__

#include <sb/SBPythonClass.h>
#include <sb/SBPython.h>

#ifndef SB_NO_PYTHON
#include <boost/python.hpp> // boost python support
#endif

#else
#ifndef SB_NO_PYTHON
#undef USE_PYTHON
#endif 
#endif

#include "sr/sr_model.h"

#if !defined (__ANDROID__) && !defined(SBM_IPHONE)  && !defined(__native_client__)// disable shader support
#ifndef __native_client__
#include "sbm/GPU/SbmShader.h"
#include "sbm/GPU/SbmTexture.h"
#include "sbm/GPU/SbmDeformableMeshGPU.h"
#endif
#endif

#if __native_client__
#include "sbm_test_cmds.hpp"
#include "resource_cmds.h"
#endif


#include "sbm_deformable_mesh.h"
#include <sbm/ODEPhysicsSim.h>
#include <sbm/locomotion_cmds.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sb/SBBoneBusManager.h>
#include <sb/SBScript.h>
#include <sb/SBServiceManager.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotion.h>
#include <sb/SBAssetManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBDebuggerServer.h>
#include <sb/SBDebuggerClient.h>
#include <controllers/me_ct_param_animation_utilities.h>
#include <sb/PABlend.h>
#include <controllers/me_ct_saccade.h>
#include <controllers/me_ct_data_receiver.h>
#include <controllers/me_ct_eyelid.h>
#include <controllers/me_ct_scheduler2.h>
#include <controllers/me_ct_breathing.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_controller_tree_root.hpp>
#include <controllers/me_ct_example_body_reach.hpp>
#include <sbm/MiscCommands.h>

#include "Heightfield.h"
#include "sb/sbm_pawn.hpp"
#include "sb/sbm_character.hpp"
#include <sb/nvbg.h>
#include <sb/SBScene.h>
#include <sbm/KinectProcessor.h>
#include <sb/SBJointMap.h>
#include <sb/SBEvent.h>
#include <sr/sr_viewer.h>
#include <sr/sr_camera.h>
#include <sb/SBCharacterListener.h>
#include <sb/SBAnimationTransition.h>


using namespace std;

#if USE_WSP
using namespace WSP;
#endif

const bool LOG_ABORTED_SEQ = false;

/////////////////////////////////////////////////////////////

#if 0
void mcuCBHandle::test_map( void )	{
	SbmPawn* pawn_p = NULL;

#if 1
	srHashMap <SbmPawn> pawn_map_cp( pawn_map );
#endif

	printf( "iterate ---\n" );

	pawn_map.reset();
	while( pawn_p = pawn_map.next() )	{

		printf( "iterate pawn: %s\n", pawn_p->name );
	
#if 1
		SbmPawn* pawn_cp_p = NULL;
		pawn_map_cp.reset();
		while( pawn_cp_p = pawn_map_cp.next() )	{
			printf( "  pawn: %s\n", pawn_cp_p->name );
		}
#endif	
	}
	printf( "---\n" );
}
#endif

/////////////////////////////////////////////////////////////
//  Singleton Instance
mcuCBHandle* mcuCBHandle::_singleton = NULL;

/////////////////////////////////////////////////////////////


SequenceManager::SequenceManager()
{
}

SequenceManager::~SequenceManager()
{
	clear();
}

void SequenceManager::clear()
{
	for (size_t x = 0; x < _sequences.size(); x++)
	{
		srCmdSeq* seq = _sequences[x].second;
		seq->reset();
		while(char* cmd = seq->pull() )	{
			delete [] cmd;
		}
		delete seq;
	}

	_sequenceSet.clear();
	_sequences.clear();
}

bool SequenceManager::addSequence(const std::string& seqName, srCmdSeq* seq)
{
	if (_sequenceSet.find(seqName) != _sequenceSet.end())
		return false;

	_sequenceSet.insert(seqName);
	_sequences.push_back(std::pair<std::string, srCmdSeq*>(seqName, seq));
	return true;
}

bool SequenceManager::removeSequence(const std::string& seqName, bool deleteSequence)
{
	std::set<std::string>::iterator iter = _sequenceSet.find(seqName);
	if (iter == _sequenceSet.end())
		return false;

	_sequenceSet.erase(iter);

	for (std::vector<std::pair<std::string, srCmdSeq*> >::iterator iter = _sequences.begin();
		iter != _sequences.end();
		iter++)
	{
		if ((*iter).first == seqName)
		{
			if (deleteSequence)
				delete (*iter).second;
			_sequences.erase(iter);
			return true;
		}
	}

	LOG("Could not find sequence in active sequence queue. Please check code - this should not happen.");
	return false;
}

srCmdSeq* SequenceManager::getSequence(const std::string& name)
{
	for (std::vector<std::pair<std::string, srCmdSeq*> >::iterator iter = _sequences.begin();
		iter != _sequences.end();
		iter++)
	{
		if ((*iter).first == name)
		{
			return (*iter).second;
		}
	}

	return NULL;
}

srCmdSeq* SequenceManager::getSequence(int num, std::string& name)
{
	if (_sequences.size() > (size_t) num)
	{
		name = _sequences[num].first;
		return _sequences[num].second;
	}
	else
	{
		return NULL;
	}
}

int SequenceManager::getNumSequences()
{
	return _sequences.size();
}

mcuCBHandle::mcuCBHandle()
:	loop( true ),
	vhmsg_enabled( false ),	
	internal_timer_p( NULL ),
	external_timer_p( NULL ),
	timer_p( NULL ),
	time( 0.0 ),
	time_dt( 0.0 ),	
	internal_profiler_p( NULL ),
	external_profiler_p( NULL ),
	profiler_p( NULL ),
	net_bone_updates( false ),
	net_world_offset_updates( true ),
	play_internal_audio( false ),
	resourceDataChanged( false ),
	perceptionData( new PerceptionData() ),
	skScale( 1.0 ),
	skmScale( 1.0 ),
	viewer_p( NULL ),
	ogreViewer_p( NULL ),
	camera_p( NULL ),
	root_group_p( new SrSnGroup() ),
	height_field_p( NULL ),
	test_character_default( "" ),
	test_recipient_default( "ALL" ),
	queued_cmds( 0 ),
	updatePhysics( false ),
	viewer_factory ( new SrViewerFactory() ),
	ogreViewerFactory ( new SrViewerFactory() ),
	snapshot_counter( 1 ),
	use_python( true ),
	sendPawnUpdates(false),
	logListener(NULL),
	useXmlCache(false),
	useXmlCacheAuto(false),
	initPythonLibPath("")
{	
	testBMLId = 0;
	registerCallbacks();
	root_group_p->ref();
	kinectProcessor = new KinectProcessor();
#if USE_WSP
	theWSP = WSP::create_manager();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );
#endif

}

/////////////////////////////////////////////////////////////

mcuCBHandle::~mcuCBHandle() {
	clear();

	// clean up factories and time profiler which are set externally
	internal_profiler_p = NULL;
	external_profiler_p = NULL;
	profiler_p = NULL;
	internal_timer_p = NULL;
	external_timer_p = NULL;
	timer_p = NULL;
	viewer_factory = NULL;
	ogreViewerFactory = NULL;

	// clean up python
#ifndef SB_NO_PYTHON
	Py_Finalize();

#if defined(WIN_BUILD)
	{
		// According to the python docs, .pyd files are not unloaded during Py_Finalize().
		// This causes issues when trying to re-load the smartbody dll over and over.
		// So, we force unload these .pyd files.  This list is all the standard .pyd files included in the Python26 DLLs folder.
		// For reference:  http://docs.python.org/2/c-api/init.html  "Dynamically loaded extension modules loaded by Python are not unloaded"

		// initPythonLibPath - eg:  "../../../../core/smartbody/Python26/Lib"

		HMODULE hmodule;
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/bz2.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/pyexpat.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/select.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/unicodedata.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/winsound.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_bsddb.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes_test.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_elementtree.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_hashlib.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_msi.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_multiprocessing.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_socket.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_sqlite3.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ssl.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_testcapi.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_tkinter.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
	}
#endif  // WIN_BUILD
#endif  // USE_PYTHON
}

void mcuCBHandle::reset( void )	
{
	// clear everything
	clear();

	// initialize everything
	loop = true;
	vhmsg_enabled = false;
	net_bone_updates = false;
	net_world_offset_updates = true;
	play_internal_audio = false;
	resourceDataChanged = false;
	perceptionData = new PerceptionData();
	skmScale = 1.0;
	skScale = 1.0;
	root_group_p = new SrSnGroup();
	test_character_default = "";
	test_recipient_default = "ALL";
	queued_cmds = 0;
	updatePhysics = false;
	snapshot_counter = 1;
	use_python = true;
	sendPawnUpdates = false;
	logListener = NULL;
	useXmlCache = false;
	useXmlCacheAuto = false;
	testBMLId = 0;
	registerCallbacks();
	root_group_p->ref();
	kinectProcessor = new KinectProcessor();
#if USE_WSP
	theWSP = WSP::create_manager();
	theWSP->init( "SMARTBODY" );
#endif


}





void mcuCBHandle::registerCallbacks()
{
	insert( "sb",			sb_main_func );
	insert( "sbm",			sbm_main_func );
	insert( "help",			mcu_help_func );

	insert( "reset",		mcu_reset_func );
	insert( "echo",			mcu_echo_func );
	
	insert( "path",			mcu_filepath_func );
	insert( "seq",			mcu_sequence_func );
	insert( "seq-chain",	mcu_sequence_chain_func );
	insert( "send",			sbm_vhmsg_send_func );

	//  cmd prefixes "set" and "print"
	insert( "set",          mcu_set_func );
	insert( "print",        mcu_print_func );
	insert( "test",			mcu_test_func );

	insert( "camera",		mcu_camera_func );
	insert( "terrain",		mcu_terrain_func );
	insert( "time",			mcu_time_func );
	insert( "tip",			mcu_time_ival_prof_func );

	insert( "panim",		mcu_panim_cmd_func );	
	insert( "physics",		mcu_physics_cmd_func );	
	insert( "mirror",       mcu_motion_mirror_cmd_func);
	insert( "motionplayer", mcu_motion_player_func);

	insert( "pawn",			pawn_cmd_func );
	insert( "char",			character_cmd_func );

	insert( "ctrl",			mcu_controller_func );
	insert( "wsp",			mcu_wsp_cmd_func );
	insert( "create_remote_pawn", create_remote_pawn_func );

	insert( "vrAgentBML",   BML_PROCESSOR::vrAgentBML_cmd_func );
	insert( "bp",		    BML_PROCESSOR::bp_cmd_func );
	insert( "vrSpeak",		BML_PROCESSOR::vrSpeak_func );
	insert( "vrExpress",  mcu_vrExpress_func );

	insert( "receiver",		mcu_joint_datareceiver_func );
	insert( "net_reset",           mcu_net_reset );
	insert( "net_check",           mcu_net_check );
	insert( "RemoteSpeechCmd"  ,   mcuFestivalRemoteSpeechCmd_func );
	insert( "RemoteSpeechReply",   remoteSpeechResult_func );
	insert( "RemoteSpeechTimeOut", remoteSpeechTimeOut_func);  // internally routed message
//	insert( "locomotion",          locomotion_cmd_func );
//	insert( "loco",                locomotion_cmd_func ); // shorthand
	insert( "syncpolicy",          mcu_syncpolicy_func );
	insert( "check",			   mcu_check_func);		// check matching between .skm and .sk
	insert( "pythonscript",		   mcu_pythonscript_func);
	insert( "python",			   mcu_python_func);
	insert( "p",				   mcu_python_func);
	insert( "interp",		       mcu_interp_func);
	insert( "adjustmotion",		   mcu_adjust_motion_function);
	insert( "mediapath",		   mcu_mediapath_func);
	insert( "bml",				   test_bml_func );
	insert( "triggerevent",		   triggerevent_func );
	insert( "addevent",			   addevent_func );
	insert( "removeevent",		   removeevent_func );
	insert( "enableevents",	       enableevents_func );
	insert( "disableevents",	   disableevents_func );
	insert( "registerevent",       registerevent_func );
	insert( "unregisterevent",     unregisterevent_func );
	insert( "setmap",			   setmap_func );
	insert( "motionmap",		   motionmap_func );
	insert( "motionmapdir",		   motionmapdir_func );
	insert( "skeletonmap",		   skeletonmap_func );
	insert( "steer",			   mcu_steer_func);	
	insert( "characters",		   showcharacters_func );
	insert( "pawns",			   showpawns_func );
	insert( "RemoteSpeechReplyRecieved", remoteSpeechReady_func);  // TODO: move to test commands
	insert( "syncpoint",		   syncpoint_func);
	insert( "pawnbonebus",		   pawnbonebus_func);
	insert( "vhmsgconnect",		   mcu_vhmsg_connect_func);
	insert( "vhmsgdisconnect",	   mcu_vhmsg_disconnect_func);
    insert( "resetanimation",	   resetanim_func);
	insert( "animation",		   animation_func);
	insert( "vhmsglog",			   vhmsglog_func);
	insert( "skscale",			   skscale_func);
	insert( "skmscale",			   skmscale_func);

	insert( "xmlcachedir",		   xmlcachedir_func);
	insert( "xmlcache",			   xmlcache_func);


#ifdef USE_GOOGLE_PROFILER
	insert( "startprofile",			   startprofile_func );
	insert( "stopprofile",			   stopprofile_func );
	insert( "startheapprofile",			   startheapprofile_func );
	insert( "stopheapprofile",			   stopheapprofile_func );
#endif
	insert_set_cmd( "bp",             BML_PROCESSOR::set_func );
	insert_set_cmd( "pawn",           pawn_set_cmd_funcx );
	insert_set_cmd( "character",      character_set_cmd_func );
	insert_set_cmd( "char",           character_set_cmd_func );
	insert_set_cmd( "face",           mcu_set_face_func );
	insert_set_cmd( "test",           sbm_set_test_func );

	insert_print_cmd( "bp",           BML_PROCESSOR::print_func );
	insert_print_cmd( "face",         mcu_print_face_func );
	insert_print_cmd( "test",         sbm_print_test_func );

	insert_test_cmd( "args", test_args_func );
	insert_test_cmd( "bml",  test_bml_func );
	insert_test_cmd( "fml",  test_fml_func );
	insert_test_cmd( "rhet", remote_speech_test);
	insert_test_cmd( "bone_pos", test_bone_pos_func );
	

	insert( "net",	mcu_net_func );

	insert( "PlaySound", mcu_play_sound_func );
	insert( "StopSound", mcu_stop_sound_func );

	insert( "uscriptexec", mcu_uscriptexec_func );

	insert( "CommAPI", mcu_commapi_func );

	insert( "vrKillComponent", mcu_vrKillComponent_func );
	insert( "vrAllCall", mcu_vrAllCall_func );
	insert( "vrPerception", mcu_vrPerception_func );
	insert( "vrBCFeedback", mcu_vrBCFeedback_func );
	insert( "vrSpeech", mcu_vrSpeech_func );
	insert( "sbmdebugger", mcu_sbmdebugger_func );

	insert( "text_speech", text_speech::text_speech_func ); // [BMLR]
}


 FILE* mcuCBHandle::open_sequence_file( const char *seq_name, std::string& fullPath ) {

	FILE* file_p = NULL;

	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];	
	// add the .seq extension if necessary
	std::string candidateSeqName = seq_name;
	if (candidateSeqName.find(".seq") == std::string::npos)
	{
		candidateSeqName.append(".seq");
	}
	sprintf( label, "%s", candidateSeqName.c_str());
	// current path containing .exe
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);

	seq_paths.reset();
	std::string filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
	//filename = mcn_return_full_filename_func( CurrentPath, filename );
	//LOG("seq name = %s, filename = %s\n",seq_name,filename.c_str());
	
	while(filename.size() > 0)	{
		file_p = fopen( filename.c_str(), "r" );
		if( file_p != NULL ) {
	
			fullPath = filename;			
			break;
		}
		filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension	
		sprintf( label, "%s.seq", seq_name );
		seq_paths.reset();
		filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
		while( filename.size() > 0 )	{
			if( ( file_p = fopen( filename.c_str(), "r" ) ) != NULL ) {
				
				fullPath = filename;
				break;
			}
			filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
			//filename = mcn_return_full_filename_func( CurrentPath, filename );
		}
	}

	// return empty string if file not found
	return file_p;
}

/**
 *  Clears the contents of the mcuCBHandle.
 *  Used by reset and destructor.
 */
void mcuCBHandle::clear( void )	
{
	if (logListener)
	{
		vhcl::Log::g_log.RemoveListener(logListener);
		delete logListener;
		logListener = NULL;
	}
	
	if (perceptionData)
	{
		delete perceptionData;
		perceptionData = NULL;
	}

	if (kinectProcessor)
	{
		delete kinectProcessor;
		kinectProcessor = NULL;
	}

	//close_viewer();
	if (viewer_p)	
	{
		viewer_factory->reset(viewer_p);
		viewer_p = NULL;
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}


	ogreViewerFactory->remove(ogreViewer_p);
	if (ogreViewer_p)
	{
		delete ogreViewer_p;
		ogreViewer_p = NULL;
	}

	if (camera_p)
	{
		delete camera_p;
		camera_p = NULL;
	}

	if( root_group_p )	
	{
		root_group_p->unref();
		root_group_p = NULL;
	}

	if( height_field_p )	
	{
		delete height_field_p;
		height_field_p = NULL;
	}

/*	cmd_map.reset();


	while (cmd_map.getHashMap().pull() != NULL)
	{
	}
	set_cmd_map.reset();
	while (set_cmd_map.getHashMap().pull() != NULL)
	{
	}
	print_cmd_map.reset();
	while (print_cmd_map.getHashMap().pull() != NULL)
	{
	}
	test_cmd_map.reset();
	while (test_cmd_map.getHashMap().pull() != NULL)
	{
	}
*/



	for (std::map<std::string, DeformableMesh*>::iterator deformableIter = deformableMeshMap.begin();
		deformableIter != deformableMeshMap.end();
		deformableIter++)
	{
		DeformableMesh* deformableMesh = (*deformableIter).second;
	//	delete deformableMesh;
	}
	deformableMeshMap.clear();
	//SbmDeformableMeshGPU::initShader = false;
	//SbmShaderManager::destroy_singleton();

	cameraTracking.clear();


}

/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////

int mcuCBHandle::open_viewer( int width, int height, int px, int py )	{	
	
	if( viewer_p == NULL )	{
		if (!viewer_factory)
			return CMD_FAILURE;
		viewer_p = viewer_factory->create( px, py, width, height );
		viewer_p->label_viewer( "SBM Viewer - Local Mode" );
		camera_p = new SrCamera;
		viewer_p->set_camera( camera_p );
		//((FltkViewer*)viewer_p)->set_mcu(this);
		viewer_p->show_viewer();
		if( root_group_p )	{
			viewer_p->root( root_group_p );
		}
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(viewer_p);
#endif
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_viewer( void )	{

	if( viewer_p )	{
		viewer_factory->remove(viewer_p);
		viewer_p = NULL;
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}
	if( camera_p )	{
		delete camera_p;
		camera_p = NULL;
	}
}

int mcuCBHandle::openOgreViewer( int width, int height, int px, int py )	{	

	if( ogreViewer_p == NULL )	{
		if (!ogreViewerFactory)
			return CMD_FAILURE;
		ogreViewer_p = ogreViewerFactory->create( px, py, width, height );
		ogreViewer_p->label_viewer( "SB Ogre Viewer" );
		camera_p = new SrCamera;
		ogreViewer_p->set_camera( camera_p );		
		ogreViewer_p->show_viewer();
		if( root_group_p )	{
			ogreViewer_p->root( root_group_p );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::closeOgreViewer( void )	{

	if( ogreViewer_p )	{
		ogreViewerFactory->remove(ogreViewer_p);
		ogreViewer_p = NULL;
	}	
}



int mcuCBHandle::add_scene( SrSnGroup *scene_p )	{

	if( root_group_p )	{
		if( scene_p )	{
			root_group_p->add( scene_p ); 
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

int mcuCBHandle::remove_scene( SrSnGroup *scene_p )	{

	if( root_group_p )	{
		if( scene_p )	{
			root_group_p->remove( scene_p ); 
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}
void mcuCBHandle::update( void )	
{
	// remote mode
	if (SmartBody::SBScene::getScene()->isRemoteMode())
	{
		SmartBody::SBScene::getScene()->getDebuggerClient()->Update();
		std::map<std::string, SbmPawn*>::iterator iter;
		for (iter = getPawnMap().begin();
			iter != getPawnMap().end();
			iter++)
		{
			SbmPawn* pawn = (*iter).second;
			pawn->ct_tree_p->evaluate(time);
			pawn->ct_tree_p->applySkeletonToBuffer();
		}

		return;
	}

	// scripts
	std::map<std::string, SmartBody::SBScript*>& scripts = SmartBody::SBScene::getScene()->getScripts();
	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->beforeUpdate(this->time);
	}

	// services
	std::map<std::string, SmartBody::SBService*>& services = SmartBody::SBScene::getScene()->getServiceManager()->getServices();
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->beforeUpdate(this->time);
	}

// 	if (physicsEngine && physicsEngine->getBoolAttribute("enable"))
// 	{		
// 		float dt = (float)physicsEngine->getDoubleAttribute("dT");//timeStep*0.03f;
// 		//elapseTime += time_dt;
// 		while (physicsTime < this->time)		
// 		//if (physicsTime < this->time)
// 		{
// 			//printf("elapse time = %f\n",elapseTime);
// 			physicsEngine->updateSimulation(dt);
// 			physicsTime += dt;
// 			//curDt -= dt;
// 		}		
// 	}
// 	else
// 	{
// 		physicsTime = this->time;
// 	}

	std::string seqName = "";
	std::vector<std::string> sequencesToDelete;
	for (int s = 0; s < activeSequences.getNumSequences(); s++)
	{
		srCmdSeq* seq = activeSequences.getSequence(s, seqName);
		char *cmd;
		while( cmd = seq->pop( (float)time ) )
		{
			int err = execute( cmd );
			if( err != CMD_SUCCESS )	{
				LOG( "mcuCBHandle::update ERR: execute FAILED: '%s'\n", cmd );
			}
			else
			{
				// we assume that every command execution could potentially change the resource data
				// therefore the data changed flag is set.
				// This is kind of rough, but should work fine.
				resourceDataChanged = true; 
			}
			delete [] cmd;
		}
		if( seq->get_count() < 1 )
		{
			sequencesToDelete.push_back(seqName);
		}
	}

	for (size_t d = 0; d < sequencesToDelete.size(); d++)
	{
		activeSequences.removeSequence(sequencesToDelete[d], true);
	}

	bool isClosingBoneBus = false;
    std::map<std::string, SbmPawn*>::iterator iter;
	for (iter = getPawnMap().begin();
		iter != getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		pawn->reset_all_channels();
		pawn->ct_tree_p->evaluate( time );
		pawn->ct_tree_p->applyBufferToAllSkeletons();

// 		if (pawn->hasPhysicsSim() && SBPhysicsSim::getPhysicsEngine()->getBoolAttribute("enable"))
// 		{
// 			//pawn->updateFromColObject();
// 		}
// 		else
		{			
			//pawn->updateToColObject();
			pawn->updateToSteeringSpaceObject();
		}
		SbmCharacter* char_p = getCharacter(pawn->getName().c_str() );
		if (!char_p)
		{
			if (SmartBody::SBScene::getScene()->getBoneBusManager()->isEnable())
			{
				if (!isClosingBoneBus && !pawn->bonebusCharacter && SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().IsOpen() && sendPawnUpdates)
				{
					// bonebus was connected after character creation, create it now
					pawn->bonebusCharacter = SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().CreateCharacter( pawn->getName().c_str(), pawn->getClassType().c_str() , false );
				}
				if (sendPawnUpdates)
					NetworkSendSkeleton( pawn->bonebusCharacter, pawn->getSkeleton(), &param_map );
				if (pawn->bonebusCharacter && pawn->bonebusCharacter->GetNumErrors() > 3)
				{
					// connection is bad, remove the bonebus character 
					LOG("BoneBus cannot connect to server. Removing pawn %s", pawn->getName().c_str());
					bool success = SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().DeleteCharacter(pawn->bonebusCharacter);
					char_p->bonebusCharacter = NULL;
					isClosingBoneBus = true;
					if (SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().GetNumCharacters() == 0)
					{
						SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().CloseConnection();
					}
				}
			}
		}
		if( char_p ) {

			// run the minibrain, if available
			SmartBody::MiniBrain* brain = char_p->getMiniBrain();
			if (brain)
			{
				SmartBody::SBCharacter* sbchar = dynamic_cast<SmartBody::SBCharacter*>(char_p);
				brain->update(sbchar, time, time_dt);
			}

			// scene update moved to renderer
			//if (char_p->scene_p)
			//	char_p->scene_p->update();
			//char_p->dMesh_p->update();
			//char_p->updateJointPhyObjs();
			/*
			bool hasPhySim = physicsEngine->getBoolAttribute("enable");
			char_p->updateJointPhyObjs(hasPhySim);
			//char_p->updateJointPhyObjs(false);
			*/
			char_p->_skeleton->update_global_matrices();

			char_p->forward_visemes( time );	
			char_p->forward_parameters( time );	

			if (char_p->bonebusCharacter && char_p->bonebusCharacter->GetNumErrors() > 3)
			{
				// connection is bad, remove the bonebus character
				isClosingBoneBus = true;
				LOG("BoneBus cannot connect to server after visemes sent. Removing all characters.");
			}

			

			if ( SmartBody::SBScene::getScene()->getBoneBusManager()->isEnable() && char_p->getSkeleton() && char_p->bonebusCharacter ) {
				NetworkSendSkeleton( char_p->bonebusCharacter, (SkSkeleton *)(char_p->getSkeleton()), &param_map );

				if ( net_world_offset_updates ) {

					const SkJoint * joint = char_p->get_world_offset_joint();

					const SkJointPos * pos = joint->const_pos();
					float x = pos->value( SkJointPos::X );
					float y = pos->value( SkJointPos::Y );
					float z = pos->value( SkJointPos::Z );

					SkJoint::RotType rot_type = joint->rot_type();
					if ( rot_type != SkJoint::TypeQuat ) {
						//strstr << "ERROR: Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")"<<endl;
					}

					// const_cast because the SrQuat does validation (no const version of value())
					const SrQuat & q = ((SkJoint *)joint)->quat()->value();

					char_p->bonebusCharacter->SetPosition( x, y, z, time );
					char_p->bonebusCharacter->SetRotation( (float)q.w, (float)q.x, (float)q.y, (float)q.z, time );

					if (char_p->bonebusCharacter->GetNumErrors() > 3)
					{
						// connection is bad, remove the bonebus character 
						isClosingBoneBus = true;
						LOG("BoneBus cannot connect to server. Removing all characters");
					}
				}
			}
			else if (!isClosingBoneBus && !char_p->bonebusCharacter && SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().IsOpen())
			{
				// bonebus was connected after character creation, create it now
				char_p->bonebusCharacter = SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().CreateCharacter( char_p->getName().c_str(), char_p->getClassType().c_str(), true );
			}
		}  // end of char_p processing
	} // end of loop

	if (isClosingBoneBus)
	{
		for (std::map<std::string, SbmPawn*>::iterator iter = getPawnMap().begin();
			iter != getPawnMap().end();
			iter++)
		{
			SbmPawn* pawn = (*iter).second;
			if (pawn->bonebusCharacter)
			{
				bool success = SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().DeleteCharacter(pawn->bonebusCharacter);
				pawn->bonebusCharacter = NULL;
			}
		}

		SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().CloseConnection();
	}

	for (std::map<std::string, SbmPawn*>::iterator iter = getPawnMap().begin();
			iter != getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		pawn->afterUpdate(time);
	}

	
	if (viewer_p && viewer_p->get_camera())
	{
		SrMat m;
		SrQuat quat = SrQuat(viewer_p->get_camera()->get_view_mat(m).get_rotation());

		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.x = viewer_p->get_camera()->getEye().x;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.y = viewer_p->get_camera()->getEye().y;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.z = viewer_p->get_camera()->getEye().z;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraLookAt.x = viewer_p->get_camera()->getCenter().x;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraLookAt.y = viewer_p->get_camera()->getCenter().y;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraLookAt.z = viewer_p->get_camera()->getCenter().z;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.x = quat.x;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.y = quat.y;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.z = quat.z;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.w = quat.w;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraFovY   = sr_todeg(viewer_p->get_camera()->getFov());
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraAspect = viewer_p->get_camera()->getAspectRatio();
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZNear  = viewer_p->get_camera()->getNearPlane();
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZFar   = viewer_p->get_camera()->getFarPlane();
	}
	
	/*
	else
	{
		SrCamera defaultCam;
		SrMat m;
		SrQuat quat = SrQuat(defaultCam.get_view_mat(m).get_rotation());

		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.x = defaultCam.eye.x;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.y = defaultCam.eye.y;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.z = defaultCam.eye.z;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.x = quat.x;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.y = quat.y;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.z = quat.z;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.w = quat.w;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraFovY   = sr_todeg(defaultCam.fovy);
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraAspect = defaultCam.aspect;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZNear  = defaultCam.znear;
		SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZFar   = defaultCam.zfar;
	}
	*/

	if (!SmartBody::SBScene::getScene()->isRemoteMode())
		SmartBody::SBScene::getScene()->getDebuggerServer()->Update();

	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		(*iter).second->update(time);
	}

	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		(*iter).second->update(time);
	}

	// scripts
	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->afterUpdate(this->time);
	}

	// services
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->afterUpdate(this->time);
	}
}

srCmdSeq* mcuCBHandle::lookup_seq( const char* name ) {
	int err = CMD_FAILURE;
	
	// Remove previous activation of sequence.
	// Anm: Why?  Need clear distrinction (and definition) between pending and active.
	abortSequence( name );

	srCmdSeq* seq = pendingSequences.getSequence( name );
	if (seq)
	{
		pendingSequences.removeSequence( name, false );
	}
	else
	{
		// Sequence not found.  Load new instance from file.
		std::string fullPath;
		FILE* file = open_sequence_file( name, fullPath );
		if( file ) {
			seq = new srCmdSeq();
			err = seq->read_file( file );
			fclose( file );

			if( err != CMD_SUCCESS ) {
				LOG("ERROR: mcuCBHandle::lookup_seq(..): '%s' PARSE FAILED\n", name ); 

				delete seq;
				seq = NULL;
			}
		} else {
			LOG("ERROR: mcuCBHandle::lookup_seq(..): '%s' NOT FOUND\n", name ); 
		}
	}
	
	return( seq );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq ) {
	ostringstream seq_id;
	seq_id << "execute_seq-" << (++queued_cmds);

	return execute_seq( seq, seq_id.str().c_str() );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq_p, const char* seq_id ) {

//	printf( "mcuCBHandle::execute_seq: id: '%s'\n", seq_id );
//	seq_p->print();

	if ( !activeSequences.addSequence( seq_id, seq_p ) ) {
		LOG("ERROR: mcuCBHandle::execute_seq(..): Failed to insert srCmdSeq \"%s\"into active_seq_map.", seq_id );
		return CMD_FAILURE;
	}	
	resourceDataChanged = true;
	return CMD_SUCCESS;
}

int mcuCBHandle::execute_seq_chain( const vector<string>& seq_names, const char* error_prefix ) {
	vector<string>::const_iterator it  = seq_names.begin();
	vector<string>::const_iterator end = seq_names.end();

	if( it == end ) {
		// No sequences -> NOOP
		return CMD_SUCCESS;
	}

	const string& first_seq_name = *it;  // convenience reference
	std::string fullPath;
	FILE* first_file_p = open_sequence_file( first_seq_name.c_str(), fullPath );
	if( first_file_p == NULL ) {
		if( error_prefix )
			LOG("%s Cannot find sequence \"%s\". Aborting seq-chain.", error_prefix, first_seq_name.c_str());
		return CMD_FAILURE;
	}

	srCmdSeq* seq_p = new srCmdSeq();
	int parse_result = seq_p->read_file( first_file_p );
	fclose( first_file_p );
	if( parse_result != CMD_SUCCESS ) {
		if( error_prefix )
			LOG("%s Unable to parse sequence\"%s\".", error_prefix, first_seq_name.c_str());

		delete seq_p;
		seq_p = NULL;

		return CMD_FAILURE;
	}

	// Test remaining sequence names, error early if invalid
	vector<string>::const_iterator second = ++it;
	for( ; it != end; ++it ) {
		const string& next_seq = *it;  // convenience reference

		std::string fullPath;
		FILE* file = open_sequence_file( next_seq.c_str(), fullPath );
		if( file == NULL ) {
			if( error_prefix )
				LOG("%s Cannot find sequence \"%s\". Aborting seq-chain.", error_prefix, next_seq.c_str() );
			return CMD_FAILURE;
		} else {
			fclose( file );
		}
	}

	if( second != end ) {  // has more than one seq_name
		// Append new seq-chian command of remaining seq_names at end of seq_p
		float time = seq_p->duration();

		// Start from second
		it = second;

		// build command
		ostringstream oss;
		oss << "seq-chain";
		for( ; it != end; ++it )
			oss << ' ' << (*it);

		// insert command or error with cleanup
		int result = seq_p->insert( time, oss.str().c_str() );
		if( result != CMD_SUCCESS ) {
			if( error_prefix )
				LOG("%s Failed to insert seq-chain command at time %f", error_prefix, time);

			delete seq_p;
			seq_p = NULL;

			return CMD_FAILURE;
		}
	}

	execute_seq( seq_p, first_seq_name.c_str() );

	return CMD_SUCCESS;
}

int mcuCBHandle::execute_later( const char* command, float seconds ) {
	srCmdSeq *temp_seq = new srCmdSeq();
	temp_seq->insert( (float)time+seconds, command );

	ostringstream seqName;
	seqName << "execute_later-" << (++queued_cmds);

	return execute_seq( temp_seq, seqName.str().c_str() );;
}

int mcuCBHandle::abortSequence( const char* name ) {
	srCmdSeq* seq = activeSequences.getSequence(name);
	if( !seq )
		return CMD_FAILURE;
	
	bool success = activeSequences.removeSequence( name, true );
	

	srCmdSeq* pending = pendingSequences.getSequence( name );
	if( pending )
		success = activeSequences.removeSequence( name, true );

	return CMD_SUCCESS; 
}

int mcuCBHandle::deleteSequence( const char* name ) {
	int result = abortSequence( name );

	srCmdSeq* seq = pendingSequences.getSequence( name );
	if( seq  )
	{
		pendingSequences.removeSequence( name, true );
		result = CMD_SUCCESS;
	}

	return result;
}

void mcuCBHandle::set_net_host( const char * net_host )
{
	// EDF
	// Sets up the network connection for sending bone rotations over to Unreal
	SmartBody::SBScene::getScene()->getBoneBusManager()->setHost(net_host);
	SmartBody::SBScene::getScene()->getBoneBusManager()->setEnable(true);
	SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().UpdateAllCharacters();
}

void mcuCBHandle::set_process_id( const char * process_id )
{
	this->process_id = process_id;
}

int mcuCBHandle::vhmsg_send( const char *op, const char* message ) {
#if LINK_VHMSG_CLIENT
	//std::cout<<"Sending :" << cmdName << ' ' << cmdArgs <<std::endl;
	//LOG("vhmsg_send, op = %s, message = %s",op,message);

	if( vhmsg_enabled ) {
		int err = vhmsg::ttu_notify2( op, message );
		if( err != vhmsg::TTU_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify2 failed on message \"" << op << "  " << message << "\"." << std::endl;
			LOG(strstr.str().c_str());
		}
	} else {
		// append to command queue if header token has callback function
		srArgBuffer tokenizer( message );
		char* token = tokenizer.read_token();
		if( cmd_map.is_command( op ) ) {
			// Append to command queue
			ostringstream command;
			command << op << " " << message;
			execute_later( command.str().c_str() );
		}
	}
#else
	// append to command queue if header token has callback function
	srArgBuffer tokenizer( message );
	char* token = tokenizer.read_token();
	if( cmd_map.is_command( op ) ) {
		// Append to command queue
		ostringstream command;
		command << op << " " << message;
		execute_later( command.str().c_str() );
	}
#endif
	return( CMD_SUCCESS );
}

int mcuCBHandle::vhmsg_send( const char* message ) {
#if LINK_VHMSG_CLIENT
	//std::cout<<"Sending :" << cmdName << ' ' << cmdArgs <<std::endl;

	if( vhmsg_enabled ) {

		int err = vhmsg::ttu_notify1( message );
		if( err != vhmsg::TTU_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify1 failed on message \"" << message << "\"." << std::endl;
			LOG(strstr.str().c_str());
		}
	} else {
		// append to command queue if header token has callback function
		srArgBuffer tokenizer( message );
		char* token = tokenizer.read_token();
		if( cmd_map.is_command( token ) ) {
			// Append to command queue
			execute_later( message );
		}
	}
#else
	// append to command queue if header token has callback function
	srArgBuffer tokenizer( message );
	char* token = tokenizer.read_token();
	if( cmd_map.is_command( token ) ) {
		// Append to command queue
		execute_later( message );
	}
#endif
	return( CMD_SUCCESS );
}


int mcuCBHandle::load_skeletons( const char* pathname, bool recursive ) {
	return load_me_skeletons( pathname, skeleton_map, recursive, skScale );
}

int mcuCBHandle::load_skeleton( const void* data, int sizeBytes, const char* skeletonName )
{
	SrInput input( (char *)data, sizeBytes );
	return load_me_skeleton_individual( input, skeletonName, skeleton_map, skScale );
}

int mcuCBHandle::map_skeleton( const char * mapName, const char * skeletonName )
{
	// ED - taken from skeletonmap_func()

	SmartBody::SBSkeleton* sbskeleton = NULL;
	std::map<std::string, SmartBody::SBSkeleton*>::iterator iter = skeleton_map.find(skeletonName);
	if (iter == skeleton_map.end())
	{
		LOG("Cannot find skeleton named %s.", skeletonName);
		return CMD_FAILURE;
	}
	else
	{
		sbskeleton = dynamic_cast<SmartBody::SBSkeleton*>((*iter).second);
	}

	// find the bone map name
	SmartBody::SBJointMap* jointMap = SmartBody::SBScene::getScene()->getJointMapManager()->getJointMap(mapName);
	if (!jointMap)
	{
		LOG("Cannot find joint map name '%s'.", mapName);
		return CMD_FAILURE;
	}

	// apply the map
	jointMap->applySkeleton(sbskeleton);

	LOG("Applied joint map %s to skeleton %s.", mapName, skeletonName);

	return CMD_SUCCESS;
}

int mcuCBHandle::map_motion( const char * mapName, const char * motionName )
{
	// taken from motionmap_func()

	SmartBody::SBMotion* sbmotion = SmartBody::SBScene::getScene()->getAssetManager()->getMotion(motionName);
	if (!sbmotion)
	{
		LOG("Cannot find motion name %s.", motionName);
		return CMD_FAILURE;
	}
	
	// find the bone map name
	SmartBody::SBJointMap* jointMap = SmartBody::SBScene::getScene()->getJointMapManager()->getJointMap(mapName);
	if (!jointMap)
	{
		LOG("Cannot find bone map name '%s'.", mapName);
		return CMD_FAILURE;
	}

	// apply the map
	jointMap->applyMotion(sbmotion);

	LOG("Applied bone map %s to motion %s.", mapName, motionName);

	return CMD_SUCCESS;
}

void mcuCBHandle::NetworkSendSkeleton( bonebus::BoneBusCharacter * character, SkSkeleton * skeleton, GeneralParamMap * param_map )
{
	if ( character == NULL )
	{
		return;
	}


	// Send the bone rotation for each joint in the skeleton
	const std::vector<SkJoint *> & joints  = skeleton->joints();

	character->IncrementTime();
	character->StartSendBoneRotations();

	std::vector<int> otherJoints;

	for ( size_t i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeJoint)
		{
			if (j->getJointType() == SkJoint::TypeOther)
				otherJoints.push_back(i); // collect the 'other' joins
			continue;
		}

		const SrQuat& q = j->quat()->value();

		character->AddBoneRotation( j->extName().c_str(), q.w, q.x, q.y, q.z, time );

		//printf( "%s %f %f %f %f\n", (const char *)j->name(), q.w, q.x, q.y, q.z );
	}

	character->EndSendBoneRotations();


	character->StartSendBonePositions();

	for ( size_t i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeJoint)
			continue;

		float posx = j->pos()->value( 0 );
		float posy = j->pos()->value( 1 );
		float posz = j->pos()->value( 2 );
		if ( false )
		{
			posx += j->offset().x;
			posy += j->offset().y;
			posz += j->offset().z;
		}

		//these coordinates are meant to mimic the setpositionbyname coordinates you give to move the character
		//so if you wanted to move a joint on the face in the x direction you'd do whatever you did to move the actor
		//itself further in the x position.
		character->AddBonePosition( j->extName().c_str(), posx, posy, posz, time );
	}

	character->EndSendBonePositions();

	if (otherJoints.size() > 0)
	{
		character->StartSendGeneralParameters();
		for (size_t i = 0; i < otherJoints.size(); i++)
		{
			SkJoint* joint = joints[otherJoints[i]];
			character->AddGeneralParameters(i, 1, joint->pos()->value( 0 ), i, time);

		}
		character->EndSendGeneralParameters();
	}
	

/*
	// Passing General Parameters
	character->StartSendGeneralParameters();
<<<<<<< .mine
	for (size_t i = 0; i < joints.size(); i++)
=======
	int numFound = 0;
	for (int i = 0; i < joints.size(); i++)
>>>>>>> .r2317
	{
		SkJoint* j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeOther)
			continue;

		// judge whether it is joint for general parameters, usually should have a prefix as "param"
		string j_name = j->name();
		int name_end_pos = j_name.find_first_of("_");
		string test_prefix = j_name.substr( 0, name_end_pos );
		if( test_prefix == character->m_name )	
		{
			// if is, prepare adding data
			int index = 0;
			GeneralParamMap::iterator pos;
			for(pos = param_map->begin(); pos != param_map->end(); pos++)
			{
				for(int n = 0; n < (int)pos->second->char_names.size(); n++)
				{
					if( character->m_name == pos->second->char_names[n] )
					{
						index ++;
						for(int m = 0 ; m < pos->second->size; m++)
						{
							std::stringstream joint_name;
							joint_name << character->m_name << "_" << index << "_" << ( m + 1 );
							if(_stricmp( j->name().c_str(), joint_name.str().c_str()) == 0)
								character->AddGeneralParameters(index, pos->second->size, j->pos()->value(0), m, time);
						}
					}
				}
			}
		}
	}
	character->EndSendGeneralParameters();
*/
	
}

int mcuCBHandle::executePythonFile(const char* filename)
{
#ifndef SB_NO_PYTHON
	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];	
	// add the .seq extension if necessary
	std::string candidateSeqName = filename;
	if (candidateSeqName.find(".py") == std::string::npos)
	{
		candidateSeqName.append(".py");
	}
	sprintf( label, "%s", candidateSeqName.c_str());
	// current path containing .exe
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);

	seq_paths.reset();
	std::string curFilename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
	while (curFilename.size() > 0)
	{
		try{
			FILE* file = fopen(curFilename.c_str(), "r");
			if (file)
			{
				fclose(file);
				std::stringstream strstr;
				strstr << "execfile(\"" << curFilename << "\")";
				PyRun_SimpleString(strstr.str().c_str());
				PyErr_Print();
				PyErr_Clear();
				return CMD_SUCCESS;
			}
			else
			{
				curFilename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
			}
		} catch (...) {
			PyErr_Print();
			return CMD_FAILURE;
		}
	}

	LOG("Could not find Python script '%s'", filename);
	return CMD_FAILURE;

#endif
	return CMD_FAILURE;
}

int mcuCBHandle::executePython(const char* command)
{
#ifndef SB_NO_PYTHON
	try {
		//LOG("executePython = %s",command);

		int result = PyRun_SimpleString(command);
		//LOG("cmd result = %d",result);

		return CMD_SUCCESS;
	} catch (...) {
		PyErr_Print();
	}
#endif
	return CMD_FAILURE;
}


// void mcuCBHandle::setPhysicsEngine( bool start )
// {
// 	if (start)
// 	{
// 		physicsTime = time;
// 		//updatePhysics = true;		
// 	}
// 	else
// 	{
// 		//updatePhysics = false;
// 	}
// 
// 	if (physicsEngine)
// 	{
// 		physicsEngine->setEnable(start);
// 	}
// }


std::map<std::string, SbmPawn*>& mcuCBHandle::getPawnMap()
{
	return pawn_map;
}

bool mcuCBHandle::addPawn(SbmPawn* pawn)
{
	SbmPawn* p = getPawn(pawn->getName());
	if (!p)
	{
		pawn_map[pawn->getName()] = pawn;
		return true;
	}
	else
	{
		return false;
	}

}

void mcuCBHandle::removePawn(const std::string& name)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(name);
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}
}

SbmPawn* mcuCBHandle::getPawn(const std::string& name)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(name);
	if (iter == pawn_map.end())
		return NULL;
	else
		return (*iter).second;
}

int mcuCBHandle::getNumPawns()
{
	return pawn_map.size();
}

std::map<std::string, SbmCharacter*>& mcuCBHandle::getCharacterMap()
{
	return character_map;
}

std::map<std::string, SmartBody::SBSkeleton*>& mcuCBHandle::getSkeletonMap()
{
	return skeleton_map;
}

std::map<std::string, DeformableMesh*>& mcuCBHandle::getDeformableMeshMap()
{
	return deformableMeshMap;
}

SbmCharacter* mcuCBHandle::getCharacter(const std::string& name)
{
	std::map<std::string, SbmCharacter*>::iterator iter = character_map.find(name);
	if (iter == character_map.end())
		return NULL;
	else
		return (*iter).second;
}

int mcuCBHandle::getNumCharacters()
{
	return character_map.size();
}

DeformableMesh* mcuCBHandle::getDeformableMesh( const std::string& name )
{
	std::map<std::string, DeformableMesh*>::iterator iter = deformableMeshMap.find(name);
	if (iter == deformableMeshMap.end())
		return NULL;
	else
		return (*iter).second;
}

std::string mcuCBHandle::getValidName(const std::string& name)
{
	bool nameFound = true;
	int nameCounter = 0;
	std::string currentName = name;
	while (nameFound)
	{
		std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(currentName);
		if (iter == pawn_map.end())
		{
			nameFound = false;
		}
		else
		{
			std::stringstream strstr;
			strstr << name << nameCounter;
			nameCounter++;
			currentName = strstr.str();
		}
	}
	return currentName;
}

int mcuCBHandle::registerPawn(SbmPawn* pawn)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(pawn->getName());
	if (iter != pawn_map.end())
	{
		LOG( "Register pawn: pawn_map.insert(..) '%s' FAILED\n", pawn->getName().c_str() ); 
		return( CMD_FAILURE );
	}

	pawn_map.insert(std::pair<std::string, SbmPawn*>(pawn->getName(), pawn));
	
	if (scene->getCharacterListener())
		scene->getCharacterListener()->OnPawnCreate( pawn->getName().c_str() );

	return CMD_SUCCESS;
}

int mcuCBHandle::unregisterPawn(SbmPawn* pawn)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if (scene->getCharacterListener())
		scene->getCharacterListener()->OnPawnDelete( pawn->getName().c_str() );

	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(pawn->getName());
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}
	return CMD_SUCCESS;
}

int mcuCBHandle::registerCharacter(SbmCharacter* character)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(character->getName());
	if (iter != pawn_map.end())
	{
		LOG( "Register character: pawn_map.insert(..) '%s' FAILED\n", character->getName().c_str() ); 
		return( CMD_FAILURE );
	}

	pawn_map.insert(std::pair<std::string, SbmPawn*>(character->getName(), character));
	

	std::map<std::string, SbmCharacter*>::iterator citer = character_map.find(character->getName());
	if (citer != character_map.end())
	{
		LOG( "Register character: character_map.insert(..) '%s' FAILED\n", character->getName().c_str() );
		pawn_map.erase(iter);
		return( CMD_FAILURE );
	}
	character_map.insert(std::pair<std::string, SbmCharacter*>(character->getName(), character));

	if (SmartBody::SBScene::getScene()->getBoneBusManager()->isEnable())
		SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().CreateCharacter( character->getName().c_str(), character->getClassType().c_str(), true );
	if ( SmartBody::SBScene::getScene()->getCharacterListener() )
		SmartBody::SBScene::getScene()->getCharacterListener()->OnCharacterCreate( character->getName().c_str(), character->getClassType() );

	return 1;
}

int mcuCBHandle::unregisterCharacter(SbmCharacter* character)
{
	if (SmartBody::SBScene::getScene()->getCharacterListener() )
		SmartBody::SBScene::getScene()->getCharacterListener()->OnCharacterDelete( character->getName().c_str() );

	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(character->getName());
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}

	std::map<std::string, SbmCharacter*>::iterator citer = character_map.find(character->getName());
	if (citer != character_map.end())
	{
		character_map.erase(citer);
	}

	return 1;
}


void mcuCBHandle::render()
{
	if( viewer_p ) { viewer_p->render(); }
	if (ogreViewer_p) { ogreViewer_p->render(); }
}


void mcuCBHandle::render_terrain( int renderMode ) {
			if( height_field_p )	{
				height_field_p->render(renderMode);
			}
		}

float mcuCBHandle::query_terrain( float x, float z, float *normal_p )	{
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

/////////////////////////////////////////////////////////////
