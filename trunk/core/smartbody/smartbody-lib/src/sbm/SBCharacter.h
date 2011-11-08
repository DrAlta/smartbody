#ifndef _SBCHARACTER_H_
#define _SBCHARACTER_H_

#include <sbm/sbm_character.hpp>
#include "sbm/SBController.h"



namespace SmartBody {

class SBSkeleton;
class SBBehavior;

class SBCharacter : public SbmCharacter
{
	public:
		SBCharacter();
		SBCharacter(std::string name, std::string type = "");

		const std::string& getName();
		void setName(std::string& name);

		void setType(const std::string& type);
		const std::string& getType();
		
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
		const std::string& getVoice();
		const std::string& getVoiceCode();

		void setVoiceBackup(std::string type);
		void setVoiceBackupCode(std::string param);
		const std::string& getVoiceBackup();
		const std::string& getVoiceBackupCode();

		SBController* getControllerByIndex(int i);
		SBController* getControllerByName(std::string name);

		void addController(SBController* controller);
		bool isFaceNeutral();
		bool initFaceController(MeCtFace* faceCtrl);
		void initLocomotion(MeCtLocomotion* locoCtrl);
		void linkControllers(SBController* ctrl);

		int getNumBehaviors();
		SBBehavior* getBehavior(int num);
		std::vector<SBBehavior*>& getBehaviors();

		virtual FaceDefinition* getFaceDefinition();
		virtual void setFaceDefinition(FaceDefinition* face);

	protected:
		std::vector<SBBehavior*> _curBehaviors;

};

};

#endif
