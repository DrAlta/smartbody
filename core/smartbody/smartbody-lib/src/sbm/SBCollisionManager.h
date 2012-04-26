#ifndef _SBCOLLISIONMANAGER_H_
#define _SBCOLLISIONMANAGER_H_

#include <sbm/SBService.h>
#include <sbm/SBSubject.h>
#include <sbm/Physics/SbmColObject.h>

namespace SmartBody {

class SBCollisionManager : public SBService
{
	public:
		SBCollisionManager();
		~SBCollisionManager();

		virtual void setEnable(bool enable);
		virtual void start();
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();
		virtual void notify(SBSubject* subject);

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
};


}
#endif