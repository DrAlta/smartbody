#ifndef _SIMULATIONMANAGER_H
#define _SIMULATIONMANAGER_H

namespace SmartBody {

class SBProfiler
{
	public:
		SBProfiler();
		~SBProfiler();

		void printLegend();
		void printStats();
};



class SBSimulationManager
{
	public:
		SBSimulationManager();
		~SBSimulationManager();

		bool isStarted();
		bool isRunning();

		void printInfo();
		void printPerf(float v);
		double getTime();
		double getTimeDt();
		void setTime(double time);
		void start();
		void stop();
		void reset();
		void pause();
		void resume();
		void step(int n);
		void setSleepFps(float v);
		void setEvalFps(float v);
		void setSimFps(float v);
		void setSleepDt(float v);
		void setEvalDt(float v);
		void setSimDt(float v);
		void setSpeed(float v);
};

};

#endif