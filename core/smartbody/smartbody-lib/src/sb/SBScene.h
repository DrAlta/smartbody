#ifndef SBSCENE_H
#define SBSCENE_H

#include <sb/SBTypes.h>
#include <sb/SBObject.h>
#include <sb/SBScript.h>

class SbmDebuggerServer;
class SbmDebuggerClient;
class SbmDebuggerUtility;

namespace SmartBody {

class SBPawn;
class SBCharacter;
class SBSkeleton;
class SBFaceDefinition;
class SBMotion;
class EventManager;
class SBSimulationManager;
class SBProfiler;
class SBBmlProcessor;
class SBAnimationBlendManager;
class SBReachManager;
class SBSteerManager;
class SBServiceManager;
class SBPhysicsManager;
class SBBoneBusManager;
class SBGestureMapManager;
class SBJointMapManager;
class SBCollisionManager;
class SBDiphoneManager;
class SBBehaviorSetManager;
class SBParser;
class SBSubject;

class SBScene : public SBObject
{
	public:
		SBAPI SBScene(void);
		SBAPI ~SBScene(void);

		SBAPI void update();
		SBAPI std::string save(bool remoteSetup = false);
		SBAPI void exportScene(const std::string& filename);
		SBAPI static SBScene* getScene();

		SBAPI void setScale(float val);
		SBAPI float getScale();

		SBAPI void command(const std::string& command);
		SBAPI void commandAt(float seconds, const std::string& command);
		SBAPI void sendVHMsg(std::string message);
		SBAPI void sendVHMsg2(std::string messageType, std::string encodedMessage);		

		SBAPI SBCharacter* createCharacter(std::string charName, std::string metaInfo);
		SBAPI SBPawn* createPawn(std::string);
		SBAPI void removeAllCharacters();
		SBAPI void removeCharacter(std::string charName);
		SBAPI void removePawn(std::string pawnName);
		SBAPI void removeAllPawns();
		SBAPI int getNumCharacters();
		SBAPI int getNumPawns();
		SBAPI SBCharacter* getCharacter(std::string name);
		SBAPI SBSkeleton* getSkeleton(std::string name);		
		SBAPI SBPawn* getPawn(std::string name);

		SBAPI SBFaceDefinition* createFaceDefinition(const std::string& name);
		SBAPI void removeFaceDefinition(const std::string& name);
		SBAPI SBFaceDefinition* getFaceDefinition(const std::string& name);
		SBAPI int getNumFaceDefinitions();
		SBAPI std::vector<std::string> getFaceDefinitionNames();

		SBAPI std::vector<std::string> getPawnNames();
		SBAPI std::vector<std::string> getCharacterNames();

		SBAPI void removePendingCommands();
		
		SBAPI SBSkeleton* createSkeleton(std::string char_name);

		SBAPI SBMotion* getMotion(std::string name);
		SBAPI int getNumMotions();
		SBAPI std::vector<std::string> getMotionNames();
		SBAPI std::vector<std::string> getSkeletonNames();
		SBAPI std::vector<std::string> getEventHandlerNames();

		SBAPI void setMediaPath(std::string path);
		SBAPI const std::string& getMediaPath();
		SBAPI void addAssetPath(std::string type, std::string path);
		SBAPI std::vector<std::string> getAssetPaths(std::string type);
		SBAPI std::vector<std::string> getLocalAssetPaths(std::string type);
		SBAPI void removeAssetPath(std::string type, std::string path);
		SBAPI void removeAllAssetPaths(std::string type);
		SBAPI void loadAssets();
		SBAPI void loadAssetsFromPath(std::string assetPath);
		SBAPI void loadMotions();
		SBAPI void addPose(std::string path, bool recursive);
		SBAPI void addMotion(std::string path, bool recursive);
		SBAPI SBSkeleton* addSkeletonDefinition(std::string skelName);
		SBAPI SBMotion* addMotionDefinition(std::string motionName, double duration);			
				
		SBAPI void run(std::string command);
		SBAPI void runScript(std::string script);

		SBAPI void setDefaultCharacter(const std::string& character);
		SBAPI void setDefaultRecipient(const std::string& recipient);

		SBAPI void reset();

		SBAPI void addScript(std::string name, SBScript* script);
		SBAPI void removeScript(std::string name);
		SBAPI int getNumScripts();
		SBAPI SBScript* getScript(std::string name);
		SBAPI std::vector<std::string> getScriptNames();
		SBAPI std::map<std::string, SBScript*>& getScripts();

		SBAPI EventManager* getEventManager();		
		SBAPI SBSimulationManager* getSimulationManager();
		SBAPI SBProfiler* getProfiler();
		SBAPI SBBmlProcessor* getBmlProcessor();
		SBAPI SBAnimationBlendManager* getBlendManager();
		SBAPI SBReachManager* getReachManager();
		SBAPI SBSteerManager* getSteerManager();
		SBAPI SBServiceManager* getServiceManager();
		SBAPI SBPhysicsManager* getPhysicsManager();
		SBAPI SBBoneBusManager* getBoneBusManager();
		SBAPI SBGestureMapManager* getGestureMapManager();
		SBAPI SBJointMapManager* getJointMapManager();
		SBAPI SBCollisionManager* getCollisionManager();
		SBAPI SBDiphoneManager* getDiphoneManager();
		SBAPI SBBehaviorSetManager* getBehaviorSetManager();

		SBAPI SBParser* getParser();

		SBAPI SbmDebuggerServer * getDebuggerServer() { return _debuggerServer; }
		SBAPI SbmDebuggerClient * getDebuggerClient() { return _debuggerClient; }
		SBAPI SbmDebuggerUtility* getDebuggerUtility() { return _debuggerUtility; }
		SBAPI bool isRemoteMode();
		SBAPI void setRemoteMode(bool val);

		SBAPI void notify(SBSubject* subject);

	protected:
		SBSimulationManager* _sim;
		SBProfiler* _profiler;
		SBBmlProcessor* _bml;
		SBAnimationBlendManager* _blendManager;
		SBReachManager* _reachManager;
		SBSteerManager* _steerManager;
		SBServiceManager* _serviceManager;
		SBPhysicsManager* _physicsManager;
		SBBoneBusManager* _boneBusManager;
		SBGestureMapManager* _gestureMapManager;
		SBJointMapManager* _jointMapManager;
		SBCollisionManager* _collisionManager;
		SBDiphoneManager* _diphoneManager;
		SBParser* _parser;

		SBBehaviorSetManager* _behaviorSetManager;

		std::map<std::string, SBScript*> _scripts;
		float _scale;
		bool _isRemoteMode;

		SbmDebuggerServer*	_debuggerServer;
		SbmDebuggerClient*	_debuggerClient;
		SbmDebuggerUtility*	_debuggerUtility;
};

SBScene* getScene();

};



#endif
