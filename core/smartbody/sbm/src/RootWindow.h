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
#include "bmlviewer/BehaviorWindow.h"
#include "bmlcreator/BMLCreatorWindow.h"
#include "resourceViewer/ResourceWindow.h"
#include "visemeviewer/VisemeViewerWindow.h"
#include "retargetcreator/RetargetCreatorWindow.h"
#include "monitorviewer/MonitorConnectWindow.h"
#include "motioneditor/MotionEditorWindow.h"
#include "panimationviewer/PanimationWindow.h"
#include "faceviewer/FaceViewer.h"
#include "channelbufferviewer/channelbufferWindow.hpp"
#include "retargetviewer/RetargetViewer.h"
#include "jointmapviewer/RetargetStepWindow.h"

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

class CharacterCreatorWindow;
class ResolutionWindow;

class  BaseWindow : public SrViewer, public Fl_Double_Window
{
	public:
	
		BaseWindow(int x, int y, int w, int h, const char* name);
		~BaseWindow();

		virtual void show_viewer();
		virtual void hide_viewer();	
		virtual void set_camera(const SrCamera* cam);
		virtual SrCamera* get_camera();
		void render();
		void root(SrSn* r);
		SrSn* root();

		void resetWindow();

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
		MotionEditorWindow* motionEditorWindow;
		RetargetCreatorWindow* retargetCreatorWindow;
		FaceViewer* faceViewerWindow;
		BehaviorWindow* bmlViewerWindow;
		ChannelBufferWindow* dataViewerWindow;
		ResourceWindow* resourceWindow;
		PanimationWindow* panimationWindow;
		//RetargetViewer* behaviorSetViewer;
		//JointMapViewer* jointMapViewer;
		RetargetStepWindow* retargetStepWindow;

		Fl_Menu_Bar* menubar;
		Fl_Button* buttonPlay;
		Fl_Button* buttonStop;
		Fl_Button* buttonPlaybackStepForward;
		CharacterCreatorWindow* characterCreator;
		ResolutionWindow* resWindow;

		Fl_Choice* cameraChoice;
		Fl_Button* saveCamera;
		Fl_Button* deleteCamera;
		Fl_Choice* resolutionChoice;

		Fl_Input *inputTimeStep;

		std::string scriptFolder;
		std::vector<std::string> windowSizes;
		std::vector<Fl_Menu_Item> resolutionMenuList;
		int setResolutionMenuIndex;
		
		int loadCameraMenuIndex, deleteCameraMenuIndex;
		std::vector<Fl_Menu_Item> loadCameraList;
		std::vector<Fl_Menu_Item> deleteCameraList;

		std::vector<SrCamera*> cameraList;
		std::map<std::string,SrCamera*> cameraMap;


		void updateCameraList();

		static void SaveCameraCB(Fl_Widget* widget, void* data);
		static void DeleteCameraCB(Fl_Widget* widget, void* data);
		static void ChooseCameraCB(Fl_Widget* widget, void* data);
	
		static void LoadCB(Fl_Widget* widget, void* data);
		static void SaveCB(Fl_Widget* widget, void* data);
		static void RunCB(Fl_Widget* widget, void* data);
		static void LaunchVisemeViewerCB(Fl_Widget* widget, void* data);
		static void LaunchBMLViewerCB(Fl_Widget* widget, void* data);
		static void LaunchDataViewerCB(Fl_Widget* widget, void* data);
		static void LaunchParamAnimViewerCB(Fl_Widget* widget, void* data);
		static void LaunchConsoleCB(Fl_Widget* widget, void* data);
		static void LaunchBMLCreatorCB(Fl_Widget* widget, void* data);
		static void LaunchRetargetCreatorCB(Fl_Widget* widget, void* data);
		//static void LaunchBehaviorSetsCB(Fl_Widget* widget, void* data);
		static void LaunchMotionEditorCB(Fl_Widget* widget, void* data);
		static void LaunchResourceViewerCB(Fl_Widget* widget, void* data);		
		static void LaunchFaceViewerCB(Fl_Widget* widget, void* data);
		static void LaunchSpeechRelayCB(Fl_Widget* widget, void* data);
		static void LaunchJointMapViewerCB(Fl_Widget* widget, void* data);
		static void NewCB(Fl_Widget* widget, void* data);
		static void QuitCB(Fl_Widget* widget, void* data);
		static void LaunchConnectCB(Fl_Widget* widget, void* data);
		static void DisconnectRemoteCB(Fl_Widget* widget, void* data);
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
		static void SettingsDefaultMediaPathCB(Fl_Widget* w, void* data);
		static void SettingsPythonLibCB(Fl_Widget* w, void* data);
		static void TrackCharacterCB(Fl_Widget* w, void* data);
		static void AudioCB(Fl_Widget* w, void* data);
		static void CreateCharacterCB(Fl_Widget* w, void* data);
		static void CreatePawnCB(Fl_Widget* w, void* data);
		static void CreateLightCB(Fl_Widget* w, void* data);
		static void CreateTerrainCB(Fl_Widget* w, void* data);
		static void KinematicFootstepsCB(Fl_Widget* w, void* data);
		static void LocomotionFootstepsCB(Fl_Widget* w, void* data);
		static void VelocityCB(Fl_Widget* w, void* data);
		static void TrajectoryCB(Fl_Widget* w, void* data);
		static void GestureCB(Fl_Widget* w, void* data);
		static void JointLabelCB(Fl_Widget* w, void* data);
		static void SteeringCharactersCB(Fl_Widget* w, void* data);
		static void SteeringAllCB(Fl_Widget* w, void* data);
		static void SteeringNoneCB(Fl_Widget* w, void* data);	
		static void GridCB(Fl_Widget* w, void* data);	
		static void CreatePythonAPICB(Fl_Widget* widget, void* data);
		static void DocumentationCB(Fl_Widget* widget, void* data);
		static void ResizeWindowCB(Fl_Widget* widget, void* data);

};

class FltkViewerFactory : public SrViewerFactory
 {
	public:
		FltkViewerFactory();

		//void setFltkViewer(FltkViewer* viewer);

		virtual SrViewer* create(int x, int y, int w, int h);
		virtual void remove(SrViewer* viewer);
		virtual void reset(SrViewer* viewer);

	private:
		static SrViewer* s_viewer;

 };
#endif
