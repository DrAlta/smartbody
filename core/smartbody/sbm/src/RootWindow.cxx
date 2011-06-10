
#include "vhcl.h"
#include "RootWindow.h"

#include <fltk/PackedGroup.h>
#include <fltk/AlignGroup.h>
#include <fltk/BarGroup.h>
#include <fltk/ask.h>
#include <fltk/file_chooser.h>
#include <sstream>
#include <fltk/filename.h>
#include "sbm/mcontrol_util.h"
#include "boost/filesystem.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "sbm/sbm_audio.h"
#include <fstream>
#include "CommandWindow.h"

using namespace fltk;

RootWindow::RootWindow(int x, int y, int w, int h, const char* name) : SrViewer(x, y, w, h), DoubleBufferWindow(x, y, w, h, name)
{
	commandWindow = NULL;
	this->begin();

	menubar = new MenuBar(0, 0, w, 30); 
	menubar->add("&File/Load...", 0, RootWindow::LoadCB, 0, NULL);
	menubar->add("&File/Save Configuration...", 0, NULL, 0, NULL);
	menubar->add("&File/Run Script...", 0, NULL, 0, NULL);
	menubar->add("&View/Character/Bones", 0, ModeBonesCB, this, NULL);
	menubar->add("&View/Character/Geometry", 0, ModeGeometryCB, this, NULL);
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
	menubar->add("&View/Grid/Toggle", 0, GridCB, this, NULL);
	menubar->add("&View/Grid/Grid Size", 0, GridSizeCB, this, NULL);
	menubar->add("&View/Grid/Grid Step", 0, GridStepCB, this, NULL);
	menubar->add("&View/Grid/Grid Height", 0, GridHeightCB, this, NULL);
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
	menubar->add("&Settings/Softeyes", 0, SettingsSofteyesToggleCB, this, NULL);
	menubar->add("&Settings/Internal Audio", 0, AudioCB, this, NULL);
	menubar->add("&Window/Data Viewer", 0, LaunchDataViewerCB,this, NULL);
	menubar->add("&Window/BML Viewer", 0, LaunchBMLViewerCB, this, NULL);
	menubar->add("&Window/Parameterized Animation Viewer", 0, LaunchParamAnimViewerCB, this, NULL);
	menubar->add("&Window/Command Window", 0, LaunchConsoleCB, this, NULL);
	menubar->add("&Scripts/Reload Scripts", 0, ReloadScriptsCB, this, NULL);
	menubar->add("&Scripts/Set Script Folder", 0, SetScriptDirCB, this, MENU_DIVIDER);

	// disable the commands that are not yet functional
	fltk::Group* fileMenuOption = dynamic_cast<fltk::Group*>(menubar->child(0));
	if (fileMenuOption)
	{
		for (int c = 0; c < fileMenuOption->children(); c++)
		{
			fileMenuOption->child(c)->deactivate();
		}
	}

	
	int curY= 30;

	/*
	fltk::PackedGroup* simGroup = new fltk::PackedGroup(10, curY, 75, 25, NULL);
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
	fltk::Output* spacer = new fltk::Output(curX, 0, 25, 25);
	simGroup->end();
	simGroup->resizable(spacer);

	curY += 30;
	*/

	fltkViewer = new FltkViewer(10, curY, w - 20, h - (curY + 10), NULL);
	fltkViewer->box(fltk::UP_BOX);

	this->end();

	this->resizable(fltkViewer);

	const boost::filesystem::path& curDir = boost::filesystem::current_path();
	scriptFolder = curDir.string();
	scriptFolder.append("/scripts");

	ReloadScriptsCB(NULL, this);

	characterCreator = NULL;

}

RootWindow::~RootWindow() {
	delete fltkViewer;
	if (commandWindow)
		delete commandWindow;
}


SbmCharacter* RootWindow::getSelectedCharacter()
{
	LocomotionData* locoData = fltkViewer->getLocomotionData();
	int charIndex = locoData->char_index;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.character_map.reset();
	int index = 0;
	while (SbmCharacter* character = mcu.character_map.next())
	{
		if (index == charIndex)
			return character;
		index++;
	}
	
	return NULL;
}

void RootWindow::show_viewer()
{
	show();
}

void RootWindow::hide_viewer()
{
	if (this->shown())
		this->hide();
}

void RootWindow::set_camera ( const SrCamera &cam )
{
   fltkViewer->set_camera(cam);

}


SrCamera* RootWindow::get_camera()
{
	return fltkViewer->get_camera();
}

void RootWindow::render () 
{ 
	redraw(); 
} 

void RootWindow::root(SrSn* r)
{
	fltkViewer->root(r);
}

SrSn* RootWindow::root()
{
	return fltkViewer->root();
}

void RootWindow::LoadCB(Widget* widget, void* data)
{
	int confirm = fltk::ask("This will reset the current session.\nContinue?");
	if (!confirm)
		return;

	const char* seqFile = fltk::file_chooser("Load file:", "*.seq", NULL);
	if (!seqFile)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("reset");

}

void RootWindow::SaveCB(Widget* widget, void* data)
{
}

void RootWindow::RunCB(Widget* widget, void* data)
{
}

void RootWindow::LaunchBMLViewerCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("bmlviewer open");
	mcu.execute("bmlviewer show");
}

void RootWindow::LaunchParamAnimViewerCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("panimviewer open");
	mcu.execute("panimviewer show");	
}

void RootWindow::LaunchDataViewerCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("cbufviewer open");
	mcu.execute("cbufviewer show");
}

void RootWindow::LaunchConsoleCB(Widget* widget, void* data)
{
	// console doesn't receive commands - why?
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (!rootWindow->commandWindow)
	{
		rootWindow->commandWindow = new CommandWindow(150, 150, 640, 480, "Commands");
		vhcl::Log::g_log.AddListener(rootWindow->commandWindow);
	}

	rootWindow->commandWindow->show();
}

void RootWindow::StartCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void RootWindow::StopCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void RootWindow::StepCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	
}

void RootWindow::PauseCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("time pause");
}

void RootWindow::ResetCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("reset");
}

void RootWindow::CameraResetCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("camera reset");
}

void RootWindow::CameraFrameCB(Widget* widget, void* data)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute("camera frame");
}

void RootWindow::RotateSelectedCB(Widget* widget, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmPawn* pawn = rootWindow->fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	if (!pawn)
		return;

	SrCamera* camera = mcu.viewer_p->get_camera();
	float x,y,z,h,p,r;
	pawn->get_world_offset(x, y, z, h, p, r);
	camera->center = SrVec(x, y, z);
}


void RootWindow::FaceCameraCB(Widget* widget, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;
	
	// position the camera such that the character's face appears in the frame
	SrBox faceBox;
	SrCamera* camera = mcu.viewer_p->get_camera();
	mcu.pawn_map.reset();

	SkSkeleton* skeleton = character->skeleton_p;
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

void RootWindow::RunScriptCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	// determine which script was selected
	Widget* widget = w;

	std::string filename = "";

	Widget* curWidget = widget;
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
}

void RootWindow::ReloadScriptsCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	std::string buff;
	const boost::filesystem::path& curDir = rootWindow->scriptFolder;
	buff.append(curDir.string());
	rootWindow->reloadScripts(buff);
}

void RootWindow::SetScriptDirCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	const char* directory = fltk::dir_chooser("Select the script folder:", rootWindow->scriptFolder.c_str());
	if (!directory)
		return;

	rootWindow->scriptFolder = directory;
}

void RootWindow::runScript(std::string filename)
{
	std::ifstream file(filename.c_str());
	if (!file.good())
	{
		std::string message = "Filename '";
		message.append(filename);
		message.append("' is not a valid file.");
		fltk::alert(message.c_str());
		file.close();
	}
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = getSelectedCharacter();
	std::string selectedCharacterName = "";
	if (character)
		selectedCharacterName = character->name;

	SbmPawn* pawn = fltkViewer->getObjectManipulationHandle().get_selected_pawn();
	std::string selectedTargetName = "";
	if (pawn)
		selectedTargetName = pawn->name;

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
			const char* response = fltk::input(text.c_str());
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

void RootWindow::reloadScripts(std::string scriptsDir)
{
	// erase the old scripts menu
	for (int x = menubar->children() - 1; x >= 0; x--)
	{
		Widget* widget = menubar->child(x);
		if (strcmp(widget->label(), "&Scripts") == 0)
		{
			Group* group = (Group*) widget;
			for (int x = group->children() - 1; x >= 0; x--)
				group->remove(x);
		}
	}

	// create the new menu
	menubar->add("Scripts/Reload Scripts", 0, RootWindow::ReloadScriptsCB, this, NULL);
	menubar->add("Scripts/Set Script Folder", 0, RootWindow::SetScriptDirCB, this, MENU_DIVIDER);
	reloadScriptsByDir(scriptsDir, "");
}

void RootWindow::reloadScriptsByDir(std::string scriptsDir, std::string parentStr)
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
				menubar->add(entry, 0, RootWindow::RunScriptCB, this, 0);
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
	danceInterp::getDirectoryListing(buff, 8192, (char*) scriptsDir);
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
		sprintf(absfilename, "%s%s", scriptsDir, allentries[x].c_str());

		if (!filename_isdir(absfilename))
		{

			if (filename_match(allentries[x].c_str(), "*.py"))
			{
				strncpy(scriptName, allentries[x].c_str(),  strlen(allentries[x].c_str()) - 3);
				scriptName[strlen(allentries[x].c_str()) - 3] = '\0';
				// add the name to the root window
				char entry[512];
				sprintf(entry, "Scripts/%s%s", parentStr, scriptName);
				dance::rootWindow->menubar->add(entry, 0, RootWindow::runUserScript_cb, 0, 0);
			}
		}
		else
		{
			if (!strcmp(allentries[x].c_str(), "..") == 0 && !strcmp(allentries[x].c_str(),  ".") == 0)
			{
				// recurse into this directory
				char newdir[1024];
				sprintf(newdir, "%s/%s", scriptsDir, allentries[x].c_str());
				char newParentStr[1024];
				sprintf(newParentStr, "%s%s/", parentStr, allentries[x].c_str());
				reloadScriptsByDir(newdir, newParentStr);
			}
		}

	}

#endif
}

void RootWindow::ModeBonesCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowBones, NULL);
}

void RootWindow::ModeGeometryCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowGeometry, NULL);
}

void RootWindow::ModeDeformableGeometryCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometry, NULL);
}

void RootWindow::ModeGPUDeformableGeometryCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowDeformableGeometryGPU, NULL);
}

void RootWindow::ModeAxisCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdCharacterShowAxis, NULL);
}

void RootWindow::ModeEyebeamsCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	if (rootWindow->fltkViewer->getData()->eyeBeamMode)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeBeams, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeBeams, NULL);
}

void RootWindow::ModeEyelidCalibrationCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->eyeLidMode == FltkViewer::ModeNoEyeLids)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdEyeLids, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoEyeLids, NULL);
}

void RootWindow::ShowSelectedCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowSelection, NULL);
}

void RootWindow::ShadowsCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->shadowmode == FltkViewer::ModeNoShadows)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShadows, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoShadows, NULL);
}

void RootWindow::TerrainShadedCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrain, NULL);
}

void RootWindow::TerrainWireframeCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeTerrainWireframe)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdTerrainWireframe, NULL);
}
void RootWindow::TerrainNoneCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->terrainMode != FltkViewer::ModeNoTerrain)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoTerrain, NULL);
}

void RootWindow::ShowPawns(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->pawnmode != FltkViewer::ModePawnShowAsSpheres)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdPawnShowAsSpheres, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoPawns, NULL);
}

void RootWindow::ModeDynamicsCOMCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOM)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOM, NULL);
}

void RootWindow::ModeDynamicsSupportPolygonCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowCOMSupportPolygon)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowCOMSupportPolygon, NULL);
}

void RootWindow::ModeDynamicsMassesCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->dynamicsMode != FltkViewer::ModeShowMasses)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowMasses, NULL);
}


void RootWindow::SettingsSofteyesToggleCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	bool currentSetting = true;
	mcu.character_map.reset();
	while (SbmCharacter* character = mcu.character_map.next())
	{
		currentSetting = character->isSoftEyes();
	}
}

void RootWindow::AudioCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
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

void RootWindow::CreateCharacterCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
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
		rootWindow->characterCreator = new CharacterCreatorWindow(rootWindow->x() + 20, rootWindow->y() + 20, 400, 450, "Create a Character");

	rootWindow->characterCreator->setSkeletons(skeletons);

	rootWindow->characterCreator->show();
}

void RootWindow::CreatePawnCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->create_pawn();
}

void RootWindow::CreateTerrainCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	const char* terrainFile = fltk::file_chooser("Load terrain:", "*.ppm", NULL);
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

void RootWindow::TrackCharacterCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (mcu.cameraTracking.size() > 0)
	{
		// if any tracks are active, remove them
		mcu.execute("camera track");
		return;
	}

	// track the selected character
	SbmCharacter* character = rootWindow->getSelectedCharacter();
	if (!character)
		return;

	if (!character->skeleton_p)
		return;

	SkJoint* joint = character->skeleton_p->joints()[0];
	std::string trackCommand = "camera track ";
	trackCommand.append(character->name);
	trackCommand.append(" ");
	trackCommand.append(joint->name());

	mcu.execute((char*)trackCommand.c_str());
}

void RootWindow::KinematicFootstepsCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowKinematicFootprints, NULL);
}

void RootWindow::TrajectoryCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowTrajectory, NULL);	
}

void RootWindow::SteeringCharactersCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerCharactersGoalsOnly, NULL);	
}

void RootWindow::SteeringAllCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdSteerAll, NULL);	
}

void RootWindow::SteeringNoneCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoSteer, NULL);	
}

void RootWindow::LocomotionFootstepsCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowLocomotionFootprints, NULL);
}

void RootWindow::VelocityCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdShowVelocity, NULL);
}

void RootWindow::GridCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
	if (rootWindow->fltkViewer->getData()->gridMode != FltkViewer::ModeShowGrid)
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdGrid, NULL);
	else
		rootWindow->fltkViewer->menu_cmd(FltkViewer::CmdNoGrid, NULL);
}

void RootWindow::GridSizeCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	std::stringstream strstr;
	strstr << rootWindow->fltkViewer->gridSize;
	const char* gridSizeStr = fltk::input("Grid Size", strstr.str().c_str());
	float gsize = (float) atof(gridSizeStr);
	rootWindow->fltkViewer->gridSize = gsize;
	glDeleteLists(rootWindow->fltkViewer->gridList, 1);
	rootWindow->fltkViewer->gridList = -1;
	rootWindow->fltkViewer->redraw();
}

void RootWindow::GridStepCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	std::stringstream strstr;
	strstr << rootWindow->fltkViewer->gridStep;
	const char* gridStepStr = fltk::input("Grid Step", strstr.str().c_str());
	float gstep = (float) atoi(gridStepStr);
	rootWindow->fltkViewer->gridStep = gstep;
	glDeleteLists(rootWindow->fltkViewer->gridList, 1);
	rootWindow->fltkViewer->gridList = -1;
	rootWindow->fltkViewer->redraw();
}

void RootWindow::GridHeightCB(fltk::Widget* w, void* data)
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);

	std::stringstream strstr;
	strstr << rootWindow->fltkViewer->gridHeight;
	const char* gridHeightStr = fltk::input("Grid Height", strstr.str().c_str());
	float gheight = (float) atoi(gridHeightStr);
	rootWindow->fltkViewer->gridHeight = gheight;
	glDeleteLists(rootWindow->fltkViewer->gridList, 1);
	rootWindow->fltkViewer->gridList = -1;
	rootWindow->fltkViewer->redraw();
}


void RootWindow::ShowPoseExamples( fltk::Widget* w, void* data )
{
	RootWindow* rootWindow = static_cast<RootWindow*>(data);
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
		s_viewer = new RootWindow(x, y, w, h, "SmartBody");
	return s_viewer;
}

void FltkViewerFactory::remove(SrViewer* viewer)
{
	if (viewer && (viewer == s_viewer))
	{
		viewer->hide_viewer();
	}
}


