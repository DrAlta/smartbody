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
#include "bml_xml_consts.hpp"

#include "ME/me_ct_blend.hpp"




using namespace std;
using namespace BML;
using namespace SmartBody;


const bool USE_CUSTOM_PRUNE_POLICY          = false; // Future feature

const bool LOG_BEHAVIOR_SYNCHPOINTS         = false;
const bool LOG_BML_BEHAVIOR_SCHEDULE        = false;
const bool LOG_METHODS						= false;
const bool LOG_CONTROLLER_SCHEDULE			= false;
const bool LOG_REQUEST_REALIZE_TIME_SPAN	= false;



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

void BmlRequest::importNamedSyncPoints( SequenceOfNamedSyncPoints& sync_seq, const std::wstring& id, const std::wstring& logging_label ) {
	// Import named SequenceOfNamedSyncPoints
	SetOfWstring names = sync_seq.get_sync_names();
	SetOfWstring::iterator it  = names.begin();
	SetOfWstring::iterator end = names.end();
	for( ; it!=end ; ++it ) {
		wstring& name = *it;
		SyncPointPtr sync( sync_seq.sync_for_name( name ) );

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
				behavior->sync_seq.validate();
#endif // VALIDATE_BEHAVIOR_SYNCS

				min_time = min( min_time, behavior->sync_seq.sp_start->time );

				if( LOG_BML_BEHAVIOR_SCHEDULE ) {
					cout << "DEBUG: BmlRequest::realize(): Behavior \""<< (behavior->unique_id) <<"\" SequenceOfNamedSyncPoints:"<<endl;
					behavior->sync_seq.printSyncTimes();
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
				
				VecOfSyncPoint::iterator syncs_end = behavior->sync_seq.end();
				for( VecOfSyncPoint::iterator j = behavior->sync_seq.begin(); j != syncs_end; ++j ) {
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

		cerr << "Realizing behavior " << behavior->unique_id << endl;
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

void BmlRequest::unschedule( Processor* bp, mcuCBHandle* mcu, time_sec duration )
{
	BmlRequestPtr request = weak_ptr.lock(); // Ref to this
	if( bp->get_auto_print_controllers() || bp->get_auto_print_sequence() )
		cout << "BmlRequest::unschedule(..) " << request->actorId << " " << request->requestId << endl;

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


	if( bp->get_auto_print_controllers() ) {
		ostringstream oss;
		oss << "print character "<< actorId << " schedule";
		string& cmd = oss.str();
		if( mcu->execute( (char*)(cmd.c_str() ) ) != CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::unschedule(..): msgId=\""<<msgId<<"\": "<<
				"Failed to execute \"" << cmd << "\" command"<<endl;
		}
	}

	if( bp->get_auto_print_sequence() ) {
		cout << "DEBUG: BML::BmlRequest::unschedule(..): Sequence \"" << start_seq_name <<"\":"<<endl;
		srCmdSeq* start_seq = mcu->lookup_seq( start_seq_name.c_str() );
		if( start_seq )
			start_seq->print();
		else
			cout << "WARNING: Cannot find sequence \"" << start_seq_name << "\"" << endl;

		cout << "DEBUG: BML::BmlRequest::unschedule(..): Sequence \"" << cleanup_seq_name <<"\":"<<endl;
		srCmdSeq* cleanup_seq = mcu->lookup_seq( cleanup_seq_name.c_str() );
		if( cleanup_seq )
			cleanup_seq->print();
		else
			cout << "WARNING: Cannot find sequence \"" << cleanup_seq_name << "\"" << endl;
	}
}


void BmlRequest::cleanup( Processor* bp, mcuCBHandle* mcu )
{
	BmlRequestPtr request = weak_ptr.lock(); // Ref to this
	if( bp->get_auto_print_controllers() || bp->get_auto_print_sequence() )
		cout << "BmlRequest::cleanup(..) " << request->actorId << " " << request->requestId << endl;

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
	mcu->abort_seq( start_seq_name.c_str() );
	mcu->abort_seq( cleanup_seq_name.c_str() );



	if( bp->get_auto_print_controllers() ) {
		ostringstream oss;
		oss << "print character "<< actorId << " schedule";
		string& cmd = oss.str();
		if( mcu->execute( (char*)(cmd.c_str() ) ) != CMD_SUCCESS ) {
			cerr << "WARNING: BML::BmlRequest::cleanup(..): msgId=\""<<msgId<<"\": "<<
				"Failed to execute \"" << cmd << "\" command"<<endl;
		}
	}

	if( bp->get_auto_print_sequence() ) {
		cout << "DEBUG: BML::BmlRequest::unschedule(..): Sequence \"" << start_seq_name <<"\":"<<endl;
		srCmdSeq* start_seq = mcu->lookup_seq( start_seq_name.c_str() );
		if( start_seq )
			start_seq->print();
		else
			cout << "WARNING: Cannot find sequence \"" << start_seq_name << "\"" << endl;

		cout << "DEBUG: BML::BmlRequest::unschedule(..): Sequence \"" << cleanup_seq_name <<"\":"<<endl;
		srCmdSeq* cleanup_seq = mcu->lookup_seq( cleanup_seq_name.c_str() );
		if( cleanup_seq )
			cleanup_seq->print();
		else
			cout << "WARNING: Cannot find sequence \"" << cleanup_seq_name << "\"" << endl;
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
		importNamedSyncPoints( behavior->sync_seq, id, L"BehaviorRequest" );
	}

	if( LOG_BEHAVIOR_SYNCHPOINTS ) {
		cout << "DEBUG: BmlRequest::registerBehavior(): SequenceOfNamedSyncPoints for " << behavior->unique_id << flush;
		if( id.size()>0 )
			wcout << " \"" << id << "\"" << flush;
		cout << ":" << endl << "\t" << flush;

		behavior->sync_seq.printSyncIds();
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
//	importNamedSyncPoints( speech->sync_seq, id, L"SpeechRequest" );
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
BehaviorRequest::BehaviorRequest( const std::string& unique_id, const SequenceOfNamedSyncPoints& sync_seq  )
:	sync_seq( sync_seq ),
	unique_id( unique_id ),
	audioOffset(TIME_UNSET),
	required(false)
{}

BehaviorRequest::~BehaviorRequest() {
	// nothing to delete.  Yay for SmartPointers!
}

void BehaviorRequest::set_scheduler( BehaviorSchedulerPtr scheduler ) {
	this->scheduler = scheduler;
}

void BehaviorRequest::schedule( time_sec now ) {
	string warning_context = string( "Behavior \"" ) + unique_id + "\"";

	sync_seq.applyParentTimes( warning_context );

	if( !scheduler ) {
		ostringstream buffer;
		buffer << "BehaviorRequest \"" << unique_id << "\" scheduler not set.";
		throw SchedulingException( buffer.str().c_str() );
	}

	scheduler->schedule( sync_seq, now );
}

void BehaviorRequest::realize( BmlRequestPtr request, mcuCBHandle* mcu ) {
	realize_impl( request, mcu );
}


bool BehaviorRequest::isPersistent() {
	time_sec duration = (sync_seq.sp_end->time) - (sync_seq.sp_start->time);
	return( duration > PERSISTENCE_THRESHOLD );
}



BehaviorSpan BehaviorRequest::getBehaviorSpan() {
	// Default algorithm for detecting persistent behaviors.
	BehaviorSpan span = sync_seq.getBehaviorSpan( PERSISTENCE_THRESHOLD );

	return span;
}


//  MeControllerRequest
MeControllerRequest::MeControllerRequest( const std::string& unique_id,
                                          MeController* anim_ct,
										  MeCtSchedulerClass* schedule_ct,
						                  const SequenceOfNamedSyncPoints& syncs_in,
										  MeControllerRequest::SchduleType sched_type )
:	BehaviorRequest( unique_id, syncs_in ),
	anim_ct( anim_ct ),
	schedule_ct( schedule_ct ),
	persistent( false )
{
	anim_ct->ref();
	schedule_ct->ref();

	switch( sched_type ) {
	case LINEAR:
		set_scheduler( buildSchedulerForController( anim_ct ) );
		break;
	case MANUAL:
		break;
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


void MeControllerRequest::register_controller_prune_policy( MePrunePolicy* prune_policy ) {
	if( anim_ct != NULL ) {
		anim_ct->prune_policy( prune_policy );
	}
}

void MeControllerRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu ) {
	// Get times from SequenceOfNamedSyncPoints
	time_sec startAt  = sync_seq.sp_start->time;
	time_sec readyAt  = sync_seq.sp_ready->time;
	time_sec strokeAt = sync_seq.sp_stroke->time;
	time_sec relaxAt  = sync_seq.sp_relax->time;
	time_sec endAt    = sync_seq.sp_end->time;

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
	schedule_ct->schedule( anim_ct, (double)startAt, (float)indt, (float)outdt );
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
	MeCtScheduler2::TrackPtr track = schedule_ct->track_for_anim_ct( anim_ct );
	if( track ) {
		MeCtUnary* unary_blend_ct = track->blending_ct();
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

	persistent = false;
}

/**
 *  Implemtents BehaviorRequest::cleanup(..),
 *  removing the MeController from its parent.
 */
void MeControllerRequest::cleanup( mcuCBHandle* mcu, BmlRequestPtr request )
{
	if( schedule_ct ) {
		if( !persistent ) {
			// TODO: If track is no longer valid, the NULL TrackPtr will be ignored by remove_track
			schedule_ct->remove_track( schedule_ct->track_for_anim_ct( anim_ct ) );
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
						      const SequenceOfNamedSyncPoints& syncs_in )
:	MeControllerRequest( unique_id,
                         motion_ct,
						 schedule_ct,
						 syncs_in )
{}


//  NodRequest
NodRequest::NodRequest( const std::string& unique_id, NodType type, float repeats, float frequency, float extent, const SbmCharacter* actor,
			            const SequenceOfNamedSyncPoints& syncs_in )
:	MeControllerRequest( unique_id, new MeCtSimpleNod(), actor->head_sched_p, syncs_in, MeControllerRequest::MANUAL ),
    type(type), repeats(repeats), frequency(frequency), extent(extent)
{
    MeCtSimpleNod* nod = (MeCtSimpleNod*)anim_ct;
	BehaviorSchedulerConstantSpeedPtr scheduler = buildSchedulerForController( nod );
	set_scheduler( scheduler );

	// Convenience References
	time_sec& readyTime  = scheduler->readyTime;
	time_sec& strokeTime = scheduler->strokeTime;
	time_sec& relaxTime  = scheduler->relaxTime;
	time_sec& endTime    = scheduler->endTime;


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
						  const SequenceOfNamedSyncPoints& syncs_in )
:	MeControllerRequest( unique_id, tilt, actor->head_sched_p, syncs_in ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/,
	transitionDuration(transitionDuration)
{
	BehaviorSchedulerConstantSpeedPtr scheduler = buildSchedulerForController( tilt );

    scheduler->readyTime = scheduler->strokeTime = transitionDuration;
    scheduler->relaxTime = scheduler->endTime - transitionDuration;

	set_scheduler( scheduler );
}

//  PostureRequest
PostureRequest::PostureRequest( const std::string& unique_id, MeController* pose, time_sec transitionDuration, const SbmCharacter* actor,
						        const SequenceOfNamedSyncPoints& syncs_in )
:	MeControllerRequest( unique_id, pose, actor->posture_sched_p, syncs_in ),
    duration(numeric_limits<time_sec>::infinity())/*hack*/,
	transitionDuration(transitionDuration)
{
	BehaviorSchedulerConstantSpeedPtr scheduler = buildSchedulerForController( pose );

    scheduler->readyTime = scheduler->strokeTime = transitionDuration;
    scheduler->relaxTime = scheduler->endTime - transitionDuration;

	set_scheduler( scheduler );
}

// SequenceRequest
SequenceRequest::SequenceRequest( const std::string& unique_id, const SequenceOfNamedSyncPoints& syncs_in,
                                  time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime )
:	BehaviorRequest( unique_id, syncs_in )
{
	set_scheduler( BehaviorSchedulerPtr( new BehaviorSchedulerConstantSpeed( startTime, readyTime, strokeTime, relaxTime, endTime, 1 ) ) );
}

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
                              const SequenceOfNamedSyncPoints& syncs_in )
:	SequenceRequest( unique_id, syncs_in,
                     /* Default Timing */ 0, 0, 0, duration, duration ),
    viseme(viseme), weight(weight), duration(duration)
{}

VisemeRequest::VisemeRequest( const std::string& unique_id, const char *viseme, float weight, time_sec duration,
                              const SequenceOfNamedSyncPoints& syncs_in, float rampup, float rampdown )
:	SequenceRequest( unique_id, syncs_in,
                     /* Default Timing */ 0, 0, 0, duration, duration ),
    viseme(viseme), weight(weight), duration(duration), rampup(rampup), rampdown(rampdown)
{}


void VisemeRequest::setVisemeName( const char* viseme ) {
    this->viseme = viseme;
}

void VisemeRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
{
	// Get times from SequenceOfNamedSyncPoints
	time_sec startAt  = sync_seq.sp_start->time;
	time_sec readyAt  = sync_seq.sp_ready->time;
	time_sec strokeAt = sync_seq.sp_stroke->time;
	time_sec relaxAt  = sync_seq.sp_relax->time;
	time_sec endAt    = sync_seq.sp_end->time;

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
