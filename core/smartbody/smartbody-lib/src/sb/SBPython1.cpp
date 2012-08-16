#include "SBPython.h"
#include "SBPythonClass.h"
#include "controllers/me_ct_scheduler2.h"
#include "SBFaceDefinition.h"
#include "sbm/nvbg.h"
#include "SBBehavior.h"
#include <sb/SBMotion.h>
#include <sb/SBParseNode.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBScript.h>
#include <sb/SBService.h>
#include <sb/SBServiceManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotionBlendBase.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBReach.h>
#include <sb/SBReachManager.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
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
#include <sr/sr_box.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "SBPythonInternal.h"


#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif

typedef std::map<std::string,SrQuat> QuatMap;
typedef std::map<std::string,SrVec> VecMap;
typedef std::map<std::string, std::string> StringMap;


#ifdef USE_PYTHON


namespace SmartBody 
{

void pythonFuncs1()
{
	boost::python::def("printlog", printLog, "Write to the log. \n Input: message string \n Output: NULL");

	boost::python::docstring_options local_docstring_options(true, true, false);

    boost::python::class_<std::vector<std::string> >("StringVec")
        .def(boost::python::vector_indexing_suite<std::vector<std::string> >())
    ;

	boost::python::class_<std::vector<float> >("FloatVec")
        .def(boost::python::vector_indexing_suite<std::vector<float> >())
    ;

	boost::python::class_<std::vector<int> >("IntVec")
        .def(boost::python::vector_indexing_suite<std::vector<int> >())
    ;

	boost::python::class_<std::vector<double> >("DoubleVec")
        .def(boost::python::vector_indexing_suite<std::vector<double> >())
    ;


	boost::python::class_< QuatMap >("QuatMap")
		.def("__len__", &QuatMap::size)
		.def("clear", &QuatMap::clear)
		.def("__getitem__", &map_item<QuatMap>::get,
		boost::python::return_value_policy<boost::python::return_by_value>())
		.def("__setitem__", &map_item<QuatMap>::set,
		boost::python::with_custodian_and_ward<1,2>()) // to let container keep value
		.def("__delitem__", &map_item<QuatMap>::del)
		;

	boost::python::class_< VecMap >("VecMap")
		.def("__len__", &VecMap::size)
		.def("clear", &VecMap::clear)
		.def("__getitem__", &map_item<VecMap>::get,
		boost::python::return_value_policy<boost::python::return_by_value>())
		.def("__setitem__", &map_item<VecMap>::set,
		boost::python::with_custodian_and_ward<1,2>()) // to let container keep value
		.def("__delitem__", &map_item<VecMap>::del)
		;

	boost::python::class_< StringMap >("StringMap")
		.def("__len__", &StringMap::size)
		.def("clear", &StringMap::clear)
		.def("__getitem__", &map_item<StringMap>::get,
		boost::python::return_value_policy<boost::python::return_by_value>())
		.def("__setitem__", &map_item<StringMap>::set,
		boost::python::with_custodian_and_ward<1,2>()) // to let container keep value
		.def("__delitem__", &map_item<StringMap>::del)
		;



	boost::python::class_<SBScene, boost::python::bases<SBObject> >("SBScene")
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
		.def("getSkeleton", &SBScene::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton object given its name. \n Input: skeleton name \nOutput: skeleton object")
		.def("getMotion", &SBScene::getMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a the motion of given name.")
		.def("getNumMotions", &SBScene::getNumMotions, "Returns the number of motions available.")
		.def("getMotionNames", &SBScene::getMotionNames, "Returns the names of motions available.")
		.def("getPawnNames", &SBScene::getPawnNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of all character names.\n Input: NULL \nOutput: list of pawn names")
		.def("getCharacterNames", &SBScene::getCharacterNames, "Returns a list of all character names.\n Input: NULL \nOutput: list of character names")
		.def("getSkeletonNames", &SBScene::getSkeletonNames, "Returns a list of all skeleton names.\n Input: NULL \nOutput: list of skeleton names")
		.def("getEventHandlerNames", &SBScene::getEventHandlerNames, "Returns a list of names of all event handlers.\n Input: NULL \nOutput: list of event handler names")
		.def("addMotion", &SBScene::addMotion, "Add motion resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL")
		.def("addPose", &SBScene::addPose, "Add pose resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL")
		.def("addAssetPath", &SBScene::addAssetPath, "Add path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL")
		.def("removeAssetPath", &SBScene::removeAssetPath, "Removes a  path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL")
		.def("getAssetPaths", &SBScene::getAssetPaths, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of all path names for a given type: seq, me, audio, mesh.")
		.def("loadAssets", &SBScene::loadAssets, "Loads the skeletons and motions from the asset paths.")
		.def("loadAssetsFromPath", &SBScene::loadAssetsFromPath, "Loads the skeletons and motions from a given path. The path will not be stored for later use.")
		.def("setMediaPath",&SBScene::setMediaPath, "Sets the media path.")
		.def("getMediaPath",&SBScene::getMediaPath, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the media path.")
		.def("setDefaultCharacter", &SBScene::setDefaultCharacter, "Sets the default character.")
		.def("setDefaultRecipient", &SBScene::setDefaultRecipient, "Sets the default recipient.")
		.def("createSkeleton", &SBScene::createSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new skeleton given a skeleton definition.")
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

		// command processing
		.def("command", &SBScene::command, "Runs an old-Style SmartBody command.")
		.def("commandAt", &SBScene::commandAt, "Runs an old-style SmartBody command at a set time in the future.")
		.def("vhmsg", &SBScene::sendVHMsg, "Sends a virtual human message.")
		.def("vhmsg2", &SBScene::sendVHMsg2, "Sends a virtual human message.")
		.def("run", &SBScene::runScript, "Runs a python script.")
		// managers
		.def("getEventManager", &SBScene::getEventManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the event manager.")
		.def("getSimulationManager", &SBScene::getSimulationManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the simulation manager object.")
		.def("getProfiler", &SBScene::getProfiler, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the  profiler object.")
		.def("getBmlProcessor", &SBScene::getBmlProcessor, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the bml processor object.")
		.def("getStateManager", &SBScene::getBlendManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the state manager object.")
		.def("getBlendManager", &SBScene::getBlendManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the state manager object.")
		.def("getReachManager", &SBScene::getReachManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the reach manager object.")
		.def("getSteerManager", &SBScene::getSteerManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the steer manager object.")
		.def("getServiceManager", &SBScene::getServiceManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the service manager object.")
		.def("getPhysicsManager", &SBScene::getPhysicsManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the physics manager object.")
		.def("getBoneBusManager", &SBScene::getBoneBusManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the Bone Bus manager object.")
		.def("getGestureMapManager", &SBScene::getGestureMapManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the gesture map manager object.")
		.def("getJointMapManager", &SBScene::getJointMapManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint mapping manager object.")
		.def("getCollisionManager", &SBScene::getCollisionManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the collision manager object.")
		.def("getDiphoneManager", &SBScene::getDiphoneManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the diphone manager object.")
		.def("getBehaviorSetManager", &SBScene::getBehaviorSetManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the behavior set manager.")
		.def("getParser", &SBScene::getParser, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the Charniak parser.")
	;

}
}


#endif
