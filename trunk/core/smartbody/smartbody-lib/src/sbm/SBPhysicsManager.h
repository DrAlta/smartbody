#ifndef _SBPHYSICSMANAGER_H_
#define _SBPHYSICSMANAGER_H_

#include <sbm/SBService.h>
#include <sbm/SBService.h>
#include <sbm/physics/SbmPhysicsSimODE.h>

namespace SmartBody {

class SBPhysicsManager : public SBService
{
	public:
		SBPhysicsManager();
		~SBPhysicsManager();

		virtual void setEnable(bool enable);
		virtual bool isEnable();
		
		virtual void start();
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();

		SbmPhysicsSim* getPhysicsEngine();

	protected:
		SbmPhysicsSimODE* _ode;
		double            physicsTime;

};

}

#endif