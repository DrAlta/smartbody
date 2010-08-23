/*
 *  sbm_main.cpp - part of SBM: SmartBody Module
 *  Copyright (C) 2008  University of Southern California
 *
 *  SBM is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SBM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SBM.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Marcus Thiebaux, USC
 *      Andrew n marshall, USC
 *      Ed Fast, USC
 *      Ashok Basawapatna, USC (no longer)
 *      Eric Forbell, USC
 *      Thomas Amundsen, USC
 */


#define SBM_REPORT_MEMORY_LEAKS  0
#define SBM_EMAIL_CRASH_REPORTS  1

#include <signal.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

#include "fltk_viewer.h"
#include <BehaviorWindow.h>
#include "wsp.h"

#include <sbm/sbm_constants.h>
#include <sbm/xercesc_utils.hpp>
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include BML_PROCESSOR_INCLUDE
#include <sbm/remote_speech.h>
#include <sbm/joint_logger.hpp>
#include <sbm/sbm_audio.h>
#include <sbm/sbm_speech_audiofile.hpp>
#include <sbm/text_speech.h> // [BMLR]
#include <sbm/locomotion_cmds.hpp>
#include <sbm/resource_cmds.h>
#include <sbm/time_regulator.h>
#include <vhcl_log.h>
//#include "SBMWindow.h"
#include "CommandWindow.h"

//#include "tip.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#include <me/me_spline_1d.hpp>
void test_1Dspline( void )	{
	MeSpline1D S;

//	Knot* make_smooth( domain x, range y, range tan, range l_control, range r_control );
//	Knot* make_cusp( domain x, range y, range l_tan, range l_control, range r_tan, range r_control );
//	Knot* make_disjoint( domain x, range y, range left_y, range l_tan, range l_control, range r_tan, range r_control );
// range eval( domain x )

#if 0
	S.make_disjoint( 0.0, 0.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_disjoint( 1.0, 1.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_disjoint( 1.005, 2.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_disjoint( 2.0, 4.0,  0.0, 0.0, 0.0, 0.0 );
#elif 1
	S.make_cusp( 0.0, 0.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_cusp( 1.0, 1.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_cusp( 1.0, 2.0,  0.0, 0.0, 0.0, 0.0 );
	S.make_cusp( 2.0, 4.0,  0.0, 0.0, 0.0, 0.0 );
#else
	S.make_smooth( 0.0, 0.0,  0.0, 0.0, 0.0 );
	S.make_smooth( 1.0, 1.0,  0.0, 0.0, 0.0 );
	S.make_smooth( 2.0, 4.0,  0.0, 0.0, 0.0 );
#endif

//	S.update_all_knots();
	for( int i=0; i<=100; i++ )	{
		double n = (double)i/100.0;
		double x = -1.0 + n * 6.0;

		double y = S.eval( x );
		LOG( " [%d]: { %.12f, %.12f }\n", i, x, y );
	}

	LOG( " : { %.12f, %.12f }\n", -0.000001, S.eval( -0.000001 ) );
	LOG( " : { %.12f, %.12f }\n", 3.9999999, S.eval( 3.9999999 ) );
	LOG( " : { %.12f, %.12f }\n", 4.0000001, S.eval( 4.0000001 ) );
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <sbm/sr_cmd_line.h>

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX        /* Don't defined min() and max() */
#endif
#include <windows.h>
#include <mmsystem.h>
#if SBM_REPORT_MEMORY_LEAKS
#include <malloc.h>
#include <crtdbg.h>
#endif
#endif

#if SBM_EMAIL_CRASH_REPORTS
#include <vhcl_crash.h>
#endif

#include <fltk/glut.h>
#include "pic.h"
#include "sr/sr_model.h"

#define ENABLE_DEFAULT_BOOTSTRAP	(1)
#define DEFAULT_SEQUENCE_FILE		("default.seq")

///////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32_LEAN_AND_MEAN
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#else
#endif

using std::vector;
using std::string;


///////////////////////////////////////////////////////////////////////////////////

int sbm_main_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{

	const char* token = args.read_token();
	if( strcmp(token,"id")==0 ) {  // Process specific
		token = args.read_token(); // Process id
		const char* process_id = mcu_p->process_id.c_str();
		if( ( mcu_p->process_id == "" )        // If process id unassigned
			|| strcmp( token, process_id )!=0 ) // or doesn't match
			return CMD_SUCCESS;                 // Ignore.
		token = args.read_token(); // Sub-command
	}

	const char* args_raw = args.read_remainder_raw();
	int result = mcu_p->execute( token, srArgBuffer( args_raw ) );
	switch( result ) {
		case CMD_NOT_FOUND:
			LOG("SBM ERR: command NOT FOUND: '%s %s' ", token, args_raw );
			break;
		case CMD_FAILURE:
			LOG("SBM ERR: command FAILED: '%s %s' ", token, args_raw );
			break;
		case CMD_SUCCESS:
			break;
	}
	return CMD_SUCCESS;
}

void sbm_vhmsg_callback( const char *op, const char *args, void * user_data ) {
	// Replace singleton with a user_data pointer
	switch( mcuCBHandle::singleton().execute( op, (char *)args ) ) {
        case CMD_NOT_FOUND:
            LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
            break;
        case CMD_FAILURE:
            LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
            break;
    }
}

int sbm_vhmsg_register_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	
	return( CMD_SUCCESS );
}

int sbm_vhmsg_send_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	char* cmdName = args.read_token();
	char* cmdArgs = args.read_remainder_raw();
	return mcu_p->vhmsg_send( cmdName, cmdArgs );
}

// snapshot <windowHeight> <windowWidth> <offsetHeight> <offsetWidth> <output file>
// The offset is according to the left bottom corner of the image frame buffer
int mcu_snapshot_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )
	{
		int windowHeight = args.read_int();
		int windowWidth = args.read_int();
		int offsetHeight = args.read_int();
		int offsetWidth = args.read_int();

		string output_file = args.read_token();

		if( windowHeight == 0 )		windowHeight = 600;							// default window size
		if( windowWidth == 0 )		windowWidth = 400;
		if( output_file == "" )		
		{
			std::stringstream output_file_os;
			output_file_os<< "snapshot_"<< mcu_p->snapshot_counter<< ".ppm";	// default output name
			mcu_p->snapshot_counter++;
			output_file = output_file_os.str();
		}
		// Allocate a picture buffer 
		Pic * in = pic_alloc(windowWidth, windowHeight, 3, NULL);
		LOG("  File to save to: %s", output_file.c_str());

		for (int i=windowHeight-1; i>=0; i--) 
		{
			glReadPixels(0 + offsetWidth, windowHeight+offsetHeight-i-1, windowWidth, 1 , GL_RGB, GL_UNSIGNED_BYTE, &in->pix[i*in->nx*in->bpp]);
		}

		if (ppm_write(output_file.c_str(), in))
		{
			pic_free(in);
			LOG("  File saved Successfully\n");
			return( CMD_SUCCESS );
		}
		else
		{
			pic_free(in);
			LOG("  Error in Saving\n");
			return( CMD_FAILURE );
		}	
	}
	return( CMD_SUCCESS );
}

int mcu_echo_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	
    LOG("%s ", args.read_remainder_raw() );
	return( CMD_SUCCESS );
}

int mcu_reset_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	// TODO: If arg, call as init, else call previous init
	mcu_p->reset();
	return( CMD_SUCCESS );
}

int mcu_quit_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{

	mcu_p->loop = false;
	ResourceManager::cleanup();
	return( CMD_SUCCESS );
}

///////////////////////////////////////////////////////////////////////////////////

SrSnGroup *build_checkerboard_floor( float width )	{

	SrSnGroup *floor_group_p = new SrSnGroup;
	SrSnBox* floor_bg_p = new SrSnBox;
	SrSnBox* tile_1_p = new SrSnBox;
	SrSnBox* tile_2_p = new SrSnBox;
	
	float half = width * 0.5f;
	floor_bg_p->color( SrColor( 220, 220, 220 ) );
	floor_bg_p->shape().a.set( -half, -1.0f, -half );
	floor_bg_p->shape().b.set(  half, -0.1f, half );
	
	tile_1_p->color( SrColor( 100, 100, 100 ) );
	tile_1_p->shape().a.set( -half, -0.5f, -half );
	tile_1_p->shape().b.set(  0.0, 0.0, 0.0 );
	
	tile_2_p->color( SrColor( 100, 100, 100 ) );
	tile_2_p->shape().a.set(  0.0, -0.5f, 0.0 );
	tile_2_p->shape().b.set(  half, 0.0, half );
	
	floor_group_p->add( floor_bg_p );
	floor_group_p->add( tile_1_p );
	floor_group_p->add( tile_2_p );
	return( floor_group_p );
}

void mcu_register_callbacks( void ) {
	
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	mcu.insert( "sbm",			sbm_main_func );
	mcu.insert( "help",			mcu_help_func );

	mcu.insert( "q",			mcu_quit_func );
	mcu.insert( "quit",			mcu_quit_func );
	mcu.insert( "reset",		mcu_reset_func );
	mcu.insert( "echo",			mcu_echo_func );
	
	mcu.insert( "path",			mcu_filepath_func );
	mcu.insert( "seq",			mcu_sequence_func );
	mcu.insert( "seq-chain",	mcu_sequence_chain_func );
	mcu.insert( "send",			sbm_vhmsg_send_func );

	mcu.insert( "snapshot",		mcu_snapshot_func );

	//  cmd prefixes "set" and "print"
	mcu.insert( "set",          mcu_set_func );
	mcu.insert( "print",        mcu_print_func );
	mcu.insert( "test",			mcu_test_func );

	mcu.insert( "viewer",		mcu_viewer_func );
	mcu.insert( "bmlviewer",    mcu_bmlviewer_func);
	mcu.insert( "camera",		mcu_camera_func );
	mcu.insert( "time",			mcu_time_func );
	mcu.insert( "tip",			mcu_time_ival_prof_func );

	mcu.insert( "load",			mcu_load_func );
	mcu.insert( "pawn",			SbmPawn::pawn_cmd_func );
	mcu.insert( "char",			SbmCharacter::character_cmd_func );

	mcu.insert( "ctrl",			mcu_controller_func );
	mcu.insert( "sched",		mcu_sched_controller_func );
	mcu.insert( "motion",		mcu_motion_controller_func );
	mcu.insert( "stepturn",		mcu_stepturn_controller_func );
	mcu.insert( "quickdraw",	mcu_quickdraw_controller_func );
	mcu.insert( "gaze",			mcu_gaze_controller_func );
	mcu.insert( "gazelimit",	mcu_gaze_limit_func );
	mcu.insert( "snod",			mcu_snod_controller_func );
	mcu.insert( "lilt",			mcu_lilt_controller_func );
	mcu.insert( "divulge",		mcu_divulge_content_func );
	mcu.insert( "wsp",			mcu_wsp_cmd_func );
	mcu.insert( "create_remote_pawn", SbmPawn::create_remote_pawn_func );

	mcu.insert( "vrAgentBML",   BML_PROCESSOR::vrAgentBML_cmd_func );
	mcu.insert( "bp",		    BML_PROCESSOR::bp_cmd_func );
	mcu.insert( "vrSpeak",		BML_PROCESSOR::vrSpeak_func );

	mcu.insert( "net_reset",           mcu_net_reset );
	mcu.insert( "RemoteSpeechReply",   remoteSpeechResult_func );
	mcu.insert( "RemoteSpeechTimeOut", remoteSpeechTimeOut_func);  // internally routed message
	mcu.insert( "joint_logger",        joint_logger::start_stop_func );
	mcu.insert( "J_L",                 joint_logger::start_stop_func );  // shorthand
	mcu.insert( "locomotion",          locomotion_cmd_func );
	mcu.insert( "loco",                locomotion_cmd_func ); // shorthand
	mcu.insert( "resource",            resource_cmd_func );
	mcu.insert( "syncpolicy",          mcu_syncpolicy_func );
	mcu.insert( "check",			   mcu_check_func);		// check matching between .skm and .sk
	
	mcu.insert( "RemoteSpeechReplyRecieved", remoteSpeechReady_func);  // TODO: move to test commands

	mcu.insert_set_cmd( "bp",             BML_PROCESSOR::set_func );
	mcu.insert_set_cmd( "pawn",           SbmPawn::set_cmd_func );
	mcu.insert_set_cmd( "character",      SbmCharacter::set_cmd_func );
	mcu.insert_set_cmd( "char",           SbmCharacter::set_cmd_func );
	mcu.insert_set_cmd( "face",           mcu_set_face_func );
	mcu.insert_set_cmd( "joint_logger",   joint_logger::set_func );
	mcu.insert_set_cmd( "J_L",            joint_logger::set_func );  // shorthand
	mcu.insert_set_cmd( "test",           sbm_set_test_func );

	mcu.insert_print_cmd( "bp",           BML_PROCESSOR::print_func );
	mcu.insert_print_cmd( "pawn",         SbmPawn::print_cmd_func );
	mcu.insert_print_cmd( "character",    SbmCharacter::print_cmd_func );
	mcu.insert_print_cmd( "char",         SbmCharacter::print_cmd_func );
	mcu.insert_print_cmd( "face",         mcu_print_face_func );
	mcu.insert_print_cmd( "joint_logger", joint_logger::print_func );
	mcu.insert_print_cmd( "J_L",          joint_logger::print_func );  // shorthand
	mcu.insert_print_cmd( "mcu",          mcu_divulge_content_func );
	mcu.insert_print_cmd( "test",         sbm_print_test_func );

	mcu.insert_test_cmd( "args", test_args_func );
	mcu.insert_test_cmd( "bml",  test_bml_func );
	mcu.insert_test_cmd( "fml",  test_fml_func );
	mcu.insert_test_cmd( "locomotion", test_locomotion_cmd_func );
	mcu.insert_test_cmd( "loco",       test_locomotion_cmd_func );  // shorthand
	mcu.insert_test_cmd( "rhet", remote_speech_test);
	mcu.insert_test_cmd( "bone_pos", test_bone_pos_func );
	

	mcu.insert( "net",	mcu_net_func );

	mcu.insert( "PlaySound", mcu_play_sound_func );
	mcu.insert( "StopSound", mcu_stop_sound_func );

	mcu.insert( "uscriptexec", mcu_uscriptexec_func );

	mcu.insert( "CommAPI", mcu_commapi_func );

	mcu.insert( "vrKillComponent", mcu_vrKillComponent_func );
	mcu.insert( "vrAllCall", mcu_vrAllCall_func );
	mcu.insert("vrQuery", mcu_vrQuery_func );

	mcu.insert( "text_speech", text_speech::text_speech_func ); // [BMLR]
}


void cleanup( void )	{
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if( mcu.loop )	{
			LOG( "SBM NOTE: unexpected exit " );
			mcu.loop = false;
		}

		if ( mcu.play_internal_audio )
		{
			AUDIO_Close();
		}


		// Shutdown notifications
		SbmCharacter::remove_from_scene( "*" );
		mcu.vhmsg_send( "vrProcEnd sbm" );

#if LINK_VHMSG_CLIENT
		if( mcu.vhmsg_enabled )	{
			vhmsg::ttu_close();
		}
#endif
	}

	mcuCBHandle::destroy_singleton();
	
	XMLPlatformUtils::Terminate();

	ResourceManager::cleanup();
	LOG( "SBM: terminated gracefully." );


#if SBM_REPORT_MEMORY_LEAKS
	_CrtMemState  state;
	_CrtMemCheckpoint( &state );
	_CrtMemDumpStatistics( &state );
#endif
}

void signal_handler(int sig) {
	std::cout << "SmartBody shutting down after catching signal " << sig << std::endl;

	// get the current directory
	char buffer[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buffer);

	// dump to an available file in the current directory
	int counter = 1;
	bool fileOk = false;
	char file[1024];
	while (!fileOk || counter > 999)
	{
		sprintf(file, "%s\\smartbodycommands%.5d.dump", buffer, counter);

		std::fstream fs(file, std::ios_base::in);// attempt open for read
		if (!fs)
		{
			fileOk = true;
			fs.close();
			std::ofstream dumpFile;
			dumpFile.open(file);

			dumpFile << "Last commands run:" << std::endl;

			// dump the command resources
			int numResources = mcuCBHandle::singleton().resource_manager->getNumResources();
			for (int r = 0; r < numResources; r++)
			{
				CmdResource * res = dynamic_cast<CmdResource  *>(mcuCBHandle::singleton().resource_manager->getResource(r));
				if(res)
					dumpFile << res->dump() << std::endl;
			}
			dumpFile.close();
			std::cout << "Wrote commands to " << file << std::endl;

		}
		else //ok, file exists. close and reopen in write mode
		{
		  fs.close();
		  counter++;
		}
	}
	
	//cleanup(); // 
	exit(sig);
}

///////////////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv )	{


#if SBM_REPORT_MEMORY_LEAKS
	// CRT Debugging flags - Search help:
	//   _CrtSetDbgFlag
	// Memory Leaks - To find out where a memory leak happens:
	//   Rerun the programs and set this variable in the debugger
	//   to the alloc # that leaks: {,,msvcr90d.dll}_crtBreakAlloc
	//   The program will stop right before the alloc happens

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF );

	_CrtMemState  state;
	_CrtMemCheckpoint( &state );
	_CrtMemDumpStatistics( &state );
#endif

#if SBM_EMAIL_CRASH_REPORTS
	// Log crashes, with a dialog option to report via email
	vhcl::Crash::EnableExceptionHandling( true );
#endif

//	SBMWindow* sbmWindow = new SBMWindow(100, 100, 640, 480, "SmartBody");
//	sbmWindow->show();
//	CommandWindow* commandWindow = sbmWindow->getCommandWindow();
//	CommandWindow* commandWindow = new CommandWindow(100, 100, 640, 480);
//	vhcl::Log::g_log.AddListener(commandWindow);
	FltkViewer* viewer = new FltkViewer(100, 150, 640, 480, "SmartBody");

	// register the log listener
	//vhcl::Log::FileListener* listener = new vhcl::Log::FileListener("c:\\aoh.txt");
	vhcl::Log::StdoutListener* listener = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(listener);

	int err;
	SrString net_host;
	vector<string> seq_paths;
	vector<string> me_paths;
	vector<string> init_seqs;
	SrString proc_id;
	
	XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	FltkViewerFactory* viewerFactory = new FltkViewerFactory();
	//viewerFactory->setFltkViewer(sbmWindow->getFltkViewer());
//	viewerFactory->setFltkViewer(viewer);
	mcu.register_viewer_factory(viewerFactory);
	mcu.register_bmlviewer_factory(new BehaviorViewerFactory());

	// Build the floor for the viewer
	mcu.add_scene( build_checkerboard_floor( 200.0 ) );
	
	mcu_register_callbacks();

	TimeRegulator timer;
	mcu.register_timer( timer );

	TimeIntervalProfiler* profiler = new TimeIntervalProfiler();
	mcu.register_profiler(*profiler);

	// EDF - taken from tre_main.cpp, a fancier command line parser can be put here if desired.
	//	check	command line parameters:
	bool lock_dt_mode = false;
	int i;
	SrString	s;
	for (	i=1; i<argc; i++ )
	{
		LOG( "SBM ARG[%d]: '%s'", i, argv[i] );
		s = argv[i];

		if( s.search(	"-host=" ) == 0 )  // argument starts with -host=
		{
			net_host = s;
			net_host.remove( 0, 6 );
		}
		else if( s == "-mepath" )  // -mepath <dirpath> to specify where Motion Engine files (.sk, .skm) should be loaded from
		{
			if( ++i < argc ) {
				LOG( "    Adding ME path '%s'\n", argv[i] );

				me_paths.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected directory path to follow -mepath\n" );
				// return -1
			}
		}
		else if( s == "-seqpath" )  // -mepath <dirpath> to specify where sequence files (.seq) should be loaded from
		{
			if( ++i < argc ) {
				LOG( "    Adding sequence path '%s'\n", argv[i] );

				seq_paths.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected directory path to follow -seqpath\n" );
				// return -1
			}
		}
		else if( s == "-seq" )  // -seq <filename> to load seq file (replaces old -initseq notation)
		{
			if( ++i < argc ) {
				LOG( "    Loading sequence '%s'\n", argv[i] );
				init_seqs.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected filename to follow -seq\n" );
				// return -1
			}
		}
		else if( s.search( "-procid=" ) == 0 )  // argument starts with -procid=
		{
			proc_id = s;
			proc_id.remove( 0, 8 );
		}
		else if( s.search( "-audio" ) == 0 )  // argument equals -audio
		{
			mcu.play_internal_audio = true;
		}
		else if( s.search( "-lockdt" ) == 0 )  // argument equals -lockdt
		{
			lock_dt_mode = true;
		}
		else if( s.search( "-fps=" ) == 0 )  // argument starts with -fps=
		{
			SrString fps = s;
			fps.remove( 0, 5 );
			timer.set_sleep_fps( atof( fps ) );
		}
		else if( s.search( "-perf=" ) == 0 )  // argument starts with -perf=
		{
			SrString interval = s;
			interval.remove( 0, 6 );
			timer.set_perf( atof( interval ) );
		}
		else if ( s.search( "-facebone" ) == 0 )
		{
			mcu.net_face_bones = true;
		}
		else
		{
			LOG( "ERROR: Unrecognized command line argument: \"%s\"\n", (const char*)s );
		}
	}
	if( lock_dt_mode )	{ 
		timer.set_sleep_lock();
	}

#if LINK_VHMSG_CLIENT
	char * vhmsg_server = getenv( "VHMSG_SERVER" );
	bool vhmsg_disabled = ( vhmsg_server != NULL && strcasecmp( vhmsg_server, "none" ) == 0 );  // hope there is no valid server named "none"

	if( !vhmsg_disabled &&
		vhmsg::ttu_open()==vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( sbm_vhmsg_callback );
		err = vhmsg::ttu_register( "sbm" );
		err = vhmsg::ttu_register( "vrAgentBML" );
		err = vhmsg::ttu_register( "vrSpeak" );
		err = vhmsg::ttu_register( "RemoteSpeechReply" );
		err = vhmsg::ttu_register( "PlaySound" );
		err = vhmsg::ttu_register( "StopSound" );
		err = vhmsg::ttu_register( "CommAPI" );
		err = vhmsg::ttu_register( "object-data" );
		err = vhmsg::ttu_register( "vrAllCall" );
		err = vhmsg::ttu_register( "vrKillComponent" );
		err = vhmsg::ttu_register( "wsp" );

		mcu.vhmsg_enabled = true;
	} else {
		if( vhmsg_disabled ) {
			LOG( "SBM: VHMSG_SERVER='%s': Messaging disabled.\n", vhmsg_server?"NULL":vhmsg_server );
		} else {
#if 0 // disable server name query until vhmsg is fixed
			const char* vhmsg_server_actual = vhmsg::ttu_get_server();
			LOG( "SBM ERR: ttu_open VHMSG_SERVER='%s' FAILED\n", vhmsg_server_actual?"NULL":vhmsg_server_actual );
#else
			LOG( "SBM ERR: ttu_open FAILED\n" );
#endif
		}
		mcu.vhmsg_enabled = false;
	}
#endif

	// Sets up the network connection for sending bone rotations over to Unreal
	if( net_host != "" )
		mcu.set_net_host( net_host );
	if( proc_id != "" ) {
		mcu.set_process_id( (const char*)proc_id );

		// Using a process id is a sign that we're running in a multiple SBM environment.
		// So.. ignore BML requests with unknown agents by default
		mcu.bml_processor.set_warn_unknown_agents( false );
	}

	if ( mcu.play_internal_audio )
	{
		if ( !AUDIO_Init() )
		{
			LOG( "ERROR: Audio initialization failed\n" );
		}
	}

	mcu.speech_audiofile_base_path = "../../../../";

	/*(void)signal( SIGABRT, signal_handler );
	(void)signal( SIGFPE, signal_handler );
	(void)signal( SIGILL, signal_handler );
	(void)signal( SIGINT, signal_handler );
	(void)signal( SIGSEGV, signal_handler );
	(void)signal( SIGTERM, signal_handler );
	(void)signal( SIGBREAK, signal_handler );*/
	//atexit( exit_callback );

	srCmdLine cmdl;
	printf("> " );
	
#if ENABLE_DEFAULT_BOOTSTRAP
	vector<string>::iterator it;

	for( it = me_paths.begin();
	     it != me_paths.end();
		 ++it )
	{
		SrString seq_command = SrString( "path me " ) << (it->c_str());
		mcu.execute( (char *)(const char *)seq_command );
	}

	if( seq_paths.empty() ) {
		LOG( "No sequence paths specified. Adding current working directory to seq path\n" );
		seq_paths.push_back( "." );
	}
	for( it = seq_paths.begin();
	     it != seq_paths.end();
		 ++it )
	{
		SrString seq_command = SrString( "path seq " ) << (it->c_str());
		mcu.execute( (char *)(const char *)seq_command );
	}

	if( init_seqs.empty() ) {
		LOG( "No sequences specified. Loading sequence '%s'\n", DEFAULT_SEQUENCE_FILE );
		init_seqs.push_back( DEFAULT_SEQUENCE_FILE );
	}

	for( it = init_seqs.begin();
	     it != init_seqs.end();
		 ++it )
	{
		SrString seq_command = SrString( "seq " ) << (it->c_str()) << " begin";
		mcu.execute( (char *)(const char *)seq_command );
	}

	me_paths.clear();
	seq_paths.clear();
	init_seqs.clear();
#endif

	// Notify world SBM is ready to receive messages
	mcu_vrAllCall_func( srArgBuffer(""), &mcu );

	timer.start();
	while( mcu.loop )	{
		
		mcu.update_profiler();
//		mcu.update_profiler( SBM_get_real_time() );
		bool update_sim = mcu.update_timer();
//		bool update_sim = mcu.update_timer( SBM_get_real_time() );


//mcu.mark( "main", 0, "fltk::check" );

		fltk::check();

//mcu.mark( "main", 0, "commands" );

#if LINK_VHMSG_CLIENT
		if( mcu.vhmsg_enabled )	{
			err = vhmsg::ttu_poll();
			if( err == vhmsg::TTU_ERROR )	{
				fprintf( stderr, "ttu_poll ERROR\n" );
			}
		}
#endif

		// [BMLR] Added to support receiving commands from renderer
		vector<string> commands = mcu.bonebus.GetCommand();
		for ( size_t i = 0; i < commands.size(); i++ ) {
			mcu.execute( (char *)commands[i].c_str() );
		}

		if( cmdl.pending_cmd() )	{
			char *cmd = cmdl.read_cmd();
			if( strlen( cmd ) )	{
				switch( int ret = mcu.execute( cmd ) ) {
					case CMD_NOT_FOUND:
						printf("SBM ERR: command NOT FOUND: '%s'\n> ", cmd );
						break;
					case CMD_FAILURE:
						printf("SBM ERR: command FAILED: '%s'\n> ", cmd );
						break;
					case CMD_SUCCESS:
						printf("> " );  // new prompt
						break;
					default:
						printf("SBM ERR: return value %d ERROR: '%s'\n> ", ret, cmd );
						break;
				}
			}
			else	{
				printf("> " );
			}
			fflush( stdout );
		}

		mcu.theWSP->broadcast_update();

//mcu.mark( "main", 0, "update" );
		if( update_sim )	{
			mcu.update();
		}

//mcu.mark( "main", 1, "render" );

		mcu.render();

//mcu.mark( "main" );
	}

	cleanup();
	//vhcl::Log::g_log.RemoveAllListeners();
	//delete listener;
//	delete sbmWindow;
}

