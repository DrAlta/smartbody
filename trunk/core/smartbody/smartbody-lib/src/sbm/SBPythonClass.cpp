#include "SBPythonClass.h"
#include "sbm/me_ct_reach.hpp"

namespace SmartBody 
{

#ifdef USE_PYTHON


std::string PyLogger::strBuffer = "";
/*
Script::Script()
{
}

Script::Script(std::string seqName)
{
	seq = seqName;
}

Script::~Script()
{
}

void Script::print()
{
	if (type == "seq")
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		srCmdSeq* seq_p = mcu.lookup_seq(seq.c_str());
		if( seq_p == NULL )	
		{
			LOG( "%s not exist", seq.c_str()); 
			return;
		}
		// Note: may need to use LOG instead of printing to stdout
		seq_p->print(stdout);
	}
	else
		LOG ("py file doesn't support abort function");
}

void Script::run()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	int err = 0;

	if (type == "seq")
	{
		srCmdSeq *seq_p = mcu.lookup_seq(seq.c_str());
		if (seq_p)
		{	
			srCmdSeq *cp_seq_p = new srCmdSeq;
			preProcessingScript(cp_seq_p, seq_p);
			cp_seq_p->offset((float)(mcu.time));

			if (!mcu.activeSequences.getSequence(seq))
			{
				mcu.activeSequences.addSequence(seq, cp_seq_p);
			}
			else
			{
				LOG("Unable to run a script '%s': this file cannot be inserted to the active sequences.", seq.c_str()); 
			}
			
		}
		else
		{
			LOG("Unable to run a script '%s': this file cannot be found.", seq.c_str()); 
		}
	}
	else if (type == "py")
	{
		std::string cmd = "execfile(\"" + seq + "\")";
		mcu.executePython(cmd.c_str());
	}

}

void Script::abort()
{
	if (type == "seq")
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		int result = mcu.abortSequence(seq.c_str());
		if (result == 0)
			LOG("'%s' not found, cannot abort.", seq.c_str()); 
	}
	else
		LOG ("py file doesn't support abort function");
}

void Script::preProcessingScript(srCmdSeq *to_seq_p, srCmdSeq *fr_seq_p)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene& scene = (*mcu._scene);

	float t;
	char *cmd;	
	fr_seq_p->reset();
	while (cmd = fr_seq_p->pull(&t))	
	{
		srArgBuffer args(cmd);
		srCmdSeq *inline_seq_p = NULL;
		char *tok = args.read_token();
		if (strcmp(tok, "path") == 0)	
		{
			char *path_tok = args.read_token();
			char* path = args.read_token();
			scene.addAssetPath(std::string(path_tok), std::string(path));
			delete [] cmd;
			cmd = NULL;
		}
		else if (strcmp(tok, "seq") == 0)
		{
			char *name = args.read_token();
			tok = args.read_token();
			if (strcmp(tok, "inline") == 0)
			{
				inline_seq_p = mcu.lookup_seq(name);
				delete [] cmd;
				cmd = NULL;
				if (inline_seq_p == NULL)	
				{
					LOG("Error when pre-process script '%s': inline seq '%s' not found.", seq.c_str(), name);
					return;
				}
			}
		}
		
		float absolute_offset = fr_seq_p->offset() + t;
		if( inline_seq_p )	{
			// iterate hierarchy
			inline_seq_p->offset(absolute_offset);
			preProcessingScript(to_seq_p, inline_seq_p);
		}
		else if (cmd)	
			to_seq_p->insert_ref(absolute_offset, cmd);
	}
	delete fr_seq_p;
}


*/


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


/*
Viseme::Viseme()
{
}

Viseme::~Viseme()
{
}

void Viseme::setWeight(float weight, float dur, float rampin, float rampout)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		if (dur < 0.0)		dur = 0.0f;
		if (rampin < 0.0)	rampin = 0.1f;
		if (rampout < 0.0)	rampout = 0.1f;
		character->schedule_viseme_trapezoid( visemeName.c_str(), mcu.time, weight, dur, rampin, rampout );
	}
}

void Viseme::setCurve(int numKeys, boost::python::list weights)
{
	if( numKeys <= 0 )	
	{
		LOG( "Viseme data is missing" );
		return;
	}

	int num_remaining = len(weights);
	int numKeyParams = num_remaining / numKeys;
	if (num_remaining != numKeys * numKeyParams)
	{
		LOG( "Viseme data is malformed" );
		return;
	}
	float* curveInfo = new float[num_remaining];
	for (int i = 0; i < num_remaining; i++)
		curveInfo[i] = boost::python::extract<float> (weights[i]);

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		character->schedule_viseme_blend_curve( visemeName.c_str(), mcu.time, 1.0f, curveInfo, numKeys, numKeyParams );
	}

	delete [] curveInfo;
}
*/


int init_motion_controller( 
	const char *ctrl_name, 
	const char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(mot_name);
	if( motionIter == mcu_p->motion_map.end() ) {
		LOG( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_p = (*motionIter).second;

	MeCtMotion* ctrl_p = new MeCtMotion;
	/*
	err = mcu_p->motion_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeCtMotion '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	*/
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, mot_p );
	return( CMD_SUCCESS );
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
	LOG(message.c_str());
}

SBScene* getScene()
{	
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	return mcu._scene;
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

void execScripts(boost::python::list& input)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::vector<std::string> seq_names;
	for (int i = 0; i < len(input); ++i)
		seq_names.push_back(boost::python::extract<std::string>(input[i]));

	if (seq_names.empty())
	{
		LOG("ERROR: seq-chain expected one or more .seq filenames.");
		return;
	}
	
	mcu.execute_seq_chain(seq_names, "ERROR: seq-chain: ");
}

/*
Script* getScript(std::string fileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (fileName.substr(fileName.find_last_of(".") + 1) == "seq")
	{
		srCmdSeq *seq_p = mcu.lookup_seq(fileName.c_str());
		if (seq_p)
		{
			Script* script = new Script(fileName);
			script->setType("seq");
			return script;
		}
		else
		{
			LOG("Script %s not found. Returns Null.", fileName.c_str());
			return NULL;
		}
	}
	else if (fileName.substr(fileName.find_last_of(".") + 1) == "py")
	{
		std::string pythonFile = getScriptFromFile(fileName);
		if (pythonFile == "")
		{
			LOG("Script %s not found. Returns Null.", fileName.c_str());
			return NULL;
		}
		else
		{
			Script* script = new Script(pythonFile);
			script->setType("py");
			return script;
		}
	}
	else
	{
		LOG("Script %s's extension not recognized, please verify.", fileName.c_str());
		return NULL;
	}
}

*/

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
