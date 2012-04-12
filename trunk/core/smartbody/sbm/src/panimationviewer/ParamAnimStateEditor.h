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

#include <vhcl.h>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Float_Input.H>
#include <vhcl.h>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Check_Button.H>
#include "PanimationWindow.h"

class PAStateCreator;
class PanimationWindow;
class ParameterVisualization;
class Parameter3DVisualization;
class PAStateEditor : public Fl_Group
{
	public:
		PAStateEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PAStateEditor();
	
		void loadStates();
		static void changeStateEditorMode(Fl_Widget* widget, void* data);
		static void updateStateTimeMarkEditor(Fl_Widget* widget, void* data, bool toAdd);
		static void editStateMotions(Fl_Widget* widget, void* data);
		static void changeStateList(Fl_Widget* widget, void* data);
		static void addStateTimeMark(Fl_Widget* widget, void* data);
		static void addFootStepMark(Fl_Widget* widget, void* data);
		static void removeStateTimeMark(Fl_Widget* widget, void* data);
		static void updateStateTimeMark(Fl_Widget* widget, void* data);
		static void updateMaxTime(Fl_Widget* widget, void* data);
		static void updateMinTime(Fl_Widget* widget, void* data);
		static void save(Fl_Widget* widget, void* data);
		static void selectStateAnimations(Fl_Widget* widget, void* data);
		static void addShape(Fl_Widget* widget, void* data);
		static void removeShape(Fl_Widget* widget, void* data);
		static void selectShape(Fl_Widget* widget, void* data);
		static void updateParameters(Fl_Widget* widget, void* data);
		static void scrub(Fl_Widget* widget, void* data);
		static void playmotion(Fl_Widget* widget, void* data);
		void updateCorrespondenceMarks(PAState* state);
		void refresh();

	public:
		PanimationWindow* paWindow;
		Fl_Check_Button*	stateEditorMode;
		Fl_Group*		stateSelectionGroup;
		Fl_Button*		createStateButton;
		Fl_Input*		newStateName;
		Fl_Multi_Browser*		stateAnimationList;
		Fl_Multi_Browser*		shapeList;
		Fl_Button*		shapeAdd;
		Fl_Button*		shapeRemove;
		Fl_Scroll*		editStateTimeMarkGroup;
		Fl_Choice*		stateList;
		Fl_Choice*		choiceStateType;
		Fl_Button*		addMark;
		Fl_Button*		removeMark;
		Fl_Button*		updateMark;
		Fl_Button*		buttonSave;
		Fl_Button*		autoFootStepMarks;
		Fl_Float_Input*		minTimeInput;
		Fl_Float_Input*		maxTimeInput;
		Fl_Float_Input* inputParameterX;
		Fl_Float_Input* inputParameterY;
		Fl_Float_Input* inputParameterZ;
		Fl_Value_Slider* sliderScrub;
		Fl_Check_Button* checkPlay;
		ParamAnimEditorWidget* stateTimeMarkWidget;
		nle::NonLinearEditorModel* stateEditorNleModel;
		PAStateCreator* creator;
		std::string lastSelectedMotion;

		Fl_Group*				visualizationGroup;
		ParameterVisualization* triangleVisualization;
		Parameter3DVisualization* tetraVisualization;
		PAStateData*	stateData;

		int lastNameIndex;
};

#endif
