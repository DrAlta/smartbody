#include "GlChartViewArchive.h"


GlChartViewSeries::GlChartViewSeries()
{
	data_type = CHART_DATA_TYPE_UNKNOWN;
	max_size = 0;
	current_ind = -1;
	size = 0;
};

GlChartViewSeries::~GlChartViewSeries()
{
	clear();
};

void GlChartViewSeries::clear()
{
	x.capacity(0);
	y.capacity(0);
	z.capacity(0);
	w.capacity(0);
	current_ind = -1;
	size = 0;
}

__forceinline int GlChartViewSeries::CheckIndex(int index)
{
	if(max_size > 0) 
	{
		index = current_ind-index;
		if(index < 0) index += max_size;
	}
	return index;
}

float GlChartViewSeries::GetValue(int index)
{
	index = CheckIndex(index);
	return x.get(index);
};

SrVec2 GlChartViewSeries::GetVec2(int index)
{
	index = CheckIndex(index);
	return SrVec2(x.get(index), y.get(index));
}

SrVec GlChartViewSeries::GetVec3(int index)
{
	index = CheckIndex(index);
	return SrVec(x.get(index), y.get(index), z.get(index));
}

SrQuat GlChartViewSeries::GetQuat(int index)
{
	index = CheckIndex(index);
	return SrQuat(x.get(index), y.get(index), z.get(index), w.get(index));
}

void GlChartViewSeries::Push(float x, float y, float z, float w)
{
	if(current_ind == max_size-1) 
	{
		current_ind = 0;
	}
	else ++current_ind;
	if(size < max_size) ++size;
	this->x.set(current_ind, x);
	this->y.set(current_ind, y);
	this->z.set(current_ind, z);
	this->w.set(current_ind, w);
}

void GlChartViewSeries::SetMaxSize(int max)
{
	int r = max - current_ind;
	int t_old;
	int t_new;

	if(size > current_ind)
	{
		for(int i = 0; i < r; ++i)
		{
			t_old = max_size-r+i+1;
			t_new = current_ind+i+1;
			x.set(t_new, x.get(t_old));
			y.set(t_new, y.get(t_old));
			z.set(t_new, z.get(t_old));
			w.set(t_new, w.get(t_old));
		}
	}

	r = current_ind - max;
	if(r > 0)
	{
		for(int i = 0; i < max; ++i)
		{
			t_old = r+i;
			t_new = i;
			x.set(t_new, x.get(t_old));
			y.set(t_new, y.get(t_old));
			z.set(t_new, z.get(t_old));
			w.set(t_new, w.get(t_old));
		}
	}
	
	this->x.capacity(max);
	this->y.capacity(max);
	this->z.capacity(max);
	this->w.capacity(max);
}
	
void GlChartViewSeries::Push(SrQuat& quat)
{
	Push(quat.x, quat.y, quat.z, quat.w);
}

GlChartViewArchive::GlChartViewArchive()
{

}

GlChartViewArchive::~GlChartViewArchive()
{
	GlChartViewSeries* series = NULL;
	for(int i = 0;; ++i)
	{
		series = series_list.pop();
		series->clear();
		delete series;
	}
	
}

void GlChartViewArchive::NewSeries(char* title, int type)
{
	GlChartViewSeries* series = new GlChartViewSeries();
	series->title.set(title);
	series->data_type = type;
}

void GlChartViewArchive::AddSeries(GlChartViewSeries* series)
{
	series_list.push() = series;
}

void GlChartViewArchive::DeleteSeries(char* title)
{
	GlChartViewSeries* series = NULL;
	for(int i = 0; i < series_list.size(); ++i)
	{
		series = series_list.get(i);
		if(strcmp(&(series->title.get(0)), title) == 0)
		{
			series->clear();
			delete series;
			series_list.remove(i, 1);
		}
	}
}

void GlChartViewArchive::DeleteSeries(int index)
{
	GlChartViewSeries* series = NULL;
	series = series_list.get(index);
	series->clear();
	delete series;
	series_list.remove(index, 1);

}

GlChartViewSeries* GlChartViewArchive::GetLastSeries()
{
	return series_list.get(series_list.size()-1);
}

int GlChartViewArchive::GetSeriesCount()
{
	return series_list.size();
}