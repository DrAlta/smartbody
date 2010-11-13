#ifndef _GL_CHART_COORDINATE_H_
#define _GL_CHART_COORDINATE_H_

#include <SR/sr_vec.h>
#include <SR/sr_vec2.h>
#include <SR/sr_quat.h>
#include <SR/sr_mat.h>
#include <SR/sr_array.h>
#include <SR/sr_camera.h>
#include <SR/sr_buffer.h>
#include <SR/sr_string.h>
#include <string>
#include <utility>
#include "math.h"
//#include "stdlib.h"
//#include "fltk/gl.h"
#include "glfont2.h"

using namespace glfont;

class GlChartViewCoordinate
{
public:
	float x_scale;
	float y_scale;

	float y_scale_zoom;

	float prev_x_scale;
	float prev_y_scale;

	float x_margin_ratio;
	float y_margin_ratio;

	GLFont label;
	bool font_initialized;

protected:
	float x_size;
	float y_size;
	int x_label_num;
	int y_label_num;

public:
	GlChartViewCoordinate();
	~GlChartViewCoordinate();

public:
	void Update(float WindowWidth, float WindowHeight, SrCamera& camera);
	void InitFont();
	float GetXScale();
	float GetYScale();
	
	//void DrawFonts();
	void Draw();
	void SetXSize(float size);
	void SetYSize(float size);
	float GetYSize();

protected:
	void DrawCoordinateLabels();
	SrVec2 GetStringSize(char* str);
	int GetStringWidth(char* str);
};

#endif //_GL_CHART_COORDINATE_H_