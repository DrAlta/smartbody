// generated by Fast Light User Interface Designer (fluid) version 1.0300

#ifndef RetargetCreatorWindow_h
#define RetargetCreatorWindow_h

#include <vector>
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>


class RetargetCreatorWindow : public Fl_Double_Window
{
public:
	RetargetCreatorWindow(int x, int y, int w, int h, char* name);
	~RetargetCreatorWindow();	
	void show();
	void hide();

	static Fl_Menu_Item menu_[];	
public:
	Fl_Choice * _choiceSrcSkeleton;
	Fl_Choice * _choiceTgtSkeleton;
	
	Fl_Multi_Browser *_browserMotion;	
	Fl_Multi_Browser *_assetDirList;
	Fl_Button *_buttonDirChoose;	
	Fl_Button *_buttonDirRemove;
	Fl_Button *_buttonRetarget;
	Fl_Input *_curOutputDir;

	Fl_Button *_buttonAssetDirChoose;	
	Fl_Input  *_curAssetDir;
	Fl_Button* _buttonReloadAsset;
	std::vector<std::string> inputMotionList;
public:

	

	void draw();
	void loadSkeletons();
	void loadMotions();
	void retargetSelectedMotion();
	void reloadAssets();
	static void OnDirChooseCB(Fl_Widget* widget, void* data);
	static void OnAssetDirChooseCB(Fl_Widget* widget, void* data);
	static void OnAssetDirRemoveCB(Fl_Widget* widget, void* data);
	static void OnRetargetCB(Fl_Widget* widget, void* data);
	static void OnReloadAssetCB(Fl_Widget* widget, void* data);
	static void OnMotionSelectCB(Fl_Widget* widget, void* data);
};
#endif
