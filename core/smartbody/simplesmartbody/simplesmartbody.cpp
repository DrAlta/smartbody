#include <vhcl.h>
#include <sb/SBPython.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBSkeleton.h>

int main(int argc, char** argv)
{
	// locate the Python libraries
	initPython("../../Python26/Lib");

	// send log messages to stdout 
	vhcl::Log::StdoutListener* listener = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(listener);

	// get the scene object 
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	// tell SmartBody where the find the skeletons and motions
	scene->addAssetPath("motion", "../../../../data/sbm-common/common-sk");

	// load the assets from the asset directories
	scene->loadAssets();

	// create a character
	SmartBody::SBCharacter* mycharacter = scene->createCharacter("foo", "");

	// attach a skeleton to the character
	SmartBody::SBSkeleton* skeleton = scene->createSkeleton("common.sk");
	mycharacter->setSkeleton(skeleton);

	// create the default set of controllers, including, posture, gazing, head movements, blinking, etc.
	mycharacter->createStandardControllers();

	// prepare the simulation
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
	double time = 0.0;
	sim->setTime(time);
	sim->start();
	while (sim->getTime() < 100.0) // run the simulation for 100 simulated seconds
	{
		// run the simulation at 60 fps
		sim->setTime(sim->getTime() + .016);
		LOG("%lf", sim->getTime());
		sim->update();
	}

	sim->stop();

	return 0;
}
