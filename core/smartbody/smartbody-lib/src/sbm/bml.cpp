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
#include <algorithm>

#include "mcontrol_util.h"
#include "bml_processor.hpp"
#include "bml_speech.hpp"

#include "ME/me_ct_blend.hpp"




using namespace std;
using namespace BML;
using namespace SmartBody;


const bool USE_CUSTOM_PRUNE_POLICY          = false; // Future feature

const bool LOG_BEHAVIOR_SYNCHPOINTS         = false;
const bool LOG_BML_BEHAVIOR_SCHEDULE        = false;
const bool LOG_METHODS						= false;
const bool LOG_CONTROLLER_SCHEDULE			= false;
const bool LOG_ABNORMAL_SPEED				= false;
const bool LOG_REQUEST_REALIZE_TIME_SPAN	= false;

// XML Constants
const XMLCh ATTR_ID[]       = L"id";
const XMLCh ATTR_TYPE[]     = L"type";
const XMLCh ATTR_NAME[]     = L"name";



///////////////////////////////////////////////////////////////////////////
//  Helper Functions
std::wstring BML::buildBmlId( const std::wstring&  behavior_id, const std::wstring& sync_id ) {
	typedef wstring::size_type size_type;
	const size_type npos = wstring::npos;

	if( behavior_id.empty() || sync_id.empty() )
		return wstring(); // no id

	if( sync_id.find( L':' ) != npos )
		return wstring( sync_id ); // copy

	wstringstream out;
	out << behavior_id << ':' << sync_id;
	return out.str();
}

bool BML::isValidBmlId( const std::wstring& id ) {
	typedef wstring::size_type size_type;

	// String must have length and begin with alpha or '_' (like a C/C++ identifier)
	if( id.empty() )
		return false;
	XMLCh c = id[0];

	bool result = XMLString::isAlpha( c ) || c=='_';

	if( result ) {
		// Remaining characters can also be digits
		size_type len = id.length();
		for( size_type i=1; result && i< len; ++i ) {
			c = id[i];
			result = XMLString::isAlphaNum( c ) && c != '_';
		}
	}

	return result;
}

bool BML::isValidTmId( const std::wstring& id ) {
	typedef wstring::size_type size_type;

	if( !isValidBmlId( id ) )
		return false;

	//  TODO: effiency?
	return( id!=TM_START &&
		    id!=TM_READY &&
		    id!=TM_STROKE_START &&
		    id!=TM_STROKE &&
		    id!=TM_STROKE_END &&
		    id!=TM_RELAX &&
		    id!=TM_END );
}


///////////////////////////////////////////////////////////////////////////////
//  BML Processor Controller Prune Policy

class BmlProcPrunePolicy : public MePrunePolicy {
public:
	virtual bool shouldPrune( MeController* ct, MeController* parent ) {
		cout << "====================> BmlProcPrunePolicy: pruning " << ct->controller_type() << " \"" << ct->name() << " from parent " << parent->controller_type() << " \"" << parent->name() << '"' << endl;
		return true;
	};
};




///////////////////////////////////////////////////////////////////////////
//  Class Member Definitions
#if USE_RECIPIENT
BmlRequest::BmlRequest( const SbmCharacter* actor, const string & actorId, const string & requestId, const string & recipientId, const string & msgId )
:	actor( actor ),
	actorId( actorId ),
	recipientId( recipientId ),
#else
BmlRequest::BmlRequest( const SbmCharacter* actor, const string & actorId, const string & requestId, const string & msgId )
:	actor( actor ),
	actorId( actorId ),
#endif
	requestId( requestId ),
	msgId( msgId )
{}

void BmlRequest::init( BmlRequestPtr self ) {
	// TODO: Assert self.get() == this
	weak_ptr = self;

	const XMLCh* start_id = L"bml:start";
	const XMLCh* end_id   = L"bml:end";

	start_trigger = createTrigger( start_id );
	bml_start = start_trigger->addSyncPoint();

	idToSync.insert( make_pair( start_id, bml_start ) );

	//// bml:end SyncPoint removed until it can be better evaluated
	//bml_end.reset( start_trigger->addSyncPoint( end_id ) );
	//bml_end->init( bml_start );
	//
	//sync_points.insert( make_pair( end_id, bml_end ) );


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
	//	first = new SyncPoint( L"act:start", triggers[0], NULL );
	//	first->next = triggers[0]->start;
	//	triggers[0]->start = first;
	//	first->time = 0.0001;
	//	last  = new SyncPoint( L"act:end", triggers[i-1], triggers[i-1]->end );
	//	triggers[i-1]->end = last;
	//	/*first = triggers[0]->start;
	//	last  = triggers[i-1]->end;*/
	//} else {
	//	//
	//	cout << "setting first and last!" << endl;
	//	first = new SyncPoint( L"act:start", NULL, NULL );
	//	first->time = 0.0001;
	//	last  = new SyncPoint( L"act:end", NULL, first );
	//	first->next = last;
	//	sync_points.insert(make_pair(L"act:start", first));
	//	sync_points.insert(make_pair(L"act:end", last));
	//}
}

BmlRequest::~BmlRequest() {
//// BehaviorRequests and SpeechRequest references are via SmartPointers.  Do nothing.
//	// delete BehaviorRequests
//	size_t count = behaviors.size();
//	for( size_t i=0; i<count; ++i ) {
//		delete behaviors[i];
//	}
//	if( speech_request != NULL ) {
//		speech_request.reset();
//	}

	// delete triggers
	size_t count = triggers.size();
	for( size_t i=0; i<count; i++ ) {
		triggers[ i ]->request.reset();  // remove ref
	}
	// The following were deleted in the above loop
	start_trigger.reset();
	bml_start.reset();
	//bml_end.reset;  // TODO: Temporarily removed
	speech_trigger.reset();
}


std::string BmlRequest::buildUniqueBehaviorId( const XMLCh* tag,
                                               const XMLCh* id,
											   size_t ordinal )
{
	ostringstream unique_id;
	char* ascii = NULL;

	unique_id << "BML_" << actorId << '_' << msgId;
	ascii = XMLString::transcode( tag );
	unique_id << "_#" << ordinal << "_<" << ascii << '>';
	XMLString::release( &ascii );

	if( XMLString::stringLen(id) > 0 ) {
		ascii = XMLString::transcode( id );
		unique_id << "_\"" << ascii << "\"";
		XMLString::release( &ascii );
	}

	// TODO: remove any whitespace
	return unique_id.str();
}


bool BmlRequest::hasExistingBehaviorId( const std::wstring& id ) {
	bool result = false;
	if( !id.empty() ) {
		MapOfBehaviorRequest::iterator pos = idToBehavior.find( id );
		result = ( pos != idToBehavior.end() );
	}

	return result;
}

void BmlRequest::importNamedSyncPoints( SyncPoints& syncs, const std::wstring& id, const std::wstring& logging_label ) {
	// Import named SyncPoints
	SetOfWstring names = syncs.get_sync_names();
	SetOfWstring::iterator it  = names.begin();
	SetOfWstring::iterator end = names.end();
	for( ; it!=end ; ++it ) {
		wstring& name = *it;
		SyncPointPtr sync( syncs.sync_for_name( name ) );

		wstring sync_id = buildBmlId( id, name );
		if( !sync_id.empty() ) {
			bool map_insert_success = idToSync.insert( make_pair( sync_id, sync ) ).second;

			if( !map_insert_success ) {
				wcerr << "ERROR: BmlRequest::registerBehavior(..): Failed to insert "<<logging_label<<" SyncPoint \""<<sync_id<<"\"." << endl;
			}
		}
	}
}

BehaviorSpan BmlRequest::getBehaviorSpan() {
	if( ! span.isSet() ) {
		if( speech_request ) {
			span.unionWith( speech_request->getBehaviorSpan() );
		}

		VecOfBehaviorRequest::iterator behav_it = behaviors.begin();
		VecOfBehaviorRequest::iterator behav_end = behaviors.end();
		for( ; behav_it != behav_end; ++behav_it ) {
			BehaviorRequestPtr behav = *behav_it;
			span.unionWith( behav->getBehaviorSpan() );
		}
	}

	return span;
}

void BML::BmlRequest::realize( Processor* bp, mcuCBHandle *mcu ) {
	// Self reference to pass on...
	BmlRequestPtr request = weak_ptr.lock();

	VecOfBehaviorRequest::iterator behav_end = behaviors.end();
	time_sec now = mcu->time;
	this->bml_start->time = now;

	if( LOG_BML_BEHAVIOR_SCHEDULE ) {
		cout << "DEBUG: BmlRequest::realize(): time = "<< (mcu->time) <<endl;
	}

	// Find earliest BehaviorRequest start time schedule before speech
	{
		time_sec min_time = numeric_limits<time_sec>::max();
		for( VecOfBehaviorRequest::iterator i = behaviors.begin(); i != behav_end;  ++i ) {
			BehaviorRequestPtr behavior = *i;
			try {
				behavior->schedule( now );
#if VALIDATE_BEHAVIOR_SYNCS
				behavior->syncs.validate();
#endif // VALIDATE_BEHAVIOR_SYNCS

				min_time = min( min_time, behavior->syncs.sp_start->time );

				if( LOG_BML_BEHAVIOR_SCHEDULE ) {
					cout << "DEBUG: BmlRequest::realize(): Behavior \""<< (behavior->unique_id) <<"\" SyncPoints:"<<endl;
					behavior->syncs.printSyncTimes();
				}
			} catch( BML::SchedulingException& e ) {
				// TODO: test if behavior is required
				ostringstream error_msg;
				error_msg << "BehaviorRequest \""<<behavior->unique_id<<"\" SchedulingException: "<<e.what();

				throw BML::RealizingException( error_msg.str().c_str() );
			}
		}

		if( LOG_BML_BEHAVIOR_SCHEDULE ) {
			cout << "DEBUG: BmlRequest::realize(): min_time: "<<min_time<<endl;
		}

		// ...and offset everything to be positive (assumes times are only relative to each other, not wall time, etc.)
		// ignore differences less than TIME_DELTA
		if( min_time < now - TIME_DELTA ) {
			time_sec offset = now - min_time;
			if( LOG_BML_BEHAVIOR_SCHEDULE ) {
				cout << "DEBUG: BmlRequest::realize(): offset: "<<offset<<endl;
			}

			for( VecOfBehaviorRequest::iterator i = behaviors.begin(); i != behav_end;  ++i ) {
				BehaviorRequestPtr behavior = *i;
				
				VecOfSyncPoint::iterator syncs_end = behavior->syncs.end();
				for( VecOfSyncPoint::iterator j = behavior->syncs.begin(); j != syncs_end; ++j ) {
					(*j)->time += offset;
				}
			}
		}
	}

	BehaviorSpan span = getBehaviorSpan();
	if( LOG_REQUEST_REALIZE_TIME_SPAN )
		cout << "DEBUG: BML::BmlRequest::realize(..): "<< actorId<<" BML \""<<msgId<<"\": time = "<<mcu->time<<"; span = "<<span.start<<" to "<<span.end<<endl;
	time_sec start_time = span.isSet()? span.start : mcu->time;
	time_sec end_time   = span.isSet()? span.end : mcu->time;


	//  Schedule vrAgentBML start sequence
	// (Separate sequence to ensure it occurs before all behavior sequence events)
	srCmdSeq *start_seq = new srCmdSeq(); //sequence that holds the startup feedback
	{

	    ostringstream start_command;
#if USE_RECIPIENT
		start_command << "send vrAgentBML " << actorId << " " << recipientId << " " << msgId << " start";
#else
		start_command << "send vrAgentBML " << actorId << " " << msgId << " start";
#endif

		if( start_seq->insert( (float)start_time, (char*)(start_command.str().c_str()) )!=CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::realize(..): msgId=\""<<msgId<<"\": "<<
				"Failed to insert \""<<start_command<<"\" command."<<endl;
		}

		// Sechdule this sequence immediately, before behavior sequences are scheduled
		{
			ostringstream oss;
			oss << actorId << ':' << msgId << ":seq-start";
			start_seq_name = oss.str();
		}
		if( mcu->execute_seq( start_seq, start_seq_name.c_str() ) != CMD_SUCCESS ) {
			ostringstream oss;
			oss << "Failed to execute BmlRequest sequence \""<<start_seq_name<<"\" (actorId=\""<< actorId << "\", msgId=\"" << msgId << "\")"; 
			throw RealizingException( oss.str().c_str() );
		}
	}

	// Realize behaviors
#if USE_CUSTOM_PRUNE_POLICY
	MePrunePolicy* prune_policy = new BmlProcPrunPolicy(); // TODO
#endif
	for( VecOfBehaviorRequest::iterator i = behaviors.begin(); i != behav_end;  ++i ) {
		BehaviorRequestPtr behavior = *i;

		behavior->realize( request, mcu );

#if USE_CUSTOM_PRUNE_POLICY
		if( behavior->has_cts() ) {
			behavior->registerControllerPrunePolicy( prune_policy );
		}
#endif
	}

	//  Schedule cleanup sequence, including vrAgentBML end
	// (Separate sequence to ensure it occurs after all behavior sequence events)
	srCmdSeq *cleanup_seq = new srCmdSeq(); //sequence that holds the feedback and cleanup commands

	//  Schedule vrAgentBML end
	{
		ostringstream end_command;

#if USE_RECIPIENT
		end_command << "send vrAgentBML " << actorId << " " << recipientId << " " << msgId << " end complete";
#else
		end_command << "send vrAgentBML " << actorId << " " << msgId << " end complete";
#endif
		if( span.persistent )
			end_command << " persistent";

		if( cleanup_seq->insert( (float)end_time, (char*)(end_command.str().c_str()) )!=CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::realize(..): msgId=\""<<msgId<<"\": "<<
				"Failed to insert \""<<end_command<<"\" command."<<endl;
		}
	}


#if 0	// TODO: Reimplement logging.  (Removed name from SyncPoint)
	//  Add Logging / Debugging commands to sequence
	if( log_syncpoints ) {
		ostringstream oss;
		oss << "echo ===\tAgent: " << bpMsg.actorId << "\tMsgId: " << bpMsg.msgId << "\tSyncPoint: ";
		string command_prefix = oss.str();
	
		MapOfSyncPoint::iterator i                = request->idToSync.begin();
		MapOfSyncPoint::iterator sync_points_end = request->idToSync.end();
	
		for(; i!=sync_points_end; ++i ) {
			SyncPointPtr sp( i->second );
			const char* ascii = XMLString::transcode( sp->name.c_str() );
			string name( ascii );
			delete []ascii;
	
			BML::time_sec time = sp->time;
			if( isTimeSet( time ) ) {
				BML::time_sec seqTime = time;
				if( seqTime < 0 )
					seqTime = 0;
	
				oss.str("");  // clear the buffer
				oss << command_prefix << name << "\t@ " << time;
				if( seq->insert( (float)seqTime, (char*)(oss.str().c_str()) )!=CMD_SUCCESS ) {
					cerr << "WARNING: BML::BmlRequest::realize(..): msgId=\""<<bpMsg.msgId<<"\": "<<
						"Failed to insert echo timesync_point message"<<endl;
				}
			}
		}
	}
#endif

	if( bp->get_auto_print_controllers() ) {
		ostringstream oss;
		oss << "print character "<< actorId << " schedule";
		string& cmd = oss.str();
		if( cleanup_seq->insert( 0, (char*)(cmd.c_str()) )!=CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::realize(..): msgId=\""<<msgId<<"\": "<<
				"Failed to insert \"" << cmd << "\" command"<<endl;
		}
	}

	{
		ostringstream oss;
		oss << actorId << ':' << msgId << ":seq-cleanup";
		cleanup_seq_name = oss.str();
	}
	if( mcu->execute_seq( cleanup_seq, cleanup_seq_name.c_str() ) != CMD_SUCCESS ) {
		ostringstream oss;
		oss << "Failed to execute BmlRequest sequence \""<<cleanup_seq_name<<"\" (actorId=\""<< actorId << "\", msgId=\"" << msgId << "\")"; 
		throw RealizingException( oss.str().c_str() );
	}


	if( bp->get_auto_print_sequence() ) {
		cout << "DEBUG: BML::BmlRequest::realize(..): Sequence \"" << start_seq_name <<"\":"<<endl;
		start_seq->print();

		cout << "DEBUG: BML::BmlRequest::realize(..): Sequence \"" << cleanup_seq_name <<"\":"<<endl;
		cleanup_seq->print();
	}
}

void BmlRequest::unschedule( mcuCBHandle* mcu,
                             time_sec duration )
{
	BmlRequestPtr request = weak_ptr.lock(); // Ref to this

	if( speech_request ) {
		speech_request->unschedule( mcu, request, duration );
	}

	VecOfBehaviorRequest::iterator it  = behaviors.begin();
	VecOfBehaviorRequest::iterator end = behaviors.end();
	for( ; it != end; ++it ) {
		BehaviorRequestPtr behavior = *it;
		behavior->unschedule( mcu, request, duration );
	}

	// Cancel the normal "vrAgentBML ... end complete"
	mcu->abort_seq( cleanup_seq_name.c_str() ); // don't clean-up self

	// Replace it with  "vrAgentBML ... end interrupted"
	ostringstream buff;
#if USE_RECIPIENT
	buff << request->actorId << " " << request->recipientId << " " << request->msgId << " end interrupted";
#else
	buff << request->actorId << " " << request->msgId << " end interrupted";
#endif
	mcu->vhmsg_send( "vrAgentBML", buff.str().c_str() );
}


void BmlRequest::cleanup( mcuCBHandle* mcu )
{
	BmlRequestPtr request = weak_ptr.lock(); // Ref to this

	if( speech_request ) {
		speech_request->cleanup( mcu, request );
	}

	bool has_controllers = false;
	VecOfBehaviorRequest::iterator it = behaviors.begin();
	VecOfBehaviorRequest::iterator end = behaviors.end();
	for( ; it != end; ++it ) {
		BehaviorRequestPtr behavior = *it;

		has_controllers = behavior->has_cts();
		behavior->cleanup( mcu, request );
	}
	if( has_controllers ) {
		// Schedule a prune command to clear them out later.
		string command( "char " );
		command += actorId;
		command += " prune";

		if( mcu->execute_later( command.c_str(), 1 ) != CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::cleanup(..): msgId=\""<<msgId<<"\": "<<
				"Failed to execute_later \""<<command<<"\"." << endl;
		}
	}
}



TriggerEventPtr BmlRequest::createTrigger( const wstring& name ) {
	TriggerEventPtr trigger( new TriggerEvent( name, weak_ptr.lock() ) );
	trigger->init( trigger );

	triggers.push_back( trigger );
	return trigger;
}

bool BmlRequest::registerBehavior( const std::wstring& id, BehaviorRequestPtr behavior ) {
	if( id.size() > 0 && hasExistingBehaviorId( id ) ) {
		wcerr <<  "ERROR: BmlRequest::registerBehavior(..): BehaviorRequest id \""<< id <<"\" is already in use!" << endl;
		return false; // duplicate id
	}

	span.unset();
	behaviors.push_back( behavior );

	if( id.size() > 0 ) {
		importNamedSyncPoints( behavior->syncs, id, L"BehaviorRequest" );
	}

	if( LOG_BEHAVIOR_SYNCHPOINTS ) {
		cout << "DEBUG: BmlRequest::registerBehavior(): SyncPoints for " << behavior->unique_id << flush;
		if( id.size()>0 )
			wcout << " \"" << id << "\"" << flush;
		cout << ":" << endl << "\t" << flush;

		behavior->syncs.printSyncIds();
	}

	return true;
}

//// TODO: Merge with above after SpeechRequest is a type of BehaviorRequest
//bool BmlRequest::registerBehavior( const std::wstring& id, SpeechRequestPtr speech ) {
//	if( speech_request ) {
//		wcerr <<  "ERROR: BmlRequest::registerBehavior(..): Only one SpeechRequest per BmlRequest (temporary limitation)." << endl;
//		return false;
//	}
//
//	if( hasExistingBehaviorId( id ) ) {
//		wcerr <<  "ERROR: BmlRequest::registerBehavior(..): SpeechRequest id \""<< (speech->id) <<"\" is already in use!" << endl;
//		return false; // duplicate id
//	}
//
//	speech_request = speech;
//
//	importNamedSyncPoints( speech->syncs, id, L"SpeechRequest" );
//
//	return true;
//}



SyncPointPtr BmlRequest::getSyncPoint( const std::wstring& notation ) {
	typedef wstring::size_type size_type;
	const size_type npos = wstring::npos;


	SyncPointPtr sync;  // result

	// Get index to last '+' or '-' character
	size_type last_plus = notation.rfind('+');
	size_type last_minus = notation.rfind('-');
	size_type index = std::max( last_plus, last_minus );

	if( index != npos && index != 0 ) { //check for offset
		float offset;
		wstring offset_str( notation.substr( index ) );
		wistringstream offset_reader( offset_str );
		if( offset_reader >> offset ) {
			wstring key( notation.substr( 0, index ) );
			/*if (XMLString::compareString( key, L"act:start" )==0 && offset < 0) {
				wcerr<<"WARNING: BmlRequest::getSyncPoint: BML offset \""<< name<<"\" is negative with regard to  act:start, offset set to 0.0!" << endl;
				offset = 0;
			}*/

			MapOfSyncPoint::iterator mySearchIter = idToSync.find(key);
			if( mySearchIter == idToSync.end() ) {
				wcerr<<"WARNING: BmlRequest::getSyncPoint: BML offset refers to unknown "<<key<<" point.  Ignoring..."<<endl;
			} else {
				SyncPointPtr parent = mySearchIter->second;
				if( parent ) {
					TriggerEventPtr trigger = parent->trigger.lock();
					if( trigger ) {
						sync = trigger->addSyncPoint( parent, offset );
						if( parent && !isTimeSet( parent->time ) ) {
							sync->time = parent->time + offset;
						}
					} else {
						wcerr << "WARNING: parent sync does not have a valid trigger" << endl;
					}
				} else {
					wcerr << "ERROR: Map returned invalid parent for key \"" << key << "\"" << endl;
				}
			}
		} else {
			wcerr << "ERROR: Invalid offset \""<<offset_str<<"\" in notation \"" << notation << "\"" << endl;
		}
	} else if( index==0 || notation.find(':')==npos ) {
		float offset;
		wistringstream offset_reader( notation );
		if( offset_reader >> offset ) {
			sync = start_trigger->addSyncPoint( bml_start, offset );
		} else {
			wcerr << "ERROR: Invalid SyncPoint numeric notation \""<<notation<<"\"." << endl;
		}
	} else {
		MapOfSyncPoint::iterator mySearchIter = idToSync.find(notation);
		if ( mySearchIter != idToSync.end()){
			SyncPointPtr parent = (*mySearchIter).second;
			sync.reset( new SyncPoint( parent->trigger.lock(), parent, 0 ) );
		} else {
			wcerr << "WARNING: Unknown sync for notation \"" << notation << "\"" << endl;
		}
	}

	return sync;  // May be NULL
}


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
SyncPoints::SyncPoints()
{}

SyncPoints::SyncPoints( const SyncPoints& other )
:	syncs( other.syncs ),
	idToSync( other.idToSync ),
	sp_start( other.sp_start ),
	sp_ready( other.sp_ready ),
	sp_stroke_start( other.sp_stroke_start ),
	sp_stroke( other.sp_stroke ),
	sp_stroke_end( other.sp_stroke_end ),
	sp_relax( other.sp_relax ),
	sp_end( other.sp_end )
{}

SyncPoints::iterator SyncPoints::insert( const wstring& id, SyncPointPtr sync, SyncPoints::iterator pos ) {
	MapOfSyncPoint::iterator map_it = idToSync.find( id );
	if( map_it != idToSync.end() )
		return end();

	if( !idToSync.insert( make_pair( id, sync ) ).second )
		return end();
	return syncs.insert( pos, sync );
}

SyncPoints::iterator SyncPoints::pos_of( SyncPointPtr sync ) {
	return find( begin(), end(), sync );
}

SetOfWstring SyncPoints::get_sync_names() {
	SetOfWstring names;

	MapOfSyncPoint::iterator it  = idToSync.begin();
	MapOfSyncPoint::iterator end = idToSync.end();
	for( ; it!=end; ++it ) {
		const wstring& name = it->first;
		if( !( names.insert( name ).second ) )
			wcerr << "ERROR: SyncPoints::get_sync_names(): Failed to insert SyncPoint name \""<<name<<"\"." << endl;
	}

	return names;
}

SyncPointPtr SyncPoints::sync_for_name( const std::wstring& name ) {
	MapOfSyncPoint::iterator result = idToSync.find( name );
	if( result != idToSync.end() )
		return result->second;
	else
		return SyncPointPtr();
}

BehaviorSpan SyncPoints::getBehaviorSpan( time_sec persistent_threshold ) {
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

void SyncPoints::parseStandardSyncPoints( DOMElement* elem, BmlRequestPtr request, const string& behavior_id ) {
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


SyncPointPtr SyncPoints::parseSyncPointAttr( DOMElement* elem, const std::wstring& elem_id, const std::wstring& sync_attr, const BmlRequestPtr request, const string& behavior_id ) {
	return parseSyncPointAttr( elem, elem_id, sync_attr, request, behavior_id, end() );
}

SyncPointPtr SyncPoints::parseSyncPointAttr( DOMElement* elem, const std::wstring& elem_id, const std::wstring& sync_attr, const BmlRequestPtr request, const string& behavior_id, iterator pos ) {
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
		wcerr << "ERROR: Behavior \""<<behavior_wid<<"\": SyncPoints contains SyncPoint with id \"" << sync_attr << "\".  Ignoring attribute in <"<<elem->getTagName();
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
			wcerr<<"WARNING: Behavior \""<<behavior_wid<<"\": SyncPoints::parseSyncPointAttr(..): <"<<(elem->getTagName())<<"> BML tag refers to unknown SyncPoint "<<sync_attr<<"=\""<<sync_ref<<"\".  Creating placeholder..."<<endl;
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

string SyncPoints::debug_label( SyncPointPtr& sync ) {
	ostringstream out;

	wstring id = idForSyncPoint( sync );

	int count = 0;
	iterator it  = syncs.begin();
	iterator end = syncs.end();
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

void SyncPoints::validate() {
	ostringstream out;
	out << "SyncPoints validation errors:";

	bool valid = true;

	iterator it  = syncs.begin();
	iterator end = syncs.end();
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

std::wstring SyncPoints::idForSyncPoint( SyncPointPtr sync ) {
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

void SyncPoints::applyParentTimes( std::wstring& warning_context ) {
	MapOfSyncPoint::iterator it  = idToSync.begin();
	MapOfSyncPoint::iterator end = idToSync.end();

	for( ; it!=end; ++it ) {
		const wstring& sync_id = it->first;
		SyncPointPtr  sync    = it->second;
		if( sync->parent ) {
			SyncPointPtr parent = sync->parent;
			if( isTimeSet( parent->time ) ) {
				if( isTimeSet( sync->time ) ) {
					wcerr << "WARNING: ";
					if( !warning_context.empty() )
						wcerr << warning_context << ": ";
					wcerr << "SyncPoint \""<<sync_id<<"\" time set before applyParentTimes()." << endl;
				}
				sync->time = parent->time + sync->offset;
			} else {
				// Parent not set!!
				wcerr << "WARNING: ";
				if( !warning_context.empty() )
					wcerr << warning_context << ": ";
				wcerr << "SyncPoint \""<<sync_id<<"\" parent unset." << endl;
			}
		}
	}
}

void SyncPoints::printSyncIds() {
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

void SyncPoints::printSyncTimes() {
	VecOfSyncPoint::iterator vec_it     = begin();
	VecOfSyncPoint::iterator vec_end    = end();

	// Iterate Syncs in order
	for( ; vec_it!=vec_end; ++vec_it ) {
		SyncPointPtr sync = (*vec_it);
		wstring sync_id = idForSyncPoint( sync );

		wcout << "\t" << sync_id << ": " << sync << endl;
	}
}



SbmCommand::SbmCommand( std::string & command, time_sec time )
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


const time_sec BehaviorRequest::PERSISTENCE_THRESHOLD = (time_sec)( MeCtScheduler2::MAX_TRACK_DURATION * 0.9 );


// methods
#if BEHAVIOR_TIMING_BY_DURATION
BehaviorRequest::BehaviorRequest( const std::string& unique_id, const SyncPoints& syncs,
                                  time_sec startReadyDur, time_sec readyStrokeDur, time_sec strokeRelaxDur, time_sec relaxEndDur, float speed )
:	syncs( syncs ),
	unique_id( unique_id ),
	audioOffset(TIME_UNSET),
	startReadyDur( min(startReadyDur,0.001) ), 
    readyStrokeDur( min(readyStrokeDur,0.001) ), 
    strokeRelaxDur( min(strokeRelaxDur,0.001) ),
    relaxEndDur( min(relaxEndDur,0.001) ),
    speed(speed)
{}
#else
BehaviorRequest::BehaviorRequest( const std::string& unique_id, const SyncPoints& syncs,
                                  time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime, float speed )
:	syncs( syncs ),
	unique_id( unique_id ),
	audioOffset(TIME_UNSET),
	startTime(startTime), 
    readyTime(readyTime), 
    strokeTime(strokeTime),
    relaxTime(relaxTime),
    endTime(endTime),
    speed(speed)
{}
#endif // BEHAVIOR_TIMING_BY_DURATION

BehaviorRequest::~BehaviorRequest() {
	// nothing to delete.  Yay for SmartPointers!
}

/** Tests ordering.  If invlaid, prints warning about ignoring SyncPoint before. */
bool testSyncBefore(
		SyncPointPtr& before, const char* before_name,
		SyncPointPtr& after, const char* after_name
) {
	if( before->time <= after->time ) {
		return true;
	} else {
        clog << "WARNING: BehaviorRequest::testSyncOrder(): "<<before_name<<" NOT before "<<after_name<<"... ignoring "<<before_name<<"." << endl;
		return false;
	}
}

/** Tests ordering.  If invlaid, prints warning about ignoring SyncPoint after. */
bool testSyncAfter(
		SyncPointPtr& before, const char* before_name,
		SyncPointPtr& after, const char* after_name
) {
	if( before->time <= after->time ) {
		return true;
	} else {
        clog << "WARNING: BehaviorRequest::testSyncOrder(): "<<before_name<<" NOT before "<<after_name<<"... ignoring "<<after_name<<"." << endl;
		return false;
	}
}


void BehaviorRequest::schedule( time_sec now ) {
	// local references to standard sync points
	SyncPointPtr start        = syncs.sp_start;
	SyncPointPtr ready        = syncs.sp_ready;
	SyncPointPtr stroke_start = syncs.sp_stroke_start;
	SyncPointPtr stroke       = syncs.sp_stroke;
	SyncPointPtr stroke_end   = syncs.sp_stroke_end;
	SyncPointPtr relax        = syncs.sp_relax;
	SyncPointPtr end          = syncs.sp_end;

	{
		XMLCh* wide_id = XMLString::transcode( unique_id.c_str() );
		wstringstream warning_context;
		warning_context << "Behavior \"" << wide_id << "\"";
		syncs.applyParentTimes( warning_context.str() );

		XMLString::release( &wide_id );
	}

	/*  The following implements a search for the two most important SyncPoints, and then scales the time to meet both.
     *  Importance is ranked in this order: stroke, ready, relax, start, end
     *  If only one SyncPoint is found, the controller maintains its natural duration.
     *  If no sync points are found, the behavior starts immediately.
     */
    bool hasStroke = isTimeSet( stroke->time );
    bool hasReady  = isTimeSet( ready->time );
    bool hasRelax  = isTimeSet( relax->time );
    bool hasStart  = isTimeSet( start->time );
    bool hasEnd    = isTimeSet( end->time );

	time_sec start_at = TIME_UNSET;


    if( hasStroke ) {  // Handle stroke first (most important)
        if(    hasReady
			&& testSyncBefore( ready, "ready", stroke, "stroke" )
		) 
		{
			// Stroke and compatible ready
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = stroke->time - ready->time;  // realtime duration
			time_sec bhvrDur  = readyStrokeDur;           // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - startReadyDur/speed;
#else
            if( readyTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before strokeTime... ignoring ready." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - ready->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasRelax
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( stroke, "stroke", relax, "relax" )
		)
		{
			// Stroke and compatible relax
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur   = relax->time - stroke->time;  // realtime duration
			time_sec bhvrDur = strokeRelaxDur;              // behavior duration

			speed = bhvrDur / rtDur;
			start_at = stroke->time - (startReadyDur+readyStrokeDur)/speed;
#else
            if( strokeTime >= relaxTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = relax->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasStart
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncBefore( start, "start", stroke, "stroke" )
		)
		{
			// Stroke and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur   = stroke->time - start->time;  // realtime duration
			time_sec bhvrDur = strokeRelaxDur;              // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before strokeTime... ignoring start." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - start->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif  // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( stroke, "stroke", end, "end" )
		)
		{
			// Stroke and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - stroke->time;      // realtime diff
			time_sec bhvrDur = strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = stroke->time - (startReadyDur+readyStrokeDur)/speed;
#else
            if( strokeTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): strokeTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = end->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = endTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only stroke
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = stroke->time - ( startReadyDur + readyStrokeDur );
#else
			start_at = stroke->time - strokeTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasReady ) {  // Next comes ready
		// Skipping stroke. Would have been caught at previous level.
        if(    hasRelax
			&& testSyncAfter( ready, "ready", relax, "relax" )
		)
		{
			// Ready and compatible relax
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( readyTime >= relaxTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): readyTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = relax->time - ready->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasStart
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncBefore( start, "start", ready, "ready" )
		)
		{
			// Ready and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= readyTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before readyTime... ignoring start." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = ready->time - start->time;  // realtime diff
				time_sec ctlrDiff = readyTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( ready, "ready", end, "end" )
		)
		{
			// Ready and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( readyTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): readyTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = end->time - ready->time;  // realtime diff
				time_sec ctlrDiff = endTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only ready
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = ready->time - startReadyDur;
#else
			start_at = ready->time - readyTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasRelax ) {  // Next comes relax
		// Skipping stroke and ready. Would have been caught at previous level.
        if(    hasStart
			&& testSyncBefore( start, "start", relax, "relax" )
		)
		{
			// Relax and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - start->time;                         // realtime diff
			time_sec bhvrDur = startReadyDur + readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= relaxTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before relaxTime... ignoring start." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = relax->time - start->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = relax->time-(relaxTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( relax, "relax", end, "end" )
		)
		{
			// Relax and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - relax->time; // realtime diff
			time_sec bhvrDur = relaxEndDur;           // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( relaxTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): relaxTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = end->time - relax->time;  // realtime diff
				time_sec ctlrDiff = endTime - relaxTime;
				speed = ctlrDiff / rtDiff;

				start_at = relax->time-(relaxTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only relax
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = relax->time - ( startReadyDur + readyStrokeDur + strokeRelaxDur );
#else
			start_at = relax->time - relaxTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasStart ) {  // Next comes start
		// Skipping stroke, ready and relax.  Would have been caught at previous level.
        if(    hasEnd
			&& testSyncAfter( start, "start", end, "end" )
		)
		{
			// Start and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - start->time;                                         // realtime diff
			time_sec bhvrDur = startReadyDur + readyStrokeDur + strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and start.
				time_sec rtDiff = end->time - start->time;  // realtime diff
				time_sec ctlrDiff = endTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = start->time - (startTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only start
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = start->time;
#else
			start_at = start->time - startTime;  // Strange....
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
	} else {
		speed = 1;  // End or nothing
		if( hasEnd ) {  // Last comes end
			// Only end
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = end->time - (startReadyDur + readyStrokeDur + strokeRelaxDur + relaxEndDur);
#else
			start_at = end->time - endTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		} else {
			// No sync_points... align to audio
			//clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): no valid time refences." << endl;
			start_at = now;
		}
	}

    // Offset all times by startAt-startTime and scale to previously determined speed
	if( !start->isSet() )
		start->time  = start_at;
#if BEHAVIOR_TIMING_BY_DURATION
	if( !ready->isSet() )
		ready->time  = start->time  + startReadyDur*speed;
	if( !stroke->isSet() )
		stroke->time = ready->time  + readyStrokeDur*speed;
	if( !relax->isSet() )
		relax->time  = stroke->time + strokeRelaxDur*speed;
	if( !end->isSet() )
		end->time    = relax->time  + relaxEndDur*speed;
#else
	if( !ready->isSet() )
		ready->time  = start_at + (readyTime-startTime)*speed;
	if( !stroke->isSet() )
		stroke->time = start_at + (strokeTime-startTime)*speed;
	if( !relax->isSet() )
		relax->time  = start_at + (relaxTime-startTime)*speed;
	if( !end->isSet() )
		end->time    = start_at + (endTime-startTime)*speed;
#endif // BEHAVIOR_TIMING_BY_DURATION

	// TODO: validate times are set and in order

	if( LOG_ABNORMAL_SPEED ) {
		if( speed > 2 )
			clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end sync constraint." << endl;
		else if( speed < 0.3 )
			clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end sync constraint." << endl;
	}
}

void BehaviorRequest::realize( BmlRequestPtr request, mcuCBHandle* mcu ) {
	realize_impl( request, mcu );
}


bool BehaviorRequest::isPersistent() {
	time_sec duration = (syncs.sp_end->time) - (syncs.sp_start->time);
	return( duration > PERSISTENCE_THRESHOLD );
}



BehaviorSpan BehaviorRequest::getBehaviorSpan() {
	// Default algorithm for detecting persistent behaviors.
	BehaviorSpan span = syncs.getBehaviorSpan( PERSISTENCE_THRESHOLD );

	return span;
}


//  MeControllerRequest
MeControllerRequest::MeControllerRequest( const std::string& unique_id,
										  MeController* anim_ct, MeCtSchedulerClass* schedule_ct, bool is_persistent,
						                  const SyncPoints& syncs_in )
:	BehaviorRequest( unique_id, syncs_in,
#if BEHAVIOR_TIMING_BY_DURATION
	                 /* startReadyDur  */ time_sec( anim_ct->indt() ), 
	                 /* readyStrokeDur */ time_sec( anim_ct->emphasist() - anim_ct->indt() ), 
	                 /* strokeRelaxDur */ time_sec( anim_ct->controller_duration() - anim_ct->emphasist() - anim_ct->outdt() ),
	                 /* relaxEndDur    */ time_sec( anim_ct->outdt() ),
#else
	                 /* startTime  */ 0, 
	                 /* readyTime  */ time_sec( anim_ct->indt() ), 
	                 /* strokeTime */ time_sec( anim_ct->emphasist() ),
	                 /* relaxTime  */ time_sec( anim_ct->controller_duration() - anim_ct->outdt() ), 
	                 /* endTime    */ time_sec( anim_ct->controller_duration() ),
#endif  // BEHAVIOR_TIMING_BY_DURATION
	                 /* speed      */ 1 ),
    anim_ct(anim_ct),
	schedule_ct( schedule_ct )
{
	anim_ct->ref();
	schedule_ct->ref();

    if( anim_ct->controller_duration() < 0 ) {
        relaxTime = endTime = numeric_limits<time_sec>::max();
    }
}

MeControllerRequest::~MeControllerRequest() {
	if( schedule_ct ) {
		schedule_ct->unref();
		schedule_ct = NULL;
	}
	if( anim_ct ) {
		anim_ct->unref();
		anim_ct = NULL;
	}
}


void MeControllerRequest::registerControllerPrunePolicy( MePrunePolicy* prune_policy ) {
	if( anim_ct != NULL ) {
		anim_ct->prune_policy( prune_policy );
	}
}

void MeControllerRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu ) {
	// Get times from SyncPoints
	time_sec startAt  = syncs.sp_start->time;
	time_sec readyAt  = syncs.sp_ready->time;
	time_sec strokeAt = syncs.sp_stroke->time;
	time_sec relaxAt  = syncs.sp_relax->time;
	time_sec endAt    = syncs.sp_end->time;

	if( LOG_METHODS || LOG_CONTROLLER_SCHEDULE ) {
		cout << "DEBUG: MeControllerRequest::schedule(): startAt="<<startAt<<",  readyAt="<<readyAt<<",  strokeAt="<<strokeAt<<",  relaxAt="<<relaxAt<<",  endAt="<<endAt<<endl;
	}

	time_sec indt  = readyAt-startAt;
	time_sec outdt = endAt-relaxAt;

	// Name unnamed controllers
	const char* name = anim_ct->name();
	if( name==NULL || name[0]=='\0' ) {
		anim_ct->name( name = unique_id.c_str() );
	}

	if(LOG_CONTROLLER_SCHEDULE) {
		cout << "MeControllerRequest::schedule(..): \""<<(anim_ct->name())<<"\" startAt="<<startAt<<",  indt="<<indt<<",  outdt="<<outdt<<endl;
	}
	track = schedule_ct->schedule( anim_ct, (double)startAt, (float)indt, (float)outdt );
	// TODO: Adapt speed and end time

	////  Old-style MeCtScheduler2 API calls
	//schedule_ct->schedule( anim_ct, (double)startAt, (float)indt, (float)outdt, MeCtScheduler::Once );
	//
    //schedule_ct->toptrack().tout  = (double)endAt;
    //schedule_ct->toptrack().speed = (float)speed;

	// TODO: set sync point times
}

/**
 *  Implemtents BehaviorRequest::unschedule(..),
 *  ramping down the blend curve of the MeController.
 */
void MeControllerRequest::unschedule( mcuCBHandle* mcu,
                                      BmlRequestPtr request,
                                      time_sec duration )
{
	if( anim_ct && schedule_ct ) {
		MeCtScheduler2::track_iterator it = schedule_ct->track_for_anim_ct( anim_ct );
		if( it != schedule_ct->end() ) {
			MeCtScheduler2::Track& track = *it;

			MeCtUnary* unary_blend_ct = track.blending_ct();
			if( unary_blend_ct &&
				unary_blend_ct->controller_type() == MeCtBlend::CONTROLLER_TYPE )
			{
				MeCtBlend* blend = static_cast<MeCtBlend*>(unary_blend_ct);
				MeSpline1D& spline = blend->blend_curve();
				
				MeSpline1D::domain time = mcu->time;
				MeSpline1D::range  y = spline.eval( time );
				MeSpline1D::range  slope = -y / duration;

				spline.erase_after( time );
				if( duration > 0 ) {
					spline.make_smooth( time, y, slope, 1, 1 );
					spline.make_smooth( time, 0, slope, 1, 1 );
				} else {
					spline.make_disjoint( time, 0, y, 0, 1, 0, 1 );
				}
			}
		}
	}
	is_persistent = false;
}

/**
 *  Implemtents BehaviorRequest::cleanup(..),
 *  removing the MeController from its parent.
 */
void MeControllerRequest::cleanup( mcuCBHandle* mcu, BmlRequestPtr request )
{
	if( schedule_ct ) {
		if( !is_persistent ) {
			// TODO: is track valid?
			schedule_ct->remove_track( track );
		}
		schedule_ct->unref();
		schedule_ct = NULL;
	}
	if( anim_ct ) {
		anim_ct->unref();
		anim_ct = NULL;
	}
}



//  MotionRequest
MotionRequest::MotionRequest( const std::string& unique_id, MeCtMotion* motion_ct, MeCtSchedulerClass* schedule_ct,
						      const SyncPoints& syncs_in )
  : MeControllerRequest( unique_id, motion_ct, schedule_ct, motion_ct->loop(),
                         syncs_in )
{
    readyTime  = motion_ct->indt();
    strokeTime = motion_ct->emphasist();
    relaxTime  = (time_sec)(motion_ct->controller_duration()-motion_ct->outdt());
}



//  NodRequest
NodRequest::NodRequest( const std::string& unique_id, NodType type, float repeats, float frequency, float extent, const SbmCharacter* actor,
			            const SyncPoints& syncs_in )
:	MeControllerRequest( unique_id, new MeCtSimpleNod(), actor->head_sched_p, false,
                         syncs_in ),
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
    
    MeCtSimpleNod* nod = (MeCtSimpleNod*)anim_ct;
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
TiltRequest::TiltRequest( const std::string& unique_id, MeCtSimpleTilt* tilt, time_sec transitionDuration,
						  const SbmCharacter* actor,
						  const SyncPoints& syncs_in )
:	MeControllerRequest( unique_id, tilt, actor->head_sched_p, false, syncs_in ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/, transitionDuration(transitionDuration)
{
    readyTime = strokeTime = transitionDuration;
    relaxTime = endTime - transitionDuration;
}

//  PostureRequest
PostureRequest::PostureRequest( const std::string& unique_id, MeController* pose, time_sec transitionDuration, const SbmCharacter* actor,
						        const SyncPoints& syncs_in )
:	MeControllerRequest( unique_id, pose, actor->posture_sched_p, true, syncs_in ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/, transitionDuration(transitionDuration)
{
    readyTime = strokeTime = transitionDuration;
    relaxTime = endTime - transitionDuration;
}

// SequenceRequest
SequenceRequest::SequenceRequest( const std::string& unique_id, const SyncPoints& syncs_in,
                                  time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime )
:	BehaviorRequest( unique_id, syncs_in,
	                 startTime, readyTime, strokeTime, relaxTime, endTime,
					 1 )
{}

/**
 *  Implemtents BehaviorRequest::unschedule(..),
 *  cancelling remaining sequence.
 */
void SequenceRequest::unschedule( mcuCBHandle* mcu, BmlRequestPtr request,
                                  time_sec duration )
{
	unschedule_sequence( mcu );
}

/**
 *  Implemtents BehaviorRequest::cleanup(..),
 *  removing the sequence.
 */
void SequenceRequest::cleanup( mcuCBHandle* mcu, BmlRequestPtr request )
{
	unschedule_sequence( mcu );
}

bool SequenceRequest::realize_sequence( VecOfSbmCommand& commands, mcuCBHandle* mcu )
{
	if( commands.empty() ) {
		return true;
	}

	if( mcu->active_seq_map.lookup( unique_id.c_str() ) ) {
		cerr << "ERROR: SequenceRequest::realize_sequence(..): SequenceRequest \"" << unique_id << "\": "<<
		        "Sequence with matching ID already exists." << endl;
		return false;
	}

	srCmdSeq *seq = new srCmdSeq(); //sequence that holds the commands

	bool success = true;

	VecOfSbmCommand::iterator it  = commands.begin();
	VecOfSbmCommand::iterator end = commands.end();
	for( ; it != end ; ++it ) {
		SbmCommand* command = *it;

		if( command != NULL ) {
			if( seq->insert( (float)(command->time), command->command.c_str() ) != CMD_SUCCESS ) {
				// TODO: Throw RealizingException
				cerr << "ERROR: SequenceRequest::realize_sequence(..): SequenceRequest \"" << unique_id << "\": "
				     << "Failed to insert SbmCommand \"" << (command->command) << "\" at time " << (command->time) << "Aborting remaining commands." << endl;
				success = false;
			}
			delete command;
			(*it) = NULL;
		}
	}
	commands.clear();

	if( success ) {
		// TODO: test result, possible throwing RealizingException
		if( mcu->execute_seq( seq, unique_id.c_str() ) != CMD_SUCCESS ) {
			// TODO: Throw RealizingException
			cerr << "ERROR: SequenceRequest::realize_sequence(..): SequenceRequest \"" << unique_id << "\": " << "Failed to execute sequence \"" << unique_id.c_str() << "\"." << endl;
		}
	}

	return success;
}

bool SequenceRequest::unschedule_sequence( mcuCBHandle* mcu )
{
	return ( mcu->abort_seq( unique_id.c_str() )==CMD_SUCCESS );
}

//  VisemeRequest
//    (no transition/blend yet)
VisemeRequest::VisemeRequest( const std::string& unique_id, const char *viseme, float weight, time_sec duration,
                              const SyncPoints& syncs_in )
:	SequenceRequest( unique_id, syncs_in,
                     /* Default Timing */ 0, 0, 0, duration, duration ),
    viseme(viseme), weight(weight), duration(duration)
{}

VisemeRequest::VisemeRequest( const std::string& unique_id, const char *viseme, float weight, time_sec duration,
                              const SyncPoints& syncs_in, float rampup, float rampdown )
:	SequenceRequest( unique_id, syncs_in,
                     /* Default Timing */ 0, 0, 0, duration, duration ),
    viseme(viseme), weight(weight), duration(duration), rampup(rampup), rampdown(rampdown)
{}


void VisemeRequest::setVisemeName( const char* viseme ) {
    this->viseme = viseme;
}

void VisemeRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
{
	// Get times from SyncPoints
	time_sec startAt  = syncs.sp_start->time;
	time_sec readyAt  = syncs.sp_ready->time;
	time_sec strokeAt = syncs.sp_stroke->time;
	time_sec relaxAt  = syncs.sp_relax->time;
	time_sec endAt    = syncs.sp_end->time;

	const SbmCharacter* actor    = request->actor;
	const string&       actor_id = request->actorId; // match string used by request?

	ostringstream start_cmd;
	if( rampup < 0 ) {
		start_cmd << "char " << actor_id << " viseme " << viseme << ' ' << weight << ' ' << (readyAt-startAt);
	} else {
		start_cmd << "char " << actor_id << " viseme " << viseme << ' ' << weight << ' ' << rampup;
	}

	ostringstream stop_cmd;
	if( rampdown < 0 ) {
		stop_cmd << "char " << actor_id << " viseme " << viseme << ' ' << 0 << ' ' << (endAt-relaxAt);
	} else {
		stop_cmd << "char " << actor_id << " viseme " << viseme << ' ' << 0 << ' ' << rampdown;
	}

	VecOfSbmCommand commands;
   	commands.push_back( new SbmCommand( start_cmd.str(), (float)startAt ) );
   	commands.push_back( new SbmCommand( stop_cmd.str(), (float)relaxAt ) );

	realize_sequence( commands, mcu );
}
