
#include "vhcl.h"

#include "SBScene.h"
#include "sbm/mcontrol_util.h"

#include "SbmDebuggerServer.h"


namespace SmartBody {

SBScene::SBScene(void)
{
	_sim = new SBSimulationManager();
	_profiler = new SBProfiler();
	_bml = new SBBmlProcessor();
	_stateManager = new SBAnimationStateManager();
	_reachManager = new SBReachManager();
	_steerManager = new SBSteerManager();
	_serviceManager = new SBServiceManager();
	_physicsManager = new SBPhysicsManager();
	_scale = .01f; // default scale is centimeters

	// add the services
	_serviceManager->addService(_steerManager);
	_serviceManager->addService(_physicsManager);

	_parser = new SBParser();

	_debuggerServer = new SbmDebuggerServer();

	createBoolAttribute("internalAudio",false,true,"",10,false,false,false,"Use SmartBody's internal audio player.");
	createDoubleAttribute("scale",false,true,"",10,false,false,false,"The scale of scene (1 = meters, .01 = centimeters, etc).");
}

SBScene::~SBScene(void)
{
	for (std::map<std::string, SBScript*>::iterator iter = _scripts.begin();
		 iter != _scripts.end();
		 iter++)
	{
		delete (*iter).second;
	}

	delete _sim;
	delete _profiler;
	delete _bml;
	delete _stateManager;
	delete _reachManager;
	delete _steerManager;
	delete _physicsManager;

	delete _parser;

	delete _debuggerServer;  // TODO: should delete these in reverse order?
}

void SBScene::setScale(float val)
{
	_scale = val;
}

float SBScene::getScale()
{
	return _scale;
}


void SBScene::notify( SBSubject* subject )
{
	BoolAttribute* boolAttr = dynamic_cast<BoolAttribute*>(subject);

	if (boolAttr && boolAttr->getName() == "internalAudio")
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		mcu.play_internal_audio = boolAttr->getValue();		
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
		mcu.registerCharacter(character);
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
		return pawn;
	}
}

void SBScene::removeCharacter(std::string charName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		mcu.unregisterCharacter(character);

		string vrProcEnd_msg = "vrProcEnd sbm ";
		vrProcEnd_msg += getName();
		mcu.vhmsg_send( vrProcEnd_msg.c_str() );

		delete character;
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
			mcu.unregisterPawn(pawn);
			delete pawn;
		}
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
	LOG("Python calls getNumPawns");
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

std::vector<std::string> SBScene::getAssetPaths(std::string type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> list;
	srPathList* path = NULL;
	if (type == "seq")
	{
		path = &mcu.seq_paths;
	}
	else if (type == "me" || type == "ME")
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
	if (type == "seq")
	{
		pres->setType("seq");
		mcu.seq_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME")
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
	if (type == "seq")
	{
		ret = mcu.seq_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME")
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

SBAnimationStateManager* SBScene::getStateManager()
{
	return _stateManager;
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

SBPhysicsManager* SBScene::getPhysicsManager()
{
	return _physicsManager;
}

SBParser* SBScene::getParser()
{
	return _parser;
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
