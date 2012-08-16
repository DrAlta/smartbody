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
void pythonFuncs2()
{
	// viewers
	boost::python::def("getCamera", getCamera, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the camera object for the viewer. \n Input: NULL \n Output: camera object");
	boost::python::def("getViewer", getViewer, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the visual debugger. \n Input: NULL \n Output: visual debugger");
	

	// assets
//	boost::python::def("execScripts", execScripts, "Execute a chain of scripts. \n Input: list of script name string e.g. [\"script1 name\", \"script2 name\", ...] \n Output: NULL");
//	boost::python::def("getScript", getScript, boost::python::return_value_policy<boost::python::manage_new_object>(), "Returns the sequence file object. \n Input: script name \n Output: script object");
	

	// resource access
	boost::python::def("showCommandResources", showCommandResources, "Returns the command resources. ");
	boost::python::def("showMotionResources", showMotionResources, "Returns the motion resources. ");
	boost::python::def("showSkeletonResources", showSkeletonResources, "Returns the motion resources. ");
	boost::python::def("showPathResources", showPathResources, "Returns the path resources. ");
	boost::python::def("showScriptResources", showScriptResources, "Returns the seq file resources. ");
	boost::python::def("showControllerResources", showControllerResources, "Returns the controller resources. ");
	boost::python::def("getResourceLimit", getResourceLimit, "Returns resource up limit. \n Input: NULL \n Output: resource display up limit");
	boost::python::def("setResourceLimit", setResourceLimit, "Set resource up limit. \n Input: resource display up limit \n Output: NULL");	


	// system
	boost::python::def("pythonexit", pythonExit, "Exits the Python interpreter. ");
	boost::python::def("reset", reset, "Reset SBM. ");
	boost::python::def("quit", quitSbm, "Quit SBM. ");	
	boost::python::def("getScene", SBScene::getScene, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the SmartBody scene object.");



	// class interface
/*	boost::python::class_<Script>("Script")
		.def("printInfo", &Script::print, "Print the content inside this script, this only works for seq script. ")
		.def("run", &Script::run, "Run the script immediately. ")
		.def("abort", &Script::abort, "Abort this running script, this only works for seq script. ")
		;
*/

	
boost::python::class_<SBSubject>("SBSubject")
		.def("notifyObservers", &SBSubject::notifyObservers,"Notifies all observers of this subject.")
		.def("registerObserver", &SBSubject::registerObserver,"Registers an observer to this subject.")
		.def("unregisterObserver", &SBSubject::unregisterObserver,"Unregisters an observer from this subject.")
		;

boost::python::class_<SBObserver>("SBObserver")
		.def("addDependency", &SBObserver::addDependency,"Adds a dependency on a subject.")
		.def("removeDependency", &SBObserver::removeDependency,"Removes a dependency on a subject.")
		.def("notify", &SBObserver::notify, "Notifies the observer of the subject.")
		;

boost::python::class_<SBAttributeInfo>("SBAttributeInfo")
		.def("getPriority", &SBAttributeInfo::getPriority, "Returns the priority of the attribute. Used for display purposes.")
		.def("setPriority", &SBAttributeInfo::setPriority, "Sets the priority of the attribute. Used for display purposes.")
		.def("getReadOnly", &SBAttributeInfo::getReadOnly, "Determines if the attribute is read-only and cannot be changed.")
		.def("setReadOnly", &SBAttributeInfo::setReadOnly, "Sets the read-only status of the attribute. Attributes marked read-only cannot have their value's changed.")
		.def("getHidden", &SBAttributeInfo::getHidden, "Determines if the attribute is hidden from view.")
		.def("setHidden", &SBAttributeInfo::setHidden, "Sets the hidden status of the attribute. Hidden attributes typically aren't visible to the user.")
		.def("setDescription", &SBAttributeInfo::setDescription, "Sets the description or help text associated with this attribute.")
		.def("getDescription", &SBAttributeInfo::getDescription, "Gets the description or help text associated with this attribute.")
	;

boost::python::class_<SBAttribute, boost::python::bases<SBSubject> >("SBAttribute")
		.def("getName", &SBAttribute::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns an attribute of a given name")
		.def("getAttributeInfo", &SBAttribute::getAttributeInfo, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the information associated with this attribute.")
	;

	boost::python::class_<ActionAttribute, boost::python::bases<SBAttribute> >("ActionAttribute")
		.def("setValue", &ActionAttribute::setValue, "Activates action attribute.")
	;

	boost::python::class_<BoolAttribute, boost::python::bases<SBAttribute> >("BoolAttribute")
		.def("getValue", &BoolAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the bool attribute.")
		.def("setValue", &BoolAttribute::setValue, "Sets the value of the boolean attribute.")
		.def("setValueFast", &BoolAttribute::setValueFast, "Sets the value of the boolean attribute without notifying observers.")
	;

	boost::python::class_<StringAttribute, boost::python::bases<SBAttribute> >("StringAttribute")
		.def("getValue", &StringAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the string attribute.")
		.def("setValue", &StringAttribute::setValue, "Sets the value of the string attribute.")
		.def("setValueFast", &StringAttribute::setValueFast, "Sets the value of the string attribute without notifying observers.")
		.def("setValidValues", &StringAttribute::setValidValues, "Sets the valid values of the string attribute.")
		.def("getValidValues", &StringAttribute::getValidValues, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the valid values of the string attribute.")
	;

	boost::python::class_<IntAttribute, boost::python::bases<SBAttribute> >("IntAttribute")
		.def("getValue", &IntAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the int attribute.")
		.def("setValue", &IntAttribute::setValue, "Sets the value of the integer attribute.")
		.def("setValueFast", &IntAttribute::setValueFast, "Sets the value of the integer attribute without notifying observers.")
	;

	boost::python::class_<DoubleAttribute, boost::python::bases<SBAttribute> >("DoubleAttribute")
		.def("getValue", &DoubleAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the double attribute.")
		.def("setValue", &DoubleAttribute::setValue, "Sets the value of the double attribute.")
		.def("setValueFast", &DoubleAttribute::setValueFast, "Sets the value of the double attribute without notifying observers.")
	;

	boost::python::class_<Vec3Attribute, boost::python::bases<SBAttribute> >("Vec3Attribute")
		.def("getValue", &Vec3Attribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the vec3 attribute.")
		.def("setValue", &Vec3Attribute::setValue, "Sets the value of the vec3 attribute.")
		.def("setValueFast", &Vec3Attribute::setValueFast, "Sets the value of the vec3 attribute without notifying observers.")
	;

	boost::python::class_<MatrixAttribute, boost::python::bases<SBAttribute> >("MatrixAttribute")
		.def("getValue", &MatrixAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the matrix attribute.")
		.def("setValue", &MatrixAttribute::setValue, "Sets the value of the matrix attribute.")
		.def("setValueFast", &MatrixAttribute::setValueFast, "Sets the value of the matrix attribute.")
	;


	boost::python::class_<SBObject>("SBObject")
		.def("getName", &SBObject::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the object.")
		.def("getNumAttributes", &SBObject::getNumAttributes,  "Returns the number of attributes associated with this object.")
		.def("getAttributeNames", &SBObject::getAttributeNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the attributes names associated with this object.")
		.def("getAttribute", &SBObject::getAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns an attribute of a given name")
		.def("clearAttributes", &SBObject::clearAttributes, "Clear all the attributes associated with this object.")
		.def("createBoolAttribute", &SBObject::createBoolAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a boolean attribute.")
		.def("createIntAttribute", &SBObject::createIntAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates an integer attribute.")
		.def("createDoubleAttribute", &SBObject::createDoubleAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a double attribute.")
		.def("createStringAttribute", &SBObject::createStringAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a string attribute.")
		.def("createActionAttribute", &SBObject::createActionAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a action attribute.")
		.def("setBoolAttribute", &SBObject::setBoolAttribute, "Sets a boolean attribute of a given name to the given value.")
		.def("setIntAttribute", &SBObject::setIntAttribute, "Sets an integer attribute of a given name to the given value.")
		.def("setDoubleAttribute", &SBObject::setDoubleAttribute, "Sets a floating point attribute of a given name to the given value.")
		.def("setStringAttribute", &SBObject::setStringAttribute, "Sets a string attribute of a given name to the given value.")
		.def("setVec3Attribute", &SBObject::setVec3Attribute, "Sets a vector attribute of a given name to the given value.")
		.def("setMatrixAttribute", &SBObject::setMatrixAttribute, "Sets a matrix attribute of a given name to the given value.")
		.def("setActionAttribute", &SBObject::setActionAttribute, "Sets a action attribute of a given name.")
		;


	boost::python::class_<SBService, boost::python::bases<SBObject> >("SBService")
		.def("setEnable", &SBService::setEnable, "Enables or disables the service.")
		.def("isEnable", &SBService::isEnable, "Is the service enabled?")
		;

	boost::python::class_<SBServiceManager>("SBServiceManager")
		.def("addService", &SBServiceManager::addService, "Adds a service to the service manager.")
		.def("removeService", &SBServiceManager::removeService, "Removes a service to the service manager.")
		.def("getServiceNames", &SBServiceManager::getServiceNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of services available.")
		.def("getNumServices", &SBServiceManager::getNumServices, "Returns the number of services present.")
		.def("getService", &SBServiceManager::getService, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a service by name.")
		;


	boost::python::class_<SBSimulationManager>("SBSimulationManager")
		.def("isRunning", &SBSimulationManager::isRunning, "Returns true if the simulation is currently running.")
		.def("isStarted", &SBSimulationManager::isStarted, "Returns true if the simulation has been started.")
		.def("printInfo", &SBSimulationManager::printInfo, "Print all the timing statistics. ")
		.def("printPerf", &SBSimulationManager::printPerf, "Print performance statistics calculated real time given a time period as input.")
		.def("getTime", &SBSimulationManager::getTime, "Returns the current simulation time.")
		.def("setTime", &SBSimulationManager::setTime, "Sets the current simulation time.")
		.def("start", &SBSimulationManager::start, "Start the simulation.")
		.def("stop", &SBSimulationManager::stop, "Stop the simulation.")
		.def("reset", &SBSimulationManager::reset, "Set the clock time to 0. ")
		.def("pause", &SBSimulationManager::pause, "Pause the clock. ")
		.def("resume", &SBSimulationManager::resume, "Resume the clock. ")
		.def("step", &SBSimulationManager::step, "Running the system in the step mode, user can input how many steps they want to run. \n Input: number of steps at a time \n Output: NULL")
		.def("setSleepFps", &SBSimulationManager::setSleepFps, "Set the sleep fps. Sleep fps defines the target loop rate. \n Input: sleep fps \n Output: NULL")
		.def("setEvalFps", &SBSimulationManager::setEvalFps, "Set the eval fps. Define the minimum interval to evaluate the frame. \n Input: evaluation fps \n Output: NULL")
		.def("setSimFps", &SBSimulationManager::setSimFps, "Set the simulation fps. Add a fixed increment to output time every update. \n Input: simulation fps \n Output: NULL")
		.def("setSleepDt", &SBSimulationManager::setSleepDt, "Set the sleep dt. \n Input: sleep dt \n Output: NULL")
		.def("setEvalDt", &SBSimulationManager::setEvalDt, "Set the eval dt. \n Input: evaluation dt \n Output: NULL")
		.def("setSimDt", &SBSimulationManager::setSimDt, "Set the sim dt. \n Input: simulation dt \n Output: NULL")
		.def("setSpeed", &SBSimulationManager::setSpeed, "Set the speed for real clock time. Actual time would be real time times speed.")
		;

	boost::python::class_<SBProfiler>("Profiler")
		.def("printLegend", &SBProfiler::printLegend, "Print time profiler legend. ")
		.def("printStats", &SBProfiler::printStats, "Print time profiler statistics. ")
		;

	boost::python::class_<Camera>("Camera")
		.def("printInfo", &Camera::printInfo, "Prints all the camera statistics. ")
		.def("reset", &Camera::reset, "Reset camera with camera eye (0 166 185), camera center (0 92 0). ")
		.def("setEye", &Camera::setEye, "Set camera eye position. \n Input: camera eye position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("setCenter", &Camera::setCenter, "Set camera center. \n Input: camera center position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("setScale", &Camera::setScale, "Set camera scale. \n camera scale: NULL \n Output: NULL")
		.def("setTrack", &Camera::setTrack, "Set camera track. \n Input: character name, joint name \n Output: NULL")
		.def("removeTrack", &Camera::removeTrack, "Remove camera track. ")
		.def("loadCamera", &Camera::loadCamera, "load Camera from file \n Input: camera file (*.cam) \n Output: NULL ")
		.def("saveCamera", &Camera::saveCamera, "save Camera to file \n Input: camera file (*.cam) \n Output: NULL ")
		;

	boost::python::class_<SrViewer>("Viewer")
		.def("show", &SrViewer::show_viewer, "Shows the viewer.")
		.def("hide", &SrViewer::hide_viewer, "Hides the viewer.")
		;

	boost::python::class_<GenericViewer>("GenericViewer")
		.def("show", &GenericViewer::show_viewer, "Shows the viewer.")
		.def("hide", &GenericViewer::hide_viewer, "Hides the viewer.")
		;

}
}


#endif
