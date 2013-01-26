
#include "vhcl.h"
#include "sbm/mcontrol_util.h"
#include "SBScene.h"
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
#include <sb/SBSkeleton.h>
#include <sb/SBParser.h>
#include <sb/SBDebuggerServer.h>
#include <sb/SBDebuggerClient.h>
#include <sb/SBDebuggerUtility.h>
#include <sbm/sbm_audio.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <sb/nvbg.h>
#include <sb/SBJointMap.h>
#include <sb/SBCharacterListener.h>
#include <sbm/ParserBVH.h>
#include <sr/sr_camera.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_ct_eyelid.h>
#include <controllers/me_ct_breathing.h>
#include <controllers/me_ct_example_body_reach.hpp>
#include <controllers/me_ct_saccade.h>
#include <sbm/KinectProcessor.h>

#ifndef WIN32
#define _stricmp strcasecmp
#endif

namespace SmartBody {

SBScene* SBScene::_scene = NULL;
bool SBScene::_firstTime = true;

std::map<std::string, std::string> SBScene::_systemParameters;

SBScene::SBScene(void) : SBObject()
{
}

void SBScene::initialize()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.reset();

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
	_scale = .01f; // default scale is centimeters

	// add the services
	_serviceManager->addService(_steerManager);
	_serviceManager->addService(_physicsManager);
	_serviceManager->addService(_boneBusManager);
	_serviceManager->addService(_collisionManager);

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

	_mediaPath = ".";
		// re-initialize
	// initialize everything
	
	mcu.loop = true;
	mcu.vhmsg_enabled = false;
	mcu.net_bone_updates = false;
	mcu.net_world_offset_updates = true;
	mcu.play_internal_audio = false;
	mcu.resourceDataChanged = false;
	mcu.perceptionData = new PerceptionData();
	mcu.skmScale = 1.0;
	mcu.skScale = 1.0;
	mcu.root_group_p = new SrSnGroup();
	mcu.test_character_default = "";
	mcu.test_recipient_default = "ALL";
	mcu.queued_cmds = 0;
	mcu.updatePhysics = false;
	mcu.resource_manager = SBResourceManager::getResourceManager();
	mcu.snapshot_counter = 1;
	mcu.use_python = true;
	mcu.sendPawnUpdates = false;
	mcu.logListener = NULL;
	mcu.useXmlCache = false;
	mcu.useXmlCacheAuto = false;
	mcu.testBMLId = 0;
	mcu.registerCallbacks();
	mcu.root_group_p->ref();
	mcu.kinectProcessor = new KinectProcessor();
#if USE_WSP
	mcu.theWSP = WSP::create_manager();
	mcu.theWSP->init( "SMARTBODY" );
#endif
	mcu.internal_timer_p = NULL;
	mcu.external_timer_p = NULL;
	mcu.timer_p = NULL;

	// Create default settings
	createDefaultControllers();
	SmartBody::SBFaceDefinition* faceDefinition = createFaceDefinition("_default_");
	
	SmartBody::SBCharacterListener* listener = getCharacterListener();
	//_scene = SmartBody::SBScene::getScene();
	setCharacterListener(listener);

	_debuggerServer->Init();
	_debuggerServer->SetSBScene(_scene);
	SmartBody::SBAnimationBlend0D* idleState = getBlendManager()->createBlend0D(PseudoIdleState);

	// reset timer & viewer window
	getSimulationManager()->reset();
	getSimulationManager()->start();

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

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// remove the motions and skeleton
	for (std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.begin();
		motionIter !=  mcu.motion_map.end();
		motionIter++)
	{

		SkMotion* motion = (*motionIter).second;
		delete motion;
	}
	mcu.motion_map.clear();

	for (std::map<std::string, SkSkeleton*>::iterator skelIter =  mcu.skeleton_map.begin();
		skelIter !=  mcu.skeleton_map.end();
		skelIter++)
	{
		SkSkeleton* skeleton = (*skelIter).second;
		delete skeleton;
	}
	mcu.skeleton_map.clear();

	// remove the deformable meshes
/*	for (std::map<std::string, DeformableMesh*>::iterator deformableIter =  mcu.deformableMeshMap.begin();
		deformableIter !=  mcu.deformableMeshMap.end();
		deformableIter++)
	{
		DeformableMesh* deformableMesh = (*deformableIter).second;
		delete deformableMesh;
	}
	mcu.deformableMeshMap.clear();
*/
	// remove the XML cache
	for (std::map<std::string, DOMDocument*>::iterator xmlIter = mcu.xmlCache.begin();
		xmlIter != mcu.xmlCache.end();
		xmlIter++)
	{
		(*xmlIter).second->release();
	}
	mcu.xmlCache.clear();

	// remove NVBG
	for (std::map<std::string, Nvbg*>::iterator nvbgIter = mcu.nvbgMap.begin();
		nvbgIter != mcu.nvbgMap.end();
		nvbgIter++)
	{
		Nvbg* nvbg = (*nvbgIter).second;
		delete nvbg;
	}
	mcu.nvbgMap.clear();


	removePendingCommands();

	clearAttributes();

	removeDefaultControllers();

	removeAllAssetPaths("script");
	removeAllAssetPaths("motion");
	removeAllAssetPaths("mesh");
	removeAllAssetPaths("audio");

	
	AUDIO_Close();
	AUDIO_Init();

#if USE_WSP
	mcu.theWSP->shutdown();

	if (mcu.theWSP)
	{
		delete mcu.theWSP;
		mcu.theWSP = NULL;
	}
#endif

	mcu.vhmsg_send( "vrProcEnd sbm" );

	

	
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

//	mcuCBHandle& mcu = mcuCBHandle::singleton();
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.process_id = id;
}

const std::string& SBScene::getProcessId()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.process_id;
}

void SBScene::update()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.update();
}



void SBScene::setScale(float val)
{
	_scale = val;
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
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if (mcu.play_internal_audio)
		{
			mcu.play_internal_audio = false;
			AUDIO_Close();
		}
		else
		{
			mcu.play_internal_audio = true;
			AUDIO_Init();
		}
		return;
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
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		LOG("Character '%s' already exists!", charName.c_str());
		return NULL;
	}
	else
	{
		SBCharacter* character = new SBCharacter(charName, metaInfo);
		SBSkeleton* skeleton = new SBSkeleton();		
		character->setSkeleton(skeleton);		
		SkJoint* joint = skeleton->add_joint(SkJoint::TypeQuat);
		joint->setName("world_offset");		
		joint->update_gmat();			
		mcu.registerCharacter(character);

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
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SbmPawn* pawn = mcu.getPawn(pawnName);
	SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
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
		mcu.registerPawn(pawn);

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
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBCharacter* character = this->getCharacter(charName);
	if (character)
	{

		string vrProcEnd_msg = "vrProcEnd sbm ";
		vrProcEnd_msg += getName();
		mcu.vhmsg_send( vrProcEnd_msg.c_str() );

		// notify the services
		std::map<std::string, SmartBody::SBService*>& services = getServiceManager()->getServices();
		for (std::map<std::string, SmartBody::SBService*>::iterator iter = services.begin();
			iter != services.end();
			iter++)
		{
			SBService* service = (*iter).second;
			service->onCharacterDelete(character);
		}
	
		mcu.unregisterCharacter(character);

		delete character;
	}	
}

void SBScene::removeAllCharacters()
{
	std::vector<std::string> characters = getCharacterNames();
	for (std::vector<std::string>::iterator iter = characters.begin();
		 iter != characters.end();
		 iter++)
	{
		removeCharacter((*iter));
	}
}

void SBScene::removePawn(const std::string& pawnName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(pawnName);
	if (pawn)
	{
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
			mcu.unregisterPawn(pawn);

			delete pawn;
		}
	}	
}

void SBScene::removeAllPawns()
{
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::iterator iter = pawns.begin();
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
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	return mcu.getNumCharacters(); 
}

int SBScene::getNumPawns() 
{  
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.getNumPawns() - mcu.getNumCharacters(); 
}

SBPawn* SBScene::getPawn(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (pawn)
	{
		SBPawn* sbpawn = dynamic_cast<SBPawn*>(pawn);
		return sbpawn;
	}
	else
	{
		//LOG("pawn %s does not exist.", name.c_str());
		return NULL;
	}
}

SBCharacter* SBScene::getCharacter(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(name);
	if (character)
	{
		SBCharacter* sbcharacter = dynamic_cast<SBCharacter*>(character);
		return sbcharacter;
	}
	else
	{
		return NULL;
	}
}

SBSkeleton* SBScene::getSkeleton(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkSkeleton*>::iterator iter = mcu.skeleton_map.find(name);
	SkSkeleton* skskel = NULL;
	if (iter != mcuCBHandle::singleton().skeleton_map.end())
		skskel = iter->second;
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}

SBMotion* SBScene::getMotion(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* motion = mcu.getMotion(name);
	SBMotion* sbmotion = dynamic_cast<SBMotion*>(motion);	
	return sbmotion;
}

int SBScene::getNumMotions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.motion_map.size();
}

std::vector<std::string> SBScene::getMotionNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.begin();
		iter != mcu.motion_map.end();
		iter++)
	{
		SkMotion* motion = (*iter).second;
		ret.push_back(motion->getName());
	}

	return ret;
}



std::vector<std::string> SBScene::getPawnNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
		if (!character)
			ret.push_back(std::string(pawn->getName()));
	}

	return ret;
}

std::vector<std::string> SBScene::getCharacterNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* sbmCharacter = (*iter).second;
		ret.push_back(std::string(sbmCharacter->getName()));
	}

	return ret;
}


int SBScene::getNumSkeletons()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.getSkeletonMap().size();
}


std::vector<std::string> SBScene::getSkeletonNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SkSkeleton*>::iterator iter = mcu.getSkeletonMap().begin();
		iter != mcu.getSkeletonMap().end();
		iter++)
	{
		SkSkeleton* skeleton = (*iter).second;
		ret.push_back(std::string(iter->first));
	}

	return ret;	
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

std::vector<std::string> SBScene::getAssetPaths(const std::string& type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> list;
	srPathList* path = NULL;
	if (type == "seq" || type == "script")
	{
		path = &mcu.seq_paths;
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		path = &mcu.me_paths;
	}
	else if (type == "audio")
	{
		path = &mcu.audio_paths;
	}
	else if (type == "mesh")
	{
		path = &mcu.mesh_paths;
	}
	else
	{
		LOG("Unknown path type: %s", type.c_str());
		return list;
	}
	
	path->reset();
	std::string nextPath = path->next_path(false);
	while (nextPath != "")
	{
		list.push_back(nextPath);
		nextPath = path->next_path(false);
	}
	return list;
}

std::vector<std::string> SBScene::getLocalAssetPaths(const std::string& type)
{

	std::string mediaPath = getMediaPath();
	boost::filesystem::path mpath(mediaPath);
	std::string completeMediaPath =  boost::filesystem::complete(mpath).string();
	size_t mediaPathLength = completeMediaPath.size();

	std::vector<std::string> paths = getAssetPaths(type);

	std::vector<std::string> localPaths;

	// remove the media path
	for (std::vector<std::string>::iterator iter = paths.begin();
		 iter != paths.end();
		 iter++)
	{
		boost::filesystem::path path((*iter));
		std::string completePath = boost::filesystem::complete( path ).string();
		size_t loc = completePath.find(completeMediaPath);
		if (loc == 0)
		{
			localPaths.push_back(completePath.substr(mediaPathLength + 1));
		}
		else
		{
			localPaths.push_back((*iter));
		}
	}

	return localPaths;

}

void SBScene::addAssetPath(const std::string& type, const std::string& path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	PathResource* pres = new PathResource();
	pres->setPath(path);
	if (type == "seq" || type == "script")
	{
		pres->setType("seq");
		mcu.seq_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		pres->setType("me");
		mcu.me_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		pres->setType("audio");
		mcu.audio_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "mesh")
	{
		pres->setType("mesh");
		mcu.mesh_paths.insert(const_cast<char *>(path.c_str()));
	}
	else
	{
		delete pres;
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
	mcu.resource_manager->addResource(pres);
}

void SBScene::removeAssetPath(const std::string& type, const std::string& path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	bool ret = false;
	if (type == "seq" || type == "script")
	{
		ret = mcu.seq_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		ret = mcu.me_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		ret = mcu.audio_paths.remove(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}

	if (ret)
	{
		// remove the resource from the resource manager
	}
}

void SBScene::removeAllAssetPaths(const std::string& type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	bool ret = false;
	if (type == "seq" || type == "script")
	{
		 mcu.seq_paths.removeAll();
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		mcu.me_paths.removeAll();
	}
	else if (type == "audio")
	{
		mcu.audio_paths.removeAll();
	}
	else if (type == "mesh")
	{
		mcu.mesh_paths.removeAll();
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}

	if (ret)
	{
		// remove the resource from the resource manager
	}
}

void SBScene::loadAssets()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.me_paths.reset();

	std::string path = mcu.me_paths.next_path(false);
	while (path != "")
	{
		mcu.load_motions(path.c_str(), true);
		mcu.load_skeletons(path.c_str(), true);
		path = mcu.me_paths.next_path(false);
	}
}

void SBScene::loadAsset(const std::string& assetPath)
{
	const std::string& mediaPath = this->getMediaPath();
	boost::filesystem::path p( mediaPath );
	boost::filesystem::path assetP( assetPath );

	boost::filesystem::path abs_p = boost::filesystem::complete( assetP );	

	if( boost::filesystem2::exists( abs_p ))
	{
		p = assetP;
	}
	else
	{
		p /= assetP;
	}
	boost::filesystem::path final = boost::filesystem::complete( p );
	std::string finalPath = p.string();

	// make sure the file exists and is readable
	std::ifstream file(finalPath.c_str());
	if (!file.good())
	{
		LOG("File %s cannot be read, asset will not be loaded.", finalPath.c_str());
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	std::string ext = boost::filesystem::extension( finalPath );
	std::string baseName = boost::filesystem::basename( finalPath );
	std::string fileName = baseName+ext;
	// determine the type of asset: skeleton, motion, mesh, texture, ...
	if( _stricmp( ext.c_str(), ".skm" ) == 0)
	{
		FILE* myfile = fopen(finalPath.c_str(), "rt");
		SrInput in( myfile );
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = motion->load( in, 1.0 );
		if (parseSuccessful)
			mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motion->getName(), motion));
		else
			delete motion;
		return;
	}

	if( _stricmp( ext.c_str(), ".bvh" ) == 0)
	{
		SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = ParserBVH::parse(*skeleton, *motion, finalPath, file, 1.0);
		if (parseSuccessful)
		{
			mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motion->getName(), motion));
			mcu.skeleton_map.insert(std::pair<std::string, SkSkeleton*>(skeleton->getName(), skeleton));
		}
		else
		{
			delete motion;
			delete skeleton;
		}
			
		return;
	}

	if( _stricmp( ext.c_str(), ".sk" ) == 0)
	{
		FILE* myfile = fopen(finalPath.c_str(), "rt");
		SrInput input(myfile);
		SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();
		SkSkeleton* skel = skeleton;
		if( skel->load( input, 1.0) )
		{
			skeleton->ref();
			skeleton->setFileName(fileName);
			skeleton->setName(skeleton->getFileName());
			mcu.skeleton_map.insert(std::pair<std::string, SkSkeleton*>(skel->getName(), skeleton));
		}
		else
		{
			delete skeleton;
		}
		return;
	}

	if( _stricmp( ext.c_str(), ".dae" ) == 0)
	{

	}

	if( _stricmp( ext.c_str(), ".xml" ) == 0)
	{

	}

	if( _stricmp( ext.c_str(), ".asf" ) == 0)
	{

	}
	if( _stricmp( ext.c_str(), ".amc" ) == 0)
	{

	}
}

void SBScene::loadAssetsFromPath(const std::string& assetPath)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.load_motions(assetPath.c_str(), true);
	mcu.load_skeletons(assetPath.c_str(), true);
}

SBSkeleton* SBScene::addSkeletonDefinition(const std::string& skelName )
{
	SBSkeleton* sbSkel = new SBSkeleton();
	SkSkeleton* skSkel = sbSkel;	
	sbSkel->setName(skelName);
	skSkel->skfilename(skelName.c_str());
	mcuCBHandle::singleton().skeleton_map.insert(std::pair<std::string, SkSkeleton*>(sbSkel->getName(), skSkel));
	return sbSkel;
}

SBMotion* SBScene::addMotionDefinition(const std::string& motionName, double duration )
{
	SBMotion* sbMotion = new SBMotion();
	if (duration > 0)
	{
		sbMotion->insert_frame(0,0.f);
		sbMotion->insert_frame(1,(float)duration);
	}	
	sbMotion->setName(motionName);	
	//mcuCBHandle::singleton().motion_map.insert(std::pair<std::string, SkMotion*>(motionName, sbMotion));
	mcuCBHandle::singleton().motion_map[motionName] = sbMotion;
	return sbMotion;
}

void SBScene::addPose(const std::string& path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_poses(path.c_str(), recursive);
}

void SBScene::addMotion(const std::string& path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_motions(path.c_str(), recursive);
}

void SBScene::setMediaPath(const std::string& path)
{
	_mediaPath = path;
	// update all the paths with the media path prefix
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.seq_paths.setPathPrefix(_mediaPath);
	mcu.me_paths.setPathPrefix(_mediaPath);
	mcu.audio_paths.setPathPrefix(_mediaPath);
	mcu.mesh_paths.setPathPrefix(_mediaPath);
}

const std::string& SBScene::getMediaPath()
{
	return _mediaPath;
}

void SBScene::setDefaultCharacter(const std::string& character)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.test_character_default = character;
}

void SBScene::setDefaultRecipient(const std::string& recipient)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.test_recipient_default = recipient;
}

SBSkeleton* SBScene::createSkeleton(const std::string& skeletonDefinition)
{
	SBSkeleton* skeleton = NULL;
	SBSkeleton* templateSkeleton = this->getSkeleton(skeletonDefinition);
	if (templateSkeleton)
	{
		skeleton = new SBSkeleton(templateSkeleton);
	}
	else
	{
		skeleton = new SBSkeleton(skeletonDefinition);
	}

	return skeleton;
	
}



SBEventManager* SBScene::getEventManager()
{
	return _eventManager;
}


bool SBScene::command(const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int ret = mcu.execute((char*) command.c_str());

	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
}

bool SBScene::commandAt(float seconds, const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int ret = mcu.execute_later((char*) command.c_str(), seconds);

	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
}

void SBScene::removePendingCommands()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.activeSequences.clear();
	mcu.pendingSequences.clear();
}

void SBScene::sendVHMsg(const std::string& message)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str());
}

void SBScene::sendVHMsg2(const std::string& message, const std::string& message2)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str(), message2.c_str());
}

bool SBScene::run(const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int ret = mcu.executePython(command.c_str());

	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
}

bool SBScene::runScript(const std::string& script)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int ret = mcu.executePythonFile(script.c_str());
	
	if (ret == CMD_SUCCESS)
		return true;
	else
		return false;
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();

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
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::iterator pawnIter = pawns.begin();
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
	std::vector<std::string> characters = getCharacterNames();
	for (std::vector<std::string>::iterator characterIter = characters.begin();
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
		std::vector<std::string> charNames = getCharacterNames();
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
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::iterator pawnIter = pawns.begin();
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
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::iterator pawnIter = pawns.begin();
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
	std::vector<std::string> characters = getCharacterNames();
	for (std::vector<std::string>::iterator characterIter = characters.begin();
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
	std::vector<std::string> characters = getCharacterNames();
	for (std::vector<std::string>::iterator characterIter = characters.begin();
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
	std::vector<std::string> pawns = getPawnNames();
	for (std::vector<std::string>::iterator pawnIter = pawns.begin();
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmPawn* pawn = mcu.getPawn(name);
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
	SBSkeleton* skeleton = new SBSkeleton();
	camera->setSkeleton(skeleton);
	SkJoint* joint = skeleton->add_joint(SkJoint::TypeQuat);
	joint->setName("world_offset");

	_cameras.insert(std::pair<std::string, SrCamera*>(name, camera));
	mcu.registerPawn(camera);

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
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmPawn* pawn = mcu.getPawn(camera->getName());
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();
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



};
