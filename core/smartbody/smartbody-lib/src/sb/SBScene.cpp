
#include "vhcl.h"
#include "sbm/mcontrol_util.h"
#include "SBScene.h"
#include <sb/SBObject.h>
#include <sb/SBCharacter.h>
#include <sb/SBMotion.h>
#include <sb/SBScript.h>
#include <sbm/Event.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBReachManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBServiceManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBSkeleton.h>
#include <sb/SBParser.h>
#include <sbm/SbmDebuggerServer.h>
#include <sbm/SbmDebuggerClient.h>
#include <sbm/SbmDebuggerUtility.h>
#include <sbm/sbm_audio.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <sbm/nvbg.h>

namespace SmartBody {

SBScene::SBScene(void)
{
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
	_scale = .01f; // default scale is centimeters

	// add the services
	_serviceManager->addService(_steerManager);
	_serviceManager->addService(_physicsManager);
	_serviceManager->addService(_boneBusManager);
	_serviceManager->addService(_collisionManager);

	_parser = new SBParser();

	_debuggerServer = new SbmDebuggerServer();
	_debuggerClient = new SbmDebuggerClient();
	_debuggerUtility = new SbmDebuggerUtility();
	_isRemoteMode = false;

	createBoolAttribute("internalAudio",false,true,"",10,false,false,false,"Use SmartBody's internal audio player.");
	createStringAttribute("speechRelaySoundCacheDir","../../../..",true,"",20,false,false,false,"Directory where sound files from speech relays will be placed. ");
	createDoubleAttribute("scale",.01,true,"",30,false,false,false,"The scale of scene (1 = meters, .01 = centimeters, etc).");
	createIntAttribute("colladaTrimFrames",0,true,"",40,false,false,false,"Number of frames to be trimmed in the front when loading a collada motion.");
	createBoolAttribute("useFastXMLParsing",false,true,"",50,false,false,false,"Use faster parsing when reading XML from a file.");
	createBoolAttribute("delaySpeechIfNeeded",true,true,"",60,false,false,false,"Delays any speech until other behaviors specified in the same BML need to execute beforehand. This can occur when a gesture is synchronized to a word early in the utterance, and the gesture motion needs to be played for awhile before the synch point.");
}

SBScene::~SBScene(void)
{
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

	delete _parser;

	_debuggerClient->Disconnect();
	_debuggerServer->Close();
	delete _debuggerServer;  // TODO: should delete these in reverse order?
	delete _debuggerClient;
	delete _debuggerUtility;
}

SBScene* SBScene::getScene()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu._scene;
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
	// stop the simulation
	getSimulationManager()->stop();

	// remove the characters
	removeAllCharacters();
	
	// remove the pawns
	removeAllPawns();

	// clear the joint maps
	getJointMapManager()->removeAllJointMaps();

	// remove all blends and transitions
	getBlendManager()->removeAllBlends();
	getBlendManager()->removeAllTransitions();

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

SBCharacter* SBScene::createCharacter(std::string charName, std::string metaInfo)
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

SBPawn* SBScene::createPawn(std::string pawnName)
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

void SBScene::removeCharacter(std::string charName)
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

void SBScene::removePawn(std::string pawnName)
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

SBPawn* SBScene::getPawn(std::string name)
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

SBCharacter* SBScene::getCharacter(std::string name)
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
		LOG("Character %s does not exist.", name.c_str());
		return NULL;
	}
}

SBSkeleton* SBScene::getSkeleton(std::string name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkSkeleton*>::iterator iter = mcu.skeleton_map.find(name);
	SkSkeleton* skskel = NULL;
	if (iter != mcuCBHandle::singleton().skeleton_map.end())
		skskel = iter->second;
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}

SBMotion* SBScene::getMotion(std::string name)
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


std::vector<std::string> SBScene::getSkeletonNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SkSkeleton*>::iterator iter = mcu.getSkeletonMap().begin();
		iter != mcu.getSkeletonMap().end();
		iter++)
	{
		SkSkeleton* skeleton = (*iter).second;
		ret.push_back(std::string(skeleton->name()));
	}

	return ret;	
}

std::vector<std::string> SBScene::getEventHandlerNames()
{
	EventManager* eventManager = getEventManager();
	
	std::vector<std::string> ret;

	for(EventHandlerMap::iterator iter = eventManager->getEventHandlers().begin();
		iter != eventManager->getEventHandlers().end();
		iter++)
	{

		ret.push_back(std::string(iter->first));
	}
	return ret;
}

std::vector<std::string> SBScene::getAssetPaths(std::string type)
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
	std::string nextPath = path->next_path();
	while (nextPath != "")
	{
		list.push_back(nextPath);
		nextPath = path->next_path();
	}
	return list;
}


void SBScene::addAssetPath(std::string type, std::string path)
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

void SBScene::removeAssetPath(std::string type, std::string path)
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

void SBScene::loadAssetsFromPath(std::string assetPath)
{
	const std::string& mediaPath = this->getMediaPath();
	boost::filesystem::path p( mediaPath );
	p /= assetPath;
	boost::filesystem::path abs_p = boost::filesystem::complete( p );

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::string finalPath = abs_p.string();
	mcu.load_motions(finalPath.c_str(), true);
	mcu.load_skeletons(finalPath.c_str(), true);
}

void SBScene::addPose(std::string path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_poses(path.c_str(), recursive);
}

void SBScene::addMotion(std::string path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_motions(path.c_str(), recursive);
}

void SBScene::setMediaPath(std::string path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.setMediaPath(path);
}

const std::string& SBScene::getMediaPath()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.getMediaPath();
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

SBSkeleton* SBScene::createSkeleton(std::string skeletonDefinition)
{
	SBSkeleton* skeleton = new SBSkeleton(skeletonDefinition);
	return skeleton;
}



EventManager* SBScene::getEventManager()
{
	return EventManager::getEventManager();
}




void SBScene::command(const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.execute((char*) command.c_str());
}

void SBScene::commandAt(float seconds, const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.execute_later((char*) command.c_str(), seconds);
}

void SBScene::removePendingCommands()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.activeSequences.clear();
	mcu.pendingSequences.clear();
}

void SBScene::sendVHMsg(std::string message)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str());
}

void SBScene::sendVHMsg2(std::string message, std::string message2)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str(), message2.c_str());
}

void SBScene::run(std::string command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.executePython(command.c_str());
}

void SBScene::runScript(std::string script)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.executePythonFile(script.c_str());
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// make sure the name doesn't already exist
	if (mcu.face_map.find(name) != mcu.face_map.end())
	{
		LOG("Face definition named '%s' already exists.", name.c_str());
		return NULL;
	}

	SBFaceDefinition* face = new SBFaceDefinition(name);
	mcu.face_map.insert(std::pair<std::string, SBFaceDefinition*>(name, face));

	return face;
}

void SBScene::removeFaceDefinition(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// make sure the name doesn't already exist
	std::map<std::string, SBFaceDefinition*>::iterator iter = mcu.face_map.find(name);
	if (iter == mcu.face_map.end())
	{
		LOG("Face definition named '%s' does not exist.", name.c_str());
		return;
	}

	delete iter->second;
	iter->second = NULL;
	mcu.face_map.erase(iter);
}

SmartBody::SBFaceDefinition* SBScene::getFaceDefinition(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// make sure the name doesn't already exist
	std::map<std::string, SBFaceDefinition*>::iterator iter = mcu.face_map.find(name);
	if (iter == mcu.face_map.end())
	{
		LOG("Face definition named '%s' does not exist.", name.c_str());
		return NULL;
	}

	return (*iter).second;
}

int SBScene::getNumFaceDefinitions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return 	mcu.face_map.size();
}

std::vector<std::string> SBScene::getFaceDefinitionNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> faces;
	for (std::map<std::string, SBFaceDefinition*>::iterator iter =  mcu.face_map.begin();
		 iter !=  mcu.face_map.end();
		 iter++)
	{
		faces.push_back((*iter).second->getName());
	}

	return faces;
}

void SBScene::addScript(std::string name, SBScript* script)
{
	std::map<std::string, SBScript*>::iterator iter = _scripts.find(name);
	if (iter != _scripts.end())
	{
		LOG("Script with name %s already exists.", name.c_str());
		return;
	}

	_scripts.insert(std::pair<std::string, SBScript*>(name, script));
}

void SBScene::removeScript(std::string name)
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

SBScript* SBScene::getScript(std::string name)
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


};
