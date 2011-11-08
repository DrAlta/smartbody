#include "SBPython.h"
#include "SBPythonClass.h"
#include "me/me_ct_scheduler2.h"
#include "SBFaceDefinition.h"
#include "nvbg.h"
#include "SBBehavior.h"
#include <sbm/SBMotion.h>

#ifdef USE_PYTHON

#ifndef __ANDROID__ 
struct NvbgWrap :  Nvbg, boost::python::wrapper<Nvbg>
{
	virtual void objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		if (boost::python::override o = this->get_override("objectEvent"))
		{
			try {
				o(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity);
			} catch (...) {
				LOG("Problem running Python command 'objectEvent'.");
			}
		}
	}

	void default_objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		return Nvbg::objectEvent(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity);
	}

	virtual bool execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		if (boost::python::override o = this->get_override("execute"))
		{
			try {
				return o(character, to, messageId, xml);
			} catch (...) {
				LOG("Problem running Python command 'execute'.");
			}
		}

		return Nvbg::execute(character, to, messageId, xml);
	}

	bool default_execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		return Nvbg::execute(character, to, messageId, xml);
	}
};
#endif

#if !defined (__ANDROID__) && !defined(SBM_IPHONE)
struct EventHandlerWrap :  EventHandler, boost::python::wrapper<EventHandler>
{
	virtual void executeAction(Event* event)
	{
		if (boost::python::override o = this->get_override("executeAction"))
		{
			try {
				o(event);
			} catch (...) {
				LOG("Problem running Python for EventHandler in 'executeAction'.");
			}
		}

		return EventHandler::executeAction(event);
	}

	void default_executeAction(Event* event)
	{
		EventHandler::executeAction(event);
	}
};
#endif


struct PythonControllerWrap : SmartBody::PythonController, boost::python::wrapper<SmartBody::PythonController>
{
	virtual void start()
	{
		if (boost::python::override o = this->get_override("start"))
		{
			try {
				o();
			} catch (...) {
				LOG("Problem running PythonController command 'start'.");
			}
		}

		return PythonController::start();
	};

	void default_start()
	{
		SmartBody::PythonController::start();
	}

	virtual void init()
	{
		if (boost::python::override o = this->get_override("init"))
		{
			try {
				o();
			} catch (...) {
				LOG("Problem running PythonController command 'init'.");
			}
		}

		return PythonController::init();
	};

	void default_init()
	{
		SmartBody::PythonController::init();
	}

	virtual void evaluate()
	{
		if (boost::python::override o = this->get_override("evaluate"))
		{
			try {
				o();
			} catch (...) {
				LOG("Problem running PythonController command 'evaluate'.");
			}
		}

		return PythonController::evaluate();
	};

	void default_evaluate()
	{
		SmartBody::PythonController::evaluate();
	}

	virtual void stop()
	{
		if (boost::python::override o = this->get_override("stop"))
		{
			try {
				o();
			} catch (...) {
				LOG("Problem running PythonController command 'stop'.");
			}
		}

		return PythonController::stop();
	};

	void default_stop()
	{
		SmartBody::PythonController::stop();
	}
};
#endif




#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif



namespace SmartBody 
{
#ifdef USE_PYTHON
BOOST_PYTHON_MODULE(SmartBody)
{
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

	//boost::python::numeric::array::set_module_and_type("numpy", "ndarray");
	

	// characters

//#ifndef __ANDROID__

	boost::python::class_<SBScene>("SBScene")
		.def("createCharacter", &SBScene::createCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new character given character name. \n Input: character name \nOutput: character object")
		.def("createPawn", &SBScene::createPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new pawn.")
		.def("createFaceDefinition", &SBScene::createFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new face definition with a given name.")
		.def("getFaceDefinition", &SBScene::getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a face definition with a given name.")
		.def("getNumFaceDefinitions", &SBScene::getNumFaceDefinitions, "Returns the number of face definitions.")
		.def("removeCharacter", &SBScene::removeCharacter, "Remove the character given its name. \n Input: character name \n Output: NULL")
		.def("removePawn", &SBScene::removePawn, "Remove the pawn given its name. \n Input: pawn name \n Output: NULL")
		.def("getNumPawns", &SBScene::getNumPawns, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.")
		.def("getNumCharacters", &SBScene::getNumCharacters, "Returns the number of characters.\n Input: NULL \nOutput: number of characters.")
		.def("getPawn", &SBScene::getPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the pawn object given its name. \n Input: pawn name \nOutput: pawn object")
		.def("getCharacter", &SBScene::getCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character object given its name. \n Input: character name \nOutput: character object")
		.def("getMotion", &SBScene::getMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a the motion of given name.")
		.def("getNumMotions", &SBScene::getNumMotions, "Returns the number of motions available.")
		.def("getMotionNames", &SBScene::getMotionNames, "Returns the names of motions available.")
		.def("getPawnNames", &SBScene::getPawnNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of all character names.\n Input: NULL \nOutput: list of pawn names")
		.def("getCharacterNames", &SBScene::getCharacterNames, "Returns a list of all character names.\n Input: NULL \nOutput: list of character names")
		.def("addMotion", &SBScene::addMotion, "Add motion resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL")
		.def("addPose", &SBScene::addPose, "Add pose resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL")
		.def("addAssetPath", &SBScene::addAssetPath, "Add path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL")
		.def("removeAssetPath", &SBScene::removeAssetPath, "Removes a  path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL")
		.def("loadAssets", &SBScene::loadAssets, "Loads the skeletons and motions from the motion paths.")
		.def("setMediaPath",&SBScene::setMediaPath, "Sets the media path.")
		.def("setDefaultCharacter", &SBScene::setDefaultCharacter, "Sets the default character.")
		.def("setDefaultRecipient", &SBScene::setDefaultRecipient, "Sets the default recipient.")
		.def("createSkeleton", &SBScene::createSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new skeleton given a skeleton definition.")
		.def("getEventManager", &SBScene::getEventManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the event manager.")
		.def("command", &SBScene::command, "Runs an old-Style SmartBody command.")
		.def("commandAt", &SBScene::commandAt, "Runs an old-style SmartBody command at a set time in the future.")
		.def("vhmsg", &SBScene::sendVHMsg, "Sends a virtual human message.")
		.def("vhmsg2", &SBScene::sendVHMsg2, "Sends a virtual human message.")
		.def("run", &SBScene::runScript, "Runs a python script.")
		.def("getSimulationManager", &SBScene::getSimulationManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the simulation manager object. \n Input: NULL \n Output: time manager object")
		.def("getProfiler", &SBScene::getProfiler, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the  profiler object. \n Input: NULL \n Output: time profiler object")
		.def("getBmlProcessor", &SBScene::getBmlProcessor, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the bml processor object.\n Input: NULL \nOutput: bml processor object")
		.def("getStateManager", &SBScene::getStateManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the state manager object.\n Input: NULL \nOutput: state manager object")
	;

	boost::python::def("createController", createController, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new controller given a controller type and a controller name.");

	

	

	//#endif

#ifdef __ANDROID__
	boost::python::def("pa", &PyLogger::pa, "Prints an a");
	boost::python::def("pb", &PyLogger::pb, "Prints an b");
	boost::python::def("pc", &PyLogger::pc, "Prints an c");
	boost::python::def("pd", &PyLogger::pd, "Prints an d");
	boost::python::def("pe", &PyLogger::pe, "Prints an e");
	boost::python::def("pf", &PyLogger::pf, "Prints an f");
	boost::python::def("pg", &PyLogger::pg, "Prints an g");
	boost::python::def("ph", &PyLogger::ph, "Prints an h");
	boost::python::def("pi", &PyLogger::pi, "Prints an i");
	boost::python::def("pj", &PyLogger::pj, "Prints an j");
	boost::python::def("pk", &PyLogger::pk, "Prints an k");
	boost::python::def("pl", &PyLogger::pl, "Prints an l");
	boost::python::def("pm", &PyLogger::pm, "Prints an m");
	boost::python::def("pn", &PyLogger::pn, "Prints an n");
	boost::python::def("po", &PyLogger::po, "Prints an o");
	boost::python::def("pp", &PyLogger::pp, "Prints an p");
	boost::python::def("pq", &PyLogger::pq, "Prints an q");
	boost::python::def("pr", &PyLogger::pr, "Prints an r");
	boost::python::def("ps", &PyLogger::ps, "Prints an s");
	boost::python::def("pt", &PyLogger::pt, "Prints an t");
	boost::python::def("pu", &PyLogger::pu, "Prints an u");
	boost::python::def("pv", &PyLogger::pv, "Prints an v");
	boost::python::def("pw", &PyLogger::pw, "Prints an w");
	boost::python::def("px", &PyLogger::px, "Prints an x");
	boost::python::def("py", &PyLogger::py, "Prints an y");
	boost::python::def("pz", &PyLogger::pz, "Prints an z");	
	boost::python::def("p1", &PyLogger::p1, "Prints an 1");
	boost::python::def("p2", &PyLogger::p2, "Prints an 2");
	boost::python::def("p3", &PyLogger::p3, "Prints an 3");
	boost::python::def("p4", &PyLogger::p4, "Prints an 4");
	boost::python::def("p5", &PyLogger::p5, "Prints an 5");
	boost::python::def("p6", &PyLogger::p6, "Prints an 6");
	boost::python::def("p7", &PyLogger::p7, "Prints an 7");
	boost::python::def("p8", &PyLogger::p8, "Prints an 8");
	boost::python::def("p9", &PyLogger::p9, "Prints an 9");
	boost::python::def("p0", &PyLogger::p0, "Prints an 0");

	boost::python::def("openparen", &PyLogger::openparen, "Prints an (");
	boost::python::def("closeparen", &PyLogger::closeparen, "Prints a )");
	boost::python::def("openbracket", &PyLogger::openbracket, "Prints an [");
	boost::python::def("closebracket", &PyLogger::closebracket, "Prints a ]");
	boost::python::def("openbrace", &PyLogger::openbrace, "Prints an {");
	boost::python::def("closebrace", &PyLogger::closebrace, "Prints an }");
	boost::python::def("plus", &PyLogger::plus, "Prints a +");
	boost::python::def("minus", &PyLogger::minus, "Prints a -");
	boost::python::def("aster", &PyLogger::aster, "Prints an *");
	boost::python::def("slash", &PyLogger::slash, "Prints a /");
	boost::python::def("backslash", &PyLogger::backslash, "Prints a \\");
	boost::python::def("comma", &PyLogger::comma, "Prints a ,");
	boost::python::def("colon", &PyLogger::colon,"Prints a :");
	boost::python::def("semicolon", &PyLogger::semicolon, "Prints a ;");
	boost::python::def("equal", &PyLogger::equal, "Prints an =");
	boost::python::def("less", &PyLogger::less, "Prints a ");
	boost::python::def("more", &PyLogger::more, "Prints a >");	
	
	boost::python::def("pspace", &PyLogger::pspace, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");	
	boost::python::def("pnon", &PyLogger::pnon, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");	
	boost::python::def("outlog", &PyLogger::outlog, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
#endif

	//boost::python::def("testGetNDArray",testGetNDArray, "Test ND Array");
	// potential APIs that can be useful:
	// 1. getPawnNames
	// 2. removePawn
	// 3. createPawn 
	// etc. 
	// 

#ifndef __ANDROID__


	// viewers
	boost::python::def("getCamera", getCamera, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the camera object for the viewer. \n Input: NULL \n Output: camera object");
	boost::python::def("getViewer", getViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the visual debugger. \n Input: NULL \n Output: visual debugger");
	boost::python::def("getBmlViewer", getBmlViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the bml viewer object. \n Input: NULL \n Output: bml viewer object");
	boost::python::def("getDataViewer", getDataViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the channel viewer object. \n Input: NULL \n Output: channel viewer object");
	

	// assets
//	boost::python::def("execScripts", execScripts, "Execute a chain of scripts. \n Input: list of script name string e.g. [\"script1 name\", \"script2 name\", ...] \n Output: NULL");
//	boost::python::def("getScript", getScript, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the sequence file object. \n Input: script name \n Output: script object");
	

	// resource access
	boost::python::def("showCommandResources", showCommandResources, "Returns the command resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showMotionResources", showMotionResources, "Returns the motion resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showSkeletonResources", showSkeletonResources, "Returns the motion resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showPathResources", showPathResources, "Returns the path resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showScriptResources", showScriptResources, "Returns the seq file resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showControllerResources", showControllerResources, "Returns the controller resources. \n Input: NULL \n Output: NULL");
	boost::python::def("getResourceLimit", getResourceLimit, "Returns resource up limit. \n Input: NULL \n Output: resource display up limit");
	boost::python::def("setResourceLimit", setResourceLimit, "Set resource up limit. \n Input: resource display up limit \n Output: NULL");	


	// system
	boost::python::def("pythonexit", pythonExit, "Exits the Python interpreter. \n Input: NULL \n Output: NULL");
	boost::python::def("reset", reset, "Reset SBM. \n Input: NULL \n Output: NULL");
	boost::python::def("quit", quitSbm, "Quit SBM. \n Input: NULL \n Output: NULL");
	boost::python::def("printlog", printLog, "Write to the log. \n Input: message string \n Output: NULL");
	boost::python::def("getScene", getScene, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the SmartBody scene object.");



	// class interface
/*	boost::python::class_<Script>("Script")
		.def("printInfo", &Script::print, "Print the content inside this script, this only works for seq script. \n Input: NULL \n Output: NULL")
		.def("run", &Script::run, "Run the script immediately. \n Input: NULL \n Output: NULL")
		.def("abort", &Script::abort, "Abort this running script, this only works for seq script. \n Input: NULL \n Output: NULL")
		;
*/
	boost::python::class_<SBSimulationManager>("SBSimulationManager")
		.def("isRunning", &SBSimulationManager::isRunning, "Returns true if the simulation is currently running.")
		.def("isStarted", &SBSimulationManager::isStarted, "Returns true if the simulation has been started.")
		.def("printInfo", &SBSimulationManager::printInfo, "Print all the timing statistics. \n Input: NULL \n Output: NULL")
		.def("printPerf", &SBSimulationManager::printPerf, "Print performance statistics calculated real time given a time period as input. \n Input: NULL \n Output: NULL")
		.def("getTime", &SBSimulationManager::getTime, "Get the current simulation time. \n Input: NULL \n Output: current simulation time")
		.def("start", &SBSimulationManager::start, "Start the simulation.")
		.def("reset", &SBSimulationManager::reset, "Set the clock time to 0. \n Input: NULL \n Output: NULL")
		.def("pause", &SBSimulationManager::pause, "Pause the clock. \n Input: NULL \n Output: NULL")
		.def("resume", &SBSimulationManager::resume, "Resume the clock. \n Input: NULL \n Output: NULL")
		.def("step", &SBSimulationManager::step, "Running the system in the step mode, user can input how many steps they want to run. \n Input: number of steps at a time \n Output: NULL")
		.def("setSleepFps", &SBSimulationManager::setSleepFps, "Set the sleep fps. Sleep fps defines the target loop rate. \n Input: sleep fps \n Output: NULL")
		.def("setEvalFps", &SBSimulationManager::setEvalFps, "Set the eval fps. Define the minimum interval to evaluate the frame. \n Input: evaluation fps \n Output: NULL")
		.def("setSimFps", &SBSimulationManager::setSimFps, "Set the simulation fps. Add a fixed increment to output time every update. \n Input: simulation fps \n Output: NULL")
		.def("setSleepDt", &SBSimulationManager::setSleepDt, "Set the sleep dt. \n Input: sleep dt \n Output: NULL")
		.def("setEvalDt", &SBSimulationManager::setEvalDt, "Set the eval dt. \n Input: evaluation dt \n Output: NULL")
		.def("setSimDt", &SBSimulationManager::setSimDt, "Set the sim dt. \n Input: simulation dt \n Output: NULL")
		.def("setSpeed", &SBSimulationManager::setSpeed, "Set the speed for real clock time. Actual time would be real time times speed.")
		;

	boost::python::class_<Profiler>("Profiler")
		.def("printLegend", &Profiler::printLegend, "Print time profiler legend. \n Input: NULL \n Output: NULL")
		.def("printStats", &Profiler::printStats, "Print time profiler statistics. \n Input: NULL \n Output: NULL")
		;

	boost::python::class_<Camera>("Camera")
		.def("printInfo", &Camera::printInfo, "Prints all the camera statistics. \n Input: NULL \n Output: NULL")
		.def("reset", &Camera::reset, "Reset camera with camera eye (0 166 185), camera center (0 92 0). \n Input: NULL \n Output: NULL")
		.def("setEye", &Camera::setEye, "Set camera eye position. \n Input: camera eye position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("setCenter", &Camera::setCenter, "Set camera center. \n Input: camera center position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("setScale", &Camera::setScale, "Set camera scale. \n camera scale: NULL \n Output: NULL")
		.def("setTrack", &Camera::setTrack, "Set camera track. \n Input: character name, joint name \n Output: NULL")
		.def("removeTrack", &Camera::removeTrack, "Remove camera track. \n Input: NULL \n Output: NULL")
		;

	boost::python::class_<SrViewer>("Viewer")
		.def("show", &SrViewer::show_viewer, "Shows the viewer.")
		.def("hide", &SrViewer::hide_viewer, "Hides the viewer.")
		;

	boost::python::class_<GenericViewer>("GenericViewer")
		.def("show", &GenericViewer::show_viewer, "Shows the viewer.")
		.def("hide", &GenericViewer::hide_viewer, "Hides the viewer.")
		;

	boost::python::class_<SBBmlProcessor>("BmlProcessor")
		.def("execBML", &SBBmlProcessor::execBML, "Execute a generic bml to a given character. \n Input: character name, bml string \n Output: NULL")
		;

	boost::python::class_<SBState>("SBState")
		.def("addCorrespondancePoints", &SBState::addCorrespondancePoints, "")
		.def("getNumMotions", &SBState::getNumMotions, "")
		.def("getMotion", &SBState::getMotion, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getNumCorrespondancePoints", &SBState::getNumCorrespondancePoints, "")
		.def("getCorrespondancePoints", &SBState::getCorrespondancePoints, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getDimension", &SBState::getDimension, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		;

	boost::python::class_<SBState0D, boost::python::bases<SBState> >("SBState0D")
		.def("addMotion", &SBState0D::addMotion, "")
	;

	boost::python::class_<SBState1D, boost::python::bases<SBState> >("SBState1D")
		.def("addMotion", &SBState1D::addMotion, "")
	;

	boost::python::class_<SBState2D, boost::python::bases<SBState> >("SBState2D")
		.def("addMotion", &SBState2D::addMotion, "")
		.def("addTriangle", &SBState2D::addTriangle, "")
	;

	boost::python::class_<SBState3D, boost::python::bases<SBState> >("SBState3D")
		.def("addMotion", &SBState3D::addMotion, "")
		.def("addTetrahedron", &SBState3D::addTetrahedron, "")
	;

	boost::python::class_<SBTransition>("SBTransition")
		.def("set", &SBTransition::set, "")
		.def("addCorrespondancePoint", &SBTransition::addCorrespondancePoint, "")
		.def("getNumCorrespondancePoints", &SBTransition::getNumCorrespondancePoints, "")
		.def("getCorrespondancePoint", &SBTransition::getCorrespondancePoint, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getFromState", &SBTransition::getFromState, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("getToState", &SBTransition::getToState, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		;

	boost::python::class_<SBStateManager>("SBStateManager")
		.def("createState0D", &SBStateManager::createState0D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D state.")
		.def("createState1D", &SBStateManager::createState1D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D state.")
		.def("createState2D", &SBStateManager::createState2D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 2D state.")
		.def("createState3D", &SBStateManager::createState3D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 3D state.")
		.def("createTransition", &SBStateManager::createTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a transition.")
		.def("getState", &SBStateManager::getState, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a state of a given name.")
		.def("getNumStates", &SBStateManager::getNumStates, "Returns the number of states.")
		.def("getStateNames", &SBStateManager::getStateNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the state names.")
		.def("getTransition", &SBStateManager::getTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a transition with a given name.")
		.def("getNumTransitions", &SBStateManager::getNumTransitions, "Returns the state names.")
		.def("getTransitionNames", &SBStateManager::getTransitionNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the state names.")
		;
/*
	boost::python::class_<Viseme>("Viseme")		
		.def("getName", &Viseme::getVisemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the viseme name. \n Input: NULL \n Output: viseme name")
		.def("getCharacterName", &Viseme::getCharName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the character name. \n Input: NULL \n Output: character name")
		.def("setWeight", &Viseme::setWeight, "Set the weight for current viseme in non-curve mode. \n Input: a list including weight, duration, rampup time, rampdown time e.g. [1,3,1,1] \n Output: NULL")
		.def("setCurve", &Viseme::setCurve, "Set the curve for current viseme in curve mode. \n Input: number of keys, a list of keys e.g. (3, [1,0,3,1,5,0]) \n Output: NULL")
		;
*/

	boost::python::class_<SBFaceDefinition>("SBFaceDefinition")
		.def("getNumVisemes", &SBFaceDefinition::getNumVisemes, "Returns the number of visemes.")
		.def("getVisemeNames", &SBFaceDefinition::getVisemeNames, "Returns the names of the visemes.")
		.def("setViseme", &SBFaceDefinition::setViseme, "Sets a viseme to a particular motion name.")
		.def("getVisemeMotion", &SBFaceDefinition::getVisemeMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a motion associated with a particular viseme.")
		.def("getNumAUs", &SBFaceDefinition::getNumAUs, "Returns the number of Action Units.")
		.def("getAUNumbers", &SBFaceDefinition::getAUNumbers, "Returns the numbers of the Action Units.")
		.def("getAUSide", &SBFaceDefinition::getAUSide, "Returns LEFT, RIGHT, or BOTH, depending on which side is used for the Action Unit.")
		.def("getAUMotion", &SBFaceDefinition::getAUMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion associated with a given Action Unit side: LEFT, RIGHT, or BOTH.")
		.def("setAU", &SBFaceDefinition::setAU, "Sets an Action Unit of a given number to a side and a motion.")
		.def("setFaceNeutral", &SBFaceDefinition::setFaceNeutral, "Sets the neutral face to a particular motion name.")
		;


	boost::python::class_<SBAttribute>("SBAttribute")
		.def("getName", &SBAttribute::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<BoolAttribute, boost::python::bases<SBAttribute> >("BoolAttribute")
		.def("getValue", &BoolAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<StringAttribute, boost::python::bases<SBAttribute> >("StringAttribute")
		.def("getValue", &StringAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<IntAttribute, boost::python::bases<SBAttribute> >("IntAttribute")
		.def("getValue", &IntAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<DoubleAttribute, boost::python::bases<SBAttribute> >("DoubleAttribute")
		.def("getValue", &DoubleAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<SBObject>("SBObject")
		.def("getName", &SBObject::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the object.")
		.def("getNumAttributes", &SBObject::getNumAttributes,  "Returns the number of attributes associated with this object.")
		.def("getAttributeNames", &SBObject::getAttributeNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the attributes names associated with this object.")
		.def("getAttribute", &SBObject::getAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns an attribute of a given name")
		.def("createBoolAttribute", &SBObject::createBoolAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a boolean attribute.")
		.def("createIntAttribute", &SBObject::createIntAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates an integer attribute.")
		.def("createDoubleAttribute", &SBObject::createDoubleAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a double attribute.")
		.def("createStringAttribute", &SBObject::createStringAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a string attribute.")
		.def("setBoolAttribute", &SBObject::setBoolAttribute, "Sets a boolean attribute of a given name to the given value.")
		.def("setIntAttribute", &SBObject::setIntAttribute, "Sets an integer attribute of a given name to the given value.")
		.def("setDoubleAttribute", &SBObject::setDoubleAttribute, "Sets a floating point attribute of a given name to the given value.")
		.def("setStringAttribute", &SBObject::setStringAttribute, "Sets a string attribute of a given name to the given value.")
		.def("setVec3Attribute", &SBObject::setVec3Attribute, "Sets a vector attribute of a given name to the given value.")
		.def("setMatrixAttribute", &SBObject::setMatrixAttribute, "Sets a matrix attribute of a given name to the given value.")
		;

	boost::python::class_<SBMotion, boost::python::bases<SBObject> >("SBMotion")
		//.def(boost::python::init<std::string>())
		.def("getMotionFileName", &SBMotion::getMotionFileName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion file name. \n Input: NULL \n Output: motion file name")
		.def("getNumFrames", &SBMotion::getNumFrames, "Returns the number of frames inside this motion. \n Input: NULL \n Output: number of frames in the motion")
		.def("getFrameData", &SBMotion::getFrameData, "Returns the frame data given frame index. \n Input: frame index \n Output: a list of frame data")
		.def("getFrameSize", &SBMotion::getFrameSize, "Returns the frame size. \n Input: NULL \n Output: frame size (how many data does one frame include)")
		.def("getNumChannels", &SBMotion::getNumChannels, "Returns the number of channels for this motion. \n Input: NULL \n Output: number of channels for this motion")
		.def("getChannels", &SBMotion::getChannels, "Returns the channels + type inside the skeleton. \n Input: NULL \n Output: channel name and type")
		.def("checkSkeleton", &SBMotion::checkSkeleton, "Print out all the motion channels and compare it with the given skeleton channels. Mark '+' in front if the skeleton channel exists in the motion. \n Input: skeleton file name \n Output: NULL")
		.def("connect", &SBMotion::connect, "Connect current motion to a skeleton object so the channels inside the motion are mapped to the channels inside skeleton. \n Input: Skeleton Object \n Output: NULL")
		.def("disconnect", &SBMotion::disconnect, "Disconnect current motion with current skeleton object. \n Input: NULL \n Output: NULL")
		.def("mirror", &SBMotion::mirror, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Mirrors the motion.")
		;


	boost::python::class_<SBController, boost::python::bases<SBObject> >("SBController")
		.def(boost::python::init<>())
		.def("setName", &SBController::setName, "Sets the name for the controller.")
		.def("getName", &SBController::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name for this controller.")
		.def("getType", &SBController::getType, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the type for this controller.")
		.def("setIgnore", &SBController::setIgnore, "Ignore this controller when evaluating controller tree.")
		.def("isIgnore", &SBController::isIgnore, "Will the controller be ignored during evaluation.")
		.def("setDebug", &SBController::setDebug, "Sets the debug state for this controller.")
		.def("isDebug", &SBController::isDebug, "Is the controller in a debug state?")
		.def("getDuration", &SBController::getDuration, "Gets the controller's duration.")
		;

	boost::python::class_<MeCtScheduler2, boost::python::bases<SBController> > ("SchedulerController")
		.def("getNumTracks", &MeCtScheduler2::count_children, "Returns the number of children/tracks.")
		;

	boost::python::class_<MeCtMotion, boost::python::bases<SBController> > ("MotionController")
		//	.def("getMotion", &MeCtMotion::motion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion associated with this motion controller.")
		;

	boost::python::class_<MeCtEyeLid, boost::python::bases<SBController> > ("EyelidController")
		;

	boost::python::class_<MeCtLocomotion, boost::python::bases<SBController> > ("LocomotionController")
		;

	boost::python::class_<MeCtGaze, boost::python::bases<SBController> > ("GazeController")
		;

	//boost::python::class_<MeCtReach, boost::python::bases<SBController> > ("ReachController")	;

	boost::python::class_<MeCtCurveWriter, boost::python::bases<SBController> > ("CurveWriterController")
		;

	boost::python::class_<SBSkeleton>("SBSkeleton")
	//	.def(boost::python::init<>())
		.def(boost::python::init<std::string>())
		.def("load", &SBSkeleton::load, "Loads the skeleton definition from the given skeleton name.")
		.def("getName", &SBSkeleton::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the skeleton.")
		.def("getNumJoints", &SBSkeleton::getNumJoints, "Returns the number of joints for this skeleton.")
		.def("getJointNames", &SBSkeleton::getJointNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the joint names for this skeleton.")
		.def("getJointByName", &SBSkeleton::getJointByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint of a given name.")
		.def("getJoint", &SBSkeleton::getJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint with a given index.")
		.def("getNumChannels", &SBSkeleton::getNumChannels, "Returns the number of the channels inside the skeleton.")
		.def("getChannelType", &SBSkeleton::getChannelType, "Returns the type of the channel of a given index.")
		.def("getChannelSize", &SBSkeleton::getChannelSize, "Returns the size of the channel given index.")	
		;

	boost::python::class_<SBJoint, boost::python::bases<SBObject> >("SBJoint")
		.def(boost::python::init<>())
		.def("setName", &SBJoint::setName, "Set the name of the joint.")
		.def("getName", &SBJoint::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the joint.")
		.def("getParent", &SBJoint::getParent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the parent joint.")
		.def("setParent", &SBJoint::setParent, "Sets the parent joint.")
		.def("getNumChildren", &SBJoint::getNumChildren, "Returns the number of child joints.")
		.def("getChild", &SBJoint::getChild, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the child joint with a given index.")
		.def("getSkeleton", &SBJoint::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton.")
		.def("setSkeleton", &SBJoint::setSkeleton, "Sets the skeleton.")
		.def("getOffset", &SBJoint::getOffset, "Returns the offset of the joint from the parent joint.") 
		.def("setOffset", &SBJoint::setOffset, "Sets the offset of the joint from the parent joint.")
		.def("getIndex", &SBJoint::getIndex, "Returns the index of the joint in current skeleton.")
		.def("getPosition", &SBJoint::getPosition, "Returns the current position of the joint in global coordinates.")
		.def("getQuat", &SBJoint::getQuaternion, "Returns the current quaterion of the joint in global coordinates.")
		.def("getMatrixGlobal", &SBJoint::getMatrixGlobal, "Returns the matrix of the joint in global coordinates.")
		.def("getMatrixLocal", &SBJoint::getMatrixLocal, "Returns the matrix of the joint in local coordinates.")
		.def("addChild", &SBJoint::addChild, "Add a child joint to current joint.")
		.def("setUseRotation", &SBJoint::setUseRotation, "Allows the joint to use rotation channels.")	
		.def("getUseRotation", &SBJoint::isUseRotation, "Determines if the joint uses rotation channels.")	
		.def("setUsePosition", &SBJoint::setUsePosition, "Allows the joint to use position channels.")	
		.def("isUsePosition", &SBJoint::isUsePosition, "Determines if the joint uses position channels.")	
		;

	boost::python::class_<SBBehavior, boost::python::bases<SBObject> >("SBBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getType", &SBBehavior::getType, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the type of the behavior.")
		;

	boost::python::class_<GazeBehavior, boost::python::bases<SBBehavior> >("GazeBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getGazeTarget", &GazeBehavior::getGazeTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the gaze target.")
	;

	boost::python::class_<LocomotionBehavior, boost::python::bases<SBBehavior> >("LocomotionBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getLocomotionTarget", &LocomotionBehavior::getLocomotionTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the locomotion target as a vector.")
	;

	boost::python::class_<SpeechBehavior, boost::python::bases<SBBehavior> >("SpeechBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getUtterance", &SpeechBehavior::getUtterance, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the current utterance.")
	;

	boost::python::class_<SBPawn, boost::python::bases<SBObject> >("SBPawn")
		.def("getName", &SBPawn::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the pawn..")
		.def("getSkeleton", &SBPawn::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton object of the pawn.")
		.def("setSkeleton", &SBPawn::setSkeleton, "Attaches the skeleton to the character.")
		.def("setName", &SBPawn::setName, "Sets or changes the name of the character.")
		.def("getPosition", &SBPawn::getPosition, "Returns the current position of the character's world offset.")
		.def("getOrientation", &SBPawn::getOrientation, "Returns the current orientation of the character's world offset.")
		.def("setPosition", &SBPawn::setPosition, "Sets the current position of the character's world offset.")
		.def("setOrientation", &SBPawn::setOrientation, "Set the current orientation of the character's world offset.")
		.def("setHPR", &SBPawn::setHPR, "Sets the heading, pitch and roll of the character's world offset.")
		.def("getHPR", &SBPawn::getHPR, "Gets the heading, pitch and roll of the character's world offset.")
	;

	boost::python::class_<SBCharacter, boost::python::bases<SBPawn, SBObject> >("SBCharacter")
		//.def(boost::python::init<std::string, std::string>())
		.def("setMeshMap", &SBCharacter::setMeshMap, "Set the OpenCollada file for the character which contains all the smoothbinding information.")
		.def("addMesh", &SBCharacter::addMesh, "Add obj mesh to current character for smoothbinding.")
		.def("isAutomaticPruning", &SBCharacter::isAutomaticPruning, "Returns true if the character's cotnroller are automatically pruned.")
		.def("setAutomaticPruning", &SBCharacter::setAutomaticPruning, "Toggles the automatic pruning mechanism on or off.")
		.def("pruneControllers", &SBCharacter::pruneControllers, "Prunes the controller tree.")
		.def("setSoftEyes", &SBCharacter::setSoftEyes, "Sets the soft eyes feature.")
		.def("isSoftEyes", &SBCharacter::isSoftEyes, "Returns the value of the soft eyes feature.")
		.def("setUseVisemeCurves", &SBCharacter::setUseVisemeCurves, "Use curves when interpreting visemes.")
		.def("isUseVisemeCurves", &SBCharacter::isUseVisemeCurves, "Are curves used when interpreting visemes.")
		.def("setVisemeTimeOffset", &SBCharacter::setVisemeTimeOffset, "Set the time delay for viseme curve mode.")
		.def("getVisemeTimeOffset", &SBCharacter::getVisemeTimeOffset, "Get the time delay for viseme curve mode.")
		.def("getNumControllers", &SBCharacter::getNumControllers, "Returns number of top level controllers inside this character.")
		.def("createStandardControllers", &SBCharacter::createStandardControllers, "Returns number of top level controllers inside this character.")
		//.def("getNumVisemes", &SBCharacter::getNumVisemes, "Returns the number of visemes.")
		.def("getControllerByIndex", &SBCharacter::getControllerByIndex, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the ith controller.")
		.def("getControllerByName", &SBCharacter::getControllerByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the controller with the given name.")
		.def("addController", &SBCharacter::addController, "Adds a controller to a character.")
		.def("setVoice", &SBCharacter::setVoice, "Sets the voice type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceCode", &SBCharacter::setVoiceBackupCode, "Sets the voice code. For audiofile type, this is a path.")
		.def("setVoiceBackup", &SBCharacter::setVoiceBackup, "Sets the voice backup type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceBackupCode", &SBCharacter::setVoiceBackupCode, "Sets the voice backup code. For audiofile type, this is a path.")
		.def("setFaceDefinition", &SBCharacter::setFaceDefinition, "Sets face definition (visemes, action units) for a character.")
		.def("getFaceDefinition", &SBCharacter::getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets face definition (visemes, action units) for a character.")
		.def("getHeight", &SBCharacter::getHeight, "Gets the height of the character.")
		.def("getNumBehaviors", &SBCharacter::getNumBehaviors, "Returns the number of behaviors of the character.")
		.def("getBehavior", &SBCharacter::getBehavior, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the ith behavior of the character.")

#endif
		//.def("getFaceDefinition", &SBCharacter::getFaceDefinition, "Gets face definition (visemes, action units) for a character.")
#ifndef __ANDROID__
		.def("setNvbg", &SBCharacter::setNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the NVBG handler for this character.")
		.def("getNvbg", &SBCharacter::getNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the NVBG handler for this character.")
		.def("setMiniBrain", &SBCharacter::setMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the mini brain handler for this character.")
		.def("getMiniBrain", &SBCharacter::getMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the mini brain handler for this character.")

#endif
		;

	boost::python::class_<Event>("Event")
		.def("getType", &Event::getType, "Returns the event type.")
		.def("setType", &Event::setType, "Sets the event type.")
		.def("getParameters", &Event::getParameters, "Returns the event parameters.")
		.def("setParameters", &Event::setParameters, "Sets the event parameters.")
		;

	boost::python::class_<EventManager>("EventManager")
		.def("addEventHandler", &EventManager::addEventHandler, "Returns the event type.")
		.def("removeEventHandler", &EventManager::removeEventHandler, "Returns the event type.")
		.def("getNumHandlers", &EventManager::getNumEventHandlers, "Gets the number of event handlers.")
		.def("getEventHandler", &EventManager::getEventHandler, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the number of event handlers.")
		;

#ifndef __ANDROID__
	boost::python::class_<SrVec>("SrVec")
		.def(boost::python::init<>())
		.def(boost::python::init<float, float, float>())
		.def("getData", &SrVec::getData, "gets the x,y,z values")
		.def("setData", &SrVec::setData, "sets the x,y,z values")
		;

	boost::python::class_<SrMat>("SrMat")
		.def(boost::python::init<>())
		.def("getData", &SrMat::getData, "gets the data in the matrix at r,c")
		.def("setData", &SrMat::setData, "sets the data in the matrix at r,c")
		;

	boost::python::class_<SrQuat>("SrQuat")
		.def(boost::python::init<>())
		.def("getData", &SrQuat::getData, "gets the data in the quaterion at location indicated by the index w,x,y,z")
		.def("setData", &SrQuat::setData, "sets the data in the quaterion at location indicated by the index w,x,y,z")
		;
#endif

#ifndef __ANDROID__
	boost::python::class_<NvbgWrap, boost::noncopyable>("Nvbg")
		.def("objectEvent", &Nvbg::objectEvent, &NvbgWrap::default_objectEvent, "An event indicating that an object of interest is present.")
		.def("execute", &Nvbg::execute, &NvbgWrap::default_execute, "Execute the NVBG processor.")
		;


	boost::python::class_<EventHandlerWrap, boost::noncopyable>("EventHandler")
		.def("executeAction", &EventHandler::executeAction, &EventHandlerWrap::default_executeAction, "Execute the event handler.")
		;
#endif

	boost::python::class_<PythonControllerWrap, boost::python::bases<SBController>, boost::noncopyable> ("PythonController")
		.def("start", &PythonController::start, &PythonControllerWrap::default_start, "start.")
		.def("stop", &PythonController::stop, &PythonControllerWrap::default_stop, "stop.")
		.def("init", &PythonController::init, &PythonControllerWrap::default_init, "init.")
		.def("evaluate", &PythonController::evaluate, &PythonControllerWrap::default_evaluate, "evaluate.")
		;

	}
#endif


}


void initPython(std::string pythonLibPath)
{	
#ifdef USE_PYTHON
#ifdef __ANDROID__
	Py_SetProgramName("/sdcard/sbmmedia/python/");
#else
	Py_SetProgramName("../../../../core/smartbody/Python26/");
#endif	
	Py_Initialize();
	try {
		//LOG("Before import __main__");
		boost::python::object objectMain = boost::python::import("__main__");
		boost::python::object objectDict = objectMain.attr("__dict__");
		//LOG("Before import sys");
		PyRun_SimpleString("import sys");
		// set the proper python path
		std::stringstream strstr;
		strstr << "sys.path.append(\"";
		strstr << pythonLibPath;
		strstr << "\");";	
		PyRun_SimpleString(strstr.str().c_str());

		// add path to site-packages
		std::string pythonSitePackagePath = pythonLibPath + "/site-packages";
		strstr.clear();
		strstr << "sys.path.append(\"";
		strstr << pythonSitePackagePath;
		strstr << "\");";
		PyRun_SimpleString(strstr.str().c_str());

		//LOG("PyGetPath = %s\")


		//LOG("Before initSmartBody");
		SmartBody::initSmartBody();

		//LOG("After initSmartBody");
		if (PyErr_Occurred())
			PyErr_Print();
		// redirect stdout
		//LOG("Before redirect stdout");

#ifdef PYLOG
#ifdef __ANDROID__
		const char* pyfilename = "/sdcard/sbmmedia/pylog.txt";
#else
		const char* pyfilename = "C:\\SbmAndroid\\android\\pylog.txt";
#endif
		FILE* file = fopen(pyfilename,"rt");
		if (file)
		{
			LOG("Open file success\n");		
			PyRun_SimpleFile(file,pyfilename);
			PyRun_SimpleString("logwriter = WritableObject()");
			//#ifndef __ANDROID__
			PyRun_SimpleString("sys.stdout = logwriter");
			PyRun_SimpleString("sys.stderr = logwriter");
		}
		else
		{
			LOG("Open File Fail!!!\n");
		}	
#else
		PyRun_SimpleString("class WritableObject:\n\tdef __init__(self):\n\t\tself.content = []\n\tdef write(self, string):\n\t\tprintlog(string)\n");
		PyRun_SimpleString("logwriter = WritableObject()");
		PyRun_SimpleString("sys.stdout = logwriter");
		PyRun_SimpleString("sys.stderr = logwriter");
#endif		
		
		if (PyErr_Occurred())
			PyErr_Print();

	
		//LOG("before import os");
		PyRun_SimpleString("from os import *");
		//LOG("before import Smartbody");
		PyRun_SimpleString("from SmartBody import *");
		//LOG("before import pydoc");
		//PyRun_SimpleString("from pydoc import *");
#ifndef __ANDROID__
		PyRun_SimpleString("scene = getScene()");
		PyRun_SimpleString("bml = scene.getBmlProcessor()");
		PyRun_SimpleString("sim = scene.getSimulationManager()");
#endif
		//LOG("After import os");

		if (PyErr_Occurred())
			PyErr_Print();
		

	} catch (...) {
		PyErr_Print();
		//LOG("PyError Exception");
	}
#endif
}


