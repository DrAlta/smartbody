#ifndef _GL_CHART_VIEW_H_
#define _GL_CHART_VIEW_H_

#include <fltk/GlWindow.H>
#include <SR/sr_camera.h>
#include <SR/sr_light.h>
#include <SR/sr_gl.h>
#include <fltk/gl.h>
#include <fltk/glut.h>

#include "GlChartViewArchive.h"

class GlChartView : public fltk::GlWindow
{
public:
	SrCamera camera;

	GlChartViewArchive archive;


public:
	GlChartView(int x, int y, int w, int h, char* name);
	~GlChartView();

public:
	GlChartViewArchive* get_archive();

protected:
	void initGL(int width, int height);
	void init_camera();
	void reshape(int width, int height);
	void draw();
	void draw_coordinate();
	void draw_series();
	void draw_series_value(GlChartViewSeries* series);
	void draw_series_vec2(GlChartViewSeries* series);
	void draw_series_vec3(GlChartViewSeries* series);
	void draw_series_quat(GlChartViewSeries* series);

};

#endif //_GL_CHART_VIEW_H_