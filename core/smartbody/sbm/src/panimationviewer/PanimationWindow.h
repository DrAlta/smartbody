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

#include <fltk/Window.h>
#include <fltk/Choice.h>
#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/TabGroup.h>
#include <fltk/Group.h>
#include <fltk/Browser.h>
#include <fltk/Input.h>
#include <fltk/ValueSlider.h>
#include <fltk/TextDisplay.h>
#include <fltk/MultiLineOutput.h>
#include <fltk/MenuBar.h>
#include <fltk/ask.h>
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
class PanimationWindow : public fltk::Window, public GenericViewer
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

		bool checkCommand(std::string command);
		static void execCmd(PanimationWindow* window, std::string command, double tOffset = 0.0);
		std::vector<std::string> tokenize(const std::string& str,const std::string& delimiters);
		void addTimeMark(nle::NonLinearEditorModel* model);
		void removeTimeMark(nle::NonLinearEditorModel* model); 
		void addTimeMarkToBlock(nle::Block* block, double t);

		static void loadCharacters(fltk::Choice* characterList);
		static void refreshUI(fltk::Widget* widget, void* data);
		static void clearTextDisplay(fltk::Widget* widget, void* data);
		static void changeMotionPlayerMode(fltk::Widget* widget, void* data);
		void motionPlayerUpdate();
		void getSelectedMarkInfo(nle::NonLinearEditorModel* model, std::string& blockName, double& time);
		static void reset(fltk::Widget* widget, void* data);

		// transition editor functions
		static void changeTransitionEditorMode(fltk::Widget* widget, void* data);
		static void changeStateList1(fltk::Widget* widget, void* data);
		static void changeStateList2(fltk::Widget* widget, void* data);
		static void changeAnimForTransition(fltk::Widget* widget, void* data);
		static void addTransitionTimeMark(fltk::Widget* widget, void* data);
		static void removeTransitionTimeMark(fltk::Widget* widget, void* data);
		static void updateTransitionTimeMark(fltk::Widget* widget, void* data);
		static void loadTransitions(fltk::Choice* transitionList);
		static void createNewTransition(fltk::Widget* widget, void* data);
		static void changeTransitionList(fltk::Widget* widget, void* data);

	public:
		std::string lastCommand;
	
		fltk::TabGroup*		tabGroup;
		PATransitionEditor* transitionEditor;
		PAStateEditor*		stateEditor;
		PAScriptEditor*		scriptEditor;
		PARunTimeEditor*	runTimeEditor;

		fltk::CheckButton*	motionPlayerMode;
		fltk::Choice*		characterList;
		fltk::Button*		refresh;
		fltk::Button*		resetCharacter;
		fltk::TextDisplay*	textDisplay;
		fltk::TextBuffer*	textBuffer;
		fltk::Button*		clearHistoryButton;
};

 class PanimationViewerFactory : public GenericViewerFactory
 {
	public:
		PanimationViewerFactory();

		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);
 };
#endif