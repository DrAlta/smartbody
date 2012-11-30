#ifndef _SBCHARACTER_H_
#define _SBCHARACTER_H_

#include <sb/SBTypes.h>
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
		SBAPI SBCharacter();
		SBAPI SBCharacter(std::string name, std::string type = "");

		SBAPI const std::string& getName();
		SBAPI void setName(std::string& name);

		SBAPI void setType(const std::string& type);
		SBAPI std::string getType();
		
		SBAPI virtual int setup();

		SBAPI void setMeshMap(std::string filename);
		SBAPI void addMesh(std::string mesh);

		SBAPI int getNumControllers();

		SBAPI void setAutomaticPruning(bool val);
		SBAPI bool isAutomaticPruning();
		SBAPI void pruneControllers();

		SBAPI void setUseVisemeCurves(bool val);
		SBAPI bool isUseVisemeCurves();

		SBAPI float getVisemeTimeOffset();
		SBAPI void setVisemeTimeOffset(float val);

		SBAPI void setVoice(std::string type);
		SBAPI void setVoiceCode(std::string param);
		SBAPI const std::string getVoice();
		SBAPI const std::string& getVoiceCode();

		SBAPI void setVoiceBackup(std::string type);
		SBAPI void setVoiceBackupCode(std::string param);
		SBAPI const std::string& getVoiceBackup();
		SBAPI const std::string& getVoiceBackupCode();

		SBAPI SBController* getControllerByIndex(int i);
		SBAPI SBController* getControllerByName(std::string name);
		SBAPI std::vector<std::string> getControllerNames();
		SBAPI void startMotionRecord(double frameRate);
		SBAPI void stopMotionRecord(const std::string& motionName);

		SBAPI int getNumBehaviors();
		SBAPI SBBehavior* getBehavior(int num);
		SBAPI std::vector<SBBehavior*>& getBehaviors();
		
		SBAPI double getLastScheduledSpeechBehavior();
		SBAPI std::string hasSpeechBehavior();

		SBAPI virtual SBFaceDefinition* getFaceDefinition();
		SBAPI virtual void setFaceDefinition(SBFaceDefinition* face);

		SBAPI void setSteerAgent(SBSteerAgent* agent);

		SBAPI void notify(SBSubject* subject);

	protected:
		std::vector<SBBehavior*> _curBehaviors;

};

};

#endif
