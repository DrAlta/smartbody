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

#include <FL/Fl_Slider.H>
#include <vhcl.h>
#include "PanimationWindow.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <sr/sr_gl.h>
#include <sr/sr_light.h>
#include <sr/sr_camera.h>
#include <sr/sr_event.h>

class PanimationWindow;
class ParameterGroup;

class Parameter3DVisualization : public Fl_Gl_Window
{
	public:
		Parameter3DVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterGroup* window);
		~Parameter3DVisualization();

		virtual void draw();
		virtual int handle(int event);
		virtual void resize(int x, int y, int w, int h);
		void init_opengl();
		void translate_event(SrEvent& e, SrEvent::EventType t, int w, int h, Parameter3DVisualization* viewer);
		void mouse_event(SrEvent& e);

		// user data
		void drawTetrahedrons();
		void drawGrid();
		void drawParameter();

	public:
		SrCamera cam;
		SrEvent e;
		float gridSize;
		float gridStep;
		float floorHeight;

	private:
		PAStateData* state;
		ParameterGroup* paramGroup;
};

class ParameterVisualization : public Fl_Group
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
		PAStateData* state;					// !!!Careful about this state, this PAState is a pointer to mcu state pool, not the current state to specific character
		ParameterGroup* paramGroup;
};

class ParameterGroup : public Fl_Group
{
	public:
		ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex = false);
		~ParameterGroup();

		virtual void resize(int x, int y, int w, int h);
		
		static void updateXAxisValue(Fl_Widget* widget, void* data);
		static void updateXYAxisValue(Fl_Widget* widget, void* data);
		static void updateXYZAxisValue(Fl_Widget* widget, void* data);
		void updateWeight();
		PAStateData* getCurrentPAStateData();
		SbmCharacter* getCurrentCharacter();

	public:
		PanimationWindow* paWindow;
		PAStateData* state;
		ParameterVisualization* paramVisualization;
		Parameter3DVisualization* param3DVisualization;
		bool exec;
		Fl_Value_Slider* xAxis;
		Fl_Value_Slider* yAxis;
		Fl_Value_Slider* zAxis;
};

class PARunTimeEditor : public Fl_Group
{
	public:
		PARunTimeEditor(int x, int y, int w, int h, PanimationWindow* window);
		~PARunTimeEditor();

	public:
		PanimationWindow*		paWindow;
		Fl_Output*				currentCycleState;
		Fl_Hold_Browser*		nextCycleStates;
		Fl_Hold_Browser*		availableTransitions;
		Fl_Button*				runNextState;
		std::string				prevCycleState;

		Fl_Group*				parameterGroup;
		ParameterGroup*			paramGroup;

	public:		
		void update();
		void updateRunTimeStates(std::string currentState);
		void addItem(Fl_Browser* browser, std::string item);
		void initializeRunTimeEditor();
		static void updateNonCycleState(Fl_Widget* widget, void* data);
		static void updateTransitionStates(Fl_Widget* widget, void* data);
		static void run(Fl_Widget* widget, void* data);
};


#endif
