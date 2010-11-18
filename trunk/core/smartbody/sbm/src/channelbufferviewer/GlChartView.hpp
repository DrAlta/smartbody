#ifndef _GL_CHART_VIEW_H_
#define _GL_CHART_VIEW_H_

#include <fltk/GlWindow.H>
#include <SR/sr_camera.h>
#include <SR/sr_light.h>
#include "GlChartViewCoordinate.hpp"
#include "GlChartViewArchive.hpp"
#include <sbm/mcontrol_util.h>
//#include "glfont.h"

class GlChartView : public fltk::GlWindow, public SrViewer
{
public:
	int th;
	SrCamera camera;
	bool update_coordinate;

	SrEvent e;

	GlChartViewCoordinate coordinate;
	GlChartViewArchive archive;

	int quat_shown_type; //0: quaternion; 1: euler angle;

	bool show_x; // if show x value when shown as quaternion or euler angle
	bool show_y; // if show y value when shown as quaternion or euler angle
	bool show_z; // if show z value when shown as quaternion or euler angle
	bool show_w; // if show z value when shown as quaternion

public:
	GlChartView(int x, int y, int w, int h, char* name);
	~GlChartView();

public:
	GlChartViewArchive* get_archive();
	void render();
	void set_max_buffer_size(int max_size);
	void set_quat_show_type(int type);

public:
	virtual int handle ( int event );

	void translate_event ( SrEvent& e, SrEvent::Type t, int w, int h, GlChartView* viewer );
	int mouse_event ( const SrEvent &e );
	SrVec rotatePoint(SrVec point, SrVec origin, SrVec direction, float angle);
	void init_camera(int type);

protected:
	void initGL(int width, int height);
	
	void reshape(int width, int height);

	void print_bitmap_string(float x,float y, float z, void *font, char* s);
	void displayText(char* text, int X1, int Y1);

	void draw();
	void draw_coordinate();
	void draw_series();
	void draw_series_value(GlChartViewSeries* series);
	void draw_series_vec2(GlChartViewSeries* series);
	void draw_series_vec3(GlChartViewSeries* series);
	void draw_series_euler(GlChartViewSeries* series);
	void draw_series_3D_euler(GlChartViewSeries* series);
	void draw_series_quat(GlChartViewSeries* series);

};

#endif //_GL_CHART_VIEW_H_