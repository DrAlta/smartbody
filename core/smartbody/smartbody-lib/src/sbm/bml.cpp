/*
 *  bml.cpp - part of SmartBody-lib
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

#include "bml.hpp"

#include <cstdlib>
#include <iostream>

#include "mcontrol_util.h"
#include "bml_processor.hpp"


using namespace std;
using namespace BML;
using namespace SmartBody;


const bool LOG_METHODS			= false;
const bool LOG_SCHEDULING		= false;
const bool LOG_SPEECH			= false;
const bool LOG_ABNORMAL_SPEED	= false;


// XML Constants
const XMLCh TAG_TM[]        = L"tm";
const XMLCh TAG_MARK[]      = L"mark";

const XMLCh ATTR_ID[]       = L"id";
const XMLCh ATTR_TYPE[]     = L"type";
const XMLCh ATTR_NAME[]     = L"name";

const XMLCh VALUE_TEXT_PLAIN[] = L"text/plain";
const XMLCh VALUE_SSML[]       = L"application/ssml+xml";


#define ENABLE_MissingSyncPoint_HACK 0


///////////////////////////////////////////////////////////////////////////
//  Helper Functions
const XMLCh* BML::buildBmlId( const XMLCh* behaviorId, const XMLCh* synchId ) {
	if( XMLString::indexOf( synchId, ':' ) != -1 )
		return synchId;
	int bLen = XMLString::stringLen( behaviorId );
	int mLen = XMLString::stringLen( synchId );
	int len = bLen+1+mLen+1; // including \0
	
	XMLCh* str = new XMLCh[ len ];
	XMLString::copyNString( str,        behaviorId, bLen );
	XMLString::copyNString( str+bLen,   L":",       1 );
	XMLString::copyNString( str+bLen+1, synchId,   mLen );
	str[ len-1 ] = 0;

	return str;
}

bool BML::isValidBmlId( const XMLCh* id ) {
	// String must have length and begin with alpha or '_' (like a C/C++ identifier)
	if( id==NULL )
		return false;
	XMLCh c = id[0];
	if( c == '\0' ||
	    !( XMLString::isAlpha( c ) ||
		   c == '_' ) )
		return false;

	// Remaining characters can also be digits
	int len = XMLString::stringLen( id );
	for( int i=1; i< len; ++i ) {
		c = id[i];
		if( !XMLString::isAlphaNum( c ) &&
			c != '_' )
			return false;
	}

	return true;
}

bool BML::isValidTmId( const XMLCh* id ) {
	if( !isValidBmlId( id ) )
		return false;
	// is not start/ready/stroke/relax/end
	switch( id[0] ) {
		case 'r':
		{
			if( id[1]=='e' ) {
				return( XMLString::compareString( id, TM_READY )!=0 &&
					    XMLString::compareString( id, TM_RELAX )==0 );
			}
			return true;
		}
		case 's':
		{
			if( id[1]=='t' ) {
				return( XMLString::compareString( id, TM_START )!=0 &&
  					    XMLString::compareString( id, TM_STROKE )==0 );
			}
			return true;
		}
		default:
		{
			return( XMLString::compareString( id, TM_END )!=0 );
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//  Class Member Definitions
BmlRequest::BmlRequest( const SbmCharacter* agent, const string & requestId, const string & recipientId, const string & msgId )
:	agent(agent),
	requestId( requestId ),
	recipientId( recipientId ),
	msgId( msgId ),
    synch_points(),
	audioPlay( NULL ),
	audioStop( NULL )
{}

void BmlRequest::init( BmlRequestPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;

	start_trigger = createTrigger( "bml:start" );

	const XMLCh* start_id = L"bml:start";
	const XMLCh* end_id   = L"bml:end";
	bml_start = start_trigger->addSynchPoint( start_id );
#if SYNC_LINKED_LIST
	bml_start->init( bml_start, SynchPointPtr() );
#else
	bml_start->init( bml_start );
#endif // SYNC_LINKED_LIST

	synch_points.insert( make_pair( start_id, bml_start ) );

	//// bml:end SynchPoint removed until it can be better evaluated
	//bml_end.reset( start_trigger->addSynchPoint( end_id ) );
	//bml_end->init( bml_start );
	//
	//synch_points.insert( make_pair( end_id, bml_end ) );


	//////////////////////////////////////////////////////////////////
	//  OLD CODE for reference
	//if( numTriggers > 0 ) {
	//	// First trigger has no prev
	//	triggers[0] = new TriggerEvent( this, NULL );
    //
	//	// rest of the triggers
	//	unsigned int i = 1;
	//	for(; i<numTriggers; i++ ) {
	//		triggers[i] = new TriggerEvent( this, triggers[numTriggers-1] );
	//	}
	//	first = new SynchPoint( L"act:start", triggers[0], NULL );
	//	first->next = triggers[0]->start;
	//	triggers[0]->start = first;
	//	first->time = 0.0001;
	//	last  = new SynchPoint( L"act:end", triggers[i-1], triggers[i-1]->end );
	//	triggers[i-1]->end = last;
	//	/*first = triggers[0]->start;
	//	last  = triggers[i-1]->end;*/
	//} else {
	//	//
	//	cout << "setting first and last!" << endl;
	//	first = new SynchPoint( L"act:start", NULL, NULL );
	//	first->time = 0.0001;
	//	last  = new SynchPoint( L"act:end", NULL, first );
	//	first->next = last;
	//	synch_points.insert(make_pair(L"act:start", first));
	//	synch_points.insert(make_pair(L"act:end", last));
	//}
}

BmlRequest::~BmlRequest() {
	// delete audio commands
	if( audioPlay )
		delete [] audioPlay;
	if( audioStop )
		delete [] audioStop;

	// delete BehaviorRequests
	size_t count = behaviors.size();
	for( size_t i=0; i<count; ++i ) {
		delete behaviors[i];
	}
	if( speech_request != NULL ) {
		speech_request.reset();
	}

	// delete triggers
	//  TODO: circular ref to clean up.
	count = triggers.size();
	for( size_t i=0; i<count; i++ ) {
		triggers[ i ]->request.reset();  // remove ref
	}
	// The following were deleted in the above loop
	start_trigger.reset();
	bml_start.reset();
	//bml_end.reset;  // TODO: Temporarily removed
	speech_trigger.reset();

	// delete visemes
	count = visemes.size();
	for( size_t i=0; i<count; ++i )
		delete visemes[i];
	//{	typedef VecOfVisemeData::iterator i_type;
	//	i_type i = visemes.begin();
	//	i_type end = visemes.end();
	//
	//	for( ; i != end; ++i )
	//		delete [] *i;
	//	visemes.clear();
	//}
}

TriggerEventPtr BmlRequest::createTrigger( const string& name ) {
	TriggerEventPtr trigger( new TriggerEvent( name, weak_ptr.lock() ) );
	trigger->init( trigger );

	triggers.push_back( trigger );
	return trigger;
}

void BmlRequest::addBehavior( BehaviorRequest* behavior ) {
	behaviors.push_back( behavior );
}

SynchPointPtr BmlRequest::getSynchPoint( const XMLCh * name ) {
	MapOfSynchPoint::iterator mySearchIter = synch_points.find(name);
	if ( mySearchIter != synch_points.end()){
		return (*mySearchIter).second;
	}

	// Get index to last '+' or '-' character
	int index = XMLString::lastIndexOf(name, '+');
	if(XMLString::lastIndexOf(name, '-') > index )
		index = XMLString::lastIndexOf(name, '-');

	if( index > -1 ) { //check for offset
		int len = XMLString::stringLen( name );
		XMLCh *off = new XMLCh[ len-index+1 ];  // +1 for '\0'
		XMLString::subString( off, name, index, len );  // include sign
		char*ascii = XMLString::transcode(off);
		float offset = (float)atof( ascii );
		delete [] ascii;

		XMLCh *key = new XMLCh[index];
		XMLString::subString(key, name, 0, index);
		/*if (XMLString::compareString( key, L"act:start" )==0 && offset < 0) {
			wcerr<<"WARNING: BmlRequest::getSynchPoint: BML offset \""<< name<<"\" is negative with regard to  act:start, offset set to 0.0!" << endl;
			offset = 0;
		}*/
		mySearchIter = synch_points.find(key);
		if( mySearchIter == synch_points.end() ) {
			wcerr<<"WARNING: BmlRequest::getSynchPoint: BML offset refers to unknown "<<key<<" point.  Ignoring..."<<endl;
		} else {
			SynchPointPtr parent = mySearchIter->second;
			if( parent ) {
				SynchPointPtr sync( new SynchPoint(name, triggers.at(triggers.size()-1), parent, offset) );
#if SYNC_LINKED_LIST
				sync->init( sync, parent );
#else
				sync->init( sync );
#endif // SYNC_LINKED_LIST

				//std::pair<XMLCh*,SynchPoint *> foo = make_pair(const_cast<XMLCh *>(name), sync);
				//synch_points.insert(foo);
				synch_points.insert( make_pair( name, sync ) );
				wcout << "insering new synch_point [" << name << "]" << endl;
				if( parent && parent->time != TIME_UNSET )
					sync->time = parent->time + offset; 
				return sync;
			} else {
				return SynchPointPtr();  // NULL
			}
		}
	} else if( XMLString::indexOf(name, ':') == -1 ) {  // TODO: fix numeric reference
		char* temp = XMLString::transcode( name );
		float time = (float)( atof( temp ) );

        SynchPointPtr sync = start_trigger->addSynchPoint( name, bml_start, bml_start, time );
#if SYNC_LINKED_LIST
		sync->init( sync, (*mySearchIter).second );
#else
		sync->init( sync );
#endif // SYNC_LINKED_LIST

		synch_points.insert( make_pair( name, sync ) );
		wcout << "insering new synch_point [" << name << "] (offset \""<< time <<"\" relative to starttime of the action)" << endl;
		return sync;
	}
	return SynchPointPtr();  // NULL
}


TriggerEvent::TriggerEvent( const string& name, BmlRequestPtr request )
:	name( name ),
	request( request )
{
	int answer = 42;  //break here
}

void TriggerEvent::init( TriggerEventPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;
}

SynchPointPtr TriggerEvent::addSynchPoint( const XMLCh* name ) {
	return addSynchPoint( name, SynchPointPtr(), SynchPointPtr(), 0 );
}

SynchPointPtr TriggerEvent::addSynchPoint( const XMLCh* name, SynchPointPtr prev ) {
	return addSynchPoint( name, prev, SynchPointPtr(), 0 );
}

SynchPointPtr TriggerEvent::addSynchPoint( const XMLCh* name, SynchPointPtr prev, SynchPointPtr par, float off ) {
	// TODO: Delay addition of synch point until behavior is added to BmlRequest,
	//       allowing for failure without artifacts.
	BmlRequestPtr request( this->request.lock() );
	if( name && request && request->synch_points[name] )
		throw BML::Processor::BodyPlannerException( "BML Request SynchPoint naming collision" );

	SynchPointPtr sync( new SynchPoint( name, weak_ptr.lock(), par, off ) );
#if SYNC_LINKED_LIST
	sync->init( sync, prev );
#else
	sync->init( sync );
#endif // SYNC_LINKED_LIST

	if( name && request ) {
		request->synch_points[name] = sync;
	}

	return sync;
}

//SynchPoint* TriggerEvent::getSynchPoint( const XMLCh* name ) {
//	return request->getSynchPoint( name );
//}

SynchPoint::SynchPoint( const XMLCh* name, const TriggerEventPtr trigger)
	: name(name? new XMLCh[XMLString::stringLen(name)+1]: NULL ),
	  trigger(trigger),
      time(TIME_UNSET),
	  offset( 0 )  // not used if parent is NULL
{
	if( name ) 
		XMLString::copyString( (XMLCh *const)(this->name), name );
}

SynchPoint::SynchPoint( const XMLCh* name, const TriggerEventPtr trigger, SynchPointPtr par, float off )
	: name(name? new XMLCh[XMLString::stringLen(name)+1]: NULL ),
	  trigger(trigger),
      time(TIME_UNSET),
	  parent( par ),
	  offset( off )
{
	if( name ) 
		XMLString::copyString( (XMLCh *const)(this->name), name );
}

#if SYNC_LINKED_LIST
void SynchPoint::init( SynchPointPtr self, SynchPointPtr prev_ptr ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;

	this->prev = prev_ptr;

	if( prev ) {
		next = prev->next;
		prev->next = self;
		if( next )
			next->prev = self;
	} // else prev & next remains unset
}
#else
void SynchPoint::init( SynchPointPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;
}
#endif // SYNC_LINKED_LIST

SynchPoint::~SynchPoint() {
   delete name;
}

//SynchPoints::SynchPoints()
//:	start( NULL ),
//	ready( NULL ),
//	strokeStart( NULL ),
//	stroke( NULL ),
//	strokeEnd( NULL ),
//	relax( NULL ),
//	end( NULL )
//{}

// The following code is used to insert sync points if they did not exist by prior references.
// It is almost completely untested, and thus off by default.
// This hack does not address the other half of the issue:
//   what to do with these unscheduled synch points at the behavior request level
void MissingSyncPoint_HACK( SynchPointPtr &sp, const XMLCh* id, const BmlRequestPtr request, SynchPointPtr prev ) {
#if ENABLE_MissingSyncPoint_HACK  // Hack enabled if 1
	if( sp==NULL ) {
		sp = new SynchPoint( id, request, prev );
	}
#endif
}

void SynchPoints::parseStandardSynchPoints( DOMElement* elem, BmlRequestPtr request ) {
	const XMLCh* tag = elem->getTagName();
	const XMLCh* id  = elem->getAttribute( ATTR_ID );


	// Load SynchPoint references
	start.reset();
	const XMLCh* str = elem->getAttribute( TM_START );
	if( str && XMLString::stringLen( str ) ) {
		start = request->getSynchPoint( str );
		if( start==NULL )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_START<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( start, TM_START, request, SynchPointPtr() );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	ready.reset();
	str = elem->getAttribute( TM_READY );
	if( str && XMLString::stringLen( str ) ) {
		ready = request->getSynchPoint( str );
		if( !ready )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_READY<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( ready, TM_READY, request, start );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	strokeStart.reset();
	str = elem->getAttribute( TM_STROKE_START );
	if( str && XMLString::stringLen( str ) ) {
		strokeStart= request->getSynchPoint( str );
		if( !strokeStart )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_STROKE_START<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( strokeStart, TM_STROKE_START, request, ready );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	stroke.reset();
	str = elem->getAttribute( TM_STROKE );
	if( str && XMLString::stringLen( str ) ) {
		stroke = request->getSynchPoint( str );
		if( !stroke )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_STROKE<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( stroke, TM_STROKE, request, strokeStart );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	strokeEnd.reset();
	str = elem->getAttribute( TM_STROKE_END );
	if( str && XMLString::stringLen( str ) ) {
		strokeEnd = request->getSynchPoint( str );
		if( !strokeEnd )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_STROKE_END<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( strokeEnd, TM_STROKE_END, request, stroke );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	relax.reset();
	str = elem->getAttribute( TM_RELAX );
	if( str && XMLString::stringLen( str ) ) {
		relax = request->getSynchPoint( str );
		if( !relax )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_RELAX<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( relax, TM_RELAX, request, strokeEnd );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?

	end.reset();
	str = elem->getAttribute( TM_END );
	if( str && XMLString::stringLen( str ) ) {
		end = request->getSynchPoint( str );
		if( !end )
			wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag refers to unknown "<<TM_END<<" point \""<<str<<"\".  Ignoring..."<<endl;
	}
	MissingSyncPoint_HACK( end, TM_END, request, relax );  //  TODO: Replace hack appropriately: if( start==NULL ) ...?
}

SbmCommand::SbmCommand( std::string & command, float time )
:	command( command ),
	time( time )
{}

//  Copy Constructor (for STL)
SbmCommand::SbmCommand( SbmCommand& other )
:	command( other.command ),
	time( other.time )
{}

//  Assignment op (for STL)
SbmCommand& SbmCommand::operator= (const SbmCommand& other ) {
	command = other.command;
	time    = other.time;
	return *this;
}



///////////////////////////////////////////////////////////////////////////
// BehaviorRequest

// methods
BehaviorRequest::BehaviorRequest( //const BehaviorType type, const void* data,
							    const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end,
								time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime, float speed )
  : //type(type), data(data),
    start(start), ready(ready), stroke(stroke), relax(relax), end(end), audioOffset(TIME_UNSET),
	startTime(startTime), 
    readyTime(readyTime), 
    strokeTime(strokeTime),
    relaxTime(relaxTime),
    endTime(endTime),
    speed(speed)
{}

BehaviorRequest::~BehaviorRequest() {
}

time_sec BehaviorRequest::getAudioRelativeStart() {
    // Calculate if uncached
    if( audioOffset == TIME_UNSET )
        audioOffset = calcAudioRelativeStart();
    return audioOffset;
}

time_sec BehaviorRequest::calcAudioRelativeStart() {
    /*  The following implements a search for the two most important TimeMArkers, and then scales the time to meet both.
     *  Importance is ranked in this order: stroke, ready, relax, start, end
     *  If only on SynchPoint is found, the controller maintains its natural duration.
     *  If no synch points are found, the behavior aligns to the start of the audio.
     */
    bool hasStart  = start  && start->time!=TIME_UNSET;
    bool hasReady  = ready  && ready->time!=TIME_UNSET;
    bool hasStroke = stroke && stroke->time!=TIME_UNSET;
    bool hasRelax  = relax  && relax->time!=TIME_UNSET;
    bool hasEnd    = end    && end->time!=TIME_UNSET;


    if( hasStroke ) {  // Handle stroke first (most important)
        if( hasReady ) {
            if( ready->time >= stroke->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before stroke... ignoring ready." << endl;
                // TODO: error recovery
            } else if( readyTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before strokeTime... ignoring ready." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - ready->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - readyTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing ready synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing ready synch constraint." << endl;
				}

				return( stroke->time-(strokeTime/speed) );
			}
        }
        if( hasRelax ) {
            if( stroke->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): stroke NOT before relax... ignoring relax." << endl;
                // TODO: error recovery
            } else if( strokeTime >= relaxTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = relax->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing ready synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing ready synch constraint." << endl;
				}

				return( stroke->time-(strokeTime/speed) );
			}
        }
        if( hasStart ) {
            if( start->time >= stroke->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before stroke... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before strokeTime... ignoring start." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - start->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - startTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing start synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing start synch constraint." << endl;
				}

				return( stroke->time-(strokeTime/speed) );
			}
        }
        if( hasEnd ) {
            if( stroke->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): stroke NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( strokeTime >= endTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = end->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = endTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end synch constraint." << endl;
				}

				return( stroke->time-(strokeTime/speed) );
			}
        }
        // Only stroke
        speed = 1;
        return( stroke->time - strokeTime );
    }
    if( hasReady ) {  // Next comes ready
        if( hasRelax ) {
            if( ready->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before relax... ignoring relax." << endl;
                // TODO: error recovery
            } else if( readyTime >= relaxTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = relax->time - ready->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - readyTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing relax synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing relax synch constraint." << endl;
				}

				return( ready->time-(readyTime/speed) );
			}
        }
        if( hasStart ) {
            if( start->time >= ready->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before ready... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= readyTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before readyTime... ignoring start." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = ready->time - start->time;  // realtime diff
				time_sec ctlrDiff = readyTime - startTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing start synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing start synch constraint." << endl;
				}

				return( ready->time-(readyTime/speed) );
			}
        }
        if( hasEnd ) {
            if( ready->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( readyTime >= endTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = end->time - ready->time;  // realtime diff
				time_sec ctlrDiff = endTime - readyTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end synch constraint." << endl;
				}

				return( ready->time-(readyTime/speed) );
			}
        }
        // Only ready
        speed = 1;
        return( ready->time - readyTime );
    }
    if( hasRelax ) {  // Next comes relax
        if( hasStart ) {
            if( start->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before relax... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= relaxTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before relaxTime... ignoring start." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = relax->time - start->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - startTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing start synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing start synch constraint." << endl;
				}

				return( relax->time-(relaxTime/speed) );
			}
        }
        if( hasEnd ) {
            if( relax->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): relax NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( relaxTime >= endTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): relaxTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = end->time - relax->time;  // realtime diff
				time_sec ctlrDiff = endTime - relaxTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end synch constraint." << endl;
				}

				return( relax->time-(relaxTime/speed) );
			}
        }
        // Only relax
        speed = 1;
        return( relax->time - relaxTime );
    }
    if( hasStart ) {  // Next comes start
        if( hasEnd ) {
            if( start->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( startTime >= endTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and start.
				time_sec rtDiff = end->time - start->time;  // realtime diff
				time_sec ctlrDiff = endTime - startTime;
				speed = ctlrDiff / rtDiff;

				if( LOG_ABNORMAL_SPEED ) {
					if( speed > 2 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end synch constraint." << endl;
					else if( speed < 0.3 )
						clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end synch constraint." << endl;
				}

				return( start->time-(startTime/speed) );
			}
        }
        // Only start
        speed = 1;
        return( start->time - startTime );
    }
    speed = 1;  // End or nothing
    if( hasEnd ) {  // Last comes end
        // Only end
        return( end->time - endTime );
    } else {
        // No synch_points... align to audio
        //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): no valid time refences." << endl;
        return 0;
    }
/*
    if( hasStroke ) {  // Handle stroke first (most important)
        if( hasReady ) {
            if( ready->time >= stroke->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before stroke... ignoring ready." << endl;
                // TODO: error recovery
            } else if( startTime >= strokeTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before strokeTime... ignoring ready." << endl;
                // TODO: error recovery
            } 

            // adjust speed to start and stroke.
            time_sec rtDiff = stroke->time - ready->time;  // realtime diff
            time_sec ctlrDiff = strokeTime - readyTime;
            speed = rtDiff / ctlrDiff;
            return( stroke->time-(strokeTime/speed) );
        }
        if( hasRelax ) {
            if( stroke->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): stroke NOT before relax... ignoring relax." << endl;
                // TODO: error recovery
            } else if( strokeTime >= relaxTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before relaxTime... ignoring relax." << endl;
                // TODO: error recovery
            } 

            // adjust speed to start and stroke.
            time_sec rtDiff = relax->time - stroke->time;  // realtime diff
            time_sec ctlrDiff = relaxTime - strokeTime;
            speed = rtDiff / ctlrDiff;
            return( stroke->time-(strokeTime/speed) );
        }
        if( hasStart ) {
            if( start->time >= stroke->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before stroke... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= strokeTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before strokeTime... ignoring start." << endl;
                // TODO: error recovery
            } 

            // adjust speed to start and stroke.
            time_sec rtDiff = stroke->time - start->time;  // realtime diff
            time_sec ctlrDiff = strokeTime - startTime;
            speed = rtDiff / ctlrDiff;
            return( stroke->time-(strokeTime/speed) );
        }
        if( hasEnd ) {
            if( stroke->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): stroke NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( strokeTime >= endTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before endTime... ignoring end." << endl;
                // TODO: error recovery
            } 

            // adjust speed to start and stroke.
            time_sec rtDiff = end->time - stroke->time;  // realtime diff
            time_sec ctlrDiff = endTime - strokeTime;
            speed = rtDiff / ctlrDiff;
            return( stroke->time-(strokeTime/speed) );
        }
        // Only stroke
        speed = 1;
        return( stroke->time - strokeTime );
    }
    if( hasReady ) {  // Next comes ready
        if( hasRelax ) {
            if( ready->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before relax... ignoring relax." << endl;
                // TODO: error recovery
            } else if( readyTime >= relaxTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before relaxTime... ignoring relax." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and ready.
            time_sec rtDiff = relax->time - ready->time;  // realtime diff
            time_sec ctlrDiff = relaxTime - readyTime;
            speed = rtDiff / ctlrDiff;
            return( ready->time-(readyTime/speed) );
        }
        if( hasStart ) {
            if( start->time >= ready->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before ready... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= readyTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before readyTime... ignoring start." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and ready.
            time_sec rtDiff = ready->time - start->time;  // realtime diff
            time_sec ctlrDiff = readyTime - startTime;
            speed = rtDiff / ctlrDiff;
            return( ready->time-(readyTime/speed) );
        }
        if( hasEnd ) {
            if( ready->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): ready NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( readyTime >= endTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before endTime... ignoring end." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and ready.
            time_sec rtDiff = end->time - ready->time;  // realtime diff
            time_sec ctlrDiff = endTime - readyTime;
            speed = rtDiff / ctlrDiff;
            return( ready->time-(readyTime/speed) );
        }
        // Only ready
        speed = 1;
        return( ready->time - readyTime );
    }
    if( hasRelax ) {  // Next comes relax
        if( hasStart ) {
            if( start->time >= relax->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before relax... ignoring start." << endl;
                // TODO: error recovery
            } else if( startTime >= relaxTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before relaxTime... ignoring start." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and relax.
            time_sec rtDiff = relax->time - start->time;  // realtime diff
            time_sec ctlrDiff = relaxTime - startTime;
            speed = rtDiff / ctlrDiff;
            return( relax->time-(relaxTime/speed) );
        }
        if( hasEnd ) {
            if( relax->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): relax NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( relaxTime >= endTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): relaxTime NOT before endTime... ignoring end." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and relax.
            time_sec rtDiff = end->time - relax->time;  // realtime diff
            time_sec ctlrDiff = endTime - relaxTime;
            speed = rtDiff / ctlrDiff;
            return( relax->time-(relaxTime/speed) );
        }
        // Only relax
        speed = 1;
        return( relax->time - relaxTime );
    }
    if( hasStart ) {  // Next comes start
        if( hasEnd ) {
            if( start->time >= end->time ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): start NOT before end... ignoring end." << endl;
                // TODO: error recovery
            } else if( startTime >= endTime ) {
                clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before endTime... ignoring end." << endl;
                // TODO: error recovery
            }

            // adjust speed to start and start.
            time_sec rtDiff = end->time - start->time;  // realtime diff
            time_sec ctlrDiff = endTime - startTime;
            speed = rtDiff / ctlrDiff;
            return( start->time-(startTime/speed) );
        }
        // Only start
        speed = 1;
        return( start->time - startTime );
    }
    speed = 1;  // End or nothing
    if( hasEnd ) {  // Last comes end
        // Only end
        return( end->time - endTime );
    } else {
        // No synch_points... align to audio
        clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): no valid time refences." << endl;
        return 0;
    }
*/
}

void BehaviorRequest::schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
                               VecOfVisemeData& visemes,
                               VecOfSbmCommand& commands,
							   time_sec startAt ) {
    // Offset all times by startAt-startTime and scale to previously determined speed
	time_sec readyAt = startAt + (readyTime-startTime)*speed;
    time_sec strokeAt = startAt + (strokeTime-startTime)*speed;
    time_sec relaxAt = startAt + (relaxTime-startTime)*speed;
    time_sec endAt = startAt + (endTime-startTime)*speed;
	schedule( mcu, actor, scheduler, visemes, commands, startAt, readyAt, strokeAt, relaxAt, endAt );
}


//  MeControllerRequest
MeControllerRequest::MeControllerRequest( TrackType trackType, MeController *controller,
                                          const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : BehaviorRequest( start, ready, stroke, relax, end,
                    /* startTime  */ 0, 
					/* readyTime  */ time_sec( controller->indt() ), 
					/* strokeTime */ time_sec( controller->emphasist() ),
					/* readyTime  */ time_sec( controller->controller_duration()-controller->outdt() ), 
					/* endtime    */ time_sec( controller->controller_duration() ),
					/* speed      */ 1 ),
    trackType(trackType), controller(controller)
{
	controller->ref();

    if( controller->controller_duration() < 0 ) {
        relaxTime = endTime = numeric_limits<time_sec>::max();
    }
}

MeControllerRequest::~MeControllerRequest() {
	controller->unref();
}

void MeControllerRequest::schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
                                    VecOfVisemeData& visemes,
                                    VecOfSbmCommand& commands,
								    time_sec startAt, time_sec readyAt, time_sec strokeAt, time_sec relaxAt, time_sec endAt ) {
	if(LOG_METHODS) cout << "MeControllerRequest::schedule( startAt="<<startAt<<",  readyAt="<<readyAt<<",  strokeAt="<<strokeAt<<",  relaxAt="<<relaxAt<<",  endAt="<<endAt<<" )"<<endl;

	switch( trackType ) {
        case UTTERANCE: {
			// input args designed for this case
			break;
        }
        case POSTURE: {
			// select posture track and adjust times accordingly
			scheduler = actor->posture_sched_p;
            //  Posture commands are relative to the start of execution
            double time = mcu->time;
			startAt  += time;
            readyAt  += time;
            strokeAt += time;
            relaxAt  += time;
            endAt    += time;
			break;
        }
		default: {
			throw BML::Processor::BodyPlannerException( "MeControllerRequest::schedule(..): Invalid trackType" );
		}
	}

	time_sec indt  = readyAt-startAt;
	time_sec outdt = endAt-relaxAt;

	if(LOG_SCHEDULING) cout << "MeControllerRequest::schedule(..): \""<<(controller->name())<<"\" startAt="<<startAt<<",  indt="<<indt<<",  outdt="<<outdt<<endl;
	scheduler->schedule( controller, (double)startAt, (float)indt, (float)outdt );
	// TODO: Adapt speed and end time

	////  Old-style MeCtScheduler2 API calls
	//scheduler->schedule( controller, (double)startAt, (float)indt, (float)outdt, MeCtScheduler::Once );
	//
    //scheduler->toptrack().tout  = (double)endAt;
    //scheduler->toptrack().speed = (float)speed;
}


//  MotionRequest
MotionRequest::MotionRequest( MeCtMotion* motion,
						      const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : MeControllerRequest( MeControllerRequest::UTTERANCE, motion,
                         start, ready, stroke, relax, end )
{
    readyTime  = motion->indt();
    strokeTime = motion->emphasist();
    relaxTime  = (time_sec)(motion->controller_duration()-motion->outdt());
}


//  NodRequest
NodRequest::NodRequest( NodType type, float repeats, float frequency, float extent, const SbmCharacter* actor,
			            const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : MeControllerRequest( MeControllerRequest::UTTERANCE, new MeCtSimpleNod(),
                         start, ready, stroke, relax, end ),
    type(type), repeats(repeats), frequency(frequency), extent(extent)
{
    endTime = time_sec( repeats / frequency );

    if( repeats >= 0.5 ) {                // Has a first valley
        relaxTime = time_sec( ((floor((repeats-.25)/.5)*.5)+.25)/frequency );  // relax @ last local extreme

        if( repeats >= 1 ) {
            readyTime = time_sec( 0.2/frequency );   // ready just before first local extreme
            strokeTime = time_sec( 0.75/frequency ); // stroke @ second local extreme
        } else {
            readyTime = time_sec( 0.1/frequency );   // ready just before first local extreme
            strokeTime = time_sec( 0.25/frequency ); // stroke @ local extreme
        }
    } else {
        strokeTime = endTime/2;
        readyTime = min( 0.1/frequency, strokeTime );
        relaxTime = endTime-readyTime;
    }

    if( extent > 1 )
        extent = 1;
    else if( extent < -1 )
        extent = -1;
    
    MeCtSimpleNod* nod = (MeCtSimpleNod*)controller;
//    nod->init( actor->skeleton_p );
    nod->init();
    //  TODO: Set a controller name
    switch( type ) {
        case VERTICAL:
            nod->set_nod( (float)endTime, extent*60, repeats, true );  // TODO: adjust offset to not look so high
            break;
        case HORIZONTAL:
            nod->set_nod( (float)endTime, extent*90, repeats, false );
            break;
        default:
            clog << "WARNING: NodRequest::NodRequest(..): Unknown nod type=" << type << endl;
    }
}


//  TiltRequest
TiltRequest::TiltRequest( MeCtSimpleTilt* tilt, time_sec transitionDuration,
						  const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : MeControllerRequest( MeControllerRequest::UTTERANCE, tilt, start, ready, stroke, relax, end ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/, transitionDuration(transitionDuration)
{
    readyTime = strokeTime = transitionDuration;
    relaxTime = endTime - transitionDuration;
}

//  PostureRequest
PostureRequest::PostureRequest( MeController* pose, time_sec transitionDuration,
						        const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : MeControllerRequest( MeControllerRequest::POSTURE, pose, start, ready, stroke, relax, end ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/, transitionDuration(transitionDuration)
{
    readyTime = strokeTime = transitionDuration;
    relaxTime = endTime - transitionDuration;
}

//  VisemeRequest
//    (no transition/blend yet)
VisemeRequest::VisemeRequest( const char *viseme, float weight, time_sec duration,
                              const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : BehaviorRequest( start, ready, stroke, relax, end,
                    /* start, stroke, and ready time */ 0, 0, 0,
					/* relax and end time */ duration, duration,
					/* speed */ 1 ),
    viseme(viseme), weight(weight), duration(duration)
{}

VisemeRequest::VisemeRequest( const char *viseme, float weight, time_sec duration,
                              const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end, float rampup, float rampdown )
  : BehaviorRequest( start, ready, stroke, relax, end,
                    /* start, stroke, and ready time */ 0, 0, 0,
					/* relax and end time */ duration, duration,
					/* speed */ 1 ),
    viseme(viseme), weight(weight), duration(duration), rampup(rampup), rampdown(rampdown)
{}


void VisemeRequest::setVisemeName( const char* viseme ) {
    this->viseme = viseme;
}

void VisemeRequest::schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
                              VecOfVisemeData& visemes,
                              VecOfSbmCommand& commands,
							  double startAt, double readyAt, double strokeAt, double relaxAt, double endAt )
{
    // TODO: BUG: these my be shifted by audioOffset in BodyPlannerImpl::vrSpeakSeq
	if( rampup < 0 ) {
		visemes.push_back( new VisemeData( viseme, float(weight), float(startAt), float(readyAt-startAt) ) );
	} else {
		visemes.push_back( new VisemeData( viseme, float(weight), float(startAt), rampup ) );
	}

	if( rampdown < 0 ) {
		visemes.push_back( new VisemeData( viseme, 0,             float(endAt), float(endAt-relaxAt) ) );
	} else {
		visemes.push_back( new VisemeData( viseme, 0,             float(endAt), rampdown ) );
	}
}


//  EventRequest
EventRequest::EventRequest( const char* message,
                            const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end )
  : BehaviorRequest( start, ready, stroke, relax, end,
                    /* start, stroke, and ready time */ 0, 0, 0,
					/* relax and end time */ 0, 0,
					/* speed */ 1 ),
	message( message )
{}

void EventRequest::schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
                             VecOfVisemeData& visemes,
                             VecOfSbmCommand& commands,
							 double startAt, double readyAt, double strokeAt, double relaxAt, double endAt )
{
	ostringstream out;
	out << "send " << message;
	commands.push_back( new SbmCommand( out.str(), (float)strokeAt ) );
}



//  SpeechRequest
//    (no transition/blend yet)
SpeechRequest::SpeechRequest( DOMElement* xml, const XMLCh* id, BmlRequestPtr request )
:	xml( xml ),
	id( id? new XMLCh[ XMLString::stringLen(id)+1 ] : NULL )
{
	trigger = request->createTrigger("SPEECH");  // TODO: Use trigger to signal speech timing is available

	if( id ) {
		start = trigger->addSynchPoint( buildBmlId( id, TM_START ), SynchPointPtr() );
		ready = trigger->addSynchPoint( buildBmlId( id, TM_READY ), start );
		relax = trigger->addSynchPoint( buildBmlId( id, TM_RELAX ), ready );
		end = trigger->addSynchPoint( buildBmlId( id, TM_END ), relax );

		XMLString::copyString( const_cast<XMLCh*>(this->id), id );

		// Register synch points
		request->synch_points.insert( make_pair( buildBmlId( id, TM_START ),  start ) );
		request->synch_points.insert( make_pair( buildBmlId( id, TM_READY ),  ready ) );
		request->synch_points.insert( make_pair( buildBmlId( id, TM_STROKE ), ready ) );
		request->synch_points.insert( make_pair( buildBmlId( id, TM_RELAX ),  relax ) );
		request->synch_points.insert( make_pair( buildBmlId( id, TM_END ),    end ) );

		SynchPointPtr lastSp( ready );

		if( XMLString::stringLen( id ) ) {  // if id not empty string
			// Parse <speech> for synch points
			const XMLCh* type = xml->getAttribute( ATTR_TYPE );
			if( type ) {
				if( XMLString::compareString( type, VALUE_TEXT_PLAIN )==0 ) {
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" << VALUE_TEXT_PLAIN << "\">" << endl;
					// Search for <tm> synch_points
					DOMElement* child = xml_utils::getFirstChildElement( xml );
					while( child!=NULL ) {
						const XMLCh* tag = child->getTagName();
						if( tag && XMLString::compareString( tag, TAG_TM )==0 ) {
							if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <tm>" << endl;

							const XMLCh* tmId = child->getAttribute( ATTR_ID );
							// test validity?
							if( XMLString::stringLen( tmId ) ) {
								if( isValidTmId( tmId ) ) {
									// Make fully qualified id
									tmId = BML::buildBmlId( id, tmId );
									child->setAttribute( ATTR_ID, tmId );
									// Create a SynchPoint
									lastSp = trigger->addSynchPoint( tmId, lastSp );
									request->synch_points.insert( make_pair( tmId, lastSp ) );
								} else {
									wcerr << "ERROR: Invalid <tm> id=\"" << tmId << "\"" << endl;
								}
							}
						}
						child = xml_utils::getNextElement( child );
					}
				} else if( XMLString::compareString( type, VALUE_SSML )==0 ) {
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" << VALUE_SSML << "\">" << endl;
					// Search for <mark> synch_points
					DOMElement* child = xml_utils::getFirstChildElement( xml );
					while( child!=NULL ) {
						const XMLCh* tag = child->getTagName();
						if( tag && XMLString::compareString( tag, TAG_MARK )==0 ) {
							if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <mark>" << endl;

							const XMLCh* tmId = child->getAttribute( ATTR_NAME );
							// test validity?
							if( XMLString::stringLen( tmId ) ) {
								if( isValidTmId( tmId ) ) {
									// Make fully qualified
									tmId = BML::buildBmlId( id, tmId );
									child->setAttribute( ATTR_NAME, tmId );
									// Create a SynchPoint
									lastSp = trigger->addSynchPoint( tmId, lastSp );
									request->synch_points.insert( make_pair( tmId, lastSp ) );
								} else {
									wcerr << "ERROR: Invalid <mark> name=\"" << tmId << "\"" << endl;
								}
							}
						}
						child = xml_utils::getNextElement( child );
					}
				} else {
					wcerr << "ERROR: SpeechRequest::SpeechRequest(..): Unrecognized speech behavior type=\"" << type << "\"" << endl;
				}
			} else {
				cerr << "ERROR: SpeechRequest::SpeechRequest(..): Speech behavior lacks type attribute" << endl;
			}
		} else if( LOG_SPEECH ) {
			cout << "WARNING: SpeechRequest::SpeechRequest(..): <speech> with no id"<<endl;
		}
	}
	////  Moved outside the constructor
	//if( trigger ) {
	//	request->speech_request.reset( this );
	//}
}

SpeechRequest::~SpeechRequest() {
	if( id ) {
		delete [] id;
	}
}

/*
SynchPoint* SpeechRequest::addMark( const XMLCh* markId ) {
	map< const XMLCh*, SynchPoint*, xml_utils::XMLStringCmp >& synch_points = trigger->request->synch_points;
	const XMLCh* tmId = buildBmlId( id, markId );

	if( synch_points.find( tmId ) == synch_points.end() ) {
		// id doesn't exist.. go ahead
		SynchPoint* sp = new SynchPoint( buildBmlId( id, markId ),
			                             trigger, relax->prev ); // append before relax
		synch_points.insert( make_pair( tmId, sp ) );
		return sp;
	} else {
		delete [] tmId;
		return NULL;
	}
}
*/

SynchPointPtr SpeechRequest::getMark( const XMLCh* markId ) {
	// TODO: overhaul this method.
	//       SpeechRequest should have its own map of this behavior's synch points

	BmlRequestPtr request( trigger->request.lock() );
	if( !request )  // Is BmlRequest still valid?
		return SynchPointPtr(); // NULL


	MapOfSynchPoint&          synch_points = request->synch_points;
	MapOfSynchPoint::iterator i;

	if( XMLString::indexOf( markId, ':' )==-1 ) {
		// no delimiter, not fully qualified id
		const XMLCh* tmId = buildBmlId( id, markId );

		i = synch_points.find( tmId );
		delete [] tmId;
	} else {
		// has delimiter, fully qualified id
		i = synch_points.find( markId );
	}

	if( i == synch_points.end() )
		return SynchPointPtr(); // NULL
	else
		return (*i).second;
}
