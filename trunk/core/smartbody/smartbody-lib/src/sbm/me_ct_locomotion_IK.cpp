/*
 *  me_ct_locomotion_IK.hpp - part of SmartBody-lib's Motion Engine
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

#include "me_ct_locomotion_IK.hpp"



MeCtLocomotionIK::MeCtLocomotionIK():MeCtIK()
{

}

MeCtLocomotionIK::~MeCtLocomotionIK()
{
}


//overwrite function calc_target of MeCtIK
__forceinline void MeCtLocomotionIK::calc_target(SrVec& orientation, SrVec& offset)
{
	orientation.normalize();
	SrVec pos = joint_pos_list.get(manipulated_joint_index);
	pos += offset;

	float normal[3];

	float height = terrain.get_height(pos.x, pos.z, normal);
	
	//SrVec plane_normal = SrVec(normal[0], normal[1], normal[2]);
	//SrVec plane_point = SrVec(pos.x, height, pos.z);
	//SrVec t = cross_point_on_plane(pos, orientation, plane_normal, plane_point);
	SrVec t = cross_point_on_plane(pos, orientation, scenario->plane_normal, scenario->plane_point);
	if(t.y < height) t.y = height;
	target.set(manipulated_joint_index, -(manipulated_joint->support_joint_comp + manipulated_joint->support_joint_height)*orientation + t);
	//if(manipulated_joint_index == 2) printf("\n(%f, %f, %f)", target.get(manipulated_joint_index).x, target.get(manipulated_joint_index).y, target.get(manipulated_joint_index).z);
}


