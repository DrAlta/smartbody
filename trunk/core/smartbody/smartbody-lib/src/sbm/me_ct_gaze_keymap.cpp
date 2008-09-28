/*
 *  me_ct_gaze_keymap.cpp - part of SmartBody-lib
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

#include "me_ct_gaze.h"

/*
	LUMBAR: 	{ SPINE1, SPINE2 }
	THORAX: 	{ SPINE3 }
	CERVICAL:	{ SPINE4, SPINE5 }
	HEAD:		{ SKULL }
	EYES:		{ EYE_L, EYE_R }
	
	SPINE1: 	LUMBAR
	SPINE2: 	LUMBAR + THORAX
	SPINE3: 	THORAX
	SPINE4: 	CERVICAL
	SPINE5: 	CERVICAL + HEAD
	SKULL:		HEAD
	EYE_L:		EYES
	EYE_R:		EYES

	back:  lumbar:   { spine1, spine2 }
	chest: thorax:   { spine3 }
	neck:  cervical: { spine4, spine5 }
	head:  cranial:  { skullbase }
	eyes:  optical:  { eyeball_left, eyeball_right }
*/

///////////////////////////////////////////////////////////////////////////

void MeCtGaze::set_bias( int key, float p, float h, float r )	{
	
	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].bias_rot = euler_t( p, h, r );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_pitch( int key, float p )	{

	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].bias_rot.p( p );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_heading( int key, float h )	{

	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].bias_rot.h( h );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_roll( int key, float r )	{

	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].bias_rot.r( r );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_pitch( int key1, int key2, float p1, float p2 )	{
	int i;

	if( ! valid_key( key1 ) ) return;
	if( ! valid_key( key2 ) ) return;
	if( key1 == key2 )	{
		set_bias_pitch( key1, 0.5f * ( p1 + p2 ) );
		return;
	}
	if( key1 > key2 )	{
		int tmp = key1; key1 = key2; key2 = tmp;
		float tmp_p = p1; p1 = p2; p2 = tmp_p;
	}
	for( i = key1; i <= key2; i++ ) {
		float f = (float)( i - key1 )/(float)( key2 - key1 );
		float p = ( 1.0f - f ) * p1 + f * p2;
		set_bias_pitch( i, p );
	}
}

void MeCtGaze::set_bias_heading( int key1, int key2, float h1, float h2 ) {
	int i;

	if( ! valid_key( key1 ) ) return;
	if( ! valid_key( key2 ) ) return;
	if( key1 == key2 )	{
		set_bias_heading( key1, 0.5f * ( h1 + h2 ) );
		return;
	}
	if( key1 > key2 )	{
		int tmp = key1; key1 = key2; key2 = tmp;
		float tmp_h = h1; h1 = h2; h2 = tmp_h;
	}
	for( i = key1; i <= key2; i++ ) {
		float f = (float)( i - key1 )/(float)( key2 - key1 );
		float h = ( 1.0f - f ) * h1 + f * h2;
		set_bias_heading( i, h );
	}
}

void MeCtGaze::set_bias_roll( int key1, int key2, float r1, float r2 )	{
	int i;

	if( ! valid_key( key1 ) ) return;
	if( ! valid_key( key2 ) ) return;
	if( key1 == key2 )	{
		set_bias_roll( key1, 0.5f * ( r1 + r2 ) );
		return;
	}
	if( key1 > key2 )	{
		int tmp = key1; key1 = key2; key2 = tmp;
		float tmp_r = r1; r1 = r2; r2 = tmp_r;
	}
	for( i = key1; i <= key2; i++ ) {
		float f = (float)( i - key1 )/(float)( key2 - key1 );
		float r = ( 1.0f - f ) * r1 + f * r2;
		set_bias_roll( i, r );
	}
}

void MeCtGaze::set_bias_pitch( float e_p, float h_p, float c_p, float t_p, float l_p ) {

	joint_key_arr[ GAZE_KEY_EYES ].bias_rot.p( e_p );
	joint_key_arr[ GAZE_KEY_HEAD ].bias_rot.p( h_p );
	joint_key_arr[ GAZE_KEY_CERVICAL ].bias_rot.p( c_p );
	joint_key_arr[ GAZE_KEY_THORAX ].bias_rot.p( t_p );
	joint_key_arr[ GAZE_KEY_LUMBAR ].bias_rot.p( l_p );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_heading( float e_h, float h_h, float c_h, float t_h, float l_h )	{

	joint_key_arr[ GAZE_KEY_EYES ].bias_rot.h( e_h );
	joint_key_arr[ GAZE_KEY_HEAD ].bias_rot.h( h_h );
	joint_key_arr[ GAZE_KEY_CERVICAL ].bias_rot.h( c_h );
	joint_key_arr[ GAZE_KEY_THORAX ].bias_rot.h( t_h );
	joint_key_arr[ GAZE_KEY_LUMBAR ].bias_rot.h( l_h );
	key_bias_dirty = 1;
}

void MeCtGaze::set_bias_roll( float e_r, float h_r, float c_r, float t_r, float l_r ) {

	joint_key_arr[ GAZE_KEY_EYES ].bias_rot.r( e_r );
	joint_key_arr[ GAZE_KEY_HEAD ].bias_rot.r( h_r );
	joint_key_arr[ GAZE_KEY_CERVICAL ].bias_rot.r( c_r );
	joint_key_arr[ GAZE_KEY_THORAX ].bias_rot.r( t_r );
	joint_key_arr[ GAZE_KEY_LUMBAR ].bias_rot.r( l_r );
	key_bias_dirty = 1;
}

void MeCtGaze::apply_bias_keys( void ) {

	if( key_bias_dirty )   {

		joint_arr[ GAZE_JOINT_SPINE1 ].forward_rot = 
			joint_key_arr[ GAZE_KEY_LUMBAR ].bias_rot;

		joint_arr[ GAZE_JOINT_SPINE2 ].forward_rot = 
			joint_key_arr[ GAZE_KEY_LUMBAR ].bias_rot.lerp( 0.5, joint_key_arr[ GAZE_KEY_THORAX ].bias_rot );

		joint_arr[ GAZE_JOINT_SPINE3 ].forward_rot = 
			joint_key_arr[ GAZE_KEY_THORAX ].bias_rot;

		joint_arr[ GAZE_JOINT_SPINE4 ].forward_rot = 
			joint_key_arr[ GAZE_KEY_CERVICAL ].bias_rot;

		joint_arr[ GAZE_JOINT_SPINE5 ].forward_rot = 
			joint_key_arr[ GAZE_KEY_CERVICAL ].bias_rot.lerp( 0.5, joint_key_arr[ GAZE_KEY_HEAD ].bias_rot );

		joint_arr[ GAZE_JOINT_SKULL ].forward_rot = 
			joint_key_arr[ GAZE_KEY_HEAD ].bias_rot;

		joint_arr[ GAZE_JOINT_EYE_L ].forward_rot = 
			joint_arr[ GAZE_JOINT_EYE_R ].forward_rot = 
				joint_key_arr[ GAZE_KEY_EYES ].bias_rot;

		key_bias_dirty = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////

void MeCtGaze::set_limit( int key, float p, float h, float r )	{

	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].limit_rot = euler_t( p, h, r );
	key_limit_dirty = 1;
}

void MeCtGaze::apply_limit_keys( void )	{
	
	if( key_limit_dirty )	{

		joint_arr[ GAZE_JOINT_SPINE1 ].limit_rot = 
			joint_key_arr[ GAZE_KEY_LUMBAR ].limit_rot * 0.5f;

		joint_arr[ GAZE_JOINT_SPINE2 ].limit_rot = 
			joint_key_arr[ GAZE_KEY_LUMBAR ].limit_rot * 0.5f;

		joint_arr[ GAZE_JOINT_SPINE3 ].limit_rot = 
			joint_key_arr[ GAZE_KEY_THORAX ].limit_rot;

		joint_arr[ GAZE_JOINT_SPINE4 ].limit_rot = 
			joint_key_arr[ GAZE_KEY_CERVICAL ].limit_rot * 0.5f;

		joint_arr[ GAZE_JOINT_SPINE5 ].limit_rot = 
			joint_key_arr[ GAZE_KEY_CERVICAL ].limit_rot * 0.5f;

		joint_arr[ GAZE_JOINT_SKULL ].limit_rot = 
			joint_key_arr[ GAZE_KEY_HEAD ].limit_rot;

		joint_arr[ GAZE_JOINT_EYE_L ].limit_rot = 
			joint_arr[ GAZE_JOINT_EYE_R ].limit_rot = 
				joint_key_arr[ GAZE_KEY_EYES ].limit_rot;

		key_limit_dirty = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////

void MeCtGaze::set_blend( int key, float w )	{

	if( ! valid_key( key ) ) return;
	joint_key_arr[ key ].blend_weight = w;
	key_blend_dirty = 1;
}

void MeCtGaze::set_blend( float l_w, float t_w, float c_w, float h_w, float e_w ) {

	joint_key_arr[ GAZE_KEY_LUMBAR ].blend_weight = l_w;
	joint_key_arr[ GAZE_KEY_THORAX ].blend_weight = t_w;
	joint_key_arr[ GAZE_KEY_CERVICAL ].blend_weight = c_w;
	joint_key_arr[ GAZE_KEY_HEAD ].blend_weight = h_w;
	joint_key_arr[ GAZE_KEY_EYES ].blend_weight = e_w;
	key_blend_dirty = 1;
}

void MeCtGaze::set_blend( int key1, int key2, float w1, float w2 )	{
	int i;
	
	if( ! valid_key( key1 ) ) return;
	if( ! valid_key( key2 ) ) return;
	if( key1 == key2 )	{
		set_blend( key1, 0.5f * ( w1 + w2 ) );
		return;
	}
	if( key1 > key2 )	{
		int tmp = key1; key1 = key2; key2 = tmp;
		float tmp_w = w1; w1 = w2; w2 = tmp_w;
	}
	for( i = key1; i <= key2; i++ ) {
		float f = (float)( i - key1 )/(float)( key2 - key1 );
		float w = ( 1.0f - f ) * w1 + f * w2;
		set_blend( i, w );
	}
}

void MeCtGaze::apply_blend_keys( void )	{
	
	if( key_blend_dirty )	{

		joint_arr[ GAZE_JOINT_SPINE1 ].blend_weight = 
			joint_key_arr[ GAZE_KEY_LUMBAR ].blend_weight;

		joint_arr[ GAZE_JOINT_SPINE2 ].blend_weight = 
			0.5f * ( joint_key_arr[ GAZE_KEY_LUMBAR ].blend_weight + joint_key_arr[ GAZE_KEY_THORAX ].blend_weight );

		joint_arr[ GAZE_JOINT_SPINE3 ].blend_weight = 
			joint_key_arr[ GAZE_KEY_THORAX ].blend_weight;

		joint_arr[ GAZE_JOINT_SPINE4 ].blend_weight = 
			joint_key_arr[ GAZE_KEY_CERVICAL ].blend_weight;

		joint_arr[ GAZE_JOINT_SPINE5 ].blend_weight = 
			0.5f * ( joint_key_arr[ GAZE_KEY_CERVICAL ].blend_weight + joint_key_arr[ GAZE_KEY_HEAD ].blend_weight );

		joint_arr[ GAZE_JOINT_SKULL ].blend_weight = 
			joint_key_arr[ GAZE_KEY_HEAD ].blend_weight;

		joint_arr[ GAZE_JOINT_EYE_L ].blend_weight = 
			joint_arr[ GAZE_JOINT_EYE_R ].blend_weight = 
				joint_key_arr[ GAZE_KEY_EYES ].blend_weight;

		key_blend_dirty = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
