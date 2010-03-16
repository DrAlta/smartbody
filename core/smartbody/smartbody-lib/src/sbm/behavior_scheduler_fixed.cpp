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
typedef std::map<std::wstring,unsigned int> map_id_indices;


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

		//// Make sure we deal with start first
		//if( it->first != BML::ATTR_START ) {
		//	sync_point_times.push_back( make_pair<wstring,float>( BML::ATTR_START, 0 ) );
		//	sync_id2index.insert( make_pair<wstring,unsigned int>( BML::ATTR_START, cur_index++ ) );
		//}

		map_id_indices::iterator map_end = sync_id2index.end();
		for( ; it != end; ++it ) {
			if( it->second < last_time ) {
				// TODO: include details like id and times.  Don't forget to transcode the wstring.
				throw BML::BmlException( "BehaviorSchedulerFixed: Invalid sync point timing." );
			}

			const wstring& sync_id = it->first;
			last_time = it->second;

			sync_point_times.push_back( make_pair<wstring,float>( sync_id, last_time ) );
			sync_id2index.insert( make_pair<wstring,unsigned int>( sync_id, cur_index++ ) );
		}
	} else {
		throw BML::BmlException( "BehaviorSchedulerFixed: No sync points specified." );
	}
}

void BehaviorSchedulerFixed::validate_ordering( SyncPoints& syncs ) {
	SyncPoints::iterator sp = syncs.begin();
	SyncPoints::iterator sp_end = syncs.end();

	vec_sync_pairs::iterator it = sync_point_times.begin();
	vec_sync_pairs::iterator it_end = sync_point_times.begin();

	SyncPoints::iterator last_sp;
	for( ; it!=it_end; ++it ) {
		wstring& name = it->first;
		SyncPointPtr sync = syncs.sync_for_name( name );
		if( !sync ) {
			throw BML::SchedulingException( "BehaviorSchedulerFixed: No such SyncPoint." );
		}
		SyncPoints::iterator pos = syncs.pos_of( sync );
		if( pos <= last_sp ) {
			throw BML::SchedulingException( "BehaviorSchedulerFixed: Out-of-order SyncPoint timing." );
		}
	}

	// Success! Valid ordering.
}

void BehaviorSchedulerFixed::schedule( SyncPoints& syncs, time_sec now ) {
	// validate the ordering before manipulating the SyncPoints
	validate_ordering( syncs );

	// Find first sync point that is set
	SyncPoints::iterator sp_end = syncs.end();
	SyncPoints::iterator first_set = syncs.first_scheduled();
	if( first_set == sp_end ) {
		// No SyncPoints previously scheduled
		// Schedule starting at time zero
		vec_sync_pairs::iterator it = sync_point_times.begin();
		vec_sync_pairs::iterator it_end = sync_point_times.begin();

		for( ; it!=it_end; ++it ) {
			SyncPointPtr sync = syncs.sync_for_name( it->first );  // Must exist.  See validate_ordering(..)
			sync->time = it->second;
		}
	} else {
		// TODO: "Merge" timing data
	}
}
