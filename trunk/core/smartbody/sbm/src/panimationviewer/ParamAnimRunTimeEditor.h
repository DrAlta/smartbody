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
class ParameterGroup;
class ParameterVisualization : public fltk::Group
{
	public:
		ParameterVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterGroup* window);
		~ParameterVisualization();

		virtual void draw();
		virtual int handle(int event);
		virtual void setup();
		virtual void resize(int x, int y, int w, int h);
		void setSlider(int x, int y);
		void getActualPixel(float paramX, float paramY, int& x, int& y);
		void getActualParam(float& paramX, float& paramY, int x, int y);
		void setPoint(int x, int y);

	private:
		void getBound(int ptX, int ptY, int& x, int& y, int& w, int& h);

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
		ParameterGroup* paramGroup;
};

class ParameterGroup : public fltk::Group
{
	public:
		ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex = false);
		~ParameterGroup();

		virtual void resize(int x, int y, int w, int h);

		static void updateXAxisValue(fltk::Widget* widget, void* data);
		static void updateXYAxisValue(fltk::Widget* widget, void* data);
		static void updateXYZAxisValue(fltk::Widget* widget, void* data);
		void updateWeight();

	public:
		PanimationWindow* paWindow;
		PAStateData* state;
		ParameterVisualization* paramVisualization;
		bool exec;
		fltk::ValueSlider* xAxis;
		fltk::ValueSlider* yAxis;
		fltk::ValueSlider* zAxis;
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

		fltk::Group*		parameterGroup;
		ParameterGroup*		paramGroup;

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
