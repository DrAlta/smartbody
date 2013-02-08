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

#include "external/glew/glew.h"
#include "vhcl.h"
#include <sbm/lin_win.h>
#include <signal.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "fltk_viewer.h"
#include "RootWindow.h"
#include <bmlviewer/BehaviorWindow.h>
#include <panimationviewer/PanimationWindow.h>
#include <channelbufferviewer/channelbufferWindow.hpp>
#include <resourceViewer/ResourceWindow.h>
#include <faceviewer/FaceViewer.h>

#ifndef USE_WSP
#define USE_WSP 1
#endif

#if USE_WSP
#include "wsp.h"
#endif

#include <sbm/sbm_constants.h>
#include <sbm/xercesc_utils.hpp>
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include BML_PROCESSOR_INCLUDE
#include <sbm/remote_speech.h>
#include <sbm/local_speech.h>
#include <sbm/sbm_audio.h>
#include <sbm/sbm_speech_audiofile.hpp>
#include <sbm/text_speech.h> // [BMLR]
#include <sbm/locomotion_cmds.hpp>
#include <sbm/time_regulator.h>
//#include "SBMWindow.h"
#include "CommandWindow.h"
#include <sb/SBPython.h>
#include <sb/SBSteerManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBSpeechManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBWSPManager.h>
#include "FLTKListener.h"
#include <sb/SBDebuggerServer.h>
#include <sb/SBDebuggerClient.h>
#include <sbm/PPRAISteeringAgent.h>

#if USE_OGRE_VIEWER > 0
#include "FLTKOgreViewer.h"
#endif

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

///////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32_LEAN_AND_MEAN
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#else
#endif

using std::vector;
using std::string;

int snapshotCounter = 0;

///////////////////////////////////////////////////////////////////////////////////

void sbm_vhmsg_callback( const char *op, const char *args, void * user_data ) 
{	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	// Replace singleton with a user_data pointer
	if (scene->isRemoteMode())
	{
		scene->getDebuggerClient()->ProcessVHMsgs(op, args);
		return;
	}
	else
	{
		scene->getDebuggerServer()->ProcessVHMsgs(op, args);
	}

	std::stringstream strstr;
	strstr << op << "  " << args;
	switch( scene->command(strstr.str() ))
	{
        case false:
            LOG("SmartBody Error: command FAILED: '%s' + '%s'", op, args );
            break;
    }
}

// snapshot <windowHeight> <windowWidth> <offsetHeight> <offsetWidth> <output file>
// The offset is according to the left bottom corner of the image frame buffer
int mcu_snapshot_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	BaseWindow* rootWindow = dynamic_cast<BaseWindow*>(mcu.viewer_p);
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
		output_file_os<< "snapshot_"<< snapshotCounter<< ".ppm";	// default output name
		snapshotCounter++;
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

	return( CMD_SUCCESS );
}

int mcu_quit_func( srArgBuffer& args, SmartBody::SBCommandManager* cmdMgr  )	{

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->getSimulationManager()->stop();
	if (scene->getSteerManager()->getEngineDriver()->isInitialized())
	{
		scene->getSteerManager()->getEngineDriver()->stopSimulation();
		scene->getSteerManager()->getEngineDriver()->unloadSimulation();
		scene->getSteerManager()->getEngineDriver()->finish();
	
		std::vector<std::string> characterNames = scene->getCharacterNames();
		for (std::vector<std::string>::iterator iter = characterNames.begin();
			iter != characterNames.end();
			iter++)
		{
			SmartBody::SBCharacter* character = scene->getCharacter((*iter));
			SmartBody::SBSteerAgent* steerAgent = SmartBody::SBScene::getScene()->getSteerManager()->getSteerAgent(character->getName());
			if (steerAgent)
			{
				PPRAISteeringAgent* ppraiAgent = dynamic_cast<PPRAISteeringAgent*>(steerAgent);
				ppraiAgent->setAgent(NULL);
			}
		}
	}
	return( CMD_SUCCESS );
}

///////////////////////////////////////////////////////////////////////////////////

void mcu_register_callbacks( void ) {
	
	SmartBody::SBCommandManager* cmdMgr = SmartBody::SBScene::getScene()->getCommandManager();
	
	// additional commands associated with this viewer
	cmdMgr->insert( "q",			mcu_quit_func );
	cmdMgr->insert( "quit",			mcu_quit_func );
	cmdMgr->insert( "snapshot",		mcu_snapshot_func );
	cmdMgr->insert( "viewer",		mcu_viewer_func );
}

void cleanup( void )	{
	{

		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if (SmartBody::SBScene::getScene()->getSimulationManager()->isStopped())
		{
			LOG( "SmartBody NOTE: unexpected exit " );
		}

		if (SmartBody::SBScene::getScene()->getBoolAttribute("internalAudio"))
		{
			AUDIO_Close();
		}

		SmartBody::SBScene::getScene()->getVHMsgManager()->send("vrProcEnd sbm");

#if LINK_VHMSG_CLIENT
		if (SmartBody::SBScene::getScene()->getVHMsgManager()->isEnable())
		{
			SmartBody::SBScene::getScene()->getVHMsgManager()->disconnect();
		}
#endif
	}

	mcuCBHandle::destroy_singleton();
	
	XMLPlatformUtils::Terminate();

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

	SmartBody::SBScene::getScene()->getVHMsgManager()->send("vrProcEnd sbm" );
	// get the current directory
#ifdef WIN32
	char buffer[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buffer);
#else
	char buffer[PATH_MAX];
	getcwd(buffer, PATH_MAX);
#endif
	/*
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
	*/
	
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
	string net_host;
	vector<string> seq_paths;
	vector<string> py_paths;
	vector<string> me_paths;
	vector<string> audio_paths;
	vector<string> init_seqs;
	vector<string> init_pys;
	string proc_id;
	
	XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	FLTKListener fltkListener;

	// change the default font size
	FL_NORMAL_SIZE = 12;
	FltkViewerFactory* viewerFactory = new FltkViewerFactory();
	//viewerFactory->setFltkViewer(sbmWindow->getFltkViewer());
	//viewerFactory->setFltkViewer(viewer);
	mcu.register_viewer_factory(viewerFactory);
#if USE_OGRE_VIEWER > 0
	mcu.register_OgreViewer_factory(new OgreViewerFactory());
#endif
	
	

	std::string python_lib_path = "../../../../core/smartbody/Python26/Lib";
	std::string festivalLibDir = "../../../../lib/festival/festival/lib/";
	std::string festivalCacheDir = "../../../../data/cache/festival/";
	std::string mediaPath = "../../../../data";


	std::string cereprocLibDir = "../../../../lib/cerevoice/voices/";	



	// look for a file called .smartbodysettings in the current directory
	// to determine the initial settings
	std::ifstream settingsFile(".smartbodysettings");

	if (!settingsFile.good())
	{
		LOG("Did not find .smartbodysettings in current directory, using default paths.");
	}
	else
	{
		LOG("Found .smartbodysettings file.");
		std::string line;
		while (!settingsFile.eof())
		{
			getline(settingsFile, line);
			
			std::vector<std::string> tokens;
			vhcl::Tokenize(line, tokens, "=");
			for (size_t t = 0; t < tokens.size(); t++)
			{
				if (tokens[t] == "pythonlibpath")
				{
					if (tokens.size() > t + 1)
					{
						python_lib_path = tokens[t + 1];
						LOG("Setting Python Library path to %s", tokens[t + 1].c_str());
						SmartBody::SBScene::setSystemParameter("pythonlibpath", python_lib_path);
						t++;
					}
					
				}
				else if (tokens[t] == "mediapath")
				{
					if (tokens.size() > t + 1)
					{
						mediaPath = tokens[t + 1];
						LOG("Setting mediapath to %s", tokens[t + 1].c_str());
						SmartBody::SBScene::setSystemParameter("mediapath", mediaPath);
						t++;
					}
				}
				else
				{
					LOG("Unknown setting found in .smartbodysettings file: %s", line.c_str());
					LOG("Valid settings are: pythonlibpath=<dir>  or mediapath=<dir>");
				}
			}
		}
	}
	settingsFile.close();
	// EDF - taken from tre_main.cpp, a fancier command line parser can be put here if desired.
	//	check	command line parameters:
	bool lock_dt_mode = false;
	float sleepFPS = -1;
	float intervalAmount = -1;
	bool isInteractive = true;
	int i;
	for (	i=1; i<argc; i++ )
	{
		LOG( "SmartBody ARG[%d]: '%s'", i, argv[i] );
		std::string s = argv[i];
		std::string mediapathstr = "";
		if (s.size() > 11)
			mediapathstr = s.substr(0, 10);

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
		else if( s.compare( "-host=" ) == 0 )  // argument starts with -host=
		{
			net_host = s;
			net_host.erase(0, 6);
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

				//seq_paths.push_back( argv[i] );
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
		else if( s.compare( "-procid=" ) == 0 )  // argument starts with -procid=
		{
			proc_id = s;
			proc_id.erase( 0, 8 );
		}
		else if( s.compare( "-audio" ) == 0 )  // argument equals -audio
		{
			 SmartBody::SBScene::getScene()->setBoolAttribute("internalAudio", true);
		}
		else if( s.compare( "-lockdt" ) == 0 )  // argument equals -lockdt
		{
			lock_dt_mode = true;
		}
		else if( s.compare( "-fps=" ) == 0 )  // argument starts with -fps=
		{
			string fps = s;
			fps.erase( 0, 5 );
			sleepFPS = (float) atof( fps.c_str() );
		}
		else if( s.compare( "-perf=" ) == 0 )  // argument starts with -perf=
		{
			string interval = s;
			interval.erase( 0, 6 );
			intervalAmount = (float) atof( interval.c_str() );
			
		}
		else if ( s.compare( "-facebone" ) == 0 )
		{
			LOG("-facebone option has been deprecated.");
		}
		else if ( s.compare( "-skscale=" ) == 0 )
		{
			string skScale = s;
			skScale.erase( 0, 9 );
			SmartBody::SBScene::getScene()->getAssetManager()->setGlobalSkeletonScale(atof(skScale.c_str()));
		}
		else if ( s.compare( "-skmscale=" ) == 0 )
		{
			string skmScale = s;
			skmScale.erase( 0, 10 );
			SmartBody::SBScene::getScene()->getAssetManager()->setGlobalSkeletonScale(atof(skmScale.c_str()));
		}
		else if (mediapathstr == "-mediapath")
		{
			mediaPath = s.substr(11);
			SmartBody::SBScene::getScene()->setMediaPath(mediaPath);
		}
        else if ( s.compare("-noninteractive") == 0)
        {
                isInteractive = false;
        }
		else
		{
			LOG( "ERROR: Unrecognized command line argument: \"%s\"\n", s.c_str() );
		}
	}

#ifndef SB_NO_PYTHON
	// initialize python
	LOG("Initializing Pyton with libraries at location: %s", python_lib_path.c_str());
	initPython(python_lib_path);
#endif

	mcu_register_callbacks();

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->setCharacterListener(&fltkListener);

	if (sleepFPS != -1.f)
		scene->getSimulationManager()->setSleepFps( sleepFPS) ;
	if (intervalAmount != -1.f)
		scene->getSimulationManager()->printPerf(intervalAmount);

	scene->getSimulationManager()->setupTimer();

	scene->getSimulationManager()->setupProfiler();

	if( lock_dt_mode )	{ 
		scene->getSimulationManager()->setSleepLock();
	}

	scene->setMediaPath(mediaPath);

	SmartBody::SBScene::getScene()->getSpeechManager()->festivalRelay()->initSpeechRelay(festivalLibDir,festivalCacheDir);
	SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->initSpeechRelay(cereprocLibDir,festivalCacheDir);

#if LINK_VHMSG_CLIENT
	char * vhmsg_server = getenv( "VHMSG_SERVER" );
	char * vhmsg_port = getenv("VHMSG_PORT");
	bool vhmsg_disabled = ( vhmsg_server != NULL && strcasecmp( vhmsg_server, "none" ) == 0 );  // hope there is no valid server named "none"
	std::string vhmsgServerStr = "";
	if (vhmsg_server)
		vhmsgServerStr = vhmsg_server;
	std::string vhmsgPortStr = "";
	if (vhmsg_port)
		vhmsgPortStr = vhmsg_port;


	SmartBody::SBVHMsgManager* vhmsgManager = SmartBody::SBScene::getScene()->getVHMsgManager();
	if( !vhmsg_disabled)
	{
		if (vhmsgServerStr != "")
			vhmsgManager->setServer(vhmsgServerStr);
		if (vhmsgPortStr != "")
			vhmsgManager->setPort(vhmsgPortStr);
		
		bool success = vhmsgManager->connect();
		if (success)
		{
			vhmsgManager->setEnable(true);
		}
		else
		{
			LOG("Could not connect to server %s, VHMSG service not enabled.", vhmsg_server);
		}
	}
	else
	{
		if( vhmsg_disabled )
		{
			LOG( "SmartBody: VHMSG_SERVER='%s': Messaging disabled.\n", vhmsg_server?"NULL":vhmsg_server );
		} else {
#if 0 // disable server name query until vhmsg is fixed
			std::string vhserver = (vhmsg_server? vhmsg_server : "localhost");
			std::string vhport = (vhmsg_port ? vhmsg_port : "61616");
			LOG( "SmartBody Error: ttu_open FAILED\n" );
			LOG("Could not connect to %s:%s", vhserver.c_str(), vhport.c_str());
#endif
		}
		vhmsgManager->setEnable(false);
	}
#endif

	// Sets up the network connection for sending bone rotations over to the renderer
	
	if( net_host != "" )
	{
		SmartBody::SBScene::getScene()->getBoneBusManager()->setHost(net_host);
		SmartBody::SBScene::getScene()->getBoneBusManager()->setEnable(true);
	}

	if( proc_id != "" )
	{
		SmartBody::SBScene::getScene()->setProcessId( proc_id );

		// Using a process id is a sign that we're running in a multiple SBM environment.
		// So.. ignore BML requests with unknown agents by default
		mcu.bml_processor.set_warn_unknown_agents( false );
	}

	if (SmartBody::SBScene::getScene()->getBoolAttribute("internalAudio"))
	{
		if ( !AUDIO_Init() )
		{
			LOG( "ERROR: Audio initialization failed\n" );
		}
	}

	SmartBody::SBScene::getScene()->getDebuggerServer()->SetID("sbm-fltk");

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
	//commandline.set_callback( cmdl_tab_callback );

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
		SmartBody::SBScene::getScene()->command( (char *) strstr.str().c_str() );
	}

	for( it = seq_paths.begin();
	     it != seq_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr << "path seq " << (it->c_str());
		SmartBody::SBScene::getScene()->command( (char *) strstr.str().c_str() );
	}

	for( it = audio_paths.begin();
	     it != audio_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr <<  "path audio " << it->c_str();
		SmartBody::SBScene::getScene()->command( (char *) strstr.str().c_str() );
	}


	for( it = py_paths.begin();
	     it != py_paths.end();
		 ++it )
	{
		std::stringstream strstr;
		strstr << "scene.addAssetPath('script', '" << it->c_str() << "')";
		SmartBody::SBScene::getScene()->run( (char *) strstr.str().c_str() );
	}

	// run the specified scripts
	if( init_seqs.empty() && init_pys.empty())
	{
		LOG( "No Python scripts specified. Loading default configuration.'\n" );
		SmartBody::SBScene::getScene()->run("getViewer().show()\ngetCamera().reset()");
	}

	for( it = init_seqs.begin();
		 it != init_seqs.end();
		 ++it )
	{
		string seq_command = "seq " + (*it) + " begin";
		SmartBody::SBScene::getScene()->command((char *)seq_command.c_str());
	}


	for( it = init_pys.begin();
		 it != init_pys.end();
		 ++it )
	{
		std::string cmd = it->c_str();
		std::stringstream strstr;
		strstr << "scene.run(\"" << cmd.c_str() << "\")";
		SmartBody::SBScene::getScene()->run(strstr.str().c_str());
	}

	me_paths.clear();
	seq_paths.clear();
	init_seqs.clear();

	// Notify world SBM is ready to receive messages
	srArgBuffer argBuff("");
	mcu_vrAllCall_func( argBuff, SmartBody::SBScene::getScene()->getCommandManager() );

	scene->getSimulationManager()->start();

#if ENABLE_808_TEST
	return( 0 );
#endif

#if EARLY_EXIT
	mcu.loop = 0;
#endif
//	commandline.render_prompt( "> " );

	std::string pythonPrompt = "# ";
	std::string commandPrompt = "> ";

	while((SmartBody::SBScene::getScene()->getSimulationManager()->isRunning()))	{


		SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
		scene->getSimulationManager()->updateProfiler();
//		mcu.update_profiler( SBM_get_real_time() );
		bool update_sim = scene->getSimulationManager()->updateTimer();
//		bool update_sim = mcu.update_timer( SBM_get_real_time() );

	//	mcu.mark( "main", 0, "fltk-check" );
		Fl::check();

		scene = SmartBody::SBScene::getScene();

#if LINK_VHMSG_CLIENT
		if (SmartBody::SBScene::getScene()->getVHMsgManager()->isEnable())
		{
			err = vhmsg::ttu_poll();
			if( err == vhmsg::TTU_ERROR )	{
				fprintf( stderr, "ttu_poll ERROR\n" );
			}
		}
#endif

		vector<string> commands;// = mcu.bonebus.GetCommand();
		for ( size_t i = 0; i < commands.size(); i++ ) {
			scene->command( (char *)commands[i].c_str() );
		}

		if (isInteractive)
		{
			bool hasCommands = false;
			hasCommands =  commandline.pending( pythonPrompt );

			if ( hasCommands )
			{
				std::string cmd_str = commandline.read();
				char *cmd = (char*)cmd_str.c_str();

				if( strlen( cmd ) )	{

					int result = CMD_FAILURE;
					result = SmartBody::SBScene::getScene()->run(cmd);

					switch( result ) {
						case CMD_NOT_FOUND:
							printf("SmartBody Error: command NOT FOUND: '%s'\n", cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
						case CMD_FAILURE:
							printf("SmartBody Error: command FAILED: '%s'\n", cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
						case CMD_SUCCESS:						
							break;
						default:
							printf("SmartBody Error: return value %d ERROR: '%s'\n", result, cmd );
							fprintf( stdout, "> " ); fflush( stdout );
							break;
					}
				}
			}
		}
#if USE_WSP
		SmartBody::SBWSPManager* wspManager = SmartBody::SBScene::getScene()->getWSPManager();
		if (wspManager->isEnable())
			wspManager->broadcastUpdate();
#endif

		if( update_sim )	{
			scene->update();
		}

		/*for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			 iter != mcu.getCharacterMap().end();
			 iter++)
		{
			SbmCharacter* character = (*iter).second;
			if (character->scene_p)
				character->scene_p->update();	
		}*/
		std::vector<std::string> pawns = SmartBody::SBScene::getScene()->getPawnNames();
		for (std::vector<std::string>::iterator pawnIter = pawns.begin();
			pawnIter != pawns.end();
			pawnIter++)
		{
			SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
 			if (pawn->scene_p)
 				pawn->scene_p->update();	
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
			SrCamera* activeCamera = scene->getActiveCamera();
			activeCamera->setEye(cameraLoc.x, cameraLoc.y, cameraLoc.z);
			SrVec targetLoc = cameraLoc - mcu.cameraTracking[x]->targetToCamera;
			activeCamera->setCenter(targetLoc.x, targetLoc.y, targetLoc.z);
		}	


		BaseWindow* rootWindow = dynamic_cast<BaseWindow*>(mcu.viewer_p);

		if(rootWindow && rootWindow->dataViewerWindow && rootWindow->dataViewerWindow->shown())
		{
			rootWindow->dataViewerWindow->update();
		}

		if(rootWindow && rootWindow->resourceWindow && rootWindow->resourceWindow->shown())
		{
			//rootWindow->resourceWindow->update();
		}

		if (rootWindow && rootWindow->panimationWindow && rootWindow->panimationWindow->shown())
		{
			 rootWindow->panimationWindow->update_viewer();
		}
		if (rootWindow && rootWindow->visemeViewerWindow && rootWindow->visemeViewerWindow->shown())
		{
			rootWindow->visemeViewerWindow->update();
		}

		mcu.render();
	
	}	
	cleanup();
	//vhcl::Log::g_log.RemoveAllListeners();
	//delete listener;
//	delete sbmWindow;

//	return( 0 ); // NOT NEEDED ??
}

