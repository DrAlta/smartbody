/*
 *  ParamAnimStateEditor.h - part of SmartBody-lib's Test Suite
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

#ifndef _PARAM_ANIM_STATE_EDITOR_H_
#define _PARAM_ANIM_STATE_EDITOR_H_

#include "PanimationWindow.h"

class PanimationWindow;
class PAStateEditor : public fltk::Group
{
	public:
		PAStateEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PAStateEditor();

		void loadMotions();
		void loadStates();
		static void changeStateEditorMode(fltk::Widget* widget, void* data);
		static void updateStateTimeMarkEditor(fltk::Widget* widget, void* data, bool toAdd);
		static void createNewState(fltk::Widget* widget, void* data);
		static void addMotion(fltk::Widget* widget, void* data);
		static void removeMotion(fltk::Widget* widget, void* data);
		static void changeStateList(fltk::Widget* widget, void* data);
		static void addStateTimeMark(fltk::Widget* widget, void* data);
		static void removeStateTimeMark(fltk::Widget* widget, void* data);
		static void updateStateTimeMark(fltk::Widget* widget, void* data);
		void updateCorrespondenceMarks(PAStateData* state);
		void refresh();

	public:
		PanimationWindow* paWindow;
		fltk::CheckButton*	stateEditorMode;
		fltk::Group*		createStateGroup;
		fltk::Button*		createStateButton;
		fltk::Input*		newStateName;
		fltk::Browser*		animationList;
		fltk::Browser*		stateAnimationList;
		fltk::Button*		animationAdd;
		fltk::Button*		animationRemove;
		fltk::Group*		editStateTimeMarkGroup;
		fltk::Choice*		stateList;
		fltk::Button*		addMark;
		fltk::Button*		removeMark;
		fltk::Button*		updateMark;
		ParamAnimEditorWidget* stateTimeMarkWidget;
		nle::NonLinearEditorModel* stateEditorNleModel;
};

#endif