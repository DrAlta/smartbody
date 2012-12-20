#ifndef _RETARGET_STEP_WINDOW_H_
#define _RETARGET_STEP_WINDOW_H_

#include <vhcl.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>
#include "jointmapviewer/JointMapViewer.h"
#include "retargetviewer/RetargetViewer.h"


class RetargetStepWindow : public Fl_Double_Window 
{
public:
	RetargetStepWindow(int x, int y, int w, int h, char* name);
	~RetargetStepWindow();
	
public:	
	virtual void draw();
	void setCharacterName(std::string charName);
	void setSkeletonName(std::string skName);
	void setJointMapName(std::string jointMapName);

	void applyRetargetSteps();

	static void ApplyCB(Fl_Widget* widget, void* data);
	static void CancelCB(Fl_Widget* widget, void* data);

protected:
	Fl_Tabs*		tabGroup;
	JointMapViewer* jointMapViewer;
	RetargetViewer* retargetViewer;
	Fl_Button* _buttonApply;
	Fl_Button* _buttonCancel;
};


#endif
