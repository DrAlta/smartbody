/*
 *  bml_speech.cpp - part of SmartBody-lib
 *  Copyright (C) 2009  University of Southern California
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

#include <iostream>
#include <sstream>
#include <vector>

#include "mcontrol_util.h"
#include "bml.hpp"
#include "bml_speech.hpp"



using namespace std;
using namespace BML;
using namespace SmartBody;



// XML Constants
const XMLCh TAG_TM[]        = L"tm";
const XMLCh TAG_MARK[]      = L"mark";

const XMLCh VALUE_TEXT_PLAIN[] = L"text/plain";
const XMLCh VALUE_SSML[]       = L"application/ssml+xml";

const char* VISEME_NEUTRAL = "_";




// Replaces <tm> with <mark> in word break processing
// TODO: Enable both as part of a backward compatibile transition mode
//       or transition fully to <text> detection and processing.
#define ENABLE_BMLR_SPEECH_REQUEST_CODE  0


// SpeechRequest Helper functions
void BML::SpeechRequest::createStandardSyncPoint( const std::wstring& sync_id, SyncPointPtr& sync ) {
	sync = trigger->addSyncPoint();
	syncs.insert( sync_id, sync, syncs.end() );
}


BML::SpeechRequestPtr BML::parse_bml_speech(
	DOMElement* xml,
	const std::string& unique_id,
	BML::SyncPoints& syncs,
	BML::BmlRequestPtr request,
	mcuCBHandle *mcu )
{
	vector<SpeechMark> marks;  // Ordered list of named bookmarks

	// Parse <speech> for sync points
	const XMLCh* type = xml->getAttribute( ATTR_TYPE );
	if( type ) {
#if ENABLE_BMLR_SPEECH_REQUEST_CODE
		// [BMLR] text/plain as default type
		if( XMLString::stringLen( type) == 0 ) {
			type = VALUE_TEXT_PLAIN;
		}
#endif

		if( XMLString::compareString( type, VALUE_TEXT_PLAIN )==0 ) {
			if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" << VALUE_TEXT_PLAIN << "\">" << endl;
			// Search for <tm> sync_points
			DOMElement* child = xml_utils::getFirstChildElement( xml );
			while( child!=NULL ) {
				const XMLCh* tag = child->getTagName();
#if ENABLE_BMLR_SPEECH_REQUEST_CODE
				 // [BMLR] Changed <tm> to <mark> and id="" to name=""
				if( XMLString::compareString( tag, TAG_MARK )==0 ) {
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <mark>" << endl;
#else
				if( XMLString::compareString( tag, TAG_TM )==0 ) {
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <tm>" << endl;
#endif

#if ENABLE_BMLR_SPEECH_REQUEST_CODE
					wstring tmId( child->getAttribute( ATTR_NAME ) );
#else
					wstring tmId( child->getAttribute( ATTR_ID ) );
#endif
					// test validity?
					if( !tmId.empty() ) {
						if( isValidTmId( tmId ) ) {
							marks.push_back( SpeechMark( tmId, TIME_UNSET ) );
						} else {
#if ENABLE_BMLR_SPEECH_REQUEST_CODE
							wcerr << "ERROR: Invalid <mark> name=\"" << tmId << "\"" << endl;
#else
							wcerr << "ERROR: Invalid <tm> id=\"" << tmId << "\"" << endl;
#endif
							// TODO: remove mark from XML
						}
					}
				}
				child = xml_utils::getNextElement( child );
			}
		} else if( XMLString::compareString( type, VALUE_SSML )==0 ) {
			if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" << VALUE_SSML << "\">" << endl;
			// Search for <mark> sync_points
			DOMElement* child = xml_utils::getFirstChildElement( xml );
			while( child!=NULL ) {
				const XMLCh* tag = child->getTagName();
				if( tag && XMLString::compareString( tag, TAG_MARK )==0 ) {
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <mark>" << endl;

					wstring tmId = child->getAttribute( ATTR_NAME );
					// test validity?
					if( !tmId.empty() ) {
						if( isValidTmId( tmId ) ) {
							marks.push_back( SpeechMark( tmId, TIME_UNSET ) );
						} else {
							wcerr << "ERROR: Invalid <mark> name=\"" << tmId << "\"" << endl;
							// TODO: remove <mark> from XML
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
	// Successfully parsed!!

	// request speech through Speech API
	SmartBody::SpeechInterface* speech_impl = request->actor->get_speech_impl();
	if( !speech_impl ) {
		ostringstream oss;
		oss << "No voice defined for actor \""<<request->actorId<<"\".  Cannot perform behavior \""<<unique_id<<"\".";
		throw BML::ParsingException( oss.str().c_str() );
	}

	// Found speech implementation.  Making request.
	RequestId speech_request_id = speech_impl->requestSpeechAudio( request->actorId.c_str(), xml, "bp speech_ready " );

	// TODO: SyncPoints of a speech behavior should be grouped under a unique TriggerEvent,
	//       rather the default start trigger.  The trigger identifies the additional processing
	//       necessary for the speech.
	//TriggerEventPtr trigger = request->createTrigger( L"SPEECH" );
	TriggerEventPtr trigger = syncs.sp_start->trigger.lock();

//// Old code:  syncs are now parsed and passed in
//	// Current Speech behavior constraints prevent us from using the sync point attributes
//	// Creating new SyncPoints instead of parsing the attributes.
//	createStandardSyncPoint( TM_START,        syncs.sp_start );
//	createStandardSyncPoint( TM_READY,        syncs.sp_ready );
//	createStandardSyncPoint( TM_STROKE_START, syncs.sp_stroke_start );
//	createStandardSyncPoint( TM_STROKE,       syncs.sp_stroke );
//	createStandardSyncPoint( TM_STROKE_END,   syncs.sp_stroke_end );
//	createStandardSyncPoint( TM_RELAX,        syncs.sp_relax );
//	createStandardSyncPoint( TM_END,          syncs.sp_end );

	return SpeechRequestPtr( new SpeechRequest( unique_id, syncs, speech_impl, speech_request_id, marks, request ) );
}

//  SpeechRequest
//    (no transition/blend yet)
BML::SpeechRequest::SpeechRequest(
	const std::string& unique_id,
	SyncPoints& syncs_in,
	SpeechInterface* speech_impl,
	RequestId speech_request_id,
	const vector<SpeechMark>& marks,
	BmlRequestPtr request
)
:	SequenceRequest( unique_id, syncs_in, 0, 0, 0, 0, 0 ),
	speech_impl( speech_impl ),
	speech_request_id( speech_request_id ),
	trigger( syncs.sp_start->trigger.lock() )
{
	// Add SyncPoints for SpeechMarks
	vector<SpeechMark>::const_iterator end = marks.end();
	for( vector<SpeechMark>::const_iterator mark = marks.begin(); mark != end; ++mark ) {
		// Create a SyncPoint
		SyncPointPtr sync( trigger->addSyncPoint() );

		// Insert just before stroke_end
		SyncPoints::iterator stroke_end_pos = syncs.pos_of( syncs.sp_stroke_end );
		SyncPoints::iterator result_pos = syncs.insert( mark->id, sync, stroke_end_pos );  // Test insertion, and throw error if problem

		// Remember Word Break
		if( !( wbToSync.insert( make_pair( mark->id, sync ) ).second ) )
			wcerr << "ERROR: SpeechRequest(..): Failed to insert word break SyncPoint \""<<mark->id<<"\" into wbToSync map." << endl;
	}
}

BML::SpeechRequest::~SpeechRequest() {
	// delete visemes
	size_t count = visemes.size();
	for( size_t i=0; i<count; ++i )
		delete visemes[i];
}

/*
SyncPoint* SpeechRequest::addWordBreakSync( const std::wstring& wbId ) {
	map< const XMLCh*, SyncPoint*, xml_utils::XMLStringCmp >& sync_points = trigger->request->sync_points;
	const XMLCh* tmId = buildBmlId( id, markId );

	if( sync_points.find( tmId ) == sync_points.end() ) {
		// id doesn't exist.. go ahead
		SyncPoint* sp = new SyncPoint( buildBmlId( id, markId ),
			                             trigger, relax->prev ); // append before relax
		sync_points.insert( make_pair( tmId, sp ) );
		return sp;
	} else {
		delete [] tmId;
		return NULL;
	}
}
*/

SyncPointPtr BML::SpeechRequest::getWordBreakSync( const std::wstring& wbId ) {
	MapOfSyncPoint::iterator it = wbToSync.find( wbId );
	if( it == wbToSync.end() )
		return SyncPointPtr();
	else
		return it->second;
}


void BML::SpeechRequest::speech_response( srArgBuffer& response_args ) {
	const char* status = response_args.read_token();
	const char* error_msg = NULL;
	if( strcmp( status, "SUCCESS" )!=0 ) {
		if( strcmp( status, "ERROR" )==0 ) {
			error_msg = response_args.read_remainder_raw();
			if( error_msg == NULL ) {
				error_msg = "!!NO ERROR MESSAGE!!";
			}
		} else {
			error_msg = "!!INVALID SPEECH CALLBACK SUBCOMMAND (bml_old_processor)!!";
			// TODO: include status in errorMsg without memory leak (use &std::String?)
		}
	}

	// TODO: parse response and set speech_error_msg
	this->speech_error_msg = error_msg? error_msg : string();
}

void BML::SpeechRequest::schedule( time_sec now ) {
	//// TODO: Sync to prior behaviors
	// syncs.applyParentTimes()
	// find set SyncPoints
	// if more than one, warn and ignore least important

	// Convience references
	SyncPointPtr sp_start( syncs.sp_start );
	SyncPointPtr sp_ready( syncs.sp_ready );
	SyncPointPtr sp_stroke_start( syncs.sp_stroke_start );
	SyncPointPtr sp_stroke( syncs.sp_stroke );
	SyncPointPtr sp_stroke_end( syncs.sp_stroke_end );
	SyncPointPtr sp_relax( syncs.sp_relax );
	SyncPointPtr sp_end( syncs.sp_end );

	syncs.applyParentTimes();

	BmlRequestPtr       request  = trigger->request.lock();
	const SbmCharacter* actor    = request->actor;
	string              actor_id = request->actorId;

	// Found speech implementation.  Making request.
	if( !speech_error_msg.empty() ) {
		ostringstream oss;
		oss << "SpeechInterface error: "<<speech_error_msg;
		throw SchedulingException( oss.str().c_str() );
	}

	audioPlay = speech_impl->getSpeechPlayCommand( speech_request_id, actor );
	audioStop = speech_impl->getSpeechStopCommand( speech_request_id, actor );
	if( LOG_AUDIO ) {
		cout << "DEBUG: BML::SpeechRequest::processReply(): audioPlay = " << audioPlay << endl;
		cout << "DEBUG: BML::SpeechRequest::processReply(): audioStop = " << audioStop << endl;
	}

	// save timing;
	time_sec first_open  = TIME_UNSET;  // start of first non-neutral viseme
	time_sec last_open   = TIME_UNSET;  // end of last non-neutral viseme
	time_sec last_viseme = TIME_UNSET;  // end of last viseme

	// Process Visemes
	const vector<VisemeData*>* result_visemes = speech_impl->getVisemes( speech_request_id );
	if( result_visemes ) {
		visemes = *result_visemes;  // Copy contents

		vector<VisemeData*>::iterator cur = visemes.begin();
		vector<VisemeData*>::iterator end = visemes.end();

		if( LOG_SPEECH && cur==end )
			cerr << "ERROR: BodyPlannerImpl::speechReply(): speech.getVisemes( " << speech_request_id << " ) is empty." << endl;

		for( ; cur!=end; ++cur ) {
			VisemeData* v = (*cur);

			if( LOG_SPEECH ) {
				//cout << "   " << (*v) << endl;  // Not linking
				cout << "   VisemeData: " << v->id() << " (" << v->weight() << ") @ " << v->time() << endl;
			}
			if( strcmp( v->id(), VISEME_NEUTRAL )!=0 ) {
				if( !isTimeSet( first_open ) )
					first_open = v->time();
				last_open = v->time() + v->duration();
			}
			last_viseme = v->time() + v->duration();
		}
	} else {
		if( LOG_SPEECH )
			cerr << "WARNING: BodyPlannerImpl::speechReply(): speech.getVisemes( " << speech_request_id << " ) returned NULL." << endl;
	}

	time_sec start_time = now; // TODO: sync to prior behaviors

	//  Set core sync_point times
	sp_start->time = start_time;
	if( isTimeSet( last_viseme ) ) {
		last_viseme += start_time;

		if( isTimeSet( first_open ) ) {
			first_open += start_time;
			last_open  += start_time;

			sp_ready->time        = first_open;
			sp_stroke_start->time = first_open;
			sp_stroke->time       = first_open;
			sp_stroke_end->time   = last_open;
			sp_relax->time        = last_open;
		} else {
			// Never opens mouth
			sp_ready->time        = start_time;
			sp_stroke_start->time = start_time;
			sp_stroke->time       = start_time;
			sp_stroke_end->time   = last_viseme;
			sp_relax->time        = last_viseme;
		}
		sp_end->time = last_viseme;
	} else {
		// No timing information
		sp_ready->time        = start_time;
		sp_stroke_start->time = start_time;
		sp_stroke->time       = start_time;
		sp_stroke_end->time   = start_time;
		sp_relax->time        = start_time;
		sp_end->time          = start_time;
	}


	// Process Word Break SyncPoints
	MapOfSyncPoint::iterator wb_it  = wbToSync.begin();
	MapOfSyncPoint::iterator wb_end = wbToSync.end();
	if( wb_it != wb_end ) {
		for(; wb_it != wb_end; ++wb_it ) {
			const wstring& wb_id = wb_it->first;
			SyncPointPtr  cur   = wb_it->second;

			if( cur->parent != NULL && !isTimeSet( cur->parent->time ) ) {
				wcerr << "ERROR: BodyPlannerImpl::speechReply(): Unhandled case of Wordbreak SyncPoint \"" << wb_id << "\" with scheduled parent SyncPoint.  Ignoring offset." << endl;
			}

			float audioTime = speech_impl->getMarkTime( speech_request_id, wb_id.c_str() );
			if( audioTime >= 0 ) {
				if( LOG_SYNC_POINTS ) wcout << "   Wordbreak SyncPoint \"" << wb_id << "\" @ " << audioTime << endl;
				cur->time = start_time + audioTime;
			} else {
				wcerr << "ERROR: BodyPlannerImpl::speechReply(): No audioTime for Wordbreak SyncPoint \"" << wb_id << "\"" << endl;
			}
		}
	} else {
		if( LOG_SYNC_POINTS )
			cout << "   BodyPlannerImpl::speechReply(..): No speech bookmarks" << endl;
	}
}

void BML::SpeechRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
{
	// Get times from SyncPoints
	time_sec startAt  = syncs.sp_start->time;
	time_sec readyAt  = syncs.sp_ready->time;
	time_sec strokeAt = syncs.sp_stroke->time;
	time_sec relaxAt  = syncs.sp_relax->time;
	time_sec endAt    = syncs.sp_end->time;

	const string& actor_id = request->actor->name;

//// SyncPoints should already be set from viseme processing
//	{	// Offset prior syncpoint times by startAt
//		SyncPoints::iterator it = syncs.begin();
//		SyncPoints::iterator end = syncs.end();
//		for( ; it != end ; ++it ) {
//			SyncPointPtr sync = (*it);
//			if( isTimeSet( sync->time ) ) {
//				sync->time += startAt;
//			}
//		}
//	}

	// Schedule visemes
	//   visemes are stored in request->visemes as VisemeData objects (defined in bml.hpp)
	// add audioOffset to each viseme time,
	if( visemes.size() > 0 ) {
		//// Replaced by addition in next loop
		//for( int i=0; i<(int)request->visemes.size(); i++ ) {
		//	request->visemes.at(i)->time+= audioOffset;
		//}

		ostringstream command;
		const size_t viseme_count = visemes.size();
		for( size_t i=0; i<viseme_count; i++ ) { //adds visemes for audio into sequence file
			VisemeData* v = visemes.at(i);
			time_sec time = (time_sec)( v->time() + startAt );

			command.str( "" );
			command << "char " << actor_id << " viseme " << v->id() << ' ' << v->weight() << ' ';
			if( v->duration() > 0 ) {
				// speech implementation doesn't appear to support durations.
				command << v->duration();
			} else {
				command << "0.1";
				time -= (time_sec)0.05;
			}

			
			if( LOG_BML_VISEMES ) cout << "command (complete): " << command.str() << endl;
			sbm_commands.push_back( new SbmCommand( command.str(), time ) );
			////visemes get set a specified time
			//if( seq->insert( time, (char*)(command.str().c_str()) )!=CMD_SUCCESS ) {
			//	cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
			//		"Failed to insert viseme \""<<v->id()<<"\" @ "<<time<<endl;
			//}
			if( LOG_BML_VISEMES ) {
				ostringstream echo;
				echo << "echo LOG_BML_VISEMES:\t" << time << ":\t" << command.str();
				sbm_commands.push_back( new SbmCommand( echo.str(), time ) );
			}
		}
	} else {
		cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): "<<//"msgId=\""<<bpMsg.msgId<<"\": "<<
			"SpeechRequest has no visemes." <<endl;
	}

	// Schedule audio
	if( !audioPlay.empty() ) {
		if( LOG_AUDIO || LOG_BML_VISEMES )
			cout << "DEBUG: BodyPlannerImpl::realizeRequest(..): scheduling request->audioPlay: " << audioPlay << endl;
		// schedule for later
		sbm_commands.push_back( new SbmCommand( audioPlay, startAt ) );
		//if( seq->insert( (float)(audioOffset<0? 0: audioOffset), audioPlay.c_str() ) != CMD_SUCCESS ) {
		//	printf( "ERROR: BodyPlannerImpl::realizeRequest: insert audio trigger into seq FAILED, msgId=%s\n", bpMsg.msgId ); 
		//}
	} else {
		cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): "<< //"msgId=\""<<bpMsg.msgId<<"\": "<<
			"SpeechRequest has no audioPlay command." <<endl;
	}

	realize_sequence( sbm_commands, mcu );
}


void BML::SpeechRequest::unschedule( mcuCBHandle* mcu,
	                            BmlRequestPtr request,
	                            time_sec duration )
{
	unschedule_sequence( mcu );
	// TODO: Force mouth closed?  What about cometing viseme sequences?

	if( !audioStop.empty() )
		mcu->execute_later( audioStop.c_str() );
	else
		cerr << "WARNING: SpeechRequest::unschedule(): unique_id \""<<unique_id<<"\": Missing audioStop." << endl;
}
	                            
void BML::SpeechRequest::cleanup( mcuCBHandle* mcu, BmlRequestPtr request )
{
	visemes.clear();
	unschedule_sequence( mcu );

	speech_impl->requestComplete( speech_request_id );
}
