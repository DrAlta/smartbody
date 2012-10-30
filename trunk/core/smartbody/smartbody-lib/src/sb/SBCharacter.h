#ifndef _SBCHARACTER_H_
#define _SBCHARACTER_H_

#include <sbm/sbm_character.hpp>

namespace SmartBody {

class SBSkeleton;
class SBBehavior;
class SBSteerAgent;
class SBController;
class SBDiphone;

class SBCharacter : public SbmCharacter
{
	public:
		SBCharacter();
		SBCharacter(std::string name, std::string type = "");

		const std::string& getName();
		void setName(std::string& name);

		void setType(const std::string& type);
		std::string getType();
		
		virtual int setup();

		void setMeshMap(std::string filename);
		void addMesh(std::string mesh);

		int getNumControllers();

		void setAutomaticPruning(bool val);
		bool isAutomaticPruning();
		void pruneControllers();

		void setUseVisemeCurves(bool val);
		bool isUseVisemeCurves();

		float getVisemeTimeOffset();
		void setVisemeTimeOffset(float val);

		void setVoice(std::string type);
		void setVoiceCode(std::string param);
		const std::string getVoice();
		const std::string& getVoiceCode();

		void setVoiceBackup(std::string type);
		void setVoiceBackupCode(std::string param);
		const std::string& getVoiceBackup();
		const std::string& getVoiceBackupCode();

		SBController* getControllerByIndex(int i);
		SBController* getControllerByName(std::string name);
		std::vector<std::string> getControllerNames();

		int getNumBehaviors();
		SBBehavior* getBehavior(int num);
		std::vector<SBBehavior*>& getBehaviors();
		
		double getLastScheduledSpeechBehavior();
		std::string hasSpeechBehavior();

		virtual SBFaceDefinition* getFaceDefinition();
		virtual void setFaceDefinition(SBFaceDefinition* face);

		void setSteerAgent(SBSteerAgent* agent);

		void notify(SBSubject* subject);

	protected:
		std::vector<SBBehavior*> _curBehaviors;

};

};

#endif
