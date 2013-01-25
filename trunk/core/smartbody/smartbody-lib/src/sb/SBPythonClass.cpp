#include "vhcl.h"
#include "SBPythonClass.h"
#include "controllers/me_ct_reach.hpp"

#include <sbm/resource_cmds.h>
#include <sb/sbm_character.hpp>
#include <sbm/me_utilities.hpp>
#include <sk/sk_skeleton.h>
#include <sk/sk_joint.h>
#include <sbm/sbm_test_cmds.hpp>
#include <controllers/me_ct_param_animation.h>
#include <controllers/me_ct_scheduler2.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_ct_eyelid.h>
#include <controllers/me_ct_face.h>
#include <controllers/me_ct_curve_writer.hpp>

namespace SmartBody 
{

SrViewer* getViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.viewer_p)
	{
		mcu.viewer_p = mcu.viewer_factory->create(100, 100, 800, 800);
		mcu.viewer_p->label_viewer("Visual Debugger");
		SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
		mcu.camera_p = scene->createCamera("cameraDefault");
		scene->setActiveCamera(mcu.camera_p);
		mcu.viewer_p->root(mcu.root_group_p);
	}
	return mcu.viewer_p;
}

#ifndef SB_NO_PYTHON


std::string PyLogger::strBuffer = "";

void pythonExit()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.use_python = false;
}

void quitSbm()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.loop = false;
}

void reset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.reset();
}

void printLog(const std::string& message)
{
	string s = message;
	if (vhcl::EndsWith(s, "\n"))
		s.erase(s.length() - 1);
	if (s.length() > 0)
		LOG(message.c_str());
}



SBController* createController(std::string controllerType, std::string controllerName)
{
	SBController* controller = NULL;

	if (controllerType == "schedule")
	{
		controller = new MeCtScheduler2();
		
	}
	else if (controllerType == "gaze")
	{
		controller = new MeCtGaze();
	}
	else if (controllerType == "eyelid")
	{
		controller = new MeCtEyeLidRegulator();
	}
	else if (controllerType == "face")
	{
		controller = new MeCtFace();
	}
	else if (controllerType == "paramanimation")
	{
		controller = new MeCtParamAnimation();
	}
	else if (controllerType == "curvewriter")
	{
		controller = new MeCtCurveWriter();
	}

	if (controller)
		controller->setName(controllerName);

	return controller;
}

SrCamera* getCamera()
{
	return SmartBody::SBScene::getScene()->getActiveCamera();
}

void showCommandResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		CmdResource * res = dynamic_cast<CmdResource *>(mcu.resource_manager->getResource(r));
		if (res)
			LOG("%s", res->dump().c_str());
	}	
}

void showMotionResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		MotionResource * res = dynamic_cast<MotionResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());

	}	
}


void showSkeletonResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		SkeletonResource * res = dynamic_cast<SkeletonResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());

	}	
}

void showPathResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		PathResource * res = dynamic_cast<PathResource *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}
}

void showScriptResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		FileResource * res = dynamic_cast<FileResource *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}
}

void showControllerResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		ControllerResource * res = dynamic_cast<ControllerResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}	
}

void getResourceLimit()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.resource_manager->getLimit();
}

void setResourceLimit(int limit)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (limit > 0)
		mcu.resource_manager->setLimit(limit);
}


std::string getScriptFromFile(std::string fileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	FILE* file_p = NULL;
	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];
	sprintf( label, "%s", fileName.c_str() );
	mcu.seq_paths.reset();
	std::string filename = mcu.seq_paths.next_filename( buffer, label );
	
	while (filename.size() > 0 && filename != "")	{
		file_p = fopen( filename.c_str(), "r" );
		if( file_p != NULL ) {
	
			// add the file resource
			FileResource* fres = new FileResource();
			std::stringstream stream;
			stream << filename;
			fres->setFilePath(stream.str());
			mcu.resource_manager->addResource(fres);
			
			break;
		}
		filename = mcu.seq_paths.next_filename( buffer, label );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension	
		sprintf( label, "%s.py", fileName.c_str() );
		mcu.seq_paths.reset();
		filename = mcu.seq_paths.next_filename( buffer, label );
		while (filename.size() > 0)	{
			if( ( file_p = fopen( filename.c_str(), "r" ) ) != NULL ) {
				
				// add the file resource
				FileResource* fres = new FileResource();
				std::stringstream stream;
				stream << filename;
				fres->setFilePath(stream.str());
				mcu.resource_manager->addResource(fres);
				break;
			}
			filename = mcu.seq_paths.next_filename( buffer, label );
		}
	}

	// return empty string if file not found
	if (file_p == NULL)	return "";
	else				return std::string(filename);	
}

////////////////////////////////////////////

void PyLogger::pa()
{
	strBuffer += "a";		
}

void PyLogger::pb()
{
	strBuffer += "b";		
}
void PyLogger::pc()
{
	strBuffer += "c";		
}

void PyLogger::pd()
{
	strBuffer += "d";		
}
void PyLogger::pe()
{
	strBuffer += "e";		
}

void PyLogger::pf()
{
	strBuffer += "f";		
}
void PyLogger::pg()
{
	strBuffer += "g";		
}

void PyLogger::ph()
{
	strBuffer += "h";		
}
void PyLogger::pi()
{
	strBuffer += "i";		
}

void PyLogger::pj()
{
	strBuffer += "j";		
}
void PyLogger::pk()
{
	strBuffer += "k";		
}

void PyLogger::pl()
{
	strBuffer += "l";		
}

void PyLogger::pm()
{
	strBuffer += "m";		
}
void PyLogger::pn()
{
	strBuffer += "n";		
}

void PyLogger::po()
{
	strBuffer += "o";		
}
void PyLogger::pp()
{
	strBuffer += "p";		
}

void PyLogger::pq()
{
	strBuffer += "q";		
}
void PyLogger::pr()
{
	strBuffer += "r";		
}

void PyLogger::ps()
{
	strBuffer += "s";		
}
void PyLogger::pt()
{
	strBuffer += "t";		
}

void PyLogger::pu()
{
	strBuffer += "u";		
}
void PyLogger::pv()
{
	strBuffer += "v";		
}

void PyLogger::pw()
{
	strBuffer += "w";		
}
void PyLogger::px()
{
	strBuffer += "x";		
}

void PyLogger::py()
{
	strBuffer += "y";		
}
void PyLogger::pz()
{
	strBuffer += "z";		
}
void PyLogger::pspace()
{
	strBuffer +=" ";
}

void PyLogger::p1()
{
	strBuffer +="1";
}
void PyLogger::p2()
{
	strBuffer +="2";
}
void PyLogger::p3()
{
	strBuffer +="3";
}
void PyLogger::p4()
{
	strBuffer +="4";
}
void PyLogger::p5()
{
	strBuffer +="5";
}
void PyLogger::p6()
{
	strBuffer +="6";
}
void PyLogger::p7()
{
	strBuffer +="7";
}
void PyLogger::p8()
{
	strBuffer +="8";
}
void PyLogger::p9()
{
	strBuffer +="9";
}
void PyLogger::p0()
{
	strBuffer +="0";
}




void PyLogger::pnon()
{
	strBuffer +=".";
}

void PyLogger::outlog()
{
	if (strBuffer.size() > 1)
	{
		LOG("pyLog : %s",strBuffer.c_str());
		strBuffer = "";
	}	
}
#endif

}