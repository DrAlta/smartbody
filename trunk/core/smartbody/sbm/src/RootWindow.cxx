#include <vhcl.h>
#include <sbm/GPU/SbmShader.h>
#include "RootWindow.h"
#include "CharacterCreatorWindow.h"
#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <sstream>
#include <FL/filename.H>
#include "boost/filesystem.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include "sbm/sbm_audio.h"
#include <fstream>
#include "CommandWindow.h"
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBDebuggerClient.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBVHMsgManager.h>
#include <sbm/Heightfield.h>
#include <sbm/KinectProcessor.h>
#include <sb/SBPython.h>

BaseWindow::BaseWindow(int x, int y, int w, int h, const char* name) : SrViewer(x, y, w, h), Fl_Double_Window(x, y, w, h, name)
{
	commandWindow = NULL;
	bmlCreatorWindow = NULL;
	this->begin();

	menubar = new Fl_Menu_Bar(0, 0, w, 30); 
	menubar->labelsize(10);
	menubar->add("&File/New", 0, NewCB, this, NULL);	
	menubar->add("&File/Save", 0, SaveCB, this, NULL);	
	menubar->add("&File/Export...", 0, ExportCB, this, NULL);	
	menubar->add("&File/Load...", 0, LoadCB, this, NULL);
	menubar->add("&File/Save Scene Setting", 0, SaveSceneSettingCB, this, NULL);	
	menubar->add("&File/Load Scene Setting...", 0, LoadSceneSettingCB, this, NULL);	
    menubar->add("&File/Quick Connect", 0, QuickConnectCB, this, NULL);
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
	menubar->add("&View/Character/Gaze Limits", 0, ModeGazeLimitCB, this, NULL);
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
	menubar->add("&View/Show Cameras", 0, ShowCamerasCB, this, NULL);
	menubar->add("&View/Show Lights", 0, ShowLightsCB, this, NULL);
	menubar->add("&View/Shadows", 0, ShadowsCB, this, NULL);
	menubar->add("&View/Grid", 0, GridCB, this, NULL);
	menubar->add("&View/Background Color", 0, BackgroundColorCB, this, NULL);
	menubar->add("&View/Floor/Show Floor", 0, FloorCB, this, NULL);
	menubar->add("&View/Floor/Floor Color", 0, FloorColorCB, this, NULL);
	//menubar->add("&View/Reach Pose Examples", 0, ShowPoseExamples, this, NULL);	
	menubar->add("&View/Terrain/Shaded", 0, TerrainShadedCB, this, NULL);
	menubar->add("&View/Terrain/Wireframe", 0, TerrainWireframeCB, this, NULL);
	menubar->add("&View/Terrain/No Terrain", 0, TerrainNoneCB, this, NULL);	
	menubar->add("&View/NavigationMesh/NoMesh", 0, NavigationMeshNoneCB, this, NULL);
	menubar->add("&View/NavigationMesh/Show RawMesh", 0, NavigationMeshRawMeshCB, this, NULL);
	menubar->add("&View/NavigationMesh/Show NaviMesh", 0, NavigationMeshNaviMeshCB, this, NULL);	
	menubar->add("&View/Steer/Characters and Goals", 0, SteeringCharactersCB, this, NULL);
	menubar->add("&View/Steer/All Steering", 0, SteeringAllCB, this, NULL);
	menubar->add("&View/Steer/No Steering", 0, SteeringNoneCB, this, NULL);


	menubar->add("&Create/Character...", 0, CreateCharacterCB, this, NULL);
	menubar->add("&Create/Pawn...", 0, CreatePawnCB, this, NULL);
	menubar->add("&Create/Light...", 0, CreateLightCB, this, NULL);
	menubar->add("&Create/Camera...", 0, CreateCameraCB, this, NULL);
	//menubar->add("&Create/Terrain...", 0, CreateTerrainCB, this, NULL); // should replace it with create navigation mesh.
	
	setResolutionMenuIndex = menubar->add("&Settings/Set Resolution", 0, 0, 0, FL_SUBMENU_POINTER);
	menubar->add("&Settings/Default Media Path", 0, SettingsDefaultMediaPathCB, this, NULL);
	menubar->add("&Settings/Internal Audio", 0, AudioCB, this, NULL);	

	menubar->add("&Camera/Save Camera View", 0, SaveCameraCB, this, NULL );
	loadCameraMenuIndex = menubar->add("&Camera/Load Camera", 0, 0, 0, FL_SUBMENU_POINTER );
	deleteCameraMenuIndex = menubar->add("&Camera/Delete Camera", 0, 0, 0, FL_SUBMENU_POINTER );
	menubar->add("&Camera/Reset", 0, CameraResetCB, this, NULL);
	menubar->add("&Camera/Frame All", 0, CameraFrameCB, this, NULL);
	menubar->add("&Camera/Face Camera", 0, FaceCameraCB, this, NULL);
	menubar->add("&Camera/Track Character", 0, TrackCharacterCB, this, NULL);
	menubar->add("&Camera/Rotate Around Selected", 0, RotateSelectedCB, this, NULL);	
   menubar->add("&Camera/Modes/Default", 0, SetDefaultCamera, this, NULL);	
   menubar->add("&Camera/Modes/Free Look", 0, SetFreeLookCamera, this, NULL);	
   menubar->add("&Camera/Modes/Follow Renderer", 0, SetFollowRendererCamera, this, NULL);	
	

	menubar->add("&Window/Data Viewer", 0, LaunchDataViewerCB,this, NULL);
//	menubar->add("&Window/BML Viewer", 0, LaunchBMLViewerCB, this, NULL);
	menubar->add("&Window/Blend Viewer", 0, LaunchParamAnimViewerCB, this, NULL);
	menubar->add("&Window/Resource Viewer", 0, LaunchResourceViewerCB, this, NULL);
	menubar->add("&Window/Command Window", 0, LaunchConsoleCB, this, NULL);
	menubar->add("&Window/BML Creator", 0, LaunchBMLCreatorCB, this, NULL);
	menubar->add("&Window/Face Viewer", 0, LaunchFaceViewerCB, this, NULL);
	menubar->add("&Window/Lip Sync Viewer", 0, LaunchVisemeViewerCB, this, NULL);
	//menubar->add("&Window/Retarget Creator", 0, LaunchRetargetCreatorCB, this, NULL);
	//menubar->add("&Window/Behavior Sets", 0, LaunchBehaviorSetsCB, this, NULL);
	menubar->add("&Window/Motion Editor", 0, LaunchMotionEditorCB, this, NULL);
	menubar->add("&Window/Retarget Viewer", 0, LaunchJointMapViewerCB, this, NULL);
	menubar->add("&Window/Speech Relay", 0, LaunchSpeechRelayCB, this, NULL);
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

	
	int curY= 2;
	//Fl_Group* cameraGroup = new Fl_Group(10, curY, w, 25, NULL);	
	//cameraGroup->type(Fl_Pack::HORIZONTAL);
 	
// 
// 	cameraChoice = new Fl_Choice(curX, curY, 80, 25, "Camera");
// 	cameraChoice->when(FL_WHEN_NOT_CHANGED|FL_WHEN_CHANGED);
// 	cameraChoice->callback(ChooseCameraCB, this);
// 	updateCameraList();	
// 	cameraChoice->value(0);
// 	curX += 85;
// 	saveCamera = new Fl_Button(curX, curY, 45, 25, "Save");
// 	saveCamera->callback(SaveCameraCB, this);
// 
  	int curX = 500;
// 	deleteCamera = new Fl_Button(curX, curY, 45, 25, "Del");
// 	deleteCamera->callback(DeleteCameraCB, this);			
	windowSizes.push_back("640x360");
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

	resolutionMenuList.clear();
	Fl_Menu_Item defaultItem = {"Default", 0, ResizeWindowCB, this};
	resolutionMenuList.push_back(defaultItem);
	for (unsigned int i=0;i<windowSizes.size();i++)
	{
		Fl_Menu_Item resItem = { windowSizes[i].c_str(), 0, ResizeWindowCB, this } ;
		resolutionMenuList.push_back(resItem);
	}

	Fl_Menu_Item customItem = {"Custom...", 0, ResizeWindowCB, this};
	Fl_Menu_Item tempItem = {0};
	resolutionMenuList.push_back(customItem);
	resolutionMenuList.push_back(tempItem);
	
	Fl_Menu_Item* menuList = const_cast<Fl_Menu_Item*>(menubar->menu());
	Fl_Menu_Item& resolutionSubMenu = menuList[setResolutionMenuIndex];
	resolutionSubMenu.user_data(&resolutionMenuList[0]);

	//cameraGroup->end();	

	curY += 28;

	cameraCount = 0;

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
	exportWindow = NULL;
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
		SbmShaderManager::singleton().setViewer(this);
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
	//fltkViewer->root(r);
}

SrSn* BaseWindow::root()
{
	return SmartBody::SBScene::getScene()->getRootGroup();
}


void BaseWindow::resetWindow()
{
	if (commandWindow)
	{
		commandWindow->hide();
		delete commandWindow;
		commandWindow = NULL;
	}
	if (bmlCreatorWindow)
	{
		bmlCreatorWindow->hide();
		delete bmlCreatorWindow;
		bmlCreatorWindow = NULL;
	}
	if (retargetCreatorWindow)
	{
		retargetCreatorWindow->hide();
		delete retargetCreatorWindow;
		retargetCreatorWindow = NULL;
	}

	if (fltkViewer->_retargetStepWindow)
	{
		fltkViewer->_retargetStepWindow->hide();
		delete fltkViewer->_retargetStepWindow;
		fltkViewer->_retargetStepWindow = NULL;
	}

	if (visemeViewerWindow)
	{
		visemeViewerWindow->hide();
		delete visemeViewerWindow;
		visemeViewerWindow = NULL;
	}
	if (monitorConnectWindow)
	{
		monitorConnectWindow->hide();
		delete monitorConnectWindow;
		monitorConnectWindow = NULL;
	}
	if (motionEditorWindow)
	{
		motionEditorWindow->hide();
		delete motionEditorWindow;
		motionEditorWindow = NULL;
	}

	if (characterCreator)
	{
		characterCreator->hide();
		delete characterCreator;
		characterCreator = NULL;
	}
	if (visemeViewerWindow)
	{
		visemeViewerWindow->hide();
		delete visemeViewerWindow;
		visemeViewerWindow = NULL;
	}

	if (faceViewerWindow)
	{
		faceViewerWindow->hide();
		delete faceViewerWindow;
		faceViewerWindow = NULL;
	}
	if (bmlViewerWindow)
	{
		bmlViewerWindow->hide();
		delete bmlViewerWindow;
		bmlViewerWindow = NULL;
	}
	if (dataViewerWindow)
	{
		dataViewerWindow->hide();
		delete dataViewerWindow;
		dataViewerWindow = NULL;
	}
	if (resourceWindow)
	{
		resourceWindow->hide();
		delete resourceWindow;
		resourceWindow = NULL;
	}
	if (panimationWindow)
	{
		panimationWindow->hide();
		delete panimationWindow;
		panimationWindow = NULL;
	}
	if (exportWindow)
	{
		exportWindow->hide();
		delete exportWindow;
		exportWindow = NULL;
	}
}

void BaseWindow::ResetScene()
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");
	resetWindow();

	SmartBody::SBCharacterListener* listener = SmartBody::SBScene::getScene()->getCharacterListener();
	SmartBody::SBScene::destroyScene();

	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->setCharacterListener(listener);

	SmartBody::SBScene::getScene()->setViewer(this);
	SmartBody::SBScene::getScene()->getViewer()->root(SmartBody::SBScene::getScene()->getRootGroup());
	SbmShaderManager::singleton().setViewer(this);

	scene->getSimulationManager()->setupTimer();

	SrCamera* camera = SmartBody::SBScene::getScene()->createCamera("cameraDefault");
	camera->reset();

	std::string pythonLibPath = SmartBody::SBScene::getSystemParameter("pythonlibpath");
	setupPython();

	if (mediaPath != "")
		SmartBody::SBScene::getScene()->setMediaPath(mediaPath);

	scene->getVHMsgManager()->setEnable(true);	
}

void BaseWindow::LoadCB(Fl_Widget* widget, void* data)
{
	BaseWindow* window = (BaseWindow*) data;
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* seqFile = fl_file_chooser("Load file:", "*.py", mediaPath.c_str());
	if (!seqFile)
		return;

   window->ResetScene();

	std::string filebasename = boost::filesystem::basename(seqFile);
	std::string fileextension = boost::filesystem::extension(seqFile);
	std::string fullfilename = std::string(seqFile);
	size_t pos = fullfilename.find(filebasename);
	std::string path = fullfilename.substr(0, pos - 1);
	SmartBody::SBScene::getScene()->addAssetPath("script", path);
	SmartBody::SBScene::getScene()->runScript(filebasename);
}

void BaseWindow::SaveCB(Fl_Widget* widget, void* data)
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* saveFile = fl_file_chooser("Save file:", "*.py", mediaPath.c_str());
	if (!saveFile)
		return;
	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
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

void BaseWindow::ExportCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->exportWindow)
	{
		rootWindow->exportWindow = new ExportWindow(rootWindow->x() + 50, rootWindow->y() + 50, 300, 600, "Export");
	}
	rootWindow->exportWindow->show();
}

void BaseWindow::SaveSceneSettingCB( Fl_Widget* widget, void* data )
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* saveFile = fl_file_chooser("Save file:", "*.py", mediaPath.c_str());
	if (!saveFile)
		return;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string fileString = scene->saveSceneSetting();

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

void BaseWindow::LoadSceneSettingCB( Fl_Widget* widget, void* data )
{
	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");

	const char* seqFile = fl_file_chooser("Load file:", "*.py", mediaPath.c_str());
	if (!seqFile)
		return;

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	if (mediaPath != "")
		scene->setMediaPath(mediaPath);
	std::string filebasename = boost::filesystem::basename(seqFile);
	std::string fileextension = boost::filesystem::extension(seqFile);
	std::string fullfilename = std::string(seqFile);
	size_t pos = fullfilename.find(filebasename);
	std::string path = fullfilename.substr(0, pos - 1);
	scene->addAssetPath("script", path);
	scene->runScript(filebasename);

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
		rootWindow->bmlViewerWindow->show_viewer();
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
		rootWindow->dataViewerWindow = new ChannelBufferWindow(rootWindow->x() + 50, rootWindow->y() + 50, 800, 600, "Data Viewer");
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
	BaseWindow* window = (BaseWindow*) data;
	int confirm = fl_choice("This will reset the current session.\nContinue?", "No", "Yes", NULL);
	if (confirm == 1)
	{
		SmartBody::SBCharacterListener* listener = SmartBody::SBScene::getScene()->getCharacterListener();
		window->resetWindow();
		SmartBody::SBScene::destroyScene();

		SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
		scene->setViewer(window);
		scene->getViewer()->root(scene->getRootGroup());
		//mcu.kinectProcessor->initKinectSkeleton();
		SbmShaderManager::singleton().setViewer(window);


		std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");
		if (mediaPath != "")
			scene->setMediaPath(mediaPath);
		scene->setCharacterListener(listener);

		scene->getSimulationManager()->setupTimer();
		
		SrCamera* camera = scene->createCamera("cameraDefault");
		camera->reset(); 

		std::string pythonLibPath = SmartBody::SBScene::getSystemParameter("pythonlibpath");
		setupPython();
		

		scene->getVHMsgManager()->setEnable(true);	
	}
}

void BaseWindow::QuitCB(Fl_Widget* widget, void* data)
{
	int confirm = fl_choice("This will quit SmartBody.\nContinue?", "No", "Yes", NULL);
	if (confirm == 1)
	{
		SmartBody::SBScene::getScene()->run("quit()");
	}
}

void BaseWindow::QuickConnectCB(Fl_Widget* widget, void* data)
{
   BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->monitorConnectWindow)
	{
		rootWindow->monitorConnectWindow = new MonitorConnectWindow(150, 150, 320, 400, "Monitor Connect", true);
	}
}

void BaseWindow::LaunchConnectCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (!rootWindow->monitorConnectWindow)
	{
		rootWindow->monitorConnectWindow = new MonitorConnectWindow(150, 150, 320, 400, "Monitor Connect", false);
	}

	rootWindow->monitorConnectWindow->show();	
}

void BaseWindow::DisconnectRemoteCB(Fl_Widget* widget, void* data)
{
   BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	SmartBody::SBScene::getScene()->getDebuggerClient()->Disconnect();
	rootWindow->ResetScene();
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
		rootWindow->motionEditorWindow = new MotionEditorWindow(150, 150, 425, 725, "Motion Editor");
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
}

void BaseWindow::StopCB(Fl_Widget* widget, void* data)
{
}

void BaseWindow::StepCB(Fl_Widget* widget, void* data)
{

}

void BaseWindow::PauseCB(Fl_Widget* widget, void* data)
{
	SmartBody::SBScene::getScene()->command((char*)"time pause");
}

void BaseWindow::ResetCB(Fl_Widget* widget, void* data)
{
	SmartBody::SBScene::getScene()->command((char*)"reset");
	BaseWindow* window = (BaseWindow*) data;
	window->resetWindow();
}

void BaseWindow::CameraResetCB(Fl_Widget* widget, void* data)
{
	SrCamera* camera = SmartBody::SBScene::getScene()->getActiveCamera();
	if (!camera)
		return;
	camera->reset();
}

void BaseWindow::CameraFrameCB(Fl_Widget* widget, void* data)
{
	//SmartBody::SBScene::getScene()->command((char*)"camera frame");
	SrBox sceneBox;
	SrCamera* camera = SmartBody::SBScene::getScene()->getActiveCamera();
	if (!camera) return;

	const std::vector<std::string>& pawnNames =  SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		iter != pawnNames.end();
		iter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(*iter);
		bool visible = pawn->getBoolAttribute("visible");
		if (!visible)
			continue;
		SrBox box = pawn->getSkeleton()->getBoundingBox();
		sceneBox.extend(box);
	}
	camera->view_all(sceneBox, camera->getFov());	
	float scale = 1.f/SmartBody::SBScene::getScene()->getScale();
	float znear = 0.01f*scale;
	float zfar = 100.0f*scale;
	camera->setNearPlane(znear);
	camera->setFarPlane(zfar);
}

void BaseWindow::RotateSelectedCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	
#if !NO_OGRE_VIEWER_CMD
	SbmPawn* pawn = rootWindow->fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	if (!pawn)
	{
		pawn = rootWindow->getSelectedCharacter();
		if (!pawn)
			return;
	}

	SrCamera* camera = SmartBody::SBScene::getScene()->getActiveCamera();
	if (!camera)
		return;
	float x,y,z,h,p,r;
	pawn->get_world_offset(x, y, z, h, p, r);
	camera->setCenter(x, y, z);
#endif
}

void BaseWindow::SetDefaultCamera(Fl_Widget* widget, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->getData()->cameraMode = FltkViewer::Default;
   SmartBody::SBScene::getScene()->SetCameraLocked(false);
#endif
}

void BaseWindow::SetFreeLookCamera(Fl_Widget* widget, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->getData()->cameraMode = FltkViewer::FreeLook;
   SmartBody::SBScene::getScene()->SetCameraLocked(false);
#endif
}

void BaseWindow::SetFollowRendererCamera(Fl_Widget* widget, void* data)
{
#if !NO_OGRE_VIEWER_CMD
   if (SmartBody::SBScene::getScene()->isRemoteMode())
   {
      BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	   rootWindow->fltkViewer->getData()->cameraMode = FltkViewer::FollowRenderer;
      SmartBody::SBScene::getScene()->SetCameraLocked(true);
   }
#endif
}

void BaseWindow::FaceCameraCB(Fl_Widget* widget, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	
	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;
	
	// position the camera such that the character's face appears in the frame
	SrBox faceBox;
	SrCamera* camera = SmartBody::SBScene::getScene()->getActiveCamera();
	if (!camera)
		return;

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

		SrVec tmpCenter = (point + point2) / 2.0f;
		camera->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
		SrVec tmp = camera->getCenter() + height / 4.0f * facingVector;
		camera->setEye(tmp.x, tmp.y, tmp.z);
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

		SmartBody::SBScene::getScene()->command((char*)lineStr.c_str());
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

void BaseWindow::ModeGazeLimitCB(Fl_Widget* w, void* data)
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	if (rootWindow->fltkViewer->getData()->gazeLimitMode)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoGazeLimit, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdGazeLimit, NULL);
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

void BaseWindow::FloorCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->getData()->showFloor = !rootWindow->fltkViewer->getData()->showFloor;
#endif
}

void BaseWindow::FloorColorCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdFloorColor, NULL);	
#endif

}

void BaseWindow::BackgroundColorCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdBackground, NULL);	
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

void BaseWindow::NavigationMeshNaviMeshCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->navigationMeshMode != FltkViewer::ModeNavigationMesh)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNavigationMesh, NULL);
#endif
}

void BaseWindow::NavigationMeshRawMeshCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->navigationMeshMode != FltkViewer::ModeRawMesh)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdRawMesh, NULL);
#endif

}

void BaseWindow::NavigationMeshNoneCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->navigationMeshMode != FltkViewer::ModeNoNavigationMesh)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoNavigationMesh, NULL);
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
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const bool val = scene->getBoolAttribute("internalAudio");
	scene->setBoolAttribute("internalAudio", !val);
}

void BaseWindow::CreateCharacterCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	std::vector<std::string> skeletonNames = SmartBody::SBScene::getScene()->getSkeletonNames();
	
	if (!rootWindow->characterCreator)
		rootWindow->characterCreator = new CharacterCreatorWindow(rootWindow->x() + 20, rootWindow->y() + 20, 480, 150, strdup("Create a Character"));
	
	
	std::string characterName = "char" + boost::lexical_cast<std::string>(rootWindow->characterCreator->numCharacter);
	rootWindow->characterCreator->inputName->value(characterName.c_str());
	rootWindow->characterCreator->setSkeletons(skeletonNames);
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
	const std::vector<std::string>& pawnNames = scene->getPawnNames();
	for (std::vector<std::string>::const_iterator iter =  pawnNames.begin();
		 iter != pawnNames.end();
		 iter++)
	{
		const std::string& pawnName = (*iter);
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
	strstr << "light.setBoolAttribute(\"visible\", false)\n";
	scene->run(strstr.str());
#endif
}

void BaseWindow::CreateCameraCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	std::string cameraName = "camera";
	cameraName += boost::lexical_cast<std::string>(rootWindow->cameraCount++);
	const char* userCamName = fl_input("Camera name:", cameraName.c_str());
	if (!userCamName)
	{
		rootWindow->cameraCount--;
		return;
	}

	std::string cameraNameStr = userCamName;
	SrCamera* camera = SmartBody::SBScene::getScene()->createCamera(cameraNameStr);

	if (!camera)
	{
		fl_alert("Camera with name '%s' cannot be created.", cameraNameStr.c_str());
		return;
	}
	float scale = 1.f/SmartBody::SBScene::getScene()->getScale();
	SrVec camEye = SrVec(0,1.66f,1.85f)*scale;
	SrVec camCenter = SrVec(0,0.92f,0)*scale;	
	float znear = 0.01f*scale;
	float zfar = 100.0f*scale;
	camera->setEye(camEye[0],camEye[1],camEye[2]);
	camera->setCenter(camCenter[0],camCenter[1],camCenter[2]);
	camera->setNearPlane(znear);
	camera->setFarPlane(zfar);

	if (!SmartBody::SBScene::getScene()->getActiveCamera())
		SmartBody::SBScene::getScene()->setActiveCamera(camera);
}

void BaseWindow::CreateTerrainCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	const char* terrainFile = fl_file_chooser("Load terrain:", "*.ppm", NULL);
	if (terrainFile)
	{
		
		std::string terrainCommand = "terrain load ";
		terrainCommand.append(terrainFile);
		SmartBody::SBScene::getScene()->command((char*)terrainCommand.c_str());
		if (SmartBody::SBScene::getScene()->getHeightfield())
		{
			SmartBody::SBScene::getScene()->getHeightfield()->set_scale( 5000.0f, 300.0f, 5000.0f );
			SmartBody::SBScene::getScene()->getHeightfield()->set_auto_origin();
		}
	}
}

void BaseWindow::TrackCharacterCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	SmartBody::SBScene::getScene()->removeCameraTrack();

	// track the selected character
	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;

	if (!character->getSkeleton())
		return;

	SkJoint* joint = character->getSkeleton()->joints()[0];

// 	std::string trackCommand = "camera track ";
// 	trackCommand.append(character->getName());
// 	trackCommand.append(" ");
// 	trackCommand.append(joint->name());
	//SmartBody::SBScene::getScene()->command((char*)trackCommand.c_str());	
	SmartBody::SBScene::getScene()->setCameraTrack(character->getName(), joint->jointName());
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

	size_t windowIndex = (size_t) data;	
	
	std::vector<std::string> tokens;	
	const Fl_Menu_Item* menuItem = ((Fl_Menu_*)widget)->mvalue();
	std::string resStr = menuItem->label();
	if (resStr == "Default")
	{
		rootWindow->resize(rootWindow->x(),rootWindow->y(),800,600);
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
	
	std::string cameraName = "camera";
	cameraName += boost::lexical_cast<std::string>(window->cameraCount++);
	const char* userCamName = fl_input("Camera name:", cameraName.c_str());
	if (!userCamName)
	{
		window->cameraCount--;
		return;
	}

	std::string cameraNameStr = userCamName;
	SrCamera* camera = SmartBody::SBScene::getScene()->createCamera(cameraNameStr);
	if (!camera)
	{
		fl_alert("Camera with name '%s' cannot be created.", cameraNameStr.c_str());
		return;
	}

	camera->copyCamera(SmartBody::SBScene::getScene()->getActiveCamera());
	
	window->updateCameraList();
	//window->cameraList.push_back(camera);
	//window->updateCameraList();
}

void BaseWindow::DeleteCameraCB( Fl_Widget* widget, void* data )
{
	const Fl_Menu_Item* menuItem = ((Fl_Menu_*)widget)->mvalue();
	std::string camName = menuItem->label();
	BaseWindow* window = (BaseWindow*)data;	
	SrCamera* camera = SmartBody::SBScene::getScene()->getCamera(camName);
	if (!camera)
	{
		fl_alert("No camera named '%s' found, cannot remove it.", camName.c_str());
		return;
	}
	SmartBody::SBScene::getScene()->removeCamera(camera);
	window->updateCameraList();
}

void BaseWindow::ChooseCameraCB( Fl_Widget* widget, void* data )
{
	const Fl_Menu_Item* menuItem = ((Fl_Menu_*)widget)->mvalue();
	//LOG("load camera %s", menuItem->label());
	std::string camName = menuItem->label();
 	BaseWindow* window = (BaseWindow*)data;	
	SrCamera* cam = SmartBody::SBScene::getScene()->getCamera(camName);
	if (!cam)
	{
		fl_alert("No camera with name '%s' found.", camName.c_str());
		return;
	}
	
	SmartBody::SBScene::getScene()->setActiveCamera(cam);
// 	Fl_Choice* choice = (Fl_Choice*)widget;
// 	int cameraIdx = choice->value() - 1;	
// 	if (cameraIdx >=0 && cameraIdx < (int)window->cameraList.size())
// 	{
// 		SrCamera* cam = window->cameraList[cameraIdx];		
// 		//window->set_camera(cam);
// 		window->get_camera()->copyCamera(cam);
// 	}
}

void BaseWindow::updateCameraList()
{
	/*
	cameraChoice->clear();
	cameraChoice->add("-----");
	for (unsigned int i=0;i<cameraList.size();i++)
	{
		std::string cameraName = "cam";
		cameraName += boost::lexical_cast<std::string>(i);
		cameraChoice->add(cameraName.c_str());
	}
	*/
	Fl_Menu_Item* menuList = const_cast<Fl_Menu_Item*>(menubar->menu());
	Fl_Menu_Item& loadCameraSubMenu = menuList[loadCameraMenuIndex];	
	Fl_Menu_Item& deleteCameraSubMenu = menuList[deleteCameraMenuIndex];	
	loadCameraList.clear();
	deleteCameraList.clear();
	std::vector<std::string> cameraNames = SmartBody::SBScene::getScene()->getCameraNames();
	for (std::vector<std::string>::iterator iter = cameraNames.begin();
		  iter != cameraNames.end();
		  iter++)
	{		
		const std::string camName = (*iter);
		Fl_Menu_Item temp_LoadCam = { strdup(camName.c_str()), 0, ChooseCameraCB, this };		
		loadCameraList.push_back(temp_LoadCam);		

		Fl_Menu_Item temp_DeleteCam = { strdup(camName.c_str()), 0, DeleteCameraCB, this };		
		deleteCameraList.push_back(temp_DeleteCam);		
	}
	Fl_Menu_Item temp = {0};
	loadCameraList.push_back(temp);
	deleteCameraList.push_back(temp);

	loadCameraSubMenu.user_data(&loadCameraList[0]);
	deleteCameraSubMenu.user_data(&deleteCameraList[0]);
}

void BaseWindow::ShowCamerasCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->getData()->showCameras = !rootWindow->fltkViewer->getData()->showCameras;
	bool showCamera = rootWindow->fltkViewer->getData()->showCameras;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> camNames = scene->getCameraNames();
	for (unsigned int i=0;i<camNames.size();i++)
	{
		SrCamera* cam = scene->getCamera(camNames[i]);
		if (cam)
		{
			cam->setBoolAttribute("visible", showCamera);
		}
	}
#endif

}

void BaseWindow::ShowLightsCB( Fl_Widget* w, void* data )
{
#if !NO_OGRE_VIEWER_CMD
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->getData()->showLights = !rootWindow->fltkViewer->getData()->showLights;
	bool showLight = rootWindow->fltkViewer->getData()->showLights;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const std::vector<std::string>& pawnNames = scene->getPawnNames();
	for (unsigned int i=0;i<pawnNames.size();i++)
	{
		std::string name = pawnNames[i];
		SmartBody::SBPawn* pawn = scene->getPawn(name);
		if (name.find("light") == 0 && pawn) // is light
		{
			pawn->setBoolAttribute("visible",showLight);			
		}
	}
#endif

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


