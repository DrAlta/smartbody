/*
 *  ParamAnimScriptEditor.h - part of SmartBody-lib's Test Suite
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

#ifndef _PARAM_ANIM_SCRIPT_EDITOR_H_
#define _PARAM_ANIM_SCRIPT_EDITOR_H_

#include "PanimationWindow.h"

class PanimationWindow;
class PAScriptEditor : public fltk::Group
{
	public:
		PAScriptEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PAScriptEditor();
		
		static void addState(fltk::Widget* widget, void* data);
		static void removeState(fltk::Widget* widget, void* data);
		static void updateStateInfo(fltk::Widget* widget, void* data);
		static void run(fltk::Widget* widget, void* data);
		static void changeCurrentStateWeight(fltk::Widget* widget, void* data);
		void initialAvailableStates();
		void updateAvailableStates(std::string currentState);
		void refresh();
		void update();

	public:
		PanimationWindow*	paWindow;
		fltk::Browser*		availableStateList;
		fltk::Browser*		currentStateList;
		fltk::Button*		addStateButton;
		fltk::Button*		removeStateButton;
		fltk::Button*		runStateList;
		fltk::Output*		currentStatePanel;
		fltk::Output*		nextStatePanel;
		fltk::ValueSlider*	currentStateWeight;

		std::map<std::string, double>	stateTimeOffset;
		std::map<std::string, bool>		stateLoopMode;
};

#endif
