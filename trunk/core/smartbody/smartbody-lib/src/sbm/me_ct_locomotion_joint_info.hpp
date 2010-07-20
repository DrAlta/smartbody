/*
 *  me_ct_locomotion_joint_info.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_JOINT_INFO_HPP
#define ME_CT_LOCOMOTION_JOINT_INFO_HPP

#include <ME/me_controller.h>

#include "me_ct_locomotion_limb.hpp"
#include "me_ct_locomotion_func.hpp"


class MeCtLocomotionLimb;

#pragma once

struct MeCtLocomotionJointInfo
{
	int joint_num;
	SrArray<const char*> joint_name;
	SrArray<int> buff_index;
	SrArray<int> joint_index;
	SrArray<SrQuat> quat;

	int iterate(SkJoint* joint, SrArray<char*>* limb_joint_name)
	{
		int sum = 0;
		const char* name = joint->name().get_string();
		if(limb_joint_name != NULL)
		{
			for(int i = 0; i < limb_joint_name->size(); ++i)
			{
				if(strcmp(limb_joint_name->get(i), name) == 0) return 0;
			}
		}
		if(joint->quat()->active() == true)
		{
			joint_name.push() = name;
			joint_index.push() = joint->index();
			sum = 1;
		}

		for(int i = 0; i < joint->num_children(); ++i)
		{
			sum += iterate(joint->child(i), limb_joint_name);
		}
		return sum;
	}

	void Init(SkSkeleton* skeleton, char* base_name, SrArray<char*>* limb_joint_name)
	{
		joint_name.capacity(0);
		joint_index.capacity(0);
		SkJoint* tjoint = skeleton->search_joint(base_name);
		joint_num = iterate(tjoint, limb_joint_name);
		quat.capacity(joint_num);
		quat.size(joint_num);
		buff_index.capacity(joint_num);
		buff_index.size(joint_num);
	}
};


#endif // ME_CT_LOCOMOTION_JOINT_INFO_HPP
