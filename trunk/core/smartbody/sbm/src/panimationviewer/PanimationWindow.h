/*
 *  PanimationWindow.h - part of SmartBody-lib's Test Suite
 *  Copyright (C) 2009  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Yuyu Xu, USC
 */

#ifndef _PANIMATION_WINDOW_H_
#define _PANIMATION_WINDOW_H_

#include <FL/Fl_Slider.H>
#include "vhcl.h"
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <sbm/GenericViewer.h>
#include <sbm/me_ct_param_animation_utilities.h>
#include <sbm/me_ct_param_animation_data.h>
#include <map>
#include "ParamAnimEditorWidget.h"
#include "ParamAnimStateEditor.h"
#include "ParamAnimTransitionEditor.h"
#include "ParamAnimRunTimeEditor.h"
#include "ParamAnimScriptEditor.h"


const static int yDis = 10;
const static int xDis = 10;
class PAStateEditor;
class PATransitionEditor;
class PAScriptEditor;
class PARunTimeEditor;

namespace SmartBody {
	class SBCharacter;
}

class PanimationWindow : public Fl_Double_Window, public GenericViewer
{
	public:
		PanimationWindow(int x, int y, int w, int h, char* name);
		~PanimationWindow();

		virtual void label_viewer(std::string name);
		virtual void show_viewer();
		virtual void hide_viewer();
		virtual void update_viewer();
		void draw();
        void show();  

		SmartBody::SBCharacter* getCurrentCharacter();
		bool checkCommand(std::string command);
		static void execCmd(PanimationWindow* window, std::string command, double tOffset = 0.0);
		std::vector<std::string> tokenize(const std::string& str,const std::string& delimiters);
		void addTimeMark(nle::NonLinearEditorModel* model, bool selective = false);
		void removeTimeMark(nle::NonLinearEditorModel* model); 
		void addTimeMarkToBlock(nle::Block* block, double t);

		static void loadCharacters(Fl_Choice* characterList);
		static void refreshUI(Fl_Widget* widget, void* data);
		static void changeMotionPlayerMode(Fl_Widget* widget, void* data);
		void motionPlayerUpdate();
		void getSelectedMarkInfo(nle::NonLinearEditorModel* model, std::string& blockName, double& time);
		static void reset(Fl_Widget* widget, void* data);
		static PanimationWindow* getPAnimationWindow(Fl_Widget* w);

		// transition editor functions
		static void changeTransitionEditorMode(Fl_Widget* widget, void* data);
		static void changeStateList1(Fl_Widget* widget, void* data);
		static void changeStateList2(Fl_Widget* widget, void* data);
		static void changeAnimForTransition(Fl_Widget* widget, void* data);
		static void addTransitionTimeMark(Fl_Widget* widget, void* data);
		static void removeTransitionTimeMark(Fl_Widget* widget, void* data);
		static void updateTransitionTimeMark(Fl_Widget* widget, void* data);
		static void loadTransitions(Fl_Choice* transitionList);
		static void createNewTransition(Fl_Widget* widget, void* data);
		static void changeTransitionList(Fl_Widget* widget, void* data);

	public:
		std::string lastCommand;
	
		Fl_Tabs*		tabGroup;
		PATransitionEditor* transitionEditor;
		PAStateEditor*		stateEditor;
		PAScriptEditor*		scriptEditor;
		PARunTimeEditor*	runTimeEditor;

		Fl_Check_Button*	motionPlayerMode;
		Fl_Choice*		characterList;
		Fl_Button*		refresh;
		Fl_Button*		resetCharacter;
};

 class PanimationViewerFactory : public GenericViewerFactory
 {
	public:
		PanimationViewerFactory();

		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);
 };
#endif
