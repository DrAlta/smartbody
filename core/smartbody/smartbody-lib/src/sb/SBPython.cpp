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
void pythonFuncs1();
void pythonFuncs2();
void pythonFuncs3();
void pythonFuncs4();

#ifdef USE_PYTHON
BOOST_PYTHON_MODULE(SmartBody)
{
    pythonFuncs1();
    pythonFuncs2();
    pythonFuncs3();
    pythonFuncs4();

#if 1

	

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


