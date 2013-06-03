#ifndef _MOTION_EIDTOR_WINDOW_H_
#define _MOTION_EIDTOR_WINDOW_H_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <sb/SBCharacter.h>
#include <sb/SBMotion.h>

namespace SmartBody
{
	class SBCharacter;
	class SBMotion;
}

class MotionEditorWindow : public Fl_Double_Window
{
public:
	MotionEditorWindow(int x, int y, int w, int h, char* label);
	~MotionEditorWindow();
	
	void show();
	void hide();

	void loadCharacters();
	SmartBody::SBCharacter* getCurrentCharacter();
	void loadMotions();
	SmartBody::SBMotion* getCurrentMotion();
   std::string getCurrentCharacterName();
   std::string getCurrentMotionName();

	static void OnChoiceCharacterList(Fl_Widget* widget, void* data);
	static void OnButtonRefresh(Fl_Widget* widget, void* data);
	static void OnButtonSaveMotion(Fl_Widget* widget, void* data);
	static void OnBrowserMotionList(Fl_Widget* widget, void* data);
	static void OnButtonPlayMotion(Fl_Widget* widget, void* data);
   static void OnButtonSetPosture(Fl_Widget* widget, void* data);
	static void OnCheckButtonPlayMotion(Fl_Widget* widget, void* data);
	static void OnSliderMotionFrame(Fl_Widget* widget, void* data);
	static void OnButtonPlayMotionFolder(Fl_Widget* widget, void* data);
   static void OnButtonGazeAt(Fl_Widget* widget, void* data);
   static void OnButtonStopGaze(Fl_Widget* widget, void* data);

	void updateSyncPointsUI();
	void updateMotionSyncPoints(const std::string& type);
	static void OnSliderSyncPoints(Fl_Widget* widget, void* data);
	static void OnButtonGetSyncPoints(Fl_Widget* widget, void* data);

	void updateMetaDataUI();
	void addMotionMetaData(const std::string& name, const std::string& value);
	static void OnBrowserMetaNames(Fl_Widget* widget, void* data);
	static void OnBrowserMetaValues(Fl_Widget* widget, void* data);
	static void OnButtonAddMetaEntry(Fl_Widget* widget, void* data);
	static void OnButtonDeleteMetaEntry(Fl_Widget* widget, void* data);
	

   void PlayAnimation(const std::string& characterName, const std::string& animName, bool setAsPosture);
   void GazeAt(const std::string& characterName, const std::string& gazeTarget);
   void StopGaze(const std::string& characterName);

public:
	// common
	Fl_Choice*			_choiceCharacaterList;
	Fl_Button*			_buttonRefresh;
	Fl_Button*			_buttonSaveMotion;
	Fl_Hold_Browser*	_browserMotionList;
	Fl_Button*			_buttonPlayMotion;
   Fl_Button*        _buttonSetPosture;
	Fl_Check_Button*	_checkButtonPlayMotion;
	Fl_Value_Slider*	_sliderMotionFrame;
	Fl_Input*			_inputFilePath;
	Fl_Button*			_buttonPlayMotionFolder;

	// meta information
	Fl_Group*			_groupMetaInfo;
   Fl_Group*         _groupGazeInfo;
	Fl_Value_Slider*	_sliderStart;
	Fl_Button*			_buttonGetStartTime;
	Fl_Value_Slider*	_sliderReady;
	Fl_Button*			_buttonGetReadyTime;
	Fl_Value_Slider*	_sliderStrokeStart;
	Fl_Button*			_buttonGetStrokeStartTime;
	Fl_Value_Slider*	_sliderStroke;
	Fl_Button*			_buttonGetStrokeTime;
	Fl_Value_Slider*	_sliderStrokeEnd;
	Fl_Button*			_buttonGetStrokeEndTime;
	Fl_Value_Slider*	_sliderRelax;
	Fl_Button*			_buttonGetRelaxTime;
	Fl_Value_Slider*	_sliderEnd;
	Fl_Button*			_buttonGetEndTime;
	Fl_Hold_Browser*	_browserMetaNames;
	Fl_Hold_Browser*	_browserMetaValues;
	Fl_Input*			_inputMetaNames;
	Fl_Input*			_inputMetaValues;
	Fl_Button*			_buttonAddMetaEntry;
	Fl_Button*			_buttonDeleteMetaEntry;

   // gaze informations
   Fl_Choice*			_choiceGazeTargetList;
   Fl_Button*        _buttonGazeAt;
   Fl_Button*        _buttonStopGaze;

};
#endif
