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

#include <FL/Fl_Slider.H>
#include <FL/Fl_Scroll.H>
#include <vhcl.h>
#include <FL/Fl_Multi_Browser.H>
#include "PanimationWindow.h"

class PanimationWindow;
class PAStateEditor : public Fl_Group
{
	public:
		PAStateEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PAStateEditor();

		void loadMotions();
		void loadStates();
		static void changeStateEditorMode(Fl_Widget* widget, void* data);
		static void updateStateTimeMarkEditor(Fl_Widget* widget, void* data, bool toAdd);
		static void createNewState(Fl_Widget* widget, void* data);
		static void addMotion(Fl_Widget* widget, void* data);
		static void removeMotion(Fl_Widget* widget, void* data);
		static void changeStateList(Fl_Widget* widget, void* data);
		static void addStateTimeMark(Fl_Widget* widget, void* data);
		static void addFootStepMark(Fl_Widget* widget, void* data);
		static void removeStateTimeMark(Fl_Widget* widget, void* data);
		static void updateStateTimeMark(Fl_Widget* widget, void* data);
		void updateCorrespondenceMarks(PAStateData* state);
		void refresh();

	public:
		PanimationWindow* paWindow;
		Fl_Check_Button*	stateEditorMode;
		Fl_Group*		createStateGroup;
		Fl_Button*		createStateButton;
		Fl_Input*		newStateName;
		Fl_Multi_Browser*		animationList;
		Fl_Multi_Browser*		stateAnimationList;
		Fl_Button*		animationAdd;
		Fl_Button*		animationRemove;
		Fl_Scroll*		editStateTimeMarkGroup;
		Fl_Choice*		stateList;
		Fl_Button*		addMark;
		Fl_Button*		removeMark;
		Fl_Button*		updateMark;
		Fl_Button*		autoFootStepMarks;
		ParamAnimEditorWidget* stateTimeMarkWidget;
		nle::NonLinearEditorModel* stateEditorNleModel;
};

#endif
