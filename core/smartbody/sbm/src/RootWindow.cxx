
#include "RootWindow.h"

#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <sstream>
#include <FL/filename.H>
#include "sbm/mcontrol_util.h"
#include "boost/filesystem.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "sbm/sbm_audio.h"
#include <fstream>
#include "CommandWindow.h"



BaseWindow::BaseWindow(int x, int y, int w, int h, const char* name) : SrViewer(x, y, w, h), Fl_Double_Window(x, y, w, h, name)
{
	commandWindow = NULL;
	bmlCreatorWindow = NULL;
	this->begin();

	menubar = new Fl_Menu_Bar(0, 0, w, 30); 
	menubar->labelsize(10);
/*	menubar->add("&File/Load...", 0, BaseWindow::LoadCB, 0, NULL);
	menubar->add("&File/Save Configuration...", 0, NULL, 0, NULL);
	menubar->add("&File/Run Script...", 0, NULL, 0, NULL);
*/	menubar->add("&View/Character/Bones", 0, ModeBonesCB, this, NULL);
	menubar->add("&View/Character/Geometry", 0, ModeGeometryCB, this, NULL);
	menubar->add("&View/Character/Collision Geometry", 0, ModeCollisionGeometryCB, this, NULL);
	menubar->add("&View/Character/Deformable Geometry", 0, ModeDeformableGeometryCB, this, NULL);
	menubar->add("&View/Character/GPU Deformable Geometry", 0, ModeGPUDeformableGeometryCB, this, NULL);
	menubar->add("&View/Character/Axis", 0, ModeAxisCB, this, NULL);
	menubar->add("&View/Character/Show Selected", 0, ShowSelectedCB, this, NULL);
	menubar->add("&View/Character/Eyebeams", 0, ModeEyebeamsCB, this, NULL);
	menubar->add("&View/Character/Eyelid calibration", 0, ModeEyelidCalibrationCB, this, NULL);
	menubar->add("&View/Character/Dynamics/COM", 0, ModeDynamicsCOMCB, this, NULL);
	menubar->add("&View/Character/Dynamics/Support Polygon", 0, ModeDynamicsSupportPolygonCB, this, NULL);
	menubar->add("&View/Character/Dynamics/Masses", 0, ModeDynamicsMassesCB, this, NULL);
	menubar->add("&View/Character/Locomotion/Kinematic Footsteps", 0, KinematicFootstepsCB, this, NULL);
	menubar->add("&View/Character/Locomotion/Locomotion Footsteps", 0, LocomotionFootstepsCB, this, NULL);
	menubar->add("&View/Character/Locomotion/Velocity", 0, VelocityCB, this, NULL);
	menubar->add("&View/Character/Locomotion/Trajectory", 0, TrajectoryCB, this, NULL);
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
	menubar->add("&Create/Character...", 0, CreateCharacterCB, this, NULL);
	menubar->add("&Create/Pawn...", 0, CreatePawnCB, this, NULL);
	menubar->add("&Create/Terrain...", 0, CreateTerrainCB, this, NULL);
	menubar->add("&Camera/Reset", 0, CameraResetCB, this, NULL);
	menubar->add("&Camera/Frame All", 0, CameraFrameCB, this, NULL);
	menubar->add("&Camera/Face Camera", 0, FaceCameraCB, this, NULL);
	menubar->add("&Camera/Track Character", 0, TrackCharacterCB, this, NULL);
	menubar->add("&Camera/Rotate Around Selected", 0, RotateSelectedCB, this, NULL);
//	menubar->add("&Settings/Softeyes", 0, SettingsSofteyesToggleCB, this, NULL);
	menubar->add("&Settings/Internal Audio", 0, AudioCB, this, NULL);
	menubar->add("&Window/Data Viewer", 0, LaunchDataViewerCB,this, NULL);
	menubar->add("&Window/BML Viewer", 0, LaunchBMLViewerCB, this, NULL);
	menubar->add("&Window/Parameterized Animation Viewer", 0, LaunchParamAnimViewerCB, this, NULL);
	menubar->add("&Window/Resource Viewer", 0, LaunchResourceViewerCB, this, NULL);
	menubar->add("&Window/Command Window", 0, LaunchConsoleCB, this, NULL);
	menubar->add("&Window/BML Creator", 0, LaunchBMLCreatorCB, this, NULL);
	menubar->add("&Window/Face Viewer", 0, LaunchFaceViewerCB, this, NULL);
	menubar->add("&Window/Speech Relay", 0, LaunchSpeechRelayCB, this, NULL);
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

	fltkViewer = new FltkViewer(10, curY, w - 20, h - (curY + 10), NULL);
	fltkViewer->box(FL_UP_BOX);

	this->end();

	this->resizable(fltkViewer);

	const boost::filesystem::path& curDir = boost::filesystem::current_path();
	scriptFolder = curDir.string();
	scriptFolder.append("/scripts");

	ReloadScriptsCB(NULL, this);

	characterCreator = NULL;

}

BaseWindow::~BaseWindow() {
	delete fltkViewer;
	if (commandWindow)
		delete commandWindow;
}


SbmCharacter* BaseWindow::getSelectedCharacter()
{
	 SbmPawn* selectedPawn = fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	 if (!selectedPawn)
		 return NULL;

	 SbmCharacter* character = dynamic_cast<SbmCharacter*> (selectedPawn);
	 return character;
}

void BaseWindow::show_viewer()
{
	#if !defined (__ANDROID__) && !defined(SBM_IPHONE)
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SbmShaderManager::singleton().setViewer(mcu.viewer_p);
	#endif
	show();
}

void BaseWindow::hide_viewer()
{
	if (this->shown())
		this->hide();
}

void BaseWindow::set_camera ( const SrCamera &cam )
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

void BaseWindow::LoadCB(Fl_Widget* widget, void* data)
{
	int confirm = fl_choice("This will reset the current session.\nContinue?", "yes", "no", NULL);
	if (!confirm)
		return;

	const char* seqFile = fl_file_chooser("Load file:", "*.seq", NULL);
	if (!seqFile)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"reset");
}

void BaseWindow::SaveCB(Fl_Widget* widget, void* data)
{
}

void BaseWindow::RunCB(Fl_Widget* widget, void* data)
{
}

void BaseWindow::LaunchBMLViewerCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"bmlviewer open");
	mcu.execute((char*)"bmlviewer show");
}

void BaseWindow::LaunchParamAnimViewerCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"panimviewer open");
	mcu.execute((char*)"panimviewer show");	
}

void BaseWindow::LaunchDataViewerCB(Fl_Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"cbufviewer open");
	mcu.execute((char*)"cbufviewer show");
}

void BaseWindow::LaunchResourceViewerCB( Fl_Widget* widget, void* data )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"resourceviewer open");
	mcu.execute((char*)"resourceviewer show");	
}

void BaseWindow::LaunchFaceViewerCB( Fl_Widget* widget, void* data )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*)"faceviewer open");
	mcu.execute((char*)"faceviewer show");	
}

void BaseWindow::LaunchSpeechRelayCB( Fl_Widget* widget, void* data )
{
	// run the speech relay launcher script
#ifdef WIN32
	system("start ..\\..\\..\\..\\core\\TtsRelay\\bin\\x86\\Release\\TtsRelayGui.exe");
#else
	system("../../../../core/FestivalRelay/speechrelay.sh&");
#endif
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
	SkJoint* joint = skeleton->linear_search_joint("skullbase");
	SkJoint* joint2 = skeleton->linear_search_joint("face_top_parent");
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
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowBones, NULL);
}

void BaseWindow::ModeGeometryCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowGeometry, NULL);
}

void BaseWindow::ModeCollisionGeometryCB( Fl_Widget* w, void* data )
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowCollisionGeometry, NULL);	
}

void BaseWindow::ModeDeformableGeometryCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometry, NULL);
}

void BaseWindow::ModeGPUDeformableGeometryCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometryGPU, NULL);
}

void BaseWindow::ModeAxisCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowAxis, NULL);
}

void BaseWindow::ModeEyebeamsCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	if (rootWindow->fltkViewer->getData()->eyeBeamMode)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeBeams, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeBeams, NULL);
}

void BaseWindow::ModeEyelidCalibrationCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->eyeLidMode == FltkViewer::ModeNoEyeLids)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeLids, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeLids, NULL);
}

void BaseWindow::ShowSelectedCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);

	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowSelection, NULL);
}

void BaseWindow::ShadowsCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->shadowmode == FltkViewer::ModeNoShadows)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShadows, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoShadows, NULL);
}

void BaseWindow::TerrainShadedCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrain, NULL);
}

void BaseWindow::TerrainWireframeCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrainWireframe)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrainWireframe, NULL);
}
void BaseWindow::TerrainNoneCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeNoTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoTerrain, NULL);
}

void BaseWindow::ShowPawns(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->pawnmode != FltkViewer::ModePawnShowAsSpheres)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdPawnShowAsSpheres, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoPawns, NULL);
}

void BaseWindow::ModeDynamicsCOMCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOM)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOM, NULL);
}

void BaseWindow::ModeDynamicsSupportPolygonCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOMSupportPolygon)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOMSupportPolygon, NULL);
}

void BaseWindow::ModeDynamicsMassesCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowMasses)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowMasses, NULL);
}


void BaseWindow::SettingsSofteyesToggleCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	bool currentSetting = true;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		currentSetting = character->isSoftEyes();
	}
}

void BaseWindow::AudioCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (mcu.play_internal_audio)
	{
		mcu.play_internal_audio = false;
		AUDIO_Close();
	}
	else
	{
		mcu.play_internal_audio = true;
		AUDIO_Init();
	}
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
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->create_pawn();
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
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowKinematicFootprints, NULL);
}

void BaseWindow::TrajectoryCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowTrajectory, NULL);	
}

void BaseWindow::SteeringCharactersCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerCharactersGoalsOnly, NULL);	
}

void BaseWindow::SteeringAllCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerAll, NULL);	
}

void BaseWindow::SteeringNoneCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoSteer, NULL);	
}

void BaseWindow::LocomotionFootstepsCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowLocomotionFootprints, NULL);
}

void BaseWindow::VelocityCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowVelocity, NULL);
}

void BaseWindow::GridCB(Fl_Widget* w, void* data)
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->gridMode != FltkViewer::ModeShowGrid)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdGrid, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoGrid, NULL);
}

void BaseWindow::ShowPoseExamples( Fl_Widget* w, void* data )
{
	BaseWindow* rootWindow = static_cast<BaseWindow*>(data);
	if (rootWindow->fltkViewer->getData()->reachRenderMode != FltkViewer::ModeShowExamples)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdReachShowExamples, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdReachNoExamples, NULL);
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
	}
}


