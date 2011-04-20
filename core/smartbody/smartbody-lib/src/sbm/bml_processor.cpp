/*
 *  bml_processor.cpp - part of SmartBody-lib
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
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 *      Corne Versloot, while at USC
 */

#include "vhcl.h"
#include <stdlib.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <map>

#include "sr_arg_buff.h"

#include <xercesc/util/XMLStringTokenizer.hpp>

#include "mcontrol_util.h"

#include "bml_exception.hpp"
#include "bml_processor.hpp"
#include "bml_xml_consts.hpp"

#include "bml_animation.hpp"
#include "bml_event.hpp"
#include "bml_face.hpp"
#include "bml_gaze.hpp"
#include "bml_reach.hpp"
#include "bml_constraint.hpp"
#include "bml_bodyreach.hpp"
#include "bml_grab.hpp"
#include "bml_interrupt.hpp"
#include "bml_speech.hpp"
#include "bml_locomotion.hpp"
#include "bml_quickdraw.hpp"
#include "bml_general_param.hpp"

#include "me_ct_examples.h"
#include "me_ct_gaze.h"

using namespace std;
using namespace BML;
using namespace SmartBody;


const bool LOG_METHODS				= false;
const bool BML_LOG_INTERRUPT        = false;

const double SQROOT_2 = 1.4142135623730950488016887242097;


///////////////////////////////////////////////////////////////////////////////
//  Implementation

// XMLStrings (utf-16 character arrays) for parsing vrSpeak's XML
const XMLCh TAG_ACT[]		= L"act";
const XMLCh TAG_BML[]       = L"bml";
const XMLCh TAG_BODY[]      = L"body";
const XMLCh TAG_REQUIRED[]  = L"required";
#ifdef BMLR_BML2ANIM
const XMLCh TAG_POSTURE[]   = L"posture"; // [BMLR] For bml2anim postures
#endif
const XMLCh TAG_HEAD[]      = L"head";
const XMLCh TAG_TM[]        = L"tm";
const XMLCh TAG_MARK[]      = L"mark";


const XMLCh TAG_SBM_COMMAND[] = L"sbm:command";

// Deprecated behavior tags
const XMLCh TAG_ANIMATION[] = L"animation";
const XMLCh TAG_EVENT[]     = L"event";

const XMLCh TAG_PANIMATION[] = L"panimation";
const XMLCh TAG_REACH[] = L"sbm:reach";

// XMLStrings (utf-16 character arrays) for parsing vrSpeak's XML
const XMLCh ATTR_SPEAKER[]      = L"speaker";
const XMLCh ATTR_ADDRESSEE[]    = L"addressee";
const XMLCh ATTR_CONTENTTYPE[]  = L"contenttype";
const XMLCh ATTR_LANG[]         = L"lang";
const XMLCh ATTR_TID[]          = L"tid";
const XMLCh ATTR_POSTURE[]      = L"posture";
const XMLCh ATTR_REPEATS[]      = L"repeats";
const XMLCh ATTR_AMOUNT[]       = L"amount";
const XMLCh ATTR_VELOCITY[]     = L"velocity";
const XMLCh ATTR_TARGET[]       = L"target";
const XMLCh ATTR_ANGLE[]        = L"angle";
const XMLCh ATTR_DIRECTION[]    = L"direction";
const XMLCh ATTR_ROLL[]         = L"sbm:roll";
const XMLCh ATTR_SMOOTH[]       = L"sbm:smooth";

////// XML Direction constants
// Angular (gaze) and orienting (head)
const XMLCh DIR_RIGHT[]        = L"RIGHT";
const XMLCh DIR_LEFT[]         = L"LEFT";
const XMLCh DIR_UP[]           = L"UP";
const XMLCh DIR_DOWN[]         = L"DOWN";
// Angular only
const XMLCh DIR_UPRIGHT[]      = L"UPRIGHT";
const XMLCh DIR_UPLEFT[]       = L"UPLEFT";
const XMLCh DIR_DOWNRIGHT[]    = L"DOWNRIGHT";
const XMLCh DIR_DOWNLEFT[]     = L"DOWNLEFT";
const XMLCh DIR_POLAR[]        = L"POLAR";
// Orienting only
const XMLCh DIR_ROLLRIGHT[]    = L"ROLLRIGHT";
const XMLCh DIR_ROLLLEFT[]     = L"ROLLLEFT";






///////////////////////////////////////////////////////////////////////
//  Helper Functions
namespace BML {
	string buildRequestId( const SbmCharacter* character, std::string messageId ) {
		ostringstream out;
		out << character->name << "|" << messageId;  // pipe is unlikely to be found in either field
		return out.str();
	}

	string buildSpeechKey( const SbmCharacter* actor, SmartBody::RequestId requestId ) {
		ostringstream speechKey;
		speechKey << actor->name << requestId;  // no space / single token
		return speechKey.str();
	}

	void bml_error(
		const char* agent_id,
		const char* message_id,
		const char* error_msg,
		mcuCBHandle *mcu )
	{
		//  Let's not error on our error messages.  Be thorough.
		if( agent_id==NULL || agent_id[0]=='\0' )
			agent_id = "?";
		if( message_id==NULL || message_id[0]=='\0' )
			message_id = "?";
		if( error_msg==NULL || error_msg[0]=='\0' )
			error_msg = "INVALID_ERROR_MESSAGE";

		LOG("WARNING: bml_error(..): %s (agent \"%s\", message id \"%s\")", error_msg, agent_id, message_id);

		// Old vrSpeakFailed form (sans recipient)
		ostringstream buff;
		buff << agent_id << " RECIPIENT " << message_id << " " << error_msg;
		// TODO: Avoid singleton
		mcu->vhmsg_send( "vrSpeakFailed", buff.str().c_str() );

		// New vrAgentBML form...
		ostringstream buff2;
#if USE_RECIPIENT
		buff2 << agent_id << " RECIPIENT " << message_id << " end error " << error_msg;
#else
		buff2 << agent_id << " " << message_id << " end error " << error_msg;
#endif
		// TODO: Avoid singleton
		mcu->vhmsg_send( "vrAgentBML", buff2.str().c_str() );
	}
};


///////////////////////////////////////////////////////////////////////////////
// BodyPlannerMsg
#if USE_RECIPIENT
BML::Processor::BMLProcessorMsg::BMLProcessorMsg( const char *actorId, const char *recipientId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, const char* args )
:	actorId(actorId),
	recipientId(recipientId),
#else
BML::Processor::BMLProcessorMsg::BMLProcessorMsg( const char *actorId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, const char* args )
:	actorId(actorId),
#endif
	msgId(msgId),
	actor(actor),
	xml(xml),
	requestId( buildRequestId( actor, msgId ) ),
	args( args )  // constructor does handle NULL
{}

#if USE_RECIPIENT
BML::Processor::BMLProcessorMsg::BMLProcessorMsg( const char *actorId, const char *recipientId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, srArgBuffer& args )
:	actorId(actorId),
	recipientId(recipientId),
#else
BML::Processor::BMLProcessorMsg::BMLProcessorMsg( const char *actorId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, srArgBuffer& args )
:	actorId(actorId),
#endif
	msgId(msgId),
	actor(actor),
	xml(xml),
	requestId( buildRequestId( actor, msgId ) ),
	args( args.read_remainder_raw() )
{}

BML::Processor::BMLProcessorMsg::~BMLProcessorMsg() {
	// char* memory is owned by arg buffer
	// *xml memory may still be in use (?)
	// agent is just a pointer to mcu managed SbmCharacter
	// requestId has its own destructor
}



///////////////////////////////////////////////////////////////////////////////
//  BML Processor


BML::Processor::Processor()
:	auto_print_controllers( false ),
	auto_print_sequence( false ),
	log_syncpoints( false ),
	warn_unknown_agents( true ),
	ct_speed_min( CONTROLLER_SPEED_MIN_DEFAULT ),
	ct_speed_max( CONTROLLER_SPEED_MAX_DEFAULT ),
	requestcb(NULL)
{
	try {
		xmlParser = boost::shared_ptr<XercesDOMParser>( new XercesDOMParser() );

		xmlErrorHandler = new HandlerBase();
		xmlParser->setErrorHandler( xmlErrorHandler );
		//ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
		//xmlParser->setErrorHandler(errHandler);
		//xmlParser->setErrorHandler( new HandlerBase() );
	} catch( const XMLException& e ) {
		LOG("ERROR: BML Processor:  XMLException during constructor: %s", e.getMessage());
	} catch( const std::exception& e ) {
		LOG("ERROR: BML Processor:  std::exception during constructor: %s", e.what());
	} catch(...) {
		LOG("ERROR: BML Processor:  UNKNOWN EXCEPTION DURING CONSTRUCTOR.     <<==================");
	}
}

void BML::Processor::registerRequestCallback(void (*cb)(BmlRequest* request, void* data), void* data)
{
	requestcb = cb;
	requestData = data;
}



BML::Processor::~Processor()
{
	delete xmlErrorHandler;
	xmlErrorHandler = NULL;
}


void BML::Processor::reset() {
	bml_requests.clear();
}



BmlRequestPtr BML::Processor::createBmlRequest(
	const SbmCharacter* agent,
	const std::string & actorId,
	const std::string & requestId,
#if USE_RECIPIENT
	const std::string & recipientId,
#endif
	const std::string & msgId,
	const DOMDocument* xmlDoc)
{
#if USE_RECIPIENT
	BmlRequestPtr request( new BmlRequest( agent, actorId, requestId, recipientId, msgId ) );
#else
	BmlRequestPtr request( new BmlRequest( agent, actorId, requestId, msgId, xmlDoc) );
#endif
	request->init( request );  // passes the smart pointer back to BmlRequest so is can create a weak copy for later use.

	return request;
}




void BML::Processor::bml_request( BMLProcessorMsg& bpMsg, mcuCBHandle *mcu ) {
    if(LOG_METHODS) cout<<"BodyPlannerImpl::vrSpeak(..)"<<endl;

	int suppress = 2;
	//BmlRequest *request = requests.lookup( bpMsg.requestId.c_str() );  // srHashMap

	MapOfBmlRequest::iterator result = bml_requests.find( bpMsg.requestId );
    if( result != bml_requests.end() ) {
		LOG("Duplicate BML Request Message ID: %s", bpMsg.requestId.c_str());
		//  TODO: call vrSpeakFailed?  How do we show we're not failing on the original agent/message-id request?
		return;
    }

   const DOMDocument* xml = bpMsg.xml;
	DOMElement* root = xml->getDocumentElement();  
	if( XMLString::compareString( root->getTagName(), TAG_ACT )!=0 )
		LOG("WARNING: BodyPlanner: Expected <act> tag as XML root.");


	DOMElement* child = xml_utils::getFirstChildElement( root );
	DOMElement* bmlElem = NULL;
	while( child!=NULL ) {
		const XMLCh *tag = child->getTagName();
		if( XMLString::compareString( tag, TAG_BML )==0 ) {
			bmlElem = child;
			break;
		} // else ingore other tags for now

		child = xml_utils::getNextElement( child );
	}

	if( bmlElem ) {
#if USE_RECIPIENT
		BmlRequestPtr request( createBmlRequest( bpMsg.actor, bpMsg.actorId, bpMsg.requestId, string(bpMsg.recipientId), string(bpMsg.msgId) ) );
#else
		BmlRequestPtr request( createBmlRequest( bpMsg.actor, bpMsg.actorId, bpMsg.requestId, string(bpMsg.msgId), xml ) );
#endif
		try {
 			parseBML( bmlElem, request, mcu );
			bml_requests.insert( make_pair( bpMsg.requestId, request ) );
			
		
			if( !( request->speech_request ) ) {
				// realize immediately
				request->realize( this, mcu );
			}

			//if (requestcb)
			//	requestcb(request.get(), requestData);

		} catch( BML::ParsingException& e ) {
			ostringstream oss;
			oss << e.type() << ": " << e.what();

			LOG("ERROR: BML::Processor::bml_request(): %s", oss.str().c_str());
			bml_requests.erase( bpMsg.requestId );  // No further references if we're going to fail.
			bml_error( bpMsg.actorId, bpMsg.msgId, oss.str().c_str(), mcu );
		}
	} else {
		const char* message = "No BML element found.";
		LOG("ERROR: BML::Processor::bml_request(): %s", message);
		bml_error( bpMsg.actorId, bpMsg.msgId, message, mcu );
	}
}

void BML::Processor::parseBehaviorGroup( DOMElement *group, BmlRequestPtr request, mcuCBHandle* mcu,
                                         size_t& behavior_ordinal, bool required ) {

	// look for BML behavior command tags
	DOMElement*  child = xml_utils::getFirstChildElement( group );
	while( child!=NULL ) {
		const XMLCh *tag = child->getTagName();  // Grand Child (behavior) Tag
		if( XMLString::compareString( tag, TAG_REQUIRED )==0 ) {
			parseBehaviorGroup( child, request, mcu, behavior_ordinal, true );
		} else {
			const XMLCh* id  = child->getAttribute( ATTR_ID );
			string unique_id = request->buildUniqueBehaviorId( tag, id, ++behavior_ordinal );

			// Load SyncPoint references
			BehaviorSyncPoints behav_syncs;  // TODO: rename (previous this was a TimeMarkers class)	
			behav_syncs.parseStandardSyncPoints( child, request, unique_id );

			BehaviorRequestPtr behavior;

			// Parse behavior specifics
			//// TODO: tag name -> behavior factory map
			if( XMLString::compareString( tag, TAG_SBM_SPEECH )==0 || XMLString::compareString( tag, TAG_SPEECH )==0 ) {
				// TEMPORARY: <speech> can only be the first behavior
				if( behavior_ordinal == 1 ) {
					// This speech is the first
					BML::SpeechRequestPtr speechPtr =  parse_bml_speech( child, unique_id, behav_syncs, required, request, mcu );
					if (!speechPtr)
						return;
					SpeechRequestPtr speech_request(speechPtr);
					if( speech_request ) {
						behavior = speech_request;

						// Store reference to the speech behavior in the speeches map for later processing
						// TODO: generalize this to TriggerEvent handling
						string speechKey = buildSpeechKey( request->actor, speech_request->speech_request_id );
						bool insert_success = speeches.insert( make_pair( speechKey, speech_request ) ).second;  // store for later reply
						if( !insert_success ) {
							LOG("ERROR: BML::Processor.vrSpeak(..): BmlProcessor::speehces already contains an entry for speechKey \"%s\".  Cannot process speech behavior.  Failing BML request.  (This error should not occur. Let Andrew know immeidately.)", speechKey.c_str());
							// TODO: Send vrSpeakFailed
						}

						request->speech_request = speech_request;
					} else {
						//  Speech is always treated as required
						std::wstringstream wstrstr;
						wstrstr<<"ERROR: BML::Processor::parseBML(): Failed to parse <"<<tag<<"> tag.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
					}
				} else {
					std::wstringstream wstrstr;
					wstrstr<<"ERROR: BML <"<<tag<<"> must be first behavior.";
					LOG(convertWStringToString(wstrstr.str()).c_str());
					LOG("\t(unique_id \"%s\".", unique_id.c_str()); // unique id is not multibyte, and I'm lazily refusing to convert just to put it on the same line).
					
				}
			} else if( XMLString::compareString( tag, TAG_ANIMATION )==0 ) {
				// DEPRECATED FORM
				behavior = parse_bml_animation( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_SBM_ANIMATION )==0 ) {
				behavior = parse_bml_animation( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_SBM_PANIMATION )==0 ) {
				behavior = parse_bml_panimation( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_BODY )==0 ) {
				behavior = parse_bml_body( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_HEAD )==0 ) {
				behavior = parse_bml_head( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_FACE )==0 ) {
				behavior = parse_bml_face( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_GAZE )==0 ) {
				behavior = /*BML::*/parse_bml_gaze( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_REACH )==0 ) {
				behavior = /*BML::*/parse_bml_reach( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_CONSTRAINT )==0 ) {
				behavior = /*BML::*/parse_bml_constraint( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_BODYREACH )==0 ) {
				behavior = /*BML::*/parse_bml_bodyreach( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_GRAB )==0 ) {
				behavior = /*BML::*/parse_bml_grab( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_EVENT )==0 ) {
				// DEPRECATED FORM
				behavior = parse_bml_event( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_PARAM )==0 ) {
				behavior = parse_bml_param( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_SBM_EVENT )==0 ) {
				behavior = parse_bml_event( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_QUICKDRAW )==0 ) {
				behavior = parse_bml_quickdraw( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_SPEECH )==0 ) {
				LOG("ERROR: BML::Processor::parseBML(): <speech> BML tag must be first behavior (TEMPORARY HACK).");
			} else if( XMLString::compareString( tag, TAG_LOCOTMOTION )==0 ) {
				behavior = parse_bml_locomotion( child, unique_id, behav_syncs, required, request, mcu );
			} else if( XMLString::compareString( tag, TAG_INTERRUPT )==0 ) {
				behavior = parse_bml_interrupt( child, unique_id, behav_syncs, required, request, mcu );
#ifdef BMLR_BML2ANIM
			// [BMLR]  Note that this brace closes out the if statement above
			}

			// [BMLR]
			if (behavior == NULL) {
				// [BMLR] support for bml to animations
				behavior = parse_bml_to_anim(child, unique_id, behav_syncs, required, request, mcu);
				if( behavior != NULL )
					request->registerBehavior( id, behavior );
				else
					wcerr<<"WARNING: BodyPlannerImpl: <"<<tag<<"> BML tag unrecognized or unsupported."<<endl;
			}
#else
			} else {
				std::wstringstream wstrstr;
				wstrstr<<"WARNING: BML::Processor::parseBML(): <"<<tag<<"> BML tag unrecognized or unsupported.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
			}
#endif
			

			if( behavior != NULL ) {
				behavior->required = required;
				request->registerBehavior( id, behavior );
			} else if( required ) {
				char* ascii_tag = XMLString::transcode( tag );

				ostringstream err_msg;
				err_msg << "Required behavior <" <<ascii_tag;
				if( id && id[0]!='\0' ) {
					char* ascii_id  = XMLString::transcode( id );
					err_msg << " id=\""<<ascii_id<<"\"";
					delete [] ascii_id;
				}
				err_msg << "> (behavior #"<<behavior_ordinal<<") failed to parse.";
				
				delete [] ascii_tag;

				throw BML::BmlException( err_msg.str().c_str() );
			}
		}

		child = xml_utils::getNextElement( child );
	}
}

void BML::Processor::parseBML( DOMElement *bmlElem, BmlRequestPtr request, mcuCBHandle *mcu ) {
	size_t behavior_ordinal	= 0;

	parseBehaviorGroup( bmlElem, request, mcu, behavior_ordinal, false );

	if( behavior_ordinal==0 ) { // No change
		// uncomment the following to see any bml being processed that does not contain any behaviors
		/*
		LOG("WARNING: BML \"%s\" does not contain any behaviors!", request->msgId.c_str());
		// dump the xml
		XMLCh tempStr[100];
		XMLString::transcode("LS", tempStr, 99);
		DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
		DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
		XMLCh* xmlOutput = theSerializer->writeToString(request->doc);
		theSerializer->release();
		std::wstring xmlStrWide = xmlOutput;
		std::string xmlStr(xmlStrWide.begin(), xmlStrWide.end());
		
		LOG("%s", xmlStr.c_str());
		*/
		return;
	}

//	// TEMPORARY: <speech> can only be the first behavior
//	if( XMLString::compareString( tag, TAG_SBM_SPEECH )==0 || XMLString::compareString( tag, TAG_SPEECH )==0 ) {
////// Old code
////		const XMLCh*     speechId;
////		speechId = child->getAttribute( ATTR_ID );
////
////		string unique_id = request->buildUniqueBehaviorId( tag, id, ++behavior_ordinal );
////		SpeechRequestPtr speech( new SpeechRequest( unique_id, child, speechId, request ) );
////		request->registerBehavior( speechId, speech );
////
////		child = xml_utils::getNextElement( child );
//
//		string unique_id = request->buildUniqueBehaviorId( tag, id, ++behavior_ordinal );
//
//		BehaviorSyncPoints behav_syncs;
//		behav_syncs.parseStandardSyncPoints( child, request, unique_id );
//
//		SpeechRequestPtr speech_request( parse_bml_speech( child, unique_id, behav_syncs, request, mcu ) );
//		if( speech_request ) {
//			request->registerBehavior( id, speech_request );
//
//			// Store reference to the speech behavior in the speeches map for later processing
//			// TODO: generalize this to TriggerEvent handling
//			string speechKey = buildSpeechKey( request->actor, speech_request->speech_request_id );
//			bool insert_success = speeches.insert( make_pair( speechKey, speech_request ) ).second;  // store for later reply
//			if( !insert_success ) {
//				strstr << "ERROR: BML::Processor.vrSpeak(..): BmlProcessor::speehces already contains an entry for speechKey \"" << speechKey << "\".  Cannot process speech behavior.  Failing BML request.  (This error should not occur. Let Andrew know immeidately.)"  << endl;
//				// TODO: Send vrSpeakFailed
//			}
//
//			request->speech_request = speech_request;
//		} else {
//			//  Speech is always treated as required
//			wcerr<<"ERROR: BML::Processor::parseBML(): Failed to parse <"<<tag<<"> tag."<<endl;
//		}
//
//		child = xml_utils::getNextElement( child );
//	}

	if( LOG_SYNC_POINTS ) {
		cout << "WARNING: LOG_SYNC_POINTS is broken.  Please fix!!" << endl;
		//cout << "TIDs:";
		//SyncPoint* sp = trigger->start;
		//while( sp != NULL ) {
		//	const XMLCh *name = sp->name;
		//	if( name )
		//		wcout << " " << name;
		//	sp = sp->next;
		//}
		//cout<<endl;

		//cout << "Behaviors:";
		//int numGestures = request->behaviors.size();
		//for( int i=0; i<numGestures; ++i )
		//	wcout << "  " << request->behaviors[i]->toString() << endl;
	}
}

BehaviorRequestPtr BML::Processor::parse_bml_body( DOMElement* elem, std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
	
	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	const XMLCh* postureName = elem->getAttribute( ATTR_POSTURE );
	if( postureName && *postureName != 0 ) {
		// Look up pose
		const char* ascii_pose_id = xml_utils::asciiString(postureName);
		string pose_id = ascii_pose_id;
		delete [] ascii_pose_id;
		std::map<std::string, SkPosture*>::iterator postureIter =  mcu->pose_map.find(pose_id);
		if( postureIter !=  mcu->pose_map.end()) {
			SkPosture* posture = (*postureIter).second;
			MeCtPose* poseCt = new MeCtPose();
			poseCt->name( posture->name() );  // TODO: include BML act and behavior ids
			poseCt->init( *posture );

			return BehaviorRequestPtr( new PostureRequest( unique_id, localId, poseCt, 1, request->actor, behav_syncs ) );
		} else {
			// Check for a motion (a motion texture, or motex) of the same name
			std::map<std::string, SkMotion*>::iterator motionIter = mcu->motion_map.find(pose_id);
			if (motionIter != mcu->motion_map.end())
			{
				SkMotion* motion = (*motionIter).second;
				MeCtMotion* motionCt = new MeCtMotion();
				motionCt->name( motion->name() );  // TODO: include BML act and behavior ids
				motionCt->init( motion );
				motionCt->loop( true );

				PostureRequest * posture_new = new PostureRequest( unique_id, localId, motionCt, 1, request->actor, behav_syncs );
				posture_new->set_persistent(true);

				return BehaviorRequestPtr( posture_new );
			} else {
				std::wstringstream wstrstr;
				wstrstr<<"WARNING: BML::Processor::parse_bml_body(): <body>: posture=\""<<postureName<<"\" not loaded; ignoring <body>.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return BehaviorRequestPtr();  // a.k.a., NULL
			}
		}
	} else {
		LOG("WARNING: BML::Processor::parse_bml_body(): <body> missing posture = attribute; ignoring <body>.");
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}

BehaviorRequestPtr BML::Processor::parse_bml_head( DOMElement* elem, std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
	
	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	
	const XMLCh* tag      = elem->getTagName();
	const XMLCh* attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && *attrType != 0 ) {
        int type = -1;

        if( XMLString::compareIString( attrType, L"nod" )==0 ) {
			type = BML::HEAD_NOD;
        } else if( XMLString::compareIString( attrType, L"shake" )==0 ) {
            type = BML::HEAD_SHAKE;
        } else if( XMLString::compareIString( attrType, L"toss" )==0 ) {
            type = BML::HEAD_TOSS;
        } else if( XMLString::compareIString( attrType, L"orient" )==0 ) {
            type = BML::HEAD_ORIENT;
        }
        switch( type ) {
            case BML::HEAD_NOD:
            case BML::HEAD_SHAKE: {
                const XMLCh* attrRepeats = elem->getAttribute( ATTR_REPEATS );
                float repeats = 1;  // default to one complete cycle
                if( attrRepeats && *attrRepeats != 0 )
                    repeats = xml_utils::xcstof( attrRepeats );

                const XMLCh* attrAmount = elem->getAttribute( ATTR_AMOUNT );
                float amount = 0.5;  // default to a moderate amount.  Range 0.0 to 1.0
                if( attrAmount && *attrAmount != 0 )
                    amount = xml_utils::xcstof( attrAmount );

                const XMLCh* attrVelocity = elem->getAttribute( ATTR_VELOCITY );
                float velocity = 1;  // default to one cycle per second
                if( attrVelocity && *attrVelocity != 0 )
                    velocity = xml_utils::xcstof( attrVelocity );

				const XMLCh* attrSmooth = elem->getAttribute( ATTR_SMOOTH );
                float smooth = .5;  
                if( attrSmooth && *attrSmooth != 0 )
                    smooth = xml_utils::xcstof( attrSmooth );

                float duration = velocity * repeats;

                return BehaviorRequestPtr( new NodRequest( unique_id,
														   localId,
				                                           (NodRequest::NodType) type,
												           repeats, velocity, amount, smooth,
                                                           request->actor,
                                                           behav_syncs ) );
            }

			case BML::HEAD_ORIENT: {
				const XMLCh* direction = elem->getAttribute( ATTR_DIRECTION );
				const XMLCh* target    = elem->getAttribute( ATTR_TARGET );
				const XMLCh* angle     = elem->getAttribute( ATTR_ANGLE );

				if( target && *target != 0 ) {
					// TODO
					std::wstringstream wstrstr;
					wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a target.  Ignoring behavior.";
					LOG(convertWStringToString(wstrstr.str()).c_str());
					return BehaviorRequestPtr();  // a.k.a., NULL
				} else if( direction && *direction != 0 ) {
					if( XMLString::compareIString( direction, DIR_RIGHT )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_RIGHT<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else if( XMLString::compareIString( direction, DIR_LEFT )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_LEFT<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else if( XMLString::compareIString( direction, DIR_UP )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_UP<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else if( XMLString::compareIString( direction, DIR_DOWN )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_DOWN<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else if( XMLString::compareIString( direction, DIR_ROLLRIGHT )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_ROLLRIGHT<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else if( XMLString::compareIString( direction, DIR_ROLLRIGHT )==0 ) {
						// TODO
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_ROLLRIGHT<<"\".  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					} else {
						std::wstringstream wstrstr;
						wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unrecognized direction \""<<direction<<"\" in <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">.  Ignoring behavior.";
						LOG(convertWStringToString(wstrstr.str()).c_str());
						return BehaviorRequestPtr();  // a.k.a., NULL
					}

					// TODO
					std::wstringstream wstrstr;
					wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction.  Ignoring behavior.";
					LOG(convertWStringToString(wstrstr.str()).c_str());
					return BehaviorRequestPtr();  // a.k.a., NULL
				} else {
					std::wstringstream wstrstr;
					wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> requires a target or a direction attribute.  Ignoring behavior.";
					LOG(convertWStringToString(wstrstr.str()).c_str());
					return BehaviorRequestPtr();  // a.k.a., NULL
				}
			}

			case BML::HEAD_TOSS:
				{
				std::wstringstream wstrstr;
				wstrstr << "WARNING: BML::Processor::parse_bml_head(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">.  Ignoring behavior.";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return BehaviorRequestPtr();  // a.k.a., NULL
				}

			default:
				{
				std::wstringstream wstrstr;
                wstrstr << "WARNING: BML::Processor::parse_bml_head(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unknown type value, ignore command";
				LOG(convertWStringToString(wstrstr.str()).c_str());
				return BehaviorRequestPtr();  // a.k.a., NULL
				}
        }
    } else {
		std::wstringstream wstrstr;
        wstrstr << "WARNING: BML::Processor::parse_bml_head(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute.";
		LOG(convertWStringToString(wstrstr.str()).c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
    }
}

#ifdef BMLR_BML2ANIM
// [BMLR] Reads the bml2anim.xml file placed in the mepath directory and maps bml elements that are not supported 
// by motion controllers to animations
// for example: 
// <gesture type="beat"/>
// is not handled by a motion controller and therefore this code is run..
// It then reads the file and finds this line:
// <gesture type="beat">CrossedArms_RArm_LowBeat</gesture>
// and plays the CrossedArms_RArm_LowBeat.skm animation
BML::BehaviorRequestPtr BML::Processor::parse_bml_to_anim( DOMElement* elem, std::string& unique_id,  BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {

	if (bml2animText.empty())
	{			
		std::string dir;
		mcu->me_paths.reset();

		while ((dir = mcu->me_paths.next_path()) != "")
		{
			std::string filename = "";
			filename.append(dir);
			filename.append("/bml2anim.xml");
		
			std::ifstream thefile;
			thefile.open(filename.c_str());

			if (thefile)
			{
				while (!thefile.eof())
				{
					string line;
					getline(thefile, line);
					bml2animText += line;
				}

				thefile.close();
			}
			else
				LOG("Unable to open bml2anim.xml in mepath");			
		}
		
	}

	if (bml2animText.length() > 0) {
		// setup xml parser
		XercesDOMParser *Prser;
		Prser = new XercesDOMParser();
		Prser->setErrorHandler( new HandlerBase() );

		// find the correct animation
		DOMDocument* textXml = xml_utils::parseMessageXml( Prser, (char*)bml2animText.c_str() );
      if (textXml == NULL)
      {
         LOG("BML::Processor::parse_bml_to_anim ERR: textXml is NULL!");
         return BehaviorRequestPtr();
      }

		DOMNode* animNode = textXml->getFirstChild()->getFirstChild();
		bool sameAttrs = true;
		std::string userValue;
		std::string configValue;
		while (animNode != NULL)
		{
			// same tag name
			if (XMLString::equals(animNode->getNodeName(), elem->getNodeName())) {
				// same attributes
				sameAttrs = true;
				DOMNamedNodeMap* attrs = animNode->getAttributes();
				XMLSize_t aSize = attrs->getLength();
				for( XMLSize_t i=0; i < aSize; i++ ) {
					DOMAttr* attr = (DOMAttr*) (attrs->item(i));
					userValue = string(XMLString::transcode(elem->getAttribute(attr->getName())));
					if (userValue.empty()) {
						sameAttrs = false;
						continue;
					}

					configValue = string(XMLString::transcode(attr->getValue()));
					std::transform(userValue.begin(), userValue.end(), userValue.begin(), tolower);
					std::transform(configValue.begin(), configValue.end(), configValue.begin(), tolower);

					if (strcmp(userValue.c_str(), configValue.c_str()) != 0)
						sameAttrs = false;
					
				}
				if (sameAttrs)
				{			
					if( XMLString::compareString( elem->getTagName(), TAG_POSTURE )==0 || XMLString::compareString( elem->getTagName(), TAG_BODY)==0 ) {
						string posture = "<body posture=\"" + string(XMLString::transcode(animNode->getTextContent())) + "\" />";
						DOMElement* e = xml_utils::parseMessageXml(Prser, (char*)posture.c_str())->getDocumentElement();
						return parse_bml_body(e, unique_id, behav_syncs, required, request, mcu);
						//behavior = parse_bml_body( child, unique_id, behav_syncs, required, request, mcu );
					}
					else {
						string animation = "<animation name=\"" + string(XMLString::transcode(animNode->getTextContent())) + "\" />";
						DOMElement* e = xml_utils::parseMessageXml(Prser, (char*)animation.c_str())->getDocumentElement();
						return parse_bml_animation(e, unique_id, behav_syncs, required, request, mcu);
					}
				}
			}
			animNode = animNode->getNextSibling();
		}
	}
	return BehaviorRequestPtr();
}
#endif  // BMLR_BML2ANIM


void BML::Processor::speechReply( SbmCharacter* actor, SmartBody::RequestId requestId, srArgBuffer& response_args, mcuCBHandle *mcu ) {
	string speechKey = buildSpeechKey( actor, requestId );
	MapOfSpeechRequest::iterator find_result = speeches.find( speechKey );

	if( find_result != speeches.end() ) {
		SpeechRequestPtr speech_request( find_result->second );
		if( speech_request ) {
			BmlRequestPtr request( speech_request->trigger->request.lock() );
			if( request ) {  // Is BmlRequest still alive?
				try {
					speech_request->speech_response( response_args );

					// Success!!  Let's Realize it!
#if USE_RECIPIENT
					BMLProcessorMsg msg( request->actor->name, request->recipientId.c_str(), request->msgId.c_str(), request->actor, NULL, NULL );
#else
					BMLProcessorMsg msg( request->actor->name, request->msgId.c_str(), request->actor, NULL, NULL );
#endif
					
					// check for negative times and change all times if needed
					/*SyncPoint * temp = request->first;
					while(temp != NULL){
						if(temp->time < 0){
							SyncPoint * temp2 = request->first->next;
							while(temp2 != NULL){
								temp2->time = temp2->time - temp->time;
								wcout << "changed TimeMaker : " << temp2->name << " into : " << temp2->time << endl;					
								temp2 = temp2->next;
							}
						}
						temp = temp->next;
					}*/

					request->realize( this, mcu );
				} catch( std::exception& e ) {
					std::stringstream strstr;
					strstr << "ERROR: BML::Processor::speechReply() exception:" << e.what();
					LOG(strstr.str().c_str());
					bml_error( actor->name, request->msgId.c_str(), e.what(), mcu );
				}
			} else {
				if( LOG_SPEECH )
				{
					std::stringstream strstr;
					strstr << "ERROR: BodyPlannerImpl::speechReply(..): SpeechRequest found for \"" << requestId << "\", but BmlRequest is missing";
					LOG(strstr.str().c_str());
				}
				// NO ERROR MESSAGE!  Missing BmlRequest means vrSpeakFailed fields are lost.
			}   // else ignore... not a part of this BodyPlanner or expired
		} else {
			if( LOG_SPEECH )
			{
				std::stringstream strstr;
				strstr << "ERROR: BodyPlannerImpl::speechReply(..): SpeechRequest not found for \"" << requestId << "\".";
				LOG(strstr.str().c_str());
			}
			// NO ERROR MESSAGE!  Missing SpeechRequest means BmlRequest's vrSpeakFailed fields are also lost.
		}   // else ignore... not a part of this BodyPlanner or expired

		// Clear the lookup table (shouldn't be referenced twice)
		speeches.erase( speechKey );
	} else if( LOG_SPEECH ) {
		std::stringstream strstr;
		strstr << "ERROR: BodyPlannerImpl::speechReply(..): No speech found for \"" << requestId << "\"";
		LOG(strstr.str().c_str());
	}   // else ignore... not a part of this BodyPlanner or expired
}


// Interrupt BML Performance (usually via message from InterruptBehavior)
int BML::Processor::interrupt( SbmCharacter* actor, const std::string& performance_id, time_sec duration, mcuCBHandle* mcu ) {
	string request_id = buildRequestId( actor, performance_id );
	MapOfBmlRequest::iterator result = bml_requests.find( request_id );
	if( result != bml_requests.end() ) {
		BmlRequestPtr request = result->second;

		if( BML_LOG_INTERRUPT )
			cout << "LOG: BML::Processor::interrupt(..): Found BehaviorRequest for \"" << performance_id << "\"." << endl;
		request->unschedule( this, mcu, duration );
		bml_requests.erase( result );
	} else {
		// Probably already cleaned up
		LOG("WARNING: BML::Processor::interrupt(..): No such BmlRequest for actor \"%s\" and performance_id %s.", actor->name, performance_id.c_str());
		// ignore without error
	}

	return CMD_SUCCESS;
}

// Cleanup Callback
int BML::Processor::bml_end( BMLProcessorMsg& bpMsg, mcuCBHandle *mcu ) {
	const char* requestId = bpMsg.requestId.c_str();
	MapOfBmlRequest::iterator find_result = bml_requests.find( requestId );
	if( find_result == bml_requests.end() ) {
		// Assume already cleaned up...
		//strstr << "WARNING: BodyPlannerImpl::bml_end(..): " << bpMsg.actorId << ": Unknown msgId=" << bpMsg.msgId << endl;
		return CMD_SUCCESS;
	}
	BmlRequestPtr request( find_result->second );

	// Parse second arguments...
	string end_code( bpMsg.args.read_token() );
	if( end_code == "complete" ) {
		string complete_code( bpMsg.args.read_token() );
		if( complete_code == "" ) {
			// Regular completion
		} else if( complete_code == "persistent" ) {
			// Persistent behaviors.  Some controllers may remain active.
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BodyPlannerImpl::bml_end(..): " << bpMsg.actorId << " " << bpMsg.msgId << ": Unknown end complete_code \""<<complete_code<<"\". Treating as normal complete.";
			LOG(strstr.str().c_str());
		}
	} else if( end_code == "interrupted" ) {
		// Ended by interruption from another behavior
	} else if( end_code == "ERROR" || end_code == "error" ) {
		// ended with error
	} else {
		std::stringstream strstr;
		strstr << "ERROR: BodyPlannerImpl::bml_end(..): " << bpMsg.actorId << " " << bpMsg.msgId << ": Unknown end_code \""<<end_code<<"\". Treating as complete." << endl;
		LOG(strstr.str().c_str());
	}

	request->cleanup( this, mcu );
	bml_requests.erase( requestId );

	return CMD_SUCCESS;
}


MapOfBmlRequest& BML::Processor::getBMLRequestMap()
{
	return bml_requests;
}


///////////////////////////////////////////////////////////////////////////////
//  Static Command and Message Hooks

int BML::Processor::vrAgentBML_cmd_func( srArgBuffer& args, mcuCBHandle *mcu )	{
	Processor& bp = mcu->bml_processor;

	const char   *character_id     = args.read_token();
	SbmCharacter *character        = mcu->character_map.lookup( character_id );
	if( character == NULL ) {
		//  Character is not managed by this SBM process
		if( bp.warn_unknown_agents )
		{
			std::stringstream strstr;
			strstr << "WARNING: BmlProcessor: Unknown agent \"" << character_id << "\".";
			LOG(strstr.str().c_str());
		}
		// Ignore
		return CMD_SUCCESS;
	}
#if USE_RECIPIENT
	// Unused.  To be removed, but will break compatibility with vrSpeak
	const char   *recipient_id = args.read_token();
#endif
	const char   *message_id   = args.read_token();
	const char   *command      = args.read_token();
	//cout << "DEBUG: vrAgentBML " << character_id << " " << recipientId << " " << messageId << endl;

	if( !character->is_initialized() ) {
		bml_error( character_id, message_id, "Uninitialized SbmCharacter.", mcu );
		return CMD_FAILURE;
	}


	if( _stricmp( command, "request" )==0 ) {
		//  NOTE: "vrAgentBML ... request" currently mimics vrSpeak,
		//  until we can figure out how to support multiple requests in an message

		char       *xml          = args.read_remainder_raw();

		if( xml[0]=='\0' ) {
			bml_error( character_id, message_id, "\"vrAgentBML .. request\" message incomplete (empty XML argument).", mcu );
			return CMD_FAILURE;
		}
		if( xml[0] == '"' ) {
			++xml;
			int len = strlen( xml );
			xml[--len] = '\0'; // shorten by one, assumed to be another double quote
		}

		try {
			DOMDocument *xmlDoc = xml_utils::parseMessageXml( bp.xmlParser.get(), xml );
			if( xmlDoc == NULL ) {
				bml_error( character_id, message_id, "XML parser returned NULL document.", mcu );
				return CMD_FAILURE;
			}

#if USE_RECIPIENT
			BMLProcessorMsg bpMsg( character_id, recipient_id, message_id, character, xmlDoc, args );
#else
			BMLProcessorMsg bpMsg( character_id, message_id, character, xmlDoc, args );
#endif
			bp.bml_request( bpMsg, mcu );

			return( CMD_SUCCESS );
		} catch( BML::BmlException& e ) {
			ostringstream msg;
			msg << e.type() << ": "<<e.what();
			bml_error( character_id, message_id, msg.str().c_str(), mcu );
			return CMD_FAILURE;
		} catch( const std::exception& e ) {
			ostringstream msg;
			msg << "std::exception: "<<e.what();
			bml_error( character_id, message_id, msg.str().c_str(), mcu );
			return CMD_FAILURE;
		//} catch( ... ) {
		//	ostringstream msg;
		//	msg << "Unknown exception."<<e.message;
		//	vrSpeakFailed( character_id, recipient_id, message_id, msg.str().c_str() );
		//	return CMD_FAILURE;
		}
	} else if( _stricmp( command, "start" )==0 ) {
		// TODO: Mark act as started
		return CMD_SUCCESS;
	} else if( _stricmp( command, "end" )==0 ) {
		try {
#if USE_RECIPIENT
			return bp.bml_end( BMLProcessorMsg( character_id, recipient_id, message_id, character, NULL, args ), mcu );
#else
			return bp.bml_end( BMLProcessorMsg( character_id, message_id, character, NULL, args ), mcu );
#endif
		} catch( BmlException& e ) {
			std::stringstream strstr;
			strstr << "vrAgentBML .. end: " << e.type() << ": " << e.what() << endl;
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		//} catch( AssertException& e ) {
		//	strstr << "vrSpeak: AssertionException: "<<e.getMessage()<< endl;
		//	return CMD_FAILURE;
		} catch( const exception& e ) {
			std::stringstream strstr;
			strstr << "vrAgentBML .. end: std::exception: "<<e.what()<< endl;
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		//} catch( ... ) {
		//	strstr << "vrSpeak: Unknown exception."<< endl;
		//	//std::unexpected();
		//	return CMD_FAILURE;
		}
	} else {
#if USE_RECIPIENT
		strstr << "ERROR: vrAgentBML: Unknown subcommand \"" << command << "\" in message:\n\t"
		     << "vrAgentBML " << character_id << " "<<recipient_id<<" " << message_id << " " << command << " " << args.read_remainder_raw() << endl;
#else
		std::stringstream strstr;
		strstr << "ERROR: vrAgentBML: Unknown subcommand \"" << command << "\" in message:\n\t"
		     << "vrAgentBML " << character_id << " " << message_id << " " << command << " " << args.read_remainder_raw();
		LOG(strstr.str().c_str());
#endif
		return CMD_FAILURE;
	}
}

int BML::Processor::vrSpeak_func( srArgBuffer& args, mcuCBHandle *mcu )	{
	Processor& bp = mcu->bml_processor;
	int suppress = 1;

	const char *agent_id     = args.read_token();
	const char *recipient_id = args.read_token();
	const char *message_id   = args.read_token();
	char       *xml          = args.read_remainder_raw();
	//cout << "DEBUG: vrSpeak " << agentId << " " << recipientId << " " << messageId << endl;

	try {
		if( xml[0]=='\0' ) {
			bml_error( agent_id, message_id, "vrSpeak message incomplete (empty XML argument).", mcu );
			return CMD_FAILURE;
		}
		if( xml[0] == '"' ) {
			++xml;
			int len = strlen( xml );
			xml[--len] = '\0'; // shorten by one, assumed to be another double quote
		}
		if( strstr( xml, "cache-overwrite" )==xml ) {  // if xml begins with "cache-overwrite"
			xml+=15; // skip it
			int whitespace = TRUE;
			while( whitespace ) {
				switch( xml[0] ) {
					case ' ':
					case '\t':
						++xml;
					default:
						whitespace = FALSE;
				}
			}
		}

		SbmCharacter *agent = mcu->character_map.lookup( agent_id );
		if( agent==NULL ) {
			//  Agent is not managed by this SBM process
			if( bp.warn_unknown_agents )
			{
				std::stringstream strstr;
				strstr << "WARNING: BmlProcessor: Unknown agent \"" << agent_id << "\".";
				LOG(strstr.str().c_str());
			}

				
			// Ignore
			return CMD_SUCCESS;
		}

		if( !agent->is_initialized() ) {
			bml_error( agent_id, message_id, "Uninitialized agent.", mcu );
			return CMD_FAILURE;
		}

        DOMDocument *xmlDoc = xml_utils::parseMessageXml( bp.xmlParser.get(), xml );

		if( xmlDoc == NULL ) {
			bml_error( agent_id, message_id, "XML parser returned NULL document.", mcu );
			return CMD_FAILURE;
		}

#if USE_RECIPIENT
		BMLProcessorMsg bpMsg( agent_id, recipient_id, message_id, agent, xmlDoc, args );
#else
		BMLProcessorMsg bpMsg( agent_id, message_id, agent, xmlDoc, args );
#endif
		bp.bml_request( bpMsg, mcu );

		return( CMD_SUCCESS );
	} catch( BmlException& e ) {
		ostringstream msg;
		msg << e.type() << ": "<<e.what();
		bml_error( agent_id, message_id, msg.str().c_str(), mcu );
		return CMD_FAILURE;
	} catch( const std::exception& e ) {
		ostringstream msg;
		msg << "std::exception: "<<e.what();
		bml_error( agent_id, message_id, msg.str().c_str(), mcu );
		return CMD_FAILURE;
	//} catch( ... ) {
	//	ostringstream msg;
	//	msg << "Unknown exception."<<e.message;
	//	vrSpeakFailed( agent_id, recipient_id, message_id, msg.str().c_str() );
	//	return CMD_FAILURE;
	}
}

int BML::Processor::vrSpoke_func( srArgBuffer& args, mcuCBHandle *mcu )	{
	Processor& bp = mcu->bml_processor;

	//cout << "DEBUG: vrSpoke " << args.read_remainder_raw() << endl;
	char *agent_id     = args.read_token();
	char *recipient_id = args.read_token();
	char *message_id   = args.read_token();
	// Ignore rest

	try {
		//cout << "DEBUG: vrSpoke " << agent_id << " " << recipientId << " " << message_id << endl;

		SbmCharacter *agent = mcu->character_map.lookup( agent_id );
		if( agent==NULL ) {
			// Ignore unknown agent.  Probably managed by other SBM process.
			return CMD_SUCCESS;
		}

#if VRAGENTBML_USES_RECIPIENT
		BMLProcessorMsg bpMsg( agent_id, recipient_id, message_id, agent, NULL, args );
#else
		BMLProcessorMsg bpMsg( agent_id, message_id, agent, NULL, args );
#endif
		return bp.bml_end( bpMsg, mcu );
	} catch( BmlException& e ) {
		ostringstream msg;
		msg << e.type() << ": "<<e.what();
		bml_error( agent_id, message_id, msg.str().c_str(), mcu );
		return CMD_FAILURE;
	} catch( const exception& e ) {
		std::stringstream strstr;
		strstr << "vrSpoke: std::exception: " << e.what();
		LOG(strstr.str().c_str());
		return CMD_FAILURE;
	//} catch( ... ) {
	//	strstr << "vrSpeak: Unknown exception."<< endl;
	//	//std::unexpected();
	//	return CMD_FAILURE;
	}
}

int BML::Processor::bp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	Processor& bp = mcu->bml_processor;

    string command = args.read_token();
    if( command == "reset" ) {
        bp.reset();
        return CMD_SUCCESS;
	} else if( command == "speech_ready" ) {
		// bp speech_ready <CharacterId> <RequestId> SUCCESS/ERROR reason
		char* actorId = args.read_token();
		SbmCharacter* actor = mcu->character_map.lookup( actorId );
		if( actor==NULL ) {
			std::stringstream strstr;
			strstr << "WARNING: BML::Processor::bp_cmd_func(): Unknown actor \"" << actorId << "\".  This is probably an error since the command \"bp speech_reply\" is not supposed to be sent over the network, thus it should not be coming from another SBM process." << endl;
			LOG(strstr.str().c_str());
			return CMD_SUCCESS;
		}

		char* requestIdStr = args.read_token(); // as string
		SmartBody::RequestId requestId = atoi( requestIdStr );

		bp.speechReply( actor, requestId, args, mcu );
		return CMD_SUCCESS;  // Errors are dealt with out of band
	} else if( command == "interrupt" ) {
		// bp speech_ready <actor id> <BML performace/act id>
		string actor_id = args.read_token();
		if( actor_id.empty() ) {
			cout << "ERROR: bp interrupt: missing actor id." << endl;
			return CMD_FAILURE;
		}

		SbmCharacter* actor = mcu->character_map.lookup( actor_id.c_str() );
		if( actor==NULL ) {
			// Unknown actor.  ignore and 
			cout << "WARNING: bp interrupt: Unknown actor \""<<actor_id<<"\"." << endl;
			return CMD_SUCCESS;  // ignored
		}

		string performance_id = args.read_token();
		if( actor_id.empty() ) {
			cout << "ERROR: bp interrupt: missing performance id." << endl;
			return CMD_FAILURE;
		}

		string duration_str = args.read_token();
		time_sec duration = 0;
		if( !duration_str.empty() ) {
			istringstream buffer( duration_str );
			if( !( buffer >> duration ) ) {
				cout << "WARNING: bp interrupt: failed to parse transition duration argument.  Assuming zero." << endl;
			}
		} else {
			cout << "WARNING: bp interrupt: missing transition duration argument.  Assuming zero." << endl;
		}

		if( duration < 0 ) {
			cout << "WARNING: bp interrupt: transition duration \""<<duration_str<<"\" less than zero.  Reseting to zero." << endl;
			duration = 0;
		}

		return bp.interrupt( actor, performance_id, duration, mcu );
    } else {
        return CMD_NOT_FOUND;
    }
}

int BML::Processor::set_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	Processor& bp = mcu->bml_processor;

	string attribute = args.read_token();
	if( attribute == "auto_print_controllers" ||
	    attribute == "auto-print-controllers" ) {
		string value = args.read_token();
		if( value == "on" ) {
			bp.auto_print_controllers = true;
			return CMD_SUCCESS;
		} else if( value == "off" ) {
			bp.auto_print_controllers = false;
			return CMD_SUCCESS;
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\".";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
	} else if( attribute == "auto_print_sequence" ||
	           attribute == "auto-print-sequence" ) {
		string value = args.read_token();
		if( value == "on" ) {
			bp.auto_print_sequence = true;
			return CMD_SUCCESS;
		} else if( value == "off" ) {
			bp.auto_print_sequence = false;
			return CMD_SUCCESS;
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\".";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
	} else if( attribute == "log_sync_points" ||
	           attribute == "log-sync-points" ) {
		string value = args.read_token();
		if( value == "on" ) {
			bp.log_syncpoints = true;
			return CMD_SUCCESS;
		} else if( value == "off" ) {
			bp.log_syncpoints = false;
			return CMD_SUCCESS;
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\".";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
	} else if( attribute == "controller_speed" ||
	           attribute == "controller-speed" ) {
		string sub_attribute = args.read_token();
		if( sub_attribute.empty() ) {
			std::stringstream strstr;
			strstr << "ERROR: Missing sub-attributes 'min <value>' or 'max <value>'.";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}

		float ct_speed_min = bp.ct_speed_min;
		float ct_speed_max = bp.ct_speed_max;

		while( !sub_attribute.empty() ) {
			if( sub_attribute == "min" ) {
				string value = args.read_token();
				if( value == "default" ) {
					ct_speed_min = BML::CONTROLLER_SPEED_MIN_DEFAULT;
				} else if( value.empty() || !(istringstream( value ) >> ct_speed_min) ) {
					std::stringstream strstr;
					strstr << "ERROR: Invalid " << attribute << ' ' << sub_attribute << " value string \"" << value << "\".";
					LOG(strstr.str().c_str());
					return CMD_FAILURE;
				}
			} else if( sub_attribute == "max" ) {
				string value = args.read_token();
				if( value == "default" ) {
					ct_speed_max = BML::CONTROLLER_SPEED_MAX_DEFAULT;
				} else if( value.empty() || !(istringstream( value ) >> ct_speed_max) ) {
					std::stringstream strstr;
					strstr << "ERROR: Invalid " << attribute << ' ' << sub_attribute << " value string \"" << value << "\".";
					LOG(strstr.str().c_str());
					return CMD_FAILURE;
				}
			} else {
				std::stringstream strstr;
				strstr << "ERROR: Unexpected sub_attribute \"" << sub_attribute << "\" for bp controller_speed.";
				LOG(strstr.str().c_str());
				return CMD_FAILURE;
			}
			sub_attribute = args.read_token();
		}

		bool valid = true;
		if( ct_speed_min >= 1 ) {
			std::stringstream strstr;
			strstr << "ERROR: controller_speed min must be less than 1.";
			LOG(strstr.str().c_str());
			valid = false;
		} else if( ct_speed_min <= 0 ) {
			std::stringstream strstr;
			strstr << "ERROR: controller_speed min must be greater than 0.";
			LOG(strstr.str().c_str());
			valid = false;
		}
		if(  ct_speed_max <= 1 ) {
			std::stringstream strstr;
			strstr << "ERROR: controller_speed max must be greater than 1.";
			LOG(strstr.str().c_str());
			valid = false;
		}
		if( valid ) {
			bp.ct_speed_min = ct_speed_min;
			bp.ct_speed_max = ct_speed_max;
			return CMD_SUCCESS;
		} else {
			return CMD_FAILURE;
		}
	} else if( attribute == "gaze" ) {
		attribute = args.read_token();
		if( attribute == "speed" ) {
			//  Currently takes three values for backward compatibility,
			//    add the first two as the total head speed.
			//  TODO: Support one value (head only) and two values (head and eye speed)
			float lumbar   = args.read_float();
			float cervical = args.read_float();
			float eyeball  = args.read_float();

			return BML::Gaze::set_gaze_speed( lumbar+cervical, eyeball );
		} else if( attribute == "smoothing" ) {
			float lumbar   = args.read_float();
			float cervical = args.read_float();
			float eyeball  = args.read_float();

			return BML::Gaze::set_gaze_smoothing( lumbar, cervical, eyeball );
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BML::Processor::set_func(): Unknown gaze attribute \"" << attribute <<"\".";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
	} else {
		std::stringstream strstr;
		strstr << "ERROR: BML::Processor::set_func(): Unknown attribute \"" << attribute <<"\".";
		LOG(strstr.str().c_str());
        return CMD_NOT_FOUND;
	}
}

int BML::Processor::print_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	Processor& bp = mcu->bml_processor;

	string attribute = args.read_token();
	if( attribute == "auto_print_controllers" ||
		attribute == "auto-print-controllers" ) {
		cout << "BML Processor auto_print_controllers: "<<
			(bp.auto_print_controllers? "on" : "off") << endl;
		return CMD_SUCCESS;
	} else if( attribute == "auto_print_sequence" ||
	           attribute == "auto-print-sequence" ) {
		cout << "BML Processor auto_print_sequence: "<<
			(bp.auto_print_sequence? "on" : "off") << endl;
		return CMD_SUCCESS;
	} else if( attribute == "log_syncpoints" ||
	           attribute == "log-syncpoints" ) {
		cout << "BML Processor log_syncpoints: "<<
			(bp.log_syncpoints? "on" : "off") << endl;
		return CMD_SUCCESS;
	} else if( attribute == "controller_speed" ||
	           attribute == "controller-speed" ) {
	   cout << "BML Processor "<<attribute<<": min="<<bp.ct_speed_min<<", max="<<bp.ct_speed_max<< endl;
	   return CMD_SUCCESS;
	} else if( attribute == "gaze" ) {
		attribute = args.read_token();
		if( attribute == "joint-speed" ||
		    attribute == "speed" ) {
			BML::Gaze::print_gaze_speed();
			return CMD_SUCCESS;
		} else if( attribute == "speed-smoothing" ||
		           attribute == "smoothing" ) {
			BML::Gaze::print_gaze_smoothing();
			return CMD_SUCCESS;
		} else {
			std::stringstream strstr;
			strstr << "ERROR: BML::Processor::set_func(): Unknown gaze attribute \"" << attribute <<"\".";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
	} else {
		std::stringstream strstr;
		strstr << "ERROR: BML::Processor::print_func(): Unknown attribute \"" << attribute <<"\".";
		LOG(strstr.str().c_str());
        return CMD_NOT_FOUND;
	}
}
