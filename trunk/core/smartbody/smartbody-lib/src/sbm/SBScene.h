#ifndef SBSCENE_H
#define SBSCENE_H

#include <vhcl.h>
#include <sbm/DObject.h>
#include <sbm/SBCharacter.h>
#include <sbm/Event.h>
#include <sbm/SBSimulationManager.h>
#include <sbm/SBBmlProcessor.h>

namespace SmartBody {

class SBScene : public DObject
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
		SBPawn* getPawn(std::string name);

		std::vector<std::string> getPawnNames();
		std::vector<std::string> getCharacterNames();

		FaceDefinition* getFaceDefinition(std::string str);

		SBSkeleton* createSkeleton(std::string char_name);

		SkMotion* getMotion(std::string name);

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

		void notify(DSubject* subject);

	private:
		SBSimulationManager* _sim;
		Profiler* _profiler;
		SBBmlProcessor* _bml;
};

};

#endif