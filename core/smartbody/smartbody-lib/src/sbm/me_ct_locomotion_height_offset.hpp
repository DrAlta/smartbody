/*
 *  me_ct_locomotion_height_offset.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_HEIGHT_OFFSET_HPP
#define ME_CT_LOCOMOTION_HEIGHT_OFFSET_HPP

#include "me_ct_locomotion_limb.hpp"

#pragma once

class MeCtLocomotionHeightOffset
{
protected:
	SrArray<MeCtLocomotionLimb*>* limb_list;
	float acceleration;

	float acceleration_max;
	float acceleration_min;

	float height_offset;

	float translation_base_joint_height;

	SrArray<SrVec> joint_pos_prev;
	SrArray<SrVec> joint_pos_curr;

public:
	MeCtLocomotionHeightOffset();
	~MeCtLocomotionHeightOffset();

public:
	void set_limb_list(SrArray<MeCtLocomotionLimb*>* limb_list);
	void set_translation_base_joint_height(float height);
	void update(SrMat& parent_mat);
	float get_height_offset();

};



#endif // ME_CT_LOCOMOTION_HEIGHT_OFFSET_HPP
