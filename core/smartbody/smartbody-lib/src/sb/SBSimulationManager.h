#ifndef _SIMULATIONMANAGER_H
#define _SIMULATIONMANAGER_H

#include <sb/SBTypes.h>

namespace SmartBody {

class SBProfiler
{
	public:
		SBAPI SBProfiler();
		SBAPI ~SBProfiler();

		SBAPI void printLegend();
		SBAPI void printStats();
};



class SBSimulationManager
{
	public:
		SBAPI SBSimulationManager();
		SBAPI ~SBSimulationManager();

		SBAPI bool isStarted();
		SBAPI bool isRunning();

		SBAPI void printInfo();
		SBAPI void printPerf(float v);
		SBAPI double getTime();
		SBAPI double getTimeDt();
		SBAPI void setTime(double time);
		SBAPI void start();
		SBAPI void stop();
		SBAPI void reset();
		SBAPI void pause();
		SBAPI void resume();
		SBAPI void step(int n);
		SBAPI void setSleepFps(float v);
		SBAPI void setEvalFps(float v);
		SBAPI void setSimFps(float v);
		SBAPI void setSleepDt(float v);
		SBAPI void setEvalDt(float v);
		SBAPI void setSimDt(float v);
		SBAPI void setSpeed(float v);
};

};

#endif