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
 *      Ed Fast, USC
 */

#include "bml_sync_point.hpp"

//#include <cstdlib>
//#include <iostream>
//#include <algorithm>

#include "bml.hpp"
#include "bml_exception.hpp"
#include "bml_xml_consts.hpp"



using namespace std;
using namespace BML;


// shorthand type for name_to_pos
typedef map<const wstring,BehaviorSyncPoints::iterator> MapNameToPos;  



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

///////////////////////////////////////////////////////////////////////////
//  NamedSyncPointPtr

NamedSyncPointPtr::NamedSyncPointPtr( const wstring& name, SyncPointPtr sync )
:	_name( name ),
	_sync( sync )
{}

NamedSyncPointPtr::NamedSyncPointPtr( const NamedSyncPointPtr& other )
:	_name( other.name() ),
	_sync( other.sync() )
{}



///////////////////////////////////////////////////////////////////////////
//  BehaviorSyncPoints

// default constructor
BehaviorSyncPoints::BehaviorSyncPoints()
{
	start_it = insert( ATTR_START, SyncPointPtr(), end() );
	ready_it = insert( ATTR_READY, SyncPointPtr(), end() );
	stroke_start_it = insert( ATTR_STROKE_START, SyncPointPtr(), end() );
	stroke_it = insert( ATTR_STROKE, SyncPointPtr(), end() );
	stroke_end_it = insert( ATTR_STROKE_END, SyncPointPtr(), end() );
	relax_it = insert( ATTR_RELAX, SyncPointPtr(), end() );
	end_it = insert( ATTR_END, SyncPointPtr(), end() );
}

BehaviorSyncPoints::BehaviorSyncPoints( const BehaviorSyncPoints& other )
:	named_syncs( other.named_syncs )
{
	// Repopulate name_to_pos with local iterators
	iterator it = named_syncs.begin();
	iterator end = named_syncs.end();

	for( ; it!=end; ++it ) {
		name_to_pos.insert( make_pair( it->name(), it ) );
	}

	// Rebuild Convience references
	start_it = find( ATTR_START );
	ready_it = find( ATTR_READY );
	stroke_start_it = find( ATTR_STROKE_START );
	stroke_it = find( ATTR_STROKE );
	stroke_end_it = find( ATTR_STROKE_END );
	relax_it = find( ATTR_RELAX );
	end_it = find( ATTR_END );
}

BehaviorSyncPoints::iterator BehaviorSyncPoints::insert( const wstring& name, SyncPointPtr sync, BehaviorSyncPoints::iterator pos ) {
	// Is there a way to test is pos is a valid iterator for named_syncs?

	iterator insert_pos;

	MapNameToPos::iterator map_it = name_to_pos.find( name );
	if( map_it != name_to_pos.end() ) {
		// Name already exists

		insert_pos = map_it->second;
		// TODO: check position relative to requested position

		if( insert_pos->sync() ) {
			cout << "ERROR: BehaviorSyncPoints::insert(..): SyncPoint already exists." << endl;
			return end();
		}

		// Update sync point
		insert_pos->set_sync( sync );
	} else {
		insert_pos = named_syncs.insert( pos, NamedSyncPointPtr( name, sync ) );
		if( insert_pos != named_syncs.end() )
			name_to_pos.insert( make_pair( name, insert_pos ) );
	}

	return insert_pos;
}

//BehaviorSyncPoints::iterator BehaviorSyncPoints::pos_of( SyncPointPtr sync ) {
//	return find( begin(), end(), sync );
//}

SetOfWstring BehaviorSyncPoints::get_sync_names() {
	SetOfWstring names;

	MapNameToPos::iterator it  = name_to_pos.begin();
	MapNameToPos::iterator end = name_to_pos.end();
	for( ; it!=end; ++it ) {
		const wstring& name = it->first;
		if( !( names.insert( name ).second ) )
			wcerr << "ERROR: BehaviorSyncPoints::get_sync_names(): Failed to insert SyncPoint name \""<<name<<"\"." << endl;
	}

	return names;
}

BehaviorSyncPoints::iterator BehaviorSyncPoints::find( const std::wstring& name ) {
	MapNameToPos::iterator map_it = name_to_pos.find( name );
	bool found_name = map_it != name_to_pos.end();

	return found_name? map_it->second : end();  // return iterator is found, else end()
}

BehaviorSyncPoints::iterator BehaviorSyncPoints::first_scheduled() {
	// Find first sync point that is set
	iterator it = begin();
	iterator it_end = end();
	while( !( it==it_end || it->is_set() ) ) {
		++it;
	}
	return it;
}

BehaviorSpan BehaviorSyncPoints::getBehaviorSpan( time_sec persistent_threshold ) {
	time_sec start_time = TIME_UNSET;
	time_sec end_time   = TIME_UNSET;
	bool     persistent = false;

	iterator it     = begin();
	iterator it_end = end();
	for( ; it != it_end; ++it ) {
		time_sec time = it->time();

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

void BehaviorSyncPoints::parseStandardSyncPoints( DOMElement* elem, BmlRequestPtr request, const string& behavior_id ) {
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
	start_it        = parseSyncPointAttr( elem, id, ATTR_START,        request, behavior_id );
	ready_it        = parseSyncPointAttr( elem, id, ATTR_READY,        request, behavior_id );
	stroke_start_it = parseSyncPointAttr( elem, id, ATTR_STROKE_START, request, behavior_id );
	stroke_it       = parseSyncPointAttr( elem, id, ATTR_STROKE,       request, behavior_id );
	stroke_end_it   = parseSyncPointAttr( elem, id, ATTR_STROKE_END,   request, behavior_id );
	relax_it        = parseSyncPointAttr( elem, id, ATTR_RELAX,        request, behavior_id );
	end_it          = parseSyncPointAttr( elem, id, ATTR_END,          request, behavior_id );
}


BehaviorSyncPoints::iterator BehaviorSyncPoints::parseSyncPointAttr( DOMElement* elem, const std::wstring& elem_id, const std::wstring& sync_attr, const BmlRequestPtr request, const string& behavior_id ) {
	//  Get behavior id as wstring
	wstring behavior_wid;
	{
		XMLCh* temp = XMLString::transcode( behavior_id.c_str() );
		behavior_wid = temp;
		XMLString::release( &temp );
	}

	// Does the sync point already exist?
	MapNameToPos::iterator map_it = name_to_pos.find( sync_attr );
	if( map_it != name_to_pos.end() && map_it->second->sync() ) {
		// SyncPoint of this name already exists

		// TODO: Throw BML ParsingException
		wcerr << "ERROR: Behavior \""<<behavior_wid<<"\": BehaviorSyncPoints contains SyncPoint with id \"" << sync_attr << "\".  Ignoring attribute in <"<<elem->getTagName();
		if( !elem_id.empty() )
			wcerr<<"> id=\""<<elem_id<<"\"";
		else
			wcerr<<">";
		wcerr << " and returning existing SyncPoint."<<endl;
		return map_it->second;
	}

	// Get the sync refernce string
	const XMLCh* sync_ref = elem->getAttribute( sync_attr.c_str() );
	bool has_sync_ref = sync_ref!=NULL && sync_ref[0]!=0;

	SyncPointPtr sync;  // unset by default
	if( sync_ref!=NULL && sync_ref[0]!=0 ) {
		// Has sync reference.
		sync = request->getSyncByReference( sync_ref );  // Parses the sync reference notation

		if( !sync ) {
			// TODO: More descriptive string
			throw BML::ParsingException( "BehaviorSyncPoints::parseSyncPointAttr: Invalid SyncPoint reference." );
		}
	} else {
		sync = request->start_trigger->addSyncPoint();
	}

	return insert( sync_attr, sync, end() );
}

#if VALIDATE_BEHAVIOR_SYNCS

string BehaviorSyncPoints::debug_label( SyncPointPtr& sync ) {
	ostringstream out;

	wstring id = idForSyncPoint( sync );

	int count = 0;
	iterator it  = named_syncs.begin();
	iterator end = named_syncs.end();
	while( it!=end && it->sync()!=sync ) {
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

void BehaviorSyncPoints::validate() {
	ostringstream out;
	out << "BehaviorSyncPoints validation errors:";

	bool valid = true;

	iterator it  = named_syncs.begin();
	iterator end = named_syncs.end();
	if( it != end ) {
		SyncPointPtr sync = it->sync();

		if( it != start_it ) {
			out << endl << "\t" << debug_label(sync) << " is not sp_start";
				valid = false;
		}
		if( !isTimeSet( sync->time ) ) {
			out << endl << "\t" << debug_label(sync) <<" time is not set";

			// Find first set time
			do {
				++it;
			} while( it!=end && !isTimeSet( it->time() ) );

			if( it == end ) {
				out << endl << "\tAll SyncPoint::time are unset";
				valid = false;
			}
		}

		SyncPointPtr prev( sync );
		for( ++it; it != end; ++it ) {
			sync = it->sync();

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

std::wstring BehaviorSyncPoints::idForSyncPoint( SyncPointPtr sync ) {
	iterator it = named_syncs.begin();
	iterator end = named_syncs.end();

	for( ; it!=end; ++it ) {
		if( it->sync() == sync ) {
			return it->name();  // early return
		}
	}

	// No match
	return wstring();
}

void BehaviorSyncPoints::applyParentTimes( std::string& warning_context ) {
	iterator it  = named_syncs.begin();
	iterator end = named_syncs.end();

	for( ; it!=end; ++it ) {
		SyncPointPtr sync = it->sync();
		if( sync ) {
			SyncPointPtr parent = sync->parent;
			if( parent ) {
				if( parent->is_set() ) {
					if( sync->is_set() ) {
						char* ascii_sync_id = XMLString::transcode( it->name().c_str() );

						cerr << "WARNING: ";
						if( !warning_context.empty() )
							cerr << warning_context << ": ";
						cerr << "SyncPoint \""<<ascii_sync_id<<"\" time set before applyParentTimes()." << endl;

						delete [] ascii_sync_id;
					}
					sync->time = parent->time + sync->offset;
				} else {
					char* ascii_sync_id = XMLString::transcode( it->name().c_str() );

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
}

void BehaviorSyncPoints::printSyncIds() {
	wostringstream buffer;

	if( !named_syncs.empty() ) {
		iterator it = named_syncs.begin();
		iterator end = named_syncs.end();

		// First SyncPoint
		buffer << it->name();

		// Remaining
		for( ++it; it!=end; ++it ) {
			buffer << ", " << it->name();
		}
	}

	wcout << buffer.str() << endl;
}

void BehaviorSyncPoints::printSyncTimes() {
	wostringstream buffer; // buffer output for single write

	if( !named_syncs.empty() ) {
		iterator it = named_syncs.begin();
		iterator end = named_syncs.end();

		// Iterate Syncs in order
		for( ; it!=end; ++it ) {
			wcout << "\t" << it->name() << ": " << it->sync() << endl;
		}
	}

	wcout << buffer.str();
}

