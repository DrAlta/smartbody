#ifndef _GL_CHART_COORDINATE_H_
#define _GL_CHART_COORDINATE_H_

#include <SR/sr_vec.h>
#include <SR/sr_vec2.h>
#include <SR/sr_quat.h>
#include <SR/sr_mat.h>
#include <SR/sr_array.h>
#include <SR/sr_buffer.h>
#include <SR/sr_string.h>
#include "string.h"
#include "math.h"
#include "stdlib.h"

class GlChartViewCoordinate
{
public:
	float x_scale;
	float y_scale;
	float z_scale;

public:
	GlChartViewCoordinate();
	~GlChartViewCoordinate();

public:
	void GetScales(float WindowWidth, float WindowHeight, float fovy, SrVec& eye, SrVec& center);

};

#endif //_GL_CHART_COORDINATE_H_