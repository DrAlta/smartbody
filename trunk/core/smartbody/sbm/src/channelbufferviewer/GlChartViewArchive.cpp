#include "GlChartViewArchive.hpp"


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

void GlChartViewSeries::SetColor(int index, SrVec& color)
{
	if(index == 1) this->color_x = color;
	else if(index == 2) this->color_y = color;
	else if(index == 3) this->color_z = color;
	else if(index == 4) this->color_w = color;
}

SrVec GlChartViewSeries::GetEulerFromQuaternion(SrQuat& quat)
{
	SrVec euler;
	euler.x = atan2(2.0f*(quat.w*quat.x+quat.y*quat.z), 1.0f-2.0f*(quat.x*quat.x+quat.y*quat.y));
	euler.y = asin(2.0f*(quat.w*quat.y - quat.z*quat.x));
	euler.z = atan2(2.0f*(quat.w*quat.z + quat.x*quat.y), 1.0f-2.0f*(quat.y*quat.y+quat.z*quat.z));
	euler.normalize();
	return euler;
}

SrVec GlChartViewSeries::GetColor(int index)
{
	if(index == 1) return color_x;
	if(index == 2) return color_y;
	if(index == 3) return color_z;
	if(index == 4) return color_w;

	else return SrVec(0,0,0);
}

void GlChartViewSeries::clear()
{
	x.capacity(0);
	y.capacity(0);
	z.capacity(0);
	w.capacity(0);
	current_ind = -1;
	size = 0;
}

int GlChartViewSeries::GetBufferIndex()
{
	return buffer_index;
}

void GlChartViewSeries::SetRGBColor()
{
	color_x.x = 1.0f;
	color_x.y = 0.0f;
	color_x.z = 0.0f;

	color_y.x = 0.0f;
	color_y.y = 1.0f;
	color_y.z = 0.0f;

	color_z.x = 0.0f;
	color_z.y = 0.0f;
	color_z.z = 1.0f;

	color_w.x = 1.0f;
	color_w.y = 1.0f;
	color_w.z = 1.0f;
}

void GlChartViewSeries::SetColorOnBufferIndex()
{
	SetColorOnBufferIndex(buffer_index);
}

void GlChartViewSeries::SetColorOnBufferIndex(int index)
{
	srand(index);
	while(true)
	{
		color_x.x = (float)(rand()%512)/512.0f;
		color_x.y = (float)(rand()%512)/512.0f;
		color_x.z = (float)(rand()%512)/512.0f;
		if(color_x.x < 0.4f && color_x.y < 0.4f && color_x.z < 0.4f) continue;
		break;
	}
	while(true)
	{
		color_y.x = (float)(rand()%512)/512.0f;
		color_y.y = (float)(rand()%512)/512.0f;
		color_y.z = (float)(rand()%512)/512.0f;
		if(color_y.x < 0.4f && color_y.y < 0.4f && color_y.z < 0.4f) continue;
		break;
	}
	while(true)
	{
		color_z.x = (float)(rand()%512)/512.0f;
		color_z.y = (float)(rand()%512)/512.0f;
		color_z.z = (float)(rand()%512)/512.0f;
		if(color_z.x < 0.4f && color_z.y < 0.4f && color_z.z < 0.4f) continue;
		break;
	}
	while(true)
	{
		color_w.x = (float)(rand()%512)/512.0f;
		color_w.y = (float)(rand()%512)/512.0f;
		color_w.z = (float)(rand()%512)/512.0f;
		if(color_w.x < 0.4f && color_w.y < 0.4f && color_w.z < 0.4f) continue;
		break;
	}
}

void GlChartViewSeries::SetBufferIndex(int index)
{
	buffer_index = index;
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

SrVec GlChartViewSeries::GetEuler(int index)
{
	index = CheckIndex(index);
	SrQuat quat(x.get(index), y.get(index), z.get(index), w.get(index));
	return GetEulerFromQuaternion(quat);
}

SrQuat GlChartViewSeries::GetQuat(int index)
{
	index = CheckIndex(index);
	return SrQuat(x.get(index), y.get(index), z.get(index), w.get(index));
}

void GlChartViewSeries::Push(float x)
{
	if(current_ind == max_size-1) 
	{
		current_ind = 0;
	}
	else ++current_ind;
	if(size < max_size) ++size;
	this->x.set(current_ind, x);
}

void GlChartViewSeries::Push(float x, float y)
{
	if(current_ind == max_size-1) 
	{
		current_ind = 0;
	}
	else ++current_ind;
	if(size < max_size) ++size;
	this->x.set(current_ind, x);
	this->y.set(current_ind, y);
}

void GlChartViewSeries::Push(float x, float y, float z)
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
	if(max_size == max || max <= 0) return;
	int r;
	int t_old;
	int t_new;

	if(size == 0)
	{
		this->x.capacity(max);
		this->x.size(max);
		this->y.capacity(max);
		this->y.size(max);
		this->z.capacity(max);
		this->z.size(max);
		this->w.capacity(max);
		this->w.size(max);
		this->max_size = max;
		return;
	}

	r = max - current_ind;

	if(r > 0)
	{
		if(max > max_size)
		{
			this->x.capacity(max);
			this->x.size(max);
			this->y.capacity(max);
			this->y.size(max);
			this->z.capacity(max);
			this->z.size(max);
			this->w.capacity(max);
			this->w.size(max);
		}
	
		int start = this->max_size-1;
		int end =  max > this->max_size? current_ind+1: current_ind + 1 + this->max_size - max;
		for(int i = start, j = max-1; i >= end; --i, --j)
		{
			t_old = i;
			t_new = j;
			if(t_old<0 || t_old>=this->max_size || t_new<0 || t_new>=max)
			{
				int y = 0;
			}
			x.set(t_new, x.get(t_old));
			y.set(t_new, y.get(t_old));
			z.set(t_new, z.get(t_old));
			w.set(t_new, w.get(t_old));
		}
		this->max_size = max;
		if(this->size > max) this->size = max;
		if(max_size > max)
		{
			this->x.capacity(max);
			this->x.size(max);
			this->y.capacity(max);
			this->y.size(max);
			this->z.capacity(max);
			this->z.size(max);
			this->w.capacity(max);
			this->w.size(max);
		}
	}

	//r = current_ind - max;
	else
	{
		for(int i = 0; i < max; ++i)
		{
			t_old = -r+i;
			t_new = i;
			if(t_old<0 || t_old>=this->max_size || t_new<0 || t_new>=this->max_size)
			{
				int y = 0;
			}
			x.set(t_new, x.get(t_old));
			y.set(t_new, y.get(t_old));
			z.set(t_new, z.get(t_old));
			w.set(t_new, w.get(t_old));
		}
		current_ind = max-1;
		this->size = max;
		this->max_size = max;
		this->x.capacity(max);
		this->x.size(max);
		this->y.capacity(max);
		this->y.size(max);
		this->z.capacity(max);
		this->z.size(max);
		this->w.capacity(max);
		this->w.size(max);
	}

}

void GlChartViewSeries::Push(SrVec& quat)
{
	Push(quat.x, quat.y, quat.z);
}


void GlChartViewSeries::Push(SrVec2& quat)
{
	Push(quat.x, quat.y);
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
	int num = series_list.size();
	for(int i = 0; i < num; ++i)
	{
		series = series_list.pop();
		series->clear();
		delete series;
	}

}

void GlChartViewArchive::NewSeries(const char* title, int type, int buffer_index)
{
	GlChartViewSeries* series = new GlChartViewSeries();
	series->title.set(title);
	series->data_type = type;
	series->SetBufferIndex(buffer_index);
	if(series_list.size() == 0) series->SetRGBColor();
	else series->SetColorOnBufferIndex();
	series->SetMaxSize(0);
	series_list.push() = series;
}

void GlChartViewArchive::AddSeries(GlChartViewSeries* series)
{
	series_list.push() = series;
}

void GlChartViewArchive::DeleteSeries(const char* title)
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

void GlChartViewArchive::ClearSeries()
{
	GlChartViewSeries* series = NULL;
	int num = series_list.size();
	for(int i = 0; i < num; ++i)
	{
		series = series_list.get(i);
		series->clear();
		delete series;
	}
	series_list.size(0);
}

GlChartViewSeries* GlChartViewArchive::GetLastSeries()
{
	return series_list.get(series_list.size()-1);
}

int GlChartViewArchive::GetSeriesCount()
{
	return series_list.size();
}

GlChartViewSeries* GlChartViewArchive::GetSeries(int index)
{
	return series_list.get(index);
}

GlChartViewSeries* GlChartViewArchive::GetSeries(const char* title)
{
	for(int i = 0; i < series_list.size(); ++i)
	{
		if(strcmp((const char*)(series_list.get(i)->title), title) == 0) return series_list.get(i);
	}
	return NULL;
}

void GlChartViewArchive::Update(SrBuffer<float>& buffer)
{
	GlChartViewSeries* series = NULL;
	float x, y, z, w;
	int buffer_index = 0;
	for(int i = 0; i < series_list.size(); ++i)
	{
		series = series_list.get(i);
		buffer_index = series->GetBufferIndex();
		if(series->data_type >= CHART_DATA_TYPE_VALUE) x = buffer[buffer_index];
		if(series->data_type >= CHART_DATA_TYPE_VEC2) y = buffer[buffer_index+1];
		if(series->data_type >= CHART_DATA_TYPE_VEC) z = buffer[buffer_index+2];
		if(series->data_type >= CHART_DATA_TYPE_QUAT) w = buffer[buffer_index+3];
		 
		if(series->data_type == CHART_DATA_TYPE_VALUE) series->Push(x);
		if(series->data_type == CHART_DATA_TYPE_VEC2) series->Push(x, y);
		if(series->data_type == CHART_DATA_TYPE_VEC) series->Push(x, y, z);
		if(series->data_type == CHART_DATA_TYPE_QUAT) series->Push(x, y, z, w);
	}
}