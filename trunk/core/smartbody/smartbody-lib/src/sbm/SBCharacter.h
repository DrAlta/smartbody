#ifndef _SBCHARACTER_H_
#define _SBCHARACTER_H_

#include <sbm/sbm_character.hpp>
#include "sbm/SBController.h"



namespace SmartBody {

class SBSkeleton;

class SBCharacter : public SbmCharacter
{
	public:
		SBCharacter();
		SBCharacter(std::string name, std::string type = "");

		std::string getName();
		void setName(std::string name);

		void setType(std::string type);
		std::string getType();

		SBSkeleton* getSkeleton();
		virtual void setSkeleton(SBSkeleton* skel);
		
		virtual int setup();

		void setMeshMap(std::string filename);
		void addMesh(std::string mesh);

		int getNumControllers();
		
		SrVec getPosition();
		SrQuat getOrientation();
		void setPosition(SrVec pos);
		void setOrientation(SrQuat quat);
		void setHPR(SrVec hpr);
		SrVec getHPR();

		void setAutomaticPruning(bool val);
		bool isAutomaticPruning();
		void pruneControllers();

		void setUseVisemeCurves(bool val);
		bool isUseVisemeCurves();

		float getVisemeTimeOffset();
		void setVisemeTimeOffset(float val);

		void setVoice(std::string type);
		void setVoiceCode(std::string param);
		std::string getVoice();
		std::string getVoiceCode();

		void setVoiceBackup(std::string type);
		void setVoiceBackupCode(std::string param);
		std::string getVoiceBackup();
		std::string getVoiceBackupCode();

		SBController* getControllerByIndex(int i);
		SBController* getControllerByName(std::string name);

		void addController(SBController* controller);
		bool isFaceNeutral();
		bool initFaceController(MeCtFace* faceCtrl);
		void initLocomotion(MeCtLocomotion* locoCtrl);
		void linkControllers(SBController* ctrl);



};

};

#endif
