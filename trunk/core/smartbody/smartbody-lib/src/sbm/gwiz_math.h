/*
 *  gwiz_math.h - part of SmartBody-lib
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

#ifndef GWIZ_MATH_H
#define GWIZ_MATH_H

// "Gee Whiz!" 
// Vector geometry wizard...

////////////////////////////////
#include <math.h>
#include <stdio.h>

#define GWIZ_version	"Version 5.0 : Jan 29, 2011"

#ifdef GWIZ_32BIT
typedef float gw_float_t;
#else
typedef double gw_float_t;
#endif

class vector_t;
class quat_t;
class euler_t;
class matrix_t;

typedef vector_t	vector3_t;
typedef quat_t		quaternion_t;
typedef euler_t		eulerPHR_t;
typedef matrix_t	matrix4x4_t;

#ifndef M_PI
#define M_PI	GWIZ::pi()
#endif

#ifndef RAD 
#define RAD 	GWIZ::rad
#endif
#ifndef DEG
#define DEG 	GWIZ::deg
#endif

////////////////////////////////

class GWIZ {
	
	public:
	
		enum math_enum_set    {
			COMP_UNKNOWN, 
			COMP_M_TR, 
			COMP_M_TRS, 
			COMP_M_TRSH, 
			COMP_M_PTRSH
		};

		static gw_float_t pi( void ) { return( 3.14159265358979323846 ); }

		static gw_float_t rad( gw_float_t deg ) { return( deg * 0.017453292519943295 ); }
		static gw_float_t deg( gw_float_t rad ) { return( rad * 57.295779513082323 ); }

		static gw_float_t safe_arc_threshold( void ) { return( 0.999999999 ); }
		
		static gw_float_t safe_asin( gw_float_t s );
		static gw_float_t safe_acos( gw_float_t c );
		static gw_float_t safe_atan( gw_float_t t );
		
		static gw_float_t epsilon4( void )	{ return( 0.00001 ); }
		static gw_float_t epsilon5( void )	{ return( 0.000001 ); } /* one millionth */
		static gw_float_t epsilon6( void )	{ return( 0.0000001 ); }
		static gw_float_t epsilon7( void )	{ return( 0.00000001 ); }
		static gw_float_t epsilon8( void )	{ return( 0.000000001 ); } /* one billionth */
		static gw_float_t epsilon9( void )	{ return( 0.0000000001 ); }
		static gw_float_t epsilon10( void ) { return( 0.00000000001 ); }
		static gw_float_t epsilon11( void ) { return( 0.000000000001 ); } /* one trillionth */
};

////////////////////////////////

class vector_t {

    public:
#if 1
		void print( void ) const { printf( " vector_t:\n  %f %f %f\n", X, Y, Z ); }
#else
		void print( void ) const { printf( " vector_t:\n  %.12f %.12f %.12f\n", X, Y, Z ); }
#endif

	// CONSTRUCT
        inline vector_t( void )
            { X = 0.0; Y = 0.0; Z = 0.0; }
        inline vector_t( gw_float_t x_in, gw_float_t y_in, gw_float_t z_in )
            { X = x_in; Y = y_in; Z = z_in; }
        inline vector_t( const vector_t & v_in )
            { X = v_in.x(); Y = v_in.y(); Z = v_in.z(); }

	// WRITE
		inline void x( gw_float_t x_in ) { X = x_in; }
		inline void y( gw_float_t y_in ) { Y = y_in; }
		inline void z( gw_float_t z_in ) { Z = z_in; }

	// READ
		inline gw_float_t x( void ) const { return( X ); }
		inline gw_float_t y( void ) const { return( Y ); }
		inline gw_float_t z( void ) const { return( Z ); }

	// MISC
        inline gw_float_t sqlen( void ) const 
			{ return( X*X + Y*Y + Z*Z ); }
        inline gw_float_t length( void ) const 
			{ return( sqrt( sqlen() ) ); }
		inline gw_float_t dot( const vector_t& v ) const 
			{ return( X*v.X + Y*v.Y + Z*v.Z ); }

		inline vector_t cross( const vector_t& v ) const 
			{ return( vector_t( Y*v.Z - Z*v.Y, Z*v.X - X*v.Z, X*v.Y - Y*v.X ) ); }
		inline vector_t& cross( const vector_t& v0, const vector_t& v1 ) 
			{ return( (*this) = v0.cross( v1 ) ); }

		inline gw_float_t box( const vector_t& v1, const vector_t& v2 ) const 
			{ return( dot( v1.cross( v2 ) ) ); }
		inline gw_float_t box( const vector_t& v1, const vector_t& v2, const vector_t& v3 ) const 
			{ return( ( v1 - *this ).dot( ( v2 - *this ).cross( v3 - *this ) ) ); }

        vector_t normal( void ) const;
        vector_t& normalize( void );

		inline vector_t lerp( gw_float_t s, const vector_t& v ) const 
#if 0
			{ return( (*this) * ( 1.0 - s ) + v * s ); }
#else
			{ return( (*this) + ( v - (*this) ) * s ); }
#endif
		inline vector_t& lerp( gw_float_t s, const vector_t& v0, const vector_t& v1 ) 
			{ return( (*this) = v0.lerp( s, v1 ) ); }
		
	// OPERATE
        inline vector_t operator - ( void ) const 
            { return( vector_t( -x(), -y(), -z() ) ); }

        inline vector_t operator * ( gw_float_t s ) const 
			{ return( vector_t( X*s, Y*s, Z*s ) ); }
        inline vector_t operator / ( gw_float_t d ) const 
			{ gw_float_t inv = 1.0 / d; return( (*this) * inv ); }
        inline vector_t& operator *= ( gw_float_t s ) 
			{ return( (*this) = (*this) * s ); }
        inline vector_t& operator /= ( gw_float_t d ) 
			{ return( (*this) = (*this) / d ); }

        inline vector_t operator + ( const vector_t& v ) const 
			{ return( vector_t( X + v.X, Y + v.Y, Z + v.Z ) ); }
        inline vector_t operator - ( const vector_t& v ) const 
			{ return( vector_t( X - v.X, Y - v.Y, Z - v.Z ) ); }
        inline vector_t operator * ( const vector_t& v ) const 
			{ return( vector_t( X * v.X, Y * v.Y, Z * v.Z ) ); }
        inline vector_t operator / ( const vector_t& v ) const 
			{ return( vector_t( X / v.X, Y / v.Y, Z / v.Z ) ); }

        inline vector_t& operator += ( const vector_t& v ) 
			{ return( (*this) = (*this) + v ); }
        inline vector_t& operator -= ( const vector_t& v ) 
			{ return( (*this) = (*this) - v ); }
        inline vector_t& operator *= ( const vector_t& v ) 
			{ return( (*this) = (*this) * v ); }
        inline vector_t& operator /= ( const vector_t& v ) 
			{ return( (*this) = (*this) / v ); }

    private:
        gw_float_t X, Y, Z;
};

class vector4_t: public vector_t {

    public:
#if 1
		void print( void ) const { printf( " vector4_t:\n  %f %f %f %f\n", x(), y(), z(), W ); }
#else
		void print( void ) const { printf( " vector4_t:\n  %.12f %.12f %.12f %.12f\n", x(), y(), z(), W ); }
#endif

	// CONSTRUCT
        inline vector4_t( void ): vector_t()
            { W = 0.0; }
        inline vector4_t( gw_float_t x_in, gw_float_t y_in, gw_float_t z_in, gw_float_t w_in )
			: vector_t( x_in, y_in, z_in )
            { W = w_in; }
        inline vector4_t( const vector_t & v_in, gw_float_t w_in )
			: vector_t( v_in )
            { W = w_in; }

	// WRITE
		inline void w( gw_float_t w_in ) { W = w_in; }

	// READ
		inline gw_float_t w( void ) const { return( W ); }

	// MISC
        inline gw_float_t sqlen( void ) const 
			{ return( vector_t::sqlen() + W*W ); }
        inline gw_float_t length( void ) const 
			{ return( sqrt( sqlen() ) ); }

	// OPERATE: not implemented

    private:
        gw_float_t W;
};

////////////////////////////////

class quat_t {

    public:
#if 1
		void print( void ) const { printf( " quat_t:\n  %f %f %f %f\n", W, X, Y, Z ); }
#else
		void print( void ) const { printf( " quat_t:\n  %.12f %.12f %.12f %.12f\n", W, X, Y, Z ); }
#endif

	// CONSTRUCT
        inline quat_t( void )
            { W = 1.0; X = 0.0; Y = 0.0; Z = 0.0; }
        inline quat_t( gw_float_t w_in, gw_float_t x_in, gw_float_t y_in, gw_float_t z_in )
            { set( w_in, x_in, y_in, z_in ); }
		quat_t( const vector_t& axis_angle );
		
		quat_t( gw_float_t angle, const vector_t& v, int use_radians = 0 );
		quat_t( gw_float_t swing_x, gw_float_t swing_y, gw_float_t twist, int use_radians = 0 );
        quat_t( const vector_t& z_axis, const vector_t& y_axis_approx );

	// CONVERT CONSTRUCT
		quat_t( const euler_t& e );

	// WRITE
		inline void set( gw_float_t w_in, gw_float_t x_in, gw_float_t y_in, gw_float_t z_in ) 
			{ W = w_in; X = x_in; Y = y_in; Z = z_in; normalize(); }

	// READ
        inline gw_float_t w( void ) const { return( W ); }
        inline gw_float_t x( void ) const { return( X ); }
        inline gw_float_t y( void ) const { return( Y ); }
        inline gw_float_t z( void ) const { return( Z ); }

#if 0
		inline bool non_identity( void ) const 
			{ return( W < 0.999999999 ); }
#endif
		inline vector_t axisangle( void ) const // Axis-Angle: Same as Exponential Map
			{ return( axis() * radians() ); }
		inline gw_float_t radians( void ) const
			{ return( 2.0 * GWIZ::safe_acos( W ) ); }
		inline gw_float_t degrees( void ) const 
			{ return( DEG( radians() ) ); }
		inline vector_t axis( void ) const 
			{ return( vector_t( X, Y, Z ).normal() ); }
		vector_t swingtwist( int use_radians = 0 ) const;
		
	// MISC
		inline gw_float_t dot( const quat_t& q ) const 
			{ return( W*q.W + X*q.X + Y*q.Y + Z*q.Z ); }

		inline quat_t reflection(void) const 
			{ return( quat_t( -W, X, Y, Z ) ); } // reverse
		inline quat_t conjugate(void) const 
			{ return( quat_t( W, -X, -Y, -Z ) ); } // inverse
		inline quat_t complement(void) const 
			{ return( quat_t( -W, -X, -Y, -Z ) ); }

		inline quat_t shortest( void ) const 
			{ if( W < 0.0 ) return( complement() ); return( *this ); }
		inline quat_t shortest( const quat_t& q_ref ) const 
			{ if( dot( q_ref ) < 0.0 ) return( complement() ); return( *this ); }

		quat_t& normalize( void );

		quat_t lerp( gw_float_t s, const quat_t& q ) const;
		inline quat_t& lerp( gw_float_t s, const quat_t& q0, const quat_t& q1 ) 
			{ return( (*this) = q0.lerp( s, q1 ) ); }

	// OPERATE
		inline quat_t operator - ( void ) const 
			{ return( conjugate() ); }
		inline quat_t operator ~ ( void ) const 
			{ return( complement() ); }

		inline quat_t operator * ( gw_float_t s ) const 
			{ quat_t tmp; return( tmp.lerp( s, *this ) ); }
		inline quat_t operator / ( gw_float_t d ) const 
			{ gw_float_t inv = 1.0 / d; return( (*this) * inv ); }

		inline quat_t& operator *= ( gw_float_t s ) 
			{ return( (*this) = (*this) * s ); }
		inline quat_t& operator /= ( gw_float_t d ) 
			{ return( (*this) = (*this) / d ); }

		inline quat_t& operator *= ( quat_t q ) 
			{ return( (*this) = (*this) * q ); }

	// PIPELINE
		/*
			http://www.cs.hmc.edu/courses/2006/spring/cs157/lectures/orientation2.ppt#307,17,Shorthand
				q=<s,v>
				q'=<s',v'>
				.: dot product
				x: cross product
				q*q' = <ss'-v.v', sv' + s'v + vxv'>
		*/

		inline quat_t product( const quat_t& q ) const { 
			// http://en.wikipedia.org/wiki/Quaternion
			/*
			 P: { a, b, c, d }
			 Q: { t, x, y, z }
			 P*Q == {( at - bx  - cy - dz ),           ( bt + ax + cz - dy ),            ( ct + ay + dx - bz ),            ( dt + az + by - cx )}
			 P*Q == {( W*q.W - X*q.X - Y*q.Y - Z*q.Z ),( X*q.W + W*q.X + Y*q.Z - Z*q.Y ),( Y*q.W + W*q.Y + Z*q.X - X*q.Z ),( Z*q.W + W*q.Z + X*q.Y - Y*q.X )}
			quat_t(
				W*q.W - X*q.X - Y*q.Y - Z*q.Z,
				X*q.W + W*q.X + Y*q.Z - Z*q.Y,
				Y*q.W + W*q.Y + Z*q.X - X*q.Z,
				Z*q.W + W*q.Z + X*q.Y - Y*q.X
			)
			*/
			// return( this * q );
			return(
				quat_t(
					W*q.W - X*q.X - Y*q.Y - Z*q.Z, 
					X*q.W + W*q.X - Z*q.Y + Y*q.Z, 
					Y*q.W + Z*q.X + W*q.Y - X*q.Z, 
					Z*q.W - Y*q.X + X*q.Y + W*q.Z
				)
			);
		}

		inline quat_t product_left( const quat_t& q ) const { 
			// Shoemake
			// http://answers.google.com/answers/threadview?id=596035
			// http://www.gamedev.net/reference/articles/article1997.asp
			// return( q * this );
			return(
				quat_t(
					W*q.W - X*q.X - Y*q.Y - Z*q.Z, 
					X*q.W + W*q.X + Z*q.Y - Y*q.Z, 
					Y*q.W - Z*q.X + W*q.Y + X*q.Z, 
					Z*q.W + Y*q.X - X*q.Y + W*q.Z
				)
				/*
				rearranged:
				quat_t(
					+ W*q.W - X*q.X - Y*q.Y - Z*q.Z, 
					+ W*q.X + X*q.W - Y*q.Z + Z*q.Y , 
					+ W*q.Y + X*q.Z + Y*q.W - Z*q.X , 
					+ W*q.Z - X*q.Y + Y*q.X + Z*q.W 
				)
				*/
			);
		}

		inline quat_t operator * ( const quat_t& q ) const { 
			return( product( q ) );
		}
		matrix_t operator * ( const matrix_t& R ) const;

	// TRANSFORM
		vector_t operator * ( const vector_t& v ) const;
		
    private:
        gw_float_t W, X, Y, Z;
};

class euler_t {

	// OGL standard:
	//   axes: { X, Y, Z } == { right, up, back } == { right-hand }
	// 6DOF tracking hardware standard:
	//   packing: { pitch, heading, roll } == { X, Y, Z }
	//   concatenation: [ Heading * Pitch * Roll ] == [ Y * X * Z ]
	
    public:
#if 1
		void print( void ) const { printf( " euler_t:\n  %f %f %f\n", X, Y, Z ); }
#else
		void print( void ) const { printf( " euler_t:\n  %.12f %.12f %.12f\n", X, Y, Z ); }
#endif

	// CONSTRUCT
        inline euler_t( void )
            { X = 0.0; Y = 0.0; Z = 0.0; }
        inline euler_t( gw_float_t x_in, gw_float_t y_in, gw_float_t z_in )
            { X = x_in; Y = y_in; Z = z_in; }
        euler_t( const vector_t& z_axis, gw_float_t roll_deg ); // NOTE: flipped lookat
        euler_t( const vector_t& z_axis, const vector_t& y_axis_approx ); // NOTE: flipped lookat

	// CONVERT CONSTRUCT
		euler_t( const quat_t& q );

	// WRITE
		inline void x( gw_float_t x_in ) { X = x_in; }
		inline void y( gw_float_t y_in ) { Y = y_in; }
		inline void z( gw_float_t z_in ) { Z = z_in; }

		inline void p( gw_float_t p_in ) { X = p_in; }
		inline void h( gw_float_t h_in ) { Y = h_in; }
		inline void r( gw_float_t r_in ) { Z = r_in; }
		
		inline euler_t& lookat( const vector_t& dir, gw_float_t roll_deg )
			{ return( (*this) = euler_t( -dir, roll_deg ) ); } // NOTE: flipped z-axis
		inline euler_t& lookat( const vector_t& dir, const vector_t& up_approx )
			{ return( (*this) = euler_t( -dir, up_approx ) ); } // NOTE: flipped z-axis

	// READ
		inline gw_float_t x( void ) const { return( X ); }
		inline gw_float_t y( void ) const { return( Y ); }
		inline gw_float_t z( void ) const { return( Z ); }

		inline gw_float_t p( void ) const { return( X ); }
		inline gw_float_t h( void ) const { return( Y ); }
		inline gw_float_t r( void ) const { return( Z ); }

	// MISC
		inline euler_t lerp( gw_float_t s, const euler_t& e ) const 
			{ return( quat_t(*this).lerp( s, e ) ); }
		inline euler_t& lerp( gw_float_t s, const euler_t& e0, const euler_t& e1 ) 
			{ return( (*this) = e0.lerp( s, e1 ) ); }

	// OPERATE
		euler_t operator - ( void ) const;
		
		inline euler_t operator * ( gw_float_t s ) const 
			{ return( quat_t(*this) * s ); }
		inline euler_t operator / ( gw_float_t d ) const 
			{ gw_float_t inv = 1.0 / d; return( (*this) * inv ); }

		inline euler_t& operator *= ( gw_float_t s ) 
			{ return( (*this) = (*this) * s ); }
		inline euler_t& operator /= ( gw_float_t d ) 
			{ return( (*this) = (*this) / d ); }

		inline euler_t& operator *= ( euler_t e ) 
			{ return( (*this) = (*this) * e ); }

	// PIPELINE
		matrix_t operator * ( const matrix_t& R ) const;
		inline euler_t operator * ( const euler_t& r ) const
			{ return( quat_t(*this) * quat_t( r ) ); }

	// TRANSFORM
		vector_t operator * ( const vector_t& v ) const;
		
    private:
        gw_float_t X, Y, Z;
};

////////////////////////////////

class matrix_t {
	
	// OGL standard:
	//   col major packing: [ col-0 ][ col-1 ][ col-2 ][ col-3 ]
	//   access: M[ col ][ row ] == M.get( col, row )
	
    public:
		void print( char *label = 0x0 ) const {
			if( label )
				printf( " matrix_t: %s\n", label );
			else
				printf( " matrix_t:\n" );
			for( int i=0;i<4;i++ )
				printf( "  %f %f %f %f\n", M[0][i], M[1][i], M[2][i], M[3][i] );
		}

	// CONSTRUCT
        inline matrix_t( void ) { stack = 0x0; }
        inline matrix_t( int init ) { stack = 0x0; if( init ) identity(); }
        inline matrix_t( const gw_float_t M_array[ 4 ][ 4 ] ) { stack = 0x0; set( M_array ); }
        inline matrix_t( const vector_t& x, const vector_t& y, const vector_t& z )
			{ stack = 0x0; lineator( x, y, z ); }
	
	// COPY CONSTRUCT
		inline matrix_t( const matrix_t& M_set ) { stack = 0x0; set( M_set.M ); }

	// COPY ASSIGN
		inline matrix_t& operator = ( const matrix_t& M_set ) { 
			if( this != &M_set ) return( set( M_set.M ) ); 
			return( *this );
		}

	// CONVERT CONSTRUCT
		matrix_t( const quat_t& q );
		matrix_t( const euler_t& e );
	
	// DESTRUCT
        inline virtual ~matrix_t( void ) { while( stack ) pop(); }

	// WRITE
		inline matrix_t& set( int c, int r, gw_float_t f ) 
			{ M[ c ][ r ] = f; return( *this ); }

		inline matrix_t& col( int i, gw_float_t a, gw_float_t b, gw_float_t c, gw_float_t d ) 
			{ M[i][0] = a; M[i][1] = b; M[i][2] = c; M[i][3] = d; return( *this ); }
		inline matrix_t& row( int i, gw_float_t a, gw_float_t b, gw_float_t c, gw_float_t d ) 
			{ M[0][i] = a; M[1][i] = b; M[2][i] = c; M[3][i] = d; return( *this ); }

		inline matrix_t& col( int i, const vector_t& v, gw_float_t f ) 
			{ M[i][0] = v.x(); M[i][1] = v.y(); M[i][2] = v.z(); M[i][3] = f; return( *this ); }
		inline matrix_t& row( int i, const vector_t& v, gw_float_t f ) 
			{ M[0][i] = v.x(); M[1][i] = v.y(); M[2][i] = v.z(); M[3][i] = f; return( *this ); }

		inline matrix_t& col( int i, const vector4_t& v ) 
			{ M[i][0] = v.x(); M[i][1] = v.y(); M[i][2] = v.z(); M[i][3] = v.w(); return( *this ); }
		inline matrix_t& row( int i, const vector4_t& v ) 
			{ M[0][i] = v.x(); M[1][i] = v.y(); M[2][i] = v.z(); M[3][i] = v.w(); return( *this ); }

		inline matrix_t& set( const gw_float_t M_array[ 4 ][ 4 ] ) { 
			for( int i=0;i<4;i++ )	{
				M[i][0] = M_array[i][0]; 
				M[i][1] = M_array[i][1]; 
				M[i][2] = M_array[i][2]; 
				M[i][3] = M_array[i][3];
			}
			return( *this );
		}
		inline matrix_t& set_col_major( const gw_float_t M_array[ 16 ] ) { 
			for( int i=0;i<4;i++ )	{
				M[i][0] = M_array[ 4 * i ];
				M[i][1] = M_array[ 4 * i + 1 ];
				M[i][2] = M_array[ 4 * i + 2 ];
				M[i][3] = M_array[ 4 * i + 3 ];
			}
			return( *this ); 
		}
		inline matrix_t& set_row_major( const gw_float_t M_array[ 16 ] ) { 
			for( int i=0;i<4;i++ )	{
				M[0][i] = M_array[ 4 * i ];
				M[1][i] = M_array[ 4 * i + 1 ];
				M[2][i] = M_array[ 4 * i + 2 ];
				M[3][i] = M_array[ 4 * i + 3 ];
			}
			return( *this ); 
		}

	// READ
		inline gw_float_t get( int c, int r ) const
			{ return( M[ c ][ r ] ); }
		inline vector4_t col( int i ) const 
			{ return( vector4_t( M[i][0], M[i][1], M[i][2], M[i][3] ) ); }
		inline vector4_t row( int i ) const 
			{ return( vector4_t( M[0][i], M[1][i], M[2][i], M[3][i] ) ); }
		inline matrix_t get( gw_float_t M_array[ 4 ][ 4 ] ) const { 
			for( int i=0;i<4;i++ )	{
				M_array[i][0] = M[i][0]; 
				M_array[i][1] = M[i][1]; 
				M_array[i][2] = M[i][2]; 
				M_array[i][3] = M[i][3]; 
			}
			return( *this );
		}
		inline matrix_t get_col_major( gw_float_t M_array[ 16 ] ) const { 
			for( int i=0;i<4;i++ )	{
				M_array[ 4 * i ] = M[i][0];
				M_array[ 4 * i + 1 ] = M[i][1];
				M_array[ 4 * i + 2 ] = M[i][2];
				M_array[ 4 * i + 3 ] = M[i][3];
			}
			return( *this ); 
		}
		inline matrix_t get_row_major( gw_float_t M_array[ 16 ] ) const { 
			for( int i=0;i<4;i++ )	{
				M_array[ 4 * i ] = M[0][i];
				M_array[ 4 * i + 1 ] = M[1][i];
				M_array[ 4 * i + 2 ] = M[2][i];
				M_array[ 4 * i + 3 ] = M[3][i];
			}
			return( *this ); 
		}

	// MISC
        inline matrix_t& identity( void ) { 
			for(int i=0;i<4;i++) 
				for(int j=0;j<4;j++) 
					M[ i ][ j ] = ( ( i==j ) ? 1.0 : 0.0 ); 
			return( *this );
		}
        inline matrix_t& id( void ) { return( identity() ); }
        inline matrix_t transposition( void ) const { 
			matrix_t tmp;
			for(int i=0;i<4;i++) 
				for(int j=0;j<4;j++) 
					tmp.M[ i ][ j ] = M[ j ][ i ];
			return( tmp );
		}
        inline matrix_t& transpose( void ) { 
			gw_float_t tmp;
			tmp = M[0][1]; M[0][1] = M[1][0]; M[1][0] = tmp;
			tmp = M[0][2]; M[0][2] = M[2][0]; M[2][0] = tmp;
			tmp = M[0][3]; M[0][3] = M[3][0]; M[3][0] = tmp;
			tmp = M[1][2]; M[1][2] = M[2][1]; M[2][1] = tmp;
			tmp = M[1][3]; M[1][3] = M[3][1]; M[3][1] = tmp;
			tmp = M[2][3]; M[2][3] = M[3][2]; M[3][2] = tmp;
			return( *this );
		}
        matrix_t inverse( void ) const;
        matrix_t& invert( void );

	// ASSIGN TRANSFORMS
		matrix_t& frustum( gw_float_t l, gw_float_t r, gw_float_t b, gw_float_t t, gw_float_t n, gw_float_t f );
		matrix_t& frustum( const vector_t& L_B_N, const vector_t& R_T_F );
		matrix_t& frustum( const vector_t& B_L_dir, const vector_t& R_T_dir, gw_float_t near, gw_float_t far );
		matrix_t& frustum( gw_float_t fovy, gw_float_t aspect, gw_float_t near, gw_float_t far );

		matrix_t& translator( const vector_t& t );
		matrix_t& rotator( const quat_t& q );
		matrix_t& rotator( const euler_t& e );
		matrix_t& shearer( const vector_t& s );
		matrix_t& scaler( const vector_t& s );
		matrix_t& lineator( const vector_t& x, const vector_t& y, const vector_t& z );

		inline matrix_t& translator( gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( translator( vector_t( x, y, z ) ) ); }
		inline matrix_t& rotator( gw_float_t w, gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( rotator( quat_t( w, x, y, z ) ) ); }
		inline matrix_t& rotator( gw_float_t angle, const vector_t& v, int use_radians = 0 )
			{ return( rotator( quat_t( angle, v, use_radians ) ) ); }
		inline matrix_t& rotator( gw_float_t p, gw_float_t h, gw_float_t r )
			{ return( rotator( euler_t( p, h, r ) ) ); }
		inline matrix_t& shearer( gw_float_t xy, gw_float_t xz, gw_float_t yz )
			{ return( shearer( vector_t( xy, xz, yz ) ) ); }
		inline matrix_t& scaler( gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( scaler( vector_t( x, y, z ) ) ); }
		
	// OPERATE
		inline matrix_t operator - ( void ) const { return( inverse() ); }

	// PIPELINE
		inline matrix_t operator * ( const matrix_t& R ) const { 
			matrix_t tmp;
			for( int j=0; j<4; j++ )
				for( int i=0; i<4; i++ )
					tmp.M[ j ][ i ] = 
						M[ 0 ][ i ] * R.M[ j ][ 0 ] + 
						M[ 1 ][ i ] * R.M[ j ][ 1 ] + 
						M[ 2 ][ i ] * R.M[ j ][ 2 ] + 
						M[ 3 ][ i ] * R.M[ j ][ 3 ] ;
			return( tmp );
		}
		inline matrix_t& operator *= ( const matrix_t& R ) 
			{ return( (*this) = (*this) * R ); }
		
		inline matrix_t& translate( const vector_t& t )	
			{ matrix_t tmp; return( (*this) *= tmp.translator( t ) ); }
		inline matrix_t& rotate( const quat_t& q )	
			{ matrix_t tmp; return( (*this) *= tmp.rotator( q ) ); }
		inline matrix_t& rotate( const euler_t& e )	
			{ matrix_t tmp; return( (*this) *= tmp.rotator( e ) ); }
		inline matrix_t& shear( const vector_t& s )	
			{ matrix_t tmp; return( (*this) *= tmp.shearer( s ) ); }
		inline matrix_t& scale( const vector_t& s )	
			{ matrix_t tmp; return( (*this) *= tmp.scaler( s ) ); }
		inline matrix_t& lineate( const vector_t& x, const vector_t& y, const vector_t& z )	
			{ matrix_t tmp; return( (*this) *= tmp.lineator( x, y, z ) ); }

		inline matrix_t& translate( gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( translate( vector_t( x, y, z ) ) ); }
		inline matrix_t& rotate( gw_float_t w, gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( rotate( quat_t( w, x, y, z ) ) ); }
		inline matrix_t& rotate( gw_float_t angle, const vector_t& v, int use_radians = 0 )
			{ return( rotate( quat_t( angle, v, use_radians ) ) ); }
		inline matrix_t& rotate( gw_float_t p, gw_float_t h, gw_float_t r )
			{ return( rotate( euler_t( p, h, r ) ) ); }
		inline matrix_t& shear( gw_float_t xy, gw_float_t xz, gw_float_t yz )
			{ return( shear( vector_t( xy, xz, yz ) ) ); }
		inline matrix_t& scale( gw_float_t x, gw_float_t y, gw_float_t z )
			{ return( scale( vector_t( x, y, z ) ) ); }
		
	// EXTRACT
		int decompose( 
			vector4_t*	Perspective, 
			vector_t*	Translation, 
			quat_t*		Quaternion, 
			euler_t*	Euler, 
			vector_t*	Shearing, 
			vector_t*	Scaling, 
			int composition_flag = GWIZ::COMP_UNKNOWN
		) const;

		vector4_t	perspective	( int composition_flag = GWIZ::COMP_UNKNOWN ) const;
		vector_t	translation	( int composition_flag = GWIZ::COMP_UNKNOWN ) const;
		quat_t		quat		( int composition_flag = GWIZ::COMP_UNKNOWN ) const;
		euler_t		euler		( int composition_flag = GWIZ::COMP_UNKNOWN ) const;
		vector_t	shearing	( int composition_flag = GWIZ::COMP_UNKNOWN ) const;
		vector_t	scaling		( int composition_flag = GWIZ::COMP_UNKNOWN ) const;

	// VECTOR TRANSFORM
		inline vector_t operator * ( const vector_t& v ) const { 
			return( 
				vector_t(
					M[ 0 ][ 0 ] * v.x() + M[ 1 ][ 0 ] * v.y() + M[ 2 ][ 0 ] * v.z() + M[ 3 ][ 0 ], 
					M[ 0 ][ 1 ] * v.x() + M[ 1 ][ 1 ] * v.y() + M[ 2 ][ 1 ] * v.z() + M[ 3 ][ 1 ], 
					M[ 0 ][ 2 ] * v.x() + M[ 1 ][ 2 ] * v.y() + M[ 2 ][ 2 ] * v.z() + M[ 3 ][ 2 ]
				) 
			); 
		}
		inline vector4_t operator * ( const vector4_t& v ) const { 
			return( 
				vector4_t(
					M[ 0 ][ 0 ] * v.x() + M[ 1 ][ 0 ] * v.y() + M[ 2 ][ 0 ] * v.z() + M[ 3 ][ 0 ] * v.w(), 
					M[ 0 ][ 1 ] * v.x() + M[ 1 ][ 1 ] * v.y() + M[ 2 ][ 1 ] * v.z() + M[ 3 ][ 1 ] * v.w(), 
					M[ 0 ][ 2 ] * v.x() + M[ 1 ][ 2 ] * v.y() + M[ 2 ][ 2 ] * v.z() + M[ 3 ][ 2 ] * v.w(), 
					M[ 0 ][ 3 ] * v.x() + M[ 1 ][ 3 ] * v.y() + M[ 2 ][ 3 ] * v.z() + M[ 3 ][ 3 ] * v.w()
				) 
			); 
		}

	// STACK
		inline matrix_t& push(void)	{
			if( matrix_t *tmp = new matrix_t( *this ) )	{
				tmp->stack = stack;
				stack = tmp;
			}
			else 
				printf( "matrix_t::push ERR: new FAILED\n" );
			return( *this );
		}
		inline matrix_t& pop( matrix_t *M_get = 0x0 )	{
			if( stack )	{
				if( M_get )	
					*M_get = *this;
				matrix_t *tmp = stack;
				*this = *stack;
				stack = tmp->stack;
				tmp->stack = 0x0;
				delete tmp;
			}
			else 
				printf( "matrix_t::pop ERR: stack exceeded\n" );
			return( *this );
		}

    private:
        gw_float_t M[ 4 ][ 4 ];
        matrix_t *stack;
};

////////////////////////////////
#endif
