#ifndef _RETARGET_STEP_WINDOW_H_
#define _RETARGET_STEP_WINDOW_H_

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
#include <SBWindowListener.h>


class RetargetStepWindow : public Fl_Double_Window , public SBWindowListener  
{
public:
	RetargetStepWindow(int x, int y, int w, int h, char* name);
	~RetargetStepWindow();
	
	virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
	virtual void OnCharacterDelete( const std::string & name );
	virtual void OnCharacterUpdate( const std::string & name );
	virtual void OnPawnCreate( const std::string & name );
	virtual void OnPawnDelete( const std::string & name );

public:	
	virtual void hide();
	virtual void show();
	virtual void draw();
	void setApplyType(bool applyAll);
	void setCharacterName(std::string charName);	
	//void setSkeletonName(std::string skName);
	void setJointMapName(std::string jointMapName);
	void applyRetargetSteps();
	void applyAutoRig(int riggingType = 0);
	void updateSkinWeight(int weightType = 0);

	void updateCharacterList();
	void updatePawnList();
	void refreshAll();

	static void CharacterCB(Fl_Widget* widget, void* data);
	static void ApplyJointMapCB(Fl_Widget* widget, void* data);
	static void ApplyBehaviorSetCB(Fl_Widget* widget, void* data);
	static void ApplyCB(Fl_Widget* widget, void* data);
	static void ApplyAutoRigCB(Fl_Widget* widget, void* data);
	static void UpdateSkinWeightCB(Fl_Widget* widget, void* data);
	static void CancelCB(Fl_Widget* widget, void* data);
	static void RefreshCB(Fl_Widget* widget, void* data);

public:
	JointMapViewer* jointMapViewer;
	RetargetViewer* retargetViewer;

protected:
	Fl_Choice* _choiceCharacters;
	Fl_Choice* _choicePawns;
	Fl_Tabs*	tabGroup;	
	Fl_Button* _buttonAutoRig;
	Fl_Button* _buttonUpdateSkinWeight;
	Fl_Button* _buttonApplyMap;
	Fl_Button* _buttonApplyBehaviorSet;
	Fl_Button* _buttonApplyAll;
	Fl_Button* _buttonCancel;
	Fl_Button* _buttonRefresh;
	Fl_Choice* _buttonVoxelRigging;
	std::string _charName;
	std::string _removeCharacterName;
	std::string _removePawnName;
};


#endif
