/*
 *  bml_sync_point.cpp - part of SmartBody-lib
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
 *      Corne Versloot, while at USC
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 */

#include "bml_sync_point.hpp"

//#include <cstdlib>
//#include <iostream>
//#include <algorithm>

#include "bml.hpp"
#include "bml_xml_consts.hpp"



using namespace std;
using namespace BML;


//const bool USE_CUSTOM_PRUNE_POLICY          = false; // Future feature
//
//const bool LOG_BEHAVIOR_SYNCHPOINTS         = false;
//const bool LOG_BML_BEHAVIOR_SCHEDULE        = false;
//const bool LOG_METHODS						= false;
//const bool LOG_CONTROLLER_SCHEDULE			= false;
//const bool LOG_REQUEST_REALIZE_TIME_SPAN	= false;

TriggerEvent::TriggerEvent( const wstring& name, BmlRequestPtr request )
:	name( name ),
	request( request )
{
	int answer = 42;  //break here
}

void TriggerEvent::init( TriggerEventPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;
}

SyncPointPtr TriggerEvent::addSyncPoint() {
	return addSyncPoint( SyncPointPtr(), 0 );
}

SyncPointPtr TriggerEvent::addSyncPoint( SyncPointPtr par, float off ) {
	SyncPointPtr sync( new SyncPoint( weak_ptr.lock(), par, off ) );
	sync->init( sync );

	return sync;
}

SyncPoint::SyncPoint( const TriggerEventPtr trigger )
:	trigger(trigger),
	time(TIME_UNSET),
	offset( 0 )  // not used if parent is NULL
{}

SyncPoint::SyncPoint( const TriggerEventPtr trigger, SyncPointPtr par, float off )
:	trigger(trigger),
	time(TIME_UNSET),
	parent( par ),
	offset( off )
{}

void SyncPoint::init( SyncPointPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;
}


// default constructor
SequenceOfNamedSyncPoints::SequenceOfNamedSyncPoints()
{}

SequenceOfNamedSyncPoints::SequenceOfNamedSyncPoints( const SequenceOfNamedSyncPoints& other )
:	sync_seq( other.sync_seq ),
	idToSync( other.idToSync ),
	sp_start( other.sp_start ),
	sp_ready( other.sp_ready ),
	sp_stroke_start( other.sp_stroke_start ),
	sp_stroke( other.sp_stroke ),
	sp_stroke_end( other.sp_stroke_end ),
	sp_relax( other.sp_relax ),
	sp_end( other.sp_end )
{}

SequenceOfNamedSyncPoints::iterator SequenceOfNamedSyncPoints::first_scheduled() {
	// Find first sync point that is set
	iterator it = begin();
	iterator it_end = end();
	while( !( it==it_end || (*it)->isSet() ) ) {
		++it;
	}
	return it;
}

SequenceOfNamedSyncPoints::iterator SequenceOfNamedSyncPoints::insert( const wstring& id, SyncPointPtr sync, SequenceOfNamedSyncPoints::iterator pos ) {
	MapOfSyncPoint::iterator map_it = idToSync.find( id );
	if( map_it != idToSync.end() )
		return end();

	if( !idToSync.insert( make_pair( id, sync ) ).second )
		return end();
	return sync_seq.insert( pos, sync );
}

SequenceOfNamedSyncPoints::iterator SequenceOfNamedSyncPoints::pos_of( SyncPointPtr sync ) {
	return find( begin(), end(), sync );
}

SetOfWstring SequenceOfNamedSyncPoints::get_sync_names() {
	SetOfWstring names;

	MapOfSyncPoint::iterator it  = idToSync.begin();
	MapOfSyncPoint::iterator end = idToSync.end();
	for( ; it!=end; ++it ) {
		const wstring& name = it->first;
		if( !( names.insert( name ).second ) )
			wcerr << "ERROR: SequenceOfNamedSyncPoints::get_sync_names(): Failed to insert SyncPoint name \""<<name<<"\"." << endl;
	}

	return names;
}

SyncPointPtr SequenceOfNamedSyncPoints::sync_for_name( const std::wstring& name ) {
	MapOfSyncPoint::iterator result = idToSync.find( name );
	if( result != idToSync.end() )
		return result->second;
	else
		return SyncPointPtr();
}

BehaviorSpan SequenceOfNamedSyncPoints::getBehaviorSpan( time_sec persistent_threshold ) {
	time_sec start_time = TIME_UNSET;
	time_sec end_time   = TIME_UNSET;
	bool     persistent = false;

	iterator it     = begin();
	iterator it_end = end();
	for( ; it != it_end; ++it ) {
		time_sec time = (*it)->time;

		if( isTimeSet( time ) ) {
			if( time < persistent_threshold ) {
				if( isTimeSet( start_time ) ) {
					start_time = min( start_time, time );
					end_time   = max( end_time,   time );
				} else {
					start_time = end_time = time;
				}
			} else {
				persistent = true;
			}
		}
	}

	return BehaviorSpan( start_time, end_time, persistent );
}

void SequenceOfNamedSyncPoints::parseStandardSyncPoints( DOMElement* elem, BmlRequestPtr request, const string& behavior_id ) {
	// DOM functions never return NULL
	const wstring tag = elem->getTagName();
	const wstring id  = elem->getAttribute( ATTR_ID );

	/////////////////////// Create fake ID value?
	//if( !id ) {
	//	// Create fake id
	//	wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_START<<" point \""<<str<<"\".  Ignoring..."<<endl;
	//	
	//	// count preceeding element siblings
	//	int count = 0;
	//	DOMNode* sibling = elem->getPreviousSibling();
	//	while( sibling != NULL ) {
	//		++count;
	//		sibling = sibling->getPreviousSibling();
	//	}

	//	wstringstream id_buff();
	//	id_buff << 'b';
	//	id_buff.width( 3 );
	//	id_fill( 0 );
	//	id_buff << count;
	//	id_buff.width( 0 );
	//	id_buff << '_' << elem->getTagName(); // TODO: Replace possible namespace colon
	//}

	// Load SyncPoint references
	sp_start        = parseSyncPointAttr( elem, id, ATTR_START,        request, behavior_id );
	sp_ready        = parseSyncPointAttr( elem, id, ATTR_READY,        request, behavior_id );
	sp_stroke_start = parseSyncPointAttr( elem, id, ATTR_STROKE_START, request, behavior_id );
	sp_stroke       = parseSyncPointAttr( elem, id, ATTR_STROKE,       request, behavior_id );
	sp_stroke_end   = parseSyncPointAttr( elem, id, ATTR_STROKE_END,   request, behavior_id );
	sp_relax        = parseSyncPointAttr( elem, id, ATTR_RELAX,        request, behavior_id );
	sp_end          = parseSyncPointAttr( elem, id, ATTR_END,          request, behavior_id );
}


SyncPointPtr SequenceOfNamedSyncPoints::parseSyncPointAttr( DOMElement* elem, const std::wstring& elem_id, const std::wstring& sync_attr, const BmlRequestPtr request, const string& behavior_id ) {
	return parseSyncPointAttr( elem, elem_id, sync_attr, request, behavior_id, end() );
}

SyncPointPtr SequenceOfNamedSyncPoints::parseSyncPointAttr( DOMElement* elem, const std::wstring& elem_id, const std::wstring& sync_attr, const BmlRequestPtr request, const string& behavior_id, iterator pos ) {
	SyncPointPtr sync;

	wstring behavior_wid;
	{
		XMLCh* temp = XMLString::transcode( behavior_id.c_str() );
		behavior_wid = temp;
		XMLString::release( &temp );
	}

	MapOfSyncPoint::iterator map_it = idToSync.find( sync_attr );
	if( map_it != idToSync.end() ) {
		// TODO: Throw BML ParsingException
		wcerr << "ERROR: Behavior \""<<behavior_wid<<"\": SequenceOfNamedSyncPoints contains SyncPoint with id \"" << sync_attr << "\".  Ignoring attribute in <"<<elem->getTagName();
		if( !elem_id.empty() )
			wcerr<<"> id=\""<<elem_id<<"\"";
		else
			wcerr<<">";
		wcerr << " and returning existing SyncPoint."<<endl;
		return map_it->second;
	}

	const XMLCh* sync_ref = elem->getAttribute( sync_attr.c_str() );
	bool has_sync_ref = sync_ref!=NULL && XMLString::stringLen( sync_ref )>0;
	if( has_sync_ref ) {
		sync = request->getSyncPoint( sync_ref );
	}

	if( !sync ) {
		if( has_sync_ref ) {
			wcerr<<"WARNING: Behavior \""<<behavior_wid<<"\": SequenceOfNamedSyncPoints::parseSyncPointAttr(..): <"<<(elem->getTagName())<<"> BML tag refers to unknown SyncPoint "<<sync_attr<<"=\""<<sync_ref<<"\".  Creating placeholder..."<<endl;
		}

		TriggerEventPtr trigger;
		SyncPointPtr prev;

		if( pos != begin() )
			prev = *(--iterator(pos));
		if( prev )
			trigger = prev->trigger.lock();
		if( !trigger )
			trigger = request->start_trigger;

		sync = trigger->addSyncPoint();
	}

	if( sync ) {  // assert( sync )?
		insert( sync_attr, sync, pos );
	}

	return sync;
}

#if VALIDATE_BEHAVIOR_SYNCS

string SequenceOfNamedSyncPoints::debug_label( SyncPointPtr& sync ) {
	ostringstream out;

	wstring id = idForSyncPoint( sync );

	int count = 0;
	iterator it  = sync_seq.begin();
	iterator end = sync_seq.end();
	while( it!=end && (*it)!=sync ) {
		++it;
		++count;
	}

	if( it==end ) {
		out << "Invalid SyncPoint";
	} else {
		out << "SyncPoint #" << count;
	}
	if( !id.empty() ) {
		const char* ascii = xml_utils::asciiString( id.c_str() );
		out << " \"" << ascii << "\"";
		delete[] ascii;
	}

	return out.str();
}

void SequenceOfNamedSyncPoints::validate() {
	ostringstream out;
	out << "SequenceOfNamedSyncPoints validation errors:";

	bool valid = true;

	iterator it  = sync_seq.begin();
	iterator end = sync_seq.end();
	if( it != end ) {
		SyncPointPtr sync = (*it);

		if( sync != sp_start ) {
			out << endl << "\t" << debug_label(sync) << " is not sp_start";
				valid = false;
		}
		if( !isTimeSet( sync->time ) ) {
			out << endl << "\t" << debug_label(sync) <<" time is not set";

			// Find first set time
			do {
				++it;
			} while( it!=end && !isTimeSet( (*it)->time ) );

			if( it == end ) {
				out << endl << "\tAll SyncPoint::time are unset";
				valid = false;
			}
		}

		SyncPointPtr prev( sync );
		for( ++it; it != end; ++it ) {
			sync = (*it);

			if( isTimeSet( sync->time ) ) {
				if( sync->time < prev->time ) {
					out << endl << "\t" << debug_label( sync ) << " time " << sync->time << " less than prev->time " << prev->time;
				}

				prev = sync;
			}
		}
	} else {
		out << endl << "\tNo SyncPoints";
		valid = false;
	}

	if( !valid ) {
		throw SchedulingException( out.str().c_str() );
	}
}

#endif // INCOMPLETE_SYNCS_VALIDATION

std::wstring SequenceOfNamedSyncPoints::idForSyncPoint( SyncPointPtr sync ) {
	MapOfSyncPoint::iterator map_it  = idToSync.begin();
	MapOfSyncPoint::iterator map_end = idToSync.end();

	for( ; map_it!=map_end; ++map_it ) {
		if( map_it->second == sync ) {
			return map_it->first;  // early return
		}
	}

	// No match
	return wstring();
}

void SequenceOfNamedSyncPoints::applyParentTimes( std::string& warning_context ) {
	MapOfSyncPoint::iterator it  = idToSync.begin();
	MapOfSyncPoint::iterator end = idToSync.end();

	for( ; it!=end; ++it ) {
		const wstring& sync_id = it->first;
		SyncPointPtr  sync    = it->second;
		if( sync->parent ) {
			SyncPointPtr parent = sync->parent;
			if( isTimeSet( parent->time ) ) {
				if( isTimeSet( sync->time ) ) {
					char* ascii_sync_id = XMLString::transcode( sync_id.c_str() );

					cerr << "WARNING: ";
					if( !warning_context.empty() )
						cerr << warning_context << ": ";
					cerr << "SyncPoint \""<<ascii_sync_id<<"\" time set before applyParentTimes()." << endl;

					delete [] ascii_sync_id;
				}
				sync->time = parent->time + sync->offset;
			} else {
				char* ascii_sync_id = XMLString::transcode( sync_id.c_str() );

				// Parent not set!!
				cerr << "WARNING: ";
				if( !warning_context.empty() )
					cerr << warning_context << ": ";
				cerr << "SyncPoint \""<<ascii_sync_id<<"\" parent unset." << endl;

				delete [] ascii_sync_id;
			}
		}
	}
}

void SequenceOfNamedSyncPoints::printSyncIds() {
	VecOfSyncPoint::iterator vec_it     = begin();
	VecOfSyncPoint::iterator vec_end    = end();

	if( vec_it != vec_end ) {
		// First SyncPoint
		wstring sync_id = idForSyncPoint( *vec_it );
		wcout << sync_id;

		// Remaining
		for( ++vec_it; vec_it!=vec_end; ++vec_it ) {
			sync_id = idForSyncPoint( *vec_it );
			wcout << ", "<< sync_id;
		}
	}
	wcout << endl;
}

void SequenceOfNamedSyncPoints::printSyncTimes() {
	VecOfSyncPoint::iterator vec_it     = begin();
	VecOfSyncPoint::iterator vec_end    = end();

	// Iterate Syncs in order
	for( ; vec_it!=vec_end; ++vec_it ) {
		SyncPointPtr sync = (*vec_it);
		wstring sync_id = idForSyncPoint( sync );

		wcout << "\t" << sync_id << ": " << sync << endl;
	}
}

