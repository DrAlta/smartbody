#ifndef _GL_CHART_VIEW_ARCHIVE_H_
#define _GL_CHART_VIEW_ARCHIVE_H_

#include <SR/sr_vec.h>
#include <SR/sr_vec2.h>
#include <SR/sr_quat.h>
#include <SR/sr_mat.h>
#include <SR/sr_array.h>
#include <SR/sr_string.h>
#include "string.h"
#include "math.h"

#define CHART_DATA_TYPE_UNKNOWN -1
#define CHART_DATA_TYPE_VALUE 0
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
	
	SrArray<float> x;
	SrArray<float> y;
	SrArray<float> z;
	SrArray<float> w;

public:
	GlChartViewSeries();
	~GlChartViewSeries();

public:
	void clear();
	float GetValue(int index);
	SrVec2 GetVec2(int index);
	SrVec GetVec3(int index);
	SrQuat GetQuat(int index);

	void Push(float x, float y, float z, float w);
	void Push(SrQuat& quat);
	void SetMaxSize(int max_size);

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
	void NewSeries(char* title, int type);
	void AddSeries(GlChartViewSeries* series);
	void DeleteSeries(char* title);
	void DeleteSeries(int index);
	int GetSeriesCount();
	GlChartViewSeries* GetSeries(int index);
	GlChartViewSeries* GetLastSeries();
	GlChartViewSeries* GetSeries(SrString title);

};

#endif //_GL_CHART_VIEW_ARCHIVE_H_