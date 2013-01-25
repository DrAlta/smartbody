#include "SBSimulationManager.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBScript.h>


namespace SmartBody {

SBProfiler::SBProfiler()
{
}

SBProfiler::~SBProfiler()
{
}

void SBProfiler::printLegend()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print_legend();
	else
		LOG("Profiler does not exist!");
}

void SBProfiler::printStats()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print();
	else
		LOG("Profiler does not exist!");
}


SBSimulationManager::SBSimulationManager()
{
	_simStarted = false;
	_simPlaying = false;
	_hasTimer = false;
}

SBSimulationManager::~SBSimulationManager()
{
	if (_hasTimer)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		delete mcu.timer_p;
	}
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
		LOG("Time regulator does not exist!");
}

double SBSimulationManager::getTime()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.time;
}

double SBSimulationManager::getTimeDt()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.time_dt;
}

void SBSimulationManager::setTime(double time)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.update_timer(time);
}

void SBSimulationManager::update()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		bool doUpdate = mcu.update_timer();
		if (!doUpdate)
			return;
	}
	mcu.update();
}

bool SBSimulationManager::isStarted()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isStarted();
	else
	{
		if (_simStarted)
			return true;
		else
			return false;
	}
}

bool SBSimulationManager::isRunning()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isRunning();
	else
	{
		if (_simPlaying)
			return true;
		else
			return false;
	}
}

void SBSimulationManager::reset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->reset();
	else
	{
		return;
	}
}

void SBSimulationManager::start()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// run the start scripts
	std::map<std::string, SBScript*>& scripts = mcu._scene->getScripts();
	for (std::map<std::string, SBScript*>::iterator iter = scripts.begin();
		 iter != scripts.end();
		 iter++)
	{
		(*iter).second->start();
	}
	

	if (mcu.timer_p)	
	{
		mcu.timer_p->start();
	}
	else
	{
		_simStarted = true;
	}
}

void SBSimulationManager::stop()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// run the stop scripts
	std::map<std::string, SBScript*>& scripts = mcu._scene->getScripts();
	for (std::map<std::string, SBScript*>::iterator iter = scripts.begin();
		 iter != scripts.end();
		 iter++)
	{
		(*iter).second->stop();
	}
	
	if (mcu.timer_p)	
	{
		mcu.timer_p->stop();
	}
	else
	{
		_simStarted = false;
		_simPlaying = false;
	}
}

void SBSimulationManager::pause()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		mcu.timer_p->pause();
	}
	else
	{
		_simPlaying = false;
	}
}

void SBSimulationManager::resume()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		mcu.timer_p->resume();
	}
	else
	{
		_simPlaying = true;
	}
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
		LOG("Time regulator does not exist!");
		return;
	}
	mcu.timer_p->set_eval_fps(v);
}

void SBSimulationManager::setSimFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator does not exist!");
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
		LOG("Time regulator does not exist!");
		return;
	}
	mcu.timer_p->set_eval_dt(v);
}

void SBSimulationManager::setSimDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator does not exist!");
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
		LOG("Time regulator does not exist!");
}

void SBSimulationManager::setupTimer()
{
	TimeRegulator* timer = new TimeRegulator();
	_hasTimer = true;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.register_timer( *timer );
}

void SBSimulationManager::setSleepLock()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator does not exist!");
		return;
	}
	mcu.timer_p->set_sleep_lock();
}

}

