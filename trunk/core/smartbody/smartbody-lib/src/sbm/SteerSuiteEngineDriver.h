#ifndef __COMMAND_LINE_ENGINE_DRIVER_H__
#define __COMMAND_LINE_ENGINE_DRIVER_H__

/// @file SteerSuiteEngineDriver.h
/// @brief Declares the SteerSuiteEngineDriver class

#include <vhcl.h>
#include <SteerLib.h>
#include <PPRAgent.h>
#include <sbm/SBObject.h>

class SteerSuiteEngineDriver : public SteerLib::EngineControllerInterface, public SmartBody::SBObject
{
public:
	SteerSuiteEngineDriver();
	~SteerSuiteEngineDriver();
	void init(SteerLib::SimulationOptions * options);
	void finish();
	void run();

	bool isInitialized();
	bool isDone();
	void setDone(bool val);
	void setStartTime(float time);
	float getStartTime();

	/// @name The EngineControllerInterface
	/// @brief The CommandLineEngineDriver does not support any of the engine controls.
	//@{
	virtual bool isStartupControlSupported() { return false; }
	virtual bool isPausingControlSupported() { return false; }
	virtual bool isPaused() { return false; }
	virtual void loadSimulation();
	virtual void unloadSimulation();
	virtual void startSimulation();
	virtual void stopSimulation();
	virtual void pauseSimulation() { throw Util::GenericException("CommandLineEngineDriver does not support pauseSimulation()."); }
	virtual void unpauseSimulation() { throw Util::GenericException("CommandLineEngineDriver does not support unpauseSimulation()."); }
	virtual void togglePausedState() { throw Util::GenericException("CommandLineEngineDriver does not support togglePausedState()."); }
	virtual void pauseAndStepOneFrame() { throw Util::GenericException("CommandLineEngineDriver does not support pauseAndStepOneFrame()."); }
	//@}

	SteerLib::SimulationEngine * _engine;

protected:
	bool _alreadyInitialized;
	bool _done;
	float _startTime;
	SteerLib::SimulationOptions * _options;

private:
	// These functions are kept here to protect us from mangling the instance.
	// Technically the CommandLineEngineDriver is not a singleton, though.
	SteerSuiteEngineDriver(const SteerSuiteEngineDriver & );  // not implemented, not copyable
	SteerSuiteEngineDriver& operator= (const SteerSuiteEngineDriver & );  // not implemented, not assignable
};

#endif
