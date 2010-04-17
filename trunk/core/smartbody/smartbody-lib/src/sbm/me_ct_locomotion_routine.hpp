/*
 *  me_ct_locomotion_routine.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_ROUTINE_HPP
#define ME_CT_LOCOMOTION_ROUTINE_HPP

#include "SR\sr_vec.h"

#pragma once

#define ME_CT_LOCOMOTION_ROUTINE_TYPE_STRAIGHT 0
#define ME_CT_LOCOMOTION_ROUTINE_TYPE_CIRCULAR 1


// Routine definition for locomotion. The effects of Routine can be accumulated. 
class MeCtLocomotionRoutine
{
public:
	char name[20];
	int id;

public:
	SrVec direction;
	float speed;
	int type; //ME_CT_LOCOMOTION_ROUTINE_TYPE_STRAIGHT or ME_CT_LOCOMOTION_ROUTINE_TYPE_CIRCULAR
	float global_rps;
	float local_rps;
	//SrVec start_pos;
	//double elapsed_time;

public:
	MeCtLocomotionRoutine();
	~MeCtLocomotionRoutine();
};

#endif // ME_CT_LOCOMOTION_ROUTINE_HPP
