/*
 *  behavior_scheduler_fixed.cpp - part of SmartBody-lib
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
 */

#include <map>
#include "bml.hpp"
#include "behavior_scheduler_fixed.hpp"

using namespace std;
using namespace BML;

typedef vector<pair<wstring,float>> vec_sync_pairs;


// local utility function


/////////////////////////////////////////////////////////////////////////////
//  BehaviorSchedulerFixed
//

BehaviorSchedulerFixed::BehaviorSchedulerFixed( const vec_sync_pairs& input ) {
	// TODO: Replace exceptions in constructor with better solution

	vec_sync_pairs::const_iterator it = input.begin();
	vec_sync_pairs::const_iterator end = input.end();

	if( it != end ) {
		// At least one sync point
		float        last_time = 0;
		unsigned int cur_index = 0;

		// Make sure we deal with start first
		if( it->first != BML::ATTR_START ) {
			sync_point_times.push_back( make_pair<wstring,float>( BML::ATTR_START, 0 ) );
			map_indices.insert( make_pair<wstring,unsigned int>( BML::ATTR_START, cur_index++ ) );
		}

		for( ; it != end; ++it ) {
			if( it->second < last_time ) {
				throw BML::BmlException( "BehaviorSchedulerFixed: Invalid sync point order." );
			}

			const wstring& sync_id = it->first;
			last_time = it->second;

			sync_point_times.push_back( make_pair<wstring,float>( sync_id, last_time ) );
			map_indices.insert( make_pair<wstring,unsigned int>( sync_id, cur_index++ ) );
		}
	} else {
		throw BML::BmlException( "BehaviorSchedulerFixed: No sync points specified." );
	}
}


void BehaviorSchedulerFixed::schedule( SyncPoints& syncs, time_sec now ) {
	// Find first sync point that is set
	// TODO
}
