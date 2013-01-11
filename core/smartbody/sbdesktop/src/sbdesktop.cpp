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

#if USE_WSP
#include "wsp.h"
#endif
#include <sb/SBScene.h>
#include <sbm/sbm_constants.h>
#include <sbm/xercesc_utils.hpp>
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include BML_PROCESSOR_INCLUDE
#include <sbm/remote_speech.h>
#include <sbm/sbm_audio.h>
#include <sbm/sbm_speech_audiofile.hpp>
#include <sbm/text_speech.h> // [BMLR]
#include <sbm/locomotion_cmds.hpp>
#include <sbm/resource_cmds.h>
#include <sbm/time_regulator.h>
//#include "SBMWindow.h"
#include <sb/SBPython.h>
#include <sb/SBSteerManager.h>
#include <sbm/PPRAISteeringAgent.h>
#include "TransparentListener.h"
#include "TransparentViewer.h"
#include "sb/SBDebuggerServer.h"
#include "sbm/GPU/SbmShader.h"

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
#include <tchar.h>
#include <mmsystem.h>
#if SBM_REPORT_MEMORY_LEAKS
#include <malloc.h>
#include <crtdbg.h>
#endif
#endif

#if SBM_EMAIL_CRASH_REPORTS
#include <vhcl_crash.h>
#endif

//#include <FL/Fl_glut.h>
#include "sr/sr_model.h"

#define ENABLE_DEFAULT_BOOTSTRAP	(1) 
//#define DEFAULT_SEQUENCE_FILE		("ELITE-all.seq")
#define DEFAULT_SEQUENCE_FILE		("default.seq")
#define DEFAULT_PY_FILE				("default.py")

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


class TransparentViewerFactory : public SrViewerFactory
 {
	public:
		TransparentViewerFactory();

		//void setFltkViewer(FltkViewer* viewer);

		virtual SrViewer* create(int x, int y, int w, int h);
		virtual void remove(SrViewer* viewer);
		virtual void reset(SrViewer* viewer);

	private:
		static SrViewer* s_viewer;

 };

SrViewer* TransparentViewerFactory::s_viewer = NULL;

TransparentViewerFactory::TransparentViewerFactory()
{
	s_viewer = NULL;
}

SrViewer* TransparentViewerFactory::create(int x, int y, int w, int h)
{
	if (!s_viewer)
		s_viewer =new TransparentViewer(x, y, w, h, "SmartBody");
	return s_viewer;
}

void TransparentViewerFactory::remove(SrViewer* viewer)
{
	if (viewer && (viewer == s_viewer))
	{
		viewer->hide_viewer();
	}
}

void TransparentViewerFactory::reset(SrViewer* viewer)
{
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

	mcuCBHandle::singleton()._scene->getDebuggerServer()->ProcessVHMsgs(op, args);
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

void mcu_register_callbacks( void ) {
	
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// additional commands associated with this viewer
	mcu.insert( "q",			mcu_quit_func );
	mcu.insert( "quit",			mcu_quit_func );
//	mcu.insert( "snapshot",		mcu_snapshot_func );
	mcu.insert( "viewer",		mcu_viewer_func );
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
		mcu.camera_p = NULL;
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
int WINAPI _tWinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str,int nWinMode)
{
//int main( int argc, char **argv )	{

	TCHAR cmdline[4096] ;
    TCHAR* argv[4096] ;
    int argc = 0 ;
    _tcscpy( cmdline, GetCommandLine() ) ;
    argv[argc] = _tcstok( cmdline, TEXT(" \t") ) ;
    while( argv[argc] != 0 )
    {
        argc++ ;
        argv[argc] = _tcstok( 0, TEXT(" \t") ) ;
    }

	int w = 320;
	int h = 240;
	int x = 200;
	int y = 150;


#if SBM_EMAIL_CRASH_REPORTS
	// Log crashes, with a dialog option to report via email
	vhcl::Crash::EnableExceptionHandling( true );
	vhcl::Crash::AddCrashCallback( new SBMCrashCallback() );
#endif

	// register the log listener
	vhcl::Log::StdoutListener* listener = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(listener);

	vhcl::Log::FileListener* fileListener = new vhcl::Log::FileListener("smartbody.log");
	vhcl::Log::g_log.AddListener(fileListener);

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

	TransparentListener transparentListener;
	SmartBody::SBScene::getScene()->setCharacterListener(&transparentListener);
	
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
		else if( s.search(	"-host=" ) == 0 )  // argument starts with -host=
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
					}
					else
					{
						LOG( "    Loading sequence '%s'\n", argv[i] );
						init_seqs.push_back( argv[i] );
						mcu.use_python = false;
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
			LOG("-facebone option has been deprecated.");
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
		else if ( s.search ("-x") == 0)
        {
			i++;
			x = atoi(argv[i]);
               
        }
		else if ( s.search ("-y") == 0)
        {
			i++;
			y = atoi(argv[i]);
               
        }
		else if ( s.search ("-w") == 0)
        {
            i++;
			w = atoi(argv[i]);
        }
		else if ( s.search ("-h") == 0)
        {
			i++;
			h = atoi(argv[i]);    
        }

		else
		{
			LOG( "ERROR: Unrecognized command line argument: \"%s\"\n", (const char*)s );
		}
	}
	if( lock_dt_mode )	{ 
		timer.set_sleep_lock();
	}

	TransparentViewerFactory* viewerFactory = new TransparentViewerFactory();
	mcu.register_viewer_factory(viewerFactory);
	TransparentViewer* viewer = dynamic_cast<TransparentViewer*>(viewerFactory->create(x, y, w, h));
	SbmShaderManager::singleton().setViewer(viewer);
	viewer->init(hThisInst, hPrevInst, str, nWinMode);
	viewer->resizeViewer();
	viewer->root( mcu.root_group_p );
	mcu.camera_p = viewer->get_camera();
	mcu.viewer_p = viewer;

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
		err = vhmsg::ttu_register( "sb" );
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
		err = vhmsg::ttu_register( "vrBCFeedback" );
		err = vhmsg::ttu_register( "vrSpeech" );

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

	mcu._scene->getDebuggerServer()->SetID("sbdesktop");


#ifdef WIN32
	(void)signal( SIGBREAK, signal_handler );
#endif

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
	std::string pythonPrompt = "# ";
	std::string commandPrompt = "> ";

	while( mcu.loop )	{

		mcu.update_profiler();
		bool update_sim = mcu.update_timer();

		MSG msg;

		 while (PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		 {
			if (msg.message == WM_QUIT)
			{
				mcu.loop = false;
				break;
			}
            if (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
		 }

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

#if USE_WSP
		mcu.theWSP->broadcast_update();
#endif

		if( update_sim )	{
			mcu.update();
		}

		for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
			 iter != mcu.getPawnMap().end();
			 iter++)
		{
			SbmPawn* pawn = (*iter).second;
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
			mcu.camera_p->setEye(cameraLoc.x, cameraLoc.y, cameraLoc.z);
			SrVec targetLoc = cameraLoc - mcu.cameraTracking[x]->targetToCamera;
			mcu.camera_p->setCenter( targetLoc.x, targetLoc.y, targetLoc.z);
			//mcu.viewer_p->set_camera( mcu.camera_p );
		}	

		mcu.render();
	
	}	
	cleanup();

}

