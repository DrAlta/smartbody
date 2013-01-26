#ifndef SBSCENE_H
#define SBSCENE_H

#include <vhcl.h>
#include <sb/SBTypes.h>
#include <sb/SBObject.h>
#include <sb/SBScript.h>

#include <map>
#include <sstream>

class SBDebuggerServer;
class SBDebuggerClient;
class SBDebuggerUtility;
class SrCamera;

namespace SmartBody {

class SBCharacterListener;
class SBPawn;
class SBCharacter;
class SBSkeleton;
class SBFaceDefinition;
class SBMotion;
class SBEventManager;
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
class SBRetargetManager;
class SBParser;
class SBSubject;
class SBController;

class SBScene : public SBObject
{
	public:
		SBAPI SBScene(void);
		SBAPI ~SBScene(void);

		SBAPI void setProcessId(const std::string& id);
		SBAPI const std::string& getProcessId();

		SBAPI void update();
		SBAPI std::string save(bool remoteSetup = false);
		SBAPI std::string saveSceneSetting();
		SBAPI std::string exportScene(const std::vector<std::string>& aspects, bool remoteSetup);
		SBAPI static SBScene* getScene();		
		SBAPI static void destroyScene();

		SBAPI void setScale(float val);
		SBAPI float getScale();

		SBAPI bool command(const std::string& command);
		SBAPI bool commandAt(float seconds, const std::string& command);
		SBAPI void sendVHMsg(const std::string& message);
		SBAPI void sendVHMsg2(const std::string&, const std::string& encodedMessage);		

		SBAPI SBCharacter* createCharacter(const std::string&, const std::string&);
		SBAPI SBPawn* createPawn(const std::string& pawnName);
		SBAPI void removeAllCharacters();
		SBAPI void removeCharacter(const std::string& charName);
		SBAPI void removePawn(const std::string& pawnName);
		SBAPI void removeAllPawns();
		SBAPI int getNumCharacters();
		SBAPI int getNumPawns();
		SBAPI SBCharacter* getCharacter(const std::string& name);
		SBAPI SBSkeleton* getSkeleton(const std::string& name);		
		SBAPI SBPawn* getPawn(const std::string& name);

		SBAPI SBFaceDefinition* createFaceDefinition(const std::string& name);
		SBAPI void removeFaceDefinition(const std::string& name);
		SBAPI SBFaceDefinition* getFaceDefinition(const std::string& name);
		SBAPI int getNumFaceDefinitions();
		SBAPI std::vector<std::string> getFaceDefinitionNames();

		SBAPI std::vector<std::string> getPawnNames();
		SBAPI std::vector<std::string> getCharacterNames();

		SBAPI void removePendingCommands();
		
		SBAPI SBSkeleton* createSkeleton(const std::string&char_name);

		SBAPI SBMotion* getMotion(const std::string& name);
		SBAPI int getNumMotions();
		SBAPI std::vector<std::string> getMotionNames();
		SBAPI int getNumSkeletons();
		SBAPI std::vector<std::string> getSkeletonNames();
		SBAPI std::vector<std::string> getEventHandlerNames();

		SBAPI void setMediaPath(const std::string& path);
		SBAPI const std::string& getMediaPath();
		SBAPI void addAssetPath(const std::string& type, const std::string& path);
		SBAPI std::vector<std::string> getAssetPaths(const std::string& type);
		SBAPI std::vector<std::string> getLocalAssetPaths(const std::string& type);
		SBAPI void removeAssetPath(const std::string& type, const std::string& path);
		SBAPI void removeAllAssetPaths(const std::string& type);
		SBAPI void loadAssets();
		SBAPI void loadAsset(const std::string& assetPath);
		SBAPI void loadAssetsFromPath(const std::string& assetPath);
		SBAPI void loadMotions();
		SBAPI void addPose(const std::string& path, bool recursive);
		SBAPI void addMotion(const std::string& path, bool recursive);
		SBAPI SBSkeleton* addSkeletonDefinition(const std::string& skelName);
		SBAPI SBMotion* addMotionDefinition(const std::string& motionName, double duration);			
				
		SBAPI bool run(const std::string& command);
		SBAPI bool runScript(const std::string& script);

		SBAPI void setDefaultCharacter(const std::string& character);
		SBAPI void setDefaultRecipient(const std::string& recipient);

		SBAPI void reset();

		SBAPI void addScript(const std::string& name, SBScript* script);
		SBAPI void removeScript(const std::string& name);
		SBAPI int getNumScripts();
		SBAPI SBScript* getScript(const std::string& name);
		SBAPI std::vector<std::string> getScriptNames();
		SBAPI std::map<std::string, SBScript*>& getScripts();

		SBAPI SBEventManager* getEventManager();		
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
		SBAPI SBRetargetManager* getRetargetManager();

		SBAPI SBParser* getParser();

		SBAPI SBDebuggerServer * getDebuggerServer();
		SBAPI SBDebuggerClient * getDebuggerClient();
		SBAPI SBDebuggerUtility* getDebuggerUtility();
		SBAPI bool isRemoteMode();
		SBAPI void setRemoteMode(bool val);

		SBAPI void setCharacterListener(SBCharacterListener* listener);
		SBAPI SBCharacterListener* getCharacterListener();

		SBAPI void notify(SBSubject* subject);

		SBAPI static void setSystemParameter(const std::string& name, const std::string& value);
		SBAPI static std::string getSystemParameter(const std::string& name);
		SBAPI static void removeSystemParameter(const std::string& name);
		SBAPI static void removeAllSystemParameters();
		SBAPI static std::vector<std::string> getSystemParameterNames();

		SBAPI SrCamera* createCamera(const std::string& name);
		SBAPI void removeCamera(SrCamera* camera);
		SBAPI void setActiveCamera(SrCamera* camera);
		SBAPI SrCamera* getActiveCamera();
		SBAPI SrCamera* getCamera(const std::string& name);
		SBAPI int getNumCameras();
		SBAPI std::vector<std::string> getCameraNames();
		SBAPI std::vector<SBController*>& getDefaultControllers();

	protected:

		void initialize();
		void cleanup();
		void saveScene(std::stringstream& strstr, bool remoteSetup);
		void saveAssets(std::stringstream& strstr, bool remoteSetup);
		void saveCameras(std::stringstream& strstr, bool remoteSetup);
		void savePawns(std::stringstream& strstr, bool remoteSetup);
		void saveCharacters(std::stringstream& strstr, bool remoteSetup);
		void saveLights(std::stringstream& strstr, bool remoteSetup);
		void saveBlends(std::stringstream& strstr, bool remoteSetup);
		void saveJointMaps(std::stringstream& strstr, bool remoteSetup);
		void saveFaceDefinitions(std::stringstream& strstr, bool remoteSetup);
		void saveGestureMaps(std::stringstream& strstr, bool remoteSetup);
		void saveLipSyncing(std::stringstream& strstr, bool remoteSetup);
		void saveServices(std::stringstream& strstr, bool remoteSetup);
		void savePositions(std::stringstream& strstr, bool remoteSetup);

		void createDefaultControllers();
		void removeDefaultControllers();


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
		SBRetargetManager* _retargetManager;
		SBEventManager* _eventManager;
		SBParser* _parser;

		SBCharacterListener* _characterListener;

		SBBehaviorSetManager* _behaviorSetManager;

		std::map<std::string, SBScript*> _scripts;
		float _scale;
		bool _isRemoteMode;
		static bool _firstTime;

		SBDebuggerServer*	_debuggerServer;
		SBDebuggerClient*	_debuggerClient;
		SBDebuggerUtility*	_debuggerUtility;
		std::map<std::string, SrCamera*> _cameras;
		std::string _activeCamera;

		std::string _mediaPath;
		std::vector<SBController*> _defaultControllers;
		

		static SBScene* _scene;
		static std::map<std::string, std::string> _systemParameters;
};

SBScene* getScene();

};



#endif
