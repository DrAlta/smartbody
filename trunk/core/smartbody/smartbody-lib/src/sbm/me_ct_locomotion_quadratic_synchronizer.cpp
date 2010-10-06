/*
 *  me_ct_locomotion_quadratic_synchronizer.hpp - part of SmartBody-lib's Motion Engine
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

#include "me_ct_locomotion_quadratic_synchronizer.hpp"


MeCtLocomotionQuadraticSynchronizer::MeCtLocomotionQuadraticSynchronizer()
{
	memset(target, 0, sizeof(float)*4);
	memset(value, 0, sizeof(float)*4);
	delta_distance = 0.0f;
}

MeCtLocomotionQuadraticSynchronizer::~MeCtLocomotionQuadraticSynchronizer()
{
	
}

void MeCtLocomotionQuadraticSynchronizer::set_target_flag(int ind1, int ind2)
{
	for(int i = 0; i < 4; ++i)
	{
		flag[i] = false;
	}
	flag[ind1] = true;
	if(ind1 == QUAD_SYNC_DISTANCE) value[ind1] = 0.0f;
	flag[ind2] = true;
	if(ind2 == QUAD_SYNC_DISTANCE) value[ind2] = 0.0f;
	primary_ind = ind1;
}

void MeCtLocomotionQuadraticSynchronizer::update_target(int ind, float target_value)
{
	if(!flag[ind]) printf("MeCtLocomotionQuadraticSynchronizer::update_target(): Error, target not exist");
	target[ind] = target_value;
	
}

void MeCtLocomotionQuadraticSynchronizer::update(float time)
{
	if(flag[QUAD_SYNC_TIME] && flag[QUAD_SYNC_SPEED]) update_for_time_speed(time);
	else if(flag[QUAD_SYNC_TIME] && flag[QUAD_SYNC_DISTANCE]) update_for_time_distance(time);
	else if(flag[QUAD_SYNC_SPEED] && flag[QUAD_SYNC_DISTANCE]) update_for_speed_distance(time);
}

void MeCtLocomotionQuadraticSynchronizer::update_for_time_speed(float time)
{
	value[QUAD_SYNC_ACCELERATION] = (target[QUAD_SYNC_SPEED]-value[QUAD_SYNC_SPEED])/(target[QUAD_SYNC_TIME]-value[QUAD_SYNC_TIME]);
	value[QUAD_SYNC_TIME] += time;
	value[QUAD_SYNC_SPEED] += time*value[QUAD_SYNC_ACCELERATION];
	if(primary_ind == QUAD_SYNC_SPEED)
	{
		if(value[QUAD_SYNC_ACCELERATION] > 0.0f && value[QUAD_SYNC_SPEED]> target[QUAD_SYNC_SPEED]) 
			value[QUAD_SYNC_SPEED] = target[QUAD_SYNC_SPEED];
		else if(value[QUAD_SYNC_ACCELERATION] < 0.0f && value[QUAD_SYNC_SPEED] < target[QUAD_SYNC_SPEED])
			value[QUAD_SYNC_SPEED] = target[QUAD_SYNC_SPEED];
	}
	else if(primary_ind == QUAD_SYNC_TIME && value[QUAD_SYNC_TIME]> target[QUAD_SYNC_TIME]) 
		value[QUAD_SYNC_SPEED] = target[QUAD_SYNC_SPEED];
}

void MeCtLocomotionQuadraticSynchronizer::update_for_time_distance(float time)
{
	float t_time = target[QUAD_SYNC_TIME] - value[QUAD_SYNC_TIME];
	value[QUAD_SYNC_ACCELERATION] = 2*(target[QUAD_SYNC_DISTANCE]-value[QUAD_SYNC_DISTANCE]-value[QUAD_SYNC_SPEED]*t_time)/t_time*t_time;
	delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*value[QUAD_SYNC_ACCELERATION]*time*time;
	value[QUAD_SYNC_DISTANCE] += delta_distance;
	value[QUAD_SYNC_SPEED] += value[QUAD_SYNC_ACCELERATION]*time;
	value[QUAD_SYNC_TIME] += time;
	
	if(primary_ind == QUAD_SYNC_TIME && value[QUAD_SYNC_TIME] > target[QUAD_SYNC_TIME]) value[QUAD_SYNC_TIME] = target[QUAD_SYNC_TIME];
	else if(primary_ind == QUAD_SYNC_DISTANCE)
	{
		if(value[QUAD_SYNC_TIME] > 0.0f && value[QUAD_SYNC_DISTANCE] > target[QUAD_SYNC_DISTANCE]) value[QUAD_SYNC_DISTANCE] = target[QUAD_SYNC_DISTANCE];
		if(value[QUAD_SYNC_TIME] < 0.0f && value[QUAD_SYNC_DISTANCE] < target[QUAD_SYNC_DISTANCE]) value[QUAD_SYNC_DISTANCE] = target[QUAD_SYNC_DISTANCE];
	}
}

void MeCtLocomotionQuadraticSynchronizer::set_bound(int index, float max, float min)
{
	if(index < 0 || index > 3) printf("MeCtLocomotionQuadraticSynchronizer::set_bound(): Error");
	this->max[index] = max;
	this->min[index] = min;
}

void MeCtLocomotionQuadraticSynchronizer::update_for_speed_distance(float time)
{
	float a = 0.0f;
	if(target[QUAD_SYNC_DISTANCE]<0.0f && value[QUAD_SYNC_SPEED]>0.0f 
		|| target[QUAD_SYNC_DISTANCE]>0.0f && value[QUAD_SYNC_SPEED]<0.0f)
	{
		if(target[QUAD_SYNC_DISTANCE]>0.0f) a = max[QUAD_SYNC_ACCELERATION];
		else if(target[QUAD_SYNC_DISTANCE]< 0.0f) a = -max[QUAD_SYNC_ACCELERATION];
		float v = value[QUAD_SYNC_SPEED] + a*time;
		if(v > 0.0f && a < 0.0f || v < 0.0f && a > 0.0f)
		{
			time = -value[QUAD_SYNC_SPEED]/a;
			if(time<0.0f) printf("time");
			delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*a*time*time;
			value[QUAD_SYNC_SPEED] = 0.0f;
		}
		else delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*a*time*time;
		value[QUAD_SYNC_SPEED] += a*time;
		value[QUAD_SYNC_ACCELERATION] = a;
	}
	else if(target[QUAD_SYNC_DISTANCE] == 0.0f)
	{
		//value[QUAD_SYNC_SPEED] = 0.0f;
		delta_distance = value[QUAD_SYNC_SPEED]*time;
		value[QUAD_SYNC_ACCELERATION] = 0.0f;
	}
	else
	{
		a = -0.5f*value[QUAD_SYNC_SPEED]*value[QUAD_SYNC_SPEED]/target[QUAD_SYNC_DISTANCE];
		if(a < -max[QUAD_SYNC_ACCELERATION] || a > max[QUAD_SYNC_ACCELERATION])
		{
			if(a < -max[QUAD_SYNC_ACCELERATION]) a = -max[QUAD_SYNC_ACCELERATION];
			else a = max[QUAD_SYNC_ACCELERATION];
			delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*a*time*time;
			value[QUAD_SYNC_SPEED] += a*time;
			value[QUAD_SYNC_ACCELERATION] = a;
			if(target[QUAD_SYNC_DISTANCE] < 0.0f && delta_distance < target[QUAD_SYNC_DISTANCE]
			|| target[QUAD_SYNC_DISTANCE] > 0.0f && delta_distance > target[QUAD_SYNC_DISTANCE])
			{
				value[QUAD_SYNC_SPEED] = 0.0f;
				delta_distance = target[QUAD_SYNC_DISTANCE];
				value[QUAD_SYNC_ACCELERATION] = 0.0f;
			}
		}
		else
		{
			if(target[QUAD_SYNC_DISTANCE]>0.0f) 
				a = max[QUAD_SYNC_ACCELERATION];
			else if(target[QUAD_SYNC_DISTANCE]<0.0f)
				a = -max[QUAD_SYNC_ACCELERATION];
			delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*a*time*time;
			value[QUAD_SYNC_SPEED] += a*time;
			value[QUAD_SYNC_ACCELERATION] = a;
			if(target[QUAD_SYNC_DISTANCE] < 0.0f && delta_distance < target[QUAD_SYNC_DISTANCE]/2
			|| target[QUAD_SYNC_DISTANCE] > 0.0f && delta_distance > target[QUAD_SYNC_DISTANCE]/2)
			{
				delta_distance = target[QUAD_SYNC_DISTANCE]/2.0f;
			}
		}

	}
	/*printf("\ndelta_distance: %f", delta_distance);
	printf(" time:%f, a:%f [", time, a);
	for(int i = 0; i < 4; ++i)
	{
		printf("%f, ", value[i]);
	}
	printf("]");*/
}
/*void MeCtLocomotionQuadraticSynchronizer::update_for_speed_distance(float time)
{
	if(target[QUAD_SYNC_DISTANCE] == value[QUAD_SYNC_DISTANCE]) return;
	float a = 0.5f*(target[QUAD_SYNC_SPEED]*target[QUAD_SYNC_SPEED]-value[QUAD_SYNC_SPEED]*value[QUAD_SYNC_SPEED])/-target[QUAD_SYNC_DISTANCE];
	if(a == 0.0f)
	{
		if(target[QUAD_SYNC_DISTANCE]>0.0f) a = abs(max[QUAD_SYNC_ACCELERATION]);
		else a = -abs(max[QUAD_SYNC_ACCELERATION]);
	}

	if(a>max[QUAD_SYNC_ACCELERATION]) a = max[QUAD_SYNC_ACCELERATION];
	else if(-a < -max[QUAD_SYNC_ACCELERATION]) a = -max[QUAD_SYNC_ACCELERATION];

	value[QUAD_SYNC_ACCELERATION] = a;
	

	delta_distance = value[QUAD_SYNC_SPEED]*time+0.5f*value[QUAD_SYNC_ACCELERATION]*time*time;
	value[QUAD_SYNC_DISTANCE] += delta_distance;
	float t_speed = value[QUAD_SYNC_SPEED];
	value[QUAD_SYNC_SPEED] += value[QUAD_SYNC_ACCELERATION]*time;
	if(primary_ind == QUAD_SYNC_SPEED)
	{
		if(t_speed < target[QUAD_SYNC_SPEED] && value[QUAD_SYNC_SPEED] > target[QUAD_SYNC_SPEED]) 
			value[QUAD_SYNC_SPEED] = target[QUAD_SYNC_SPEED];
		else if(t_speed > target[QUAD_SYNC_SPEED] && value[QUAD_SYNC_SPEED] < target[QUAD_SYNC_SPEED])
			value[QUAD_SYNC_SPEED] = target[QUAD_SYNC_SPEED];
	}
	else if(primary_ind == QUAD_SYNC_DISTANCE)
	{
		if(value[QUAD_SYNC_ACCELERATION] > 0.0f && value[QUAD_SYNC_DISTANCE] > target[QUAD_SYNC_DISTANCE]) value[QUAD_SYNC_DISTANCE] = target[QUAD_SYNC_DISTANCE];
		if(value[QUAD_SYNC_ACCELERATION] < 0.0f && value[QUAD_SYNC_DISTANCE] < target[QUAD_SYNC_DISTANCE]) value[QUAD_SYNC_DISTANCE] = target[QUAD_SYNC_DISTANCE];
	}
}*/

float MeCtLocomotionQuadraticSynchronizer::get_value(int index)
{
	if(index < 0 || index > 3) 
	{
		printf("MeCtLocomotionQuadraticSynchronizer::get_value(): Error.");
	}
	return value[index];
}

float MeCtLocomotionQuadraticSynchronizer::get_delta_distance()
{
	return delta_distance;
}