#ifndef SBSCENE_H
#define SBSCENE_H

#include <vhcl.h>
#include <sbm/SBObject.h>
#include <sbm/SBCharacter.h>
#include <sbm/SBMotion.h>
#include <sbm/Event.h>
#include <sbm/SBSimulationManager.h>
#include <sbm/SBBmlProcessor.h>
#include <sbm/SBStateManager.h>

namespace SmartBody {

class SBScene : public SBObject
{
	public:
		SBScene(void);
		~SBScene(void);

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

		void setMediaPath(std::string path);
		void addAssetPath(std::string type, std::string path);
		void removeAssetPath(std::string type, std::string path);
		void loadAssets();
		void loadMotions();
		void addPose(std::string path, bool recursive);
		void addMotion(std::string path, bool recursive);
		
		void runScript(std::string script);

		void setDefaultCharacter(const std::string& character);
		void setDefaultRecipient(const std::string& recipient);

		EventManager* getEventManager();		
		SBSimulationManager* getSimulationManager();
		Profiler* getProfiler();
		SBBmlProcessor* getBmlProcessor();
		SBStateManager* getStateManager();

		void notify(SBSubject* subject);

	private:
		SBSimulationManager* _sim;
		Profiler* _profiler;
		SBBmlProcessor* _bml;
		SBStateManager* _stateManager;
};

};

#endif