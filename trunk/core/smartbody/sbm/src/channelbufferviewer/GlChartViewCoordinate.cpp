/*
 *  GlChartViewCoordinate.cpp - part of SmartBody-lib's Test Suite
 *  Copyright (C) 2009  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Jingqiao Fu, USC
 */

#include "vhcl.h"

#include "GlChartViewCoordinate.hpp"

#include <fltk/gl.h>


GlChartViewCoordinate::GlChartViewCoordinate()
{
	x_margin_ratio = 0.05f;
	y_margin_ratio = 0.1f;
	x_scale = 1.0f;
	y_scale = 1.0f;
	y_scale_zoom = 1.0f;
	x_label_num = 10;
	default_x_label_num = 10;
	y_label_num = 10;
	x_size = 800.0f;
	y_size = 1.0f;
	font_initialized = false;
}

GlChartViewCoordinate::~GlChartViewCoordinate()
{
}

void GlChartViewCoordinate::SetYSize(float size)
{
	y_size = size;
}

void GlChartViewCoordinate::SetXSize(float size)
{
	x_size = size;
	/*for(int i = default_x_label_num; i > 0; --i)
	{
		if(x_size/(float)i-(int)(x_size/(float)i) == 0.0f)
		{
			x_label_num = i;
			break;
		}
	}*/
}

void GlChartViewCoordinate::InitFont()
{
	GLuint textureName;	
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGenTextures(0, &textureName);

	if (!label.Create("../../../../data/fonts/font.glf", 0))
	{
		if(!label.Create(".font.glf", 0))
			LOG("GlChartViewCoordinate::InitFont(): Error: Cannot load font file\n");
	}
}

void GlChartViewCoordinate::Update(float WindowWidth, float WindowHeight, SrCamera& camera)
{
	prev_x_scale = x_scale;
	prev_y_scale = y_scale;
	SrVec dis = camera.eye - camera.center;
	y_scale = tan(camera.fovy/2)*dis.len()*(1.0f-y_margin_ratio);
	x_scale = 2.0f * y_scale * (1.0f-x_margin_ratio) * WindowWidth / WindowHeight;

	camera.center.x *= x_scale/prev_x_scale;
	camera.eye.x *= x_scale/prev_x_scale;
}

float GlChartViewCoordinate::GetXScale()
{
	return x_scale;
}

float GlChartViewCoordinate::GetYScale()
{
	return y_scale*y_scale_zoom;
}

float GlChartViewCoordinate::GetYSize()
{
	return y_size;
}

void GlChartViewCoordinate::Draw()
{
	float y_length = GetYScale()*y_size;
	float x_length = GetXScale();
	glColor4f(0.1f, 0.1f, 0.1f, 0.3f);
	float x;
	glBegin(GL_LINES);
		for(int i = y_label_num; i > 0; --i)
		{
			glVertex3f(1.0f, i*y_length/y_label_num, 0.0f);
			glVertex3f(x_length, i*y_length/y_label_num, 0.0f);
			glVertex3f(1.0f, -i*y_length/y_label_num, 0.0f);
			glVertex3f(x_length, -i*y_length/y_label_num, 0.0f);

		}
		for(int i = x_label_num; i > 0; --i)
		{
			x = i*x_length/x_label_num;
			x = x*((int)(i*x_size/x_label_num))/(i*x_size/x_label_num);
			glVertex3f(x, y_length, 0.0f);
			glVertex3f(x, -y_length, 0.0f);
		}
	glEnd();

	DrawCoordinateLabels();

	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x_length, 0.0f, 0.0f);

		glVertex3f(0.0f, y_length, 0.0f);
		glVertex3f(0.0f, -y_length, 0.0f);

	glEnd();
	
}

SrVec2 GlChartViewCoordinate::GetStringSize(char* str)
{
	SrVec2 size;
	size.y = (float)label.GetCharHeightGL(str[0]);
	int length = strlen(str);
	for(int i = 0; i < length; ++i)
	{
		size.x += (float)label.GetCharWidthGL(str[i]);
	}
	return size;
}

int GlChartViewCoordinate::GetStringWidth(char* str)
{
	int length = strlen(str);
	int width = 0;
	for(int i = 0; i < length; ++i)
	{
		width += label.GetCharWidthGL(str[i]);
	}
	return width;
}

void GlChartViewCoordinate::DrawCoordinateLabels()
{
	if(!font_initialized) 
	{
		InitFont();
		font_initialized = true;
	}
	float x_length = GetXScale();
	float y_length = GetYScale()*y_size;

	char value[10];
	SrVec size;

	glEnable(GL_TEXTURE_2D);

	glColor4f(0.4f, 0.4f, 0.4f, 0.3f);

	label.Begin();

	sprintf(value, "%d", 0);
	size = GetStringSize(value);
	label.DrawString(value, 2.0f, -size.x*2.4f, 0.0f + 1.5f * size.y/2.0f);
	for(int i = y_label_num; i > 0; --i)
	{
		sprintf(value, "%.2f", y_size*i/y_label_num);
		size = GetStringSize(value);
		label.DrawString(value, 2.0f, -size.x*2.4f, i*y_length/y_label_num + 1.5f*size.y/2.0f);

		sprintf(value, "-%.2f", y_size*i/y_label_num);
		size = GetStringSize(value);
		label.DrawString(value, 2.0f, -size.x*2.4f, -i*y_length/y_label_num + 1.5f*size.y/2.0f);
	}

	float x;

	for(int i = x_label_num; i > 0; --i)
	{
		sprintf(value, "%d", (int)(x_size*i/x_label_num));
		size = GetStringSize(value);
		x = i*x_length/x_label_num;
		x = x*((int)(i*x_size/x_label_num))/(i*x_size/x_label_num);
		label.DrawString(value, 2.0f, x - size.x/2.0f, -size.y);
	}

	glDisable(GL_TEXTURE_2D);
}
