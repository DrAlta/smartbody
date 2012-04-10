#ifndef SBSCENE_H
#define SBSCENE_H

#include <sbm/SBObject.h>

class SbmDebuggerServer;

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
class SBAnimationStateManager;
class SBReachManager;
class SBSteerManager;
class SBServiceManager;
class SBPhysicsManager;
class SBBoneBusManager;
class SBGestureMapManager;
class SBJointMapManager;
class SBParser;
class SBScript;
class SBSubject;

class SBScene : public SBObject
{
	public:
		SBScene(void);
		~SBScene(void);
		void update();

		static SBScene* getScene();

		void setScale(float val);
		float getScale();

		void command(const std::string& command);
		void commandAt(float seconds, const std::string& command);
		void sendVHMsg(std::string message);
		void sendVHMsg2(std::string messageType, std::string encodedMessage);

		SBCharacter* createCharacter(std::string charName, std::string metaInfo);
		SBPawn* createPawn(std::string);
		void removeCharacter(std::string charName);
		void removePawn(std::string pawnName);
		int getNumCharacters();
		int getNumPawns();
		SBCharacter* getCharacter(std::string name);
		SBSkeleton* getSkeleton(std::string name);
		SBPawn* getPawn(std::string name);

		SBFaceDefinition* createFaceDefinition(const std::string& name);
		SBFaceDefinition* getFaceDefinition(const std::string& name);
		int getNumFaceDefinitions();
		std::vector<std::string> getFaceDefinitionNames();

		std::vector<std::string> getPawnNames();
		std::vector<std::string> getCharacterNames();
		

		SBSkeleton* createSkeleton(std::string char_name);

		SBMotion* getMotion(std::string name);
		int getNumMotions();
		std::vector<std::string> getMotionNames();
		std::vector<std::string> getSkeletonNames();
		std::vector<std::string> getEventHandlerNames();

		void setMediaPath(std::string path);
		void addAssetPath(std::string type, std::string path);
		std::vector<std::string> getAssetPaths(std::string type);
		void removeAssetPath(std::string type, std::string path);
		void loadAssets();
		void loadMotions();
		void addPose(std::string path, bool recursive);
		void addMotion(std::string path, bool recursive);
		
		void runScript(std::string script);

		void setDefaultCharacter(const std::string& character);
		void setDefaultRecipient(const std::string& recipient);

		void addScript(std::string name, SBScript* script);
		void removeScript(std::string name);
		int getNumScripts();
		SBScript* getScript(std::string name);
		std::vector<std::string> getScriptNames();
		std::map<std::string, SBScript*>& getScripts();

		EventManager* getEventManager();		
		SBSimulationManager* getSimulationManager();
		SBProfiler* getProfiler();
		SBBmlProcessor* getBmlProcessor();
		SBAnimationStateManager* getStateManager();
		SBReachManager* getReachManager();
		SBSteerManager* getSteerManager();
		SBServiceManager* getServiceManager();
		SBPhysicsManager* getPhysicsManager();
		SBBoneBusManager* getBoneBusManager();
		SBGestureMapManager* getGestureMapManager();
		SBJointMapManager* getJointMapManager();

		SBParser* getParser();

		SbmDebuggerServer * getDebuggerServer() { return _debuggerServer; }

		void notify(SBSubject* subject);

	protected:
		SBSimulationManager* _sim;
		SBProfiler* _profiler;
		SBBmlProcessor* _bml;
		SBAnimationStateManager* _stateManager;
		SBReachManager* _reachManager;
		SBSteerManager* _steerManager;
		SBServiceManager* _serviceManager;
		SBPhysicsManager* _physicsManager;
		SBBoneBusManager* _boneBusManager;
		SBGestureMapManager* _gestureMapManager;
		SBJointMapManager* _jointMapManager;
		SBParser* _parser;

		std::map<std::string, SBScript*> _scripts;
		float _scale;

		SbmDebuggerServer* _debuggerServer;
};

SBScene* getScene();

};



#endif
