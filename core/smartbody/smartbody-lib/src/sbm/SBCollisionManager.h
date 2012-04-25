#ifndef _SBCOLLISIONMANAGER_H_
#define _SBCOLLISIONMANAGER_H_

#include <sbm/SBService.h>
#include <sbm/SBSubject.h>

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

	protected:
		float _characterRadius;
		int _maxIterations;
		std::map<std::string, SrVec> _velocities;
		std::map<std::string, SrVec> _positions;

};


}
#endif