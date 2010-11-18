#ifndef _GL_CHART_VIEW_ARCHIVE_H_
#define _GL_CHART_VIEW_ARCHIVE_H_

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
//#include <sbm/mcontrol_util.h>

#define CHART_DATA_TYPE_UNKNOWN -1
#define CHART_DATA_TYPE_VALUE 1
#define CHART_DATA_TYPE_VEC2 2
#define CHART_DATA_TYPE_VEC 3
#define CHART_DATA_TYPE_QUAT 4

class GlChartViewSeries
{
public:
	SrString title;
	int data_type;
	int max_size;
	int current_ind;
	int size;
	bool bold;

	SrArray<float> x;
	SrArray<float> y;
	SrArray<float> z;
	SrArray<float> w;

protected:
	int buffer_index;
	SrVec color_x;
	SrVec color_y;
	SrVec color_z;
	SrVec color_w;

public:
	GlChartViewSeries();
	~GlChartViewSeries();

public:
	void clear();
	float GetValue(int index);
	SrVec2 GetVec2(int index);
	SrVec GetVec3(int index);
	SrVec GetEuler(int index);
	SrQuat GetQuat(int index);

	void Push(float x);
	void Push(float x, float y);
	void Push(float x, float y, float z);
	void Push(float x, float y, float z, float w);

	void Push(SrVec2& quat);
	void Push(SrVec& quat);
	void Push(SrQuat& quat);
	void SetMaxSize(int max_size);
	void SetBufferIndex(int index);
	void SetColorOnBufferIndex();
	void SetColorOnBufferIndex(int index);
	void SetRGBColor();
	int GetBufferIndex();
	void SetColor(int index, SrVec& color);
	SrVec GetColor(int index);

	SrVec GetEulerFromQuaternion(SrQuat& quat);

protected:
	int CheckIndex(int index);

};

class GlChartViewArchive
{
public:
	SrArray<GlChartViewSeries*> series_list;

public:
	GlChartViewArchive();
	~GlChartViewArchive();

public:

	void NewSeries(const char* title, int type, int buffer_index);
	void AddSeries(GlChartViewSeries* series);
	void DeleteSeries(const char* title);
	void DeleteSeries(int index);
	void ClearSeries();
	int GetSeriesCount();
	GlChartViewSeries* GetSeries(int index);
	GlChartViewSeries* GetSeries(const char* title);
	GlChartViewSeries* GetLastSeries();

	void Update(SrBuffer<float>& buffer);

};

#endif //_GL_CHART_VIEW_ARCHIVE_H_