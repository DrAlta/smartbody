/*
 *  ParamAnimTransitionEditor.h - part of SmartBody-lib's Test Suite
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

#ifndef _PARAM_ANIM_TRANSITION_EDITOR_H_
#define _PARAM_ANIM_TRANSITION_EDITOR_H_

#include "PanimationWindow.h"

class PanimationWindow;
class PATransitionEditor : public fltk::Group
{
	public:
		PATransitionEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PATransitionEditor();

		void loadStates();
		void loadTransitions();
		static void changeTransitionEditorMode(fltk::Widget* widget, void* data);
		static void changeStateList1(fltk::Widget* widget, void* data);
		static void changeStateList2(fltk::Widget* widget, void* data);
		static void changeAnimForTransition(fltk::Widget* widget, void* data);
		static void addTransitionTimeMark(fltk::Widget* widget, void* data);
		static void removeTransitionTimeMark(fltk::Widget* widget, void* data);
		static void updateTransitionTimeMark(fltk::Widget* widget, void* data);
		static void createNewTransition(fltk::Widget* widget, void* data);
		static void changeTransitionList(fltk::Widget* widget, void* data);

	public:
		PanimationWindow* paWindow;
		fltk::CheckButton*	transitionEditorMode;
		fltk::Group*		createTransitionGroup;
		fltk::Button*		createTransitionButton;
		fltk::Choice*		stateList1;
		fltk::Choice*		stateList2;
		fltk::Browser*		animForTransition1;
		fltk::Browser*		animForTransition2;
		fltk::Group*		editTransitionTimeMarkGroup;
		fltk::Choice*		transitionList;
		fltk::Button*		addMark1;
		fltk::Button*		removeMark1;
		fltk::Button*		updateMark1;
		ParamAnimEditorWidget* transitionTimeMarkWidget;
		nle::NonLinearEditorModel* transitionEditorNleModel;
};

#endif