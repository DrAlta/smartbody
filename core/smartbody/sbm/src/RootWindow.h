#ifndef _ROOTWINDOW_
#define _ROOTWINDOW_

#include <fltk/DoubleBufferWindow.h>
#include <fltk/MenuBar.h>
#include <fltk/Button.h>
#include <fltk/PackedGroup.h>
#include <fltk/FloatInput.h>
#include <fltk/Output.h>
#include "fltk_viewer.h"
#include <sr/sr_viewer.h>
#include "CommandWindow.h"

class SbmCharacter;

class  RootWindow : public SrViewer, public fltk::DoubleBufferWindow
{
	public:
		RootWindow(int x, int y, int w, int h, const char* name);
		~RootWindow();

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

		FltkViewer* fltkViewer;
		CommandWindow* commandWindow;
		fltk::MenuBar* menubar;
		fltk::Button* buttonPlay;
		fltk::Button* buttonStop;
		fltk::Button* buttonPlaybackStepForward;

		fltk::Input *inputTimeStep;

		std::string scriptFolder;

		static void LoadCB(Widget* widget, void* data);
		static void SaveCB(Widget* widget, void* data);
		static void RunCB(Widget* widget, void* data);
		static void LaunchBMLViewerCB(Widget* widget, void* data);
		static void LaunchDataViewerCB(Widget* widget, void* data);
		static void LaunchParamAnimViewerCB(Widget* widget, void* data);
		static void LaunchConsoleCB(Widget* widget, void* data);
		static void StartCB(Widget* widget, void* data);
		static void StopCB(Widget* widget, void* data);
		static void StepCB(Widget* widget, void* data);
		static void PauseCB(Widget* widget, void* data);
		static void ResumeCB(Widget* widget, void* data);
		static void ResetCB(Widget* widget, void* data);
		static void CameraResetCB(Widget* widget, void* data);
		static void CameraFrameCB(Widget* widget, void* data);
		static void FaceCameraCB(Widget* widget, void* data);
		static void RotateSelectedCB(Widget* widget, void* data);
		static void RunScriptCB(fltk::Widget* w, void* data);
		static void ReloadScriptsCB(fltk::Widget* w, void* data);
		static void SetScriptDirCB(fltk::Widget* w, void* data);
		static void ShowSelectedCB(fltk::Widget* w, void* data);
		static void ModeBonesCB(fltk::Widget* w, void* data);
		static void ModeGeometryCB(fltk::Widget* w, void* data);
		static void ModeDeformableGeometryCB(fltk::Widget* w, void* data);
		static void ModeGPUDeformableGeometryCB(fltk::Widget* w, void* data);
		static void ModeAxisCB(fltk::Widget* w, void* data);
		static void ModeEyebeamsCB(fltk::Widget* w, void* data);
		static void ModeEyelidCalibrationCB(fltk::Widget* w, void* data);
		static void ShadowsCB(fltk::Widget* w, void* data);
		static void TerrainShadedCB(fltk::Widget* w, void* data);
		static void TerrainWireframeCB(fltk::Widget* w, void* data);
		static void TerrainNoneCB(fltk::Widget* w, void* data);
		static void ShowPawns(fltk::Widget* w, void* data);
		static void ShowPoseExamples(fltk::Widget* w, void* data);
		static void ModeDynamicsCOMCB(fltk::Widget* w, void* data);
		static void ModeDynamicsSupportPolygonCB(fltk::Widget* w, void* data);
		static void ModeDynamicsMassesCB(fltk::Widget* w, void* data);
		static void SettingsSofteyesToggleCB(fltk::Widget* w, void* data);
		static void TrackCharacterCB(fltk::Widget* w, void* data);
		static void AudioCB(fltk::Widget* w, void* data);
		static void CreateCharacterCB(fltk::Widget* w, void* data);
		static void CreatePawnCB(fltk::Widget* w, void* data);
		static void CreateTerrainCB(fltk::Widget* w, void* data);
		static void KinematicFootstepsCB(fltk::Widget* w, void* data);
		static void LocomotionFootstepsCB(fltk::Widget* w, void* data);
		static void VelocityCB(fltk::Widget* w, void* data);
		static void TrajectoryCB(fltk::Widget* w, void* data);
		static void SteeringCharactersCB(fltk::Widget* w, void* data);
		static void SteeringAllCB(fltk::Widget* w, void* data);
		static void SteeringNoneCB(fltk::Widget* w, void* data);		
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
