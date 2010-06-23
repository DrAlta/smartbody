/*
 *  me_ct_examples.h - part of SmartBody-lib
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

#ifndef ME_CT_EXAMPLES_H
#define ME_CT_EXAMPLES_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

#include "gwiz_math.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

///////////////////////////////////////////////////////////////////////////

class MeCtHeadOrient : public MeController	{
	
	public:
		MeCtHeadOrient();
		virtual ~MeCtHeadOrient();
		
		void init( void );
		void set_orient( float dur, float p, float h, float r );
		

		// Following must be public for comparisons
		static const char* _type_name;

	private:
		virtual void controller_start();
		virtual bool controller_evaluate( double t, MeFrameData& frame );
		
		virtual SkChannelArray& controller_channels() 	{ return( _channels ); }
		virtual double controller_duration()			{ return( (double)_duration ); }
		virtual const char* controller_type()			{ return( _type_name ); }
		virtual void print_state( int tabs );
		
		SkChannelArray _channels;
		
		float _duration;
		float _pitch_deg;
		float _heading_deg;
		float _roll_deg;
};

///////////////////////////////////////////////////////////////////////////

class MeCtSimpleTilt : public MeController	{
	
	public:
		MeCtSimpleTilt();
		virtual ~MeCtSimpleTilt();
		
		void init( void );
		void set_tilt( float dur, float angle_deg );
		
		// Following must be public for comparisons
		static const char* _type_name;

	private:
		virtual void controller_start();
		virtual bool controller_evaluate( double t, MeFrameData& frame );
		
		virtual SkChannelArray& controller_channels()	{ return( _channels ); }
		virtual double controller_duration()			{ return( (double)_duration ); }
		virtual const char* controller_type()			{ return( _type_name ); }
		virtual void print_state( int tabs );
		
		SkChannelArray _channels;
		
		float _duration;
		float _angle_deg;
};

///////////////////////////////////////////////////////////////////////////

class MeCtSimpleNod : public MeController	{
	
	public:
		MeCtSimpleNod();
		virtual ~MeCtSimpleNod();
		
		void init( void );
		void set_nod( float dur, float mag, float rep, int aff, float smooth = .5);
		
		// Following must be public for comparisons
		static const char* _type_name;

	private:
		virtual void controller_start();
		virtual bool controller_evaluate( double t, MeFrameData& frame );
		
		virtual SkChannelArray& controller_channels()	{ return( _channels ); }
		virtual double controller_duration()			{ return( (double)_duration ); }
		virtual const char* controller_type() const		{ return( _type_name ); }
		virtual void print_state( int tabs );
		
		SkChannelArray _channels;
		
		float _duration;
		float _magnitude;
		float _repetitions;
		int _affirmative;
		float _smooth;
		double _prev_time;
		bool _first_eval;
};

///////////////////////////////////////////////////////////////////////////
#endif
