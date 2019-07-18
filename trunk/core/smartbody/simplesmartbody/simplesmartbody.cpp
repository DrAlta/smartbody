#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBSceneListener.h>
#include <sb/SBUtilities.h>

class SimpleListener : public SmartBody::SBSceneListener
{
   public:
	   SimpleListener() {}
	   ~SimpleListener() {}
     
	  virtual void OnLogMessage( const std::string & message )
	  {
#ifdef WIN32
		SmartBody::util::log("%s", message.c_str());
#else
		std::cout << message << std::endl;
#endif
	  }
};

int main( int argc, char ** argv )
{

	if (argc < 4)
	{
		std::cout << "Usage: simplesmartbody <pythondir> <scriptpath> <script>" << std::endl;
		exit(-1);
	}
	std::string pythonPath = "../Python27/Libs";
	std::string scriptPath = "";
	std::string script = "";

	pythonPath = argv[1];
	scriptPath = argv[2];
	script = argv[3];

	std::cout << "Python location is [" << pythonPath << "]" << std::endl;
	std::cout << "Script path [" << scriptPath << "]" << std::endl;
	std::cout << "Script is [" << script << "]" << std::endl;

	// add a message logger to stdout
	SmartBody::util::StdoutListener* stdoutLog = new SmartBody::util::StdoutListener();
	SmartBody::util::g_log.AddListener(stdoutLog);

	SmartBody::util::log("Loading Python...");

	// initialize the Python libraries
	initPython(pythonPath);

	// get the scene object
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SimpleListener listener;
	scene->addSceneListener(&listener);

	// set the mediapath which dictates the top-level asset directory
	//scene->setMediaPath(mediaPath);
	scene->addAssetPath("script", scriptPath);
		
	scene->runScript(script);


	// get the simulation object 
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();

	// if you want to use a real-time clock do the following:
	bool useRealTimeClock = true;
	if (useRealTimeClock)
	{
		sim->setupTimer();
	}
	else
	{
		// otherwise, the time will run according
		sim->setTime(0.0);
	}

	
	SmartBody::util::log("Starting the simulation...");
	double lastPrint = 0;
	sim->start();
	while (true)
	{
		if (scene->getSimulationManager()->isStopped())
		{
			SmartBody::util::log("simulation stopped from script");
			break;
		}
		scene->update();
		if (!useRealTimeClock)
			sim->setTime(sim->getTime() + 0.16); // update at 1/60 of a second when running in simulated time
		else
			sim->updateTimer();

	}

	sim->stop();
	SmartBody::util::log("Ending the simulation...");

	return 0;
}
