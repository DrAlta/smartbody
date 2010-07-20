/*
 *  me_ct_locomotion_func.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_FUNC_HPP
#define ME_CT_LOCOMOTION_FUNC_HPP

#include <SR/sr_hash_table.h>
#include <SR/sr_buffer.h>
#include <SR/sr_shared_class.h>
#include <SK/sk_channel_array.h>
#include <SK/sk_motion.h>
#include <SK/sk_posture.h>
#include <SK/sk_skeleton.h>

#include "me_ct_locomotion_joint_info.hpp"

#pragma once

SrMat get_lmat(SkJoint* joint, SrQuat* quat);

SrArray<SrQuat>* get_blended_quat_buffer(MeCtLocomotionJointInfo* joint_info, float weight);
SrArray<SrQuat>* get_blended_quat_buffer(SrArray<SrQuat>* dest, SrArray<SrQuat>* quat_buffer1, SrArray<SrQuat>* quat_buffer2, float weight);

int iterate_set(SkJoint* base, int index, int depth, SrArray<SrQuat>* buff, SrArray<int>* index_buff);

void get_frame(SkMotion* walking, SkSkeleton* walking_skeleton, float frame, char* limb_base, MeCtLocomotionJointInfo* joint_info);
void get_frame(SkMotion* walking, SkSkeleton* walking_skeleton, float frame, char* limb_base, SrArray<SrQuat>* quat_buffer, SrArray<SrQuat>* quat_buffer1, SrArray<SrQuat>* quat_buffer2, SrArray<int>* index_buff);


#endif // ME_CT_LOCOMOTION_FUNC_HPP
