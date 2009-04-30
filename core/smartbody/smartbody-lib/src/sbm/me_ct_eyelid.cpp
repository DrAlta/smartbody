/*
 *  me_ct_tether.cpp - part of SmartBody-lib
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
 *      Andrew n marshall, USC
 */

#include "gwiz_math.h"
#include "me_ct_eyelid.h"

/*
	JOINTS:
	
		eyeball_left
		eyeball_right

		lower_eyelid_left
		upper_eyelid_left
		
		lower_eyelid_right
		upper_eyelid_right
		
	DEPRECATED REF MOTIONS:
		face_neutral.skm			: eyes open
		fac_45_blink.skm			: eyes closed
		fac_5_upper_lid_raiser.skm	: eyes wide open
*/

/*
	upper_eyelid_left
		offx = 3.15
		offy = 3.038
		offz = 11.75
	
	lookup
		posy = 0.372
		pitch = 0.0
	
	neutral
		posy = 0.0
		pitch = 0.0
	
	flat
		posy = -0.538
		pitch = 0.0

	lookdown
		posy = -0.788
		pitch = 0.0
	
	blink
		posy = -0.87
		pitch = 12.8
*/

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtEyeLid::type_name = "EyeLid";

MeCtEyeLid::MeCtEyeLid( void )	{

   _skeleton_ref_p = NULL;
}

MeCtEyeLid::~MeCtEyeLid( void )	{
	
	clear();
}

void MeCtEyeLid::clear( void )	{
	
   _skeleton_ref_p = NULL;
}

float MeCtEyeLid::get_motion_joint_pitch( SkMotion* mot_p, const char *chan_name ) {

	SkChannelArray& channels = mot_p->channels();
	float *f_arr = mot_p->posture( 0 );
	int f_index = channels.float_position( channels.search( chan_name, SkChannel::Quat ) );
	euler_t e = quat_t( 
		f_arr[ f_index ], 
		f_arr[ f_index + 1 ], 
		f_arr[ f_index + 2 ], 
		f_arr[ f_index + 3 ] 
	);
	return( (float)( e.p() ) );
}

#if 0
float MeCtEyeLid::calculate_upper_correction( 
	float in_lid, 
	float in_eye
	)	{
	// NOTE: 
	//  neutral eyelid is open, and typically 0.
	//  positive rotation closes eyelid.
	float adj_lid = 0.0;
	float out_lid = 0.0;
	float threshhold = neut_lid_deg + 0.5f * ( blink_lid_deg - neut_lid_deg );

//	printf( "lid: %f eye: %f thresh: %f\n", in_lid, in_eye, threshhold );

	adj_lid = in_eye + in_lid;
	if( in_lid <= threshhold )	{ // eye is effectively open
		out_lid = adj_lid;
	}
	else	{ // eye is effectively closing
		float norm = ( in_lid - threshhold ) / ( blink_lid_deg - threshhold );
//		printf( " norm: %f\n", norm );
		out_lid = adj_lid + ( blink_lid_deg - adj_lid ) * norm;
	}

//	printf( " out_lid: %f\n\n", out_lid );
	return( out_lid );
}
#endif

void MeCtEyeLid::print_motion_channel( SkMotion* mot_p, const char *chan_name ) {
		
	SkChannelArray& channels = mot_p->channels();
	float *f_arr = mot_p->posture( 0 );
	int f_index;
	
	float x, y, z;

	f_index = channels.float_position( channels.search( chan_name, SkChannel::XPos ) );
	x = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::YPos ) );
	y = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::ZPos ) );
	z = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::Quat ) );
	euler_t e = quat_t( 
		f_arr[ f_index ], 
		f_arr[ f_index + 1 ], 
		f_arr[ f_index + 2 ], 
		f_arr[ f_index + 3 ] 
	);
	
	printf( "MeCtEyeLid:: '%s'::'%s'\n",
		mot_p->name(), chan_name
	);
	printf( " euler-HPR = %.2f %.2f %.2f\n",
		e.h(), e.p(), e.r()
	);
	printf( " pos-XYZ = %.3f %.3f %.3f\n",
		x, y, z
	);
}

//void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* lkup_p, SkMotion* lkdn_p, SkMotion* flat_p ) {
//void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* raise_p ) {
//void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p ) {
void MeCtEyeLid::init( void ) {
	
	clear();
	
//	_neutral_pose_p = neutral_p;
//	_blink_pose_p = blink_p;
//	_lookup_pose_p = lkup_p;
//	_lookdown_pose_p = lkdn_p;
//	_flat_pose_p = flat_p;
	
//	_neutral_pose_p->ref();
//	_blink_pose_p->ref();
//	_lookup_pose_p->ref();
//	_lookdown_pose_p->ref();
//	_flat_pose_p->ref();
	
//	_neutral_pose_p->move_keytimes( 0 );
//	_blink_pose_p->move_keytimes( 0 );
//	_lookup_pose_p->move_keytimes( 0 );
//	_lookdown_pose_p->move_keytimes( 0 );
//	_flat_pose_p->move_keytimes( 0 );

//	SkChannelArray& mChannels = _neutral_pose_p->channels();
//	float *f_arr = _neutral_pose_p->posture( 0 );
//	int f_pos = mChannels.float_position( mChannels.search( "lower_eyelid_left", SkChannel::YPos ) );
#if 0
	print_motion_channel( _neutral_pose_p, "upper_eyelid_left" ); 
//	print_motion_channel( _neutral_pose_p, "lower_eyelid_left" ); 

	print_motion_channel( _blink_pose_p, "upper_eyelid_left" ); 
//	print_motion_channel( _blink_pose_p, "lower_eyelid_left" ); 

//	print_motion_channel( _lookup_pose_p, "upper_eyelid_left" ); 
//	print_motion_channel( _lookup_pose_p, "lower_eyelid_left" ); 

//	print_motion_channel( _lookdown_pose_p, "upper_eyelid_left" ); 
//	print_motion_channel( _lookdown_pose_p, "lower_eyelid_left" ); 

//	print_motion_channel( _flat_pose_p, "upper_eyelid_left" ); 
//	print_motion_channel( _flat_pose_p, "lower_eyelid_left" ); 
#endif

//	neut_lid_deg = get_motion_joint_pitch( _neutral_pose_p, "upper_eyelid_left" ); // 0.0
//	blink_lid_deg = get_motion_joint_pitch( _blink_pose_p, "upper_eyelid_left" );  // 12.81
//	raise_lid_deg = get_motion_joint_pitch( _raise_pose_p, "upper_eyelid_left" );  // -1.84

#if 0
printf( "neutral: %f\n", neut_lid_deg );
printf( "blink:  %f\n", blink_lid_deg );
//printf( "raise:  %f\n", raise_lid_deg );

	float p;
	printf( "look fwd:\n" );   // ( lid, eye )
	p = calculate_upper_correction( -2.0, 0.0 ); // wide
	p = calculate_upper_correction( 0.0, 0.0 ); // neutral
	p = calculate_upper_correction( 4.0, 0.0 );
	p = calculate_upper_correction( 6.3, 0.0 );
	p = calculate_upper_correction( 6.4, 0.0 ); // threshhold
	p = calculate_upper_correction( 6.5, 0.0 );
	p = calculate_upper_correction( 12.8, 0.0 ); // closed

	printf( "look up:\n" );
	p = calculate_upper_correction( -2.0, -10.0 );
	p = calculate_upper_correction( 0.0, -10.0 );
	p = calculate_upper_correction( 4.0, -10.0 );
	p = calculate_upper_correction( 6.3, -10.0 );
	p = calculate_upper_correction( 6.4, -10.0 );
	p = calculate_upper_correction( 6.5, -10.0 );
	p = calculate_upper_correction( 12.8, -10.0 );

	printf( "look down:\n" );
	p = calculate_upper_correction( -2.0, 10.0 );
	p = calculate_upper_correction( 0.0, 10.0 );
	p = calculate_upper_correction( 4.0, 10.0 );
	p = calculate_upper_correction( 6.3, 10.0 );
	p = calculate_upper_correction( 6.4, 10.0 );
	p = calculate_upper_correction( 6.5, 10.0 );
	p = calculate_upper_correction( 12.8, 10.0 );
#endif


	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );

//	_channels.add( "lower_eyelid_left", SkChannel::XPos );
//	_channels.add( "lower_eyelid_left", SkChannel::YPos );
//	_channels.add( "lower_eyelid_left", SkChannel::ZPos );
//	_channels.add( "lower_eyelid_left", SkChannel::Quat );
	
//	_channels.add( "upper_eyelid_left", SkChannel::XPos );
	_channels.add( "upper_eyelid_left", SkChannel::YPos );
//	_channels.add( "upper_eyelid_left", SkChannel::ZPos );
	_channels.add( "upper_eyelid_left", SkChannel::Quat );
	
//	_channels.add( "lower_eyelid_right", SkChannel::XPos );
//	_channels.add( "lower_eyelid_right", SkChannel::YPos );
//	_channels.add( "lower_eyelid_right", SkChannel::ZPos );
//	_channels.add( "lower_eyelid_right", SkChannel::Quat );
	
//	_channels.add( "upper_eyelid_right", SkChannel::XPos );
	_channels.add( "upper_eyelid_right", SkChannel::YPos );
//	_channels.add( "upper_eyelid_right", SkChannel::ZPos );
	_channels.add( "upper_eyelid_right", SkChannel::Quat );
	
//	UL_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::Quat );

	MeController::init();
}

///////////////////////////////////////////////////////////////////////////////////

SkJoint* MeCtEyeLid::get_joint( char *joint_str, SkJoint *joint_p )	{

	if( joint_str )	{
		if( joint_p == NULL )	{
			if( _skeleton_ref_p )	{
				joint_p = _skeleton_ref_p->search_joint( joint_str );
				if( joint_p == NULL )	{
					fprintf( stderr, "MeCtEyeLid::get_joint ERR: joint '%s' NOT FOUND in skeleton\n", joint_str );
					free( joint_str );
					joint_str = NULL;
				}
			}
			else	{
				fprintf( stderr, "MeCtEyeLid::get_joint ERR: skeleton NOT FOUND\n" );
			}
		}
	}
	return( joint_p );
}

///////////////////////////////////////////////////////////////////////////////////

void MeCtEyeLid::context_updated( void ) {
}

void MeCtEyeLid::controller_map_updated( void ) {

}

void MeCtEyeLid::controller_start( void )	{

	if( _context->channels().size() > 0 )	{
		_skeleton_ref_p = _context->channels().skeleton();
	}
}

bool MeCtEyeLid::controller_evaluate( double t, MeFrameData& frame ) {

#if 0
	static int once = 1;
	if( once )	{
		once = 0;
		if( _context )	{
			SkSkeleton* skeleton_p = NULL;
			if( _context->channels().size() > 0 )	{
				skeleton_p = _context->channels().skeleton();
				if( skeleton_p )	{
					SkJoint * joint_p = skeleton_p->search_joint( "upper_eyelid_left" );
					if( joint_p )	{
						SrVec offset_v = joint_p->offset();
						printf( " off-XYZ = %.3f %.3f %.3f\n",
							offset_v.x, offset_v.y, offset_v.z
						);
					}
					else	{
						printf( "MeCtEyeLid::init NOTICE: SkJoint not available\n" );
					}
				}
				else	{
					printf( "MeCtEyeLid::init NOTICE: SkSkeleton not available\n" );
				}
			}
			else	{
				printf( "MeCtEyeLid::init NOTICE: _context->channels().size() is ZERO\n" );
			}
		}
		else	{
			printf( "MeCtEyeLid::init NOTICE: _context not available\n" );
		}
	}
#endif

	if( t < 0.0 )	{
		return( true );
	}

	float *fbuffer = &( frame.buffer()[0] );
	int n_chan = _channels.size();
// _channels.name( i )
// _channels.type( i )

	int UL_eye_quat_chan_index = _channels.search( SkJointName( "eyeball_left" ), SkChannel::Quat );
	int UR_eye_quat_chan_index = _channels.search( SkJointName( "eyeball_right" ), SkChannel::Quat );
	int UL_lid_posy_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::YPos );
	int UL_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::Quat );
	int UR_lid_posy_chan_index = _channels.search( SkJointName( "upper_eyelid_right" ), SkChannel::YPos );
	int UR_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_right" ), SkChannel::Quat );
	int i_map;
	
	i_map = _context->toBufferIndex( UL_eye_quat_chan_index );
	euler_t L_eye_e = quat_t(
		fbuffer[ i_map ],
		fbuffer[ i_map + 1 ],
		fbuffer[ i_map + 2 ],
		fbuffer[ i_map + 3 ]
	);

	i_map = _context->toBufferIndex( UR_eye_quat_chan_index );
	euler_t R_eye_e = quat_t(
		fbuffer[ i_map ],
		fbuffer[ i_map + 1 ],
		fbuffer[ i_map + 2 ],
		fbuffer[ i_map + 3 ]
	);

	int UL_lid_y_map = _context->toBufferIndex( UL_lid_quat_chan_index );
	float UL_lid_y = fbuffer[ UL_lid_y_map ];

	int UL_lid_q_map = _context->toBufferIndex( UL_lid_quat_chan_index );
	euler_t UL_lid_e = quat_t(
		fbuffer[ UL_lid_q_map ],
		fbuffer[ UL_lid_q_map + 1 ],
		fbuffer[ UL_lid_q_map + 2 ],
		fbuffer[ UL_lid_q_map + 3 ]
	);

	int UR_lid_y_map = _context->toBufferIndex( UR_lid_quat_chan_index );
	float UR_lid_y = fbuffer[ UR_lid_y_map ];
	
	int UR_lid_q_map = _context->toBufferIndex( UR_lid_quat_chan_index );
	euler_t UR_lid_e = quat_t(
		fbuffer[ UR_lid_q_map ],
		fbuffer[ UR_lid_q_map + 1 ],
		fbuffer[ UR_lid_q_map + 2 ],
		fbuffer[ UR_lid_q_map + 3 ]
	);

/*
	upper_eyelid_left
		offx = 3.15
		offy = 3.038
		offz = 11.75
	
	lookup
		posy = 0.372
		pitch = 0.0
	
	neutral
		posy = 0.0
		pitch = 0.0
	
	flat
		posy = -0.538
		pitch = 0.0

	lookdown
		posy = -0.788
		pitch = 0.0
	
	blink
		posy = -0.87
		pitch = 12.8
*/

#define EYEBALL_ROT_LIMIT_UP	-35.0
#define EYEBALL_ROT_LIMIT_DN	30.0

#define EYELID_Y_FLAT		-0.538

#define EYELID_Y_LIMIT_UP	0.372
#define EYELID_Y_LIMIT_DN	-0.788

// NOTE: 4/29/09 hacked pipeline precludes proper eyelid control, ignore blinking problem
	
	float L_eye_pitch = (float)( L_eye_e.p() );
	float UL_corrected_posy = 0.0;
	if( L_eye_pitch < 0.0 ) { // looking up
		float eye_norm = L_eye_pitch / EYEBALL_ROT_LIMIT_UP;
		UL_corrected_posy = eye_norm * EYELID_Y_LIMIT_UP;
	}
	else
	if( L_eye_pitch > 0.0 )	{ // looking down
		float eye_norm = L_eye_pitch / EYEBALL_ROT_LIMIT_DN;
		UL_corrected_posy = eye_norm * EYELID_Y_LIMIT_DN;
	}
	fbuffer[ UL_lid_y_map ] = UL_corrected_posy;
	fbuffer[ UR_lid_y_map ] = UL_corrected_posy;
	
#if 0
	static int once = 1;
	if( once )	{
		once = 0;
		printf( "pitch:%f in:%f out:%f\n", L_eye_pitch, UL_lid_y, UL_corrected_posy );
	}
#endif
		
#if 0
	float correct_pitch;
	
	correct_pitch = calculate_upper_correction( (float)UL_lid_e.p(), (float)L_eye_e.p() );
	UL_lid_e.p( correct_pitch );
	quat_t UL_lid_q = UL_lid_e;
	
	fbuffer[ UL_lid_q_map ] = (float)UL_lid_q.w();
	fbuffer[ UL_lid_q_map + 1 ] = (float)UL_lid_q.x();
	fbuffer[ UL_lid_q_map + 2 ] = (float)UL_lid_q.y();
	fbuffer[ UL_lid_q_map + 3 ] = (float)UL_lid_q.z();

	correct_pitch = calculate_upper_correction( (float)UR_lid_e.p(), (float)R_eye_e.p() );
	UR_lid_e.p( correct_pitch );
	quat_t UR_lid_q = UR_lid_e;

	fbuffer[ UR_lid_q_map ] = (float)UR_lid_q.w();
	fbuffer[ UR_lid_q_map + 1 ] = (float)UR_lid_q.x();
	fbuffer[ UR_lid_q_map + 2 ] = (float)UR_lid_q.y();
	fbuffer[ UR_lid_q_map + 3 ] = (float)UR_lid_q.z();
#endif
	
#if 0
	SkJoint* joint_p = source_ref_joint();
	joint_state_t state_in = capture_joint_state( joint_p );
	joint_state_t state_out = calc_channel_state( state_in );
	
	for( int i=0; i<n_chan; i++ ) {

		int index = frame.toBufferIndex( _toContextCh[ i ] );

		if( _channels.type( i ) == SkChannel::XPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.x();
		}
		else
		if( _channels.type( i ) == SkChannel::YPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.y();
		}
		else
		if( _channels.type( i ) == SkChannel::ZPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.z();
		}
		else
		if( _channels.type( i ) == SkChannel::Quat )	{
			fbuffer[ index + 0 ] = (float) state_out.local_rot.w();
			fbuffer[ index + 1 ] = (float) state_out.local_rot.x();
			fbuffer[ index + 2 ] = (float) state_out.local_rot.y();
			fbuffer[ index + 3 ] = (float) state_out.local_rot.z();
		}

		// Mark channel changed
		frame.channelUpdated( i );
	}
#endif

	return true;
}

SkChannelArray& MeCtEyeLid::controller_channels( void )	{
	return( _channels );
}

double MeCtEyeLid::controller_duration( void ) {
	return( -1.0 );
}

const char* MeCtEyeLid::controller_type( void )	{
	return( type_name );
}

void MeCtEyeLid::print_state( int tabCount ) {

	fprintf( stdout, "MeCtEyeLid" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

	fprintf( stdout, "\n" );
}

