
#include "vhcl.h"

#include "SBScene.h"
#ifdef WIN32
#include <direct.h>
#endif

#include <sb/SBObject.h>
#include <sb/SBCharacter.h>
#include <sb/SBMotion.h>
#include <sb/SBScript.h>
#include <sb/SBEvent.h>
#include <sb/SBPhoneme.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationState.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBReach.h>
#include <sb/SBReachManager.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBSteerManager.h>
#include <sb/SBServiceManager.h>
#include <sb/SBService.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBRetargetManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBSpeechManager.h>
#include <sb/SBCommandManager.h>
#include <sb/SBWSPManager.h>
#include <sb/SBSkeleton.h>
#include <sb/SBParser.h>
#include <sb/SBDebuggerServer.h>
#include <sb/SBDebuggerClient.h>
#include <sb/SBDebuggerUtility.h>
#include <sb/SBVHMsgManager.h>
#include <sbm/sbm_audio.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <sb/nvbg.h>
#include <sb/SBJointMap.h>
#include <sb/SBCharacterListener.h>
#include <sb/SBNavigationMesh.h>
#include <sbm/ParserBVH.h>
#include <sbm/ParserOpenCOLLADA.h>
#include <sbm/ParserOgre.h>
#include <sbm/Heightfield.h>
#include <sbm/action_unit.hpp>
#include <sbm/xercesc_utils.hpp>
#include <sr/sr_camera.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_ct_eyelid.h>
#include <controllers/me_ct_breathing.h>
#include <controllers/me_ct_example_body_reach.hpp>
#include <controllers/me_ct_saccade.h>
#include <sbm/KinectProcessor.h>
#include <controllers/me_controller_tree_root.hpp>
#include <sr/sr_sn_group.h>
#if !defined(SBM_IPHONE)
#include <sbm/GPU/SbmShader.h>
#endif
#include <sbm/KinectProcessor.h>
#include <sr/sr_sn_group.h>

#ifndef WIN32
#define _stricmp strcasecmp
#endif

#define SHOW_DEPRECATION_MESSAGES 0
namespace SmartBody {

SBScene* SBScene::_scene = NULL;
bool SBScene::_firstTime = true;

std::map<std::string, std::string> SBScene::_systemParameters;


class ForwardLogListener : public vhcl::Log::Listener
{
    public:
		ForwardLogListener() {}
		virtual ~ForwardLogListener() {}

        virtual void OnMessage( const std::string & message )
		{
			SBScene* scene = SmartBody::SBScene::getScene();
			if (!scene)
				return;
			SmartBody::SBCharacterListener* listener = scene->getCharacterListener();
			if (listener)
				listener->OnLogMessage(message);
		}
};


SBScene::SBScene(void) : SBObject()
{
	initialize();
}

void SBScene::initialize()
{
#ifndef SB_NO_PYTHON
#ifndef __native_client__
//	_mainModule = NULL;
//	_mainDict = NULL;
#endif
#endif
	_processId = "";

	createDefaultControllers();

	_characterListener = NULL;

	_sim = new SBSimulationManager();
	_profiler = new SBProfiler();
	_bml = new SBBmlProcessor();
	_blendManager = new SBAnimationBlendManager();
	_reachManager = new SBReachManager();
	_steerManager = new SBSteerManager();
	_serviceManager = new SBServiceManager();
	_physicsManager = new SBPhysicsManager();
	_gestureMapManager = new SBGestureMapManager();
	_jointMapManager = new SBJointMapManager();
	_boneBusManager = new SBBoneBusManager();
	_collisionManager = new SBCollisionManager();
	_diphoneManager = new SBDiphoneManager();
	_behaviorSetManager = new SBBehaviorSetManager();
	_retargetManager = new SBRetargetManager();
	_eventManager = new SBEventManager();
	_assetManager = new SBAssetManager();
	_speechManager = new SBSpeechManager();
	_vhmsgManager = new SBVHMsgManager();
	_commandManager = new SBCommandManager();
	_wspManager = new SBWSPManager();

	_scale = .01f; // default scale is centimeters

	// add the services
	_serviceManager->addService(_steerManager);
	_serviceManager->addService(_physicsManager);
	_serviceManager->addService(_boneBusManager);
	_serviceManager->addService(_collisionManager);
	_serviceManager->addService(_vhmsgManager);
	_serviceManager->addService(_wspManager);

	_parser = new SBParser();

	_debuggerServer = new SBDebuggerServer();
	_debuggerClient = new SBDebuggerClient();
	_debuggerUtility = new SBDebuggerUtility();
	_isRemoteMode = false;

	createBoolAttribute("internalAudio",false,true,"",10,false,false,false,"Use SmartBody's internal audio player.");
	createStringAttribute("speechRelaySoundCacheDir","../../../..",true,"",20,false,false,false,"Directory where sound files from speech relays will be placed. ");
	createDoubleAttribute("scale",.01,true,"",30,false,false,false,"The scale of scene (1 = meters, .01 = centimeters, etc).");
	createIntAttribute("colladaTrimFrames",0,true,"",40,false,false,false,"Number of frames to be trimmed in the front when loading a collada motion.");
	createBoolAttribute("useFastXMLParsing",false,true,"",50,false,false,false,"Use faster parsing when reading XML from a file.");
	createBoolAttribute("delaySpeechIfNeeded",true,true,"",60,false,false,false,"Delays any speech until other behaviors specified in the same BML need to execute beforehand. This can occur when a gesture is synchronized to a word early in the utterance, and the gesture motion needs to be played for awhile before the synch point.");
	createBoolAttribute("useXMLCache",false,true,"",500,false,false,false,"Cache the XML used when processing audio files.");
	createBoolAttribute("useXMLCacheAuto",false,true,"",510,false,false,false,"Automatically add the XML to the cache when processing audio files after playing for the first time.");
	createStringAttribute("defaultCharacter","",true,"",550,false,false,false,"Default character when processing BML.");
	createStringAttribute("defaultRecipient","ALL",true,"",550,false,false,false,"Default recipient when processing BML.");
	createIntAttribute("queuedCommandsIndex",1,true,"",560,false,false,false,"Unique identifier when executing sequence commands.");
	createIntAttribute("bmlIndex",1,true,"",560,false,false,false,"Unique identifier when executing BML commands.");
	BoolAttribute* consoleAttr = createBoolAttribute("enableConsoleLogging",false,true,"",70,false,false,false,"Use SmartBody's internal audio player.");
	
	vhcl::Log::g_log.RemoveAllListeners();
	ForwardLogListener* forwardListener = new ForwardLogListener();
	vhcl::Log::g_log.AddListener(forwardListener);

	//consoleAttr->setValue(true); // set up the console logging
	
	_mediaPath = ".";
		// re-initialize
	// initialize everything

	_viewer = NULL;
	_ogreViewer = NULL;
	_viewerFactory = NULL;
	_ogreViewerFactory = NULL;
	
	_rootGroup = new SrSnGroup();
	_rootGroup->ref();

	_heightField = NULL;
	_navigationMesh = NULL;
	_kinectProcessor = new KinectProcessor();

	// Create default settings
	createDefaultControllers();
	
	SmartBody::SBCharacterListener* listener = getCharacterListener();
	//_scene = SmartBody::SBScene::getScene();
	setCharacterListener(listener);

	_debuggerServer->Init();
	_debuggerServer->SetSBScene(_scene);
	SmartBody::SBAnimationBlend0D* idleState = getBlendManager()->createBlend0D(PseudoIdleState);

	// reset timer & viewer window
	getSimulationManager()->reset();
//	getSimulationManager()->start();

	/*
#ifndef __native_client__
	SrViewer* viewer = SmartBody::getViewer();
	if (viewer)
		viewer->show_viewer();
#endif

	command("vhmsgconnect");
#ifndef __native_client__
	//Py_Finalize();
	//initPython(initPythonLibPath);
#ifndef SB_NO_PYTHON
	PyRun_SimpleString("scene = getScene()");
	PyRun_SimpleString("bml = scene.getBmlProcessor()");
	PyRun_SimpleString("sim = scene.getSimulationManager()");
#endif
#endif
	*/	
	if (_viewer)	
	{
		if (_viewerFactory)
			_viewerFactory->reset(_viewer);
		_viewer = NULL;
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}

	if (_viewerFactory)
		_viewerFactory->remove(_ogreViewer);
	if (_ogreViewer)
	{
		delete _ogreViewer;
		_ogreViewer = NULL;
	}

	_logListener = NULL;


}

void SBScene::cleanup()
{
	// stop the simulation
	getSimulationManager()->stop();
	
	// reset the simulation parameters
	getSimulationManager()->setSimFps(0);

	// remove the characters
	removeAllCharacters();
	
	// remove the pawns
	removeAllPawns();

	// clear the joint maps
	getJointMapManager()->removeAllJointMaps();

	// remove all blends and transitions
	getBlendManager()->removeAllBlends();
	getBlendManager()->removeAllTransitions();

	// always need a PseudoIdle state
	//SmartBody::SBAnimationBlend0D* idleState = getBlendManager()->createBlend0D(PseudoIdleState);
	//addPABlend(idleState);


	// clear out the default face definitions
	std::vector<std::string> faceDefinitions = getFaceDefinitionNames();

	for (std::vector<std::string>::iterator iter = faceDefinitions.begin();
		 iter != faceDefinitions.end();
		 iter++)
	{
		std::string faceName = (*iter);
		removeFaceDefinition(faceName);
	}

	// stop the services
	SBServiceManager* serviceManager = getServiceManager();
	std::vector<std::string> serviceNames =  serviceManager->getServiceNames();
	for (std::vector<std::string>::iterator iter = serviceNames.begin();
		 iter != serviceNames.end();
		 iter++)
	{
		SBService* service = serviceManager->getService(*iter);
		service->stop();
	}

	removePendingCommands();

	clearAttributes();

	removeDefaultControllers();

	removeAllAssetPaths("script");
	removeAllAssetPaths("motion");
	removeAllAssetPaths("mesh");
	removeAllAssetPaths("audio");


	delete _sim;
	delete _profiler;
	delete _bml;
	delete _blendManager;
	delete _reachManager;
	delete _steerManager;
	delete _serviceManager;
	delete _physicsManager;
	delete _gestureMapManager;
	delete _jointMapManager;
	delete _boneBusManager;
	delete _collisionManager;
	delete _diphoneManager;
	delete _behaviorSetManager;
	delete _retargetManager;
	delete _eventManager;
	delete _assetManager;
	delete _speechManager;
	delete _commandManager;
	delete _wspManager;

	delete _kinectProcessor;

	_sim = NULL;
	_profiler = NULL;
	_bml = NULL;
	_blendManager = NULL;
	_reachManager = NULL;
	_steerManager= NULL;
	_serviceManager = NULL;
	_physicsManager = NULL;
	_gestureMapManager= NULL;
	_jointMapManager = NULL;
	_boneBusManager = NULL;
	_collisionManager = NULL;
	_diphoneManager = NULL;
	_behaviorSetManager = NULL;
	_retargetManager = NULL;
	_eventManager = NULL;
	_assetManager = NULL;
	_commandManager = NULL;
	_speechManager = NULL;
	_wspManager = NULL;

	_kinectProcessor = NULL;

	_cameraTracking.clear();
	
	if (_heightField)
	{
		delete _heightField;
	}
	_heightField = NULL;

	if (_navigationMesh)
	{
		delete _navigationMesh;
	}
	_navigationMesh = NULL;

	_rootGroup->unref();
	_rootGroup = NULL;

	_viewer = NULL;
	_ogreViewer = NULL;
	_viewerFactory = NULL;
	_ogreViewerFactory = NULL;
	
	AUDIO_Close();
	AUDIO_Init();

	if (_vhmsgManager->isEnable())
		_vhmsgManager->send( "vrProcEnd sbm" );
	
	delete _vhmsgManager;	

#ifndef SB_NO_PYTHON
//	Py_Finalize();

#if defined(WIN_BUILD)
	{
		// According to the python docs, .pyd files are not unloaded during Py_Finalize().
		// This causes issues when trying to re-load the smartbody dll over and over.
		// So, we force unload these .pyd files.  This list is all the standard .pyd files included in the Python26 DLLs folder.
		// For reference:  http://docs.python.org/2/c-api/init.html  "Dynamically loaded extension modules loaded by Python are not unloaded"

		// initPythonLibPath - eg:  "../../../../core/smartbody/Python26/Lib"
		std::string pythonLibPath = Py_GetPythonHome();
		HMODULE hmodule;
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/bz2.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/pyexpat.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/select.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/unicodedata.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/winsound.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_bsddb.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes_test.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_elementtree.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_hashlib.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_msi.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_multiprocessing.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_socket.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_sqlite3.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ssl.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_testcapi.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_tkinter.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
	}
#endif  // WIN_BUILD
#endif  // USE_PYTHON
}

SBScene::~SBScene(void)
{
	cleanup();
	for (std::map<std::string, SBScript*>::iterator iter = _scripts.begin();
		 iter != _scripts.end();
		 iter++)
	{
	//	delete (*iter).second;
	}

	delete _sim;
	delete _profiler;
	delete _bml;
	delete _blendManager;
	delete _reachManager;
	delete _steerManager;
	delete _physicsManager;
	delete _boneBusManager;
	delete _collisionManager;
	delete _gestureMapManager;
	delete _jointMapManager;
	delete _diphoneManager;
	delete _behaviorSetManager;
	delete _serviceManager;
	delete _eventManager;

	delete _parser;

	_debuggerClient->Disconnect();
	_debuggerServer->Close();
	delete _debuggerServer;  // TODO: should delete these in reverse order?
	delete _debuggerClient;
	delete _debuggerUtility;

//	
	//mcu.reset();


}

SBDebuggerServer* SBScene::getDebuggerServer()
{
	return _debuggerServer; 
}

SBDebuggerClient* SBScene::getDebuggerClient()
{
	return _debuggerClient; 
}


SBDebuggerUtility* SBScene::getDebuggerUtility()
{
	return _debuggerUtility; 
}

SBScene* SBScene::getScene()
{
	if (_firstTime)
	{
		XMLPlatformUtils::Initialize(); 
		_firstTime = false;
		_scene = new SBScene();
		_scene->initialize();
	}

	return _scene;
}

void SBScene::destroyScene()
{
	if (_scene)
	{
		delete _scene;
		_scene = NULL;
		_firstTime = true;
	}
}

void SBScene::setProcessId(const std::string& id)
{
	_processId = id;
}

const std::string& SBScene::getProcessId()
{
	return _processId;
}

void SBScene::update()
{
	
	// remote mode
	if (isRemoteMode())
	{
		getDebuggerClient()->Update();
		const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
		for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
			pawnIter != pawns.end();
			pawnIter++)
		{
			SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
			pawn->ct_tree_p->evaluate(getSimulationManager()->getTime());
			pawn->ct_tree_p->applySkeletonToBuffer();
		}

		return;
	}

	// scripts
	std::map<std::string, SmartBody::SBScript*>& scripts = getScripts();
	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->beforeUpdate(getSimulationManager()->getTime());
	}

	// services
	std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->beforeUpdate(getSimulationManager()->getTime());
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
	for (int s = 0; s < getCommandManager()->getActiveSequences()->getNumSequences(); s++)
	{
		srCmdSeq* seq = getCommandManager()->getActiveSequences()->getSequence(s, seqName);
		char *cmd;
		while( cmd = seq->pop( (float) getSimulationManager()->getTime() ) )
		{
			int err = getCommandManager()->execute( cmd );
			if( err != CMD_SUCCESS )	{
				LOG( "update ERR: execute FAILED: '%s'\n", cmd );
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
		getCommandManager()->getActiveSequences()->removeSequence(sequencesToDelete[d], true);
	}

	bool isClosingBoneBus = false;
	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		pawn->reset_all_channels();
		pawn->ct_tree_p->evaluate( getSimulationManager()->getTime() );
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
			if (getBoneBusManager()->isEnable())
			{
				if (pawn->bonebusCharacter && pawn->bonebusCharacter->GetNumErrors() > 3)
				{
					// connection is bad, remove the bonebus character 
					LOG("BoneBus cannot connect to server. Removing pawn %s", pawn->getName().c_str());
					bool success = getBoneBusManager()->getBoneBus().DeleteCharacter(pawn->bonebusCharacter);
					char_p->bonebusCharacter = NULL;
					isClosingBoneBus = true;
					if (getBoneBusManager()->getBoneBus().GetNumCharacters() == 0)
					{
						getBoneBusManager()->getBoneBus().CloseConnection();
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
				brain->update(sbchar, getSimulationManager()->getTime(), getSimulationManager()->getTimeDt());
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

			char_p->forward_visemes( getSimulationManager()->getTime() );	
			char_p->forward_parameters( getSimulationManager()->getTime() );	

			if (char_p->bonebusCharacter && char_p->bonebusCharacter->GetNumErrors() > 3)
			{
				// connection is bad, remove the bonebus character
				isClosingBoneBus = true;
				LOG("BoneBus cannot connect to server after visemes sent. Removing all characters.");
			}

			if ( getBoneBusManager()->isEnable() && 
				 char_p->getSkeleton() && 
				 char_p->bonebusCharacter)
			{
				getBoneBusManager()->NetworkSendSkeleton( char_p->bonebusCharacter, char_p->getSkeleton(), &getGeneralParameters() );

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

				char_p->bonebusCharacter->SetPosition( x, y, z, getSimulationManager()->getTime() );
				char_p->bonebusCharacter->SetRotation( (float)q.w, (float)q.x, (float)q.y, (float)q.z, getSimulationManager()->getTime() );

				if (char_p->bonebusCharacter->GetNumErrors() > 3)
				{
					// connection is bad, remove the bonebus character 
					isClosingBoneBus = true;
					LOG("BoneBus cannot connect to server. Removing all characters");
				}
			}
			else if (!isClosingBoneBus && !char_p->bonebusCharacter && getBoneBusManager()->getBoneBus().IsOpen())
			{
				// bonebus was connected after character creation, create it now
				char_p->bonebusCharacter = getBoneBusManager()->getBoneBus().CreateCharacter( char_p->getName().c_str(), char_p->getClassType().c_str(), true );
			}
		}  // end of char_p processing
	} // end of loop

	if (isClosingBoneBus)
	{
		const std::vector<std::string>& pawnNames = getPawnNames();
		for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
			iter != pawnNames.end();
			iter++)
		{
			SBPawn* pawn = getPawn(*iter);
			if (pawn->bonebusCharacter)
			{
				bool success = getBoneBusManager()->getBoneBus().DeleteCharacter(pawn->bonebusCharacter);
				pawn->bonebusCharacter = NULL;
			}
		}

		getBoneBusManager()->getBoneBus().CloseConnection();
	}

	const std::vector<std::string>& pawnNames = getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		iter != pawnNames.end();
		iter++)
	{
		SBPawn* pawn = getPawn(*iter);
		pawn->afterUpdate(getSimulationManager()->getTime());
	}

	
	SrCamera* camera = getActiveCamera();
	if (_viewer && camera)
	{
		SrMat m;
		SrQuat quat = SrQuat(camera->get_view_mat(m).get_rotation());

		getDebuggerServer()->m_cameraPos.x = camera->getEye().x;
		getDebuggerServer()->m_cameraPos.y = camera->getEye().y;
		getDebuggerServer()->m_cameraPos.z = camera->getEye().z;
		getDebuggerServer()->m_cameraLookAt.x = camera->getCenter().x;
		getDebuggerServer()->m_cameraLookAt.y = camera->getCenter().y;
		getDebuggerServer()->m_cameraLookAt.z = camera->getCenter().z;
		getDebuggerServer()->m_cameraRot.x = quat.x;
		getDebuggerServer()->m_cameraRot.y = quat.y;
		getDebuggerServer()->m_cameraRot.z = quat.z;
		getDebuggerServer()->m_cameraRot.w = quat.w;
		getDebuggerServer()->m_cameraFovY   = sr_todeg(camera->getFov());
		getDebuggerServer()->m_cameraAspect = camera->getAspectRatio();
		getDebuggerServer()->m_cameraZNear  = camera->getNearPlane();
		getDebuggerServer()->m_cameraZFar   = camera->getFarPlane();
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
		getDebuggerServer()->Update();

	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		(*iter).second->update(getSimulationManager()->getTime());
	}

	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		(*iter).second->update(getSimulationManager()->getTime());
	}

	// scripts
	for (std::map<std::string, SmartBody::SBScript*>::iterator iter = scripts.begin();
		iter != scripts.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->afterUpdate(getSimulationManager()->getTime());
	}

	// services
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		if ((*iter).second->isEnable())
			(*iter).second->afterUpdate(getSimulationManager()->getTime());
	}

}



void SBScene::setScale(float val)
{
	_scale = val;

	DoubleAttribute* scaleAttribute = dynamic_cast<DoubleAttribute*>(getAttribute("scale"));
	scaleAttribute->setValueFast(_scale);
}

float SBScene::getScale()
{
	return _scale;
}

void SBScene::reset()
{
	cleanup();
	initialize();	
}

void SBScene::notify( SBSubject* subject )
{
	BoolAttribute* boolAttr = dynamic_cast<BoolAttribute*>(subject);

	if (boolAttr && boolAttr->getName() == "internalAudio")
	{
		bool val = boolAttr->getValue();
		if (!val)
		{
			AUDIO_Close();
		}
		else
		{
			AUDIO_Init();
		}
		return;
	}
	else if (boolAttr && boolAttr->getName() == "enableConsoleLogging")
	{
		bool val = boolAttr->getValue();
		if (val)
		{
			if (vhcl::Log::g_log.IsEnabled())
				return;

			vhcl::Log::StdoutListener* listener = new vhcl::Log::StdoutListener();
			vhcl::Log::g_log.AddListener(listener);
		}
		else
		{
			vhcl::Log::g_log.RemoveAllListeners();
		}
	}

	DoubleAttribute* doubleAttr = dynamic_cast<DoubleAttribute*>(subject);
	if (doubleAttr && doubleAttr->getName() == "scale")
	{
		setScale((float) doubleAttr->getValue());
		return;
	}
}

SBCharacter* SBScene::createCharacter(const std::string& charName, const std::string& metaInfo)
{	
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(charName);
	if (character)
	{
		LOG("Character '%s' already exists!", charName.c_str());
		return NULL;
	}
	else
	{
		SBCharacter* character = new SBCharacter(charName, metaInfo);
		
		std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.find(character->getName());
		if (iter != _pawnMap.end())
		{
			LOG( "Register character: pawn_map.insert(..) '%s' FAILED\n", character->getName().c_str() );
			delete character;
			return NULL;
		}

		_pawnMap.insert(std::pair<std::string, SbmPawn*>(character->getName(), character));
		_pawnNames.push_back(character->getName());
	
		std::map<std::string, SbmCharacter*>::iterator citer = _characterMap.find(character->getName());
		if (citer != _characterMap.end())
		{
			LOG( "Register character: character_map.insert(..) '%s' FAILED\n", character->getName().c_str() );
			_pawnMap.erase(iter);
			delete character;
			return NULL;
		}
		_characterMap.insert(std::pair<std::string, SbmCharacter*>(character->getName(), character));
		_characterNames.push_back(character->getName());

		if (getCharacterListener() )
			getCharacterListener()->OnCharacterCreate( character->getName().c_str(), character->getClassType() );
		SBSkeleton* skeleton = new SBSkeleton();		
		character->setSkeleton(skeleton);
//		SkJoint* joint = skeleton->add_joint(SkJoint::TypeQuat);
//		joint->setName("world_offset");		
//		joint->update_gmat();

		if (getBoneBusManager()->isEnable())
			getBoneBusManager()->getBoneBus().CreateCharacter( character->getName().c_str(), character->getClassType().c_str(), true );
		if (getCharacterListener() )
			getCharacterListener()->OnCharacterCreate( character->getName().c_str(), character->getClassType() );


		// notify the services		
		std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
		for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
			iter != services.end();
			iter++)
		{
			SBService* service = (*iter).second;
			service->onCharacterCreate(character);
		}		
		return character;
	}
}

SBPawn* SBScene::createPawn(const std::string& pawnName)
{
	SBPawn* pawn = getPawn(pawnName);
	SBCharacter* character = dynamic_cast<SBCharacter*>(pawn);
	if (character)
	{
		LOG("Pawn '%s' is a character.", pawnName.c_str());
		return NULL;
	}
	if (pawn)
	{
		LOG("Pawn '%s' already exists!", pawnName.c_str());
		return NULL;
	}
	else
	{
		SBPawn* pawn = new SBPawn(pawnName.c_str());
		SBSkeleton* skeleton = new SBSkeleton();
		pawn->setSkeleton(skeleton);
		SkJoint* joint = skeleton->add_joint(SkJoint::TypeQuat);
		joint->setName("world_offset");

		std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.find(pawn->getName());
		if (iter != _pawnMap.end())
		{
			LOG( "Register pawn: pawn_map.insert(..) '%s' FAILED\n", pawn->getName().c_str() );
			delete pawn;
			return NULL;
		}

		_pawnMap.insert(std::pair<std::string, SbmPawn*>(pawn->getName(), pawn));
		_pawnNames.push_back(pawn->getName());
	
		if (getCharacterListener())
			getCharacterListener()->OnPawnCreate( pawn->getName().c_str() );


		// notify the services
		std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
		for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
			iter != services.end();
			iter++)
		{
			SBService* service = (*iter).second;
			service->onPawnCreate(character);
		}
		return pawn;
	}
}

void SBScene::removeCharacter(const std::string& charName)
{
	SBCharacter* character = this->getCharacter(charName);
	const std::string& name = character->getName();
	if (character)
	{

		string vrProcEnd_msg = "vrProcEnd sbm ";
		vrProcEnd_msg += getName();
		SmartBody::SBScene::getScene()->getVHMsgManager()->send( vrProcEnd_msg.c_str() );

		// notify the services
		std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
		for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
			iter != services.end();
			iter++)
		{
			SBService* service = (*iter).second;
			service->onCharacterDelete(character);
		}
	

		if (getCharacterListener() )
			getCharacterListener()->OnCharacterDelete( name );

		std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.find(name);
		if (iter != _pawnMap.end())
		{
			_pawnMap.erase(iter);
		}
		for (std::vector<std::string>::iterator iter = _pawnNames.begin();
			 iter != _pawnNames.end();
			 iter++)
		{
			if (name == (*iter))
			{
				_pawnNames.erase(iter);
				break;
			}
		}

		std::map<std::string, SbmCharacter*>::iterator citer = _characterMap.find(name);
		if (citer != _characterMap.end())
		{
			_characterMap.erase(citer);
		}
		for (std::vector<std::string>::iterator iter = _characterNames.begin();
			 iter != _characterNames.end();
			 iter++)
		{
			if (name == (*iter))
			{
				_characterNames.erase(iter);
				break;
			}
		}

		delete character;
	}	
}

void SBScene::removeAllCharacters()
{
	std::vector<std::string> characters = getCharacterNames();
	for (std::vector<std::string>::const_iterator iter = characters.begin();
		 iter != characters.end();
		 iter++)
	{
		removeCharacter((*iter));
	}
	
}

void SBScene::removePawn(const std::string& pawnName)
{
	SbmPawn* pawn = SmartBody::SBScene::getScene()->getPawn(pawnName);
	if (pawn)
	{
		const std::string& name = pawn->getName();

		SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
		if (!character)
		{
			// notify the services
			std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
			for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
				iter != services.end();
				iter++)
			{
				SBService* service = (*iter).second;
				SBPawn* sbpawn = dynamic_cast<SBPawn*>(pawn);
				service->onPawnDelete(sbpawn);
			}
			
			if (getCharacterListener())
				getCharacterListener()->OnPawnDelete( name );

			std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.find(name);
			if (iter != _pawnMap.end())
			{
				_pawnMap.erase(iter);
			}
			for (std::vector<std::string>::iterator iter = _pawnNames.begin();
			 iter != _pawnNames.end();
			 iter++)
			{
				if (name == (*iter))
				{
					_pawnNames.erase(iter);
					break;
				}
			}

			delete pawn;
		}
	}	
}

void SBScene::removeAllPawns()
{
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawns.begin();
		 iter != pawns.end();
		 iter++)
	{
		removePawn((*iter));
	}
	

	// clear the cameras
	_cameras.clear();
}

int SBScene::getNumCharacters() 
{  
	 
	return _characterMap.size(); 
}

int SBScene::getNumPawns() 
{  
	return _pawnMap.size() - _characterMap.size(); 
}

SBPawn* SBScene::getPawn(const std::string& name)
{
	std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.find(name);
	if (iter == _pawnMap.end())
	{
		return NULL;
	}
	else
	{
		SBPawn* sbpawn = dynamic_cast<SBPawn*>((*iter).second);
		return sbpawn;
	}
}

SBCharacter* SBScene::getCharacter(const std::string& name)
{
	std::map<std::string, SbmCharacter*>::iterator iter = _characterMap.find(name);
	if (iter == _characterMap.end())
	{
		return NULL;
	}
	else
	{
		SBCharacter* sbcharacter = dynamic_cast<SBCharacter*>((*iter).second);
		return sbcharacter;
	}
}

const std::vector<std::string>& SBScene::getPawnNames()
{
	return _pawnNames;
}

const std::vector<std::string>& SBScene::getCharacterNames()
{
	return _characterNames;
}

std::vector<std::string> SBScene::getEventHandlerNames()
{
	SBEventManager* eventManager = getEventManager();
	
	std::vector<std::string> ret;

	for(SBEventHandlerMap::iterator iter = eventManager->getEventHandlers().begin();
		iter != eventManager->getEventHandlers().end();
		iter++)
	{

		ret.push_back(std::string(iter->first));
	}
	return ret;
}


void SBScene::setMediaPath(const std::string& path)
{
	_mediaPath = path;
}

const std::string& SBScene::getMediaPath()
{
	return _mediaPath;
}

void SBScene::setDefaultCharacter(const std::string& character)
{
	SmartBody::SBScene::getScene()->setStringAttribute("defaultCharacter", character);
}

void SBScene::setDefaultRecipient(const std::string& recipient)
{
	SmartBody::SBScene::getScene()->setStringAttribute("defaultRecipient", recipient);
}

SBEventManager* SBScene::getEventManager()
{
	return _eventManager;
}


bool SBScene::command(const std::string& command)
{
	int ret = getCommandManager()->execute((char*) command.c_str());

	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
}

bool SBScene::commandAt(float seconds, const std::string& command)
{
	int ret = getCommandManager()->execute_later((char*) command.c_str(), seconds);

	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
}

void SBScene::removePendingCommands()
{
	SmartBody::SBScene::getScene()->getCommandManager()->getActiveSequences()->clear();
	SmartBody::SBScene::getScene()->getCommandManager()->getPendingSequences()->clear();
}

void SBScene::sendVHMsg(const std::string& message)
{
	 
	SmartBody::SBScene::getScene()->getVHMsgManager()->send(message.c_str());
}

void SBScene::sendVHMsg2(const std::string& message, const std::string& message2)
{
	 
	SmartBody::SBScene::getScene()->getVHMsgManager()->send2(message.c_str(), message2.c_str());
}

bool SBScene::run(const std::string& command)
{
#ifndef SB_NO_PYTHON
	try {
		//LOG("executePython = %s",command);

		int result = PyRun_SimpleString(command.c_str());
		//LOG("cmd result = %d",result);

		return true;
	} catch (...) {
		PyErr_Print();
		return false;
	}
#endif
	return true;
}

bool SBScene::runScript(const std::string& script)
{
#ifndef SB_NO_PYTHON
	// add the .seq extension if necessary
	std::string candidateSeqName = script;
	if (candidateSeqName.find(".py") == std::string::npos)
	{
		candidateSeqName.append(".py");
	}

	std::string curFilename = SmartBody::SBScene::getScene()->getAssetManager()->findFileName("script", candidateSeqName);
	if (curFilename != "")
	{
		try {
			std::stringstream strstr;
			strstr << "execfile(\"" << curFilename << "\")";
			PyRun_SimpleString(strstr.str().c_str());
			PyErr_Print();
			PyErr_Clear();
			return true;
		} catch (...) {
			PyErr_Print();
			return false;
		}
	}

	LOG("Could not find Python script '%s'", script.c_str());
	return false;

#endif
	return true;
}

SBSimulationManager* SBScene::getSimulationManager()
{
	return _sim;
}

SBProfiler* SBScene::getProfiler()
{
	return _profiler;
}

SBBmlProcessor* SBScene::getBmlProcessor()
{
	return _bml;
}

SBAnimationBlendManager* SBScene::getBlendManager()
{
	return _blendManager;
}

SBReachManager* SBScene::getReachManager()
{
	return _reachManager;
}

SBSteerManager* SBScene::getSteerManager()
{
	return _steerManager;
}

SBServiceManager* SBScene::getServiceManager()
{
	return _serviceManager;
}


SBCollisionManager* SBScene::getCollisionManager()
{
	return _collisionManager;
}

SBDiphoneManager* SBScene::getDiphoneManager()
{
	return _diphoneManager;
}

SBBehaviorSetManager* SBScene::getBehaviorSetManager()
{
	return _behaviorSetManager;
}

SBRetargetManager* SBScene::getRetargetManager()
{
	return _retargetManager;
}

SBAssetManager* SBScene::getAssetManager()
{
	return _assetManager;
}

SBSpeechManager* SBScene::getSpeechManager()
{
	return _speechManager;
}

SBPhysicsManager* SBScene::getPhysicsManager()
{
	return _physicsManager;
}

SBBoneBusManager* SBScene::getBoneBusManager()
{
	return _boneBusManager;
}

SBGestureMapManager* SBScene::getGestureMapManager()
{
	return _gestureMapManager;
}

SBJointMapManager* SBScene::getJointMapManager()
{
	return _jointMapManager;
}

SBCommandManager* SBScene::getCommandManager()
{
	return _commandManager;
}

SBWSPManager* SBScene::getWSPManager()
{
	return _wspManager;
}


SBVHMsgManager* SBScene::getVHMsgManager()
{
	return _vhmsgManager;
}

SBParser* SBScene::getParser()
{
	return _parser;
}

bool SBScene::isRemoteMode()	
{ 
	return _isRemoteMode; 
}

void SBScene::setRemoteMode(bool val)	
{ 
	_isRemoteMode = val; 
}

SmartBody::SBFaceDefinition* SBScene::createFaceDefinition(const std::string& name)
{
	// make sure the name doesn't already exist
	if (_faceDefinitions.find(name) != _faceDefinitions.end())
	{
		LOG("Face definition named '%s' already exists.", name.c_str());
		return NULL;
	}

	SBFaceDefinition* face = new SBFaceDefinition(name);
	_faceDefinitions.insert(std::pair<std::string, SBFaceDefinition*>(name, face));

	return face;
}

void SBScene::removeFaceDefinition(const std::string& name)
{
	

	// make sure the name doesn't already exist
	std::map<std::string, SBFaceDefinition*>::iterator iter = _faceDefinitions.find(name);
	if (iter ==_faceDefinitions.end())
	{
		LOG("Face definition named '%s' does not exist.", name.c_str());
		return;
	}

	delete iter->second;
	iter->second = NULL;
	_faceDefinitions.erase(iter);
}

SmartBody::SBFaceDefinition* SBScene::getFaceDefinition(const std::string& name)
{
	// make sure the name doesn't already exist
	std::map<std::string, SBFaceDefinition*>::iterator iter = _faceDefinitions.find(name);
	if (iter == _faceDefinitions.end())
	{
		LOG("Face definition named '%s' does not exist.", name.c_str());
		return NULL;
	}

	return (*iter).second;
}

int SBScene::getNumFaceDefinitions()
{
	return 	_faceDefinitions.size();
}

std::vector<std::string> SBScene::getFaceDefinitionNames()
{
	std::vector<std::string> faces;
	for (std::map<std::string, SBFaceDefinition*>::iterator iter =  _faceDefinitions.begin();
		 iter !=  _faceDefinitions.end();
		 iter++)
	{
		faces.push_back((*iter).second->getName());
	}

	return faces;
}

void SBScene::addScript(const std::string& name, SBScript* script)
{
	std::map<std::string, SBScript*>::iterator iter = _scripts.find(name);
	if (iter != _scripts.end())
	{
		LOG("Script with name %s already exists.", name.c_str());
		return;
	}

	_scripts.insert(std::pair<std::string, SBScript*>(name, script));
}

void SBScene::removeScript(const std::string& name)
{
	std::map<std::string, SBScript*>::iterator iter = _scripts.find(name);
	if (iter != _scripts.end())
	{
		_scripts.erase(iter);
		return;
	}
	LOG("Script with name %s does not exist.", name.c_str());

}

int SBScene::getNumScripts()
{
	return _scripts.size();
}

std::vector<std::string> SBScene::getScriptNames()
{
	std::vector<std::string> scriptNames;

	for (std::map<std::string, SBScript*>::iterator iter = _scripts.begin();
		 iter != _scripts.end();
		 iter++)
	{
		scriptNames.push_back((*iter).first);
	}

	return scriptNames;

}

SBScript* SBScene::getScript(const std::string& name)
{
	std::map<std::string, SBScript*>::iterator iter = _scripts.find(name);
	if (iter == _scripts.end())
	{
		LOG("Script with name %s already exists.", name.c_str());
		return NULL;
	}

	return (*iter).second;
}

std::map<std::string, SBScript*>& SBScene::getScripts()
{
	return _scripts;
}

void SBScene::setCharacterListener(SBCharacterListener* listener)
{
	_characterListener = listener;
}

SBCharacterListener* SBScene::getCharacterListener()
{
	return _characterListener;
}


std::string SBScene::saveSceneSetting()
{
	std::stringstream strstr;
	strstr << "# Autogenerated by SmartBody\n";
	// save all default cameras
	std::vector<std::string> cameras = getCameraNames();
	for (std::vector<std::string>::iterator cameraIter = cameras.begin();
		cameraIter != cameras.end();
		cameraIter++)
	{
		if (*cameraIter == "cameraDefault")
			continue; // don't save default camera
		SrCamera* camera = getCamera((*cameraIter));
		strstr << "obj = scene.getCamera(\"" << camera->getName() << "\")\n";
		strstr << "if obj == None:\n";
		strstr << "\tobj = scene.createCamera(\"" << camera->getName() << "\")\n";		
		strstr << "obj.setEye(" << camera->getEye().x << ", " << camera->getEye().y << ", " << camera->getEye().z << ")\n";
		strstr << "obj.setCenter(" << camera->getCenter().x << ", " << camera->getCenter().y << ", " << camera->getCenter().z << ")\n";
		strstr << "obj.setUpVector(SrVec(" << camera->getUpVector().x << ", " << camera->getUpVector().y << ", " << camera->getUpVector().z << "))\n";
		strstr << "obj.setScale(" << camera->getScale() << ")\n";
		strstr << "obj.setFov(" << camera->getFov() << ")\n";
		strstr << "obj.setFarPlane(" << camera->getFarPlane() << ")\n";
		strstr << "obj.setNearPlane(" << camera->getNearPlane() << ")\n";
		strstr << "obj.setAspectRatio(" << camera->getAspectRatio() << ")\n";

		std::vector<std::string> attributeNames = camera->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			iter != attributeNames.end();
			iter++)
		{
			SmartBody::SBAttribute* attr = camera->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}
	}	

	// save all pawns (including light pawns) position & orientation.
	const std::vector<std::string>& pawns = getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SBPawn* pawn = getPawn((*pawnIter));
		SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
		if (camera)
			continue; // already wrote out pawns
		strstr << "\n# ---- pawn: " << pawn->getName() << "\n";
		strstr << "obj = scene.getPawn(\"" << pawn->getName() << "\")\n";
		strstr << "if obj == None:\n";
		strstr << "\tobj = scene.createPawn(\"" << pawn->getName() << "\")\n";
		SrVec position = pawn->getPosition();
		strstr << "obj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		SrVec hpr = pawn->getHPR();
		strstr << "obj.setHPR(SrVec(" << hpr[0] << ", " << hpr[1] << ", " << hpr[2] << "))\n";
		// attributes
		std::vector<std::string> attributeNames = pawn->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			iter != attributeNames.end();
			iter++)
		{
			SmartBody::SBAttribute* attr = pawn->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}
	}

	// restore all character position/orientation
	const std::vector<std::string>& characters = getCharacterNames();
	for (std::vector<std::string>::const_iterator characterIter = characters.begin();
		characterIter != characters.end();
		characterIter++)
	{
		SBCharacter* character = getCharacter((*characterIter));
		strstr << "\n# ---- character: " << character->getName() << "\n";
		strstr << "obj = scene.getCharacter(\"" << character->getName() << "\")\n";
		strstr << "if obj != None:\n";				
		SrVec position = character->getPosition();
		strstr << "\tobj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		SrVec hpr = character->getHPR();
		strstr << "\tobj.setHPR(SrVec(" << hpr[0] << ", " << hpr[1] << ", " << hpr[2] << "))\n";
	}

	return strstr.str();
}

std::string SBScene::save(bool remoteSetup)
{
	std::stringstream strstr;

	strstr << "# Autogenerated by SmartBody\n";
	
	saveScene(strstr, remoteSetup);
	saveAssets(strstr, remoteSetup);
	saveCameras(strstr, remoteSetup);
	saveLights(strstr, remoteSetup);
	saveFaceDefinitions(strstr, remoteSetup);
	saveJointMaps(strstr, remoteSetup);
	saveLipSyncing(strstr, remoteSetup);
	saveBlends(strstr, remoteSetup);
	saveGestureMaps(strstr, remoteSetup);
	savePawns(strstr, remoteSetup);
	saveCharacters(strstr, remoteSetup);
	saveServices(strstr, remoteSetup);


	return strstr.str();
}

std::string SBScene::exportScene(const std::vector<std::string>& aspects, bool remoteSetup)
{
	std::stringstream strstr;
	strstr << "# Autogenerated by SmartBody\n";
	
	std::set<std::string> set;
	for (std::vector<std::string>::const_iterator iter = aspects.begin();
		iter != aspects.end(); iter++)
	{
		set.insert(*iter);
	}

	if (set.find("scene") != set.end())
		saveScene(strstr, remoteSetup);
	if (set.find("assets") != set.end())
		saveAssets(strstr, remoteSetup);
	if (set.find("cameras") != set.end())
		saveCameras(strstr, remoteSetup);
	if (set.find("lights") != set.end())
		saveLights(strstr, remoteSetup);
	if (set.find("face definitions") != set.end())
		saveFaceDefinitions(strstr, remoteSetup);
	if (set.find("joint maps") != set.end())
		saveJointMaps(strstr, remoteSetup);
	if (set.find("lip syncing") != set.end())
		saveLipSyncing(strstr, remoteSetup);
	if (set.find("blends and transitions") != set.end())
		saveBlends(strstr, remoteSetup);
	if (set.find("gesture maps") != set.end())
		saveGestureMaps(strstr, remoteSetup);
	if (set.find("pawns") != set.end())
		savePawns(strstr, remoteSetup);
	if (set.find("characters") != set.end())
		saveCharacters(strstr, remoteSetup);
	if (set.find("services") != set.end())
		saveServices(strstr, remoteSetup);
	if (set.find("services") != set.end())
		savePositions(strstr, remoteSetup);

	return strstr.str();
}


void SBScene::saveScene(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- cameras\n";
	strstr << "scene.setScale(" << getScale() << ")\n";

	strstr << "# -------------------- scene\n";
	strstr << "obj = scene\n";
	// scene attributes
	std::vector<std::string> attributeNames = this->getAttributeNames();
	for (std::vector<std::string>::iterator iter = attributeNames.begin();
			iter != attributeNames.end();
			iter++)
	{
		SmartBody::SBAttribute* attr = this->getAttribute((*iter));
		std::string attrWrite = attr->write();
		strstr << attrWrite;
	}
	
}

void SBScene::saveAssets(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- media and asset paths\n";
	// mediapath
	std::string mediaPath = getMediaPath();
	strstr << "scene.setMediaPath(\"" << mediaPath << "\")\n";

	// asset paths
	std::vector<std::string>::iterator iter;
	std::vector<std::string> motionPaths = getAssetPaths("motion");
	for (iter = motionPaths.begin(); iter != motionPaths.end(); iter++)
	{
		const std::string& path = (*iter);
		strstr << "scene.addAssetPath(\"motion\", \"" << path << "\")\n";
	}
	
	std::vector<std::string> scriptPaths = getAssetPaths("script");
	for (iter = scriptPaths.begin(); iter != scriptPaths.end(); iter++)
	{
		const std::string& path = (*iter);
		strstr << "scene.addAssetPath(\"script\", \"" << path << "\")\n";
	}

	std::vector<std::string> audioPaths = getAssetPaths("audio");
	for (iter = audioPaths.begin(); iter != audioPaths.end(); iter++)
	{
		const std::string& path = (*iter);
		strstr << "scene.addAssetPath(\"audio\", \"" << path << "\")\n";
	}

	std::vector<std::string> meshPaths = getAssetPaths("mesh");
	for (iter = meshPaths.begin(); iter != meshPaths.end(); iter++)
	{
		const std::string& path = (*iter);
		strstr << "scene.addAssetPath(\"mesh\", \"" << path << "\")\n";
	}
	if (remoteSetup) // to-do : different treatment when saving setup script for remote connection
	{
		// need to go through all skeleton, and explicitly create those skeletons from script
		std::vector<std::string> skeletonNames = getSkeletonNames();
		const std::vector<std::string>& charNames = getCharacterNames();
		std::map<std::string, std::string> charSkelMap;
		for (unsigned int i=0;i<charNames.size(); i++)
		{
			std::string charName = charNames[i];
			SBCharacter* sbChar = getCharacter(charName);
			if (!sbChar)
				continue;
			std::string skelName = sbChar->getSkeleton()->getName();
			SBSkeleton* skel = getSkeleton(skelName);
			if (skel && charSkelMap.find(skelName) == charSkelMap.end())
			{
				std::string skelStr = skel->saveToString();
				strstr << "tempSkel = scene.addSkeletonDefinition(\"" << skelName << "\")\n";	
				std::string skelSaveStr = skel->saveToString();
				//skelSaveStr.replace('\n',)
				boost::replace_all(skelSaveStr,"\n","\\n");
				boost::replace_all(skelSaveStr,"\"","");
				strstr << "tempSkel.loadFromString(\"" << skelSaveStr << "\")\n";
				charSkelMap[skelName] = charName;
			}
		}
		std::vector<std::string> motionNames = getMotionNames();
		for (unsigned int i=0;i<motionNames.size();i++)
		{
			std::string moName = motionNames[i];
			SBMotion* motion = getMotion(moName);
			if (motion)
			{
				// add motion definition
				strstr << "tempMotion = scene.addMotionDefinition(\"" << moName << "\"," << motion->getDuration() << ")\n";	
				// add sync points
				strstr << "tempMotion.setSyncPoint(\"start\"," << motion->getTimeStart() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"ready\"," << motion->getTimeReady() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"stroke_start\"," << motion->getTimeStrokeStart() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"stroke\"," << motion->getTimeStroke() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"stroke_stop\"," << motion->getTimeStrokeEnd() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"relax\"," << motion->getTimeRelax() <<")\n";
				strstr << "tempMotion.setSyncPoint(\"stop\"," << motion->getTimeStop() <<")\n";				
				// add meta data tag
				BOOST_FOREACH(const std::string& tagName, motion->getMetaDataTags())
				{
					strstr << "tempMotion.addMetaData(\"" << tagName << "\",\"" << motion->getMetaDataString(tagName) << "\")\n";
				}
			}
		}
	}
	else
	{
		strstr << "# -------------------- load assets\n";
		strstr << "scene.loadAssets()\n";
	}	

	// create any mirrored assets
	std::vector<std::string> motions = this->getMotionNames();
	for (std::vector<std::string>::iterator iter = motions.begin();
		 iter != motions.end();
		 iter++)
	{
		SBMotion* mirroredMotion = this->getMotion(*iter);
		StringAttribute* mirroredMotionAttr = dynamic_cast<StringAttribute*>(mirroredMotion->getAttribute("mirrorMotion"));
		if (mirroredMotionAttr)
		{
			strstr << "motion = scene.getMotion(\"" << mirroredMotionAttr->getValue() << "\")\n";
			// make sure the skeleton exists
			StringAttribute* mirroredSkeletonAttr = dynamic_cast<StringAttribute*>(mirroredMotion->getAttribute("mirrorSkeleton"));
			if (mirroredSkeletonAttr)
			{
				const std::string& skeletonName = mirroredSkeletonAttr->getValue();
				strstr << "mirrorSkeleton = scene.getSkeleton(\"" << skeletonName << "\")\n";
				strstr << "if mirrorSkeleton is not None:\n";
				strstr << "\tmirroredMotion = motion.mirror(\"" << mirroredMotion->getName() << "\", \"" << skeletonName << "\")\n";
			}

		}
	}
	

}
void SBScene::saveCameras(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- cameras\n";

	// save all default cameras
	std::vector<std::string> cameras = getCameraNames();
	for (std::vector<std::string>::iterator cameraIter = cameras.begin();
		cameraIter != cameras.end();
		cameraIter++)
	{
		SrCamera* camera = getCamera((*cameraIter));
		strstr << "obj = scene.getCamera(\"" << camera->getName() << "\")\n";
		strstr << "if obj == None:\n";
		strstr << "\tobj = scene.createCamera(\"" << camera->getName() << "\")\n";		
		strstr << "obj.setEye(" << camera->getEye().x << ", " << camera->getEye().y << ", " << camera->getEye().z << ")\n";
		strstr << "obj.setCenter(" << camera->getCenter().x << ", " << camera->getCenter().y << ", " << camera->getCenter().z << ")\n";
		strstr << "obj.setUpVector(SrVec(" << camera->getUpVector().x << ", " << camera->getUpVector().y << ", " << camera->getUpVector().z << "))\n";
		strstr << "obj.setScale(" << camera->getScale() << ")\n";
		strstr << "obj.setFov(" << camera->getFov() << ")\n";
		strstr << "obj.setFarPlane(" << camera->getFarPlane() << ")\n";
		strstr << "obj.setNearPlane(" << camera->getNearPlane() << ")\n";
		strstr << "obj.setAspectRatio(" << camera->getAspectRatio() << ")\n";

		std::vector<std::string> attributeNames = camera->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			iter != attributeNames.end();
			iter++)
		{
			SmartBody::SBAttribute* attr = camera->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}
	}	
}

void SBScene::savePawns(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- pawns and characters\n";
	// pawns
	const std::vector<std::string>& pawns = getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		 pawnIter != pawns.end();
		 pawnIter++)
	{
		SBPawn* pawn = getPawn((*pawnIter));
		SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
		if (camera)
			continue; // already wrote out pawns
		if (pawn->getName().find("light") == 0)
		{
			// already wrote lights
			continue;
		}
		strstr << "\n# ---- pawn: " << pawn->getName() << "\n";
		strstr << "obj = scene.createPawn(\"" << pawn->getName() << "\")\n";
		SrVec position = pawn->getPosition();
		strstr << "obj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		SrQuat orientation = pawn->getOrientation();
		strstr << "obj.setOrientation(SrQuat(" << orientation.w << ", " << orientation.x << ", " << orientation.y << ", " << "orientation.z))\n";
		// attributes
		std::vector<std::string> attributeNames = pawn->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			 iter != attributeNames.end();
			 iter++)
		{
			SmartBody::SBAttribute* attr = pawn->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}
	}
}

void SBScene::savePositions(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- pawn positions\n";
	const std::vector<std::string>& pawns = getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		 pawnIter != pawns.end();
		 pawnIter++)
	{
		SBPawn* pawn = getPawn((*pawnIter));
		SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
		if (camera)
			continue; // already wrote out pawns
		if (pawn->getName().find("light") == 0)
		{
			// already wrote lights
			continue;
		}
		strstr << "obj = scene.getPawn(\"" << pawn->getName() << "\")\n";
		SrVec position = pawn->getPosition();
		SrQuat orientation = pawn->getOrientation();
		strstr << "if obj is not None:\n";
		strstr << "\tobj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		strstr << "\tobj.setOrientation(SrQuat(" << orientation.w << ", " << orientation.x << ", " << orientation.y << ". " << orientation.z << "))\n";
	}

	strstr << "# -------------------- character positions\n";
	const std::vector<std::string>& characters = getCharacterNames();
	for (std::vector<std::string>::const_iterator characterIter = characters.begin();
			characterIter != characters.end();
			characterIter++)
	{
		SBCharacter* character = getCharacter((*characterIter));
		SrVec position = character->getPosition();
		SrQuat orientation = character->getOrientation();
		strstr << "obj = scene.getCharacter(\"" << character->getName() << "\", \"" << character->getType() << "\")\n";
		strstr << "if obj is not None:\n";
		strstr << "\tobj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		strstr << "\tobj.setOrientation(SrQuat(" << orientation.w << ", " << orientation.x << ", " << orientation.y << ". " << orientation.z << "))\n";
	}
}

void SBScene::saveCharacters(std::stringstream& strstr, bool remoteSetup)
{
	// characters
	const std::vector<std::string>& characters = getCharacterNames();
	for (std::vector<std::string>::const_iterator characterIter = characters.begin();
		 characterIter != characters.end();
		 characterIter++)
	{
		SBCharacter* character = getCharacter((*characterIter));
		strstr << "\n# ---- character: " << character->getName() << "\n";
		strstr << "obj = scene.createCharacter(\"" << character->getName() << "\", \"" << character->getType() << "\")\n";
		strstr << "skeleton = scene.createSkeleton(\"" << character->getSkeleton()->getName() << "\")\n";
		strstr << "obj.setSkeleton(skeleton)\n";
		SrVec position = character->getPosition();
		strstr << "obj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		SrQuat orientation = character->getOrientation();
		strstr << "obj.setOrientation(SrQuat(" << orientation.w << ", " << orientation.x << ", " << orientation.y << ", " << "orientation.z))\n";

		// face definition
		SBFaceDefinition* faceDef = character->getFaceDefinition();
		if (faceDef)
			strstr << "obj.setFaceDefinition(scene.getFaceDefinition(\"" << faceDef->getName() << "\"))\n";

		// controllers
		strstr << "obj.createStandardControllers()\n";
		// attributes
		std::vector<std::string> attributeNames = character->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			 iter != attributeNames.end();
			 iter++)
		{
			SmartBody::SBAttribute* attr = character->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}

		// reach
		strstr << "# -------------------- reaching for character " << character->getName() << "\n";
		// reach
		SBReachManager* reachManager = this->getReachManager();
		SBReach* reach = reachManager->getReach(character->getName());
		if (reach)
		{
			strstr << "reach = scene.getReachManager().createReach(\"" << character->getName() << "\")\n";
			std::string interpType = reach->getInterpolatorType();
			strstr << "reach.setInterpolatorType(\"" << interpType << "\")\n";
			// motions
			const std::vector<std::string>& leftMotionNames = reach->getMotionNames("left");
			for (std::vector<std::string>::const_iterator iter = leftMotionNames.begin();
				 iter != leftMotionNames.end();
				 iter++)
			{
				strstr << "reach.addMotion(\"left\", scene.getMotion(\"" << (*iter) << "\"))\n";
			}
			const std::vector<std::string>& rightMotionNames = reach->getMotionNames("right");
			for (std::vector<std::string>::const_iterator iter = rightMotionNames.begin();
				 iter != rightMotionNames.end();
				 iter++)
			{
				strstr << "reach.addMotion(\"right\", scene.getMotion(\"" << (*iter) << "\"))\n";
			}
			// point hand
			SBMotion* m = NULL;
			m = reach->getPointHandMotion("left");
			if (m)
			{
				strstr << "reach.setPointHandMotion(\"left\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			m = reach->getPointHandMotion("right");
			if (m)
			{
				strstr << "reach.setPointHandMotion(\"right\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			// grab hand
			m = reach->getGrabHandMotion("left");
			if (m)
			{
				strstr << "reach.setGrabHandMotion(\"left\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			m = reach->getGrabHandMotion("right");
			if (m)
			{
				strstr << "reach.setGrabHandMotion(\"right\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			// release hand
			m = reach->getReleaseHandMotion("left");
			if (m)
			{
				strstr << "reach.setReleaseHandMotion(\"left\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			m = reach->getReleaseHandMotion("right");
			if (m)
			{
				strstr << "reach.setReleaseHandMotion(\"right\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			// reach hand
			m = reach->getReachHandMotion("left");
			if (m)
			{
				strstr << "reach.setReachHandMotion(\"left\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			m = reach->getReachHandMotion("right");
			if (m)
			{
				strstr << "reach.setReachHandMotion(\"right\", scene.getMotion(\"" << m->getName() << "\"))\n";
			}
			strstr << "reach.build(scene.getCharacter(\"" << character->getName() << "\"))\n";
		}
		else
		{
			strstr << "# -- no reach\n";
		}

		// steering
		strstr << "# --- steering for character " << character->getName() << "\n";
		SBSteerManager* steerManager = this->getSteerManager();
		SBSteerAgent* steerAgent = steerManager->getSteerAgent(character->getName());
		if (steerAgent)
		{
			strstr << "steeragent = scene.getSteerManager().createSteerAgent(\"" << character->getName() << "\")\n";
			strstr << "steeragent.setSteerStateNamePrefix(\"" << steerAgent->getSteerStateNamePrefix() << "\")\n";
			strstr << "steeragent.setSteerType(\"" << steerAgent->getSteerType() << "\")\n";
		}
		else
		{
			strstr << "# --- no steering for character " << character->getName() << "\n";
		}
	}

	// enable steering
	if (this->getSteerManager()->getSteerAgents().size() > 0)
	{
		strstr << "scene.getServiceManager().getService(\"steering\").setBoolAttribute(\"enable\", True)\n";
	}
}

void SBScene::saveLights(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- lights\n";
	// lights
	const std::vector<std::string>& pawns = getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		 pawnIter != pawns.end();
		 pawnIter++)
	{
		SBPawn* pawn = getPawn((*pawnIter));
		SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
		if (camera)
			continue; 
		if (pawn->getName().find("light") != 0)
		{
			continue;
		}
		strstr << "\n# ---- light: " << pawn->getName() << "\n";
		strstr << "obj = scene.createPawn(\"" << pawn->getName() << "\")\n";
		SrVec position = pawn->getPosition();
		strstr << "obj.setPosition(SrVec(" << position[0] << ", " << position[1] << ", " << position[2] << "))\n";
		SrQuat orientation = pawn->getOrientation();
		strstr << "obj.setOrientation(SrQuat(" << orientation.w << ", " << orientation.x << ", " << orientation.y << ", " << "orientation.z))\n";

		// attributes
		std::vector<std::string> attributeNames = pawn->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			 iter != attributeNames.end();
			 iter++)
		{
			SmartBody::SBAttribute* attr = pawn->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}
	}
}

void SBScene::saveBlends(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- blends and transitions\n";
	// blends & transitions
	SBAnimationBlendManager* blendManager = getBlendManager();
	int numBlends = blendManager->getNumBlends();
	std::vector<std::string> blends = blendManager->getBlendNames();
	for (std::vector<std::string>::iterator blendIter = blends.begin();
		 blendIter != blends.end();
		 blendIter++)
	{
		SBAnimationBlend* blend = blendManager->getBlend((*blendIter));
		std::string blendString = blend->saveToString();
		strstr << blendString;
		strstr << "\n";
	}

	int numTransitions = blendManager->getNumTransitions();
	std::vector<std::string> transitions = blendManager->getBlendNames();
	for (int t = 0; t < numTransitions; t++)
	{
		SBAnimationTransition* transition = blendManager->getTransitionByIndex(t);
		std::string transitionString = transition->saveToString();
		strstr << transitionString;
		strstr << "\n";
	}
}

void SBScene::saveJointMaps(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- joint maps\n";

	// joint maps
	SBJointMapManager* jointMapManager = getJointMapManager();
	std::vector<std::string> jointMapNames = jointMapManager->getJointMapNames();
	for (std::vector<std::string>::iterator iter = jointMapNames.begin(); iter != jointMapNames.end(); iter++)
	{
		const std::string& jointMapName = (*iter);
		SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);

		strstr << "jointMap = scene.getJointMapManager().createJointMap(\"" << jointMapName << "\")\n";

		int numMappings = jointMap->getNumMappings();
		for (int m = 0; m < numMappings; m++)
		{
			std::string target = jointMap->getTarget(m);
			std::string source = jointMap->getSource(m);
			strstr << "jointMap.setMapping(\"" << source << "\", \"" << target << "\")\n";
		}
	}

	strstr << "# -------------------- applying joint maps\n";
	for (std::vector<std::string>::iterator iter = jointMapNames.begin(); iter != jointMapNames.end(); iter++)
	{
		const std::string& jointMapName = (*iter);
		SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);

		strstr << "jointMap = scene.getJointMapManager().getJointMap(\"" << jointMapName << "\")\n";

		std::vector<std::string>& mappedMotions = jointMap->getMappedMotions();
		for (std::vector<std::string>::iterator iter = mappedMotions.begin();
			 iter != mappedMotions.end();
			 iter++)
		{
			strstr << "jointMap.applyMotion(scene.getMotion(\"" << (*iter) << "\"))\n";
		}

		std::vector<std::string>& mappedSkeletons = jointMap->getMappedSkeletons();
		for (std::vector<std::string>::iterator iter = mappedSkeletons.begin();
			 iter != mappedSkeletons.end();
			 iter++)
		{
			strstr << "jointMap.applySkeleton(scene.getSkeleton(\"" << (*iter) << "\"))\n";
		}
	}
}

void SBScene::saveFaceDefinitions(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- face definitions\n";
	// face definitions
	std::vector<std::string> faceDefinitions = getFaceDefinitionNames();
	for (std::vector<std::string>::iterator iter = faceDefinitions.begin(); iter != faceDefinitions.end(); iter++)
	{
		const std::string& faceDefName = (*iter);		
		SmartBody::SBFaceDefinition* faceDef = getFaceDefinition(faceDefName);
		std::string faceDefinitionName = "face";
		faceDefinitionName.append(faceDef->getName());
		strstr << faceDefinitionName << " = scene.getFaceDefinition(\"" << faceDefName << "\")\n";
		strstr << "if " << faceDefinitionName << " == None:\n";
		strstr << "\t" << faceDefinitionName << " = scene.createFaceDefinition(\""<< faceDefName << "\")\n";

		SkMotion* neutral = faceDef->getFaceNeutral();
		if (neutral)
		{
			strstr << faceDefinitionName << ".setFaceNeutral(\"" << neutral->getName() << "\")\n";
		}

		std::vector<std::string> visemeNames = faceDef->getVisemeNames();
		for (std::vector<std::string>::iterator faceIter = visemeNames.begin(); 
			 faceIter != visemeNames.end(); 
			 faceIter++)
		{
			const std::string& viseme = (*faceIter);
			strstr << faceDefinitionName << ".setViseme(\"" << viseme << "\", ";
			SkMotion* visemeMotion = faceDef->getVisemeMotion(viseme);
			if (visemeMotion)
			{
				strstr << "\"" + visemeMotion->getName() + "\")\n";
			}
			else
			{
				strstr << "\"\")\n";
			}
		}	

		std::vector<int> auNum = faceDef->getAUNumbers();
		for (std::vector<int>::iterator auIter = auNum.begin(); 
			 auIter != auNum.end(); 
			 auIter++)
		{
			int num = (*auIter);
			ActionUnit* au = faceDef->getAU((*auIter));
			strstr << faceDefinitionName << ".setAU(" << num << ", ";
			SkMotion* motion = NULL;
			if (au->is_bilateral())
			{
				strstr << "\"both\", \"";
				motion = au->left;
			}
			else
			{
				
				if (au->is_left())
				{
					strstr << "\"left\", \"";
					motion = au->left;
				}
				else
				{
					strstr << "\"right\", \"";
					motion = au->right;
				}
			}
			if (motion)
			{
				strstr << motion->getName(); 
			}
			strstr << "\")\n";

		}
	}
}

void SBScene::saveGestureMaps(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- gesture maps\n";
	const std::vector<std::string>& gestureMapNames = this->getGestureMapManager()->getGestureMapNames();
	for (std::vector<std::string>::const_iterator iter = gestureMapNames.begin();
		 iter != gestureMapNames.end();
		 iter++)
	{
		SBGestureMap* gestureMap = this->getGestureMapManager()->getGestureMap(*iter);
		strstr << "gestureMap = scene.getGestureMapManager().createGestureMap(\"" << (*iter) << "\")\n";
		int numMappings = gestureMap->getNumMappings();
		for (int m = 0; m < numMappings; m++)
		{
			SBGestureMap::GestureInfo& info = gestureMap->getGestureByIndex(m);
			strstr << "gestureMap.addGestureMapping(\"" << info._animation << "\", \"" << info._lexeme << "\", \"" << info._type << "\", \"" 
				                                        << info._hand << "\", \"" << info._style << "\", \"" << info._posture << "\")\n";

		}
	}

}

void SBScene::saveLipSyncing(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- lip syncing\n";

	// diphones
	SBDiphoneManager* diphoneManager = getDiphoneManager();
	std::vector<std::string> diphoneMapNames = diphoneManager->getDiphoneMapNames();
	strstr << "diphoneMapManager = scene.getDiphoneManager()\n";
	for (std::vector<std::string>::iterator iter = diphoneMapNames.begin(); iter != diphoneMapNames.end(); iter++)
	{
		const std::string& diphoneMapName = (*iter);
		std::vector<SBDiphone*>& diphones = diphoneManager->getDiphones(diphoneMapName);
		for (std::vector<SBDiphone*>::iterator diphoneIter = diphones.begin();
			 diphoneIter != diphones.end();
			 diphoneIter++)
		{
			SBDiphone* diphone = (*diphoneIter);
			const std::string& fromPhoneme = diphone->getFromPhonemeName();
			const std::string& toPhoneme = diphone->getToPhonemeName();
			strstr << "diphone = diphoneMapManager.createDiphone(\"" << fromPhoneme << "\", \"" << toPhoneme << "\", \"" << diphoneMapName << "\")\n";
			int numVisemes = diphone->getNumVisemes();
			std::vector<std::string> visemes = diphone->getVisemeNames();
			for (std::vector<std::string>::iterator visemeIter = visemes.begin();
				 visemeIter != visemes.end();
				 visemeIter++)
			{
				std::vector<float>& keys = diphone->getKeys((*visemeIter));
				for (size_t x = 0; x < keys.size(); x++)
				{
					if (x + 1 < keys.size())
					{
						strstr << "diphone.addKey(\"" << (*visemeIter) << "\", " << keys[x] << ", " << keys[x + 1] << ")\n";
					}
					x++;
				}
				strstr << "\n";
			}
		}
	}
}

void SBScene::saveServices(std::stringstream& strstr, bool remoteSetup)
{
	strstr << "# -------------------- lip syncing\n";
	// services
	SmartBody::SBServiceManager* serviceManager = this->getServiceManager();
	std::vector<std::string> serviceNames = serviceManager->getServiceNames();
	for (std::vector<std::string>::iterator iter = serviceNames.begin();
		 iter != serviceNames.end();
		 iter++)
	{
		SmartBody::SBService* service = serviceManager->getService((*iter));
		strstr << "obj = scene.getServiceManager().getService(\"" << service->getName() << "\")\n";
		std::vector<std::string> attributeNames = service->getAttributeNames();
		for (std::vector<std::string>::iterator iter = attributeNames.begin();
			 iter != attributeNames.end();
			 iter++)
		{
			SmartBody::SBAttribute* attr = service->getAttribute((*iter));
			std::string attrWrite = attr->write();
			strstr << attrWrite;
		}

	}

}



void SBScene::setSystemParameter(const std::string& name, const std::string& value)
{
	std::map<std::string, std::string>::iterator iter = _systemParameters.find(name);
	if (iter != _systemParameters.end())
	{
		(*iter).second = value;
	}
	else
	{
		_systemParameters.insert(std::pair<std::string, std::string>(name, value));
	}
		
}

std::string SBScene::getSystemParameter(const std::string& name)
{
	std::map<std::string, std::string>::iterator iter = _systemParameters.find(name);
	if (iter != _systemParameters.end())
	{
		return (*iter).second;
	}
	else
	{
		return "";
	}
}

void SBScene::removeSystemParameter(const std::string& name)
{
	std::map<std::string, std::string>::iterator iter = _systemParameters.find(name);
	if (iter != _systemParameters.end())
	{
		_systemParameters.erase(iter);
		return;
	}

	LOG("Cannot remove system parameter named '%s', does not exist.", name.c_str());

}

void SBScene::removeAllSystemParameters()
{
	_systemParameters.clear();
}

std::vector<std::string> SBScene::getSystemParameterNames()
{

	std::vector<std::string> names;
	for (std::map<std::string, std::string>::iterator iter = _systemParameters.begin();
		 iter != _systemParameters.end();
		 iter++)
	{
		names.push_back((*iter).first);
	}

	return names;
}

SrCamera* SBScene::createCamera(const std::string& name)
{
	SBPawn* pawn = getPawn(name);
	SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
// 	if (camera)
// 	{
// 		LOG("A camera with name '%s' already exists.", name.c_str());
// 		return camera;
// 	}
// 	else 
	if (pawn)
	{
		LOG("A pawn with name '%s' already exists. Camera will not be created.", name.c_str());
		return NULL;
	}
	camera = new SrCamera();
	camera->setName(name);
	//SBSkeleton* skeleton = new SBSkeleton();
	//camera->setSkeleton(skeleton);
	//SkJoint* joint = skeleton->add_joint(SkJoint::TypeQuat);
	//joint->setName("world_offset");

	_cameras.insert(std::pair<std::string, SrCamera*>(name, camera));

	std::map<std::string, SbmPawn*>:: iterator iter = _pawnMap.find(camera->getName());
	if (iter != _pawnMap.end())
	{
		LOG( "Register pawn: pawn_map.insert(..) '%s' FAILED\n", camera->getName().c_str() ); 
	}

	_pawnMap.insert(std::pair<std::string, SbmPawn*>(camera->getName(), camera));
	
	if (getCharacterListener())
		getCharacterListener()->OnPawnCreate( camera->getName().c_str() );
	

	// notify the services
	std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
		iter != services.end();
		iter++)
	{
		SBService* service = (*iter).second;
		service->onPawnCreate(camera);
	}
	

	// if this is the first camera that is created, make it the active camera
	if (_cameras.size() == 1)
	{
		this->setActiveCamera(camera);
	}

	return camera;
}

void SBScene::removeCamera(SrCamera* camera)
{
	SBPawn* pawn = getPawn(camera->getName());
	if (!pawn)
	{
		LOG("No camera with name '%s' already exists. Camera will not be removed.", camera->getName().c_str());
		return;
	}

	std::map<std::string, SrCamera*>::iterator iter = _cameras.find(camera->getName());
	if (iter == _cameras.end())
	{
		LOG("Pawn with name '%s' already exists, but is not a camera. It will not be removed.", camera->getName().c_str());
		return;
	}

	// is this the active camera?
	if (this->getActiveCamera() == camera)
	{
		setActiveCamera(NULL);
	}
	_cameras.erase(iter);
	removePawn(camera->getName());
}

void SBScene::setActiveCamera(SrCamera* camera)
{
	if (!camera)
	{
		_activeCamera = "";
		return;
	}

	_activeCamera = camera->getName();
}

SrCamera* SBScene::getActiveCamera()
{
	if (_activeCamera == "")
		return NULL;
	std::map<std::string, SrCamera*>::iterator iter = _cameras.find(_activeCamera);
	if (iter == _cameras.end())
		return NULL;

	return (*iter).second;
}

SrCamera* SBScene::getCamera(const std::string& name)
{
	
	std::map<std::string, SrCamera*>::iterator iter = _cameras.find(name);
	if (iter == _cameras.end())
	{
		LOG("No camera with name '%s' found.", name.c_str());
		return NULL;
	}
	return (*iter).second;
}

int SBScene::getNumCameras()
{
	return _cameras.size();
}

std::vector<std::string> SBScene::getCameraNames()
{
	std::vector<std::string> cameraNames;
	for (std::map<std::string, SrCamera*>::iterator iter = _cameras.begin();
		iter != _cameras.end();
		iter++)
	{
		cameraNames.push_back((*iter).first);
	}

	return cameraNames;
}


std::vector<SBController*>& SBScene::getDefaultControllers()
{
	return _defaultControllers;
}

void SBScene::createDefaultControllers()
{
	 _defaultControllers.push_back(new MeCtEyeLidRegulator());
	 _defaultControllers.push_back(new MeCtSaccade(NULL));
	 std::map<int, MeCtReachEngine*> reachMap;
	 _defaultControllers.push_back(new MeCtExampleBodyReach(reachMap));
	 _defaultControllers.push_back(new MeCtBreathing());
	 _defaultControllers.push_back(new MeCtGaze());

	 for (size_t x = 0; x < _defaultControllers.size(); x++)
		 _defaultControllers[x]->ref();
}

void SBScene::removeDefaultControllers()
{
	 for (size_t x = 0; x < _defaultControllers.size(); x++)
		 _defaultControllers[x]->unref();
	 _defaultControllers.clear();
}

std::vector<std::string> SBScene::getAssetPaths(const std::string& type)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getAssetPaths() instead.");
	return getAssetManager()->getAssetPaths(type);
}

std::vector<std::string> SBScene::getLocalAssetPaths(const std::string& type)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getLocalAssetPaths() instead.");
	return getAssetManager()->getLocalAssetPaths(type);
}

void SBScene::addAssetPath(const std::string& type, const std::string& path)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addAssetPath() instead.");
	getAssetManager()->addAssetPath(type, path);
}

void SBScene::removeAssetPath(const std::string& type, const std::string& path)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addAssetPath() instead.");
	getAssetManager()->removeAssetPath(type, path);
}

void SBScene::removeAllAssetPaths(const std::string& type)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.removeAllAssetPaths() instead.");
	getAssetManager()->removeAllAssetPaths(type);
}

void SBScene::loadAssets()
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addAssetPath() instead.");
	getAssetManager()->loadAssets();
}

void SBScene::loadAsset(const std::string& assetPath)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.loadAsset() instead.");
	getAssetManager()->loadAsset(assetPath);
}

void SBScene::loadAssetsFromPath(const std::string& assetPath)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.loadAssetsFromPath() instead.");
	getAssetManager()->loadAssetsFromPath(assetPath);
}

SBSkeleton* SBScene::addSkeletonDefinition(const std::string& skelName )
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addSkeletonDefinition() instead.");
	return getAssetManager()->addSkeletonDefinition(skelName);
}

SBMotion* SBScene::addMotionDefinition(const std::string& motionName, double duration )
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addMotionDefinition() instead.");
	return getAssetManager()->addMotionDefinition(motionName, duration);
}

void SBScene::addMotions(const std::string& path, bool recursive)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.addMotion() instead.");
	getAssetManager()->addMotions(path, recursive);
}

int SBScene::getNumSkeletons()
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getNumSkeletons() instead.");
	return getAssetManager()->getNumSkeletons();
}

std::vector<std::string> SBScene::getSkeletonNames()
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getSkeletonNames() instead.");
	return getAssetManager()->getSkeletonNames();
}

SBMotion* SBScene::getMotion(const std::string& name)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getMotion() instead.");
	return getAssetManager()->getMotion(name);
}

int SBScene::getNumMotions()
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getNumMotions() instead.");
	return getAssetManager()->getNumMotions();
}

std::vector<std::string> SBScene::getMotionNames()
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getNumMotions() instead.");
	return getAssetManager()->getMotionNames();
}

SBSkeleton* SBScene::createSkeleton(const std::string& skeletonDefinition)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.createSkeleton() instead.");
	return getAssetManager()->createSkeleton(skeletonDefinition);
}

SBSkeleton* SBScene::getSkeleton(const std::string& name)
{
	if (SHOW_DEPRECATION_MESSAGES)
		LOG("DEPRECATED: Use AssetManager.getSkeleton() instead.");
	return getAssetManager()->getSkeleton(name);
}

SrSnGroup* SBScene::getRootGroup()
{
	return _rootGroup;
}


std::string SBScene::getValidName(const std::string& name)
{
	bool nameFound = true;
	int nameCounter = 0;
	std::string currentName = name;
	while (nameFound)
	{
		SmartBody::SBPawn* pawn = getPawn(currentName);
		if (!pawn)
		{
			nameFound = false;
		}
		else
		{
			SmartBody::SBCharacter* character = getCharacter(currentName);
			if (!character)
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
	}
	return currentName;
}

void SBScene::updatePawnNames()
{
	std::vector<SbmPawn*> allPawns;
	for (std::map<std::string, SbmPawn*>::iterator iter = _pawnMap.begin();
		 iter != _pawnMap.end();
		 iter++)
	{
		allPawns.push_back((*iter).second);
	}
	_pawnMap.clear();
	_pawnNames.clear();

	for (std::vector<SbmPawn*>::iterator iter = allPawns.begin();
		 iter != allPawns.end();
		 iter++)
	{
		_pawnMap.insert(std::pair<std::string, SbmPawn*>((*iter)->getName(), (*iter))); 
		_pawnNames.push_back((*iter)->getName());
	}

}

void SBScene::updateCharacterNames()
{
	std::vector<SbmCharacter*> allCharacters;
	for (std::map<std::string, SbmCharacter*>::iterator iter = _characterMap.begin();
		 iter != _characterMap.end();
		 iter++)
	{
		allCharacters.push_back((*iter).second);
	}
	_characterMap.clear();
	_characterNames.clear();

	for (std::vector<SbmCharacter*>::iterator iter = allCharacters.begin();
		 iter != allCharacters.end();
		 iter++)
	{
		_characterMap.insert(std::pair<std::string, SbmCharacter*>((*iter)->getName(), (*iter))); 
		_characterNames.push_back((*iter)->getName());
	}
}

Heightfield* SBScene::getHeightfield()
{
	return _heightField;
}


Heightfield* SBScene::createHeightfield()
{
	if (_heightField)
		delete _heightField;
	_heightField = new Heightfield();
	return _heightField;
}

void SBScene::removeHeightfield()
{
	if (_heightField)
		delete _heightField;
	_heightField = NULL;
}

float SBScene::queryTerrain( float x, float z, float *normal_p )
{
	if (_heightField)
	{
		return( _heightField->get_elevation( x, z, normal_p ) );
	}
	if( normal_p )	{
		normal_p[ 0 ] = 0.0;
		normal_p[ 1 ] = 1.0;
		normal_p[ 2 ] = 0.0;
	}
	return( 0.0 );
}

#ifndef SB_NO_PYTHON
void SBScene::setPythonMainModule(boost::python::object pyobject)
{
	_mainModule = pyobject;
}

void SBScene::setPythonMainDict(boost::python::object pyobject)
{
	_mainDict = pyobject;
}

boost::python::object SBScene::getPythonMainModule()
{
	return _mainModule;
}

boost::python::object SBScene::getPythonMainDict()
{
	return _mainDict;
}
#endif

void SBScene::setCameraTrack(const std::string& characterName, const std::string& jointName)
{
	SrCamera* camera = getActiveCamera();
	if (!camera)
	{
		LOG("No active camera found. Cannot create camera track.");
		return;
	}
	SbmPawn* pawn = SmartBody::SBScene::getScene()->getPawn(characterName);
	if (!pawn)
	{
		LOG("Object %s was not found, cannot track.", characterName.c_str());
		return;
	}
	if (jointName == "")
	{
		LOG("Need to specify a joint to track.");
		return;
	}

	SkSkeleton* skeleton = NULL;
	skeleton = pawn->getSkeleton();

	SkJoint* joint = pawn->getSkeleton()->search_joint(jointName.c_str());
	if (!joint)
	{
		LOG("Could not find joint %s on object %s.", jointName.c_str(), characterName.c_str());
		return;
	}

	joint->skeleton()->update_global_matrices();
	joint->update_gmat();
	const SrMat& jointMat = joint->gmat();
	SrVec jointPos(jointMat[12], jointMat[13], jointMat[14]);
	CameraTrack* cameraTrack = new CameraTrack();
	cameraTrack->joint = joint;
	cameraTrack->jointToCamera = camera->getEye() - jointPos;
	LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
	cameraTrack->targetToCamera = camera->getEye() - camera->getCenter();
	LOG("Vector from target to eye is %f %f %f", cameraTrack->targetToCamera.x, cameraTrack->targetToCamera.y, cameraTrack->targetToCamera.z);				
	_cameraTracking.push_back(cameraTrack);
	LOG("Object %s will now be tracked at joint %s.", characterName.c_str(), jointName.c_str());
}

void SBScene::removeCameraTrack()
{
	if (_cameraTracking.size() > 0)
	{
		for (std::vector<CameraTrack*>::iterator iter = _cameraTracking.begin();
			 iter != _cameraTracking.end();
			 iter++)
		{
			CameraTrack* cameraTrack = (*iter);
			delete cameraTrack;
		}
		_cameraTracking.clear();
		LOG("Removing current tracked object.");
	}
}

bool SBScene::hasCameraTrack()
{
	return _cameraTracking.size() > 0;
}

void SBScene::updateTrackedCameras()
{
	for (size_t x = 0; x < _cameraTracking.size(); x++)
	{
		// move the camera relative to the joint
		SkJoint* joint = _cameraTracking[x]->joint;
		joint->skeleton()->update_global_matrices();
		joint->update_gmat();
		const SrMat& jointGmat = joint->gmat();
		SrVec jointLoc(jointGmat[12], jointGmat[13], jointGmat[14]);
		SrVec newJointLoc = jointLoc;
		if (fabs(jointGmat[13] - _cameraTracking[x]->yPos) < _cameraTracking[x]->threshold)
			newJointLoc.y = (float) _cameraTracking[x]->yPos;
		SrVec cameraLoc = newJointLoc + _cameraTracking[x]->jointToCamera;
		SrCamera* activeCamera = getActiveCamera();
		activeCamera->setEye(cameraLoc.x, cameraLoc.y, cameraLoc.z);
		SrVec targetLoc = cameraLoc - _cameraTracking[x]->targetToCamera;
		activeCamera->setCenter(targetLoc.x, targetLoc.y, targetLoc.z);
	}	
}

SrViewer* SBScene::getViewer()
{
	return _viewer;
}

SrViewer* SBScene::getOgreViewer()
{
	return _ogreViewer;
}

void SBScene::setViewer(SrViewer* viewer)
{
	_viewer = viewer;
}

void SBScene::setOgreViewer(SrViewer* viewer)
{
	_ogreViewer = viewer;
}

SrViewerFactory* SBScene::getViewerFactory()
{
	return _viewerFactory;
}

SrViewerFactory* SBScene::getOgreViewerFactory()
{
	return _ogreViewerFactory;
}

void SBScene::setViewerFactory(SrViewerFactory* viewerFactory)
{
	if (_viewerFactory)
		delete _viewerFactory;
	_viewerFactory = viewerFactory;
}

void SBScene::setOgreViewerFactory(SrViewerFactory* viewerFactory)
{
	if (_ogreViewerFactory)
		delete _ogreViewerFactory;
	_ogreViewerFactory = viewerFactory;
}

KinectProcessor* SBScene::getKinectProcessor()
{
	return _kinectProcessor;
}

std::map<std::string, GeneralParam*>& SBScene::getGeneralParameters()
{
	return _generalParams;
}

SBAPI bool SBScene::createNavigationMesh( const std::string& meshfilename )
{	
	std::vector<SrModel*> meshVec;
	std::string ext = boost::filesystem2::extension(meshfilename);
	std::string file = boost::filesystem::basename(meshfilename);	
	bool loadSuccess = false;
	if (ext == ".obj" || ext == ".OBJ")
	{
		SrModel *mesh = new SrModel();
		loadSuccess = mesh->import_obj(meshfilename.c_str());		
		meshVec.push_back(mesh);
	}
	else if ( ext == ".xml" || ext == ".XML" )
	{
		std::vector<SkinWeight*> tempWeights;
		loadSuccess = ParserOgre::parseSkinMesh(meshVec,tempWeights,meshfilename,1.0,true,false);	
	}
	else if ( ext == ".dae" || ext == ".DAE" )
	{
		loadSuccess = ParserOpenCOLLADA::parseStaticMesh(meshVec, meshfilename);
	}
	
	if (!loadSuccess || meshVec.size() == 0)
	{
		LOG("Error loading navigation mesh, filename = %s",meshfilename.c_str());
		return false;
	}
	//mesh.scale(0.3f);
	SrModel* srMesh = meshVec[0];
	for (unsigned int i=1;i<meshVec.size();i++)
		srMesh->add_model(*meshVec[i]); // adding all mesh together
	srMesh->validate();
	srMesh->computeNormals();
	if (_navigationMesh) 
	{
		delete _navigationMesh;
		_navigationMesh = NULL;
	}

	_navigationMesh = new SBNavigationMesh();
	_navigationMesh->buildNavigationMesh(*srMesh);

	return true;
}

SBAPI SBNavigationMesh* SBScene::getNavigationMesh()
{
	return _navigationMesh;
}

void SBScene::startFileLogging(const std::string& filename)
{
	_logListener = new vhcl::Log::FileListener(filename.c_str());
	vhcl::Log::g_log.AddListener(_logListener);
}

void SBScene::stopFileLogging()
{
	if (_logListener)
		vhcl::Log::g_log.RemoveListener(_logListener);
}

};


