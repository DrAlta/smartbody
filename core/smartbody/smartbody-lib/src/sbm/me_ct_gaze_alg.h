/*
 *  me_ct_gaze_alg.h - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
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
 *      Marcus Thiebaux, USC
 */

#ifndef ME_CT_GAZE_ALG_H
#define ME_CT_GAZE_ALG_H

#include "gwiz_math.h"

//////////////////////////////////////////////////////////////////////////

quat_t rotation_ray_to_target_orient(
	quat_t Q,     // target orientation
	vector_t Fd,  // forward ray direction
	vector_t Fr = vector_t( 0.0, 0.0, 1.0 ) // null reference ray direction
);

quat_t rotation_ray_to_target_point(
	vector_t X,   			// target
	vector_t R,   			// center of rotation
	vector_t Fo,  			// forward ray origin
	vector_t Fd,  			// forward ray direction
	gw_float_t buffer_ratio = 0.1,	// proportion of buffer zone for pathological case
	int heading_only = false
);

void test_forward_ray( void );

//////////////////////////////////////////////////////////////////////////
#endif
