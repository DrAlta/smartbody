#include "GlChartView.hpp"
# include <fltk/events.h>
#include <SR/sr_gl.h>


GlChartView::GlChartView(int x, int y, int w, int h, char* name) : fltk::GlWindow( x, y, w, h, name ), SrViewer(x, y, w, h, name)
{
	initGL(w, h);
	init_camera(0);
	th = 0;
	//max_buffer_size = 800;
	quat_shown_type = 0;
	update_coordinate = true;
	show_x = true;
	show_y = true;
	show_z = true;
	show_w = true;
}

GlChartView::~GlChartView()
{
}

void GlChartView::set_max_buffer_size(int max_size)
{
	//max_buffer_size = max_size;
	coordinate.SetXSize((float)max_size);
}

void GlChartView::initGL(int width, int height)
{
	//float pos1[4] = {1500.0, 1500.0, 1500.0, 1.0};
	float pos0[4] = {-15000.0f, -12000.0f, 15000.0f, 1.0f};
	float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	float diffuse[4] = {0.6f, 0.6f, 0.6f, 1.0f};
	float specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	//float lmodel_ambient[4] = {0.4, 0.4, 0.4, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, pos0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);       
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glViewport ( 0, 0, width, height );

	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LEQUAL );
	glFrontFace ( GL_CCW );

	glEnable ( GL_POLYGON_SMOOTH );

	glEnable ( GL_POINT_SMOOTH );
	glPointSize ( 1.0 );

	glShadeModel ( GL_SMOOTH );
}

void GlChartView::init_camera(int type)
{
	camera.init();
	camera.aspect = (float)w()/(float)h();
	coordinate.Update((float)w(), (float)h(), camera);
	camera.eye.x = coordinate.GetXScale()/2;
	camera.eye.z = 2000.0f;
	camera.eye.y = 0.0f;
	camera.center.x = coordinate.GetXScale()/2;
	camera.center.y = 0.0f;
	camera.center.z = 0.0f;
	if(type == 0) 
	{
		coordinate.y_scale_zoom = 1.0f;
		coordinate.SetYSize(1.0f);
	}
	else if(type == 1)
	{
		coordinate.y_scale_zoom = 1.0f/180.0f;
		coordinate.SetYSize(180.0f);
	}
}

void GlChartView::reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	// transform
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 100.0f, 1000000.0f);
}

void GlChartView::render()
{
	if(update_coordinate) coordinate.Update((float)w(), (float)h(), camera);
	redraw();
}

void GlChartView::draw()
{
	glViewport ( 0, 0, w(), h() );
	SrLight light1;
	SrLight light2;
	SrMat mat;

	light1.directional = true;
	light1.diffuse = SrColor( 1.0f, 0.95f, 0.8f );
	light1.position = SrVec( 100.0, 250.0, 400.0 );
	light1.constant_attenuation = 1.0f;

	light2 = light1;
	light2.directional = false;
	light2.diffuse = SrColor( 0.8f, 0.85f, 1.0f );
	light2.position = SrVec( 100.0, 500.0, -200.0 );

	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	camera.aspect = (float)w()/(float)h();

	glMatrixMode ( GL_PROJECTION );
	glLoadMatrix ( camera.get_perspective_mat(mat) );

	glMatrixMode ( GL_MODELVIEW );
	glLoadMatrix ( camera.get_view_mat(mat) );

	glScalef ( camera.scale, camera.scale, camera.scale );

	glEnable ( GL_LIGHTING );
	glLight ( 0, light1 );
	glLight ( 1, light2 );

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

	draw_coordinate();
	draw_series();
}

void GlChartView::draw_coordinate()
{
	coordinate.Draw();
}

void GlChartView::draw_series()
{
	GlChartViewSeries* series = NULL;
	for(int i = 0; i < archive.GetSeriesCount(); ++i)
	{
		series = archive.GetSeries(i);
		if(series->data_type == CHART_DATA_TYPE_VALUE)
		{
			draw_series_value(series);
		}
		else if(series->data_type == CHART_DATA_TYPE_VEC2)
		{
			draw_series_vec2(series);
		}
		else if(series->data_type == CHART_DATA_TYPE_VEC)
		{
			draw_series_vec3(series);
		}

		else if(series->data_type == CHART_DATA_TYPE_QUAT)
		{
			if(quat_shown_type == 0) draw_series_quat(series);
			else if(quat_shown_type == 1) draw_series_euler(series);
			else if(quat_shown_type == 2) draw_series_3D_euler(series);
		}
	}
}

void GlChartView::set_quat_show_type(int type)
{
	quat_shown_type = type;
	if(type == 0 && coordinate.GetYSize() == 180.0f) 
	{
		coordinate.SetYSize(1.0f);
		coordinate.y_scale_zoom = 1.0f;
	}
	else if(type == 1 && coordinate.GetYSize() == 1.0f) 
	{
		coordinate.SetYSize(180.0f);
		coordinate.y_scale_zoom = 1.0f/180.0f;
	}
}

void GlChartView::draw_series_value(GlChartViewSeries* series)
{
	float value = 0.0f;
	SrVec color;
	float step = coordinate.GetXScale()/(series->max_size-1);
	float y_scale = coordinate.GetYScale();
	float y_size = coordinate.GetYSize();
	if(series->bold)
	{
		glLineWidth(3.0f);
	}
	color = series->GetColor(1);
	glColor4f(color.x, color.y, color.z, 0.5f);
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i < series->size; ++i)
		{
			value = series->GetValue(i);
			if(abs(value) > this->coordinate.GetYSize()) 
			{
				coordinate.y_scale_zoom = 1.0f/abs(value);
				this->coordinate.SetYSize(abs(value));
			}
			glVertex3f(i*step, value*y_scale, 0.0f);
		}
	glEnd();
	glLineWidth(1.0f);
}

void GlChartView::draw_series_vec2(GlChartViewSeries* series)
{
}

void GlChartView::draw_series_vec3(GlChartViewSeries* series)
{

}

// not used for now
void GlChartView::draw_series_3D_euler(GlChartViewSeries* series)
{
	SrVec euler;
	SrVec prev_euler;
	SrVec color;
	color = series->GetColor(1);
	glColor4f(color.x, color.y, color.z, 0.5f);
	glBegin(GL_LINES);
		for(int i = 0; i < series->size; ++i)
		{
			euler = series->GetEuler(i);
			euler *= 500.0f;
			if(i > 0)
			{
				glVertex3f((i-1)*5.0f+prev_euler.x, prev_euler.y, prev_euler.z);
				glVertex3f(i*5.0f+euler.x, euler.y, euler.z);
			}
			glVertex3f(i*5.0f, 0.0f, 0.0f);
			glVertex3f(i*5.0f+euler.x, euler.y, euler.z);
			prev_euler = euler;
		}
	glEnd();

}

void GlChartView::draw_series_euler(GlChartViewSeries* series)
{
	SrVec euler;
	SrVec color;
	float step = coordinate.GetXScale()/(series->max_size-1);
	float y_scale = coordinate.GetYScale();
	if(series->bold)
	{
		glLineWidth(3.0f);
	}


	if(show_x)
	{
		color = series->GetColor(1);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				euler = series->GetEuler(i);
				glVertex3f(i*step, euler.x*y_scale, 0.0f);
			}
		glEnd();
	}
	if(show_y)
	{
		color = series->GetColor(2);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				euler = series->GetEuler(i);
				glVertex3f(i*step, euler.y*y_scale, 0.0f);
			}
		glEnd();
	}
	if(show_z)
	{
		color = series->GetColor(3);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				euler = series->GetEuler(i);
				glVertex3f(i*step, euler.z*y_scale, 0.0f);
			}
		glEnd();
	}
	glLineWidth(1.0f);
}

void GlChartView::draw_series_quat(GlChartViewSeries* series)
{
	SrQuat quat;
	SrVec color;

	float step = coordinate.GetXScale()/(series->max_size-1);
	float y_scale = coordinate.GetYScale();
	if(series->bold)
	{
		glLineWidth(3.0f);
	}

	if(show_x)
	{
		color = series->GetColor(1);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				quat = series->GetQuat(i);
				glVertex3f(i*step, quat.x*y_scale, 0.0f);
			}
		glEnd();
	}
	if(show_y)
	{
		color = series->GetColor(2);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				quat = series->GetQuat(i);
				glVertex3f(i*step, quat.y*y_scale, 0.0f);
			}
		glEnd();
	}
	if(show_z)
	{
		color = series->GetColor(3);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				quat = series->GetQuat(i);
				glVertex3f(i*step, quat.z*y_scale, 0.0f);
			}
		glEnd();
	}

	if(show_w)
	{
		color = series->GetColor(4);
		glColor4f(color.x, color.y, color.z, 0.5f);
		glBegin(GL_LINE_STRIP);
			for(int i = 0; i < series->size; ++i)
			{
				quat = series->GetQuat(i);
				glVertex3f(i*step, quat.w*y_scale, 0.0f);
			}
		glEnd();
	}

	glLineWidth(1.0f);
}

GlChartViewArchive* GlChartView::get_archive()
{
	return &archive;
}

int GlChartView::handle ( int event )
{
	//printf("\n%d", event);
	switch ( event )
	{ 
	case fltk::PUSH:
		{ 
			translate_event ( e, SrEvent::Push, w(), h(), this );
		} 
		break;

	case fltk::RELEASE:
        translate_event ( e, SrEvent::Release, w(), h(), this);
		break;

	case fltk::DRAG:
		update_coordinate = false;
	case fltk::MOVE:
        translate_event ( e, SrEvent::Drag, w(), h(), this );
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

void GlChartView::translate_event ( SrEvent& e, SrEvent::Type t, int w, int h, GlChartView* viewer )
 {
   e.init_lmouse ();
   e.type = t;
   // put coordinates inside [-1,1] with (0,0) in the middle :
   e.mouse.x  = ((float)fltk::event_x())*2.0f / ((float)w) - 1.0f;
   e.mouse.y  = ((float)fltk::event_y())*2.0f / ((float)h) - 1.0f;
   e.mouse.y *= -1.0f;
   e.width = w;
   e.height = h;
   e.mouseCoord.x = (float)fltk::event_x();
   e.mouseCoord.y = (float)fltk::event_y();

   if ( t==SrEvent::Push)
   {
	   e.button = fltk::event_button();
	   e.origUp = viewer->camera.up;
	   e.origEye = viewer->camera.eye;
	   e.origCenter = viewer->camera.center;
	   e.origMouse.x = e.mouseCoord.x;
	   e.origMouse.y = e.mouseCoord.y;
   }
   else if (t==SrEvent::Release )
   {
	   e.button = fltk::event_button();
	   e.origMouse.x = -1;
	   e.origMouse.y = -1;
   }


   if ( fltk::event_state(fltk::BUTTON1) ) 
	   e.button1 = 1;
   if ( fltk::event_state(fltk::BUTTON2) ) 
	   e.button2 = 1;
   if ( fltk::event_state(fltk::BUTTON3) ) 
	   e.button3 = 1;

   if ( fltk::event_state(fltk::ALT)   ) e.alt = 1;
   if ( fltk::event_state(fltk::CTRL)  ) e.ctrl = 1;

   if ( fltk::event_state(fltk::SHIFT) ) e.shift = 1;
   
   e.key = fltk::event_key();

 }

SrVec GlChartView::rotatePoint(SrVec point, SrVec origin, SrVec direction, float angle)
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

int GlChartView::mouse_event ( const SrEvent &e )
{
	int res=0;

	if ( e.mouse_event() )
	{ 
		if ( e.type==SrEvent::Drag )
		{ 
			float dx = e.mousedx() * camera.aspect;
			float dy = e.mousedy() / camera.aspect;

			if ( e.alt && e.button3 )
			{
				//camera.fovy += (dx+dy);//40.0f;
				//camera.fovy = SR_BOUND ( camera.fovy, 0.001f, srpi );

				if(coordinate.y_scale_zoom < 1.0f) 
				{
					//coordinate.y_scale_zoom = 1.0f;
					if(e.lmouse.y > e.mouse.y) coordinate.y_scale_zoom = 0.93f*coordinate.y_scale_zoom;
					else coordinate.y_scale_zoom = coordinate.y_scale_zoom/0.93f;
					if(coordinate.y_scale_zoom < 0.0001f) coordinate.y_scale_zoom = 0.0001f;
					coordinate.SetYSize(1.0f/coordinate.y_scale_zoom);
				}
				else 
				{
					float s = e.mouse.y - e.lmouse.y;
					//if(e.lmouse.y > e.mouse.y) s = -s;
					coordinate.y_scale_zoom += s*coordinate.y_scale_zoom;
					coordinate.SetYSize(1.0f);
				}
			}
			else if(e.button1 && e.alt)
			{
				//camera.center.x += (e.lmouse.x - e.mouse.x)*coordinate.GetXScale()/2;
				//camera.eye.x += (e.lmouse.x - e.mouse.x)*coordinate.GetXScale()/2;

				camera.center.y += (e.lmouse.y - e.mouse.y)*coordinate.GetYScale();
				camera.eye.y += (e.lmouse.y - e.mouse.y)*coordinate.GetYScale();
			}
			else if ( e.alt && e.button3 )
			{ 
				float amount = dx;
				SrVec cameraPos(camera.eye);
				SrVec targetPos(camera.center);
				SrVec diff = targetPos - cameraPos;
				float distance = diff.len();
				diff.normalize();

				if (amount >= distance)
					amount = distance - .000001f;

				SrVec diffVector = diff;
				SrVec adjustment = diffVector * distance * amount;
				cameraPos += adjustment;
				camera.eye = cameraPos;			
			}
			else if ( e.alt && e.button2 )
			{ 
				camera.apply_translation_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y );
			}
			else if (e.alt && e.shift && e.button1)
			{ 
			}
			//rotation with mouse doesn't seem useful in this case?
			/*else if (e.alt && e.button1) 
			{ 
 				float deltaX = -(e.mouseCoord.x - e.origMouse.x) / e.width;
				float deltaY = -(e.mouseCoord.y -  e.origMouse.y) / e.height;
				if (deltaX == 0.0 && deltaY == 0.0)
					return 1;

				SrVec origUp = e.origUp;
				SrVec origCenter = e.origCenter;
				SrVec origCamera = e.origEye;

				SrVec dirX = origUp;
				SrVec  dirY;
				dirY.cross(origUp, (origCenter - origCamera));
				dirY /= dirY.len();

				SrVec camera_p = rotatePoint(origCamera, origCenter, dirX, -deltaX * float(M_PI));
				camera_p = rotatePoint(camera_p, origCenter, dirY, deltaY * float(M_PI));

				camera.eye = camera_p;
			}*/
		}
		else if ( e.type==SrEvent::Release )
		{ 
		}
	}

	return res;
 }
