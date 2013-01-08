#include <vhcl.h>
#include "RootWindow.h"
#include "CharacterCreatorWindow.h"
#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <sstream>
#include <FL/filename.H>
#include "boost/filesystem.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "sbm/sbm_audio.h"
#include <fstream>
#include "CommandWindow.h"
#include <sb/SBSkeleton.h>
#include <sb/SBDebuggerClient.h>

BaseWindow::BaseWindow(int x, int y, int w, int h, const char* name) : SrViewer(x, y, w, h), Fl_Double_Window(x, y, w, h, name)
{
	commandWindow = NULL;
	bmlCreatorWindow = NULL;
	this->begin();

	menubar = new Fl_Menu_Bar(0, 0, w, 30); 
	menubar->labelsize(10);
	menubar->add("&File/New", 0, NewCB, this, NULL);	
	menubar->add("&File/Save", 0, SaveCB, this, NULL);	
	menubar->add("&File/Load...", 0, LoadCB, this, NULL);
	menubar->add("&File/Connect...", 0, LaunchConnectCB, this, NULL);
	menubar->add("&File/Disconnect", 0, DisconnectRemoteCB, this, NULL);
	menubar->add("&File/&Quit", 0, QuitCB, this, NULL);
//	menubar->add("&File/Save Configuration...", 0, NULL, 0, NULL);
//	menubar->add("&File/Run Script...", 0, NULL, 0, NULL);
	menubar->add("&View/Character/Bones", 0, ModeBonesCB, this, NULL);
	menubar->add("&View/Character/Geometry", 0, ModeGeometryCB, this, NULL);
	menubar->add("&View/Character/Collision Geometry", 0, ModeCollisionGeometryCB, this, NULL);
	menubar->add("&View/Character/Deformable Geometry", 0, ModeDeformableGeometryCB, this, NULL);
	menubar->add("&View/Character/GPU Deformable Geometry", 0, ModeGPUDeformableGeometryCB, this, NULL);
	menubar->add("&View/Character/Axis", 0, ModeAxisCB, this, NULL);
//	menubar->add("&View/Character/Show Selected", 0, ShowSelectedCB, this, NULL);	
	menubar->add("&View/Character/Eyebeams", 0, ModeEyebeamsCB, this, NULL);
//	menubar->add("&View/Character/Eyelid calibration", 0, ModeEyelidCalibrationCB, this, NULL);
	menubar->add("&View/Character/Bounding Volumes", 0, ShowBoundingVolumeCB, this, NULL);
//	menubar->add("&View/Character/Dynamics/COM", 0, ModeDynamicsCOMCB, this, NULL);
//	menubar->add("&View/Character/Dynamics/Support Polygon", 0, ModeDynamicsSupportPolygonCB, this, NULL);
//	menubar->add("&View/Character/Dynamics/Masses", 0, ModeDynamicsMassesCB, this, NULL);
//	menubar->add("&View/Character/Locomotion/Kinematic Footsteps", 0, KinematicFootstepsCB, this, NULL);
//	menubar->add("&View/Character/Locomotion/Locomotion Footsteps", 0, LocomotionFootstepsCB, this, NULL);
//	menubar->add("&View/Character/Locomotion/Velocity", 0, VelocityCB, this, NULL);
//	menubar->add("&View/Character/Locomotion/Trajectory", 0, TrajectoryCB, this, NULL);
	menubar->add("&View/Character/Show Trajectory", 0, TrajectoryCB, this, NULL);
	menubar->add("&View/Character/Show Gesture", 0, GestureCB, this, NULL);
	menubar->add("&View/Character/Show Joint Labels", 0, JointLabelCB, this, NULL);
	menubar->add("&View/Pawns", 0, ShowPawns, this, NULL);
	menubar->add("&View/Shadows", 0, ShadowsCB, this, NULL);
	menubar->add("&View/Grid", 0, GridCB, this, NULL);
	//menubar->add("&View/Reach Pose Examples", 0, ShowPoseExamples, this, NULL);	
	menubar->add("&View/Terrain/Shaded", 0, TerrainShadedCB, this, NULL);
	menubar->add("&View/Terrain/Wireframe", 0, TerrainWireframeCB, this, NULL);
	menubar->add("&View/Terrain/No Terrain", 0, TerrainNoneCB, this, NULL);	
	menubar->add("&View/Steer/Characters and Goals", 0, SteeringCharactersCB, this, NULL);
	menubar->add("&View/Steer/All Steering", 0, SteeringAllCB, this, NULL);
	menubar->add("&View/Steer/No Steering", 0, SteeringNoneCB, this, NULL);
/*
	windowSizes.push_back("640x480");
	windowSizes.push_back("720x480");
	windowSizes.push_back("720x576");
	windowSizes.push_back("800x600");
	windowSizes.push_back("854x480");
	windowSizes.push_back("960x600");
	windowSizes.push_back("1024x576");
	windowSizes.push_back("1024x768");
	windowSizes.push_back("1280x720");
	windowSizes.push_back("1280x768");
	windowSizes.push_back("1366x768");
	windowSizes.push_back("1280x800");
	windowSizes.push_back("1280x1024");
	windowSizes.push_back("1440x900");
	windowSizes.push_back("1600x900");
	windowSizes.push_back("1920x1080");

	menubar->add("&View/Size/640.480", 0, ResizeWindowCB, this, 0);
	menubar->add("&View/Size/720.480", 0, ResizeWindowCB, this, 1);
	menubar->add("&View/Size/720x576", 0, ResizeWindowCB, this, 2);
	menubar->add("&View/Size/800x600", 0, ResizeWindowCB, this, 3);
	menubar->add("&View/Size/854x480", 0, ResizeWindowCB, this, 4);
	menubar->add("&View/Size/960x600", 0, ResizeWindowCB, this, 5);
	menubar->add("&View/Size/1024x576", 0, ResizeWindowCB, this, 6);
	menubar->add("&View/Size/1024x768", 0, ResizeWindowCB, this, 7);
	menubar->add("&View/Size/1280x720", 0, ResizeWindowCB, this, 8);
	menubar->add("&View/Size/1280x768", 0, ResizeWindowCB, this, 9);
	menubar->add("&View/Size/1366x768", 0, ResizeWindowCB, this, 10);
	menubar->add("&View/Size/1280x800", 0, ResizeWindowCB, this, 11);
	menubar->add("&View/Size/1280x1024", 0, ResizeWindowCB, this, 12);
	menubar->add("&View/Size/1440x900", 0, ResizeWindowCB, this, 13);
	menubar->add("&View/Size/1600x900", 0, ResizeWindowCB, this, 14);
	menubar->add("&View/Size/1920x1080", 0, ResizeWindowCB, this, 15);
*/
	menubar->add("&Create/Character...", 0, CreateCharacterCB, this, NULL);
	menubar->add("&Create/Pawn...", 0, CreatePawnCB, this, NULL);
	menubar->add("&Create/Light...", 0, CreateLightCB, this, NULL);
	menubar->add("&Create/Terrain...", 0, CreateTerrainCB, this, NULL);
	menubar->add("&Camera/Reset", 0, CameraResetCB, this, NULL);
	menubar->add("&Camera/Frame All", 0, CameraFrameCB, this, NULL);
	menubar->add("&Camera/Face Camera", 0, FaceCameraCB, this, NULL);
	menubar->add("&Camera/Track Character", 0, TrackCharacterCB, this, NULL);
	menubar->add("&Camera/Rotate Around Selected", 0, RotateSelectedCB, this, NULL);
	menubar->add("&Settings/Default Media Path", 0, SettingsDefaultMediaPathCB, this, NULL);
	menubar->add("&Settings/Internal Audio", 0, AudioCB, this, NULL);	
	menubar->add("&Window/Data Viewer", 0, LaunchDataViewerCB,this, NULL);
	menubar->add("&Window/BML Viewer", 0, LaunchBMLViewerCB, this, NULL);
	menubar->add("&Window/Blend Viewer", 0, LaunchParamAnimViewerCB, this, NULL);
	menubar->add("&Window/Resource Viewer", 0, LaunchResourceViewerCB, this, NULL);
	menubar->add("&Window/Command Window", 0, LaunchConsoleCB, this, NULL);
	menubar->add("&Window/BML Creator", 0, LaunchBMLCreatorCB, this, NULL);
	menubar->add("&Window/Face Viewer", 0, LaunchFaceViewerCB, this, NULL);
	menubar->add("&Window/Speech Relay", 0, LaunchSpeechRelayCB, this, NULL);
	menubar->add("&Window/Viseme Viewer", 0, LaunchVisemeViewerCB, this, NULL);
	menubar->add("&Window/Retarget Creator", 0, LaunchRetargetCreatorCB, this, NULL);
	//menubar->add("&Window/Behavior Sets", 0, LaunchBehaviorSetsCB, this, NULL);
	menubar->add("&Window/Motion Editor", 0, LaunchMotionEditorCB, this, NULL);
	menubar->add("&Window/Retarget Viewer", 0, LaunchJointMapViewerCB, this, NULL);
	menubar->add("&Help/Documentation", 0, DocumentationCB, this, NULL);
	menubar->add("&Help/Create Python API", 0, CreatePythonAPICB, this, NULL);
	//menubar->add("&Scripts/Reload Scripts", 0, ReloadScriptsCB, this, NULL);
	//menubar->add("&Scripts/Set Script Folder", 0, SetScriptDirCB, this, FL_MENU_DIVIDER);

	// disable the commands that are not yet functional
	/*
	Fl_Group* fileMenuOption = dynamic_cast<Fl_Group*>(menubar->child(0));
	if (fileMenuOption)
	{
		for (int c = 0; c < fileMenuOption->children(); c++)
		{
			fileMenuOption->child(c)->deactivate();
		}
	}
	*/

	
	int curY= 30;
	Fl_Group* cameraGroup = new Fl_Group(10, curY, w, 25, NULL);	
	//cameraGroup->type(Fl_Pack::HORIZONTAL);
	int curX = 60;

	cameraChoice = new Fl_Choice(curX, curY, 80, 25, "Camera");
	cameraChoice->when(FL_WHEN_NOT_CHANGED|FL_WHEN_CHANGED);
	cameraChoice->callback(ChooseCameraCB, this);
	updateCameraList();	
	cameraChoice->value(0);
	curX += 85;
	saveCamera = new Fl_Button(curX, curY, 45, 25, "Save");
	saveCamera->callback(SaveCameraCB, this);

 	curX += 125;
// 	deleteCamera = new Fl_Button(curX, curY, 45, 25, "Del");
// 	deleteCamera->callback(DeleteCameraCB, this);			
	windowSizes.push_back("640x480");
	windowSizes.push_back("720x480");
	windowSizes.push_back("720x576");
	windowSizes.push_back("800x600");
	windowSizes.push_back("854x480");
	windowSizes.push_back("960x600");
	windowSizes.push_back("1024x576");
	windowSizes.push_back("1024x768");
	windowSizes.push_back("1280x720");
	windowSizes.push_back("1280x768");
	windowSizes.push_back("1366x768");
	windowSizes.push_back("1280x800");
	windowSizes.push_back("1280x1024");
	windowSizes.push_back("1440x900");
	windowSizes.push_back("1600x900");
	windowSizes.push_back("1920x1080");

	resolutionChoice = new Fl_Choice(curX, curY, 80, 25, "Resolution");
	resolutionChoice->add("Default");
	for (unsigned int i=0;i<windowSizes.size();i++)
		resolutionChoice->add(windowSizes[i].c_str());
	resolutionChoice->add("Custom...");	
	resolutionChoice->when(FL_WHEN_NOT_CHANGED|FL_WHEN_CHANGED);
	resolutionChoice->callback(ResizeWindowCB,this);
	resolutionChoice->value(0);
	cameraGroup->end();	

	curY += 30;

	/*
	Fl_Pack* simGroup = new Fl_Pack(10, curY, 75, 25, NULL);
	simGroup->begin();
	int curX = 0;

	buttonPlay = new Button(curX, 0, 25, 25, "@>");
	buttonPlay->callback(StartCB, this);
	buttonPlay->set_vertical();

	buttonPlaybackStepForward = new Button(curX, 0, 25, 25, "@>|");
	buttonPlaybackStepForward->callback(StepCB, this);
	buttonPlaybackStepForward->set_vertical();
	curX += 25;

	buttonStop = new Button(curX, 0, 25, 25, "@square");
	buttonStop->callback(StopCB, this);
	buttonStop->set_vertical();

	inputTimeStep = new FloatInput(curX, 0, 25, 25);
	inputTimeStep->value("0");
	inputTimeStep->callback(NULL, this);
	curX += 25;	
	Fl_Output* spacer = new Fl_Output(curX, 0, 25, 25);
	simGroup->end();
	simGroup->resizable(spacer);

	curY += 30;
	*/
#if USE_OGRE_VIEWER < 1
	fltkViewer = new FltkViewer(10, curY, w - 20, h - (curY + 10), NULL);
#else
	fltkViewer = new FLTKOgreWindow(10, curY, w - 20, h - (curY + 10), NULL);	
#endif
	fltkViewer->box(FL_UP_BOX);

	this->end();

	this->resizable(fltkViewer);

	const boost::filesystem::path& curDir = boost::filesystem::current_path();
	scriptFolder = curDir.string();
	scriptFolder.append("/scripts");

	ReloadScriptsCB(NULL, this);

	characterCreator = NULL;

	resWindow = NULL;

	visemeViewerWindow = NULL;

	monitorConnectWindow = NULL;

	motionEditorWindow = NULL;

	retargetCreatorWindow = NULL;

	faceViewerWindow = NULL;
	bmlViewerWindow = NULL;
	dataViewerWindow = NULL;
	resourceWindow = NULL;
	panimationWindow = NULL;	
}

BaseWindow::~BaseWindow() {

	delete fltkViewer;
	if (commandWindow)
		delete commandWindow;
	if (characterCreator)
		delete characterCreator;
	if (visemeViewerWindow)
		delete visemeViewerWindow;
	if (monitorConnectWindow)
		delete monitorConnectWindow;
	if (motionEditorWindow)
		delete motionEditorWindow;
	if (retargetCreatorWindow)
		delete retargetCreatorWindow;
	if (faceViewerWindow)
		delete faceViewerWindow;
	if (bmlViewerWindow)
		delete bmlViewerWindow;
	if (bmlCreatorWindow)
		delete bmlCreatorWindow;
	if (dataViewerWindow)
		delete dataViewerWindow;
	if (resourceWindow)
		delete resourceWindow;
	if (panimationWindow)
		delete panimationWindow;

}


SbmCharacter* BaseWindow::getSelectedCharacter()
{
#if !NO_OGRE_VIEWER_CMD
	 SbmPawn* selectedPawn = fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	 if (!selectedPawn)
		 return NULL;

	 SbmCharacter* character = dynamic_cast<SbmCharacter*> (selectedPawn);
	 return character;
#else
	return NULL;
#endif
}

void BaseWindow::show_viewer()
{
	#if !defined (__ANDROID__) && !defined(SBM_IPHONE)
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SbmShaderManager::singleton().setViewer(mcu.viewer_p);
	#endif	
	show();
	fltkViewer->show_viewer();
}

void BaseWindow::hide_viewer()
{
	if (this->shown())
		this->hide();
}

void BaseWindow::set_camera ( const SrCamera* cam )
{
   fltkViewer->set_camera(cam);
}

SrCamera* BaseWindow::get_camera()
{
	return fltkViewer->get_camera();
}

void BaseWindow::render () 
{ 
	fltkViewer->redraw();
} 

void BaseWindow::root(SrSn* r)
{
	fltkViewer->root(r);
}

SrSn* BaseWindow::root()
{
	return fltkViewer->root();
}


void BaseWindow::resetWindow()
{
	if (commandWindow)
	{
		delete commandWindow;
		commandWindow = NULL;
	}
	if (bmlCreatorWindow)
	{
		delete bmlCreatorWindow;
		bmlCreatorWindow = NULL;
	}
	if (retargetCreatorWindow)
	{
		delete retargetCreatorWindow;
		retargetCreatorWindow = NULL;
	}
	if (visemeViewerWindow)
	{
		delete visemeViewerWindow;
		visemeViewerWindow = NULL;
	}
	if (monitorConnectWindow)
	{
		delete monitorConnectWindow;
		monitorConnectWindow = NULL;
	}
	if (motionEditorWindow)
	{
		delete motionEditorWindow;
		motionEditorWindow = NULL;
	}

	if (characterCreator)
	{
		delete characterCreator;
		characterCreator = NULL;
	}
	if (visemeViewerWindow)
	{
		delete visemeViewerWindow;
		visemeViewerWindow = NULL;
	}

	if (monitorConnectWindow)
	{
		delete monitorConnectWindow;
		monitorConnectWindow = NULL;
	}
	if (motionEditorWindow)
	{
		delete motionEditorWindow;
		motionEditorWindow = NULL;
	}

	if (faceViewerWindow)
	{
		delete faceViewerWindow;
		faceViewerWindow = NULL;
	}
	if (bmlViewerWindow)
	{
		delete bmlViewerWindow;
		bmlViewerWindow = NULL;
	}
	if (dataViewerWindow)
	{
		delete dataViewerWindow;
		dataViewerWindow = NULL;
	}
	if (resourceWindow)
	{
		delete resourceWindow;
		resourceWindow = NULL;
	}
	if (panimationWindow)
	{
		delete panimationWindow;
		panimationWindow = NULL;
	}

}

void BaseWindow::LoadCB(Fl_Widget* widget, void* data)
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* seqFile = fl_file_chooser("Load file:", "*.py", mediaPath.c_str());
	if (!seqFile)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.reset();

	if (mediaPath != "")
		SmartBody::SBScene::getScene()->setMediaPath(mediaPath);
	std::string filebasename = boost::filesystem::basename(seqFile);
	std::string fileextension = boost::filesystem::extension(seqFile);
	std::string fullfilename = std::string(seqFile);
	size_t pos = fullfilename.find(filebasename);
	std::string path = fullfilename.substr(0, pos - 1);
	mcu._scene->addAssetPath("script", path);
	mcu._scene->runScript(filebasename);
}

void BaseWindow::SaveCB(Fl_Widget* widget, void* data)
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* saveFile = fl_file_chooser("Save file:", "*.py", mediaPath.c_str());
	if (!saveFile)
		return;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SmartBody::SBScene* scene = mcu._scene;
	std::string fileString = scene->save();

	std::ofstream file(saveFile);
	if (!file.good())
	{
		std::string message = "Cannot save to file '";
		message.append(saveFile);
		message.append("'");
		fl_alert(message.c_str());
		file.close();
	}
	file << fileString;
	file.close();
	
	std::string scenePrompt = "Scene saved to file '";
	scenePrompt.append(saveFile);
	scenePrompt.append("'");
	fl_message(scenePrompt.c_str());
}

void BaseWindow::RunCB(Fl_Widget* widget, void* data)
{
}

void BaseWindow::LaunchBMLViewerCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->bmlViewerWindow)
	{
		rootWindow->bmlViewerWindow = new BehaviorWindow(rootWindow->x() + 50, rootWindow->y() + 50, 800, 600, "BML Viewer");
	}
	rootWindow->bmlViewerWindow->show();
}

void BaseWindow::LaunchParamAnimViewerCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->panimationWindow)
	{
		rootWindow->panimationWindow = new PanimationWindow(rootWindow->x() + 50, rootWindow->y() + 50, 800, 800, "Blend Viewer");
	}
	rootWindow->panimationWindow->show();
}

void BaseWindow::LaunchDataViewerCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->dataViewerWindow)
	{
		rootWindow->dataViewerWindow = new ChannelBufferWindow(rootWindow->x() + 50, rootWindow->y() + 50, 800, 600, "Blend Viewer");
	}
	rootWindow->dataViewerWindow->show();
}

void BaseWindow::LaunchResourceViewerCB( Fl_Widget* widget, void* data )
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->resourceWindow)
	{
		rootWindow->resourceWindow = new ResourceWindow(rootWindow->x() + 50, rootWindow->y() + 50, 800, 600, "Resource Viewer");
	}
	rootWindow->resourceWindow->show();
}

void BaseWindow::LaunchFaceViewerCB( Fl_Widget* widget, void* data )
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->faceViewerWindow)
	{
		rootWindow->faceViewerWindow = new FaceViewer(rootWindow->x() + 50, rootWindow->y() + 50, 800, 600, "Face Viewer");
	}
	rootWindow->faceViewerWindow->show();
}

void BaseWindow::LaunchSpeechRelayCB( Fl_Widget* widget, void* data )
{
	// run the speech relay launcher script
#ifdef WIN32
	system("start ..\\..\\..\\..\\bin\\TtsRelay\\bin\\x86\\Release\\TtsRelayGui.exe");
#else
	system("../../../../core/FestivalRelay/speechrelay.sh&");
#endif
}

void BaseWindow::NewCB(Fl_Widget* widget, void* data)
{
	int confirm = fl_choice("This will reset the current session.\nContinue?", "No", "Yes", NULL);
	if (confirm == 1)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		mcu.reset();
		std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");
		if (mediaPath != "")
			SmartBody::SBScene::getScene()->setMediaPath(mediaPath);
		
		CameraResetCB(widget, data);
	}
}

void BaseWindow::QuitCB(Fl_Widget* widget, void* data)
{
	int confirm = fl_choice("This will quit SmartBody.\nContinue?", "No", "Yes", NULL);
	if (confirm == 1)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		mcu.executePython("quit()");
	}
}

void BaseWindow::LaunchConnectCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->monitorConnectWindow)
	{
		rootWindow->monitorConnectWindow = new MonitorConnectWindow(150, 150, 320, 400, "Monitor Connect");
	}

	rootWindow->monitorConnectWindow->show();	
}

void BaseWindow::DisconnectRemoteCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu._scene->getDebuggerClient()->Disconnect();
	mcu.reset();
}

void BaseWindow::LaunchConsoleCB(Fl_Widget* widget, void* data)
{
	// console doesn't receive commands - why?
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->commandWindow)
	{
		rootWindow->commandWindow = new CommandWindow(150, 150, 640, 480, "Commands");
	}

	rootWindow->commandWindow->show();
}

void BaseWindow::LaunchBMLCreatorCB(Fl_Widget* widget, void* data)
{
	// console doesn't receive commands - why?
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->bmlCreatorWindow)
	{
		rootWindow->bmlCreatorWindow = new BMLCreatorWindow(150, 150, 800, 600, "BML Commands");
	}

	rootWindow->bmlCreatorWindow->show();
}

void BaseWindow::LaunchRetargetCreatorCB(Fl_Widget* widget, void* data)
{
	// console doesn't receive commands - why?
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->retargetCreatorWindow)
	{
		rootWindow->retargetCreatorWindow = new RetargetCreatorWindow(150, 150, 800, 600, "Retarget Creator");
	}
	rootWindow->retargetCreatorWindow->show();
}

// void BaseWindow::LaunchBehaviorSetsCB(Fl_Widget* widget, void* data)
// {
// 	// console doesn't receive commands - why?
// 	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
// 	if (!rootWindow->behaviorSetViewer)
// 	{
// 		rootWindow->behaviorSetViewer = new RetargetViewer(150, 150, 320, 520, "Behavior Sets");
// 	}
// 	rootWindow->behaviorSetViewer->show();
// }


void BaseWindow::LaunchJointMapViewerCB( Fl_Widget* widget, void* data )
{
	// console doesn't receive commands - why?
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer && !rootWindow->fltkViewer->_retargetStepWindow)
	{
		rootWindow->fltkViewer->_retargetStepWindow = new RetargetStepWindow(150, 150, 1024, 500, "Joint Map Viewer");
	}
	rootWindow->fltkViewer->_retargetStepWindow->setApplyType(false);
	rootWindow->fltkViewer->_retargetStepWindow->show();
}


void BaseWindow::LaunchMotionEditorCB(Fl_Widget* widget, void* data)
{
	// console doesn't receive commands - why?
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->motionEditorWindow)
	{
		rootWindow->motionEditorWindow = new MotionEditorWindow(150, 150, 450, 700, "Motion Editor");
	}
	rootWindow->motionEditorWindow->show();
}

void BaseWindow::LaunchVisemeViewerCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->visemeViewerWindow)
	{
		rootWindow->visemeViewerWindow = new VisemeViewerWindow(150, 150, 800, 600, "Viseme Configuration");
	}

	rootWindow->visemeViewerWindow->show();
}




void BaseWindow::StartCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void BaseWindow::StopCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void BaseWindow::StepCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	
}

void BaseWindow::PauseCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"time pause");
}

void BaseWindow::ResetCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"reset");
}

void BaseWindow::CameraResetCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"camera reset");
}

void BaseWindow::CameraFrameCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"camera frame");
}

void BaseWindow::RotateSelectedCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

#if !NO_OGRE_VIEWER_CMD
	SbmPawn* pawn = rootWindow->fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	if (!pawn)
	{
		pawn = rootWindow->getSelectedCharacter();
		if (!pawn)
			return;
	}

	SrCamera* camera = mcu.viewer_p->get_camera();
	float x,y,z,h,p,r;
	pawn->get_world_offset(x, y, z, h, p, r);
	camera->center = SrVec(x, y, z);
#endif
}


void BaseWindow::FaceCameraCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;
	
	// position the camera such that the character's face appears in the frame
	SrBox faceBox;
	SrCamera* camera = mcu.viewer_p->get_camera();

	SkSkeleton* skeleton = character->getSkeleton();
	float height = skeleton->getCurrentHeight();
	SkJoint* joint = skeleton->linear_search_joint("eyeball_left");
	SkJoint* joint2 = skeleton->linear_search_joint("eyeball_right");
	SkJoint* baseJoint = skeleton->linear_search_joint("base");

	skeleton->update_global_matrices();
	if (joint && joint2 && baseJoint)
	{
		joint->update_gmat();
		const SrMat& gmat = joint->gmat();
		SrVec point(gmat.get(3, 0), gmat.get(3, 1), gmat.get(3, 2));
		faceBox.extend(point);

		joint2->update_gmat();
		const SrMat& gmat2 = joint2->gmat();
		SrVec point2(gmat2.get(3, 0), gmat2.get(3, 1), gmat2.get(3, 2));
		faceBox.extend(point2);

		// get the facing vector of the character's body
		baseJoint->update_gmat();
		SkJointQuat* quat = baseJoint->quat();
		SrVec facing(.0f, .0f, 1.0f);
		SrVec facingVector = facing * quat->value();
		facingVector.y = .0f;
		facingVector.normalize();

		float max = faceBox.max_size();
		float min = faceBox.min_size();

		camera->center = (point + point2) / 2.0f;		
		camera->eye = camera->center + height / 4.0f * facingVector;
	}
	else
	{
		return;
	}
}

void BaseWindow::RunScriptCB(Fl_Widget* w, void* data)
{
	fl_alert("Not implemented");
	/*
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	// determine which script was selected
	Fl_Widget* widget = w;

	std::string filename = "";

	Fl_Menu_Bar* curWidget = widget;
	while (curWidget->parent() != rootWindow->menubar)
	{
		filename.insert(0, curWidget->label());
		filename.insert(0, "/");
		curWidget = curWidget->parent();
	}

	const boost::filesystem::path& scriptPath(rootWindow->scriptFolder);
	std::string scriptName = scriptPath.string();
	scriptName.append(filename);
	rootWindow->runScript(scriptName);
	*/
}

void BaseWindow::ReloadScriptsCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	std::string buff;
	const boost::filesystem::path& curDir = rootWindow->scriptFolder;
	buff.append(curDir.string());
	rootWindow->reloadScripts(buff);
}

void BaseWindow::SetScriptDirCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	const char* directory = fl_dir_chooser("Select the script folder:", rootWindow->scriptFolder.c_str());
	if (!directory)
		return;

	rootWindow->scriptFolder = directory;
}

void BaseWindow::runScript(std::string filename)
{
	std::ifstream file(filename.c_str());
	if (!file.good())
	{
		std::string message = "Filename '";
		message.append(filename);
		message.append("' is not a valid file.");
		fl_alert(message.c_str());
		file.close();
	}
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = getSelectedCharacter();
	std::string selectedCharacterName = "";
	if (character)
		selectedCharacterName = character->getName();
#if !NO_OGRE_VIEWER_CMD
	SbmPawn* pawn = fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	std::string selectedTargetName = "";
	if (pawn)
		selectedTargetName = pawn->getName();

	char line[8192];
	while(!file.eof() && file.good())
	{
		file.getline(line, 8192, '\n');
                // remove any trailing \r
                if (line[strlen(line) - 1] == '\r')
                        line[strlen(line) - 1] = '\0';
		if (strlen(line) == 0) // ignore blank lines
			continue;

		std::string lineStr = line;
		// process special handlers
		int inputPos = lineStr.find("$INPUT(");
		if (inputPos != std::string::npos)
		{
			// find the final ')'
			int parenPos = lineStr.find(")", inputPos);
			if (parenPos == std::string::npos)
				parenPos = lineStr.length() - 1;
			std::string text = lineStr.substr(inputPos + 7, parenPos - inputPos);
			const char* response = fl_input(text.c_str());
			std::string responseStr = "";
			if (response)
				responseStr = response;
			
			// reassemble the line
			std::string begin = lineStr.substr(0, inputPos);
			std::string end = lineStr.substr(parenPos);
			lineStr = begin;
			lineStr.append(response);
			lineStr.append(end);
		}

		// replace any variables
		boost::replace_all(lineStr, "$CHARACTER", selectedCharacterName);
		boost::replace_all(lineStr, "$TARGET", selectedTargetName);

		mcu.execute((char*)lineStr.c_str());
	}
	file.close();
#endif	
}

void BaseWindow::reloadScripts(std::string scriptsDir)
{
	//LOG("Not yet implemented");
	/*
	// erase the old scripts menu
	for (int x = menubar->size() - 1; x >= 0; x--)
	{
		if (strcmp( menubar->menu()[x].label(), "&Scripts") == 0)
		{
			menubar->value(clear();
		}
	}

	// create the new menu
	menubar->add("Scripts/Reload Scripts", 0, BaseWindow::ReloadScriptsCB, this, NULL);
	menubar->add("Scripts/Set Script Folder", 0, BaseWindow::SetScriptDirCB, this, FL_MENU_DIVIDER);
	reloadScriptsByDir(scriptsDir, "");
	*/
}

void BaseWindow::reloadScriptsByDir(std::string scriptsDir, std::string parentStr)
{
#ifdef WIN32
	// eliminate the current list
	char path[2048];
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;
	//if( !bCountHidden) dwAttr |= FILE_ATTRIBUTE_HIDDEN;
	sprintf( path, "%s/*", scriptsDir.c_str());
	HANDLE hFind = FindFirstFile( path, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			if( !(fd.dwFileAttributes & dwAttr))
			{
				// add the name to the root window
				char entry[512];
				sprintf(entry, "Scripts/%s%s", parentStr.c_str(), fd.cFileName);
				menubar->add(entry, 0, BaseWindow::RunScriptCB, this, 0);
			}
			else 
			{
				if (!strcmp(fd.cFileName, "..") == 0 && !strcmp(fd.cFileName, ".") == 0)
				{
					// recurse into this directory
					char newdir[1024];
					sprintf(newdir, "%s/%s", scriptsDir.c_str(), fd.cFileName);
					char newParentStr[1024];
					sprintf(newParentStr, "%s%s/", parentStr.c_str(), fd.cFileName);

					std::string nextDir = newParentStr;
					reloadScriptsByDir(newdir, nextDir);
				}
			}
		} while( FindNextFile( hFind, &fd));
		FindClose( hFind);
	}
#else
	char buff[8192];
	//danceInterp::getDirectoryListing(buff, 8192, (char*) scriptsDir);
	char* token = strtok(buff, " ");
	char scriptName[512];
	std::vector<std::string> allentries;
	while (token != NULL)
	{
		std::string s = token;
		allentries.push_back(s);
		token = strtok(NULL, " ");
	}

	for (unsigned int x = 0; x < allentries.size(); x++)
	{
		char absfilename[2048];
		sprintf(absfilename, "%s%s", scriptsDir.c_str(), allentries[x].c_str());

#ifdef __APPL__
		if (!filename_isdir(absfilename))
		{

			if (filename_match(allentries[x].c_str(), "*.py"))
			{
				strncpy(scriptName, allentries[x].c_str(),  strlen(allentries[x].c_str()) - 3);
				scriptName[strlen(allentries[x].c_str()) - 3] = '\0';
				// add the name to the root window
				char entry[512];
				sprintf(entry, "Scripts/%s%s", parentStr.c_str(), scriptName);
				menubar->add(entry, 0, BaseWindow::RunScriptCB, 0, 0);
			}
		}
		else
		{
			if (!strcmp(allentries[x].c_str(), "..") == 0 && !strcmp(allentries[x].c_str(),  ".") == 0)
			{
				// recurse into this directory
				char newdir[1024];
				sprintf(newdir, "%s/%s", scriptsDir.c_str(), allentries[x].c_str());
				char newParentStr[1024];
				sprintf(newParentStr, "%s%s/", parentStr.c_str(), allentries[x].c_str());
				reloadScriptsByDir(newdir, newParentStr);
			}
		}
#endif

	}

#endif
}

void BaseWindow::ModeBonesCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowBones, NULL);
#endif
}

void BaseWindow::ModeGeometryCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowGeometry, NULL);
#endif
}

void BaseWindow::ModeCollisionGeometryCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowCollisionGeometry, NULL);	
#endif
}

void BaseWindow::ModeDeformableGeometryCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometry, NULL);
#endif
}

void BaseWindow::ModeGPUDeformableGeometryCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometryGPU, NULL);
#endif
}

void BaseWindow::ModeAxisCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowAxis, NULL);
#endif
}

void BaseWindow::ModeEyebeamsCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	if (rootWindow->fltkViewer->getData()->eyeBeamMode)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeBeams, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeBeams, NULL);
#endif
}

void BaseWindow::ModeEyelidCalibrationCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->eyeLidMode == FltkViewer::ModeNoEyeLids)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeLids, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeLids, NULL);
#endif
}

void BaseWindow::ShowSelectedCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowSelection, NULL);
#endif
}

void BaseWindow::ShadowsCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->shadowmode == FltkViewer::ModeNoShadows)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShadows, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoShadows, NULL);
#endif
}

void BaseWindow::TerrainShadedCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrain, NULL);
#endif
}

void BaseWindow::TerrainWireframeCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrainWireframe)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrainWireframe, NULL);
#endif
}
void BaseWindow::TerrainNoneCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeNoTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoTerrain, NULL);
#endif
}

void BaseWindow::ShowPawns(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->pawnmode != FltkViewer::ModePawnShowAsSpheres)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdPawnShowAsSpheres, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoPawns, NULL);
#endif
}

void BaseWindow::ModeDynamicsCOMCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOM)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOM, NULL);
#endif
}

void BaseWindow::ModeDynamicsSupportPolygonCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOMSupportPolygon)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOMSupportPolygon, NULL);
#endif
}

void BaseWindow::ModeDynamicsMassesCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowMasses)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowMasses, NULL);
#endif
}


void BaseWindow::ShowBoundingVolumeCB( Fl_Widget* w, void* data )
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowBoundingVolume, NULL);
}

void BaseWindow::SettingsDefaultMediaPathCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string path = scene->getSystemParameter("mediapath");

	const char* result = fl_input("Default Media Path", path.c_str());
	if (result)
	{
		scene->setSystemParameter("mediapath", result);
	}
}

void BaseWindow::AudioCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SmartBody::SBScene* scene = mcu._scene;
	const bool val = scene->getBoolAttribute("internalAudio");
	scene->setBoolAttribute("internalAudio", !val);
}

void BaseWindow::CreateCharacterCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	// get a list of existing skeletons
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> skeletons;
	for (std::map<std::string, SkSkeleton*>::iterator iter = mcu.skeleton_map.begin(); 
		 iter != mcu.skeleton_map.end();
		 iter++)
	{
		skeletons.push_back((*iter).first);
	}

	if (!rootWindow->characterCreator)
		rootWindow->characterCreator = new CharacterCreatorWindow(rootWindow->x() + 20, rootWindow->y() + 20, 480, 150, strdup("Create a Character"));

	rootWindow->characterCreator->setSkeletons(skeletons);

	rootWindow->characterCreator->show();
}

void BaseWindow::CreatePawnCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->create_pawn();
#endif
}

void BaseWindow::CreateLightCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	int highestLightNum = 0;
	std::vector<std::string> pawnNames = scene->getPawnNames();
	for (std::vector<std::string>::iterator iter =  pawnNames.begin();
		 iter != pawnNames.end();
		 iter++)
	{
		std::string& pawnName = (*iter);
		if (pawnName.find("light") == 0)
		{
			std::string lightNumStr = pawnName.substr(5, pawnName.size());
			int lightNum = atoi(lightNumStr.c_str());
			if (lightNum >= highestLightNum)
				highestLightNum = lightNum + 1;
		}

	}
	std::stringstream strstr;
	strstr << "light = scene.createPawn(\"light" << highestLightNum << "\")\n";
	strstr << "light.createBoolAttribute(\"lightIsDirectional\", True, True, \"LightParameters\", 200, False, False, False, \"Is the light directional?\")\n";
	strstr << "light.createVec3Attribute(\"lightDiffuseColor\", 1, .95, .8, True, \"LightParameters\", 210, False, False, False, \" Diffuse light color\")\n";
	strstr << "light.createVec3Attribute(\"lightAmbientColor\", 0, 0, 0, True, \"LightParameters\", 220, False, False, False, \" Ambient light color\")\n";
	strstr << "light.createVec3Attribute(\"lightSpecularColor\", 0, 0, 0, True, \"LightParameters\", 230, False, False, False, \"Specular light color\")\n";
	strstr << "light.createDoubleAttribute(\"lightSpotExponent\", 0, True, \"LightParameters\", 240, False, False, False, \" Spotlight exponent.\")\n";
	strstr << "light.createVec3Attribute(\"lightSpotDirection\", 0, 0, -1, True, \"LightParameters\", 250, False, False, False, \"Spotlight direction\")\n";
	strstr << "light.createDoubleAttribute(\"lightSpotCutoff\", 180, True, \"LightParameters\", 260, False, False, False, \"Spotlight cutoff angle\")\n";
	strstr << "light.createDoubleAttribute(\"lightConstantAttenuation\", 1, True, \"LightParameters\", 270, False, False, False, \"Constant attenuation\")\n";
	strstr << "light.createDoubleAttribute(\"lightLinearAttenuation\", 1, True, \"LightParameters\", 280, False, False, False, \" Linear attenuation.\")\n";
	strstr << "light.createDoubleAttribute(\"lightQuadraticAttenuation\", 0, True, \"LightParameters\", 290, False, False, False, \"Quadratic attenuation\")\n";
	scene->run(strstr.str());
#endif
}

void BaseWindow::CreateTerrainCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	const char* terrainFile = fl_file_chooser("Load terrain:", "*.ppm", NULL);
	if (terrainFile)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		std::string terrainCommand = "terrain load ";
		terrainCommand.append(terrainFile);
		mcu.execute((char*)terrainCommand.c_str());
		if (mcu.height_field_p)
		{
			mcu.height_field_p->set_scale( 5000.0f, 300.0f, 5000.0f );
			mcu.height_field_p->set_auto_origin();
		}
	}
}

void BaseWindow::TrackCharacterCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (mcu.cameraTracking.size() > 0)
	{
		// if any tracks are active, remove them
		mcu.execute((char*)"camera track");
		return;
	}

	// track the selected character
	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;

	if (!character->getSkeleton())
		return;

	SkJoint* joint = character->getSkeleton()->joints()[0];
	std::string trackCommand = "camera track ";
	trackCommand.append(character->getName());
	trackCommand.append(" ");
	trackCommand.append(joint->name());

	mcu.execute((char*)trackCommand.c_str());
}

void BaseWindow::KinematicFootstepsCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowKinematicFootprints, NULL);
#endif
}

void BaseWindow::TrajectoryCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowTrajectory, NULL);	
#endif
}

void BaseWindow::GestureCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowGesture, NULL);	
#endif
}

void BaseWindow::JointLabelCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowJoints, NULL);	
#endif
}

void BaseWindow::SteeringCharactersCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerCharactersGoalsOnly, NULL);
#endif
}

void BaseWindow::SteeringAllCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerAll, NULL);
#endif
}

void BaseWindow::SteeringNoneCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoSteer, NULL);	
#endif
}

void BaseWindow::LocomotionFootstepsCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowLocomotionFootprints, NULL);
#endif
}

void BaseWindow::VelocityCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowVelocity, NULL);
#endif
}

void BaseWindow::GridCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->gridMode != FltkViewer::ModeShowGrid)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdGrid, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoGrid, NULL);
#endif
}

void BaseWindow::ShowPoseExamples( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->reachRenderMode != FltkViewer::ModeShowExamples)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdReachShowExamples, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdReachNoExamples, NULL);
#endif
}

void BaseWindow::CreatePythonAPICB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	const char* docFile = fl_file_chooser("Save documentation to:", "*.html", NULL);
	if (!docFile)
		return;

	std::stringstream strstr;
	strstr << "from pydoc import *\n";
	strstr << "d = HTMLDoc()\n";
	strstr << "content = d.docmodule(sys.modules[\"SmartBody\"])\n";
	strstr << "import io\n";
	strstr << "f = io.open('" << docFile << "', 'w')\n";
	strstr << "f.write(unicode(content))\n";
	strstr << "f.close()\n";

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->run(strstr.str());
}

void BaseWindow::DocumentationCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	std::stringstream strstr;
	strstr << "import webbrowser\n";
	strstr << "url = \"http://smartbody.ict.usc.edu/documentation\"\n";
	strstr << "webbrowser.open(url)\n";

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->run(strstr.str());
}

void BaseWindow::ResizeWindowCB(Fl_Widget* widget, void* data)
{

	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	Fl_Choice* resChoice = static_cast<Fl_Choice*>(widget);

	int windowIndex = (int) data;	
	
	std::vector<std::string> tokens;	
	std::string resStr = resChoice->text();
	if (resStr == "Default")
	{
		rootWindow->resize(rootWindow->x(),rootWindow->y(),800,800);
		return;
	}
	else if (resStr == "Custom...")
	{
		if (!rootWindow->resWindow)
		{
			rootWindow->resWindow = new ResolutionWindow(rootWindow->x() + 20, rootWindow->y() + 20, 480, 150, strdup("Change Resolution"));
			rootWindow->resWindow->baseWin = rootWindow;
		}
		rootWindow->resWindow->show();
		return;
	}

	vhcl::Tokenize(resStr, tokens, "x");
	if (tokens.size() < 2)
		return;

	int width = atoi(tokens[0].c_str());
	int height = atoi(tokens[1].c_str());
	//std::cout << width << " " << height << std::endl;	
	rootWindow->resize(rootWindow->x(),rootWindow->y(),width+20,height+70);
// 	rootWindow->w(width);
// 	rootWindow->h(height);

}

void BaseWindow::SaveCameraCB( Fl_Widget* widget, void* data )
{
	BaseWindow* window = (BaseWindow*)data;	
	SrCamera* camera = new SrCamera(window->get_camera());
	window->cameraList.push_back(camera);
	window->updateCameraList();
}

void BaseWindow::DeleteCameraCB( Fl_Widget* widget, void* data )
{

}

void BaseWindow::ChooseCameraCB( Fl_Widget* widget, void* data )
{
	BaseWindow* window = (BaseWindow*)data;	
	Fl_Choice* choice = (Fl_Choice*)widget;
	int cameraIdx = choice->value() - 1;	
	if (cameraIdx >=0 && cameraIdx < (int)window->cameraList.size())
	{
		SrCamera* cam = window->cameraList[cameraIdx];		
		//window->set_camera(cam);
		window->get_camera()->copyCamera(cam);
	}
}

void BaseWindow::updateCameraList()
{
	cameraChoice->clear();
	cameraChoice->add("-----");
	for (unsigned int i=0;i<cameraList.size();i++)
	{
		std::string cameraName = "cam";
		cameraName += boost::lexical_cast<std::string>(i);
		cameraChoice->add(cameraName.c_str());
	}
}

//== Viewer Factory ========================================================
SrViewer* FltkViewerFactory::s_viewer = NULL;

FltkViewerFactory::FltkViewerFactory()
{
	s_viewer = NULL;
}

SrViewer* FltkViewerFactory::create(int x, int y, int w, int h)
{
	if (!s_viewer)
		s_viewer = new BaseWindow(x, y, w, h, "SmartBody");
	return s_viewer;
}

void FltkViewerFactory::remove(SrViewer* viewer)
{
	if (viewer && (viewer == s_viewer))
	{
		viewer->hide_viewer();
		BaseWindow* baseWindow = dynamic_cast<BaseWindow*> (s_viewer);
		if (baseWindow)
		{
			baseWindow->resetWindow();
			baseWindow->render();
		}
	}
}

void FltkViewerFactory::reset(SrViewer* viewer)
{
	if (viewer && (viewer == s_viewer))
	{
		BaseWindow* baseWindow = dynamic_cast<BaseWindow*> (s_viewer);
		if (baseWindow)
		{
			baseWindow->resetWindow();
			baseWindow->render();
		}
	}
}


