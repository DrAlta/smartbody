/*
 *  ParamAnimRunTimeEditor.h - part of SmartBody-lib's Test Suite
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

#ifndef _PARAM_ANIM_RUN_TIME_EDITOR_H_
#define _PARAM_ANIM_RUN_TIME_EDITOR_H_

#include "PanimationWindow.h"

class PanimationWindow;
class ParameterWindow;
class ParameterVisualization : public fltk::Group
{
	public:
		ParameterVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterWindow* window);
		~ParameterVisualization();

		virtual void draw();
		virtual int handle(int event);
		virtual void setup();
		virtual void resize(int x, int y, int w, int h);
		void setParam(float x, float y);
		void setSlider(int x, int y);

	private:
		void getBound(int ptX, int ptY, int& x, int& y, int& w, int& h);
		void getActualPixel(SrVec vec, int& x, int& y);

	private:
		static const int pad = 5;
		static const int margin = 10;
		static const int gridSizeX = 50;
		static const int gridSizeY = 30;
		float scaleX;
		float scaleY;
		int centerX;
		int centerY;
		int width;
		int height;
		int paramX;
		int paramY;
		PAStateData* state;
		ParameterWindow* paramWindow;
};

class ParameterWindow : public fltk::Window
{
	public:
		ParameterWindow(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex = false);
		~ParameterWindow();

		static void updateXAxisValue(fltk::Widget* widget, void* data);
		static void updateAxisValue(fltk::Widget* widget, void* data);
		void updateWeight();

	public:
		PanimationWindow* paWindow;
		PAStateData* state;
		ParameterVisualization* paramVisualization;
		bool exec;
		fltk::ValueSlider* xAxis;
		fltk::ValueSlider* yAxis;
};

class PARunTimeEditor : public fltk::Group
{
	public:
		PARunTimeEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PARunTimeEditor();

	public:
		PanimationWindow*	paWindow;
		fltk::Output*		currentCycleState;
		fltk::Browser*		nextCycleStates;
		fltk::Browser*		availableTransitions;
		fltk::Button*		runNextState;
		std::string			prevCycleState;

		ParameterWindow*	nonCycleParamWindow;
		ParameterWindow*	cycleParamWindow;
	public:
		void update();
		void updateRunTimeStates(std::string currentState);
		void addItem(fltk::Browser* browser, std::string item);
		void initializeRunTimeEditor();
		static void updateNonCycleState(fltk::Widget* widget, void* data);
		static void updateTransitionStates(fltk::Widget* widget, void* data);
		static void run(fltk::Widget* widget, void* data);
};


#endif
