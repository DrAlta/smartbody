#include "me_ct_reach_IK.hpp"


#include <SR/sr_alg.h>


MeCtReachIK::MeCtReachIK(void)
{
	max_iteration = 30;
}

MeCtReachIK::~MeCtReachIK(void)
{
}

void MeCtReachIK::adjust()
{
	int reach = 0;
	int i,j;
	for(i = 0; i < max_iteration; ++i)
	{
		//for(j = start_joint_index; j != manipulated_joint_index; ++j)
		for(j = manipulated_joint_index-1 ; j >= start_joint_index; j--) 
		{
			/*if(reach_destination()) 
			{
				reach = 1;
				break;
			}*/
			ccdRotate(scenario->joint_pos_list.get(manipulated_joint_index), j);
		}
		if(reach) break;
	}
}

int MeCtReachIK::check_joint_limit( SrQuat* quat, int index )
{
	//return 0;	
	int modified = 0;
	MeCtIKScenarioJointInfo* info = &(scenario->joint_info_list.get(index));

	// for some reason, the twist axis is aligned with local x-axis, instead of z-axis
	// therefore we need to do some "shift" version of swingTwist conversion.
	// so we map x->z, y->x, z->y for computing swing-twist parameteriation.
	// all of the angle limits are adjust accordingly.
	{
		MeCtIKJointLimit& limit = info->joint_limit;
		quat_t quat_st = quat_t(quat->w, quat->x, quat->y, quat->z);			
		vector_t st = quat2SwingTwist(quat_st);//quat_st.swingtwist();
		float sw_y = (float)st.x();
		float sw_z = (float)st.y();		

		float sw_limit_z, sw_limit_y;
		sw_limit_z = sw_z > 0 ? limit.x_limit[0] : limit.x_limit[1];
		sw_limit_y = sw_y > 0 ? limit.y_limit[0] : limit.y_limit[1];

		// project swing angles onto the joint limit ellipse
		if( sr_in_ellipse( sw_z, sw_y, sw_limit_z, sw_limit_y ) > 0.0 )	{
			sr_get_closest_on_ellipse( sw_limit_z, sw_limit_y, sw_z, sw_y );			
		}		
		// handle twist angle limit
		float tw = (float)st.z(); 
 		if( tw > limit.twist_limit[0] ) tw = limit.twist_limit[0];
 		if( tw < limit.twist_limit[1] ) tw = limit.twist_limit[1];
		
		quat_t ql = swingTwist2Quat(vector_t(sw_y, sw_z, tw));//quat_t( sw_x, sw_y, tw);
		quat->set((float)ql.w(),(float)ql.x(),(float)ql.y(),(float)ql.z());
	}	
	return modified;
}


void MeCtReachIK::ccdRotate(SrVec& src, int start_index)
{
	SrVec v1, v2, v3, v4;
	SrVec v, i_target, i_src;
	//SrVec axis, r_axis;
	SrVec r_axis;
	SrQuat q;
	SrMat mat, mat_inv;

	MeCtIKScenarioJointInfo* info = &(scenario->joint_info_list.get(start_index));
    float damping_angle = (float)RAD(info->angular_speed_limit*getDt());

	if(start_index == 0) mat_inv = scenario->gmat.inverse();
	else mat_inv = scenario->joint_global_mat_list.get(start_index-1).inverse();
	SrVec& pivot = scenario->joint_pos_list.get(start_index);
	pivot = pivot * mat_inv; // tranverse joint back to its local coordinate
	i_target = target.get(manipulated_joint_index) * mat_inv;
	i_src = src * mat_inv;
	
	v1 = i_src - pivot;
	v1.normalize();


	if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_BALL)
	{
		v2 = i_target - pivot;
		v2.normalize();

		double dot_v = dot(v1, v2);
		if(dot_v >= 0.9999995000000f) 
		{
			if(start_index >= scenario->joint_pos_list.size()-1) return;
			dot_v = 1.0f;
// 			v3 = scenario->joint_pos_list.get(start_index+1);
// 			v3 = v3 * mat_inv; // tranverse joint back to its local coordinate
// 			v2 = v3 - pivot;
		}
		else if(dot_v < -1.0f) dot_v = -1.0f;
		double angle = acos(dot_v);

		if (angle > damping_angle) angle = damping_angle;

		r_axis = cross(v2, v1);
		r_axis.normalize();

		mat.rot(r_axis, (float)-angle);
	}
	else if(scenario->joint_info_list.get(start_index).type == JOINT_TYPE_HINGE)
	{
		r_axis = scenario->joint_info_list.get(start_index).axis;
		v = upright_point_to_plane(i_target, r_axis, i_src);
		v2 = v - pivot;
		v2.normalize();
		
		double dot_v = dot(v1, v2);
		if(dot_v > 1.0f) dot_v = 1.0f;
		else if(dot_v < -1.0f) dot_v = -1.0f;
		double angle = acos(dot_v);

		if (angle > damping_angle) angle = damping_angle;

		v3 = cross(v1, v2);
		v3.normalize();
		if(dot(v3, r_axis) > 0.0f) mat.rot(r_axis, (float)angle);
		else mat.rot(r_axis, (float)-angle);
	}
	
	
	q = scenario->joint_quat_list.get(start_index);
	q = mat * q;

	check_joint_limit(&q,start_index);

	scenario->joint_quat_list.set(start_index, q);
	get_limb_section_local_pos(start_index, -1);
}


void MeCtReachIK::update(MeCtIKScenario* scenario)
{
	this->scenario = scenario;
	int modified = 0;
	SrMat inv_end;

	SrQuat quat;
	SrMat mat;	
	init();
	//unstretch_joints();
	adjust();	
}

void MeCtReachIK::calc_target(SrVec& orientation, SrVec& offset)
{
	SrVec pos = scenario->joint_pos_list.get(manipulated_joint_index);
	target.set(manipulated_joint_index,offset);
}


#define INVERT_XZ

vector_t MeCtReachIK::quat2SwingTwist( quat_t& quat )
{
	const float EPSILON6 = 0.0000001f;

#ifdef INVERT_XZ	
	if( ( quat.w() < EPSILON6 )&&( quat.x() < EPSILON6 ) )	{
		return( vector_t( 0.0, 0.0, M_PI ) );
	}
#else
	if( ( quat.w() < EPSILON6 )&&( quat.z() < EPSILON6 ) )	{
		return( vector_t( M_PI, 0.0, 0.0 ) );
	}
#endif

	quat_t q;
	if( quat.w() < 0.0 )	{
		q = quat.complement();
	}
	else	{
		q = quat;
	}

#ifdef INVERT_XZ
	gw_float_t gamma = atan2( q.x(), q.w() );
	gw_float_t beta = atan2( sqrt( q.z()*q.z() + q.y()*q.y() ), sqrt( q.x()*q.x() + q.w()*q.w() ) );
	gw_float_t sinc = 1.0;
	if( beta > EPSILON6 )	{
		sinc = sin( beta )/beta;
	}
	gw_float_t s = sin( gamma );
	gw_float_t c = cos( gamma );
	gw_float_t sinc2 = 2.0 / sinc;
	gw_float_t swing_x = sinc2 * ( c * q.y() - s * q.z());
	gw_float_t swing_y = sinc2 * ( s * q.y() + c * q.z());
	gw_float_t twist = 2.0 * gamma;
#else
	gw_float_t gamma = atan2( q.z(), q.w() );
	gw_float_t beta = atan2( sqrt( q.x()*q.x() + q.y()*q.y() ), sqrt( q.z()*q.z() + q.w()*q.w() ) );
	gw_float_t sinc = 1.0;
	if( beta > EPSILON6 )	{
		sinc = sin( beta )/beta;
	}
	gw_float_t s = sin( gamma );
	gw_float_t c = cos( gamma );
	gw_float_t sinc2 = 2.0 / sinc;
	gw_float_t swing_x = sinc2 * ( c * q.x() - s * q.y() );
	gw_float_t swing_y = sinc2 * ( s * q.x() + c * q.y());
	gw_float_t twist = 2.0 * gamma;
#endif

// 	if( use_radians )	{
// 		return( vector_t( swing_x, swing_y, twist ) );
// 	}
	return( vector_t( ( swing_x ), ( swing_y ), ( twist ) ) );
}

quat_t MeCtReachIK::swingTwist2Quat( vector_t& sw )
{
#ifdef INVERT_XZ
	quat_t swing = quat_t( vector_t( 0.0, sw.x(), sw.y() ) );	
    quat_t result = swing * quat_t( sw.z(), vector_t( 1.0, 0.0, 0.0 ), 1);
#else
	quat_t swing = quat_t(vector_t( sw.x(), sw.y(), 0.0) );	
	quat_t result = swing * quat_t( sw.z(), vector_t( 0.0, 0.0, 1.0 ), 1);
#endif
	
	return result;
}

