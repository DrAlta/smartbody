#include "SBSimulationManager.h"
#include <sbm/mcontrol_util.h>

Profiler::Profiler()
{
}

Profiler::~Profiler()
{
}

void Profiler::printLegend()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print_legend();
	else
		LOG("Profiler does not exist!");
}

void Profiler::printStats()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print();
	else
		LOG("Profiler does not exist!");
}


SBSimulationManager::SBSimulationManager()
{
}

SBSimulationManager::~SBSimulationManager()
{
}

void SBSimulationManager::printInfo()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->print();
	else	
	{
		LOG( "TIME:%.3f ~ DT:%.3f %.2f:FPS\n",
			mcu.time,
			mcu.time_dt,
			1.0 / mcu.time_dt
		);
	}
}

void SBSimulationManager::printPerf(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		if (v > 0.0) 
			mcu.timer_p->set_perf(v);
		else	
			mcu.timer_p->set_perf(10.0);	
	}
	else
		LOG("Time regulator not exist!");
}

double SBSimulationManager::getTime()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.time;
}

bool SBSimulationManager::isStarted()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isStarted();
	else
		return false;
}

bool SBSimulationManager::isRunning()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isRunning();
	else
		return false;
}

void SBSimulationManager::reset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->reset();
	else
		LOG("Time regulator not exist!");
}

void SBSimulationManager::start()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->start();
	else
		LOG("Time regulator not exist!");
}

void SBSimulationManager::pause()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->pause();
	else
		LOG("Time regulator not exist!");
}

void SBSimulationManager::resume()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->resume();
	else
		LOG("Time regulator not exist!");
}

void SBSimulationManager::step(int n)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		if (n)
			mcu.timer_p->step(n);
		else
			mcu.timer_p->step(1);
	}
	else
		LOG("Time regulator not exist!");
}

void SBSimulationManager::setSleepFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sleep_fps(v);
}

void SBSimulationManager::setEvalFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_eval_fps(v);
}

void SBSimulationManager::setSimFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sim_fps(v);
}

void SBSimulationManager::setSleepDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sleep_dt(v);
}

void SBSimulationManager::setEvalDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_eval_dt(v);
}

void SBSimulationManager::setSimDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sim_dt(v);
}

void SBSimulationManager::setSpeed(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->set_speed(v);
	else
		LOG("Time regulator not exist!");
}