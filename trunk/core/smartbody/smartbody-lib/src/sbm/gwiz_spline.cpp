/*
 *  gwiz_spline.cpp - part of SmartBody-lib
 *  Copyright (C) 2001-2008  Marcus Thiebaux, University of Southern California
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

#include "gwiz_math.h"

using namespace gwiz;

////////////////////////////////

bool gwiz::ctrl_key::bound_box( 
	float_t p_comp, 
	float_t v_comp, 
	float_t radius 
) const {
	return(
		( fabs( param - p_comp ) < radius )&&( fabs( value - v_comp ) < radius )
	);
}

////////////////////////////////

void gwiz::tempordinal_key::simple( 
	const ctrl_key& k0, 
	const ctrl_key& k1, 
	const ctrl_key& k2 
)	{
	gw_float_t m = ( k2.v() - k0.v() ) / ( k2.p() - k0.p() );
	set( k1.p(), k1.v(), m, m, 1.0, 1.0 );
}

void gwiz::tempordinal_key::cardinal(
	gw_float_t c,
	const ctrl_key& k0,
	const ctrl_key& k1,
	const ctrl_key& k2
)	{
	gw_float_t m = ( 1.0 - c ) * ( k2.v() - k0.v() ) / ( k2.p() - k0.p() );
	set( k1.p(), k1.v(), m, m, k1.p() - k0.p(), k2.p() - k1.p() );
}

void gwiz::tempordinal_key::cardinal_alt(
	gw_float_t c,
	const ctrl_key& k0,
	const ctrl_key& k1,
	const ctrl_key& k2
)	{
	gw_float_t m = 0.5 * (
		( k2.v() - k1.v() ) / ( k2.p() - k1.p() ) +
		( k1.v() - k0.v() ) / ( k1.p() - k0.p() )
	);
	set( k1.p(), k1.v(), m, m, k1.p() - k0.p(), k2.p() - k1.p() );
}

void gwiz::tempordinal_key::kochbartels(
	gw_float_t tension,
	gw_float_t bias,
	gw_float_t continuity,
	const ctrl_key& k0,
	const ctrl_key& k1,
	const ctrl_key& k2
)	{
	gw_float_t dv0 = k1.v() - k0.v();
	gw_float_t dv1 = k2.v() - k1.v();
	gw_float_t m0 = 
		0.5 * ( 1.0 - tension ) * ( 1.0 + bias ) * ( 1.0 + continuity ) * dv0 +
		0.5 * ( 1.0 - tension ) * ( 1.0 - bias ) * ( 1.0 - continuity ) * dv1;
	gw_float_t m1 = 
		0.5 * ( 1.0 - tension ) * ( 1.0 + bias ) * ( 1.0 - continuity ) * dv0 +
		0.5 * ( 1.0 - tension ) * ( 1.0 - bias ) * ( 1.0 + continuity ) * dv1;
	set( k1.p(), k1.v(), m0, m1, k1.p() - k0.p(), k2.p() - k1.p() );
}

////////////////////////////////

float_t gwiz::bezier(
	float_t s, // unit interpolant
	float_t f0,
	float_t f1,
	float_t f2,
	float_t f3
)	{
	// de Casteljau linear recursion
	float_t A = f0 + s * ( f1 - f0 );
	float_t B = f1 + s * ( f2 - f1 );
	float_t C = A + s * ( B - A );
	return( C + s*( ( B + s*( ( f2 + s*( f3 - f2 ) ) - B ) ) - C ) );
}

gw_float_t gwiz::hermite(
	gw_float_t s,
	gw_float_t v1,
	gw_float_t v2,
	gw_float_t m1,
	gw_float_t m2
) {
#if 1
		return(
			bezier(
				s,
				v1,
				v1 + m1 * 0.333333333,
				v2 - m2 * 0.333333333,
				v2
			)
		);
#elif 0
	// equivalents...
		register gw_float_t s_2 = s * s;
		register gw_float_t s_3 = s_2 * s;
		return(
			v1 * ( 2.0 * s_3 - 3.0 * s_2 + 1.0 ) +
			m1 * ( s_3 - 2.0 * s_2 + s ) +
			v2 * ( -2.0 * s_3 + 3.0 * s_2 ) +
			m2 * ( s_3 - s_2 );
		);
#else
		return(
			s * (
				s * (
					s * ( 2.0 * v1 - 2.0 * v2 + m1 + m2 ) +
					( -3.0 * v1 + 3.0 * v2 - 2.0 * m1 - m2 )
				) + m1
			) + v1
		);
#endif
}

gw_float_t gwiz::hermite(
	const gw_float_t t, 
	const cardinal_key& K1, 
	const cardinal_key& K2 
)	{
	gw_float_t s = ( t - K1.p() ) / ( K2.p() - K1.p() );
	return(
		hermite( s, K1.v(), K2.v(), K1.mr(), K2.ml() )
	);
}

gw_float_t gwiz::hermite(
	const gw_float_t t, 
	const tempordinal_key& K1, 
	const tempordinal_key& K2 
)	{

//	if( K1.t >= K2.t )
//	if( t < K1.t )
//	if( t >= K2.t )

	gw_float_t s = ( t - K1.p() ) / ( K2.p() - K1.p() ); // normalize parametric interpolant
	// FaceFX algorithm from http://www.facefx.com/documentation/2010/W99
	gw_float_t m1 = K1.mr() * K1.dr();
	gw_float_t m2 = K2.ml() * K2.dl();
	return(
		hermite( s, K1.v(), K2.v(), m1, m2 )
	);
}

////////////////////////////////

