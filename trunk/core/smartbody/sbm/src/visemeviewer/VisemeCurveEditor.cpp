#include "VisemeCurveEditor.h"

#include <FL/Fl.H>
#include <FL/fl_device.H>
#include <FL/fl_draw.H>
#include <stdlib.h>

VisemeCurveEditor::VisemeCurveEditor(int x, int y, int w, int h, char* name) : Fl_Widget(x, y, w, h)
{
	_pointIsSelected = false;
	_lineIsSelected = false;

	_selectedPoint = -1;
	_selectedLine = -1;

	_gridSizeX = 5;
	_gridSizeY = 5;
}
void VisemeCurveEditor::selectLine(int viseme){
	_lineIsSelected = true;
	_selectedLine = viseme;

	_pointIsSelected = false;
	_selectedPoint = -1;
}

VisemeCurveEditor::~VisemeCurveEditor()
{
}

void VisemeCurveEditor::clear()
{
	fl_color(FL_GRAY);
	fl_rectf(x(), y(), w(), h());
}

void VisemeCurveEditor::draw()
{
	clear();

	drawGrid();
	drawPoints();
	drawCurve();
}


int VisemeCurveEditor::handle(int event)
{
	int mousex = Fl::event_x();
	int mousey = Fl::event_y();

	switch (event)
	{
		case FL_PUSH:
		{
			int isButton1Pressed = Fl::event_state(FL_BUTTON1);
			int isButton2Pressed = Fl::event_state(FL_BUTTON3);

			if (isButton1Pressed )
			{			
				bool pointIsSelected = isPointSelected(mousex, mousey);

				if(pointIsSelected)
				{
					_pointIsSelected = true;
				}
				else
				{
					if (_lineIsSelected)
					{
						int insertIndx = getInsertionIndex(mousex, mousey);

						SrVec point((float) mousex, (float) mousey, 0.f);
						
						_curves[_selectedLine].insert(_curves[_selectedLine].begin() + insertIndx, point);
						_selectedPoint = insertIndx;
						_pointIsSelected = true;
						// draw a point in that location
						redraw();
					}
				}
			}
			else if (isButton2Pressed)
			{
				bool pointIsSelected = isPointSelected(mousex, mousey);
				
				if(pointIsSelected)
				{
					_curves[_selectedLine].erase(_curves[_selectedLine].begin() + _selectedPoint);
					_pointIsSelected = false;
					_selectedPoint = -1;
				}
				redraw();
			}

			return 1;
			break;
		}

		case FL_DRAG:
		{
			if(_pointIsSelected)
			{
				SrVec& point = _curves[_selectedLine][_selectedPoint];

				// Enforce Constraints
				float minx = (float)x();
				float miny = (float)y();
				float maxx = (float)(x() + w());
				float maxy = (float)(y() +h());

				if(_selectedPoint > 0)
					minx =  _curves[_selectedLine][_selectedPoint - 1].x;
				if(_selectedPoint < (int)_curves[_selectedLine].size() - 1)
					maxx =  _curves[_selectedLine][_selectedPoint + 1].x;

				if(mousex > maxx)
					break;
				else if(mousex< minx )
					break;
				else if(mousey > maxy)
					break;
				else if(mousey < miny )
					break;
				else
				{
					point.x = (float)mousex;
					point.y = (float)mousey;
				}

				redraw();
			}
			break;
		}

		case FL_RELEASE:
		{
			//_pointIsSelected = false;
		}
	}
	return Fl_Widget::handle(event);
}

bool VisemeCurveEditor::isPointSelected(int mousex, int mousey)
{
	SrVec mouseClick((float) mousex, (float) mousey, 0.f);
	_pointIsSelected = false;
	for (int i = 0; i < (int)_curves.size(); i++)
	{
		if(!_curves[i].isVisible())
			continue;

		for(int j = 0; j < (int)_curves[i].size(); j++)
		{	
			SrVec& point = _curves[i][j];
			float distance = dist(mouseClick, point);
			
			if(distance <= _curves[i].getPointRadius())
			{
				_pointIsSelected = true;
				_lineIsSelected = true;

				_selectedPoint = j;
				_selectedLine = i;

				break;
			}
		}
	}

	return _pointIsSelected;
}

int VisemeCurveEditor::getInsertionIndex(int mousex, int mousey)
{
	SrVec mouseClick((float) mousex, (float) mousey, 0.f);
	
	int indx = 0;
	if(_curves.size() == 0 || !_lineIsSelected)
		return 0;

	if(mouseClick.x < _curves[_selectedLine][0].x)
		return 0;

	for (indx = 0; indx < (int)_curves[_selectedLine].size() - 1; indx++)
	{	
		SrVec& point1 = _curves[_selectedLine][indx];
		SrVec& point2 = _curves[_selectedLine][indx + 1];
	
		if(mouseClick.x > point1.x && mouseClick.x < point2.x)
			break;
	}

	return (indx + 1);
}

void VisemeCurveEditor::drawPoints()
{
	fl_begin_complex_polygon();

	// draw points
	for (int i = 0; i < (int)_curves.size(); i++)
	{
		fl_color(_curves[i].getPointColor());

		if(!_curves[i].isVisible())
			continue;

		for(int j = 0; j < (int)_curves[i].size(); j++)
		{
			SrVec& point = _curves[i][j];

			if(j == _selectedPoint && i == _selectedLine)
				continue;

			fl_circle(point.x, point.y, _curves[i].getPointRadius());
		}
	}

	// draw selected point
	if(_pointIsSelected)
	{
		SrVec& point = _curves[_selectedLine][_selectedPoint];

		fl_color(_curves[_selectedLine].getPointColor() - 30);
		fl_circle(point.x, point.y, _curves[_selectedLine].getPointRadius() + 3);
	}

	fl_end_complex_polygon();
}

void VisemeCurveEditor::drawCurve()
{
	// connect the points with a line
	SrVec lastPoint;
	for (int i = 0; i < (int)_curves.size(); i++)
	{
		fl_color(_curves[i].getLineColor());

		if(!_curves[i].isVisible())
			continue;

		for(int j = 0; j < (int)_curves[i].size(); j++)
		{
			if (j == 0)
			{
				lastPoint =_curves[i][0];
					continue;
			}
				SrVec& point = _curves[i][j];
				fl_line((int) lastPoint.x, (int) lastPoint.y, (int) point.x, (int) point.y);
				lastPoint = point;
		}
	}
}
void VisemeCurveEditor::drawGrid()
{
	int centerX = x() + w() / 2;
	int centerY = y() + h() / 2;
	int ymax = y();
	int ymin = y() + h();
	int xmin = x();
	int xmax = x() + w();

	// draw grid
	fl_color(FL_WHITE);
	int numLinesX = w() / _gridSizeX;
	for (int i = -numLinesX / 2; i <= numLinesX / 2; i++)
		fl_line(centerX + i *  _gridSizeX, ymin, centerX + i *  _gridSizeX, ymax);
	int numLinesY = h() /  _gridSizeY;
	for (int i = -numLinesY / 2; i <=  numLinesY / 2; i++)
		fl_line(xmin, centerY + i * _gridSizeY, xmax, centerY + i *  _gridSizeY);
}

void VisemeCurveEditor::setVisibility(int viseme, bool isVisible)
{
	_curves[viseme].setVisibilty( isVisible);

}

void VisemeCurveEditor::generateCurves(int count)
{
	for (int i = 0; i < count; i++)
	{
		VisemeCurve curve;

		for(int j = 0; j < 5; j++)
		{
			SrVec point( rand() % 25 + (float)(x() + j * 70), (float)( y() + h()/2.0f + rand() % 70), (float)0);
			curve.push_back(point);
		}
		curve.SetLineColor( rand() % 256);
		curve.SetPointColor( rand() % 256);

		_curves.push_back(curve);
	}
}
