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
#include "sbm/BMLDefs.h"
#include <sb/SBScene.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBCommandManager.h>
#include <sb/SBCharacter.h>
#include <sbm/sbm_speech_audiofile.hpp>

using namespace std;
using namespace BML;
using namespace SmartBody;



// XML Constants


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
	SmartBody::SBScene* scene )
{
	if (!request->actor->face_ct)
	{
		LOG("Character %s does not have a face controller, so cannot create speech.", request->actor->getName().c_str());
		return SpeechRequestPtr();
	}

	const XMLCh* id = xml->getAttribute(BMLDefs::ATTR_ID);
	std::string localId;
	xml_utils::xml_translate(&localId, id);

	request->localId = localId;

	// get the utterance policy: ignore, queue or interrupt
	const XMLCh* policy = xml->getAttribute(BMLDefs::ATTR_POLICY);
	std::string policyStr;
	xml_utils::xml_translate(&policyStr, policy);
	
	vector<SpeechMark> marks;  // Ordered list of named bookmarks

	// Parse <speech> for sync points
	const XMLCh* type = xml->getAttribute( BMLDefs::ATTR_TYPE );
	std::string typeStr;
	xml_utils::xml_translate(&typeStr, type);
	if( type ) {
#if ENABLE_BMLR_SPEECH_REQUEST_CODE
		// [BMLR] text/plain as default type
		if( *type == 0 ) {
			type = VALUE_TEXT_PLAIN;
		}
#endif

		if( XMLString::compareString( type, BML::BMLDefs::VALUE_TEXT_PLAIN )==0 || typeStr.size() == 0) {
#ifndef __ANDROID__
			if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" << BML::BMLDefs::VALUE_TEXT_PLAIN << "\">" << endl;
#endif
			// Search for <tm> sync_points
			DOMElement* child = xml_utils::getFirstChildElement( xml );
			while( child!=NULL ) {
				const XMLCh* tag = child->getTagName();

#if ENABLE_BMLR_SPEECH_REQUEST_CODE
				 // [BMLR] Changed <tm> to <mark> and id="" to name=""
				if( XMLString::compareString( tag, TAG_MARK )==0 ) {
#ifndef __ANDROID__
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <mark>" << endl;
#endif
#else
				if( XMLString::compareString( tag, BMLDefs::TAG_TM )==0 ) {
#ifndef __ANDROID__
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <tm>" << endl;
#endif
#endif

#if ENABLE_BMLR_SPEECH_REQUEST_CODE
					wstring tmId( child->getAttribute( ATTR_NAME ) );
#else
//					wstring tmId( child->getAttribute( BMLDefs::ATTR_ID ) );
					wstring tmId( xml_utils::xml_translate_wide( BMLDefs::ATTR_ID ) );
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
		} else if( XMLString::compareString( type, BMLDefs::VALUE_SSML )==0 ) {
#ifndef __ANDROID__
			if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): <speech type=\"" <<  BMLDefs::VALUE_SSML << "\">" << endl;
#endif
			// Search for <mark> sync_points
			DOMElement* child = xml_utils::getFirstChildElement( xml );
			while( child!=NULL ) {
				const XMLCh* tag = child->getTagName();
				if( tag && XMLString::compareString( tag, BMLDefs::TAG_MARK )==0 ) {
#ifndef __ANDROID__
					if(LOG_SPEECH) wcout << "LOG: SpeechRequest::SpeechRequest(..): Found <mark>" << endl;
#endif
					const XMLCh* tdIdXml = child->getAttribute(BMLDefs::ATTR_NAME);
					wstring tmId = xml_utils::xml_translate_wide(tdIdXml);
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

	/* Removed by AS 2/10/12 - add this if <sync id=""/> from the audiofiles ever needs to be added directly to the sync points

	// convert any <sync id=""/> to synch points
	AudioFileSpeech* audioSpeech = dynamic_cast<AudioFileSpeech*>(cur_speech_impl);
	if (audioSpeech)		
	{
		std::map< SmartBody::RequestId, SmartBody::AudioFileSpeech::SpeechRequestInfo >& speechRequestInfo = 
			audioSpeech->getSpeechRequestInfo();

		std::map< SmartBody::RequestId, SmartBody::AudioFileSpeech::SpeechRequestInfo >::iterator iter = speechRequestInfo.find(speech_request_id);
		if (iter != speechRequestInfo.end())
		{
			SmartBody::AudioFileSpeech::SpeechRequestInfo& info = (*iter).second;
			for(std::map< std::string, float >::iterator markerIter = info.timeMarkers.begin();
				markerIter != info.timeMarkers.end();
				markerIter++)
			{
				std::string markerName = (*markerIter).first;
				float time = (*markerIter).second;
				SpeechMark speechMark(xml_utils::xml_s2w(markerName), time);
				marks.push_back(speechMark);
			}
		}
	}
	*/
	
	SpeechRequestPtr speechResult( new SpeechRequest( unique_id, localId, behav_syncs, cur_speech_impl, cur_speech_impl_backup, speech_request_id, policyStr, marks, request ) );
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
	const std::string& policyOverride,
	const vector<SpeechMark>& marks,
	BmlRequestPtr request
)
:	SequenceRequest( unique_id, localId, syncs_in, 0, 0, 0, 0, 0 ),
	speech_impl( speech_impl ),
	speech_impl_backup( speech_impl_backup ),
	speech_request_id( speech_request_id ),
	trigger( behav_syncs.sync_start()->sync()->trigger.lock() ),
	policy(policyOverride)
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

	// delete phonemes
	count = phonemes.size();
	for( size_t i=0; i<count; ++i )
		delete phonemes[i];
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

void BML::SpeechRequest::processVisemes(std::vector<VisemeData*>* result_visemes, BmlRequestPtr request, float scale)
{
	if (result_visemes == NULL)
		return;

	SBCharacter* character = dynamic_cast<SBCharacter*>(request->actor);
	const std::string& diphoneMap = character->getStringAttribute("diphoneSetName");
	VisemeData* curViseme = NULL;
	VisemeData* prevViseme = NULL;
	VisemeData* nextViseme = NULL;
	std::vector<float> visemeTimeMarkers;
	std::vector<VisemeData*> visemeRawData;
	for ( size_t i = 0; i < (*result_visemes).size(); i++ )
	{
		if (i > 0)
			prevViseme = (*result_visemes)[i - 1];
		if (i < ((*result_visemes).size() - 1))
			nextViseme = (*result_visemes)[i + 1];
		curViseme = (*result_visemes)[i];
		visemeTimeMarkers.push_back(curViseme->time());
		if (prevViseme != NULL)
		{
			SBDiphone* diphone = SmartBody::SBScene::getScene()->getDiphoneManager()->getDiphone(prevViseme->id(), curViseme->id(), diphoneMap);
			float blendIval = 0.0f;
			if (nextViseme != NULL)
				blendIval = nextViseme->time() - prevViseme->time();
			else
				blendIval = 2.0f * (curViseme->time() - prevViseme->time());

			// ad-hoc, blend interval should not be less than 0.1f
//			if (blendIval < 0.2f)
//				blendIval = 0.2f;

			if (diphone)
			{
				const std::vector<std::string>& visemeNames = diphone->getVisemeNames();
				for (int v = 0; v < diphone->getNumVisemes(); v++)
				{
					std::vector<float> curve = diphone->getKeys(visemeNames[v]);
					for (size_t k = 0; k < curve.size(); k++)
					{
						if ((k % 2) == 0)
						{
							curve[k] *= blendIval;
							curve[k] += prevViseme->time();
							curve[k] -= (*result_visemes)[0]->time();
						}
						else
						{
							curve[k] *= scale;
							if (curve[k] > 1.0f)	//clamp to 1
								curve[k] = 1.0f;
						}
					}
					VisemeData* vcopy = new VisemeData( visemeNames[v], 0.0f);
					vcopy->setFloatCurve(curve, curve.size() / 2, 2);
					visemeRawData.push_back(vcopy);
				}
			}
		}
	}

	// get rid of all zero data
	for (size_t i = 0; i < visemeRawData.size(); i++)
	{
		if (visemeRawData[i]->getFloatCurve().size() == 0)
		{
			delete visemeRawData[i];
			visemeRawData[i] = NULL;
			visemeRawData.erase(visemeRawData.begin() + i);
			i--;
		}
	}

	// process the diphone raw data
	std::vector<VisemeData*> visemeProcessedData;
	std::map<std::string, int> isProcessed;
	int counter = 0;
	for (size_t i = 0; i < visemeRawData.size(); i++)
	{
		bool firstTime = true;
		if (isProcessed.find(visemeRawData[i]->id()) == isProcessed.end())
		{
			isProcessed.insert(std::make_pair(visemeRawData[i]->id(), counter));
			counter++;
		}
		else
			firstTime = false;

		int index = isProcessed[visemeRawData[i]->id()];
		if (firstTime)
		{
			VisemeData* newV = new VisemeData(visemeRawData[i]->id(), visemeRawData[i]->time());
			newV->setFloatCurve(visemeRawData[i]->getFloatCurve(), visemeRawData[i]->getFloatCurve().size() / 2, 2);
			visemeProcessedData.push_back(newV);
		}
		else	// stitch the curves
		{
			std::vector<float>& stitchingCurve = visemeRawData[i]->getFloatCurve();
			std::vector<float>& origCurve = visemeProcessedData[index]->getFloatCurve();
			std::vector<float> newCurve(stitchCurve(origCurve, stitchingCurve));
			visemeProcessedData[index]->setFloatCurve(newCurve, newCurve.size() / 2, 2);
		}
	}

	// assign back the viseme
	for ( size_t i = 0; i < (*result_visemes).size(); i++ )
	{
		delete (*result_visemes)[i];
	}
	(*result_visemes).clear();
	//(*result_visemes) = visemeProcessedData;
	
	for (size_t i = 0; i < visemeProcessedData.size(); i++)
	{
		VisemeData* newV = new VisemeData(visemeProcessedData[i]->id(), visemeProcessedData[i]->time());
		smoothCurve(visemeProcessedData[i]->getFloatCurve(), visemeTimeMarkers, character->getDiphoneSmoothWindow());
		newV->setFloatCurve(visemeProcessedData[i]->getFloatCurve(), visemeProcessedData[i]->getFloatCurve().size() / 2, 2);
		(*result_visemes).push_back(newV);
	}
}

std::vector<float> BML::SpeechRequest::stitchCurve(std::vector<float>& c1, std::vector<float>& c2)
{
	int size1 = c1.size();
	int size2 = c2.size();
	if (size1 == 0 || size2 == 0)
	{
		return std::vector<float>();
	}
#if 0
	// copy original curve
	std::vector<float> c3;	

	// handle stitching curve
	// -- finding the boundry
	int s1Id = -1;
	int e1Id = -1;
	int s2Id = -1;
	int e2Id = -1;

	for (int i = 0; i < size2 / 2; ++i)
	{
		if (c2[i] > c1[size1 / 2 - 1])
		{
			s2Id = 0;
			e2Id = i;
			break;
		}
	}
	for (int i = size1 / 2 - 1 ; i >= 0; --i)
	{
		if (c1[i] < c2[0])
		{
			s1Id = i;
			s2Id = size1 / 2 - 1;
			break;
		}
	}
	bool notIntersect = ((s1Id >= 0) && (e1Id >= 0) && (s2Id >= 0) && (e2Id >= 0)) ? false : true;
	if (notIntersect)
	{
		for (int i = 0; i < size1; i++)
			c3.push_back(c1[i]);
		for (int i = 0; i < size2; i++)
			c3.push_back(c2[i]);
		return c3;
	}

	for (int i = 0; i < s1Id * 2; i++)	//push all the data before intersect
		c3.push_back(c1[i]);
	
	int startingId = s1Id;
	for (int i = s2Id; i <= e2Id; ++i)
	{
		for (int j = startingId; j < e1Id; ++j)
		{
			if (c1[j * 2] <= c2[i * 2] && c1[j * 2 + 2] >= c2[i * 2])
			{
				// smooth and insert
				float f = (c2[i * 2] - c1[j * 2]) / (c1[(j + 1) * 2] - c1[j * 2]);
				float curY1 = f * (c1[(j + 1) * 2 + 1] - c1[j * 2 + 1]) + c1[j * 2 + 1];
				float newY2 = curY1 > c2[i * 2 + 1] ? curY1 : c2[i * 2 + 1]; //(curY1 + y2[j]) * 0.5f;
				c3.push_back(c1[j * 2]);
				c3.push_back(c1[j * 2 + 1]);
				c3.push_back(c2[i * 2]);
				c3.push_back(newY2);

				startingId = j + 1;
				break;
			}
		}
	}

	for (int i = e2Id * 2; i < size2; i++)	//push all the data after intersect
		c3.push_back(c2[i]);	

	return c3;
#else
	std::vector<float> x1;
	std::vector<float> y1;
	std::vector<float> x2;
	std::vector<float> y2;
	for (int i = 0; i < size1; i++)
	{
		if ((i % 2) == 0)
			x1.push_back(c1[i]);
		else
			y1.push_back(c1[i]);
	}
	for (int i = 0; i < size2; i++)
	{
		if ((i % 2) == 0)
			x2.push_back(c2[i]);
		else
			y2.push_back(c2[i]);
	}

	std::vector<float> smoothedY1 = y1;
	std::vector<float> smoothedY2 = y2;

#if 1
	// simple algorithm
	// do for second curve
	for (int i = ((int)x1.size() - 1); i > 0; i--)
	{
		if (x1[i] < x2[0])
			break;

		for (size_t j = 0; j < x2.size(); j++)
		{
			if (x1[i] >= x2[j] && x1[i - 1] <= x2[j])
			{
				float f = (x2[j] - x1[i - 1])/ (x1[i] - x1[i - 1]);
				float curY1 = f * (y1[i] - y1[i - 1]) + y1[i - 1];
				float newY2 = curY1 > y2[j] ? curY1 : y2[j]; //(curY1 + y2[j]) * 0.5f;
				smoothedY2[j] = newY2;
			}
		}
	}

	// do for first curve
	for (size_t i = 0; i < (x2.size() - 1); i++)
	{
		for (int j = ((int)x1.size() - 1); j >= 0; j--)
		{
			if (x1[j] < x2[0])
				break;

			if (x2[i] <= x1[j] && x2[i + 1] >= x1[j])
			{
				float f = (x1[j] - x2[i])/ (x2[i + 1] - x2[i]);
				float curY2 = f * (y2[i + 1] - y2[i]) + y2[i];
				float newY1 = curY2 > y1[j] ? curY2 : y1[j]; //(curY2 + y1[j]) * 0.5f;
				smoothedY1[j] = newY1;
			}
		}
	}
#else
	for (size_t i = 0; i < x1.size() - 1; i++)
	{
		for (size_t j = 0; j < x2.size(); j++)
		{
			if (x1[i] <= x2[j] && x1[i + 1] >= x2[j])
			{
				float f = (x2[j] - x1[i])/ (x1[i + 1] - x1[i]);
				float curY1 = f * (y1[i + 1] - y1[i]) + y1[i];
				float newY2 = curY1 > y2[j] ? curY1 : y2[j]; //(curY1 + y2[j]) * 0.5f;
				smoothedY2[j] = newY2;
			}
		}
	}

	// do for first curve
	for (size_t i = 0; i < x2.size() - 1; i++)
	{
		for (size_t j = 0; j < x1.size(); j++)
		{
			if (x2[i] <= x1[j] && x2[i + 1] >= x1[j])
			{
				float f = (x1[j] - x2[i])/ (x2[i + 1] - x2[i]);
				float curY2 = f * (y2[i + 1] - y2[i]) + y2[i];
				float newY1 = curY2 > y1[j] ? curY2 : y1[j]; //(curY2 + y1[j]) * 0.5f;
				smoothedY1[j] = newY1;
			}
		}
	}
#endif

	std::vector<float> newX;
	std::vector<float> newY;

	for (size_t i = 0; i < x1.size(); i++)
	{
		newX.push_back(x1[i]);
		newY.push_back(smoothedY1[i]);
	}

	for (size_t i = 0; i < x2.size(); i++)
	{
		bool isAppending = true;
		for (size_t j = 0; j < newX.size(); j++)
		{
			if (newX[j] >= x2[i])
			{
				newX.insert(newX.begin() + j, x2[i]);
				newY.insert(newY.begin() + j, smoothedY2[i]);
				isAppending = false;
				break;
			}
		}
		if (isAppending)
		{
			newX.push_back(x2[i]);
			newY.push_back(smoothedY2[i]);
		}
	}

	std::vector<float> retFloats;
	for (size_t i = 0; i < newX.size(); i++)
	{
		retFloats.push_back(newX[i]);
		retFloats.push_back(newY[i]);
	}
	return retFloats;
#endif
}


void BML::SpeechRequest::smoothCurve(std::vector<float>& c, std::vector<float>& timeMarkers, float windowSize)
{
	if (windowSize <= 0)
		return;

	std::vector<float> x;
	std::vector<float> y;
	std::vector<bool> markDelete;
	for (size_t i = 0; i < c.size(); i++)
	{
		if ((i % 2) == 0)
		{
			x.push_back(c[i]);
			markDelete.push_back(false);
		}
		else
			y.push_back(c[i]);
	}

	// local smoothing, by window size or phoneme intervals
	for (size_t i = 1; i < timeMarkers.size() - 1; i++)
	{
		// left&right time and window size
		//float leftTime = timeMarkers[i - 1];
		//float rightTime = timeMarkers[i + 1];
		float leftTime = timeMarkers[i] - windowSize / 2;
		float rightTime = timeMarkers[i] + windowSize / 2;

		int leftId = 0;
		int rightId = 0;
		for (size_t j = 0; j < x.size(); j++)
		{
			if (x[j] >= leftTime)
			{
				leftId = j;
				break;
			}
		}
		for (size_t j = 0; j < x.size(); j++)
		{
			if (x[j] >= rightTime)
			{
				rightId = j;
				break;
			}
		}

		std::vector<int> localMaxId;
		for (int j = (leftId + 1); j <= (rightId - 1); j++)
		{
			if ((y[j] - y[j - 1]) > 0 &&
				(y[j] - y[j + 1]) > 0)
			{
				localMaxId.push_back(j);
			}
		}
		if (localMaxId.size() >= 2)
		{
			for (size_t l = 0; l < (localMaxId.size() - 1); l++)
			{
				for (int markId = (localMaxId[l] + 1); markId < (localMaxId[l + 1]); markId++)
					markDelete[markId] = true;
			}
		}
	}

	// global smoothing(by windowsize or phoneme intervals
	/*
	if ((x[x.size() - 1] - x[0]) <=  windowSize)
	{
		std::vector<int> localMaxId;
		for (size_t j = 1; j < (x.size() - 1); j++)
		{
			if ((y[j] - y[j - 1]) > 0 &&
				(y[j] - y[j + 1]) > 0)
			{
				localMaxId.push_back(j);
			}
			if (localMaxId.size() >= 2)
			{
				for (size_t l = 0; l < (localMaxId.size() - 1); l++)
				{
					for (int markId = (localMaxId[l] + 1); markId < (localMaxId[l + 1]); markId++)
						markDelete[markId] = true;
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < x.size(); i++)
		{
			for (size_t t = 0; t < timeMarkers.size(); t++)
			{
				if (x[i] >= timeMarkers[t] && t < (timeMarkers.size() - 3))
				{
					windowSize = (timeMarkers[t + 3] - timeMarkers[t]);
					break;
				}
			}

			std::vector<int> localMaxId;
			for (size_t j = i + 1; j < (x.size() - 1); j++)
			{
				if ((y[j] - y[j - 1]) > 0 &&
					(y[j] - y[j + 1]) > 0)
				{
					localMaxId.push_back(j);
				}

				if ((x[j] - x[i]) > windowSize) // within the window, get rid of all the points between local max
				{
					if (localMaxId.size() >= 2)
					{
						for (size_t l = 0; l < (localMaxId.size() - 1); l++)
						{
							for (int markId = (localMaxId[l] + 1); markId < (localMaxId[l + 1]); markId++)
								markDelete[markId] = true;
						}
		//				i = j;
					}
					break;
				}
			}
		}
	}
	*/

	std::vector<float> newCurve;
	for (size_t i = 0; i < markDelete.size(); i++)
	{
		if (!markDelete[i])
		{
			newCurve.push_back(x[i]);
			newCurve.push_back(y[i]);
		}
	}
	c.clear();
	for (size_t i = 0; i < newCurve.size(); i++)
		c.push_back(newCurve[i]);
}

void BML::SpeechRequest::schedule( time_sec now ) {
	//// TODO: Sync to prior behaviors
	// behav_syncs.applyParentTimes()
	// find set SyncPoints
	// if more than one, warn and ignore least important
	mcuCBHandle& mcu = mcuCBHandle::singleton();

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

	
	time_sec offset = 0;
	if (isTimeSet(sp_start->time))
		offset = sp_start->time - now;

	BmlRequestPtr       request  = trigger->request.lock();
	SbmCharacter* actor    = request->actor;
	string              actor_id = request->actorId;

	// Found speech implementation.  Making request.
	if( !speech_error_msg.empty() ) {
		ostringstream oss;
		oss << "SpeechInterface error: "<<speech_error_msg;
		throw SchedulingException( oss.str().c_str() );
	}

	audioPlay = speech_impl->getSpeechPlayCommand( speech_request_id, actor );
	audioStop = "";//speech_impl->getSpeechStopCommand( speech_request_id, actor );
	if( LOG_AUDIO ) {
		cout << "DEBUG: BML::SpeechRequest::processReply(): audioPlay = " << audioPlay << endl;
		cout << "DEBUG: BML::SpeechRequest::processReply(): audioStop = " << audioStop << endl;
	}

	// save timing;
	time_sec first_open  = TIME_UNSET;  // start of first non-neutral viseme
	time_sec last_open   = TIME_UNSET;  // end of last non-neutral viseme
	time_sec last_viseme = TIME_UNSET;  // end of last viseme
	time_sec longest_viseme = TIME_UNSET;

	// Extract Visemes	
	// first copy -> m_speechRequestInfo parsed the visemes from files
	// second copy -> now getVisems copied it again so it's ready for process
	vector<VisemeData*>* result_visemes = speech_impl->getVisemes( speech_request_id, actor );
	if( !result_visemes ) {
		if (speech_impl_backup) // run the backup speech server if available
			result_visemes = speech_impl->getVisemes( speech_request_id, NULL );
	}

	// Save Phonemes
	for ( size_t i = 0; i < (*result_visemes).size(); i++ )
	{
		VisemeData* v = (*result_visemes)[ i ];
		VisemeData* newV = new VisemeData(v->id(), v->time());
		phonemes.push_back(newV);
	}

	// Process Visemes
	if (actor && actor->isDiphone()) // if use diphone, reconstruct the curves
		processVisemes(result_visemes, request, actor->getDiphoneScale());			

	if (result_visemes)
	{
		//visemes = *result_visemes;  // Copy contents
		// third copy -> to drop any visemes that don't exceed the viseme threshold
		for ( size_t i = 0; i < (*result_visemes).size(); i++ )
		{
			VisemeData* v = (*result_visemes)[ i ];

			// drop any visemes that don't exceed the viseme threshold
			if (v->duration() < actor->getMinVisemeTime() && !actor->isDiphone())
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
			if ( !isTimeSet(longest_viseme))
				longest_viseme = v->duration();
			else if (longest_viseme < v->duration())
				longest_viseme = v->duration();
		}
	} else {

		if( LOG_SPEECH )
		{
			std::stringstream strstr;
			strstr << "WARNING: BodyPlannerImpl::speechReply(): speech.getVisemes( " << speech_request_id << " ) returned NULL.";
			LOG(strstr.str().c_str());
		}
	}

	if (last_open < longest_viseme) // ensures that curve mode will send bml:end at the proper time
	{
		last_open = longest_viseme;
		last_viseme = longest_viseme;
	}

	time_sec start_time = now + offset;
	if (isTimeSet(sp_start->time))
		start_time = sp_start->time;
	else
		sp_start->time = start_time;
	//  Set core sync_point times
	
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

//			float audioTime = speech_impl->getMarkTime( speech_request_id, wb_id.c_str() );

			XMLCh *xml_ch_p = xml_utils::xmlch_translate( xml_utils::xml_w2s( wb_id ) );
			float audioTime = speech_impl->getMarkTime( speech_request_id, xml_ch_p );
			xml_utils::xmlch_release( &xml_ch_p );
			
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
#ifndef __ANDROID__
				if( LOG_SYNC_POINTS ) wcout << "   Wordbreak SyncPoint \"" << wb_id << "\" @ " << audioTime << endl;
#endif
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

void BML::SpeechRequest::realize_impl( BmlRequestPtr request, SmartBody::SBScene* scene )
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
	const string& actor_id = request->actor->getName();

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
			time_sec time = (time_sec)( v->time() + startAt );
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
				string cmd_str = command.str();
				SbmCommand *cmd = new SbmCommand( cmd_str, time );
				sbm_commands.push_back( cmd );
			}
			else if (!v->isCurveMode())
			{
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
				
				string cmd_str = command.str();
				SbmCommand *cmd = new SbmCommand( cmd_str, time );
				sbm_commands.push_back( cmd );
//				sbm_commands.push_back( new SbmCommand( command.str(), time ) );
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
				string cmd_str = command.str();
				SbmCommand *cmd = new SbmCommand( cmd_str, time );
				sbm_commands.push_back( cmd );
//				sbm_commands.push_back( new SbmCommand( command.str(), time ) );
#endif
				if( LOG_BML_VISEMES ) cout << "command (complete): " << command.str() << endl;
			}

			////visemes get set a specified time
			//if( seq->insert( time, (char*)(command.str().c_str()) )!=CMD_SUCCESS ) {
			//	strstr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
			//		"Failed to insert viseme \""<<v->id()<<"\" @ "<<time<<endl;
			//}
			if( LOG_BML_VISEMES ) {
				ostringstream echo;
				echo << "echo LOG_BML_VISEMES:\t" << time << ":\t" << command.str();
				string cmd_str = echo.str();
				SbmCommand *cmd = new SbmCommand( cmd_str, time );
				sbm_commands.push_back( cmd );
//				sbm_commands.push_back( new SbmCommand( echo.str(), time ) );
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
		sbm_commands.push_back( new SbmCommand( audioPlay, startAt + request->actor->get_viseme_sound_delay() ) );
		//if( seq->insert( (float)(audioOffset<0? 0: audioOffset), audioPlay.c_str() ) != CMD_SUCCESS ) {
		//	LOG( "ERROR: BodyPlannerImpl::realizeRequest: insert audio trigger into seq FAILED, msgId=%s\n", bpMsg.msgId ); 
		//}
	} else {
		LOG("WARNING: BodyPlannerImpl::realizeRequest(..): SpeechRequest has no audioPlay command.");
	}

	realize_sequence( sbm_commands, scene );
}


void BML::SpeechRequest::unschedule(  SmartBody::SBScene* scene,
	                            BmlRequestPtr request,
	                            time_sec duration )
{
	unschedule_sequence( scene );

	// Clear visemes
	ostringstream cmd;
	cmd << "char " << request->actor->getName() << " viseme ALL 0 " << duration;
	SmartBody::SBScene::getScene()->getCommandManager()->execute_later( cmd.str().c_str(), 0 );

	if( !audioStop.empty() )
		SmartBody::SBScene::getScene()->getCommandManager()->execute_later( audioStop.c_str(), request->actor->get_viseme_sound_delay() );
	else
		LOG("WARNING: SpeechRequest::unschedule(): unique_id \"%s\": Missing audioStop.", unique_id.c_str());
}
	                            
void BML::SpeechRequest::cleanup( SmartBody::SBScene* scene, BmlRequestPtr request )
{
	visemes.clear();
	unschedule_sequence( scene );

	speech_impl->requestComplete( speech_request_id );
}
