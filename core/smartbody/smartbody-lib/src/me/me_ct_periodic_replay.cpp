/*
 *  me_ct_periodic_replay.cpp - part of SmartBody-lib's Motion Engine
 *  Copyright (C) 2010  University of Southern California
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

#include <ME/me_ct_periodic_replay.hpp>


#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>


const char* MeCtPeriodicReplay::CONTROLLER_TYPE = "MeCtPeriodicReplay";


MeCtPeriodicReplay::MeCtPeriodicReplay( MeController* child )
:	MeCtUnary( new MeCtUnary::Context(this), child )
{
	if( child ) {
		_sub_context->add_controller( child );
	}
}

const char* MeCtPeriodicReplay::controller_type() const {
	return CONTROLLER_TYPE;
}

void MeCtPeriodicReplay::init( double period ) {
	init( period, 0, 0 );
}

void MeCtPeriodicReplay::init( double period, double period_offset, double child_offset ) {
	this->period = period;
	this->period_offset = period_offset;
	this->child_time_offset = child_time_offset;
}

double MeCtPeriodicReplay::controller_duration() {
	return -1;  // indefinite, regardless of child duration
}

bool MeCtPeriodicReplay::controller_evaluate( double t, MeFrameData & frame ) {
	if( child() ) {
		t = fmod( t-period_offset, period ) + child_time_offset;;
		child()->evaluate( t, frame );
		return true;
	} else {
		return false;
	}
}

void MeCtPeriodicReplay::print_state( int tab_count ) {
	using namespace std;

	string indent( tab_count, '\t' );
	ostringstream out;
	out << CONTROLLER_TYPE;
	const char* name = this->name();
	if( name && name[0]!='\0' )
		out << " \"" << name << "\"";

	// Don't show scientific notation
	out << fixed << setprecision(2);
	out << " period=" << period;
	if( period_offset != 0 || child_time_offset!= 0 ) {  // abbreviate if it isn't interesting
		out << "; period_offset=" << period_offset;
		out << "; child_time_offset=" << child_time_offset;
	}
	cout << out.str();

	print_children( tab_count );
}
