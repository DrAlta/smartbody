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

#ifndef ME_CT_LOCOMOTION_QUADRATIC_SYNCHRONIZER_HPP
#define ME_CT_LOCOMOTION_QUADRATIC_SYNCHRONIZER_HPP

#define QUAD_SYNC_TIME 0
#define QUAD_SYNC_DISTANCE 1
#define QUAD_SYNC_ACCELERATION 2
#define QUAD_SYNC_SPEED 3

#pragma once

#include <iostream>

class MeCtLocomotionQuadraticSynchronizer
{
protected:
	float target[4];
	float value[4];
	float max[4];
	float min[4];
	bool flag[4];
	int primary_ind;

	float delta_distance;
	

public:
	MeCtLocomotionQuadraticSynchronizer();
	~MeCtLocomotionQuadraticSynchronizer();

public:
	void set_target_flag(int ind1, int ind2);
	void update(float time);
	void update_target(int ind, float target_value);
	void update_for_time_speed(float time);
	void update_for_time_distance(float time);
	void update_for_speed_distance(float time);
	float get_value(int index);
	float get_delta_distance();
	void set_bound(int index, float max, float min);
};



#endif // ME_CT_LOCOMOTION_QUADRATIC_SYNCHRONIZER_HPP
