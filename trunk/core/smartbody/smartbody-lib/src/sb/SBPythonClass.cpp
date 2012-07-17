#include "SBPythonClass.h"
#include "controllers/me_ct_reach.hpp"

namespace SmartBody 
{

#ifdef USE_PYTHON


std::string PyLogger::strBuffer = "";

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::printInfo()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::cout << "   Camera Info " << std::endl;
	std::cout << "-> eye position:	" << "(" << mcu.camera_p->eye.x << ", " << mcu.camera_p->eye.y << ", " << mcu.camera_p->eye.z << ")" << std::endl;
	std::cout << "-> center position:	" << "(" << mcu.camera_p->center.x << ", " << mcu.camera_p->center.y << ", " << mcu.camera_p->center.z << ")" << std::endl;
	std::cout << "-> up vector:		" << "(" << mcu.camera_p->up.x << ", " << mcu.camera_p->up.y << ", " << mcu.camera_p->up.z << ")" << std::endl;
	std::cout << "-> fovy:		" << mcu.camera_p->fovy << std::endl;
	std::cout << "-> near plane:		" << mcu.camera_p->znear << std::endl;
	std::cout << "-> far plane:		" << mcu.camera_p->zfar << std::endl;
	std::cout << "-> aspect:		" << mcu.camera_p->aspect << std::endl;
	std::cout << "-> scale:		" << mcu.camera_p->scale << std::endl;
}


void Camera::setEye(float x, float y, float z)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->eye.x = x;
	mcu.camera_p->eye.y = y;
	mcu.camera_p->eye.z = z;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::setCenter(float x, float y, float z)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->center.x = x;
	mcu.camera_p->center.y = y;
	mcu.camera_p->center.z = z;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::setScale(float s)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->scale = s;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::reset()
{
	setEye(0, 166, 185);
	setCenter(0, 92, 0);
}

void Camera::setDefault(int preset)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (preset == 1)
		mcu.viewer_p->view_all();
	else
		LOG("defaultCamera func option not valid.");
}

void Camera::setTrack(std::string cName, std::string jName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(cName);
	if (!pawn)
	{
		LOG("Object %s was not found, cannot track.", cName.c_str());
		return;
	}
	if (jName == "")
	{
		LOG("Need to specify a joint to track.");
		return;
	}

	SkSkeleton* skeleton = NULL;
	skeleton = pawn->getSkeleton();

	SkJoint* joint = pawn->getSkeleton()->search_joint(jName.c_str());
	if (!joint)
	{
		LOG("Could not find joint %s on object %s.", jName.c_str(), cName.c_str());
		return;
	}

	joint->skeleton()->update_global_matrices();
	joint->update_gmat();
	const SrMat& jointMat = joint->gmat();
	SrVec jointPos(jointMat[12], jointMat[13], jointMat[14]);
	CameraTrack* cameraTrack = new CameraTrack();
	cameraTrack->joint = joint;
	cameraTrack->jointToCamera = mcu.camera_p->eye - jointPos;
	LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
	cameraTrack->targetToCamera = mcu.camera_p->eye - mcu.camera_p->center;
	LOG("Vector from target to eye is %f %f %f", cameraTrack->targetToCamera.x, cameraTrack->targetToCamera.y, cameraTrack->targetToCamera.z);				
	mcu.cameraTracking.push_back(cameraTrack);
	LOG("Object %s will now be tracked at joint %s.", cName.c_str(), jName.c_str());
}

void Camera::removeTrack()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.cameraTracking.size() > 0)
	{
		for (std::vector<CameraTrack*>::iterator iter = mcu.cameraTracking.begin();
			 iter != mcu.cameraTracking.end();
			 iter++)
		{
			CameraTrack* cameraTrack = (*iter);
			delete cameraTrack;
		}
		mcu.cameraTracking.clear();
		LOG("Removing current tracked object.");
	}
}

void Camera::loadCamera(std::string camFileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	FILE * pFile;
	SrString f(camFileName.c_str());
	pFile = fopen (f,"r");
	if (pFile!=0)
	{
		SrInput file_in (pFile);
		file_in >> *(mcu.camera_p);
		fclose (pFile);
		mcu.viewer_p->set_camera(*mcu.camera_p);
	}
	else
		LOG("WARNING: can not load cam file!");
}

void Camera::saveCamera(std::string camFileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SrCamera* cam = mcu.viewer_p->get_camera();

	FILE * pFile = 0;
	SrString f(camFileName.c_str());
	pFile = fopen (f,"w");
	if (pFile!=0)
	{
		SrOutput file_out (pFile);
		file_out << *(cam);
		fclose (pFile);
	}
	else
		LOG("WARNING: can not save cam file");
}


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
	else if (controllerType == "locomotion")
	{
		controller = new MeCtLocomotion();
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

Camera* getCamera()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (mcu.camera_p)
	{
		Camera* camera = new Camera();
		return camera;
	}
	else
	{
		LOG("Camera not exists, returning NULL instead.");
		return NULL;
	}
}

SrViewer* getViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.viewer_p)
	{
		mcu.viewer_p = mcu.viewer_factory->create(100, 100, 640, 480);
		mcu.viewer_p->label_viewer("Visual Debugger");
		mcu.camera_p = new SrCamera();
		mcu.viewer_p->set_camera(*mcu.camera_p);
		mcu.viewer_p->root(mcu.root_group_p);
	}
	return mcu.viewer_p;
}

GenericViewer* getBmlViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.bmlviewer_p)
	{
		mcu.bmlviewer_p = mcu.bmlviewer_factory->create(100, 100, 640, 480);
		mcu.bmlviewer_p->label_viewer( "BML Viewer");	
	}
	return mcu.bmlviewer_p;
}

GenericViewer* getDataViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.channelbufferviewer_p)
	{
		mcu.channelbufferviewer_p = mcu.channelbufferviewer_factory->create(100, 100, 640, 480);
		mcu.channelbufferviewer_p->label_viewer( "Data Viewer");	
	}

	return mcu.channelbufferviewer_p;
}

GenericViewer* getPanimationViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.panimationviewer_p)
	{
		mcu.panimationviewer_p = mcu.panimationviewer_factory->create(100, 100, 640, 480);
		mcu.panimationviewer_p->label_viewer( "Parameterized Animation Viewer");	
	}

	return mcu.panimationviewer_p;
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
