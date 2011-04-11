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

#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "bml_speech.hpp"

#include "bml.hpp"
#include "bml_exception.hpp"
#include "bml_xml_consts.hpp"



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
	behav_syncs.insert( sync_id, sync, behav_syncs.end() );
}


BML::SpeechRequestPtr BML::parse_bml_speech(
	DOMElement* xml,
	const std::string& unique_id,
	BML::BehaviorSyncPoints& behav_syncs,
	bool required,
	BML::BmlRequestPtr request,
	mcuCBHandle *mcu )
{
	const XMLCh* id = xml->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	vector<SpeechMark> marks;  // Ordered list of named bookmarks

	// Parse <speech> for sync points
	const XMLCh* type = xml->getAttribute( ATTR_TYPE );
	if( type ) {
#if ENABLE_BMLR_SPEECH_REQUEST_CODE
		// [BMLR] text/plain as default type
		if( *type == 0 ) {
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
							wstrstr << "ERROR: Invalid <mark> name=\"" << tmId << "\"" << endl;
#else
							std::wstringstream wstrstr;
							wstrstr << "ERROR: Invalid <tm> id=\"" << tmId << "\"";
							LOG(convertWStringToString(wstrstr.str()).c_str());
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
							std::wstringstream wstrstr;
							wstrstr << "ERROR: Invalid <mark> name=\"" << tmId << "\"" << endl;
							LOG(convertWStringToString(wstrstr.str()).c_str());
							// TODO: remove <mark> from XML
						}
					}
				}
				child = xml_utils::getNextElement( child );
			}
		} else {
			std::wstringstream wstrstr;
			wstrstr << "ERROR: SpeechRequest::SpeechRequest(..): Unrecognized speech behavior type=\"" << type << "\"";
			LOG(convertWStringToString(wstrstr.str()).c_str());
		}
	} else {
		LOG("ERROR: SpeechRequest::SpeechRequest(..): Speech behavior lacks type attribute");
	}
	// Successfully parsed!!

	// request speech through Speech API
	SmartBody::SpeechInterface* speech_impl = request->actor->get_speech_impl();
	// get the backup speech
	SmartBody::SpeechInterface* speech_impl_backup = request->actor->get_speech_impl_backup();

	if( !speech_impl && speech_impl_backup ) {
		speech_impl = speech_impl_backup;
		speech_impl_backup = NULL;
	}

	SmartBody::SpeechInterface* cur_speech_impl = speech_impl;
	SmartBody::SpeechInterface* cur_speech_impl_backup = speech_impl_backup;

	if (!cur_speech_impl) {
		ostringstream oss;
		oss << "No voice defined for actor \""<<request->actorId<<"\".  Cannot perform behavior \""<<unique_id<<"\".";
		throw BML::ParsingException( oss.str().c_str() );
	}

	

	// Before speech implementation, check if it's audio implementation, if yes, set the viseme mode
	AudioFileSpeech* audioSpeechImpl = dynamic_cast<AudioFileSpeech*>(cur_speech_impl);
	if (audioSpeechImpl)
	{	
		bool visemeMode = request->actor->get_viseme_curve_mode();
		audioSpeechImpl->setVisemeMode(visemeMode);
	}
	AudioFileSpeech* audioSpeechImplBackup = dynamic_cast<AudioFileSpeech*>(cur_speech_impl_backup);
	if (audioSpeechImplBackup)
	{	
		bool visemeMode = request->actor->get_viseme_curve_mode();
		audioSpeechImplBackup->setVisemeMode(visemeMode);
	}

	// Found speech implementation.  Making request.
	RequestId speech_request_id;
	try {
		speech_request_id = cur_speech_impl->requestSpeechAudio( request->actorId.c_str(), request->actor->get_voice_code(), xml, "bp speech_ready " );
	} catch (...) {
		if (cur_speech_impl_backup) {
			cur_speech_impl = cur_speech_impl_backup;
			cur_speech_impl_backup = NULL;
			speech_request_id = cur_speech_impl->requestSpeechAudio( request->actorId.c_str(), request->actor->get_voice_code_backup(), xml, "bp speech_ready " );
		}
		else
			throw BML::ParsingException("No backup speech available");
	}
	if (speech_request_id == 0)
	{
		if (cur_speech_impl_backup) {
			cur_speech_impl = cur_speech_impl_backup;
			cur_speech_impl_backup = NULL;
			speech_request_id = cur_speech_impl->requestSpeechAudio( request->actorId.c_str(), request->actor->get_voice_code_backup(), xml, "bp speech_ready " );
		}
		else 
			throw BML::ParsingException("No backup speech available");
	}

	// TODO: SyncPoints of a speech behavior should be grouped under a unique TriggerEvent,
	//       rather the default start trigger.  The trigger identifies the additional processing
	//       necessary for the speech.
	//TriggerEventPtr trigger = request->createTrigger( L"SPEECH" );
	TriggerEventPtr trigger = behav_syncs.sync_start()->sync()->trigger.lock();

//// Old code:  behav_syncs are now parsed and passed in
//	// Current Speech behavior constraints prevent us from using the sync point attributes
//	// Creating new BehaviorSyncPoints instead of parsing the attributes.
//	createStandardSyncPoint( TM_START,        behav_syncs.sp_start );
//	createStandardSyncPoint( TM_READY,        behav_syncs.sp_ready );
//	createStandardSyncPoint( TM_STROKE_START, behav_syncs.sp_stroke_start );
//	createStandardSyncPoint( TM_STROKE,       behav_syncs.sp_stroke );
//	createStandardSyncPoint( TM_STROKE_END,   behav_syncs.sp_stroke_end );
//	createStandardSyncPoint( TM_RELAX,        behav_syncs.sp_relax );
//	createStandardSyncPoint( TM_END,          behav_syncs.sp_end );

	SpeechRequestPtr speechResult( new SpeechRequest( unique_id, localId, behav_syncs, cur_speech_impl, cur_speech_impl_backup, speech_request_id, marks, request ) );
	return speechResult;

}

//  SpeechRequest
//    (no transition/blend yet)
BML::SpeechRequest::SpeechRequest(
	const std::string& unique_id,
	const std::string& localId,
	BehaviorSyncPoints& syncs_in,
	SpeechInterface* speech_impl,
	SpeechInterface* speech_impl_backup,
	RequestId speech_request_id,
	const vector<SpeechMark>& marks,
	BmlRequestPtr request
)
:	SequenceRequest( unique_id, localId, syncs_in, 0, 0, 0, 0, 0 ),
	speech_impl( speech_impl ),
	speech_impl_backup( speech_impl_backup ),
	speech_request_id( speech_request_id ),
	trigger( behav_syncs.sync_start()->sync()->trigger.lock() )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	// Add SyncPoints for SpeechMarks
	vector<SpeechMark>::const_iterator end = marks.end();
	for( vector<SpeechMark>::const_iterator mark = marks.begin(); mark != end; ++mark ) {
		// save the speech marks
		speechMarks.push_back(*mark);

		// Create a SyncPoint
		SyncPointPtr sync( trigger->addSyncPoint() );

		// Insert just before stroke_end
		BehaviorSyncPoints::iterator stroke_end_pos = behav_syncs.sync_stroke_end();
		BehaviorSyncPoints::iterator result_pos = behav_syncs.insert( mark->id, sync, stroke_end_pos );  // Test insertion, and throw error if problem

		// Remember Word Break
		if( !( wbToSync.insert( make_pair( mark->id, sync ) ).second ) )
		{
			std::wstringstream wstrstr;
			wstrstr << "ERROR: SpeechRequest(..): Failed to insert word break SyncPoint \""<<mark->id<<"\" into wbToSync map.";
			LOG(convertWStringToString(wstrstr.str()).c_str());
		}
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

MapOfSyncPoint& BML::SpeechRequest::getWorkBreakSync()
{
	return wbToSync;
}

const std::vector<SpeechMark>& BML::SpeechRequest::getMarks()
{
	return speechMarks;
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
	// behav_syncs.applyParentTimes()
	// find set SyncPoints
	// if more than one, warn and ignore least important

	// Convience references
	SyncPointPtr sp_start( behav_syncs.sync_start()->sync() );
	SyncPointPtr sp_ready( behav_syncs.sync_ready()->sync() );
	SyncPointPtr sp_stroke_start( behav_syncs.sync_stroke_start()->sync() );
	SyncPointPtr sp_stroke( behav_syncs.sync_stroke()->sync() );
	SyncPointPtr sp_stroke_end( behav_syncs.sync_stroke_end()->sync() );
	SyncPointPtr sp_relax( behav_syncs.sync_relax()->sync() );
	SyncPointPtr sp_end( behav_syncs.sync_end()->sync() );

	string warning_context = string( "Behavior \"" ) + unique_id + "\"";
	behav_syncs.applyParentTimes( warning_context );

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
	vector<VisemeData*>* result_visemes = speech_impl->getVisemes( speech_request_id, actor );
	if( !result_visemes ) {
		if (speech_impl_backup) // run the backup speech server if available
			result_visemes = speech_impl->getVisemes( speech_request_id, NULL );
	}

	if (result_visemes)
	{
		//visemes = *result_visemes;  // Copy contents
		for ( size_t i = 0; i < (*result_visemes).size(); i++ )
		{
			VisemeData* v = (*result_visemes)[ i ];
			// drop any visemes that don't exceed the viseme threshold
			if (v->duration() < actor->getMinVisemeTime())
				continue;
			if (!v->isCurveMode() && !v->isTrapezoidMode() && !v->isFloatCurveMode())
				visemes.push_back( new VisemeData( v->id(), v->weight(), v->time() ) );
			else if (v->isTrapezoidMode() && !v->isFloatCurveMode())
				visemes.push_back( new VisemeData( v->id(), v->weight(), v->time(), v->duration(), v->rampin(), v->rampout() ) );
			else if (!v->isFloatCurveMode())
				visemes.push_back( new VisemeData( v->id(), v->getNumKeys(), v->getCurveInfo() ));
			else
			{
				VisemeData* vcopy = new VisemeData( v->id(), v->time());
				vcopy->setFloatCurve(v->getFloatCurve(), v->getNumKeys(), v->getFloatsPerKey());
				visemes.push_back( vcopy );
			}

		}

		vector<VisemeData*>::iterator cur = visemes.begin();
		vector<VisemeData*>::iterator end = visemes.end();

		if( LOG_SPEECH && cur==end )
		{
			std::stringstream strstr;
			strstr << "ERROR: BodyPlannerImpl::speechReply(): speech.getVisemes( " << speech_request_id << " ) is empty.";
			LOG(strstr.str().c_str());
		}

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
		{
			std::stringstream strstr;
			strstr << "WARNING: BodyPlannerImpl::speechReply(): speech.getVisemes( " << speech_request_id << " ) returned NULL.";
			LOG(strstr.str().c_str());
		}
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

			if( cur->parent != NULL && !isTimeSet( cur->parent->time ) )
			{
				std::wstringstream wstrstr;
				wstrstr << "ERROR: BodyPlannerImpl::speechReply(): Unhandled case of Wordbreak SyncPoint \"" << wb_id << "\" with scheduled parent SyncPoint.  Ignoring offset.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
			}

			float audioTime = speech_impl->getMarkTime( speech_request_id, wb_id.c_str() );
			if (audioTime < 0)
			{
				std::string wordBreakId(wb_id.begin(), wb_id.end());
				int pos = wordBreakId.find(":");
				if (pos == std::string::npos)
				{ // prefix was not given - try again with proper prefix
					std::string wordBreakIdWithPrefix = this->local_id;
					wordBreakIdWithPrefix.append(":");
					wordBreakIdWithPrefix.append(wordBreakId);
					XMLCh tempStr[256];
					XMLString::transcode(wordBreakIdWithPrefix.c_str(), tempStr, 255);
					audioTime = speech_impl->getMarkTime(speech_request_id, tempStr);
				}
				else
				{ // prefix was given - try again without prefix
					std::string wordBreakSuffix = wordBreakId.substr(pos + 1, wordBreakId.size() - pos - 1);
					XMLCh tempStr[256];
					XMLString::transcode(wordBreakSuffix.c_str(), tempStr, 255);
					audioTime = speech_impl->getMarkTime(speech_request_id, tempStr);
				}

			}
			if( audioTime >= 0 ) {
				if( LOG_SYNC_POINTS ) wcout << "   Wordbreak SyncPoint \"" << wb_id << "\" @ " << audioTime << endl;
				cur->time = start_time + audioTime;
			} else {
				std::wstringstream wstrstr;
				wstrstr << "ERROR: BodyPlannerImpl::speechReply(): No audioTime for Wordbreak SyncPoint \"" << wb_id << "\"";
				LOG(convertWStringToString(wstrstr.str()).c_str());
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
	time_sec startAt  = behav_syncs.sync_start()->time();
	time_sec readyAt  = behav_syncs.sync_ready()->time();
	time_sec strokeAt = behav_syncs.sync_stroke()->time();
	time_sec relaxAt  = behav_syncs.sync_relax()->time();
	time_sec endAt    = behav_syncs.sync_end()->time();

#if ENABLE_DIRECT_VISEME_SCHEDULE
	SbmCharacter *actor_p = (SbmCharacter*)( request->actor );
#endif
	const string& actor_id = request->actor->name;

//// SyncPoints should already be set from viseme processing
//	{	// Offset prior syncpoint times by startAt
//		BehaviorSyncPoints::iterator it = behav_syncs.begin();
//		BehaviorSyncPoints::iterator end = behav_syncs.end();
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
			if (v->isFloatCurveMode())
			{
				command.str( "" );
				std::vector<float>& data = v->getFloatCurve();
				int numKeys = v->getNumKeys();
				int floatsPerKey = v->getFloatsPerKey();
				command << "char " << actor_id << " viseme " << v->id() << " curve " << numKeys << ' ';
				for (int x = 0; x < numKeys; x++)
				{
					command << data[x * floatsPerKey] << " " << data[x * floatsPerKey + 1] << " "; 
				}
				
				time_sec time = mcu->time;
				sbm_commands.push_back( new SbmCommand( command.str(), time ) );
			}
			else if (!v->isCurveMode())
			{
				time_sec time = (time_sec)( v->time() + startAt );
#if ENABLE_DIRECT_VISEME_SCHEDULE
				float ramp_dur;
				if( v->duration() > 0 ) {
					ramp_dur = v->duration();
				} else {
					// speech implementation doesn't appear to support durations.
					// using 0.1 transition duration (and start transition early)
					ramp_dur = 0.1f;
					time -= (time_sec)0.05;
				}
				actor_p->set_viseme_blend_ramp( v->id(), time, v->weight(), ramp_dur );
#else
				float duration = v->duration();
				if( duration <= 0 ) {
					// speech implementation doesn't appear to support durations.
					// using 0.1 transition duration (and start transition early)
					duration = .1f;
					time -= (time_sec)0.05;
				}
				
				command.str( "" );
				command << "char " << actor_id << " viseme " << v->id() << " trap " 
						<< v->weight() << " " 
						<< duration << " " 
						<< v->rampin() << " "
						<< v->rampout() << " ";
				
				sbm_commands.push_back( new SbmCommand( command.str(), time ) );
#endif
				if( LOG_BML_VISEMES ) cout << "command (complete): " << command.str() << endl;
			}
			else
			{
#if ENABLE_DIRECT_VISEME_SCHEDULE

				int n = v->getNumKeys();
				float *curve_info = new float[ 2 * n ];
				srArgBuffer curve_string( v->getCurveInfo() );
				curve_string.read_float_vect( curve_info, 2 * n );
#if 0
				actor_p->set_viseme_blend_curve( v->id(), mcu->time, 1.0f, curve_info, n, 2 );
#else
				actor_p->set_viseme_curve( v->id(), mcu->time + startAt, curve_info, n, 2, 0.1f, 0.1f );
#endif
				delete [] curve_info;

#else
				command.str( "" );
				command << "char " << actor_id << " viseme " << v->id() << " curve " << v->getNumKeys() << ' ' << v->getCurveInfo();
				time_sec time = mcu->time;
				sbm_commands.push_back( new SbmCommand( command.str(), time ) );
#endif
				if( LOG_BML_VISEMES ) cout << "command (complete): " << command.str() << endl;
			}

			////visemes get set a specified time
			//if( seq->insert( time, (char*)(command.str().c_str()) )!=CMD_SUCCESS ) {
			//	strstr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
			//		"Failed to insert viseme \""<<v->id()<<"\" @ "<<time<<endl;
			//}
			time_sec time = v->time();
			if( LOG_BML_VISEMES ) {
				ostringstream echo;
				echo << "echo LOG_BML_VISEMES:\t" << time << ":\t" << command.str();
				sbm_commands.push_back( new SbmCommand( echo.str(), time ) );
			}
		}
	} else {
		LOG("WARNING: BodyPlannerImpl::realizeRequest(..): SpeechRequest has no visemes.");
	}

	// Schedule audio
	if( !audioPlay.empty() ) {
		if( LOG_AUDIO || LOG_BML_VISEMES )
			cout << "DEBUG: BodyPlannerImpl::realizeRequest(..): scheduling request->audioPlay: " << audioPlay << endl;
		// schedule for later
		sbm_commands.push_back( new SbmCommand( audioPlay, startAt ) );
		//if( seq->insert( (float)(audioOffset<0? 0: audioOffset), audioPlay.c_str() ) != CMD_SUCCESS ) {
		//	LOG( "ERROR: BodyPlannerImpl::realizeRequest: insert audio trigger into seq FAILED, msgId=%s\n", bpMsg.msgId ); 
		//}
	} else {
		LOG("WARNING: BodyPlannerImpl::realizeRequest(..): SpeechRequest has no audioPlay command.");
	}

	realize_sequence( sbm_commands, mcu );
}


void BML::SpeechRequest::unschedule( mcuCBHandle* mcu,
	                            BmlRequestPtr request,
	                            time_sec duration )
{
	unschedule_sequence( mcu );

	// Clear visemes
	ostringstream cmd;
	cmd << "char " << request->actor->name << " viseme ALL 0 " << duration;
	mcu->execute_later( cmd.str().c_str(), 0 );

	if( !audioStop.empty() )
		mcu->execute_later( audioStop.c_str() );
	else
		LOG("WARNING: SpeechRequest::unschedule(): unique_id \"%s\": Missing audioStop.", unique_id.c_str());
}
	                            
void BML::SpeechRequest::cleanup( mcuCBHandle* mcu, BmlRequestPtr request )
{
	visemes.clear();
	unschedule_sequence( mcu );

	speech_impl->requestComplete( speech_request_id );
}
