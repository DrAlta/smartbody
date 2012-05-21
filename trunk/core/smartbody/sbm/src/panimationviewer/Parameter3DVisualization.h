#ifndef _PARAMETER3DVISUALIZATION_H_
#define _PARAMETER3DVISUALIZATION_H_

#include <FL/Fl_Group.H>
#include <FL/Fl_Gl_Window.H>
#include <controllers/me_ct_param_animation_data.h>
#include "ParameterGroup.h"
#include <sr/sr_camera.h>

class Parameter3DVisualization : public Fl_Gl_Window
{
	public:
		Parameter3DVisualization(int x, int y, int w, int h, char* name, PABlendData* s, ParameterGroup* window);
		~Parameter3DVisualization();

		virtual void draw();
		virtual int handle(int event);
		virtual void resize(int x, int y, int w, int h);
		void init_opengl();
		void translate_event(SrEvent& e, SrEvent::EventType t, int w, int h, Parameter3DVisualization* viewer);
		void mouse_event(SrEvent& e);

		// user data
		void setSelectedTetrahedrons(std::vector<bool>& selected);
		void setSelectedParameters(std::vector<bool>& selected);
		void drawTetrahedrons();
		void drawGrid();
		void drawParameter();
		SrVec determineScale();

	public:
		SrCamera cam;
		SrEvent e;
		float gridSize;
		float gridStep;
		float floorHeight;
		std::vector<bool> selectedTetrahedrons;
		std::vector<bool> selectedParameters;
		int lastMouseX;
		int lastMouseY;

	private:
		PABlendData* blendData;
		ParameterGroup* paramGroup;
		std::vector<SrVec> tet;
};

#endif
