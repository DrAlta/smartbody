#ifndef _SBPHYSICSMANAGER_H_
#define _SBPHYSICSMANAGER_H_

#include <sb/SBService.h>

class SbmPhysicsSim;
class SbmPhysicsSimODE;

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
				
		SmartBody::SBObject* createPhysicsCharacter(std::string charName);
		SmartBody::SBObject* createPhysicsPawn(std::string pawnName, std::string geomType, SrVec geomSize);

		SmartBody::SBObject* getPhysicsSimulationEngine();
		SmartBody::SBObject* getPhysicsCharacter(std::string charName);
		SmartBody::SBObject* getPhysicsJoint(std::string charName, std::string jointName);
		SmartBody::SBObject* getJointObj(std::string charName, std::string jointName);
		SmartBody::SBObject* getPhysicsPawn(std::string pawnName);

		void applyForceToPawn(std::string pawnName, SrVec force);
		void applyForceToCharacter(std::string charName, std::string jointName, SrVec force);


	protected:
		SbmPhysicsSimODE* _ode;
		double            physicsTime;
		void updatePhysicsCharacter(std::string charName);
		void updatePhysicsPawn(std::string pawnName);
;};

}

#endif
