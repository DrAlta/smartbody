#include "vhcl.h"
#include "SBPython.h"
#include "SBPythonClass.h"
#include "controllers/me_ct_scheduler2.h"
#include "controllers/me_ct_gaze.h"
#include "controllers/me_ct_eyelid.h"
#include "controllers/me_ct_curve_writer.hpp"
#include "SBFaceDefinition.h"
#include <sb/nvbg.h>
#include "SBBehavior.h"
#include <sb/SBMotion.h>
#include <sb/SBParseNode.h>

#include <sb/SBScene.h>
#include <sb/SBScript.h>
#include <sb/SBService.h>
#include <sb/SBServiceManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotionBlendBase.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBAnimationTransitionRule.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBReach.h>
#include <sb/SBReachManager.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBParser.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBRetarget.h>
#include <sb/SBRetargetManager.h>
#include <sb/SBEvent.h>
#include <sb/SBCharacterListener.h>
#include <sr/sr_box.h>
#include <sr/sr_camera.h>
#include <stdlib.h>
#include <sbm/GenericViewer.h>
#include <controllers/me_ct_motion.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif

#include "SBPythonInternal.h"


typedef std::map<std::string,SrQuat> QuatMap;
typedef std::map<std::string,SrVec> VecMap;
typedef std::map<std::string, std::string> StringMap;


#ifdef USE_PYTHON


namespace SmartBody
{ 

void pythonFuncsScene()
{
		boost::python::class_<SBScene, boost::python::bases<SBObject> >("SBScene")
		.def("update", &SBScene::update, "Updates the simulation at the given time.")
		.def("setProcessId", &SBScene::setProcessId, "Sets the process id of the SmartBody instance.")
		.def("getProcessId", &SBScene::getProcessId,  boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the process id of the SmartBody instance.")
		.def("createCharacter", &SBScene::createCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new character given character name. \n Input: character name \nOutput: character object")
		.def("createPawn", &SBScene::createPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new pawn.")
		.def("createFaceDefinition", &SBScene::createFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new face definition with a given name.")
		.def("getFaceDefinition", &SBScene::getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a face definition with a given name.")
		.def("getNumFaceDefinitions", &SBScene::getNumFaceDefinitions, "Returns the number of face definitions.")
		.def("getFaceDefinitionNames", &SBScene::getFaceDefinitionNames, "Return a list of all face definition names. \n Input: NULL \nOutput: list of face definition names.")
		.def("removeCharacter", &SBScene::removeCharacter, "Remove the character given its name. \n Input: character name \n Output: NULL")
		.def("removeAllCharacters", &SBScene::removeAllCharacters, "Removes all the characters.")
		.def("removePawn", &SBScene::removePawn, "Remove the pawn given its name. \n Input: pawn name \n Output: NULL")
		.def("removeAllPawns", &SBScene::removeAllPawns, "Removes all the pawns.")
		.def("getNumPawns", &SBScene::getNumPawns, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.")
		.def("getNumCharacters", &SBScene::getNumCharacters, "Returns the number of characters.\n Input: NULL \nOutput: number of characters.")
		.def("getPawn", &SBScene::getPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the pawn object given its name. \n Input: pawn name \nOutput: pawn object")
		.def("getCharacter", &SBScene::getCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character object given its name. \n Input: character name \nOutput: character object")
		.def("getPawnNames", &SBScene::getPawnNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of all character names.\n Input: NULL \nOutput: list of pawn names")
		.def("getCharacterNames", &SBScene::getCharacterNames, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a list of all character names.\n Input: NULL \nOutput: list of character names")
		.def("getEventHandlerNames", &SBScene::getEventHandlerNames, "Returns a list of names of all event handlers.\n Input: NULL \nOutput: list of event handler names")
		.def("setMediaPath",&SBScene::setMediaPath, "Sets the media path.")
		.def("getMediaPath",&SBScene::getMediaPath, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the media path.")
		.def("setDefaultCharacter", &SBScene::setDefaultCharacter, "Sets the default character.")
		.def("setDefaultRecipient", &SBScene::setDefaultRecipient, "Sets the default recipient.")
		.def("addSkeletonDefinition", &SBScene::addSkeletonDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Add an new empty skeleton into system. \n Input: skeleton name \nOutput: skeleton object")
		.def("addMotionDefinition", &SBScene::addMotionDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Add an new empty motion into system. \n Input: motion name, motion duration \nOutput: motion object")
		.def("addScript", &SBScene::addScript, "Adds a script to the scene.")
		.def("removeScript", &SBScene::removeScript, "Returns the number of scripts.")
		.def("getNumScripts", &SBScene::getNumScripts, "Returns the number of scripts.")
		.def("getScriptNames", &SBScene::getScriptNames, "Returns the names of all the scripts.")
		.def("getScript", &SBScene::getScript, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a script.")
		.def("getScale", &SBScene::getScale, "Returns the scene scale in meters (default is centimeters .01)")
		.def("setScale", &SBScene::setScale, "Sets the scene scale in meters.")
		.def("isRemoteMode", &SBScene::isRemoteMode, "Returns the boolean indicating whether scene is in remote mode.")
		.def("setRemoteMode", &SBScene::setRemoteMode, "Sets the scene remote mode.")
		.def("removePendingCommands", &SBScene::removePendingCommands, "Removes any commands stored in SmartBody awaiting execution.")
		.def("setCharacterListener", &SBScene::setCharacterListener, "Sets the listener for character and pawn events.")
		.def("getCharacterListener", &SBScene::getCharacterListener, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the listener for character and pawn events.")
		.def("save", &SBScene::save, "Saves the SmartBody configuration. Returns a string containing Python commands representing the configuration.")
		.def("exportScene", &SBScene::exportScene, "Saves the entire SmartBody configuration, including assets, into a given file location.")
		.def("createNavigationMesh", &SBScene::createNavigationMesh, "Create navigation mesh from the input mesh.\n Input : OBJ file name")
		.def("startFileLogging", &SBScene::startFileLogging, "Starts logging SmartBody messages to a given log file.")
		.def("stopFileLogging", &SBScene::stopFileLogging, "Stops logging SmartBody messages to the given log file.")
	;
}
}


#endif