#include "ParameterVisualization.h"

ParameterVisualization::ParameterVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterGroup* group) : Fl_Group(x, y, w, h, name), state(s), paramGroup(group)
{
	paramX = -9999;
	paramY = -9999;
	setup();
}

ParameterVisualization::~ParameterVisualization()
{
}

void ParameterVisualization::draw()
{
	Fl_Group::draw();

	setup();
	int xmin = centerX - width/2;
	int xmax = centerX + width/2;
	int ymin = centerY - height/2;
	int ymax = centerY + height/2;
	fl_rectf(xmin,ymin,width,height,FL_GRAY); // manually clean up the drawing area
	
	// draw axis
	fl_color(FL_BLACK);
	fl_line(xmin, centerY, xmax, centerY);
	fl_line(centerX, ymin, centerX, ymax);
	int recX, recY, recW, recH;
	getBound(centerX, centerY, recX, recY, recW, recH);	
	fl_rectf(recX, recY, recW, recH);

	// draw grid
	fl_color(FL_WHITE);
	int numLinesX = width / gridSizeX;
	for (int i = -numLinesX / 2; i <= numLinesX / 2; i++)
		fl_line(centerX + i * gridSizeX, ymin, centerX + i * gridSizeX, ymax);
	int numLinesY = height / gridSizeY;
	for (int i = -numLinesY / 2; i <=  numLinesY / 2; i++)
		fl_line(xmin, centerY + i * gridSizeY, xmax, centerY + i * gridSizeY);

	// draw parameters
	fl_color(FL_GREEN);
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		int recX, recY, recW, recH;
		SrVec vec = state->paramManager->getVec(i);
		int x = 0;
		if (fabs(scaleX) > 0.0001)
			x = int(vec.x / scaleX);
		int y = 0;
		if (fabs(scaleY) > 0.0001)
			y = int(vec.y / scaleY);
		getBound(centerX + x, centerY - y, recX, recY, recW, recH);
		fl_rectf(recX, recY, recW, recH);
	}

	// draw lines connecting parameters
	for (int i = 0; i < state->paramManager->getNumTriangles(); i++)
	{
		SrVec vec1 = state->paramManager->getTriangle(i).a;
		SrVec vec2 = state->paramManager->getTriangle(i).b;
		SrVec vec3 = state->paramManager->getTriangle(i).c;
		int x1, y1, x2, y2, x3, y3;
		getActualPixel(vec1.x, vec1.y, x1, y1);
		getActualPixel(vec2.x, vec2.y, x2, y2);
		getActualPixel(vec3.x, vec3.y, x3, y3);
		fl_line(x1, y1, x2, y2);
		fl_line(x1, y1, x3, y3);
		fl_line(x3, y3, x2, y2);
	}

	// draw parameters info
	fl_color(FL_BLACK);
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		SrVec vec = state->paramManager->getVec(i);
		int x = int(vec.x / scaleX);
		int y = int(vec.y / scaleY);
		char buff[200];
//		sprintf(buff, "%s(%d,%d)", state->paramManager->getMotionName(i).c_str(), x, y);
		sprintf(buff, "%s", state->paramManager->getMotionName(i).c_str());
		Fl_Font prevFont = fl_font();
		Fl_Fontsize prevSize = fl_size();

		fl_font(FL_COURIER,12);		
		fl_draw(buff, centerX + x, centerY - 10);
		// restore the previous font size
		fl_font(prevFont,prevSize);
	}

	// draw parameter
	fl_color(FL_RED);
	if (paramX != -9999 && paramY != -9999)
	{
		int recX, recY, recW, recH;
		getBound(paramX, paramY, recX, recY, recW, recH);
		fl_rectf(recX, recY, recW, recH);		
	}
}

int ParameterVisualization::handle(int event)
{
	int mousex = Fl::event_x();
	int mousey = Fl::event_y();
	switch (event)
	{
		case FL_DRAG:
		{
			bool altKeyPressed = (Fl::event_state(FL_BUTTON1) || Fl::event_state(FL_BUTTON3));
			if (altKeyPressed)
			{
				paramX = mousex;
				paramY = mousey;				
				setSlider(paramX, paramY);
				//redraw();
				break;
			}
		}
	}
	if (event == FL_PUSH)
		return 1;
	return Fl_Group::handle(event);
}

void ParameterVisualization::setup()
{
	centerX = w() / 2  + x();//paramGroup->x();
	centerY = h() / 2  + y();//paramGroup->y();
	width = w();
	height = h();

	SrVec vec = state->paramManager->getVec(state->paramManager->getMaxVecX());
	float maxX = vec.x;
	vec = state->paramManager->getVec(state->paramManager->getMinVecX());
	float minX = vec.x;
	if (fabs(maxX) < fabs(minX)) maxX = minX;
	vec = state->paramManager->getVec(state->paramManager->getMaxVecY());
	float maxY = vec.y;
	vec = state->paramManager->getVec(state->paramManager->getMinVecY());
	float minY = vec.y;
	if (fabs(maxY) < fabs(minY)) maxY = minY;
	scaleX = fabs(maxX * 3 / (float)width);
	scaleY = fabs(maxY * 3 / (float)height);
}

void ParameterVisualization::resize(int x, int y, int w, int h)
{
	Fl_Group::resize(x, y, w, h);
	setup();
	redraw();
}

void ParameterVisualization::getBound(int ptX, int ptY, int& x, int& y, int& w, int& h)
{
	x = ptX - pad;
	y = ptY - pad;
	w = 2 * pad;
	h = 2 * pad;
}

// input actual parameter, returning pixel on the screen
void ParameterVisualization::getActualPixel(float paramX, float paramY, int& x, int& y)
{
	if (fabs(scaleX) > 0.0001)
		x = int(paramX / scaleX);
	else
		x = int(paramX);
	x = centerX + x;
	if (fabs(scaleY) > 0.0001)
		y = int(paramY / scaleY);
	else
		y = int(paramY);
	y = centerY - y;
}

void ParameterVisualization::getActualParam(float& paramX, float& paramY, int x, int y)
{
	paramX = (x - centerX) * scaleX;
	paramY = (centerY - y) * scaleY;
}


// set the parameter location on the screen
void ParameterVisualization::setPoint(int x, int y)
{
	paramX = x;
	paramY = y;
	redraw();
}

// given mouse position on the screen, set the parameter and slider (slider shows the value of parameter)
void ParameterVisualization::setSlider(int x, int y)
{
	float valueX, valueY;
	getActualParam(valueX, valueY, x, y);
	if (state->paramManager->getType() == 0)
		state->paramManager->setWeight(valueX);
	if (state->paramManager->getType() == 1)
		state->paramManager->setWeight(valueX, valueY);
	paramGroup->updateWeight();
	float actualParamX, actualParamY;
	state->paramManager->getParameter(actualParamX, actualParamY);
	this->getActualPixel(actualParamX, actualParamY, paramX, paramY);
	paramGroup->xAxis->value(actualParamX);
	if (paramGroup->yAxis)
		paramGroup->yAxis->value(actualParamY);
	redraw();
}
