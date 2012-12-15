#include "vhcl.h"
#include <sb/SBPython.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSimulationManager.h>

int main( int argc, char ** argv )
{
	// add a message logger to stdout
	vhcl::Log::StdoutListener* stdoutLog = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(stdoutLog);

	LOG("Loading Python...");

	// initialize the Python libraries
	initPython("../../Python26/Libs");

	// get the scene object
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	// set the mediapath which dictates the top-level asset directory
	scene->setMediaPath("../../../../data");

	// indicate where different assets will be located
	// "motion" = animations and skeletons
	// "script" = Python scripts to be executed
	// "mesh" = models and textures
	scene->addAssetPath("motion", "sbm-common/common-sk");

	// load the assets from the indicated locations
	LOG("Loading Assets...");
	scene->loadAssets();
	int numMotions = scene->getNumMotions();
	LOG("Loaded %d motions...", numMotions);

	// create a character
	LOG("Creating the character...");
	SmartBody::SBCharacter* character = scene->createCharacter("mycharacter", "");

	// load the skeleton from one of the available skeleton types
	SmartBody::SBSkeleton* skeleton = scene->createSkeleton("common.sk");

	// attach the skeleton to the character
	character->setSkeleton(skeleton);

	// get the simulation object 
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();

	sim->setTime(0.0);
	LOG("Starting the simulation...");
	sim->start();
	while (sim->getTime() < 100.0) // run for 100 simulated seconds
	{
		sim->update();
		sim->setTime(sim->getTime() + 0.16); // update at 1/60 of a second
	}

	sim->stop();

}



