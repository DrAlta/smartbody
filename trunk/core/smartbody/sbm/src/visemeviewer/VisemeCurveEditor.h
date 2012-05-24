#ifndef _VISEMECURVEEDITOR_H_
#define _VISEMECURVEEDITOR_H_

#include <FL/Fl_Widget.H>
#include <vector>
#include <sr/sr_vec.h>
#include "VisemeCurve.h"

class VisemeCurveEditor : public Fl_Widget
{
	public:
		VisemeCurveEditor(int x, int y, int w, int h, char* name);
		~VisemeCurveEditor();

		void draw();
		void clear();
		int handle(int event);
		
		void setVisibility(int viseme, bool isVisible);

		//Temporary
		void generateCurves(int count);

	protected:
		std::vector<VisemeCurve> _curves;
		
		int _selectedPoint;
		int _selectedLine;
		
		int _gridSizeX;
		int _gridSizeY;

		bool _pointIsSelected;
		bool _lineIsSelected;

		bool isPointSelected(int mousex, int mousey);
		int getInsertionIndex(int mousex, int mousey);

		void drawPoints();
		void drawCurve();
		void drawGrid();
};

#endif