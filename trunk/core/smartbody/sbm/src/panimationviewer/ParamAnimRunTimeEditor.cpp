/*
 *  ParamAnimRunTimeEditor.cpp - part of SmartBody-lib's Test Suite
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

#include "ParamAnimRunTimeEditor.h"
#include <fltk/gl.h>
#include <GL/glu.h>
#include <fltk/draw.h>
#include <sbm/mcontrol_util.h>
#include <sbm/me_ct_param_animation_data.h>


# define ROTATING2(e)	(e.alt && e.button1)
# define DOLLYING(e)	(e.alt && e.button3)
# define TRANSLATING(e)	(e.alt && e.button2)

Parameter3DVisualization::Parameter3DVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterGroup* window) : fltk::GlWindow(x, y, w, h), state(s), paramGroup(window)
{
	cam.center.set(0, 0, 0);
	cam.eye.set(300, -300, 400);
	cam.up.set(0, 0, 1);
	gridSize = 700;
	gridStep = 40;
	floorHeight = 0;
}

Parameter3DVisualization::~Parameter3DVisualization()
{
}

void Parameter3DVisualization::draw()
{
	if (!visible()) 
		return;
	if (!valid()) 
	{
		init_opengl();
	}

	//----- Clear Background --------------------------------------------
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//----- Set Projection ----------------------------------------------
	SrMat mat;
	cam.aspect = (float)w()/(float)h();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(cam.get_perspective_mat(mat));

	//----- Set Visualisation -------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrix(cam.get_view_mat(mat));
	glScalef(cam.scale, cam.scale, cam.scale);
	SrLight light;
//	light.directional = false;
	light.directional = true;
	light.diffuse = SrColor( 1.0f, 0.95f, 0.8f );
	light.position = SrVec( 100.0, 250.0, 400.0 );
//	light.constant_attenuation = 1.0f/cam.scale;
	light.constant_attenuation = 1.0f;

	SrLight light2 = light;
	light2.directional = false;
	light2.diffuse = SrColor( 0.8f, 0.85f, 1.0f );
	light2.position = SrVec( 100.0, 500.0, -200.0 );
//	light2.constant_attenuation = 1.0f;
//	light2.linear_attenuation = 2.0f;
	glEnable(GL_LIGHTING);
	glLight( 0, light );
	glLight( 1, light2 );

	static GLfloat mat_emissin[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_ambient[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_diffuse[] = { 1.0,  1.0,    1.0,    1.0 };
	static GLfloat mat_speclar[] = { 0.0,  0.0,    0.0,    1.0 };
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emissin );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_speclar );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );
	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_NORMALIZE );
	glDisable( GL_COLOR_MATERIAL );

	//----- Render user scene -------------------------------------------	
	// draw axis
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex(SrVec(gridSize, 0, 0));
		glVertex(SrVec(-gridSize, 0, 0));
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex(SrVec(0, gridSize, 0));
		glVertex(SrVec(0, -gridSize, 0));
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex(SrVec(0, 0, gridSize));
		glVertex(SrVec(0, 0, -gridSize));
	glEnd();

	drawGrid();
	drawParameter();

	// others
	drawTetrahedrons();
}

int Parameter3DVisualization::handle(int event)
{
	switch ( event )
	{ 
	case fltk::PUSH:
		translate_event ( e, SrEvent::Push, w(), h(), this );
		break;

	case fltk::RELEASE:
		translate_event ( e, SrEvent::Release, w(), h(), this);
		break;

	case fltk::DRAG:

	case fltk::MOVE:
		translate_event ( e, SrEvent::Drag, w(), h(), this );
		break;

	case fltk::WHEN_RELEASE:
		//translate_event ( e, SrEvent::Release, w(), h(), this);
		break;

	case fltk::KEY:
		break;

	case fltk::HIDE: // Called when the window is iconized
		break;

	case fltk::SHOW: // Called when the window is de-iconized or when show() is called
		show ();
		break;

	  default:
		  break;
	}

	mouse_event(e);

	return GlWindow::handle(event);
}

void Parameter3DVisualization::init_opengl()
{
	// valid() is turned on by fltk after draw() returns
	glViewport ( 0, 0, w(), h() );
	glEnable ( GL_DEPTH_TEST );
	glEnable ( GL_LIGHT0 ); 
	glEnable ( GL_LIGHTING );

	//glEnable ( GL_BLEND ); // for transparency
	//glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LEQUAL );
	glFrontFace ( GL_CCW );

	glEnable ( GL_POLYGON_SMOOTH );

	//glEnable ( GL_LINE_SMOOTH );
	//glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );

	glEnable ( GL_POINT_SMOOTH );
	glPointSize ( 2.0 );

	glShadeModel ( GL_SMOOTH );	
}

void Parameter3DVisualization::translate_event(SrEvent& e, SrEvent::Type t, int w, int h, Parameter3DVisualization* viewer)
{
	e.init_lmouse ();

	// put coordinates inside [-1,1] with (0,0) in the middle :
	e.mouse.x  = ((float)fltk::event_x())*2.0f / ((float)w) - 1.0f;
	e.mouse.y  = ((float)fltk::event_y())*2.0f / ((float)h) - 1.0f;
	e.mouse.y *= -1.0f;
	e.width = w;
	e.height = h;
	e.mouseCoord.x = (float)fltk::event_x();
	e.mouseCoord.y = (float)fltk::event_y();

	if ( fltk::event_state(fltk::BUTTON1) ) 
	   e.button1 = 1;

	if ( fltk::event_state(fltk::BUTTON2) ) 
	   e.button2 = 1;

	if ( fltk::event_state(fltk::BUTTON3) ) 
	   e.button3 = 1;


	if(e.button1 == 0 && e.button2 == 0 && e.button3 == 0) 
	{
	   t = SrEvent::Release;
	}

	e.type = t;

	if ( t==SrEvent::Push)
	{
	   e.button = fltk::event_button();
	   e.origUp = viewer->cam.up;
	   e.origEye = viewer->cam.eye;
	   e.origCenter = viewer->cam.center;
	   e.origMouse.x = e.mouseCoord.x;
	   e.origMouse.y = e.mouseCoord.y;
	}
	else if (t==SrEvent::Release )
	{
	   e.button = fltk::event_button();
	   e.origMouse.x = -1;
	   e.origMouse.y = -1;
	}


	if ( fltk::event_state(fltk::ALT)   ) e.alt = 1;
	else e.alt = 0;
	if ( fltk::event_state(fltk::CTRL)  ) e.ctrl = 1;
	else e.ctrl = 0;
	if ( fltk::event_state(fltk::SHIFT) ) e.shift = 1;
	else e.shift = 0;

	e.key = fltk::event_key();	
}

SrVec rotate_point(SrVec point, SrVec origin, SrVec direction, float angle)
{
	float originalLength = point.len();

	SrVec v = direction;
	SrVec o = origin;
	SrVec p = point;
	float c = cos(angle);
	float s = sin(angle);
	float C = 1.0f - c;

	SrMat mat;
	mat.e11() = v[0] * v[0] * C + c;
	mat.e12() = v[0] * v[1] * C - v[2] * s;
	mat.e13() = v[0] * v[2] * C + v[1] * s;
	mat.e21() = v[1] * v[0] * C + v[2] * s;
	mat.e22() = v[1] * v[1] * C + c;
	mat.e23() = v[1] * v[2] * C - v[0] * s;
	mat.e31() = v[2] * v[0] * C - v[1] * s;
	mat.e32() = v[2] * v[1] * C + v[0] * s;
	mat.e33() = v[2] * v[2] * C + c;

	mat.transpose();

	SrVec result = origin + mat * (point - origin);

	return result;
}

void Parameter3DVisualization::mouse_event(SrEvent& e)
{
	if ( e.type==SrEvent::Drag )
	{ 
		float dx = e.mousedx() * cam.aspect;
		float dy = e.mousedy() / cam.aspect;
		if ( DOLLYING(e) )
		{ 
			float amount = dx;
			SrVec cameraPos(cam.eye);
			SrVec targetPos(cam.center);
			SrVec diff = targetPos - cameraPos;
			float distance = diff.len();
			diff.normalize();

			if (amount >= distance)
				amount = distance - .000001f;

			SrVec diffVector = diff;
			SrVec adjustment = diffVector * distance * amount;
			cameraPos += adjustment;
			SrVec oldEyePos = cam.eye;
			cam.eye = cameraPos;
			SrVec cameraDiff = cam.eye - oldEyePos;
			cam.center += cameraDiff;
			redraw();
		}
		else if ( TRANSLATING(e) )
		{ 
			cam.apply_translation_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y );
			redraw();
		}
		else if ( ROTATING2(e) )
		{ 
			float deltaX = -(e.mouseCoord.x - e.origMouse.x) / e.width;
			float deltaY = -(e.mouseCoord.y -  e.origMouse.y) / e.height;
			if (deltaX == 0.0 && deltaY == 0.0)
				return;

			SrVec origUp = e.origUp;
			SrVec origCenter = e.origCenter;
			SrVec origCamera = e.origEye;

			SrVec dirX = origUp;
			SrVec  dirY;
			dirY.cross(origUp, (origCenter - origCamera));
			dirY /= dirY.len();

			SrVec camera = rotate_point(origCamera, origCenter, dirX, -deltaX * float(M_PI));
			camera = rotate_point(camera, origCenter, dirY, deltaY * float(M_PI));

			cam.eye = camera;
			redraw();
		}
	}
}

void Parameter3DVisualization::drawTetrahedrons()
{
	glColor3f(1.0f, 1.0f, 0.0f);
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < state->paramManager->getParameters().size(); i++)
	{
		SrVec param = state->paramManager->getParameters()[i];
		glVertex(param);
	}		
	glEnd();

	glColor3f(0.2f, 0.5f, 0.9f);
	glBegin(GL_LINES);
	glLineWidth(0.8f);
	for (unsigned int i = 0; i < state->paramManager->getTetrahedrons().size(); i++)
	{
		TetrahedronInfo info = state->paramManager->getTetrahedrons()[i];
		glVertex(info.v1);
		glVertex(info.v2);
		glVertex(info.v1);
		glVertex(info.v3);
		glVertex(info.v1);
		glVertex(info.v4);
		glVertex(info.v2);
		glVertex(info.v3);
		glVertex(info.v2);
		glVertex(info.v4);
		glVertex(info.v3);
		glVertex(info.v4);
	}
	glEnd();

	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(0.2f, 0.5f, 0.9f);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < state->paramManager->getTetrahedrons().size(); i++)
	{
		TetrahedronInfo info = state->paramManager->getTetrahedrons()[i];
		glVertex(info.v1);
		glVertex(info.v2);
		glVertex(info.v3);
		glVertex(info.v1);
		glVertex(info.v2);
		glVertex(info.v4);
		glVertex(info.v1);
		glVertex(info.v3);
		glVertex(info.v4);
		glVertex(info.v2);
		glVertex(info.v3);
		glVertex(info.v4);
	}
	glEnd();
	glDisable(GL_BLEND);
	*/
}

void Parameter3DVisualization::drawGrid()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);

	glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1);
	glBegin(GL_LINES);

	for (float x = -gridSize; x <= gridSize; x += gridStep)
	{
		glVertex3f(x, -gridSize, floorHeight);
		glVertex3f(x, gridSize, floorHeight);
	}

	for (float x = -gridSize; x <= gridSize; x += gridStep)
	{
		glVertex3f(-gridSize, x, floorHeight);
		glVertex3f(gridSize, x, floorHeight);
	}

	glEnd();
	glDisable(GL_BLEND);
	glPopAttrib();
}

void Parameter3DVisualization::drawParameter()
{
	SrVec vec = state->paramManager->getPrevVec();
	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(5.0f);
	glBegin(GL_POINTS);
		glVertex(vec);
	glEnd();
}

ParameterVisualization::ParameterVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterGroup* group) : fltk::Group(x, y, w, h, name), state(s), paramGroup(group)
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
	fltk::Group::draw();

	setup();

	// draw axis
	fltk::setcolor(fltk::BLACK);
	fltk::drawline(0, centerY, width, centerY);
	fltk::drawline(centerX, 0, centerX, height);
	int recX, recY, recW, recH;
	getBound(centerX, centerY, recX, recY, recW, recH);	
	fltk::fillrect(recX, recY, recW, recH);

	// draw grid
	fltk::setcolor(fltk::WHITE);
	int numLinesX = width / gridSizeX;
	for (int i = -numLinesX / 2; i <= numLinesX / 2; i++)
		fltk::drawline(centerX + i * gridSizeX, 0, centerX + i * gridSizeX, height);
	int numLinesY = height / gridSizeY;
	for (int i = -numLinesY / 2; i <=  numLinesY / 2; i++)
		fltk::drawline(0, centerY + i * gridSizeY, width, centerY + i * gridSizeY);

	// draw parameters
	fltk::setcolor(fltk::GREEN);
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
		fltk::fillrect(recX, recY, recW, recH);
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
		fltk::drawline(x1, y1, x2, y2);
		fltk::drawline(x1, y1, x3, y3);
		fltk::drawline(x3, y3, x2, y2);
	}

	// draw parameters info
	fltk::setcolor(fltk::BLACK);
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		SrVec vec = state->paramManager->getVec(i);
		int x = int(vec.x / scaleX);
		int y = int(vec.y / scaleY);
		char buff[200];
//		sprintf(buff, "%s(%d,%d)", state->paramManager->getMotionName(i).c_str(), x, y);
		sprintf(buff, "%s", state->paramManager->getMotionName(i).c_str());
		fltk::drawtext(buff, float(centerX + x), float(centerY - y));
	}

	// draw parameter
	fltk::setcolor(fltk::RED);
	if (paramX != -9999 && paramY != -9999)
	{
		int recX, recY, recW, recH;
		getBound(paramX, paramY, recX, recY, recW, recH);
		fltk::fillrect(recX, recY, recW, recH);		
	}
}

int ParameterVisualization::handle(int event)
{
	int mousex = fltk::event_x();
	int mousey = fltk::event_y();
	switch (event)
	{
		case fltk::MOVE:
		{
			bool altKeyPressed = (fltk::get_key_state(fltk::LeftAltKey) || fltk::get_key_state(fltk::RightAltKey));
			if (altKeyPressed)
			{
				paramX = mousex;
				paramY = mousey;
				setSlider(paramX, paramY);
				redraw();
				break;
			}
		}
	}
	return fltk::Group::handle(event);
}

void ParameterVisualization::setup()
{
	centerX = w() / 2;
	centerY = h() / 2;
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
	fltk::Group::resize(x, y, w, h);
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

ParameterGroup::ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex) : fltk::Group(x, y, w, h, name), state(s), paWindow(window), exec(ex)
{
	this->label(s->stateName.c_str());
	this->begin();
		int type = state->paramManager->getType();
		if (type == 0)
		{
			paramVisualization = new ParameterVisualization(4 * xDis, yDis, w - 5 * xDis, h - 5 * yDis, "", s, this);
			this->resizable(paramVisualization);
			yAxis = NULL;
			zAxis = NULL;
			double min = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double max = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			xAxis = new fltk::ValueSlider(4 * xDis, h - 4 * yDis, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(min);
			xAxis->maximum(max);
			xAxis->value(min);
			xAxis->callback(updateXAxisValue, this);
			float actualValue;
			s->paramManager->getParameter(actualValue);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValue, 0.0f, actualX, actualY);
			paramVisualization->setSlider(actualX, actualY);

			param3DVisualization = NULL;
		}
		if (type == 1)
		{
			paramVisualization = new ParameterVisualization(4 * xDis, yDis, w - 5 * xDis, h - 5 * yDis, "", s, this);
			this->resizable(paramVisualization);
			double minX = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double maxX = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			double minY = state->paramManager->getVec(state->paramManager->getMinVecY()).y;
			double maxY = state->paramManager->getVec(state->paramManager->getMaxVecY()).y;
			xAxis = new fltk::ValueSlider(4 * xDis, h - 4 * yDis, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(minX);
			xAxis->maximum(maxX);
			xAxis->callback(updateXYAxisValue, this);
			yAxis = new fltk::ValueSlider(xDis, yDis, 3 * xDis, h - 5 * yDis, "Y");
			yAxis->minimum(minY);
			yAxis->maximum(maxY);
			yAxis->callback(updateXYAxisValue, this);
			yAxis->set_vertical();
			float actualValueX, actualValueY;
			s->paramManager->getParameter(actualValueX, actualValueY);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValueX, actualValueY, actualX, actualY);
			paramVisualization->setSlider(actualX, actualY);

			param3DVisualization = NULL;
		}
		if (type == 2)
		{
			param3DVisualization = new Parameter3DVisualization(4 * xDis, 4 * yDis, w - 5 * xDis, h - 8 * yDis, "", s, this);
			this->resizable(param3DVisualization);
			paramVisualization = NULL;
			double minX = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double maxX = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			double minY = state->paramManager->getVec(state->paramManager->getMinVecY()).y;
			double maxY = state->paramManager->getVec(state->paramManager->getMaxVecY()).y;
			xAxis = new fltk::ValueSlider(4 * xDis, h - 4 * yDis, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(minX);
			xAxis->maximum(maxX);
			xAxis->callback(updateXYZAxisValue, this);
			yAxis = new fltk::ValueSlider(xDis, yDis, 3 * xDis, h - 5 * yDis, "Y");
			yAxis->minimum(minY);
			yAxis->maximum(maxY);
			yAxis->callback(updateXYZAxisValue, this);
			yAxis->set_vertical();
			zAxis = new fltk::ValueSlider(4 * xDis, yDis, w - 5 * xDis, 2 * yDis, "Z");
			zAxis->minimum(-90);	// TODO: remove this hard code part
			zAxis->maximum(90);
			zAxis->callback(updateXYZAxisValue, this);
		}
	this->end();
	
}

void ParameterGroup::resize(int x, int y, int w, int h)
{
	Group::resize(x, y, w, h);
}


ParameterGroup::~ParameterGroup()
{
}

void ParameterGroup::updateXAxisValue(fltk::Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	double w = group->xAxis->value();
	group->state->paramManager->setWeight(w);
	if (group->exec)
		group->updateWeight();
	int x, y;
	group->paramVisualization->getActualPixel(float(w), 0.0f, x, y);
	group->paramVisualization->setPoint(x, y);
}

void ParameterGroup::updateXYAxisValue(fltk::Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	group->state->paramManager->setWeight(x, y);
	if (group->exec)
		group->updateWeight();
	int pixelX, pixelY;
	group->paramVisualization->getActualPixel(float(x), float(y), pixelX, pixelY);
	group->paramVisualization->setPoint(pixelX, pixelY);
}

void ParameterGroup::updateXYZAxisValue(fltk::Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	double z = group->zAxis->value();
	bool success = false;
	success = group->state->paramManager->setWeight(x, y, z);
	if (success)
		group->updateWeight();
	group->redraw();
}

void ParameterGroup::updateWeight()
{
	if (!state->cycle)
		return;
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	std::stringstream command;
	command << "panim update char " << charName;
	int wNumber = state->getNumMotions();
	if (wNumber == 1)
		state->weights[0] = 1.0;
	for (int j = 0; j < wNumber; j++)
		command << " " << state->weights[j];
	paWindow->execCmd(paWindow, command.str());
}

PARunTimeEditor::PARunTimeEditor(int x, int y, int w, int h, PanimationWindow* window) : fltk::Group(x, y, w, h), paWindow(window)
{
	this->label("Run Time Editor");
	this->begin();
		currentCycleState = new fltk::Output(2 * xDis + 100, yDis, 100, 2 * yDis, "Current State");
		nextCycleStates = new fltk::Browser(2 * xDis, 5 * yDis, w / 2 - 4 * xDis, h / 4, "Next State");
		nextCycleStates->callback(updateTransitionStates, this);

	
		availableTransitions = new fltk::Browser(w / 2 + 2 * xDis, 5 * yDis, w / 2 - 4 * xDis, h / 4, "Available Transitions");
		availableTransitions->callback(updateNonCycleState, this);
		availableTransitions->when(fltk::WHEN_ENTER_KEY_ALWAYS);
		runNextState = new fltk::Button(2 * xDis, h / 4 + 6 * yDis, 100, 2 * yDis, "Run");
		runNextState->callback(run, this);
		parameterGroup = new fltk::Group(2 * xDis, h / 4 + 9 * yDis, w - 2 * xDis, 3 * h / 4 - 10 * yDis);
		parameterGroup->box(fltk::UP_BOX);
	this->end();
	this->resizable(parameterGroup);
	paramGroup = NULL;
	initializeRunTimeEditor();
}

PARunTimeEditor::~PARunTimeEditor()
{
}

void PARunTimeEditor::update()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	SbmCharacter* character = mcuCBHandle::singleton().character_map.lookup(charName.c_str());
	if (!character)
		return;
	if (!character->param_animation_ct)
		return;
	std::string currentState = character->param_animation_ct->getCurrentStateName();
	if (prevCycleState != currentState)
	{
		updateRunTimeStates(currentState);
		prevCycleState = currentState;
		currentCycleState->value(currentState.c_str());
	}

	if (paramGroup)
	{
		PAStateData* state = character->param_animation_ct->getCurrentPAStateData();
		if (state)
		{
			if (state->cycle)
			{
				float x, y;
				state->paramManager->getParameter(x, y);
				int actualPixelX = 0;
				int actualPixelY = 0;
				if (paramGroup->paramVisualization)
				{
					paramGroup->paramVisualization->getActualPixel(x, y, actualPixelX, actualPixelY);
					paramGroup->paramVisualization->setPoint(actualPixelX, actualPixelY);
				}
				if (paramGroup->param3DVisualization)
				{
					SrVec vec = state->paramManager->getPrevVec();
					paramGroup->xAxis->value(vec.x);
					paramGroup->yAxis->value(vec.y);
					paramGroup->zAxis->value(vec.z);
					paramGroup->param3DVisualization->redraw();
				}
			}
		}
	}
}

void PARunTimeEditor::updateRunTimeStates(std::string currentState)
{
	nextCycleStates->clear();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PAStateData* stateData = mcu.lookUpPAState(currentState);

	if (stateData)
		if (!stateData->cycle)
			return;

	if (currentState == "")
		return;

	if (currentState == PseudoIdleState)
	{
		for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
		{
			if (mcu.param_anim_states[i]->cycle)
				addItem(nextCycleStates, mcu.param_anim_states[i]->stateName);
		}
	}
	else
	{
		for (size_t i = 0; i < stateData->toStates.size(); i++)
		{
			if (stateData->toStates[i]->toStates.size() == 0)
				addItem(nextCycleStates, PseudoIdleState);
			else
				for (size_t j = 0; j < stateData->toStates[i]->toStates.size(); j++)
					addItem(nextCycleStates, stateData->toStates[i]->toStates[j]->stateName.c_str());
		}
	}
	for (int i = 0; i < nextCycleStates->size(); i++)
		nextCycleStates->select(i, false);
	availableTransitions->clear();

	if (paramGroup)
	{
		parameterGroup->remove(paramGroup);
		delete paramGroup;
		paramGroup = NULL;
	}
	if (stateData)
	{
		paramGroup = new ParameterGroup(0, 0, parameterGroup->w(), parameterGroup->h(), "", stateData, paWindow);
		parameterGroup->add(paramGroup);
		paramGroup->show();
	}
}

void PARunTimeEditor::addItem(fltk::Browser* browser, std::string item)
{
	for (int i = 0; i < browser->size(); i++)
	{
		if (item == browser->goto_index(i)->label())
			return;
	}
	browser->add(item.c_str());
}

void PARunTimeEditor::initializeRunTimeEditor()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	SbmCharacter* character = mcuCBHandle::singleton().character_map.lookup(charName.c_str());
	if (character)
	{
		if (character->param_animation_ct == NULL)
			return;
		currentCycleState->value(character->param_animation_ct->getCurrentStateName().c_str());

		nextCycleStates->clear();
		availableTransitions->clear();
		updateRunTimeStates(currentCycleState->value());
	}
	prevCycleState = "";
}

void PARunTimeEditor::updateNonCycleState(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;

	std::string nonCycleState;
	for (int i = 0; i < editor->availableTransitions->size(); i++)
	{
		if (editor->availableTransitions->goto_index(i)->selected())
			nonCycleState = editor->availableTransitions->goto_index(i)->label();
	}
	if (mcuCBHandle::singleton().lookUpPAState(nonCycleState)->paramManager->getNumParameters() > 0)
	{
		if (editor->paramGroup)
		{
			editor->parameterGroup->remove(editor->paramGroup);
			delete editor->paramGroup;
			editor->paramGroup = NULL;
		}
		editor->paramGroup = new ParameterGroup(0, 0, editor->parameterGroup->w(), editor->parameterGroup->h(), "", mcuCBHandle::singleton().lookUpPAState(nonCycleState), editor->paWindow);
		editor->parameterGroup->add(editor->paramGroup);
		editor->paramGroup->show();
	}
}

void PARunTimeEditor::updateTransitionStates(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	editor->availableTransitions->clear();
	std::string currentState = editor->currentCycleState->value();
	std::string nextState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
	{
		if (editor->nextCycleStates->goto_index(i)->selected())
			nextState = editor->nextCycleStates->goto_index(i)->label();
	}
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		bool fromHit = false;
		bool toHit = false;
		if (currentState == PseudoIdleState)
		{
			if (mcu.param_anim_states[i]->fromStates.size() == 0)
			{
				for (size_t j = 0; j < mcu.param_anim_states[i]->toStates.size(); j++)
				{
					if (mcu.param_anim_states[i]->toStates[j]->stateName == nextState)
					{
						editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
						break;
					}
				}
			}
		}
		else if (nextState == PseudoIdleState)
		{
			if (mcu.param_anim_states[i]->toStates.size() == 0)
			{
				for (size_t j = 0; j < mcu.param_anim_states[i]->fromStates.size(); j++)
				{
					if (mcu.param_anim_states[i]->fromStates[j]->stateName == currentState)
					{
						editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
						break;
					}
				}
			}
		}
		else
		{
			for (size_t j = 0; j < mcu.param_anim_states[i]->fromStates.size(); j++)
				if (mcu.param_anim_states[i]->fromStates[j]->stateName == currentState)
				{
					fromHit = true;
					break;
				}
			for (size_t j = 0; j < mcu.param_anim_states[i]->toStates.size(); j++)
				if (mcu.param_anim_states[i]->toStates[j]->stateName == nextState)
				{
					toHit = true;
					break;
				}

			if (fromHit && toHit)
				editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
		}
	}
}

void PARunTimeEditor::run(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	std::string charName = editor->paWindow->characterList->child(editor->paWindow->characterList->value())->label();
	std::string nextCycleState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
		if (editor->nextCycleStates->selected(i))
		{	
			nextCycleState = editor->nextCycleStates->goto_index(i)->label();
			break;
		}
	std::string transitionState = "";
	for (int i = 0; i < editor->availableTransitions->size(); i++)
		if (editor->availableTransitions->selected(i))
		{	
			transitionState = editor->availableTransitions->goto_index(i)->label();
			break;
		}

	double timeoffset = 0.0;
	if (transitionState != "")
	{
		std::stringstream command1;
		command1 << "panim schedule char " << charName << " state " << transitionState << " loop false playnow false";
		editor->paWindow->execCmd(editor->paWindow, command1.str(), timeoffset);
		timeoffset += 0.1;
	}
	if (nextCycleState != PseudoIdleState && nextCycleState != "")
	{
		std::stringstream command2;
		command2 << "panim schedule char " << charName << " state " << nextCycleState << " loop true playnow false";
		editor->paWindow->execCmd(editor->paWindow, command2.str(), timeoffset);
	}
	
	if (nextCycleState == PseudoIdleState || nextCycleState == "") return;
}