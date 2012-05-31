#ifndef _VISEMECURVEEDITOR_H_
#define _VISEMECURVEEDITOR_H_

#include <FL/Fl_Widget.H>
#include <vector>
#include <sr/sr_vec.h>
#include "VisemeCurve.h"
#include "VisemeViewerWindow.h"


class VisemeViewerWindow;
class VisemeCurveEditor : public Fl_Widget
{
	public:
		VisemeCurveEditor(int x, int y, int w, int h, char* name);
		~VisemeCurveEditor();

		void draw();
		void clear();
		int handle(int event);
		
		void setVisemeWindow(VisemeViewerWindow* w);
		void setVisibility(int viseme, bool isVisible);
		void selectLine(int viseme);
		//Temporary
		void generateCurves(int count);
		void changeCurve(int viseme, std::vector<float>& curveData);
		SrVec mapCurveData(SrVec& origData);
		SrVec mapDrawData(SrVec& origData);
		std::vector<VisemeCurve>& getCurves();

	protected:
		std::vector<VisemeCurve> _curves;
		
		VisemeViewerWindow* visemeWindow;

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
		void drawName();
};

#endif