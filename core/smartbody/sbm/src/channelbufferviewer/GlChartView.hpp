#ifndef _GL_CHART_VIEW_H_
#define _GL_CHART_VIEW_H_

#include <fltk/GlWindow.H>
#include <SR/sr_camera.h>
#include <SR/sr_light.h>
#include "GlChartViewArchive.hpp"
#include <sbm/mcontrol_util.h>
//#include ""

class GlChartView : public fltk::GlWindow, public SrViewer
{
public:
	int th;
	SrCamera camera;
	int max_buffer_size;

	SrEvent e;

	GlChartViewArchive archive;

	int quat_shown_type; //0: 4 series of values; 1: euler angle;


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
	void init_camera();

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
	void draw_series_quat(GlChartViewSeries* series);

};

#endif //_GL_CHART_VIEW_H_