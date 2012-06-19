#ifndef _ROOTWINDOW_
#define _ROOTWINDOW_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Output.H>
#include "fltk_viewer.h"
#include <sr/sr_viewer.h>
#include "CommandWindow.h"
#include "bmlcreator/BMLCreatorWindow.h"
#include "visemeviewer/VisemeViewerWindow.h"
#include "monitorviewer/MonitorConnectWindow.h"
#include "CharacterCreatorWindow.h"

class SbmCharacter;

#ifdef WIN32
#define USE_OGRE_VIEWER 0
#else
#define USE_OGRE_VIEWER 0
#endif
#define NO_OGRE_VIEWER_CMD 0

#if USE_OGRE_VIEWER > 0
#include "FLTKOgreViewer.h"
#endif

class  BaseWindow : public SrViewer, public Fl_Double_Window
{
	public:
	
		BaseWindow(int x, int y, int w, int h, const char* name);
		~BaseWindow();

		virtual void show_viewer();
		virtual void hide_viewer();	
		virtual void set_camera(const SrCamera &cam);
		virtual SrCamera* get_camera();
		void render();
		void root(SrSn* r);
		SrSn* root();

		void runScript(std::string filename);
		void reloadScripts(std::string scriptsDir);
		void reloadScriptsByDir(std::string scriptsDir, std::string parentStr);
		SbmCharacter* getSelectedCharacter();

#if USE_OGRE_VIEWER > 0
		FLTKOgreWindow* fltkViewer;
#else
		FltkViewer* fltkViewer;
#endif
		
		CommandWindow* commandWindow;
		BMLCreatorWindow* bmlCreatorWindow;
		VisemeViewerWindow* visemeViewerWindow;
		MonitorConnectWindow* monitorConnectWindow;

		Fl_Menu_Bar* menubar;
		Fl_Button* buttonPlay;
		Fl_Button* buttonStop;
		Fl_Button* buttonPlaybackStepForward;
		CharacterCreatorWindow* characterCreator;

		Fl_Input *inputTimeStep;

		std::string scriptFolder;

		static void LoadCB(Fl_Widget* widget, void* data);
		static void SaveCB(Fl_Widget* widget, void* data);
		static void RunCB(Fl_Widget* widget, void* data);
		static void LaunchVisemeViewerCB(Fl_Widget* widget, void* data);
		static void LaunchBMLViewerCB(Fl_Widget* widget, void* data);
		static void LaunchDataViewerCB(Fl_Widget* widget, void* data);
		static void LaunchParamAnimViewerCB(Fl_Widget* widget, void* data);
		static void LaunchConsoleCB(Fl_Widget* widget, void* data);
		static void LaunchBMLCreatorCB(Fl_Widget* widget, void* data);
		static void LaunchResourceViewerCB(Fl_Widget* widget, void* data);		
		static void LaunchFaceViewerCB(Fl_Widget* widget, void* data);
		static void LaunchSpeechRelayCB(Fl_Widget* widget, void* data);
		static void LaunchConnectCB(Fl_Widget* widget, void* data);
		static void StartCB(Fl_Widget* widget, void* data);
		static void StopCB(Fl_Widget* widget, void* data);
		static void StepCB(Fl_Widget* widget, void* data);
		static void PauseCB(Fl_Widget* widget, void* data);
		static void ResumeCB(Fl_Widget* widget, void* data);
		static void ResetCB(Fl_Widget* widget, void* data);
		static void CameraResetCB(Fl_Widget* widget, void* data);
		static void CameraFrameCB(Fl_Widget* widget, void* data);
		static void FaceCameraCB(Fl_Widget* widget, void* data);
		static void RotateSelectedCB(Fl_Widget* widget, void* data);
		static void RunScriptCB(Fl_Widget* w, void* data);
		static void ReloadScriptsCB(Fl_Widget* w, void* data);
		static void SetScriptDirCB(Fl_Widget* w, void* data);
		static void ShowSelectedCB(Fl_Widget* w, void* data);
		static void ModeBonesCB(Fl_Widget* w, void* data);
		static void ModeGeometryCB(Fl_Widget* w, void* data);
		static void ModeCollisionGeometryCB(Fl_Widget* w, void* data);
		static void ModeDeformableGeometryCB(Fl_Widget* w, void* data);
		static void ModeGPUDeformableGeometryCB(Fl_Widget* w, void* data);
		static void ModeAxisCB(Fl_Widget* w, void* data);
		static void ModeEyebeamsCB(Fl_Widget* w, void* data);
		static void ModeEyelidCalibrationCB(Fl_Widget* w, void* data);
		static void ShadowsCB(Fl_Widget* w, void* data);
		static void TerrainShadedCB(Fl_Widget* w, void* data);
		static void TerrainWireframeCB(Fl_Widget* w, void* data);
		static void TerrainNoneCB(Fl_Widget* w, void* data);
		static void ShowPawns(Fl_Widget* w, void* data);
		static void ShowPoseExamples(Fl_Widget* w, void* data);
		static void ModeDynamicsCOMCB(Fl_Widget* w, void* data);
		static void ModeDynamicsSupportPolygonCB(Fl_Widget* w, void* data);
		static void ModeDynamicsMassesCB(Fl_Widget* w, void* data);
		static void ShowBoundingVolumeCB(Fl_Widget* w, void* data);
		static void SettingsSofteyesToggleCB(Fl_Widget* w, void* data);
		static void TrackCharacterCB(Fl_Widget* w, void* data);
		static void AudioCB(Fl_Widget* w, void* data);
		static void CreateCharacterCB(Fl_Widget* w, void* data);
		static void CreatePawnCB(Fl_Widget* w, void* data);
		static void CreateTerrainCB(Fl_Widget* w, void* data);
		static void KinematicFootstepsCB(Fl_Widget* w, void* data);
		static void LocomotionFootstepsCB(Fl_Widget* w, void* data);
		static void VelocityCB(Fl_Widget* w, void* data);
		static void TrajectoryCB(Fl_Widget* w, void* data);
		static void SteeringCharactersCB(Fl_Widget* w, void* data);
		static void SteeringAllCB(Fl_Widget* w, void* data);
		static void SteeringNoneCB(Fl_Widget* w, void* data);	
		static void GridCB(Fl_Widget* w, void* data);	
};

class FltkViewerFactory : public SrViewerFactory
 {
	public:
		FltkViewerFactory();

		//void setFltkViewer(FltkViewer* viewer);

		virtual SrViewer* create(int x, int y, int w, int h);
		virtual void remove(SrViewer* viewer);

	private:
		static SrViewer* s_viewer;

 };
#endif
