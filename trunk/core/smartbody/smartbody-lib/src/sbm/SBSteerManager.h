#ifndef _STEERMANAGER_H_
#define _STEERMANAGER_H_

#include <vhcl.h>
#include <sbm/SBSteerAgent.h>
#include <SteerLib.h>
#include <PPRAgent.h>
#include <sbm/SBService.h>
#include <sbm/SteerSuiteEngineDriver.h>

namespace SmartBody {

class SBSteerManager : public SmartBody::SBService
{
	public:
		SBSteerManager();
		~SBSteerManager();
		
		virtual void setEnable(bool enable);
		virtual void start();
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();

		void setSteerUnit(std::string unit);
		std::string getSteerUnit();
		float getSteerUnitValue();

		SteerSuiteEngineDriver* getEngineDriver();

		SBSteerAgent* createSteerAgent(std::string name);
		void removeSteerAgent(std::string name);
		int getNumSteerAgents();
		SBSteerAgent* getSteerAgent(std::string name);
		std::vector<std::string> getSteerAgentNames();
		std::map<std::string, SBSteerAgent*>& getSteerAgents();

	protected:
		std::map<std::string, SBSteerAgent*> _steerAgents;
		std::vector<SteerLib::BoxObstacle*> _boundaryObstacles;

		SteerSuiteEngineDriver _driver;


};

}

#endif 