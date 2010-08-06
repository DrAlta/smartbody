/*
 *  me_ct_IK_scenario.cpp - part of SmartBody-lib's Motion Engine
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
 *      Jingqiao Fu
 */

#include "me_ct_IK.hpp"

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtIK::TYPE = "MeCtIK";


/** Constructor */
MeCtIK::MeCtIK() {

	max_iteration = 20;
	threshold = 0.01f;
	recrod_endmat = 0;
}

/** Destructor */
MeCtIK::~MeCtIK() {
	// Nothing allocated to the heap
}

void MeCtIK::set_max_iteration(int iter)
{
	max_iteration = iter;
}

void MeCtIK::update(MeCtIKScenario* scenario)
{
	this->scenario = scenario;
	int i, j;
	int reach = 0;
	int modified = 0;
	SrMat inv_end;

	//temp.........delete this
	SrQuat before = scenario->quat_list.get(2);
	//temp.........delete this
	
	init();

	for(int k = 0; k < support_joint_num; ++k)
	{
		//LOG("\ntarget: (%f, %f, %f)", target.x, target.y, target.z);
		reach = 0;
		for(i = 0; i < max_iteration; ++i)
		{
			for(j = 0; j != manipulated_joint_index; ++j)
			{
				if(reach_destination()) 
				{
					reach = 1;
					break;
				}
				rotate(joint_pos_list.get(manipulated_joint_index), j);
			}
			if(reach) break;
		}

		//handles the support joints
		SrQuat before = scenario->quat_list.get(manipulated_joint_index);
		//temp

		pm = joint_global_mat_list.get(manipulated_joint_index);
		pm.set(12, 0.0f);//??
		pm.set(13, 0.0f);
		pm.set(14, 0.0f);

		lm = joint_init_mat_list.get(manipulated_joint_index);
		lm.set(12, 0.0f);//??
		lm.set(13, 0.0f);
		lm.set(14, 0.0f);

		inv_end = lm*pm.inverse();
		before = inv_end * before;
		SrQuat after = before;
		//temp

		/*inv_end = end_mat.inverse();
		before = inv_end * before;
		SrQuat after = before;*/
		modified = check_constraint(&after, manipulated_joint_index);
		scenario->quat_list.set(manipulated_joint_index, before);
		if(modified == 0) break;
		else 
		{
			recrod_endmat = 0;
			get_next_support_joint();
			get_limb_section_local_pos(0, -1);
			scenario->quat_list.set(manipulated_joint_index-1, after);
			calc_target();
			//end_mat.rot(after.axis(), after.angle() - before.angle());
		}
	}
	
}

void MeCtIK::adjust_support_joints()
{
	for(int i = manipulated_joint_index; i < scenario->joint_info_list.size(); ++i)
	{
		if(scenario->joint_info_list.get(i).is_support_joint)
		{
			
		}
	}
}

int MeCtIK::get_support_joint_num()
{
	int num = 0;
	for(int i = 0; i < scenario->joint_info_list.size(); ++i)
	{
		if(scenario->joint_info_list.get(i).is_support_joint) ++num;
	}
	return num;
}

__forceinline int MeCtIK::check_constraint(SrQuat* quat, int index)
{
	float angle = quat->angle();
	int modified = 0;
	MeCtIKScenarioJointInfo* info = &(scenario->joint_info_list.get(index));
	if(info->type == JOINT_TYPE_HINGE)
	{
		if(info->constraint.hinge.max > 0.0f && angle > info->constraint.hinge.max) 
		{
			angle = info->constraint.hinge.max;
			modified = 1;
		}
		else if(info->constraint.hinge.min < 0.0f && angle < info->constraint.hinge.min) 
		{
			angle = info->constraint.hinge.min;
			modified = 1;
		}
	}
	else if(info->type == JOINT_TYPE_BALL && info->constraint.ball.max > 0.0f)
	{
		if(angle > info->constraint.ball.max) 
		{
			angle = info->constraint.ball.max;
			modified = 1;
		}
		else if(angle < -info->constraint.ball.max) 
		{
			angle = -info->constraint.ball.max;
			modified = 1;
		}
	}
	if(modified) quat->set(quat->axis(), angle);
	return modified;
}

__forceinline int MeCtIK::reach_destination()
{
	SrVec v = joint_pos_list.get(manipulated_joint_index) - target;
	//LOG("\n dis: %f", v.len());
	if(v.len() < threshold) return 1;
	return 0;
}

__forceinline float MeCtIK::distance_to_plane(SrVec& point, SrVec& plane_normal, SrVec& plane_point)
{
	SrVec v;
	plane_normal.normalize();
	v = point - plane_point;
	float dis = dot(plane_normal, v);
	return dis;
}

__forceinline SrVec MeCtIK::upright_point_to_plane(SrVec& point, SrVec& plane_normal, SrVec& plane_point)
{
	float dis = distance_to_plane(point, plane_normal, plane_point);
	return (point - plane_normal*dis);
}

__forceinline bool MeCtIK::cross_point_with_plane(SrVec* cross_point, SrVec& line_point, SrVec& direction, SrVec& plane_normal, SrVec& plane_point)
{
	float angle = dot(direction, plane_normal);
	if(angle == 0.0f) return false;
	SrVec v;
	v = plane_point - line_point;
	v = direction * dot(v, plane_normal)/angle;
	*cross_point = v + line_point;
	return true;
}

__forceinline void MeCtIK::update_limb_section_local_pos(int start_index)
{
	/*SrVec v;
	for(int i = start_index; i < joint_pos_list->size()-1; ++i)
	{
		v = joint_pos_list.get(j);
		v = mat * v;
		joint_pos_list.set(j, v);
	}*/


	//temp solution, for efficiency, this must be replaced.
	get_limb_section_local_pos(start_index, -1);
	//temp solution, for efficiency, this must be replaced.
}

__forceinline void MeCtIK::rotate(SrVec& src, int start_index)
{

	/*SrVec ppp(11,0,0);
	SrMat matk;
	matk.rot(SrVec(1,1,1), 45);
	ppp = matk*ppp;
	matk = matk.inverse();
	ppp = matk*ppp;*/

	SrVec v1, v2, v3, v4;
	SrVec v, i_target, i_src;
	SrVec axis, r_axis;
	SrMat mat, mat_inv;
	if(start_index == 0) mat_inv = scenario->mat.inverse();
	else mat_inv = joint_global_mat_list.get(start_index-1).inverse();
	SrVec& pivot = joint_pos_list.get(start_index);
	pivot = pivot * mat_inv;
	i_target = target * mat_inv;
	i_src = src * mat_inv;

	v4 = i_target - i_src;
	v1 = i_src - pivot;
	if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_BALL)
	{
		//compute the axis 
		v2 = i_target - pivot;
		axis = cross(v2, v1);
		r_axis = axis;
	}
	else if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_HINGE)
	{
		r_axis = SrVec(1,0,0);
		//r_axis = joint_axis_list.get(start_index);
		v = upright_point_to_plane(i_target, r_axis, i_src);
		v2 = v - pivot;
	}
	
	v1.normalize();
	v2.normalize();
	float dot_v = dot(v1, v2);
	if(dot_v > 1.0f) dot_v = 1.0f;
	float angle = (float)acos(dot_v);

	v3 = cross(v1, v2);
	if(dot(v3, r_axis) > 0.0f) mat.rot(r_axis, angle);
	else mat.rot(r_axis, -angle);
	
	SrQuat q = scenario->quat_list.get(start_index);
	q = mat * q;

	check_constraint(&q, start_index);

	//end_mat = mat * end_mat;
	scenario->quat_list.set(start_index, q);
	get_limb_section_local_pos(start_index, -1);
	//update_manipulated_joint_pos(start_index);
}

/*__forceinline void MeCtIK::rotate(SrVec& src, int start_index)
{
	SrVec v1, v2, v3, v4;
	SrVec v;
	SrVec axis, r_axis;
	SrMat mat;
	SrVec& pivot = joint_pos_list.get(start_index);
	v4 = target - src;
	v1 = src - pivot;
	if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_BALL)
	{
		//compute the axis 
		v2 = target - pivot;
		axis = cross(v2, v1);
		r_axis = axis;
	}
	else if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_HINGE)
	{
		axis = joint_axis_list.get(start_index);
		v = upright_point_to_plane(target, axis, src);
		v2 = v - pivot;
		r_axis = SrVec(1,0,0);
	}
	
	v1.normalize();
	v2.normalize();
	float dot_v = dot(v1, v2);
	if(dot_v > 1.0f) dot_v = 1.0f;
	float angle = (float)acos(dot_v);

	v3 = cross(v1, v2);
	if(dot(v3, axis) > 0.0f) mat.rot(r_axis, angle);
	else mat.rot(r_axis, -angle);
	
	SrQuat q = scenario->quat_list.get(start_index);
	q = mat * q;

	check_constraint(&q, start_index);

	//end_mat = mat * end_mat;
	scenario->quat_list.set(start_index, q);
	get_limb_section_local_pos(start_index, -1);
	//update_manipulated_joint_pos(start_index);
}*/

__forceinline void MeCtIK::calc_target()
{
	SrVec pos = joint_pos_list.get(manipulated_joint_index);
	target = (manipulated_joint->support_joint_comp + manipulated_joint->support_joint_height - distance_to_plane(pos, scenario->plane_normal, scenario->plane_point)) * scenario->plane_normal + pos;
}

__forceinline void MeCtIK::get_next_support_joint()
{
	int i;
	for(i = manipulated_joint_index+1; i < scenario->joint_info_list.size(); ++i)
	{
		if(scenario->joint_info_list.get(i).is_support_joint)
		{
			manipulated_joint_index = i;
			manipulated_joint = &(scenario->joint_info_list.get(manipulated_joint_index));
			break;
		}
	}

	if(manipulated_joint == NULL)// in case the user sets no support joint. set the manipulated joint the last joint
	{
		manipulated_joint_index = scenario->joint_info_list.size()-1;
		manipulated_joint = &(scenario->joint_info_list.get(manipulated_joint_index));
	}
}

void MeCtIK::init()
{
	manipulated_joint = NULL;
	recrod_endmat = 0;
	end_mat.identity();

	int size = scenario->quat_list.size();

	joint_pos_list.capacity(size);
	joint_pos_list.size(size);
	joint_axis_list.capacity(size);
	joint_axis_list.size(size);
	//joint_local_mat_list.capacity(size);
	//joint_local_mat_list.size(size);
	joint_global_mat_list.capacity(size);
	joint_global_mat_list.size(size);
	joint_init_mat_list.capacity(size);
	joint_init_mat_list.size(size);

	get_limb_section_local_pos(0, -1);

	get_init_mat_list();

	scenario->plane_normal.normalize();
	manipulated_joint_index = -1;
	support_joint_num = get_support_joint_num();
	get_next_support_joint();
	calc_target();
	adjust_support_joints();
}

__forceinline void MeCtIK::get_init_mat_list()
{
	joint_init_mat_list = joint_global_mat_list;
}

__forceinline void MeCtIK::update_manipulated_joint_pos(int index)
{
	joint_pos_list.set(manipulated_joint_index, joint_global_mat_list.get(index) * joint_pos_list.get(manipulated_joint_index));
}

__forceinline void MeCtIK::get_limb_section_local_pos(int start_index, int end_index)
{
	SrMat gmat;
	SrMat pmat;
	SrMat lmat;
	SrVec axis;
	SrVec ppos;
	float* pt;
	SkJoint* tjoint = scenario->start_joint->sk_joint;
	for(int i = 0; i < start_index; ++i)
	{
		tjoint = tjoint->child(0);
	}

	if(end_index < 0 || end_index > scenario->joint_info_list.size()-1) end_index = scenario->joint_info_list.size()-1;

	for(int j  = start_index; j <= end_index; ++j)
	{
		scenario->joint_info_list.get(j).index = j;
		if(j == 0) pmat = scenario->mat;
		else pmat = joint_global_mat_list.get(j-1);

		lmat = get_lmat(tjoint, &(scenario->quat_list.get(j)));
		//joint_local_mat_list.set(j, lmat);

		gmat = lmat * pmat;
		joint_global_mat_list.set(j, gmat);

		ppos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
		joint_pos_list.set(j, ppos);

		if(scenario->joint_info_list.get(j).type == JOINT_TYPE_BALL)
		{

		}
		else if(scenario->joint_info_list.get(j).type == JOINT_TYPE_HINGE)
		{
			pmat = gmat;
			pt = pmat.pt(12);
			pt[0] = 0.0f;
			pt[1] = 0.0f;
			pt[2] = 0.0f;
			axis = scenario->joint_info_list.get(j).axis;
			SrVec n_axis = pmat*axis;
			pmat.rot(axis, n_axis);
			axis = pmat*axis;
			joint_axis_list.set(j, axis);
		}

		if(tjoint->num_children()>0 && tjoint != scenario->end_joint->sk_joint)
		{
			tjoint = tjoint->child(0);
		}
		else break;
	}
}
