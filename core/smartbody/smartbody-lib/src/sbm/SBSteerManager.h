#ifndef _STEERMANAGER_H_
#define _STEERMANAGER_H_

#include <sbm/SBSteerAgent.h>

class SBSteerManager
{
	public:
		SBSteerManager();
		~SBSteerManager();
		
		void start();
		void stop();
		void setSteerUnit(std::string unit);
		std::string getSteerUnit();
		

		SBSteerAgent* createSteerAgent(std::string name);
		void removeSteerAgent(std::string name);
		int getNumSteerAgents();
		SBSteerAgent* getSteerAgent(std::string name);
		std::vector<std::string> getSteerAgentNames();
		std::map<std::string, SBSteerAgent*>& getSteerAgents();

	private:
		std::map<std::string, SBSteerAgent*> _steerAgents;
};

#endif 