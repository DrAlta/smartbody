#ifndef _SBCOLLISIONMANAGER_H_
#define _SBCOLLISIONMANAGER_H_

#include <sb/SBService.h>
#include <sb/SBSubject.h>
#include <sbm/Physics/SbmColObject.h>
#include <sk/sk_joint.h>

namespace SmartBody {

class SBCollisionManager : public SBService
{
	public:
		SBCollisionManager();
		~SBCollisionManager();

		virtual void setEnable(bool enable);
		virtual void start();
		virtual bool getJointCollisionMode() { return _singleChrCapsuleMode; }
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();
		virtual void notify(SBSubject* subject);

		virtual void onCharacterDelete(SBCharacter* character);
		virtual void onPawnDelete(SBPawn* character);

		SbmGeomObject* createCollisionObject(const std::string& geomName, const std::string& geomType, SrVec size, SrVec from = SrVec(), SrVec to = SrVec());	
		SbmGeomObject* getCollisionObject(const std::string& geomName);
		bool           removeCollisionObject(const std::string& geomName);
		bool           addObjectToCollisionSpace(const std::string& geomName);
		bool           removeObjectFromCollisionSpace(const std::string& geomName);

	protected:
		SbmCollisionSpace* collisionSpace;
		float _characterRadius;
		int _maxIterations;
		std::map<std::string, SrVec> _velocities;
		std::map<std::string, SrVec> _positions;
		std::map<std::string, SbmGeomObject*> geomObjectMap;

		bool _singleChrCapsuleMode;
		float _jointBVLenRadRatio;
		bool isJointExcluded(SkJoint* j, const std::vector<SkJoint*>& jnt_excld_list);
};


}
#endif