/*
 *  me_ct_time_shift_warp.cpp - part of SmartBody-lib's Motion Engine
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
 *      Andrew n marshall, USC
 *      Ed Fast, USC
 */

#include <ME/me_ct_time_shift_warp.hpp>


#include <iostream>
#include <iomanip>
#include <sstream>


typedef MeSpline1D::Knot  Knot;


const char* MeCtTimeShiftWarp::CONTROLLER_TYPE = "MeCtTimeShiftWarp";


MeCtTimeShiftWarp::MeCtTimeShiftWarp( MeController* child )
:	MeCtUnary( new MeCtUnary::Context(this), child )
{
	if( child ) {
		_sub_context->add_controller( child );
	}
}

const char* MeCtTimeShiftWarp::controller_type() {
	return CONTROLLER_TYPE;
}

double MeCtTimeShiftWarp::controller_duration() {
	MeSpline1D::Knot* knot = _time_func.knot_last();
	if( knot ) {
		return knot->get_x();  // greatest valid input time
	} else {
		return 0;
	}
}

bool MeCtTimeShiftWarp::controller_evaluate( double t, MeFrameData & frame ) {
	if( child() ) {
		t = _time_func( t );
		child()->evaluate( t, frame );
		return true;
	} else {
		return false;
	}
}

void MeCtTimeShiftWarp::print_state( int tab_count ) {
	using namespace std;

	string indent( tab_count, '\t' );
	ostringstream out;
	out << "MeCtTimeShiftWarp";
	const char* name = this->name();
	if( name && name[0]!='\0' )
		out << " \"" << name << "\"";

	// Don't show scientific notation
	out << fixed << setprecision(2);
	Knot* knot = _time_func.knot_first();
	if( knot ) {
		out << ":" << endl << indent << "time_func: " << *knot;
		knot = knot->get_next();
		while( knot ) {
			out << "; " << *knot;
			knot = knot->get_next();
		}
	}
	cout << out.str();

	print_children( tab_count );
}
