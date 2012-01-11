#ifndef _PARAMETERVISUALIZATION_H_
#define _PARAMETERVISUALIZATION_H_

#include <FL/Fl_Group.H>
#include <sbm/me_ct_param_animation_data.h>
#include "ParameterGroup.h"

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

#endif
