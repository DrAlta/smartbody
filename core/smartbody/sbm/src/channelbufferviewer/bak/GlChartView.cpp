#include "GlChartView.h"

GlChartView::GlChartView(int x, int y, int w, int h, char* name) : fltk::GlWindow( x, y, w, h, name )
{
	initGL(w, h);
	init_camera();
}

GlChartView::~GlChartView()
{
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
    //glShadeModel(GL_SMOOTH);           
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
	glPointSize ( 2.0 );

	glShadeModel ( GL_SMOOTH );

	
}

void GlChartView::init_camera()
{
	camera.eye.z = 1000.0f;
	//set_camera(camera);
}

void GlChartView::reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	// transform
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 100.0f, 1000000.0f);
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


	//glLoadIdentity();
	draw_coordinate();
	draw_series();

	//glutSwapBuffers();

}

void GlChartView::draw_coordinate()
{
	glBegin(GL_LINES);
		glColor4f(1.0f, 0.0f, 0.0f, 0.3f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(1000.0f, 0.0f, 0.0f);

		glColor4f(0.0f, 1.0f, 0.0f, 0.3f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1000.0f, 0.0f);

		glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();
}

void GlChartView::draw_series()
{
	GlChartViewSeries* series = NULL;
	for(int i = 0; i < archive.GetSeriesCount(); ++i)
	{
		series = archive.GetSeries(i);
		/*if(series->data_type == CHART_DATA_TYPE_VALUE)
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

		else */if(series->data_type == CHART_DATA_TYPE_QUAT)
		{
			draw_series_quat(series);
		}
	}
}

void GlChartView::draw_series_value(GlChartViewSeries* series)
{

}

void GlChartView::draw_series_vec2(GlChartViewSeries* series)
{
}

void GlChartView::draw_series_vec3(GlChartViewSeries* series)
{

}

void GlChartView::draw_series_quat(GlChartViewSeries* series)
{
	SrQuat quat;
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i < series->size; ++i)
		{
			quat = series->GetQuat(i);
			glVertex3f(i*5.0f, quat.x, 0.0f);
		}
	glEnd();
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i < series->size; ++i)
		{
			quat = series->GetQuat(i);
			glVertex3f(i*5.0f, quat.y, 0.0f);
		}
	glEnd();
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i < series->size; ++i)
		{
			quat = series->GetQuat(i);
			glVertex3f(i*5.0f, quat.z, 0.0f);
		}
	glEnd();
}

