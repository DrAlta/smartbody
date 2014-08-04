#ifndef _AUTORIGVIEWER_
#define _AUTORIGVIEWER_

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>
#include <sr/sr_camera.h>
#include <sk/sk_scene.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_light.h>
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <string>

class RetargetStepWindow;

class AutoRigViewer : public Fl_Double_Window
{
	public:
		AutoRigViewer(int x, int y, int w, int h, char* name);
		
		~AutoRigViewer();
		
		virtual void draw();
		void updatePawnList();
		void applyAutoRig(int riggingType = 0);
		static void ApplyAutoRigCB(Fl_Widget* widget, void* data);
		void updateAutoRigViewer();

		void setRetargetStepWindow(RetargetStepWindow* val) { retargetStepWindow = val; }
	public:
		std::string _deletePawnName;
		Fl_Choice* _choicePawns;
		Fl_Choice* _choiceVoxelRigging;
		Fl_Button* _buttonAutoRig;	
	protected:
		RetargetStepWindow* retargetStepWindow;		
};
#endif
