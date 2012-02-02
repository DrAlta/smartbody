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

#define ENABLE_CMDL_TEST		0
#define ENABLE_808_TEST			0

#define SBM_REPORT_MEMORY_LEAKS  0
#define SBM_EMAIL_CRASH_REPORTS  1

#include "vhcl.h"
#include "external/glew/glew.h"

#include <signal.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

#include "fltk_viewer.h"
#include "RootWindow.h"
#include <bmlviewer/BehaviorWindow.h>
#include <panimationviewer/PanimationWindow.h>
#include <channelbufferviewer/channelbufferWindow.hpp>
#include <resourceViewer/ResourceWindow.h>
#include <faceviewer/FaceViewer.h>

#if USE_WSP
#include "wsp.h"
#endif

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
//#include "SBMWindow.h"
#include "CommandWindow.h"
#include <sbm/SBPython.h>
#include "FLTKListener.h"
#include "sbm/SbmDebuggerServer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <sbm/sr_cmd_line.h>
#include <sbm/gwiz_cmdl.h>

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

#include "pic.h"
//#include <FL/Fl_glut.h>
#include "sr/sr_model.h"

#define ENABLE_DEFAULT_BOOTSTRAP	(1) 
//#define DEFAULT_SEQUENCE_FILE		("ELITE-all.seq")
#define DEFAULT_SEQUENCE_FILE		("default.seq")
#define DEFAULT_PY_FILE				("default-init.py")

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

	mcuCBHandle::singleton()._scene->getDebuggerServer()->ProcessVHMsgs(op, args);
}

int sbm_vhmsg_register_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	
	return( CMD_SUCCESS );
}


// snapshot <windowHeight> <windowWidth> <offsetHeight> <offsetWidth> <output file>
// The offset is according to the left bottom corner of the image frame buffer
int mcu_snapshot_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
	if( mcu_p )
	{
		BaseWindow* rootWindow = dynamic_cast<BaseWindow*>(mcu_p->viewer_p);
		if (!rootWindow)
		{
			LOG("Viewer doesn't exist. Cannot take snapshot.");
			return CMD_FAILURE;
		}

		int windowHeight = args.read_int();
		int windowWidth = args.read_int();
		int offsetHeight = args.read_int();
		int offsetWidth = args.read_int();

		string output_file = args.read_token();

		if( windowHeight == 0 )		windowHeight = rootWindow->fltkViewer->h();;							// default window size
		if( windowWidth == 0 )		windowWidth = rootWindow->fltkViewer->w();
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

int mcu_quit_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{

	mcu_p->loop = false;

	if (mcu_p->_scene->getSteerManager()->getEngineDriver()->isInitialized())
	{
		mcu_p->_scene->getSteerManager()->getEngineDriver()->stopSimulation();
		mcu_p->_scene->getSteerManager()->getEngineDriver()->unloadSimulation();
		mcu_p->_scene->getSteerManager()->getEngineDriver()->finish();
	
		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
			iter != mcu_p->getCharacterMap().end();
			iter++)
		{
			SbmCharacter* character = (*iter).second;
			if (character->steeringAgent)
				character->steeringAgent->setAgent(NULL);
		}
	}
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

	// additional commands associated with this viewer
	mcu.insert( "q",			mcu_quit_func );
	mcu.insert( "quit",			mcu_quit_func );
	mcu.insert( "snapshot",		mcu_snapshot_func );
	mcu.insert( "viewer",		mcu_viewer_func );
	mcu.insert( "bmlviewer",    mcu_bmlviewer_func);
	mcu.insert( "panimviewer",  mcu_panimationviewer_func);
	mcu.insert( "cbufviewer",	mcu_channelbufferviewer_func); // deprecated
	mcu.insert( "dataviewer",	mcu_channelbufferviewer_func);
	mcu.insert( "resourceviewer",	mcu_resourceViewer_func);	
	mcu.insert( "faceviewer",	mcu_faceViewer_func);
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

		mcu.vhmsg_send( "vrProcEnd sbm" );

#if LINK_VHMSG_CLIENT
		if( mcu.vhmsg_enabled )	{
			vhmsg::ttu_close();
		}
#endif
	}

	mcuCBHandle::destroy_singleton();
	
	XMLPlatformUtils::Terminate();

	SBResourceManager::cleanup();
	LOG( "SBM: terminated gracefully." );


#if SBM_REPORT_MEMORY_LEAKS
	_CrtMemState  state;
	_CrtMemCheckpoint( &state );
	_CrtMemDumpStatistics( &state );
#endif
}

void signal_handler(int sig) {
//	std::cout << "SmartBody shutting down after catching signal " << sig << std::endl;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.vhmsg_send( "vrProcEnd sbm" );
	// get the current directory
#ifdef WIN32
	char buffer[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buffer);
#else
	char buffer[PATH_MAX];
	getcwd(buffer, PATH_MAX);
#endif

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
			//std::cout << "Wrote commands to " << file << std::endl;

			dumpFile.close();

		}
		else //ok, file exists. close and reopen in write mode
		{
		  fs.close();
		  counter++;
		}
	}
	
	//cleanup(); // 
	//exit(sig);
}


class SBMCrashCallback : public vhcl::Crash::CrashCallback
{
   public:
      virtual void OnCrash() { signal_handler( -1 ); }
};


///////////////////////////////////////////////////////////////////////////////////
std::string test_cmdl_tab_callback( std::string io_str )	{
	std::string prompt( "X> " );

	gwiz::cmdl commandline;
	commandline.set_callback( gwiz::cmdl::editor_callback );
	commandline.render_prompt( prompt );
	commandline.write( io_str );

	bool quit = false;
	while( !quit ) {
		if( commandline.pending( prompt ) )	{

			std::string new_str = commandline.read();
			if( new_str == "t" )	{
				commandline.write( std::string( "test" ) );
			}
			else
			if( new_str == "q" )	{
				quit = true;
				io_str = "done";
			}
		}
	}
	fprintf( stdout, "<exit>" ); 
	fprintf( stdout, "\n" ); 
	return( io_str );
}

///////////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )	{

 // 808: undefined reference to `bonebus::BoneBusCharacter::StartSendBoneRotations()'
//	return( 0 );

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
	vhcl::Crash::AddCrashCallback( new SBMCrashCallback() );
#endif

	
	
	

	// init glew to use OpenGL extension
	//bool hasShaderSupport = SbmShader::initShader();

	//CommandWindow* commandWindow = new CommandWindow(100, 100, 640, 480, "Commands");
	//commandWindow->show();
	//vhcl::Log::g_log.AddListener(commandWindow);
	//FltkViewer* viewer = new FltkViewer(100, 150, 640, 480, "SmartBody");

	// register the log listener
	vhcl::Log::StdoutListener* listener = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(listener);


	int err;
	SrString net_host;
	vector<string> seq_paths;
	vector<string> py_paths;
	vector<string> me_paths;
	vector<string> audio_paths;
	vector<string> init_seqs;
	vector<string> init_pys;
	SrString proc_id;
	
	XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	FLTKListener fltkListener;
	mcu.sbm_character_listener = &fltkListener;

	// change the default font size
	FL_NORMAL_SIZE = 12;
	FltkViewerFactory* viewerFactory = new FltkViewerFactory();
	//viewerFactory->setFltkViewer(sbmWindow->getFltkViewer());
	//viewerFactory->setFltkViewer(viewer);
	mcu.register_viewer_factory(viewerFactory);
	mcu.register_bmlviewer_factory(new BehaviorViewerFactory());
	mcu.register_panimationviewer_factory(new PanimationViewerFactory());
	mcu.register_channelbufferviewer_factory(new ChannelBufferViewerFactory());	
	mcu.register_ResourceViewer_factory(new ResourceViewerFactory());
	mcu.register_FaceViewer_factory(new FaceViewerFactory());

	// Build the floor for the viewer
	//mcu.add_scene( build_checkerboard_floor( 200.0 ) );
	
	mcu_register_callbacks();

	TimeRegulator timer;
	mcu.register_timer( timer );

	TimeIntervalProfiler* profiler = new TimeIntervalProfiler();
	mcu.register_profiler(*profiler);

	std::string python_lib_path = "../../../../core/smartbody/Python26/Lib";
	std::string festivalLibDir = "../../../../lib/festival/festival/lib/";
	std::string festivalCacheDir = "../../../../data/cache/festival/";
	// EDF - taken from tre_main.cpp, a fancier command line parser can be put here if desired.
	//	check	command line parameters:
	bool lock_dt_mode = false;
	int i;
	SrString	s;
	for (	i=1; i<argc; i++ )
	{
		LOG( "SBM ARG[%d]: '%s'", i, argv[i] );
		s = argv[i];

		if( s == "-pythonpath" )  // argument -pythonpath
		{
			if( ++i < argc ) {
				LOG( "    Adding path path '%s'\n", argv[i] );
				python_lib_path = argv[i];
			} else {
				LOG( "ERROR: Expected directory path to follow -pythonpath\n" );
				// return -1
			}
		}
		else if( s == "-python" )  // argument -python
		{
			mcu.use_python = true;
		}
		else if( s.search(	"-host=" ) == 0 )  // argument starts with -host=
		{
			net_host = s;
			net_host.remove( 0, 6 );
		}
		else if( s == "-python" )  // argument -python
		{
			mcu.use_python = true;
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
		else if( s == "-seqpath" || s == "-scriptpath" )  // -mepath <dirpath> to specify where sequence files (.seq) should be loaded from
		{
			if( ++i < argc ) {
				LOG( "    Adding sequence path '%s'\n", argv[i] );

				seq_paths.push_back( argv[i] );
				py_paths.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected directory path to follow -seqpath\n" );
				// return -1
			}
		}
		else if( s == "-pypath" )  // -mepath <dirpath> to specify where sequence files (.seq) should be loaded from
		{
			if( ++i < argc ) {
				LOG( "    Adding python script path '%s'\n", argv[i] );

				py_paths.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected directory path to follow -pypath\n" );
				// return -1
			}
		}
		else if( s == "-audiopath" )  // -audiopath <dirpath> to specify where audio files (.wav and .bml) should be loaded from
		{
			if( ++i < argc ) {
				LOG( "    Addcore/smartbody/sbm/src/sbm_main.cpping audio path '%s'\n", argv[i] );

				audio_paths.push_back( argv[i] );
			} else {
				LOG( "ERROR: Expected directory path to follow -audiopath\n" );
				// return -1
			}
		}
		else if( s == "-seq" || s == "-script")  // -seq <filename> to load seq file (replaces old -initseq notation)
		{
			if( ++i < argc ) {
				std::string filename = argv[i];
				std::string::size_type idx;

				idx = filename.rfind('.');

				if(idx != std::string::npos)
				{
					std::string extension = filename.substr(idx+1);
					if (extension == "py")
					{
						LOG( "    Loading Python scrypt '%s'\n", argv[i] );
						init_pys.push_back( argv[i] );
						mcu.use_python = true;
					}
					else
					{
						LOG( "    Loading sequence '%s'\n", argv[i] );
						init_seqs.push_back( argv[i] );
					}
				}
				else
				{
					// No extension found
					LOG( "    Loading sequence '%s'\n", argv[i] );
					init_seqs.push_back( argv[i] );
				}
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
		else if ( s.search( "-skscale=" ) == 0 )
		{
			SrString skScale = s;
			skScale.remove( 0, 9 );
			mcu.skScale = atof(skScale);
		}
		else if ( s.search( "-skmscale=" ) == 0 )
		{
			SrString skmScale = s;
			skmScale.remove( 0, 10 );
			mcu.skmScale = atof(skmScale);
		}
		else if ( s.search( "-mediapath=" ) == 0 )
		{
			std::string mediaPath = (const char*) s;
			mediaPath = mediaPath.substr(11);
			mcu.setMediaPath(mediaPath);
		}
                else if ( s.search ("-noninteractive") == 0)
                {
                       mcu.setInteractive(false);
                }
		else
		{
			LOG( "ERROR: Unrecognized command line argument: \"%s\"\n", (const char*)s );
		}
	}
	if( lock_dt_mode )	{ 
		timer.set_sleep_lock();
	}

	// initialize python
	initPython(python_lib_path);
	mcu.festivalRelay()->initSpeechRelay(festivalLibDir,festivalCacheDir);

#if LINK_VHMSG_CLIENT
	char * vhmsg_server = getenv( "VHMSG_SERVER" );
	char * vhmsg_port = getenv("VHMSG_PORT");
	bool vhmsg_disabled = ( vhmsg_server != NULL && strcasecmp( vhmsg_server, "none" ) == 0 );  // hope there is no valid server named "none"

	if( !vhmsg_disabled &&
		vhmsg::ttu_open()==vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( sbm_vhmsg_callback );
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

		mcu.vhmsg_enabled = true;
	} else {
		if( vhmsg_disabled ) {
			LOG( "SBM: VHMSG_SERVER='%s': Messaging disabled.\n", vhmsg_server?"NULL":vhmsg_server );
		} else {
#if 0 // disable server name query until vhmsg is fixed
			const char* vhmsg_server_actual = vhmsg::ttu_get_server();
			LOG( "SBM ERR: ttu_open VHMSG_SERVER='%s' FAILED\n", vhmsg_server_actual?"NULL":vhmsg_server_actual );
#else
			std::string vhserver = (vhmsg_server? vhmsg_server : "localhost");
			std::string vhport = (vhmsg_port ? vhmsg_port : "61616");
			LOG( "SBM ERR: ttu_open FAILED\n" );
			LOG("Could not connect to %s:%s", vhserver.c_str(), vhport.c_str());
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

	mcu._scene->getDebuggerServer()->SetID("sbm-fltk-" + vhcl::ToString(rand()));  // TODO:  populate with process id

//	(void)signal( SIGABRT, signal_handler );
//	(void)signal( SIGFPE, signal_handler );
//	(void)signal( SIGILL, signal_handler );
//	(void)signal( SIGINT, signal_handler );
//	(void)signal( SIGSEGV, signal_handler );
//	(void)signal( SIGTERM, signal_handler );
#ifdef WIN32
	(void)signal( SIGBREAK, signal_handler );
#endif
//	atexit( exit_callback );

	gwiz::cmdl commandline;
	commandline.set_callback( mcuCBHandle::cmdl_tab_callback );

#if ENABLE_DEFAULT_BOOTSTRAP
	vector<string>::iterator it;

	if( seq_paths.empty() && py_paths.empty() ) {
		LOG( "No script paths specified. Adding current working directory to script path.\n" );
		seq_paths.push_back( "." );
	}

	for( it = me_paths.begin();
	     it != me_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr << "path me " << it->c_str();
		mcu.execute( (char *) strstr.str().c_str() );
	}

	for( it = seq_paths.begin();
	     it != seq_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr << "path seq " << (it->c_str());
		SrString seq_command = SrString( "path seq " );
		mcu.execute( (char *) strstr.str().c_str() );
	}

	for( it = audio_paths.begin();
	     it != audio_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr <<  "path audio " << it->c_str();
		mcu.execute( (char *) strstr.str().c_str() );
	}


	for( it = py_paths.begin();
	     it != py_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr << "scene.addAssetPath('seq', '" << it->c_str() << "')";
		mcu.executePython( (char *) strstr.str().c_str() );
	}

	// run the specified scripts
	if( init_seqs.empty() && init_pys.empty())
	{
		if (!mcu.use_python)
		{
			LOG( "No sequences specified. Loading sequence '%s'\n", DEFAULT_SEQUENCE_FILE );
			init_seqs.push_back( DEFAULT_SEQUENCE_FILE );
		}
		else
		{
			LOG( "No Python scripts specified. Loading script '%s'\n", DEFAULT_PY_FILE );
			init_pys.push_back( DEFAULT_PY_FILE );
		}
	}

	for( it = init_seqs.begin();
		 it != init_seqs.end();
		 ++it )
	{
		SrString seq_command = SrString( "seq " ) << (it->c_str()) << " begin";
		mcu.execute( (char *)(const char *)seq_command );
	}


	for( it = init_pys.begin();
		 it != init_pys.end();
		 ++it )
	{
		std::string cmd = it->c_str();
		std::stringstream strstr;
		strstr << "scene.run(\"" << cmd.c_str() << "\")";
		mcu.executePython(strstr.str().c_str());
	}

	me_paths.clear();
	seq_paths.clear();
	init_seqs.clear();
#endif

	// Notify world SBM is ready to receive messages
	srArgBuffer argBuff("");
	mcu_vrAllCall_func( argBuff, &mcu );

	timer.start();

#if ENABLE_808_TEST
	return( 0 );
#endif

#if EARLY_EXIT
	mcu.loop = 0;
#endif
//	commandline.render_prompt( "> " );

	while( mcu.loop )	{

		mcu.update_profiler();
//		mcu.update_profiler( SBM_get_real_time() );
		bool update_sim = mcu.update_timer();
//		bool update_sim = mcu.update_timer( SBM_get_real_time() );

	//	mcu.mark( "main", 0, "fltk-check" );
		Fl::check();

#if LINK_VHMSG_CLIENT
		if( mcu.vhmsg_enabled )	{
			err = vhmsg::ttu_poll();
			if( err == vhmsg::TTU_ERROR )	{
				fprintf( stderr, "ttu_poll ERROR\n" );
			}
		}
#endif

		vector<string> commands;// = mcu.bonebus.GetCommand();
		for ( size_t i = 0; i < commands.size(); i++ ) {
			mcu.execute( (char *)commands[i].c_str() );
		}

		if (mcu.getInteractive())
		{
			if( commandline.pending( "> " ) )	{
				std::string cmd_str = commandline.read();
				char *cmd = (char*)cmd_str.c_str();

				if( strlen( cmd ) )	{

					int result = CMD_FAILURE;
					if (mcu.use_python) {
						result = mcu.executePython(cmd);
					} else {
						result = mcu.execute(cmd);
					}

					switch( result ) {
						case CMD_NOT_FOUND:
							printf("SBM ERR: command NOT FOUND: '%s'\n", cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
						case CMD_FAILURE:
							printf("SBM ERR: command FAILED: '%s'\n", cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
						case CMD_SUCCESS:						
							break;
						default:
							printf("SBM ERR: return value %d ERROR: '%s'\n", result, cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
					}
				}
			}
		}

#if USE_WSP
		mcu.theWSP->broadcast_update();
#endif

		if( update_sim )	{
			mcu.update();
		}

		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			 iter != mcu.getCharacterMap().end();
			 iter++)
		{
			SbmCharacter* character = (*iter).second;
			if (character->scene_p)
				character->scene_p->update();	
		}

		// update any tracked cameras
		for (size_t x = 0; x < mcu.cameraTracking.size(); x++)
		{
			// move the camera relative to the joint
			SkJoint* joint = mcu.cameraTracking[x]->joint;
			joint->skeleton()->update_global_matrices();
			joint->update_gmat();
			const SrMat& jointGmat = joint->gmat();
			SrVec jointLoc(jointGmat[12], jointGmat[13], jointGmat[14]);
			SrVec newJointLoc = jointLoc;
			if (fabs(jointGmat[13] - mcu.cameraTracking[x]->yPos) < mcu.cameraTracking[x]->threshold)
				newJointLoc.y = (float)mcu.cameraTracking[x]->yPos;
			SrVec cameraLoc = newJointLoc + mcu.cameraTracking[x]->jointToCamera;
			mcu.camera_p->eye.set(cameraLoc.x, cameraLoc.y, cameraLoc.z);
			SrVec targetLoc = cameraLoc - mcu.cameraTracking[x]->targetToCamera;
			mcu.camera_p->center.set( targetLoc.x, targetLoc.y, targetLoc.z);
			mcu.viewer_p->set_camera(*( mcu.camera_p ));
		}	

		if((ChannelBufferWindow*)mcu.channelbufferviewer_p != NULL)
		{
			((ChannelBufferWindow*)mcu.channelbufferviewer_p)->update();
		}

		if((ResourceWindow*)mcu.resourceViewer_p != NULL)
		{
			((ResourceWindow*)mcu.resourceViewer_p)->update();
		}

		mcu.render();
	
	}	
	cleanup();
	//vhcl::Log::g_log.RemoveAllListeners();
	//delete listener;
//	delete sbmWindow;

//	return( 0 ); // NOT NEEDED ??
}

