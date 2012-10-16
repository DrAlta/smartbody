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
#include <sr/sr_camera.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifdef USE_PYTHON

struct NvbgWrap :  Nvbg, boost::python::wrapper<Nvbg>
{
	virtual void objectEvent(std::string character, std::string name, bool isAnimate, SrVec charPosition, SrVec charVelocity, SrVec objPosition, SrVec objVelocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		if (boost::python::override o = this->get_override("objectEvent"))
		{
			try {
				o(character, name, isAnimate, charPosition, charVelocity, objPosition, objVelocity, relativePosition, relativeVelocity);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_objectEvent(std::string character, std::string name, bool isAnimate, SrVec charPosition, SrVec charVelocity, SrVec objPosition, SrVec objVelocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		return Nvbg::objectEvent(character, name, isAnimate, charPosition, charVelocity, objPosition, objVelocity, relativePosition, relativeVelocity);
	}

	virtual bool execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		if (boost::python::override o = this->get_override("execute"))
		{
			try {
				return o(character, to, messageId, xml);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::execute(character, to, messageId, xml);
	}

	bool default_execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		return Nvbg::execute(character, to, messageId, xml);
	}

	virtual bool executeEvent(std::string character, std::string messageId, std::string state)
	{
		if (boost::python::override o = this->get_override("executeEvent"))
		{
			try {
				return o(character, messageId, state);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::executeEvent(character, messageId, state);
	}

	bool default_executeEvent(std::string character, std::string messageId, std::string state)
	{
		return Nvbg::executeEvent(character, messageId, state);
	}

	virtual bool executeSpeech(std::string character, std::string speechStatus, std::string speechId, std::string speaker)
	{
		if (boost::python::override o = this->get_override("executeSpeech"))
		{
			try {
				return o(character, speechStatus, speechId, speaker);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::executeSpeech(character, speechStatus, speechId, speaker);
	}


	bool default_executeSpeech(std::string character, std::string speechStatus, std::string speechId, std::string speaker)
	{
		return Nvbg::executeSpeech(character, speechStatus, speechId, speaker);
	}

	virtual void notifyAction(std::string name)
	{
		if (boost::python::override o = this->get_override("notifyAction"))
		{
			try {
				o(name);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyAction(std::string name)
	{
		notifyLocal(name);
	}

	virtual void notifyBool(std::string name, bool val)
	{
		if (boost::python::override o = this->get_override("notifyBool"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyBool(std::string name, bool val)
	{
		notifyLocal(name);
	}

	virtual void notifyInt(std::string name, int val)
	{
		if (boost::python::override o = this->get_override("notifyInt"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyInt(std::string name, int val)
	{
		notifyLocal(name);
	}

	virtual void notifyDouble(std::string name, double val)
	{
		if (boost::python::override o = this->get_override("notifyDouble"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyDouble(std::string name, double val)
	{
		notifyLocal(name);
	}

	virtual void notifyString(std::string name, std::string val)
	{
		if (boost::python::override o = this->get_override("notifyString"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyString(std::string name, std::string val)
	{
		notifyLocal(name);
	}

	virtual void notifyVec3(std::string name, SrVec val)
	{
		if (boost::python::override o = this->get_override("notifyVec3"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyVec3(std::string name, SrVec val)
	{
		notifyLocal(name);
	}

	virtual void notifyMatrix(std::string name, SrMat val)
	{
		if (boost::python::override o = this->get_override("notifyMatrix"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyMatrix(std::string name, SrMat val)
	{
		notifyLocal(name);
	}


};


struct SBScriptWrap :  SBScript, boost::python::wrapper<SBScript>
{
	virtual void start()
	{
		if (boost::python::override o = this->get_override("start"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_start()
	{
		return SBScript::start();
	}

	virtual void stop()
	{
		if (boost::python::override o = this->get_override("stop"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_stop()
	{
		return SBScript::stop();
	}

	virtual void beforeUpdate(double time)
	{
		if (boost::python::override o = this->get_override("beforeUpdate"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_beforeUpdate(double time)
	{
		return SBScript::beforeUpdate(time);
	}

	virtual void update(double time)
	{
		if (boost::python::override o = this->get_override("update"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_update(double time)
	{
		return SBScript::update(time);
	}

	virtual void afterUpdate(double time)
	{
		if (boost::python::override o = this->get_override("afterUpdate"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_afterUpdate(double time)
	{
		return SBScript::afterUpdate(time);
	}
};


struct EventHandlerWrap :  EventHandler, boost::python::wrapper<EventHandler>
{
	virtual void executeAction(Event* event)
	{
		if (boost::python::override o = this->get_override("executeAction"))
		{
			try {
				o(event);
			} catch (...) {
				PyErr_Print();
			}
		}

		return EventHandler::executeAction(event);
	}

	void default_executeAction(Event* event)
	{
		EventHandler::executeAction(event);
	}
};


struct PythonControllerWrap : SmartBody::PythonController, boost::python::wrapper<SmartBody::PythonController>
{
	virtual void start()
	{
		if (boost::python::override o = this->get_override("start"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
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
				PyErr_Print();
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
				PyErr_Print();
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
				PyErr_Print();
			}
		}

		return PythonController::stop();
	};

	void default_stop()
	{
		SmartBody::PythonController::stop();
	}
};

// wrapper for std::map
template<class T>
struct map_item
{
	typedef typename T::key_type K;
	typedef typename T::mapped_type V;
	static V get(T const& x, K const& i)
	{
		V temp;
		if( x.find(i) != x.end() ) 
			return x.find(i)->second;
		PyErr_SetString(PyExc_KeyError, "Key not found");
		return temp;		
	}
	static void set(T & x, K const& i, V const& v)
	{
		x[i]=v; // use map autocreation feature
	}
	static void del(T & x, K const& i)
	{
		if( x.find(i) != x.end() ) x.erase(i);
		else PyErr_SetString(PyExc_KeyError, "Key not found");
	}
};

#endif



#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif

typedef std::map<std::string,SrQuat> QuatMap;
typedef std::map<std::string,SrVec> VecMap;
typedef std::map<std::string, std::string> StringMap;


namespace SmartBody 
{
#ifdef USE_PYTHON
BOOST_PYTHON_MODULE(SmartBody)
{
	boost::python::def("printlog", printLog, "Write to the log. \n Input: message string \n Output: NULL");
#if 1
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

	

	//boost::python::numeric::array::set_module_and_type("numpy", "ndarray");
	

	// characters

//#ifndef __ANDROID__

	

	boost::python::def("createController", createController, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a new controller given a controller type and a controller name.");

	

	

	//#endif

#if 0
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
		.def("hasDependency", &SBObserver::hasDependency, "Returns True if there is a dependency on the given subject.")
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
		.def("getDefaultValue", &BoolAttribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the bool attribute.")
		.def("setDefaultValue", &BoolAttribute::setDefaultValue, "Sets the default value of the boolean attribute.")
		.def("setValueFast", &BoolAttribute::setValueFast, "Sets the value of the boolean attribute without notifying observers.")
	;

	boost::python::class_<StringAttribute, boost::python::bases<SBAttribute> >("StringAttribute")
		.def("getValue", &StringAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the string attribute.")
		.def("setValue", &StringAttribute::setValue, "Sets the value of the string attribute.")
		.def("getDefaultValue", &StringAttribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the string attribute.")
		.def("setDefaultValue", &StringAttribute::setDefaultValue, "Sets the default value of the string attribute.")
		.def("setValueFast", &StringAttribute::setValueFast, "Sets the value of the string attribute without notifying observers.")
		.def("setValidValues", &StringAttribute::setValidValues, "Sets the valid values of the string attribute.")
		.def("getValidValues", &StringAttribute::getValidValues, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the valid values of the string attribute.")
	;

	boost::python::class_<IntAttribute, boost::python::bases<SBAttribute> >("IntAttribute")
		.def("getValue", &IntAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the int attribute.")
		.def("setValue", &IntAttribute::setValue, "Sets the value of the integer attribute.")
		.def("getDefaultValue", &IntAttribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the string attribute.")
		.def("setDefaultValue", &IntAttribute::setDefaultValue, "Sets the default value of the string attribute.")
		.def("setValueFast", &IntAttribute::setValueFast, "Sets the value of the integer attribute without notifying observers.")
	;

	boost::python::class_<DoubleAttribute, boost::python::bases<SBAttribute> >("DoubleAttribute")
		.def("getValue", &DoubleAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the double attribute.")
		.def("setValue", &DoubleAttribute::setValue, "Sets the value of the double attribute.")
		.def("getDefaultValue", &DoubleAttribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the double attribute.")
		.def("setDefaultValue", &DoubleAttribute::setDefaultValue, "Sets the default value of the double attribute.")
		.def("setValueFast", &DoubleAttribute::setValueFast, "Sets the value of the double attribute without notifying observers.")
	;

	boost::python::class_<Vec3Attribute, boost::python::bases<SBAttribute> >("Vec3Attribute")
		.def("getValue", &Vec3Attribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the vec3 attribute.")
		.def("setValue", &Vec3Attribute::setValue, "Sets the value of the vec3 attribute.")
		.def("getDefaultValue", &Vec3Attribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the vec3 attribute.")
		.def("setDefaultValue", &Vec3Attribute::setDefaultValue, "Sets the default value of the vec3 attribute.")
		.def("setValueFast", &Vec3Attribute::setValueFast, "Sets the value of the vec3 attribute without notifying observers.")
	;

	boost::python::class_<MatrixAttribute, boost::python::bases<SBAttribute> >("MatrixAttribute")
		.def("getValue", &MatrixAttribute::getValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the value of the matrix attribute.")
		.def("setValue", &MatrixAttribute::setValue, "Sets the value of the matrix attribute.")
		.def("getDefaultValue", &MatrixAttribute::getDefaultValue, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the default value of the matrix attribute.")
		.def("setDefaultValue", &MatrixAttribute::setDefaultValue, "Sets the default value of the matrix attribute.")
		.def("setValueFast", &MatrixAttribute::setValueFast, "Sets the value of the matrix attribute.")
	;


	boost::python::class_<SBObject>("SBObject")
		.def("getName", &SBObject::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the object.")
		.def("getNumAttributes", &SBObject::getNumAttributes,  "Returns the number of attributes associated with this object.")
		.def("getAttributeNames", &SBObject::getAttributeNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the attributes names associated with this object.")
		.def("getAttribute", &SBObject::getAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns an attribute of a given name")
		.def("clearAttributes", &SBObject::clearAttributes, "Clear all the attributes associated with this object.")
		.def("createBoolAttribute", &SBObject::createBoolAttribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a boolean attribute.")
		.def("createVec3Attribute", &SBObject::createVec3Attribute, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a vec3 attribute.")
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

	boost::python::class_<SrCamera>("Camera")
		.def("print", &SrCamera::print, "Shows all the camera statistics. ")
		.def("reset", &SrCamera::reset, "Reset camera with camera eye (0 166 185), camera center (0 92 0). ")
		.def("setEye", &SrCamera::setEye, "Set camera eye position. \n Input: camera eye position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("getEye", &SrCamera::getEye, "Get camera eye position.")
		.def("setCenter", &SrCamera::setCenter, "Set camera center. \n Input: camera center position(should only have three number in the input list) e.g. [0, 0, 0] \n Output: NULL")
		.def("getCenter", &SrCamera::getCenter, "Get camera center.")
		.def("setScale", &SrCamera::setScale, "Set camera scale. \n camera scale: NULL \n Output: NULL")
		.def("getScale", &SrCamera::getScale, "Get camera scale.")
		.def("setTrack", &SrCamera::setTrack, "Set camera track. \n Input: character name, joint name \n Output: NULL")
		.def("removeTrack", &SrCamera::removeTrack, "Remove camera track.")
		.def("setUpVector", &SrCamera::setUpVector, "Set camera up vector.")
		.def("getUpVector", &SrCamera::getUpVector, "Returns the camera up vector.")
		.def("setFov", &SrCamera::setFov, "Set's the camera's field of view.")
		.def("getFov", &SrCamera::getFov, "Get's the camera's field of view.")
		.def("setNearPlane", &SrCamera::setNearPlane, "Set's the camera's near plane.")
		.def("getNearPlane", &SrCamera::getNearPlane, "Get's the camera's near plane.")
		.def("setFarPlane", &SrCamera::setFarPlane, "Set's the camera's far plane.")
		.def("getFarPlane", &SrCamera::getFarPlane, "Get's the camera's far plane.")
		.def("setAspectRatio", &SrCamera::setAspectRatio, "Set's the camera's aspect ratio.")
		.def("getAspectRatio", &SrCamera::getAspectRatio, "Get's the camera's aspect ratio.")
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
		.def("execBML", &SBBmlProcessor::execBML, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute a generic BML instruction to a given character. Adds the <?xml..> and <act><bml>...</bml></act> elements.")
		.def("execBMLFile", &SBBmlProcessor::execBMLFile, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute the BML instructions contained in a file for a given character.")
		.def("execXML", &SBBmlProcessor::execXML, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute a generic XML instruction to a given character. Adds the <?xml..> header.")
		;

	boost::python::class_<SBAnimationBlend>("SBAnimationBlend")
		.def("addCorrespondencePoints", &SBAnimationBlend::addCorrespondencePoints, "Correspondence points for motions inside the blend.")
		.def("addCorrespondancePoints", &SBAnimationBlend::addCorrespondencePoints, "Correspondence points for motions inside the blend.")
		.def("setIncrementWorldOffsetY", &SBAnimationBlend::setIncrementWorldOffsetY, "Boolean flag that increment world offset y-axis value according to the base joint value.")
		.def("getNumMotions", &SBAnimationBlend::getNumMotions, "Number of motions inside the blend.")
		.def("getMotion", &SBAnimationBlend::getMotion, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the motion name given index. \n Input: index of motion \n Output: motion name")
		.def("getNumCorrespondancePoints", &SBAnimationBlend::getNumCorrespondencePoints, "Number of correspondence points for the motions in the blend")
		.def("getNumCorrespondencePoints", &SBAnimationBlend::getNumCorrespondencePoints, "Number of correspondence points for the motions in the blend")
		.def("getCorrespondancePoints", &SBAnimationBlend::getCorrespondencePoints, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the correspondence points in one motion given the index. \n Input: index of motion \n Output: correspondence points vector of this motion")
		.def("getCorrespondencePoints", &SBAnimationBlend::getCorrespondencePoints, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the correspondence points in one motion given the index. \n Input: index of motion \n Output: correspondence points vector of this motion")
		.def("setCorrespondencePoints", &SBAnimationBlend::setCorrespondencePoints, "Sets the correspondence points given a motion index, a parameter index and a value.")
		.def("getDimension", &SBAnimationBlend::getDimension, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the dimension of the state. Dimension represents the number of parameter for each motion. 0D means no parameter, 1D means one parameter for each motion etc.")
		.def("addEvent", &SBAnimationBlend::addEvent, "Adds an event to the blend at a specific local time for the given motion.")
		.def("removeAllEvents", &SBAnimationBlend::removeAllEvents, "Removes all events from the blend at a specific local time for the given motion.")
		.def("getNumEvents", &SBAnimationBlend::getNumEvents, "Returns the number of events associated with this blend.")
		.def("getEvent", &SBAnimationBlend::getEvent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the event of a given index.")
		.def("removeEvent", &SBAnimationBlend::removeEvent, "Removes the event of a given index.") 
		.def("buildVisSurfaces", &SBAnimationBlend::buildVisSurfaces, "Build a visualization surface. \n Input : Error Type, Surface Type, Num of Segements, Grid Resolutions \n Output: NULL")
		.def("createMotionVectorFlow", &SBAnimationBlend::createMotionVectorFlow, "create Vector Flow visualization. \n Input: motion name. \n Output: NULL")
		.def("clearMotionVectorFlow", &SBAnimationBlend::clearMotionVectorFlow, "clear Vector Flow visualization. \n Input: NULL. \n Output: NULL")
		.def("plotMotion", &SBAnimationBlend::plotMotion, "Plot motion frames with stick skeleton. \n Input: motion name, intervals, ifClearAll \n Output: NULL")
		.def("plotMotionFrameTime", &SBAnimationBlend::plotMotionFrameTime, "Plot one single motion frame (at given time) with stick skeleton. \n Input: motion name, time, ifClearAll \n Output: NULL")
		.def("plotMotionJointTrajectory", &SBAnimationBlend::plotMotionJointTrajectory, "Plot joint trajectory over entire motion (at given time). \n Input: motion name, jointName, ifClearAll \n Output: NULL")
		.def("clearPlotMotion", &SBAnimationBlend::clearPlotMotion, "clear Plotted motions. \n Input: NULL. \n Output: NULL")
		;

	boost::python::class_<SBAnimationBlend0D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend0D")
		.def("addMotion", &SBAnimationBlend0D::addMotion, "Add motion to 0D state. \n Input: motion name. \n Output: NULL")
	;

	boost::python::class_<SBAnimationBlend1D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend1D")
		.def("addMotion", &SBAnimationBlend1D::addMotion, "Add motion and one parameter to 1D state. \n Input: motion name, parameter. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend1D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter. \n Output: NULL")
	;

	boost::python::class_<SBAnimationBlend2D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend2D")
		.def("addMotion", &SBAnimationBlend2D::addMotion, "Add motion and two parameters to 2D state. \n Input: motion name, parameter1, parameter2. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend2D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter1, parameter2. \n Output: NULL")
		.def("addTriangle", &SBAnimationBlend2D::addTriangle, "Add triangles to the state. By changing the point inside triangle, you can get different blending weights and different results")
	;

	boost::python::class_<SBAnimationBlend3D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend3D")
		.def("addMotion", &SBAnimationBlend3D::addMotion, "Add motion and three parameters to 3D state. \n Input: motion name, parameter1, parameter2, parameter3. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend3D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter1, parameter2, parameter3. \n Output: NULL")
		.def("addTetrahedron", &SBAnimationBlend3D::addTetrahedron, "Add tetrahedrons to the state. By changing the point inside tetrahedron, you can get different blending weights and different results")
	;

	boost::python::class_<SBMotionBlendBase, boost::python::bases<SBAnimationBlend> >("SBMotionBlendBase")
		.def("addMotion", &SBMotionBlendBase::addMotion, "Add motion and its parameters to animation state. \n Input: motion name, vector of parameters. \n Output: NULL")
		.def("setParameter", &SBMotionBlendBase::setMotionParameter, "Set/Change the parameter for given motion. \n Input: motion name, vector of parameters. \n Output: NULL")
		.def("getParameter", &SBMotionBlendBase::getMotionParameter, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the parameter of a given motion")
		.def("buildBlendBase", &SBMotionBlendBase::buildBlendBase, "Initialize BlendBase. \n Input : Motion Parameter Name, Interpolator Type \n Output: NULL")
		.def("addTetrahedron", &SBMotionBlendBase::addTetrahedron, "Add tetrahedrons to the state. By changing the point inside tetrahedron, you can get different blending weights and different results")
		;


	boost::python::class_<SBAnimationTransition>("SBAnimationTransition")
		.def("set", &SBAnimationTransition::set, "")
		.def("setSourceState", &SBAnimationTransition::setSourceBlend, "")
		.def("getSourceState", &SBAnimationTransition::getSourceBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setSourceBlend", &SBAnimationTransition::setSourceBlend, "")
		.def("getSourceBlend", &SBAnimationTransition::getSourceBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setDestinationState", &SBAnimationTransition::setDestinationBlend, "")
		.def("getDestinationState", &SBAnimationTransition::getDestinationBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setDestinationBlend", &SBAnimationTransition::setDestinationBlend, "")
		.def("getDestinationBlend", &SBAnimationTransition::getDestinationBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("getSourceMotionName", &SBAnimationTransition::getSourceMotionName, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("setEaseInInterval", &SBAnimationTransition::setEaseInInterval, "")
		.def("addEaseOutInterval", &SBAnimationTransition::addEaseOutInterval, "")
		.def("removeEaseOutInterval", &SBAnimationTransition::removeEaseOutInterval, "")
		.def("getNumEaseOutIntervals", &SBAnimationTransition::getNumEaseOutIntervals, "")
		.def("getEaseOutInterval", &SBAnimationTransition::getEaseOutInterval, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getDestinationMotionName", &SBAnimationTransition::getDestinationMotionName, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getEaseInStart", &SBAnimationTransition::getEaseInStart, "")
		.def("getEaseInEnd", &SBAnimationTransition::getEaseInEnd, "")
		;

	boost::python::class_<SBAnimationBlendManager>("SBAnimationBlendManager")
		.def("createState0D", &SBAnimationBlendManager::createBlend0D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createState1D", &SBAnimationBlendManager::createBlend1D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createState2D", &SBAnimationBlendManager::createBlend2D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 2D blend.")
		.def("createState3D", &SBAnimationBlendManager::createBlend3D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 3D blend.")
		.def("createBlend0D", &SBAnimationBlendManager::createBlend0D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createBlend1D", &SBAnimationBlendManager::createBlend1D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createBlend2D", &SBAnimationBlendManager::createBlend2D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 2D blend.")
		.def("createBlend3D", &SBAnimationBlendManager::createBlend3D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 3D blend.")
		.def("createMotionBlendBase", &SBAnimationBlendManager::createMotionBlendBase, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a motion blend base.")
		.def("createTransition", &SBAnimationBlendManager::createTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a transition.")
		.def("getState", &SBAnimationBlendManager::getBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a blend of a given name.")
		.def("getBlend", &SBAnimationBlendManager::getBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a blend of a given name.")
		.def("getNumStates", &SBAnimationBlendManager::getNumBlends, "Returns the number of states.")
		.def("getNumBlends", &SBAnimationBlendManager::getNumBlends, "Returns the number of states.")
		.def("getStateNames", &SBAnimationBlendManager::getBlendNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getBlendNames", &SBAnimationBlendManager::getBlendNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getTransition", &SBAnimationBlendManager::getTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a transition with a given name.")
		.def("getTransitionByIndex", &SBAnimationBlendManager::getTransitionByIndex, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a transition with a given name.")
		.def("getNumTransitions", &SBAnimationBlendManager::getNumTransitions, "Returns the state names.")
		.def("getTransitionNames", &SBAnimationBlendManager::getTransitionNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getCurrentState", &SBAnimationBlendManager::getCurrentBlend, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentStateParameters", &SBAnimationBlendManager::getCurrentBlendParameters, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentBlend", &SBAnimationBlendManager::getCurrentBlend, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentBlendParameters", &SBAnimationBlendManager::getCurrentBlendParameters, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("isStateScheduled", &SBAnimationBlendManager::isBlendScheduled, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns whether the character has the blend scheduled. Used to avoid scheduling the same blend.")
		.def("isBlendScheduled", &SBAnimationBlendManager::isBlendScheduled, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns whether the character has the blend scheduled. Used to avoid scheduling the same blend.")
		;

	boost::python::class_<SBSteerManager, boost::python::bases<SBService> >("SBSteerManager")
		.def("createSteerAgent", &SBSteerManager::createSteerAgent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a steer agent.")
		.def("removeSteerAgent", &SBSteerManager::removeSteerAgent, "Remove a steer agent.")
		.def("getNumSteerAgents", &SBSteerManager::getNumSteerAgents, "Return number of steer agents.")
		.def("getSteerAgent", &SBSteerManager::getSteerAgent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return steer agent given its name.")
		.def("getSteerAgentNames", &SBSteerManager::getSteerAgentNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Return steer agent names.")
		.def("start", &SBSteerManager::start, "Start the steer simulation.")
		.def("stop", &SBSteerManager::stop, "Stop the steer simulation.")
		;

	boost::python::class_<SBCollisionManager, boost::python::bases<SBService> >("SBCollisionManager")
		.def("start", &SBCollisionManager::start, "Starts the collision manager.")
		.def("stop", &SBCollisionManager::stop, "Stops the collision manager.")
		;


	boost::python::class_<SBPhysicsManager, boost::python::bases<SBService> >("SBPhysicsManager")
		.def("createPhysicsCharacter", &SBPhysicsManager::createPhysicsCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a physics character.")
		.def("createPhysicsPawn", &SBPhysicsManager::createPhysicsPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a physics rigid body and attach it to the pawn.")
		.def("getPhysicsEngine", &SBPhysicsManager::getPhysicsSimulationEngine, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return the current physics engine")  
		.def("getPhysicsCharacter", &SBPhysicsManager::getPhysicsCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based character")  
		.def("getPhysicsJoint", &SBPhysicsManager::getPhysicsJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based joint")  
		.def("getJointObj", &SBPhysicsManager::getJointObj, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based body link") 
		.def("getPhysicsPawn", &SBPhysicsManager::getPhysicsPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a rigid body pawn")
		.def("start", &SBPhysicsManager::start, "Start the physics simulation.(not implemented yet)") 
		.def("stop", &SBPhysicsManager::stop, "Stop the physics simulation.(not implemented yet)")		
		;

	boost::python::class_<SBBoneBusManager, boost::python::bases<SBService> >("SBBoneBusManager")
		;

	boost::python::class_<SBSteerAgent>("SBSteerAgent")
		.def("setSteerStateNamePrefix", &SBSteerAgent::setSteerStateNamePrefix, "Set the animation state name prefix used for steering, only applies to steering type locomotion.")
		.def("getSteerStateNamePrefix", &SBSteerAgent::getSteerStateNamePrefix, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the animation state name prefix used for steering, only applies to steering type locomotion.")
		.def("setSteerType", &SBSteerAgent::setSteerType, "Sets the type of steering locomotion, can be one of the following: basic, example, procedural")
		.def("getSteerType", &SBSteerAgent::getSteerType, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the type of steering locomotion, is one of the following: basic, example, procedural")
		.def("getCurrentSBCharacter", &SBSteerAgent::getCurrentSBCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return SBCharacter that SBSteerAgent is attached to.")		
		;


	boost::python::class_<SBDiphoneManager>("SBDiphoneManager")
		.def("createDiphone", &SBDiphoneManager::createDiphone, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a diphone.")
		.def("getDiphones", &SBDiphoneManager::getDiphones, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get diphones given diphone set name.")
		.def("getDiphone", &SBDiphoneManager::getDiphone, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get diphone given from phoneme, to phoneme and diphone set name.")
		.def("getNumDiphoneMap", &SBDiphoneManager::getNumDiphoneMap, "Return number of diphone set.")
		.def("getNumDiphones", &SBDiphoneManager::getNumDiphones, "Return number of diphones given the diphone set name.")
		.def("getDiphoneMapNames", &SBDiphoneManager::getDiphoneMapNames, "Returns the names of all the diphone sets.")
		;

	boost::python::class_<SBDiphone>("SBDiphone")
		.def("addKey", &SBDiphone::addKey, "add key to the diphone.")
		.def("getKeys", &SBDiphone::getKeys, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return the keys given viseme name.")
		.def("getVisemeNames", &SBDiphone::getVisemeNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the viseme names.")
		.def("getNumVisemes", &SBDiphone::getNumVisemes, "Return the number of viseme.")
		.def("getFromPhonemeName", &SBDiphone::getFromPhonemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Return FROM phoneme name.")
		.def("getToPhonemeName", &SBDiphone::getToPhonemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Return TO phoneme name.")		
		;

		boost::python::class_<SBBehaviorSetManager>("SBBehaviorSetManager")
		.def("createBehaviorSet", &SBBehaviorSetManager::createBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a behavior set.")
		.def("getBehaviorSets", &SBBehaviorSetManager::getBehaviorSets, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets all the behavior sets.")
		.def("getBehaviorSet", &SBBehaviorSetManager::getBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets a behavior set with a given name.")
		.def("getNumBehaviorSets", &SBBehaviorSetManager::getNumBehaviorSets, "Returns the number of behavior sets.")
		.def("removeBehaviorSet", &SBBehaviorSetManager::removeBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Removes a behavior set with a given name.")
		.def("removeAllBehaviorSets", &SBBehaviorSetManager::removeAllBehaviorSets, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Removes all the behavior sets.")
		;

	boost::python::class_<SBBehaviorSet>("SBBehaviorSet")
		.def("setName", &SBBehaviorSet::setName, "Sets the name of the behavior set.")
		.def("getName", &SBBehaviorSet::getName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the name of the behavior set.")
		.def("setScript", &SBBehaviorSet::setScript, "Sets the name of the script to be run for this behavior set.")
		.def("getScript", &SBBehaviorSet::getScript, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the name of the script to be run for this behavior set.")
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
		.def("getName", &SBFaceDefinition::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns name of the face definition.")
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
		.def("save", &SBFaceDefinition::save, "Save face definition to a file.")
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
		.def("disconnect", &SBMotion::disconnect, "Disconnect current motion with current skeleton object. ")
		.def("mirror", &SBMotion::mirror, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Mirrors the motion.")
		.def("retarget", &SBMotion::retarget, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Retarget the motion to a different skeleton.")
		.def("footSkateCleanUp", &SBMotion::footSkateCleanUp, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Retarget the motion to a different skeleton.")
		.def("constrain", &SBMotion::constrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Constrain the retargeted motion to based on the source skeleton and motion.")
		.def("smoothCycle", &SBMotion::smoothCycle, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Build the smooth cycle the motion.")
		.def("alignToEnd", &SBMotion::alignToEnd, "Cut the first x number of frames and stitch them to the end. x is the input number")
		.def("alignToBegin", &SBMotion::alignToBegin, "Cut the last x number of frames and stitch them to the begin. x is the input number")
		.def("duplicateCycle", &SBMotion::duplicateCycle, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Duplicate motion by x amount of cycles.")
		.def("getJointSpeed", &SBMotion::getJointSpeed, "Get the accumulative joint speed. \n Input: SBJoint, start time, end time \n Output: joint speed(unit: same with the skeleton)")
		.def("getJointSpeedAxis", &SBMotion::getJointSpeedAxis, "Get the accumulative joint speed of a given axis: X, Y or Z. \n Input: SBJoint, axis, start time, end time \n Output: joint speed(unit: same with the skeleton)")
		.def("getJointAngularSpeed", &SBMotion::getJointAngularSpeed, "Get the joint accumulative angular speed. \n Input: SBJoint, start time, end time \n Output: joint angular speed(unit: degree/sec)")		
		.def("getJointAngularSpeedAxis", &SBMotion::getJointAngularSpeedAxis, "Get the joint accumulative angular speed of a given axis: X, Y or Z. \n Input: SBJoint, axis, start time, end time \n Output: joint angular speed(unit: degree/sec)")		
		.def("getJointTransition", &SBMotion::getJointTransition, "Get the joint transition vector. \n Input: SBJoint, start time, end time \n Output: joint transition vector containing x, y, z value (unit: same with the skeleton)")		
		.def("getJointPosition", &SBMotion::getJointPosition, "Get the joint position. \n Input: SBJoint, time \n Output: joint position containing x, y, z value (unit: same with the skeleton)")		
		.def("translate", &SBMotion::translate, "Translates the base joint name by x,y,z values.")		
		.def("rotate", &SBMotion::rotate, "Rotates the base joint name by x,y,z axis.")			
		.def("scale", &SBMotion::scale, "Scales all translations in skeleton by scale factor.")		
		.def("trim", &SBMotion::trim, "Trims the starting and ending frames in the motion.")	
		.def("saveToSkm", &SBMotion::saveToSkm, "Saves the file in .skm format to a given file name.")	
		.def("getTimeStart", &SBMotion::getTimeStart, "Returns the start time of the motion.")
		.def("getTimeReady", &SBMotion::getTimeReady, "Returns the ready time of the motion.")
		.def("getTimeStrokeStart", &SBMotion::getTimeStrokeStart, "Returns the stroke start time of the motion.")
		.def("getTimeStroke", &SBMotion::getTimeStroke, "Returns the stroke time of the motion.")
		.def("getTimeStrokeEnd", &SBMotion::getTimeStrokeEnd, "Returns the stroke end time of the motion.")
		.def("getTimeRelax", &SBMotion::getTimeRelax, "Returns the relax time of the motion.")
		.def("getTimeStop", &SBMotion::getTimeStop, "Returns the stop time of the motion.")	
		.def("getDuration", &SBMotion::getDuration, "Return the duration of the motion")
		.def("addEvent", &SBMotion::addEvent, "Adds an event associated with this motion that will be triggered at the given time. The last paramter determines if the event will be triggered only once, or every time the motion is looped.")
		.def("addMetaData", &SBMotion::addMetaData, "Add a tagged metadata as string to the motion.")
		.def("removeMetaData", &SBMotion::removeMetaData, "Remove a tagged metadata from the motion.")
		.def("getMetaDataString", &SBMotion::getMetaDataString, "Get the first metadata based on tag name")
		.def("getMetaDataDouble", &SBMotion::getMetaDataDouble, "Get the first metadata based on tag name")
		.def("getMetaDataTags", &SBMotion::getMetaDataTags, "Get all tag names in the metadata map.")
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

	boost::python::class_<SBSkeleton, boost::python::bases<SBObject> >("SBSkeleton")
	//	.def(boost::python::init<>())
		.def(boost::python::init<std::string>())
		.def("load", &SBSkeleton::load, "Loads the skeleton definition from the given skeleton name.")
		.def("save", &SBSkeleton::save, "Saves the skeleton definition with the given skeleton name.")
		.def("rescale", &SBSkeleton::rescale, "Adjust the skeleton size to scale ratio")
		.def("getName", &SBSkeleton::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the skeleton.")
		.def("getFileName", &SBSkeleton::getFileName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the original filename where the skeleton was loaded from.")
		.def("getNumJoints", &SBSkeleton::getNumJoints, "Returns the number of joints for this skeleton.")
		.def("getJointNames", &SBSkeleton::getJointNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the joint names for this skeleton.")
		.def("getJointByName", &SBSkeleton::getJointByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint of a given name.")
		.def("getJoint", &SBSkeleton::getJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint with a given index.")
		.def("getNumChannels", &SBSkeleton::getNumChannels, "Returns the number of the channels inside the skeleton.")
		.def("getChannelType", &SBSkeleton::getChannelType, "Returns the type of the channel of a given index.")
		.def("getChannelSize", &SBSkeleton::getChannelSize, "Returns the size of the channel given index.")
		.def("createSkelWithoutPreRot", &SBSkeleton::createSkelWithoutPreRot, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a new standard T-pose skel from source but without pre-rotations")	
		.def("orientJointsLocalAxesToWorld", &SBSkeleton::orientJointsLocalAxesToWorld, "Orient skeleton joints local axes to match world coordinate axes (Y-up Z-front)")			
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
		.def("getMass", &SBJoint::getMass, "Gets the mass of the joint.")
		.def("setMass", &SBJoint::setMass, "Sets the mass of the joint.")
		.def("getPrerotation", &SBJoint::getPrerotation, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the prerotation values for the joint.")
		.def("setPrerotation", &SBJoint::setPrerotation, "Sets the prerotation values for the joint.")
		.def("getPostrotation", &SBJoint::getPostrotation, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the postrotation values for the joint.")
		.def("setPostrotation", &SBJoint::setPostrotation, "Sets the postrotation values for the joint.")		;

	boost::python::class_<SBBehavior, boost::python::bases<SBObject> >("SBBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getType", &SBBehavior::getType, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the type of the behavior.")
		;

	boost::python::class_<GazeBehavior, boost::python::bases<SBBehavior> >("GazeBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getGazeTarget", &GazeBehavior::getGazeTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the gaze target.")
		.def("isFadingIn", &GazeBehavior::isFadingIn, "Returns true if the gaze controller is currently fading in.")
		.def("isFadingOut", &GazeBehavior::isFadingOut, "Returns true if the gaze controller is currently fading out.")
		.def("isFadedOut", &GazeBehavior::isFadedOut, "Returns true if the gaze controller is currently faded out and thus not active.")
		.def("getHandle", &GazeBehavior::getHandle, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the handle name of the gaze, or an empty string if it has no handle.")
;

	boost::python::class_<LocomotionBehavior, boost::python::bases<SBBehavior> >("LocomotionBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getLocomotionTarget", &LocomotionBehavior::getLocomotionTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the locomotion target as a vector.")
	;

	boost::python::class_<PostureBehavior, boost::python::bases<SBBehavior> >("PostureBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getPosture", &PostureBehavior::getPosture, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the current posture.")
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
		.def("getControllerNames", &SBCharacter::getControllerNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the controller name vector.")
		.def("setVoice", &SBCharacter::setVoice, "Sets the voice type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceCode", &SBCharacter::setVoiceCode, "Sets the voice code. For audiofile type, this is a path.")
		.def("setVoiceBackup", &SBCharacter::setVoiceBackup, "Sets the voice backup type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceBackupCode", &SBCharacter::setVoiceBackupCode, "Sets the voice backup code. For audiofile type, this is a path.")
		.def("getVoice", &SBCharacter::getVoice, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the voice of the character..")
		.def("getVoiceCode", &SBCharacter::getVoiceBackupCode, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the voice code. For audiofile type, this is a path.")
		.def("getVoiceBackup", &SBCharacter::getVoiceBackup, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the voice backup type: remote, audiofile, text or none (use \"\").")
		.def("setFaceDefinition", &SBCharacter::setFaceDefinition, "Sets face definition (visemes, action units) for a character.")
		.def("getFaceDefinition", &SBCharacter::getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets face definition (visemes, action units) for a character.")
		.def("getHeight", &SBCharacter::getHeight, "Gets the height of the character.")
		.def("getBoundingBox", &SBCharacter::getBoundingBox, "Gets the boundary dimensions of the character.")
		.def("getNumBehaviors", &SBCharacter::getNumBehaviors, "Returns the number of behaviors of the character.")
		.def("getBehavior", &SBCharacter::getBehavior, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the ith behavior of the character.")
		.def("setSteerAgent", &SBCharacter::setSteerAgent, "Set the steer agent of the character")
		//.def("getFaceDefinition", &SBCharacter::getFaceDefinition, "Gets face definition (visemes, action units) for a character.")
#ifndef __ANDROID__
		.def("setNvbg", &SBCharacter::setNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the NVBG handler for this character.")
		.def("getNvbg", &SBCharacter::getNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the NVBG handler for this character.")
		.def("setMiniBrain", &SBCharacter::setMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the mini brain handler for this character.")
		.def("getMiniBrain", &SBCharacter::getMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the mini brain handler for this character.")
#endif
		;

boost::python::class_<SBReach>("SBReach")
		.def("getCharacter", &SBReach::getCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character associated with this reach engine.")
		.def("copy", &SBReach::copy, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Copies the reach engine.")
		.def("addMotion", &SBReach::addMotion, "Adds a motion to the reach engine.")
		.def("removeMotion", &SBReach::removeMotion, "Removes a motion from the reach engine.")
		.def("getNumMotions", &SBReach::getNumMotions, "Returns the number of motions in the reach engine.")
		.def("getMotionNames", &SBReach::getMotionNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the names of the motions used in the reach engine.")
		.def("build", &SBReach::build, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Constructs the reach engine using the exisiting motions.")
		.def("setGrabHandMotion", &SBReach::setGrabHandMotion, "Sets the hand pose to be used during grasping.")
		.def("getGrabHandMotion", &SBReach::getGrabHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used during grasping.")
		.def("setReleaseHandMotion", &SBReach::setReleaseHandMotion,  "Sets the hand pose to be used when releasing an object.")
		.def("getReleaseHandMotion", &SBReach::getReleaseHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when releasing an object.")
		.def("setReachHandMotion", &SBReach::setReachHandMotion, "Sets the hand pose to be used when reaching for an object.")
		.def("getReachHandMotion", &SBReach::getReachHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when reaching for an object.")
		.def("setPointHandMotion", &SBReach::setPointHandMotion, "Sets the hand pose to be used when reaching for an object.")
		.def("getPointHandMotion", &SBReach::getPointHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when reaching for an object.")
		.def("setInterpolatorType", &SBReach::setInterpolatorType, "Set the interpolation type when building the reach engine")
		.def("getInterpolatorType", &SBReach::getInterpolatorType, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the interpolation type used building the reach engine")
		.def("isPawnAttached", &SBReach::isPawnAttached, boost::python::return_value_policy<boost::python::return_by_value>(), "Return True is the pawn is currently attached to the character's hand.")
		;

	boost::python::class_<SBReachManager>("SBReachManager")
		.def("createReach", &SBReachManager::createReach, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a reach engine for a character.")
		.def("removeReach", &SBReachManager::removeReach, "Removes a reach engine for a character")
		.def("getNumReaches", &SBReachManager::getNumReaches, "Returns the number of reach engines present.")
		.def("getReach", &SBReachManager::getReach, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a reach engine for a given character.")
		;

	boost::python::class_<SBGestureMap>("SBGestureMap")
		.def("addGestureMapping", &SBGestureMap::addGestureMapping, "Add a gesture mapping. Input: name of the animation/state, type, posture, hand. Output: null")
		.def("getGestureByInfo", &SBGestureMap::getGestureByInfo, "Return a gesture given the type and hand of the gesture. Input: type, hand, style. Output: corresponding gesture name")
		.def("getGestureByIndex", &SBGestureMap::getGestureByIndex, "Return a gesture given the index inside the map.")
		.def("getNumMappings", &SBGestureMap::getNumMappings, "Return a number of entries inside the map.")
		;

	boost::python::class_<SBGestureMapManager>("SBGestureMapManager")
		.def("createGestureMap", &SBGestureMapManager::createGestureMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a gesture map for a character.")
		.def("removeGestureMap", &SBGestureMapManager::removeGestureMap, "Remove a gesture map for a character given character name.")
		.def("getNumGestureMaps", &SBGestureMapManager::getNumGestureMaps, "Return number of gesture maps in the scene.")
		.def("getGestureMap", &SBGestureMapManager::getGestureMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return gesture map given character name.")
		;

	boost::python::class_<SBJointMap>("SBJointMap")
		.def("setMapping", &SBJointMap::setMapping, "Sets the mapping from one joint name to another.")
		.def("removeMapping", &SBJointMap::removeMapping, "Removes a mapping from a given joint to whichever joint is mapped.")
		.def("getMapTarget", &SBJointMap::getMapTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the target joint for a given mapping.")
		.def("getMapSource", &SBJointMap::getMapSource, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the source joint for a given mapping.")
		.def("getNumMappings", &SBJointMap::getNumMappings, "Returns the number of joint mappings.")
		.def("getSource", &SBJointMap::getSource, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the source joint of the nth mapping.")
		.def("getTarget", &SBJointMap::getTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the target joint of the nth mapping.")
		.def("applyMotion", &SBJointMap::applyMotion, "Applies the current joint mapping to a motion.")
		.def("applySkeleton", &SBJointMap::applySkeleton, "Applies the current skeleton mapping to a motion.")
		.def("applyMotionInverse", &SBJointMap::applyMotionInverse, "Applies the inverse joint mapping to a motion.")
		.def("applySkeletonInverse", &SBJointMap::applySkeletonInverse, "Applies the inverse skeleton mapping to a motion.")
		.def("guessMapping", &SBJointMap::guessMapping, "Automatic joint name matching to standard SmartBody names.")
		.def("getMappedMotions", &SBJointMap::getMappedMotions, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of the motions that have been mapped by this joint map.")
		.def("getMappedSkeletons", &SBJointMap::getMappedSkeletons, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns a list of the skeletons that have been mapped by this joint map.")
		;

	boost::python::class_<SBJointMapManager>("SBJointMapManager")
		.def("getJointMap", &SBJointMapManager::getJointMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint map associated with a given name.")
		.def("createJointMap", &SBJointMapManager::createJointMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a joint map with a given name. Returns null is the map already exists.")
		.def("getJointMapNames", &SBJointMapManager::getJointMapNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the names of all joint maps.")
		.def("removeJointMap", &SBJointMapManager::removeJointMap, "Removes a joint map of a given name.")
		.def("removeAllJointMaps", &SBJointMapManager::removeAllJointMaps, "Removes all the joint maps.")
		;

	boost::python::class_<Event>("Event")
		.def(boost::python::init<>())
		.def("getType", &Event::getType, "Returns the event type.")
		.def("setType", &Event::setType, "Sets the event type.")
		.def("getParameters", &Event::getParameters, "Returns the event parameters.")
		.def("setParameters", &Event::setParameters, "Sets the event parameters.")
		;

	boost::python::class_<EventManager>("EventManager")
		.def("handleEvent", &EventManager::handleEvent, "Processes an event by the appropriate event handler.")
		.def("addEventHandler", &EventManager::addEventHandler, "Returns the event type.")
		.def("removeEventHandler", &EventManager::removeEventHandler, "Returns the event type.")
		.def("getNumHandlers", &EventManager::getNumEventHandlers, "Gets the number of event handlers.")
		.def("getEventHandler", &EventManager::getEventHandler, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the number of event handlers.")
		.def("createEvent", &EventManager::createEvent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates an event of a given type with given parameters.")
		;

	boost::python::class_<SBParseNode>("SBParseNode")
		.def("getWord", &SBParseNode::getWord, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the word, empty word if not a terminal node.")
		.def("getTerm", &SBParseNode::getTerm, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the terminal, empty word if not a terminal node.")
		.def("isTerminal", &SBParseNode::isTerminal, "Is this node a terminal node.")
		.def("getNumChildren", &SBParseNode::getNumChildren, "Deletes parse tree.")
		.def("getChild", &SBParseNode::getChild, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a child node.")
		;

	boost::python::class_<SBParser>("SBParser")
		.def("parse", &SBParser::parse, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Parses an utterance using the Charniak parser.")
		.def("initialize", &SBParser::initialize, "Initializes the Charniak parser with parameters.")
		.def("cleanUp", &SBParser::cleanUp, "Deletes parse tree.")
		.def("isInitialized", &SBParser::isInitialized, "Return boolean telling if Charniak parser is initialized.")
		;

	boost::python::class_<SrVec>("SrVec")
		.def(boost::python::init<>())
		.def(boost::python::init<float, float, float>())
		.def("getData", &SrVec::getData, "gets the x,y,z values")
		.def("setData", &SrVec::setData, "sets the x,y,z values")
		.def("len", &SrVec::norm, "gets the length of the vector")
		.def("normalize", &SrVec::normalize, "normalizes the vector")
		.def("isZero", &SrVec::iszero, "returns True if the vector is zero")
		.def("rotY", &SrVec::rotY, "rotate vector around Y axis (radian)")
		.def("vecAngle", &SrVec::vecAngle, "Returns the angle between v1 and v2 (radian)")
		.def("vecYaw", &SrVec::vecYaw, "Returns Yaw angle on X-Z plane of given vec (radian)")
		;

	boost::python::class_<SrMat>("SrMat")
		.def(boost::python::init<>())
		.def("getData", &SrMat::getData, "gets the data in the matrix at r,c")
		.def("setData", &SrMat::setData, "sets the data in the matrix at r,c")
		.def("identity", &SrMat::identity, "sets the data in the matrix to an identity matrix")
		.def("transpose", &SrMat::transpose, "transposes the data in the matrix")
		;

	boost::python::class_<SrQuat>("SrQuat")
		.def(boost::python::init<>())
		.def(boost::python::init<float, float, float, float>())
		.def("getData", &SrQuat::getData, "gets the data in the quaterion at location indicated by the index w,x,y,z")
		.def("setData", &SrQuat::setData, "sets the data in the quaterion at location indicated by the index w,x,y,z")
		;	
	
	boost::python::class_<SrBox>("SrBox")
		.def(boost::python::init<>())
		.def(boost::python::init<SrVec, SrVec>())
		.def(boost::python::init<SrBox>())
		.def("setMinimum", &SrBox::setMinimum, "sets the minimum values of the box")
		.def("setMaximum", &SrBox::setMaximum,  "sets the maximum values of the box")
		.def("getMinimum", &SrBox::getMinimum, boost::python::return_value_policy<boost::python::return_by_value>(), "gets the minimum values of the box")
		.def("getMaximum", &SrBox::getMaximum, boost::python::return_value_policy<boost::python::return_by_value>(), "gets the maximum values of the box")
		.def("getCenter", &SrBox::getCenter, boost::python::return_value_policy<boost::python::return_by_value>(), "gets center of the box")
		.def("getMinSize", &SrBox::min_size, "gets the minimum dimension of the box")
		.def("getMaxSize", &SrBox::max_size, "gets the maximum dimension of the box")
		.def("getSize", &SrBox::getSize, boost::python::return_value_policy<boost::python::return_by_value>(), "returns the size of each dimension")
		.def("doesContain", &SrBox::contains, "returns the center of the box")
		.def("doesIntersect", &SrBox::intersects, "returns the center of the box")
		.def("getVolume", &SrBox::volume, "returns the volume of the box")
		.def("isEmpty", &SrBox::empty, "returns true if the box is empty")
		;

	boost::python::class_<NvbgWrap, boost::python::bases<SBObject>, boost::noncopyable>("Nvbg")
		.def("objectEvent", &Nvbg::objectEvent, &NvbgWrap::default_objectEvent, "An event indicating that an object of interest is present.")
		.def("execute", &Nvbg::execute, &NvbgWrap::default_execute, "Execute the xml vrX message.")
		.def("executeEvent", &Nvbg::executeEvent, &NvbgWrap::default_executeEvent, "Execute the vrAgent message.")
		.def("executeSpeech", &Nvbg::executeSpeech, &NvbgWrap::default_executeSpeech, "Execute the vrSpeech message.")
		.def("notifyAction", &Nvbg::notifyAction, &NvbgWrap::default_notifyAction, "Notifies NVBG processor of a bool attribute.")
		.def("notifyBool", &Nvbg::notifyBool, &NvbgWrap::default_notifyBool, "Notifies NVBG processor of a bool attribute")
		.def("notifyInt", &Nvbg::notifyInt, &NvbgWrap::default_notifyInt, "Notifies NVBG processor of an int attribute")
		.def("notifyDouble", &Nvbg::notifyDouble, &NvbgWrap::default_notifyDouble, "Notifies NVBG processor of a double attribute")
		.def("notifyString", &Nvbg::notifyString, &NvbgWrap::default_notifyString, "Notifies NVBG processor of a string attribute")
		.def("notifyVec3", &Nvbg::notifyVec3, &NvbgWrap::default_notifyVec3, "Notifies NVBG processor of a vec3 attribute.")
		.def("notifyMatrix", &Nvbg::notifyMatrix, &NvbgWrap::default_notifyMatrix, "Notifies NVBG processor of a matrix attribute.")
		;

	boost::python::class_<SBScriptWrap, boost::noncopyable>("SBScript")
		.def("start", &SBScript::start, &SBScriptWrap::default_start, "Script start.")
		.def("beforeUpdate", &SBScript::beforeUpdate, &SBScriptWrap::default_beforeUpdate, "Script before update step.")
		.def("update", &SBScript::update, &SBScriptWrap::default_update, "Script updates.")
		.def("afterUpdate", &SBScript::afterUpdate, &SBScriptWrap::default_afterUpdate, "Script after update step.")
		.def("stop", &SBScript::stop, &SBScriptWrap::default_stop, "Script stop.")
		;

	boost::python::class_<EventHandlerWrap, boost::noncopyable>("EventHandler")
		.def("executeAction", &EventHandler::executeAction, &EventHandlerWrap::default_executeAction, "Execute the event handler.")
		;

	boost::python::class_<PythonControllerWrap, boost::python::bases<SBController>, boost::noncopyable> ("PythonController")
		.def("start", &PythonController::start, &PythonControllerWrap::default_start, "start.")
		.def("stop", &PythonController::stop, &PythonControllerWrap::default_stop, "stop.")
		.def("init", &PythonController::init, &PythonControllerWrap::default_init, "init.")
		.def("evaluate", &PythonController::evaluate, &PythonControllerWrap::default_evaluate, "evaluate.")
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
		.def("save", &SBScene::save, "Saves the SmartBody configuration. Returns a string containing Python commands representing the configuration.")
		.def("exportScene", &SBScene::exportScene, "Saves the entire SmartBody configuration, including assets, into a given file location.")

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
#endif
	}
#endif


}


void initPython(std::string pythonLibPath)
{	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.initPythonLibPath = pythonLibPath;
	std::string pythonHome = pythonLibPath + "/..";
#ifdef USE_PYTHON
#ifdef __ANDROID__
	Py_SetProgramName("/sdcard/sbmmedia/python/");
#else
	Py_SetProgramName("../../../../core/smartbody/Python26/");
#ifdef WIN32
	Py_SetPythonHome((char*)pythonHome.c_str());
#endif
#endif	
	Py_Initialize();
	
	try {
#ifdef USE_PYTHON
		mcu.mainModule = boost::python::import("__main__");
		mcu.mainDict = mcu.mainModule.attr("__dict__");
	
		PyRun_SimpleString("import sys");
#endif
#ifndef WIN32
		// set the proper python path
		std::stringstream strstr;
		strstr << "sys.path.append(\"";
		strstr << pythonLibPath;
		strstr << "\");";	
		PyRun_SimpleString(strstr.str().c_str());

		// add path to site-packages
		std::string pythonSitePackagePath = pythonLibPath + "/site-packages";
		strstr.str(std::string());
		strstr.clear();
		strstr << "sys.path.append(\"";
		strstr << pythonSitePackagePath;
		strstr << "\");";
		PyRun_SimpleString(strstr.str().c_str());

		// add path to DLLs
		std::string pythonDLLPath = pythonLibPath + "/../DLLs";
		strstr.str(std::string());
		strstr.clear();
		strstr << "sys.path.append(\"";
		strstr << pythonDLLPath;
		strstr << "\");";
		PyRun_SimpleString(strstr.str().c_str());
#endif

		SmartBody::initSmartBody();
		
		

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
		PyRun_SimpleString("from pydoc import *");
#if 1
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


