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
		
		void start();
		void stop();
		void setSteerUnit(std::string unit);
		std::string getSteerUnit();

		SteerSuiteEngineDriver* getEngineDriver();

		SBSteerAgent* createSteerAgent(std::string name);
		void removeSteerAgent(std::string name);
		int getNumSteerAgents();
		SBSteerAgent* getSteerAgent(std::string name);
		std::vector<std::string> getSteerAgentNames();
		std::map<std::string, SBSteerAgent*>& getSteerAgents();

	private:
		std::map<std::string, SBSteerAgent*> _steerAgents;

		SteerSuiteEngineDriver _driver;


};

}

#endif 