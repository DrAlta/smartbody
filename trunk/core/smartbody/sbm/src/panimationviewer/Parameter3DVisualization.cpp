#include "Parameter3DVisualization.h"
#include <sr/sr_gl.h>
#include <sr/sr_light.h>

# define ROTATING2(e)	(e.alt && e.button1)
# define DOLLYING(e)	(e.alt && e.button3)
# define TRANSLATING(e)	(e.alt && e.button2)

Parameter3DVisualization::Parameter3DVisualization(int x, int y, int w, int h, char* name, PABlendData* s, ParameterGroup* window) : Fl_Gl_Window(x, y, w, h, ""), blendData(s), paramGroup(window)
{	
	this->begin();
	this->end();
	
	cam.center.set(0, 0, 0);
	cam.eye.set(300, -300, 400);
	cam.up.set(0, 0, 1);
	gridSize = 700;
	gridStep = 40;
	floorHeight = 0;


	for (int t = 0; t < 4; t++)
		tet.push_back(SrVec());

	blendData = s;

	lastMouseX = -1;
	lastMouseY = -1;
}

Parameter3DVisualization::~Parameter3DVisualization()
{
}

void Parameter3DVisualization::draw()
{
	//LOG("Para3D Draw()\n");

	if (!visible()) 
		return;
	if (!valid()) 
	{
		init_opengl();
		valid(1);
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
	case FL_PUSH:
		translate_event ( e, SrEvent::EventPush, w(), h(), this );
		break;

	case FL_RELEASE:
		translate_event ( e, SrEvent::EventRelease, w(), h(), this);
		break;

	case FL_DRAG:
		translate_event ( e, SrEvent::EventDrag, w(), h(), this );
		break;

	case FL_MOVE:
		
		break;

	case FL_WHEN_RELEASE:
		//translate_event ( e, SrEvent::EventRelease, w(), h(), this);
		break;

	case FL_KEYBOARD:
		break;

	case FL_HIDE: // Called when the window is iconized
		break;

	case FL_SHOW: // Called when the window is de-iconized or when show() is called
		show ();
		break;

	  default:
		  break;
	}

	mouse_event(e);

	if (event == FL_PUSH)
		return 1;

	return Fl_Gl_Window::handle(event);
}

void Parameter3DVisualization::resize(int x, int y, int w, int h)
{
	Fl_Gl_Window::resize(x, y, w, h);
	redraw();
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

void Parameter3DVisualization::translate_event(SrEvent& e, SrEvent::EventType t, int w, int h, Parameter3DVisualization* viewer)
{
	e.init_lmouse ();

	// put coordinates inside [-1,1] with (0,0) in the middle :
	e.mouse.x  = ((float)Fl::event_x())*2.0f / ((float)w) - 1.0f;
	e.mouse.y  = ((float)Fl::event_y())*2.0f / ((float)h) - 1.0f;
	e.mouse.y *= -1.0f;
	e.width = w;
	e.height = h;
	e.mouseCoord.x = (float)Fl::event_x();
	e.mouseCoord.y = (float)Fl::event_y();

	if ( Fl::event_state(FL_BUTTON1) ) 
	   e.button1 = 1;

	if ( Fl::event_state(FL_BUTTON2) ) 
	   e.button2 = 1;

	if ( Fl::event_state(FL_BUTTON3) ) 
	   e.button3 = 1;


	if(e.button1 == 0 && e.button2 == 0 && e.button3 == 0) 
	{
	   t = SrEvent::EventRelease;
	}

	e.type = t;

	if ( t==SrEvent::EventPush)
	{
	   e.button = Fl::event_button();
	   e.origUp = viewer->cam.up;
	   e.origEye = viewer->cam.eye;
	   e.origCenter = viewer->cam.center;
	   e.origMouse.x = e.mouseCoord.x;
	   e.origMouse.y = e.mouseCoord.y;
	}
	else if (t==SrEvent::EventRelease )
	{
	   e.button = Fl::event_button();
	   e.origMouse.x = -1;
	   e.origMouse.y = -1;
	}


	if ( Fl::event_state(FL_ALT)   ) e.alt = 1;
	else e.alt = 0;
	if ( Fl::event_state(FL_CTRL)  ) e.ctrl = 1;
	else e.ctrl = 0;
	if ( Fl::event_state(FL_SHIFT) ) e.shift = 1;
	else e.shift = 0;

	e.key = Fl::event_key();	
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
	if ( e.type==SrEvent::EventDrag )
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

SrVec Parameter3DVisualization::determineScale()
{
	SrVec min(99999, 99999, 99999);
	SrVec max(-99999, -99999, -99999);

	// determine the tetrahedron scale
	for (unsigned int i = 0; i < blendData->state->getParameters().size(); i++)
	{
		SrVec& param = blendData->state->getParameters()[i];
		for (int x = 0; x < 3; x++)
		{
			if (param[x] < min[x])
				min[x] = param[x];
			if (param[x] > max[x])
				max[x] = param[x];
		}
	}

	SrVec scale(1.0f, 1.0f, 1.0f);
	SrVec sizes(max[0] - min[0], max[1] - min[1], max[2] - min[2]);
	float largest = 0;
	if (sizes[0] > sizes[1])
	{
		if (sizes[0] > sizes[2])
		{
			largest = sizes[0];
			if (scale[1] != 0.0f)
				scale[1] = sizes[0] / sizes[1];
			if (scale[2] != 0.0f)
				scale[2] = sizes[0] / sizes[2];

		}
		else
		{
			largest = sizes[2];
			if (scale[0] != 0.0f)
				scale[0] = sizes[2] / sizes[0];
			if (scale[1] != 0.0f)
				scale[1] = sizes[2] / sizes[1];
		}
	}
	else
	{
		if (sizes[1] > sizes[2])
		{
			largest = sizes[1];
			if (scale[0] != 0.0f)
				scale[0] = sizes[1] / sizes[0];
			if (scale[2] != 0.0f)
				scale[2] = sizes[1] / sizes[2];
		}
		else
		{
			largest = sizes[2];
			if (scale[0] != 0.0f)
				scale[0] = sizes[2] / sizes[0];
			if (scale[1] != 0.0f)
				scale[1] = sizes[2] / sizes[1];
		}
	}

	return scale;
}

void Parameter3DVisualization::setSelectedTetrahedrons(std::vector<bool>& selected)
{
	selectedTetrahedrons = selected;
	redraw();
}

void Parameter3DVisualization::setSelectedParameters(std::vector<bool>& selected)
{
	selectedParameters = selected;
	redraw();
}

void Parameter3DVisualization::drawTetrahedrons()
{
	SrVec scale = determineScale();

	glColor3f(1.0f, 1.0f, 0.0f);
	glPointSize(5.0f);
	glBegin(GL_POINTS);

	bool highLight = false;
	if (blendData->state->getNumParameters() == selectedParameters.size())
		highLight = true;

	for (unsigned int i = 0; i < blendData->state->getParameters().size(); i++)
	{
		if (highLight && selectedParameters[i])
			glColor3f(1.0f, 0.0f, 0.0f);
		else
			glColor3f(1.0f, 1.0f, 0.0f);

		SrVec& param = blendData->state->getParameters()[i];
		SrVec scaledParam;
		for (int i = 0; i < 3; i++)
			scaledParam[i] = param[i] * scale[i];
		glVertex(scaledParam);
	}		
	glEnd();

	glColor3f(0.2f, 0.5f, 0.9f);
	glBegin(GL_LINES);
	glLineWidth(0.8f);

	bool selectedColor = false;

	std::vector<TetrahedronInfo>& tetrahedrons = blendData->state->getTetrahedrons();
	int numTetrahedrons = tetrahedrons.size();
	for (int i = 0; i < numTetrahedrons; i++)
	{
		TetrahedronInfo& info = tetrahedrons[i];
		tet[0] = info.v1;
		tet[1] = info.v2;
		tet[2] = info.v3;
		tet[3] = info.v4;

		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				tet[x][y] = tet[x][y] * scale[y];
			}
		}
		if (selectedTetrahedrons.size() == numTetrahedrons)
		{
			if (selectedTetrahedrons[i])
			{
				if (!selectedColor)
				{
					glEnd();
					glLineWidth(1.5f);
					glColor3f(1.0f, 0.5f, 0.9f);
					selectedColor = true;
					glBegin(GL_LINES);
					
				}
			}
			else
			{
				if (selectedColor)
				{
					glEnd();
					glLineWidth(0.8f);
					glColor3f(0.2f, 0.5f, 0.9f);
					selectedColor = false;
					glBegin(GL_LINES);
				}
			}
		}

		glVertex(tet[0]);
		glVertex(tet[1]);
		glVertex(tet[0]);
		glVertex(tet[2]);
		glVertex(tet[0]);
		glVertex(tet[3]);
		glVertex(tet[1]);
		glVertex(tet[2]);
		glVertex(tet[1]);
		glVertex(tet[3]);
		glVertex(tet[2]);
		glVertex(tet[3]);
	}
	glEnd();

	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(0.2f, 0.5f, 0.9f);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < state->getTetrahedrons().size(); i++)
	{
		TetrahedronInfo info = state->getTetrahedrons()[i];
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
	if (!paramGroup)
		return;

	SrVec scale = determineScale();

	SrVec vec;
	PABlendData* curStateData = paramGroup->getCurrentPABlendData();
	if (!curStateData)
		return;
	curStateData->state->getParametersFromWeights(vec.x, vec.y, vec.z, curStateData->weights);	
	for (int s = 0; s < 3; s++)
		vec[s] = vec[s] * scale[s];

	fl_color(FL_RED);
	glPointSize(7.0f);
	glBegin(GL_POINTS);
		glVertex(vec);
	glEnd();
}
