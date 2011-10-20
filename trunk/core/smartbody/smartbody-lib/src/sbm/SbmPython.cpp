#include "SbmPython.h"
#include "SbmPythonClass.h"
#include "me/me_ct_scheduler2.h"
#include "VisemeMap.hpp"
#include "nvbg.h"
#include "SBBehavior.h"

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
	//boost::python::numeric::array::set_module_and_type("numpy", "ndarray");
	

	// characters

//#ifndef __ANDROID__
	boost::python::def("getNumPawns", getNumPawns, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("getNumCharacters", getNumCharacters, "Returns the number of characters.\n Input: NULL \nOutput: number of characters.");
	boost::python::def("getCharacterByIndex", getCharacterByIndex, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character object given its index. \n Input: character index \nOutput: character object");
	boost::python::def("getCharacter", getCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character object given its name. \n Input: character name \nOutput: character object");
	boost::python::def("getCharacterNames", getCharacterNames, "Returns a list of all character names.\n Input: NULL \nOutput: list of character names");
	boost::python::def("getEventManager", getEventManager, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the event manager.");
	boost::python::def("getMotion", getMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a the motion of given name.");
	boost::python::def("run", runScript, "Runs a python script.");
	boost::python::def("vhmsg", sendVHMsg, "Sends a virtual human message.");
	boost::python::def("vhmsg2", sendVHMsg2, "Sends a virtual human message.");
	//#endif

#ifdef __ANDROID__
	boost::python::def("pa", &PyLogger::pa, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pb", &PyLogger::pb, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pc", &PyLogger::pc, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pd", &PyLogger::pd, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pe", &PyLogger::pe, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pf", &PyLogger::pf, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pg", &PyLogger::pg, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("ph", &PyLogger::ph, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pi", &PyLogger::pi, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pj", &PyLogger::pj, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pk", &PyLogger::pk, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pl", &PyLogger::pl, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pm", &PyLogger::pm, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pn", &PyLogger::pn, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("po", &PyLogger::po, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pp", &PyLogger::pp, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pq", &PyLogger::pq, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pr", &PyLogger::pr, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("ps", &PyLogger::ps, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pt", &PyLogger::pt, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pu", &PyLogger::pu, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pv", &PyLogger::pv, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pw", &PyLogger::pw, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("px", &PyLogger::px, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("py", &PyLogger::py, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("pz", &PyLogger::pz, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");	
	boost::python::def("p1", &PyLogger::p1, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p2", &PyLogger::p2, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p3", &PyLogger::p3, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p4", &PyLogger::p4, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p5", &PyLogger::p5, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p6", &PyLogger::p6, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p7", &PyLogger::p7, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p8", &PyLogger::p8, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");	
	boost::python::def("p9", &PyLogger::p9, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("p0", &PyLogger::p0, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");

	boost::python::def("openparen", &PyLogger::openparen, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("closeparen", &PyLogger::closeparen, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("openbracket", &PyLogger::openbracket, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("closebracket", &PyLogger::closebracket, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("openbrace", &PyLogger::openbrace, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("closebrace", &PyLogger::closebrace, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("plus", &PyLogger::plus, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("minus", &PyLogger::minus, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("aster", &PyLogger::aster, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("slash", &PyLogger::slash, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("backslash", &PyLogger::backslash, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("comma", &PyLogger::comma, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("colon", &PyLogger::colon, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("semicolon", &PyLogger::semicolon, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("equal", &PyLogger::equal, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("less", &PyLogger::less, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");
	boost::python::def("more", &PyLogger::more, "Returns the number of pawns.\n Input: NULL \nOutput: number of pawns.");	
	
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
	boost::python::def("createCharacter", createCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new character given character name. \n Input: character name \nOutput: character object");
	boost::python::def("removeCharacter", removeCharacter, "Remove the character given its name. \n Input: character name \n Output: NULL");
	boost::python::def("createSkeleton", createSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new skeleton given a skeleton definition.");
	boost::python::def("createController", createController, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new controller given a controller type and a controller name.");

	// bml
	boost::python::def("getBmlProcessor", getBmlProcessor, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the bml processor object.\n Input: NULL \nOutput: bml processor object");

	// viewers
	boost::python::def("getCamera", getCamera, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the camera object for the viewer. \n Input: NULL \n Output: camera object");
	boost::python::def("getViewer", getViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the sbm viewer object. \n Input: NULL \n Output: sbm viewer object");
	boost::python::def("getBmlViewer", getBmlViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the bml viewer object. \n Input: NULL \n Output: bml viewer object");
	boost::python::def("getDataViewer", getDataViewer, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the channel viewer object. \n Input: NULL \n Output: channel viewer object");
	

	// assets
	boost::python::def("setMediaPath", setMediaPath, "Sets the media path.");
	boost::python::def("execScripts", execScripts, "Execute a chain of scripts. \n Input: list of script name string e.g. [\"script1 name\", \"script2 name\", ...] \n Output: NULL");
	boost::python::def("getScript", getScript, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the sequence file object. \n Input: script name \n Output: script object");
	boost::python::def("addMotion", addMotion, "Add motion resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL");
	boost::python::def("addPose", addPose, "Add pose resource given filepath and recursive flag. \n Input: path, recursive flag(boolean variable indicating whether to tranverse all the children directories) \n Output: NULL");
	boost::python::def("addAssetPath", addAssetPath, "Add path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL");
	boost::python::def("removeAssetPath", removeAssetPath, "Removes a  path resource given path type and actual path string. \n Input: type(can be seq|me|ME), path \n Output: NULL");
	boost::python::def("loadAssets", loadAssets, "Loads the skeletons and motions from the motion paths.");

	// resource access
	boost::python::def("showCommandResources", showCommandResources, "Returns the command resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showMotionResources", showMotionResources, "Returns the motion resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showSkeletonResources", showSkeletonResources, "Returns the motion resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showPathResources", showPathResources, "Returns the path resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showScriptResources", showScriptResources, "Returns the seq file resources. \n Input: NULL \n Output: NULL");
	boost::python::def("showControllerResources", showControllerResources, "Returns the controller resources. \n Input: NULL \n Output: NULL");
	boost::python::def("getResourceLimit", getResourceLimit, "Returns resource up limit. \n Input: NULL \n Output: resource display up limit");
	boost::python::def("setResourceLimit", setResourceLimit, "Set resource up limit. \n Input: resource display up limit \n Output: NULL");	

	// timing
	boost::python::def("getSimulationManager", getSimulationManager, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the simulation manager object. \n Input: NULL \n Output: time manager object");
	boost::python::def("getProfiler", getProfiler, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the  profiler object. \n Input: NULL \n Output: time profiler object");

	// system
	boost::python::def("pythonexit", pythonExit, "Exits the Python interpreter. \n Input: NULL \n Output: NULL");
	boost::python::def("reset", reset, "Reset SBM. \n Input: NULL \n Output: NULL");
	boost::python::def("quit", quitSbm, "Quit SBM. \n Input: NULL \n Output: NULL");
	boost::python::def("printlog", printLog, "Write to the log. \n Input: message string \n Output: NULL");


	// class interface
	boost::python::class_<Script>("Script")
		.def("printInfo", &Script::print, "Print the content inside this script, this only works for seq script. \n Input: NULL \n Output: NULL")
		.def("run", &Script::run, "Run the script immediately. \n Input: NULL \n Output: NULL")
		.def("abort", &Script::abort, "Abort this running script, this only works for seq script. \n Input: NULL \n Output: NULL")
		;

	boost::python::class_<SimulationManager>("SimulationManager")
		.def("isRunning", &SimulationManager::isRunning, "Returns true if the simulation is currently running.")
		.def("isStarted", &SimulationManager::isStarted, "Returns true if the simulation has been started.")
		.def("printInfo", &SimulationManager::printInfo, "Print all the timing statistics. \n Input: NULL \n Output: NULL")
		.def("printPerf", &SimulationManager::printPerf, "Print performance statistics calculated real time given a time period as input. \n Input: NULL \n Output: NULL")
		.def("getTime", &SimulationManager::getTime, "Get the current simulation time. \n Input: NULL \n Output: current simulation time")
		.def("start", &SimulationManager::start, "Start the simulation.")
		.def("reset", &SimulationManager::reset, "Set the clock time to 0. \n Input: NULL \n Output: NULL")
		.def("pause", &SimulationManager::pause, "Pause the clock. \n Input: NULL \n Output: NULL")
		.def("resume", &SimulationManager::resume, "Resume the clock. \n Input: NULL \n Output: NULL")
		.def("step", &SimulationManager::step, "Running the system in the step mode, user can input how many steps they want to run. \n Input: number of steps at a time \n Output: NULL")
		.def("setSleepFps", &SimulationManager::setSleepFps, "Set the sleep fps. Sleep fps defines the target loop rate. \n Input: sleep fps \n Output: NULL")
		.def("setEvalFps", &SimulationManager::setEvalFps, "Set the eval fps. Define the minimum interval to evaluate the frame. \n Input: evaluation fps \n Output: NULL")
		.def("setSimFps", &SimulationManager::setSimFps, "Set the simulation fps. Add a fixed increment to output time every update. \n Input: simulation fps \n Output: NULL")
		.def("setSleepDt", &SimulationManager::setSleepDt, "Set the sleep dt. \n Input: sleep dt \n Output: NULL")
		.def("setEvalDt", &SimulationManager::setEvalDt, "Set the eval dt. \n Input: evaluation dt \n Output: NULL")
		.def("setSimDt", &SimulationManager::setSimDt, "Set the sim dt. \n Input: simulation dt \n Output: NULL")
		.def("setSpeed", &SimulationManager::setSpeed, "Set the speed for real clock time. Actual time would be real time times speed.")
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

	boost::python::class_<GazeBML>("GazeBML")
		.def("setTarget", &GazeBML::setTarget, "Set the target of gaze.  \n Input: target character name \n Output: NULL")
		.def("setDirection", &GazeBML::setDirection, "Set the direction of gaze. \n Input: direction string e.g. \"up|down|left\" \n Output: NULL")
		.def("setAngle", &GazeBML::setAngle, "Set the angle of gaze. \n Input: angle string e.g. \"90\" \n Output: NULL")
		.def("setSpeed", &GazeBML::setSpeed, "Set the speed of gaze on lumbar neck and eye. \n Input: speed string list for lumbar neck eye respectively e.g. [\"10\",\"40\",\"50\"] \n Output: NULL")
		.def("setSmooth", &GazeBML::setSmoothing, "Set the smooth of gaze on lumbar neck and eye. \n Input: smooth string list for lumbar neck eye respectively e.g. [\"10\",\"40\",\"50\"] \n Output: NULL")
		;

	boost::python::class_<BmlProcessor>("BmlProcessor")
		.def("execAnimation", &BmlProcessor::execAnimation, "Execute a animation bml command. \n Input: character name, animation name \n Output: NULL")
		.def("execPosture", &BmlProcessor::execPosture, "Execute a posture bml command. \n Input: character name, posture name \n Output: NULL")
		.def("execGaze", &BmlProcessor::execGaze, "Execute a gaze bml command. \n Input: character name, GazeBML object \n Output: NULL")
		.def("execBML", &BmlProcessor::execBML, "Execute a generic bml to a given character. \n Input: character name, bml string \n Output: NULL")
		;

	boost::python::class_<Viseme>("Viseme")		
		.def("getName", &Viseme::getVisemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the viseme name. \n Input: NULL \n Output: viseme name")
		.def("getCharacterName", &Viseme::getCharName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the character name. \n Input: NULL \n Output: character name")
		.def("setWeight", &Viseme::setWeight, "Set the weight for current viseme in non-curve mode. \n Input: a list including weight, duration, rampup time, rampdown time e.g. [1,3,1,1] \n Output: NULL")
		.def("setCurve", &Viseme::setCurve, "Set the curve for current viseme in curve mode. \n Input: number of keys, a list of keys e.g. (3, [1,0,3,1,5,0]) \n Output: NULL")
		;

	boost::python::class_<Motion>("Motion")
		.def(boost::python::init<std::string>())
		.def("getMotionFileName", &Motion::getMotionFileName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion file name. \n Input: NULL \n Output: motion file name")
		.def("getMotionName", &Motion::getMotionName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion name. \n Input: NULL \n Output: motion name")
		.def("getNumFrames", &Motion::getNumFrames, "Returns the number of frames inside this motion. \n Input: NULL \n Output: number of frames in the motion")
		.def("getFrameData", &Motion::getFrameData, "Returns the frame data given frame index. \n Input: frame index \n Output: a list of frame data")
		.def("getFrameSize", &Motion::getFrameSize, "Returns the frame size. \n Input: NULL \n Output: frame size (how many data does one frame include)")
		.def("getNumChannel", &Motion::getNumChannel, "Returns the number of channels for this motion. \n Input: NULL \n Output: number of channels for this motion")
		.def("getChannels", &Motion::getChannels, "Returns the channels + type inside the skeleton. \n Input: NULL \n Output: channel name and type")
		.def("checkSkeleton", &Motion::checkSkeleton, "Print out all the motion channels and compare it with the given skeleton channels. Mark '+' in front if the skeleton channel exists in the motion. \n Input: skeleton file name \n Output: NULL")
		.def("connect", &Motion::connect, "Connect current motion to a skeleton object so the channels inside the motion are mapped to the channels inside skeleton. \n Input: Skeleton Object \n Output: NULL")
		.def("disconnect", &Motion::disconnect, "Disconnect current motion with current skeleton object. \n Input: NULL \n Output: NULL")
		;

	boost::python::def("getFaceDefinition", getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a defined viseme and AU set given the first parameter. To get the default set, use \"_default_\"");
	boost::python::class_<FaceDefinition>("FaceDefinition")
		.def("getNumVisemes", &FaceDefinition::getNumVisemes, "Returns the number of visemes.")
		.def("setViseme", &FaceDefinition::setViseme, "Sets a viseme to a particular motion name.")
		.def("getVisemeMotion", &FaceDefinition::getVisemeMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a motion associated with a particular viseme.")
		.def("getNumAUs", &FaceDefinition::getNumAUs, "Returns the number of Action Units.")
		.def("setAU", &FaceDefinition::setAU, "Sets an Action Unit of a given number to a side and a motion.")
		.def("setFaceNeutral", &FaceDefinition::setFaceNeutral, "Sets the neutral face to a particular motion name.")
		;


	boost::python::class_<DAttribute>("SBAttribute")
		.def("getName", &DAttribute::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<BoolAttribute, boost::python::bases<DAttribute> >("BoolAttribute")
		.def("getValue", &BoolAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<StringAttribute, boost::python::bases<DAttribute> >("StringAttribute")
		.def("getValue", &StringAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<IntAttribute, boost::python::bases<DAttribute> >("IntAttribute")
		.def("getValue", &IntAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<DoubleAttribute, boost::python::bases<DAttribute> >("DoubleAttribute")
		.def("getValue", &DoubleAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
	;

	boost::python::class_<DObject>("SBObject")
		.def("getNumAttributes", &DObject::getNumAttributes,  "Returns the number of attributes associated with this object.")
		.def("getAttribute", &DObject::getAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns an attribute of a given name")
		.def("createBoolAttribute", &DObject::createBoolAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a boolean attribute.")
		.def("createIntAttribute", &DObject::createIntAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates an integer attribute.")
		.def("createDoubleAttribute", &DObject::createDoubleAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a double attribute.")
		.def("createStringAttribute", &DObject::createStringAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a string attribute.")
		;

	boost::python::class_<SBController, boost::python::bases<DObject> >("SBController")
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
		.def("getJointByName", &SBSkeleton::getJointByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint of a given name.")
		.def("getJoint", &SBSkeleton::getJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint with a given index.")
		.def("getNumChannels", &SBSkeleton::getNumChannels, "Returns the number of the channels inside the skeleton.")
		.def("getChannelType", &SBSkeleton::getChannelType, "Returns the type of the channel of a given index.")
		.def("getChannelSize", &SBSkeleton::getChannelSize, "Returns the size of the channel given index.")	
		;

	boost::python::class_<SBJoint>("SBJoint")
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

	boost::python::class_<SBBehavior, boost::python::bases<DObject> >("SBBehavior")
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


	boost::python::class_<SBCharacter, boost::python::bases<DObject> >("SBCharacter")
		//.def(boost::python::init<std::string, std::string>())
		.def("getName", &SBCharacter::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the character.")
		.def("setName", &SBCharacter::setName, "Sets or changes the name of the character.")
		.def("getSkeleton", &SBCharacter::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton object of the character.")
		.def("setSkeleton", &SBCharacter::setSkeleton, "Attaches the skeleton to the character.")
		.def("setMeshMap", &SBCharacter::setMeshMap, "Set the OpenCollada file for the character which contains all the smoothbinding information.")
		.def("addMesh", &SBCharacter::addMesh, "Add obj mesh to current character for smoothbinding.")
		.def("getPosition", &SBCharacter::getPosition, "Returns the current position of the character's world offset.")
		.def("getOrientation", &SBCharacter::getOrientation, "Returns the current orientation of the character's world offset.")
		.def("setPosition", &SBCharacter::setPosition, "Sets the current position of the character's world offset.")
		.def("setOrientation", &SBCharacter::setOrientation, "Set the current orientation of the character's world offset.")
		.def("setHPR", &SBCharacter::setHPR, "Sets the heading, pitch and roll of the character's world offset.")
		.def("getHPR", &SBCharacter::getHPR, "Gets the heading, pitch and roll of the character's world offset.")
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
		PyRun_SimpleString("bml = getBmlProcessor()");
		PyRun_SimpleString("sim = getSimulationManager()");
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


