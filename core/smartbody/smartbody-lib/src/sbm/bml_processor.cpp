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

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <map>

#include "sr_arg_buff.h"

#include <xercesc/util/XMLStringTokenizer.hpp>

#include "mcontrol_util.h"
#include "bml_processor.hpp"
#include "bml_animation.hpp"
#include "bml_face.hpp"
#include "bml_gaze.hpp"
#include "bml_quickdraw.hpp"

#include "me_ct_examples.h"
#include "me_ct_gaze.h"

using namespace std;
using namespace BML;
using namespace SmartBody;


const bool LOG_METHODS				= false;
const bool LOG_BEHAVIORS			= false;
const bool LOG_SEQ_XML				= false;
const bool LOG_SPEECH				= false;
const bool LOG_TIME_MARKERS			= false;
const bool LOG_AUDIO				= false;
const bool LOG_SPEECH_REQUEST_ID	= false;
const bool LOG_REQUEST_MARKERS		= false;
// See also LOG_BML_VISEMES in bml_face.hpp


const double TIME_DELTA = 0.01;  // hundredths of a sec should be less than one animation frame
const double SQROOT_2 = 1.4142135623730950488016887242097;
const char* VISEME_NEUTRAL = "_";


///////////////////////////////////////////////////////////////////////////////
//  Implementation

// XMLStrings (utf-16 character arrays) for parsing vrSpeak's XML
const XMLCh TAG_ACT[]		= L"act";
const XMLCh TAG_BML[]       = L"bml";
const XMLCh TAG_BODY[]      = L"body";
const XMLCh TAG_SBM_EVENT[] = L"sbm:event";
const XMLCh TAG_HEAD[]      = L"head";
const XMLCh TAG_SPEECH[]    = L"speech";
const XMLCh TAG_TM[]        = L"tm";
const XMLCh TAG_MARK[]      = L"mark";


const XMLCh TAG_SBM_COMMAND[] = L"sbm:command";

// Deprecated behavior tags
const XMLCh TAG_ANIMATION[] = L"animation";
const XMLCh TAG_EVENT[]     = L"event";

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
const XMLCh ATTR_MESSAGE[]      = L"message";

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

	string buildSpeechKey( const SbmCharacter* character, SmartBody::RequestId requestId ) {
		ostringstream speechKey;
		speechKey << character->name << requestId;  // no space / single token
		return speechKey.str();
	}

	void vrSpeakFailed(
		const char* agent_id,
		const char* recipient,
		const char* message_id,
		const char* error_msg )
	{
		//  Let's not error on our error messages.  Be thorough.
		if( agent_id==NULL || agent_id[0]=='\0' )
			agent_id = "?";
		if( recipient==NULL || recipient[0]=='\0' )
			recipient = "?";
		if( message_id==NULL || message_id[0]=='\0' )
			message_id = "?";
		if( error_msg==NULL || error_msg[0]=='\0' )
			error_msg = "INVALID_ERROR_MESSAGE";

		ostringstream buff;
		cerr << "ERROR: vrSpeak: " << error_msg << "   (agent \"" << agent_id <<
			"\", recipient \"" << recipient << "\", message id \"" << message_id << "\")" << endl;
		buff << agent_id << " " << recipient << " " << message_id << " " << error_msg;
		mcuCBHandle::singleton().vhmsg_send( "vrSpeakFailed", buff.str().c_str() );
	}
};


///////////////////////////////////////////////////////////////////////////////
// BodyPlannerMsg
BML::Processor::BodyPlannerMsg::BodyPlannerMsg( const char *agentId, const char *recipientId, const char *msgId, const SbmCharacter *agent, DOMDocument *xml )
:	agentId(agentId),
	recipientId(recipientId),
	msgId(msgId),
	agent(agent),
	xml(xml),
	requestId( buildRequestId( agent, msgId ) )
{}

BML::Processor::BodyPlannerMsg::~BodyPlannerMsg() {
	// char* memory is owned by arg buffer
	// *xml memory may still be in use (?)
	// agent is just a pointer to mcu managed SbmCharacter
	// requestId has its own destructor
}

///////////////////////////////////////////////////////////////////////////////
//  Body Planner


BML::Processor::Processor()
:	auto_print_controllers( false ),
	auto_print_sequence( false ),
	log_synchpoints( false ),
	warn_unknown_agents( true )
{
	try {
		xmlParser = boost::shared_ptr<XercesDOMParser>( new XercesDOMParser() );

		xmlErrorHandler = new HandlerBase();
		xmlParser->setErrorHandler( xmlErrorHandler );
		//ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
		//xmlParser->setErrorHandler(errHandler);
		//xmlParser->setErrorHandler( new HandlerBase() );
	} catch( const XMLException& e ) {
		cerr << "ERROR: BML Processor:  XMLException during constructor: "<< e.getMessage() << endl;
	} catch( const std::exception& e ) {
		cerr << "ERROR: BML Processor:  std::exception during constructor: "<< e.what() << endl;
	} catch(...) {
		cerr << "ERROR: BML Processor:  UNKNOWN EXCEPTION DURING CONSTRUCTOR.     <<==================" << endl;
	}
}


BML::Processor::~Processor()
{
	delete xmlErrorHandler;
	xmlErrorHandler = NULL;
}


void BML::Processor::reset() {
	requests.clear();
}



BmlRequestPtr BML::Processor::createBmlRequest(
	const SbmCharacter* agent,
	const std::string & requestId,
	const std::string & recipientId,
	const std::string & msgId )
{
	BmlRequestPtr request( new BmlRequest( agent, requestId, recipientId, msgId ) );
	request->init( request );

	return request;
}










void BML::Processor::vrSpeak( BodyPlannerMsg& bpMsg, mcuCBHandle *mcu ) {
    if(LOG_METHODS) cout<<"BodyPlannerImpl::vrSpeak(..)"<<endl;

	//BmlRequest *request = requests.lookup( bpMsg.requestId.c_str() );  // srHashMap
	MapOfBmlRequest::iterator result = requests.find( bpMsg.requestId );
    if( result != requests.end() ) {
        cerr << "Duplicate BML Request Message ID: "<<bpMsg.msgId<<endl;
		//  TODO: call vrSpeakFailed
        return;
    }

    const DOMDocument* xml = bpMsg.xml;
    DOMElement* root = xml->getDocumentElement();
    if( XMLString::compareString( root->getTagName(), TAG_ACT )!=0 )
		cerr << "WARNING: BodyPlanner: Expected <act> tag as XML root." << endl;

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
		BmlRequestPtr request( createBmlRequest( bpMsg.agent, bpMsg.requestId, string(bpMsg.recipientId), string(bpMsg.msgId) ) );

		parseBML( bmlElem, request, mcu );
		requests.insert( make_pair( bpMsg.requestId, request ) );

		if( request->speech_request ) {
			// request speech through Speech API
			SmartBody::SpeechInterface* speech = bpMsg.agent->get_speech_impl();
			if( !speech ) {
				vrSpeakFailed( bpMsg.agent->name, bpMsg.requestId.c_str(), bpMsg.msgId, "No voice defined.  Cannot perform speech." );
				return;
			}

			// Found speech implementation.  Making request.
			SmartBody::RequestId reqId = speech->requestSpeechAudio( bpMsg.agentId, request->speech_request->getXML(), "bp speech_ready " );
			string speechKey = buildSpeechKey( bpMsg.agent, reqId );
			bool insert_success = speeches.insert( make_pair( speechKey, request->speech_request ) ).second;  // store for later reply
			if( !insert_success ) {
				cerr << "ERROR: BML::Processor.vrSpeak(..): srHashMap already contains an entry for speechKey \"" << speechKey << "\".  Cannot process speech behavior.  Failing BML request.  (This error should not occur. Let Andrew know immeidately.)"  << endl;
				// TODO: Send vrSpeakFailed
			}
		} else {
			// realize immediately
			realizeRequest( request, bpMsg, mcu );
		}
	} else {
		cerr << "ERROR: BodyPlanner: No BML element found." << endl;
		// TODO: Send vrSpeakFailed
	}
}
void BML::Processor::parseBML( DOMElement *bmlElem, BmlRequestPtr request, mcuCBHandle *mcu ) {
	// look for BML animation command tags
	DOMElement*      child = xml_utils::getFirstChildElement( bmlElem );
	const XMLCh*     speechId;
	//SpeechRequestPtr speech;

	// TEMPORARY: <speech> can only be the first behavior
	if( child && XMLString::compareString( child->getTagName(), TAG_SPEECH )==0 ) {
		speechId = child->getAttribute( ATTR_ID );

		//speech.reset( new SpeechRequest( child, speechId, request ) );
		request->speech_request.reset( new SpeechRequest( child, speechId, request ) );
		child = xml_utils::getNextElement( child );
	}

	while( child!=NULL ) {
		const XMLCh *tag = child->getTagName();  // Grand Child (behavior) Tag
		const XMLCh* id  = child->getAttribute( ATTR_ID );

		// Load SynchPoint references
		SynchPoints tms;
		tms.parseStandardSynchPoints( child, request );

		// Simplify references to synch points.
		SynchPointPtr start( tms.start );
		SynchPointPtr ready( tms.ready );
		SynchPointPtr stroke( tms.stroke );
		SynchPointPtr relax( tms.ready );
		SynchPointPtr end( tms.end );

		BehaviorRequest* behavior = NULL;

		// Parse behavior specifics
		if( XMLString::compareString( tag, TAG_ANIMATION )==0 ) {
			// DEPRECATED FORM
			behavior = parse_bml_animation( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
			////  TODO: Generalize behavior success/fail logging
			//if( LOG_BEHAVIORS )
			//	wcout<<"BodyPlannerImpl::parseBML(): <animation name=\""<<animName<<"\" .../> "<<(behavior?"Success.":"FAILED!")<<endl;
		} else if( XMLString::compareString( tag, TAG_SBM_ANIMATION )==0 ) {
			behavior = parse_bml_animation( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
			////  TODO: Generalize behavior success/fail logging
			//if( LOG_BEHAVIORS )
			//	wcout<<"BodyPlannerImpl::parseBML(): <animation name=\""<<animName<<"\" .../> "<<(behavior?"Success.":"FAILED!")<<endl;
		} else if( XMLString::compareString( tag, TAG_BODY )==0 ) {
			behavior = parse_bml_body( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
			////  TODO: Generalize behavior success/fail logging
			//if(LOG_BEHAVIORS) wcout<<"BodyPlannerImpl::parseBML(): <body posture=\""<<postureName<<"\" .../> Success."<<endl;
		} else if( XMLString::compareString( tag, TAG_HEAD )==0 ) {
			behavior = parse_bml_head( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_FACE )==0 ) {
			behavior = parse_bml_face( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_GAZE )==0 ) {
			behavior = /*BML::*/parse_bml_gaze( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_EVENT )==0 ) {
			// DEPRECATED FORM
			behavior = parse_bml_event( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_SBM_EVENT )==0 ) {
			behavior = parse_bml_event( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_QUICKDRAW )==0 ) {
			behavior = parse_bml_quickdraw( child, tms, request, mcu );
			if( behavior != NULL )
				request->addBehavior( behavior );
		} else if( XMLString::compareString( tag, TAG_SPEECH )==0 ) {
			cerr<<"ERROR: BodyPlannerImpl: <speech> BML tag must be first behavior (TEMPORARY HACK)." <<endl;
		} else {
			wcerr<<"WARNING: BodyPlannerImpl: <"<<tag<<"> BML tag unrecognized or unsupported."<<endl;
		}

		if( behavior != NULL ) {
			// Gesture success... Register sync points
			if( id ) {
				request->synch_points.insert( make_pair( buildBmlId( id, TM_START ),  start ) );
				request->synch_points.insert( make_pair( buildBmlId( id, TM_READY ),  ready ) );
				request->synch_points.insert( make_pair( buildBmlId( id, TM_STROKE ), ready ) );
				request->synch_points.insert( make_pair( buildBmlId( id, TM_RELAX ),  relax ) );
				request->synch_points.insert( make_pair( buildBmlId( id, TM_END ),    end ) );

				if( LOG_REQUEST_MARKERS ) {
					MapOfSynchPoint& synch_points = request->synch_points;
					MapOfSynchPoint::iterator i = synch_points.begin();
					MapOfSynchPoint::iterator end = synch_points.end();

					wcout << "request->synch_points after <"<<tag<<" id=\""<<id<<"\" .. />" << endl;
					for( ; i!=end; ++i ) {
						const XMLCh* tm_id = i->second->name;
						if( tm_id ) {
							wcout << "\t" << i->second->name << endl;
						} else {
							wcout << "\tUNNAMED!" << endl;
						}
					}
				}
			}
		}

		child = xml_utils::getNextElement( child );
	}

	if( LOG_TIME_MARKERS ) {
		cout << "LOG_TIME_MARKERS is broken.  Please fix!!";
		//cout << "TIDs:";
		//SynchPoint* sp = trigger->start;
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

BehaviorRequest* BML::Processor::parse_bml_body( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
	const XMLCh* postureName = elem->getAttribute( ATTR_POSTURE );
	if( postureName && XMLString::stringLen( postureName ) ) {
		// Look up pose
		const char* ascii_pose_id = xml_utils::asciiString(postureName);
		string pose_id = ascii_pose_id;
		delete [] ascii_pose_id;
		SkPosture* posture = mcu->pose_map.lookup( pose_id );
		if( posture ) {
			MeCtPose* poseCt = new MeCtPose();
			poseCt->init( *posture );
			poseCt->name( posture->name() );  // TODO: include BML act and behavior ids

			return new PostureRequest( poseCt, 1, tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
		} else {
			// Check for a motion (a motion texture, or motex) of the same name
			SkMotion* motion = mcu->motion_map.lookup( pose_id );
			if( motion ) {
				MeCtMotion* motionCt = new MeCtMotion();
				motionCt->init( motion );
				motionCt->name( motion->name() );  // TODO: include BML act and behavior ids
				motionCt->loop( true );

				return new PostureRequest( motionCt, 1, tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
			} else {
				wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <body>: posture=\""<<postureName<<"\" not loaded; ignoring <body>."<<endl;
				return NULL;
			}
		}
	} else {
		wcerr<<"WARNING: BodyPlannerImpl::parseBML(): <body> missing posture = attribute; ignoring <body>."<<endl;
		return NULL;
	}
}

BehaviorRequest* BML::Processor::parse_bml_event( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
    const XMLCh* attrMesg = elem->getAttribute( ATTR_MESSAGE );

	if( attrMesg && attrMesg[0]!='\0' ) {
        return new EventRequest( XMLString::transcode( attrMesg ), tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
	} else {
		// TODO: Use exception?
        wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_MESSAGE<<"= attribute.  Behavior ignored."<< endl;
		return NULL;
	}
}

BehaviorRequest* BML::Processor::parse_bml_head( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
	const XMLCh* attrType = elem->getAttribute( ATTR_TYPE );
	if( attrType && XMLString::stringLen( attrType ) ) {
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
                if( attrRepeats && XMLString::stringLen( attrRepeats ) )
                    repeats = xml_utils::xcstof( attrRepeats );

                const XMLCh* attrAmount = elem->getAttribute( ATTR_AMOUNT );
                float amount = 0.5;  // default to a moderate amount.  Range 0.0 to 1.0
                if( attrAmount && XMLString::stringLen( attrAmount ) )
                    amount = xml_utils::xcstof( attrAmount );

                const XMLCh* attrVelocity = elem->getAttribute( ATTR_VELOCITY );
                float velocity = 1;  // default to one cycle per second
                if( attrVelocity && XMLString::stringLen( attrVelocity ) )
                    velocity = xml_utils::xcstof( attrVelocity );

                float duration = velocity * repeats;

                NodRequest* nod = new NodRequest( (NodRequest::NodType) type, repeats, velocity, amount, 
                                                    request->agent,
                                                    tms.start, tms.ready, tms.stroke, tms.relax, tms.end );
                return nod;
            }

			case BML::HEAD_ORIENT: {
				const XMLCh* direction = elem->getAttribute( ATTR_DIRECTION );
				const XMLCh* target    = elem->getAttribute( ATTR_TARGET );
				const XMLCh* angle     = elem->getAttribute( ATTR_ANGLE );

				if( target && XMLString::stringLen( target ) ) {
					// TODO
					wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a target.  Ignoring behavior." << endl;
					return NULL;
				} else if( direction && XMLString::stringLen( direction ) ) {
					if( XMLString::compareIString( direction, DIR_RIGHT )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_RIGHT<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else if( XMLString::compareIString( direction, DIR_LEFT )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_LEFT<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else if( XMLString::compareIString( direction, DIR_UP )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_UP<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else if( XMLString::compareIString( direction, DIR_DOWN )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_DOWN<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else if( XMLString::compareIString( direction, DIR_ROLLRIGHT )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_ROLLRIGHT<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else if( XMLString::compareIString( direction, DIR_ROLLRIGHT )==0 ) {
						// TODO
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction=\""<<DIR_ROLLRIGHT<<"\".  Ignoring behavior." << endl;
						return NULL;
					} else {
						wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unrecognized direction \""<<direction<<"\" in <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">.  Ignoring behavior." << endl;
						return NULL;
					}

					// TODO
					wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> using a direction.  Ignoring behavior." << endl;
					return NULL;
				} else {
					wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\"> requires a target or a direction attribute.  Ignoring behavior." << endl;
					return NULL;
				}
			}

			case BML::HEAD_TOSS:
				wcerr << "WARNING: BodyPlannerImpl::parseBML(): Unimplemented: <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">.  Ignoring behavior." << endl;
				return NULL;

			default:
                wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<" "<<ATTR_TYPE<<"=\""<<attrType<<"\">: Unknown type value, ignore command" << endl;
				return NULL;
        }
    } else {
        wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_TYPE<<"= attribute." << endl;
		return NULL;
    }
}

void BML::Processor::speechReply( SbmCharacter* character, SmartBody::RequestId requestId, const char* errorMsg, mcuCBHandle *mcu ) {
	string speechKey = buildSpeechKey( character, requestId );
	MapOfSpeechRequest::iterator find_result = speeches.find( speechKey );

	if( find_result != speeches.end() ) {
		SpeechRequestPtr speechReq( find_result->second );

		// Clear the lookup table (shouldn't be referenced twice)
		speeches.erase( speechKey );

		TriggerEventPtr trigger( speechReq->trigger );
		BmlRequestPtr   request( trigger->request.lock() );

		if( request ) {  // Is BmlRequest still alive?
			// request speech through Speech API
			SmartBody::SpeechInterface* speech = request->agent->get_speech_impl();
			if( !speech ) {
				cerr << "ERROR: Character \"" << request->agent->name << "\" does not have a voice defined.  Cannot perform speech." << endl;
				// TODO: Send vrSpeakFailed
				return;
			}

			// Found speech implementation.  Making request.
			if( !errorMsg ) {
				if( LOG_SPEECH_REQUEST_ID )
					clog << "LOG: BodyPlannerImpl::speechReply(..): speech found for RequestId " << requestId << endl;
				if ( character->bonebusCharacter )
					request->audioPlay = speech->getSpeechPlayCommand( requestId, character->bonebusCharacter->m_charId );
				else
					request->audioPlay = speech->getSpeechPlayCommand( requestId, 0 );
				if( LOG_AUDIO )
					cout << "DEBUG: BodyPlannerImpl::speechReply: request->audioPlay = " << request->audioPlay << endl;

				SynchPointPtr ready( speechReq->ready );
				SynchPointPtr relax( speechReq->relax );
				SynchPointPtr end( speechReq->end );

				// save timing;
				float firstOpen = -1; // unset
				float lastOpen  = 0;
				float lastAny   = 0;

				// Process Visemes
				const vector<VisemeData*>* visemes = speech->getVisemes( requestId );
				if( visemes ) {
					request->visemes = *visemes;  // Copy contents
					vector<VisemeData*>::const_iterator cur = visemes->begin();
					vector<VisemeData*>::const_iterator end = visemes->end();

					if( LOG_SPEECH && cur==end )
						cerr << "ERROR: BodyPlannerImpl::speechReply(): speech.getVisemes( " << requestId << " ) is empty." << endl;

					for( ; cur!=end; ++cur ) {
						VisemeData* v = (*cur);
						if( LOG_SPEECH ) {
							//cout << "   " << (*v) << endl;  // Not linking
							cout << "   VisemeData: " << v->id() << " (" << v->weight() << ") @ " << v->time() << endl;
						}
						if( strcmp( v->id(), VISEME_NEUTRAL )!=0 ) {
							if( firstOpen==-1 )
								firstOpen = v->time();
							lastOpen = v->time();
						}
						lastAny = v->time();
					}
				} else {
					if( LOG_SPEECH )
						cerr << "ERROR: BodyPlannerImpl::speechReply(): speech.getVisemes( " << requestId << " ) returned NULL." << endl;
				}
				//  Set core synch_point times
				if( firstOpen==-1 ) {
					ready->time = 0;
					relax->time = lastAny;
				} else {
					ready->time = firstOpen;
					relax->time = lastOpen;
				}
				end->time = lastAny;


				// Process Speech Time Markers
				if( !speechReq->tms.empty() ) {
					VecOfSynchPoint::iterator it = speechReq->tms.begin();
					VecOfSynchPoint::iterator end = speechReq->tms.end();

					for(; it != end; ++it ) {
						SynchPointPtr cur( *it );
						if( cur->parent == NULL ) {
							float audioTime = speech->getMarkTime( requestId, cur->name );
							if( audioTime >= 0 ) {
								if( LOG_TIME_MARKERS ) wcout << "   SynchPoint \"" << cur->name << "\" @ " << audioTime << endl;
								cur->time = audioTime;
							} else {
								wcerr << "ERROR: BodyPlannerImpl::speechReply(): No audioTime for SynchPoint \"" << cur->name << "\"" << endl;
							}
						} else {  // cur is an offset sync point relative to some parent
							if( cur->parent->time != TIME_UNSET ) {
								cur->time = cur->parent->time + cur->offset;
								//wcout << "   SynchPoint \"" << cur->name << "\" @ " << cur->time << endl;
							} else {
								wcerr << "ERROR: BodyPlannerImpl::speechReply(): No time for parent SynchPoint \"" << cur->parent->name << "\" of relative SynchPoint \"" << cur->name << "\" (offset "<<cur->offset<<")" << endl;
							}
						}
					}
				} else {
					if( LOG_TIME_MARKERS )
						cout << "   BodyPlannerImpl::speechReply(..): No speech bookmarks" << endl;
				}
				
				BodyPlannerMsg msg( request->agent->name, request->recipientId.c_str(), request->msgId.c_str(), request->agent, NULL );
				
				// check for negative times and change all times if needed
				/*SynchPoint * temp = request->first;
				while(temp != NULL){
					if(temp->time < 0){
						SynchPoint * temp2 = request->first->next;
						while(temp2 != NULL){
							temp2->time = temp2->time - temp->time;
							wcout << "changed TimeMaker : " << temp2->name << " into : " << temp2->time << endl;					
							temp2 = temp2->next;
						}
					}
					temp = temp->next;
				}*/

				realizeRequest( request, msg, mcu );
			} else {
				cerr << "ERROR: BodyPlannerImpl::speechReply(..): Error during speech RequestId " << requestId << ": " << errorMsg << endl;
				// NO ERROR MESSAGE!  Missing BmlRequest means vrSpeakFailed fields are lost.
			}
			speech->requestComplete( requestId );
		} else if( LOG_SPEECH ) {
			cerr << "ERROR: BodyPlannerImpl::speechReply(..): SpeechRequest found for \"" << requestId << "\", but BmlRequest is missing" << endl;
			vrSpeakFailed( request->agent->name, request->recipientId.c_str(), request->msgId.c_str(), errorMsg );
		}   // else ignore... not a part of this BodyPlanner or expired
	} else if( LOG_SPEECH ) {
		cerr << "ERROR: BodyPlannerImpl::speechReply(..): No speech found for \"" << requestId << "\"" << endl;
	}   // else ignore... not a part of this BodyPlanner or expired
}


void BML::Processor::realizeRequest( BmlRequestPtr request, BodyPlannerMsg& bpMsg, mcuCBHandle *mcu ) {
	// Find earliest event
	time_sec audioOffset = 0;
	VecOfBehaviorRequest::iterator gest_end = request->behaviors.end();
	for( VecOfBehaviorRequest::iterator i = request->behaviors.begin(); i != gest_end;  ++i ) {
		BehaviorRequest* behavior = *i;
		audioOffset = min( audioOffset, behavior->getAudioRelativeStart() );
	}

	audioOffset *= -1;
	if( auto_print_sequence )
		cout << "BodyPlannerImpl::realizeRequest(..):  audioOffset = "<<audioOffset<<endl;

////// OLD CODE: All behaviors lumped into a single schedule
//
//	//  Schedule all behaviors relative to earliest
//	MeCtScheduler2* scheduler = new MeCtScheduler2();
//	scheduler->init();
//
//	string request_sched_name( bpMsg.agentId );
//	request_sched_name += "'s act ";
//	request_sched_name += bpMsg.msgId;
//	scheduler->name( request_sched_name.c_str() );

	VecOfSbmCommand commands;
	bool has_controllers = false;  // TODO: schedule prune by last synch point  
	for( VecOfBehaviorRequest::iterator i = request->behaviors.begin(); i != gest_end;  ++i ) {
		BehaviorRequest* behavior = *i;
		time_sec startAt = audioOffset + ( behavior->getAudioRelativeStart() );
		behavior->schedule( mcu, request->agent, request->visemes, commands, startAt );

		has_controllers |= behavior->has_cts();
	}

////// OLD CODE: All behaviors lumped into a single schedule
//
//	//  From scheduled behavior tracks, find the proper blend indt/oudt
//    time_sec indt = 0;
//    time_sec outdt = 0;
//    time_sec tin = 0;
    time_sec tout = 0;     // TODO: Calculate the last synch point
//    const int numTracks = scheduler->count_children();
//	if( scheduler->begin() != scheduler->end() ) {
//		// TODO: Find proper sub-schedule blend-in-out points
//
//		MeCtScheduler2::track_iterator track_i = request->agent->scheduler_p->schedule( scheduler, mcu->time, (float)indt, (float)outdt );
//	}


	////////////////////////////////////////////////////////////////////
	//    Old code previously used to get track schedule times
	//    Keeping for reference when implementing above TODO
	//
	//const int numTracks	= scheduler->tracks();
	//if(	numTracks )	{
	//	MeCtScheduler::Track* curTrack = &scheduler->track(0);
	//	tin	= curTrack->tin;
	//	tout = curTrack->tout;
	//
	//	time_sec tind =	tin	+ curTrack->indt;
	//	time_sec toutd = tout -	curTrack->outdt;
	//
	//	vector<MeCtScheduler::Track*> firstTracks;
	//	firstTracks.push_back( curTrack	);
	//	vector<MeCtScheduler::Track*> lastTracks;
	//	lastTracks.push_back( curTrack );
	//
	//	for( int i=1; i<numTracks; i++ ) {
	//		curTrack = &scheduler->track(i);
	//
	//		if(	curTrack->tin <	tin-TIME_DELTA ) {
	//			firstTracks.clear();  // should	really check for overlap..
	//			firstTracks.push_back( curTrack	);
	//
	//			tin	= curTrack->tin;
	//			tind = tin + curTrack->indt;
	//		} else if( curTrack->tin < tind-TIME_DELTA ) {
	//			time_sec new_tind =	curTrack->tin+curTrack->indt;
	//			if(	new_tind < tind-TIME_DELTA ) //	Shorter	blend?
	//				firstTracks.clear();
	//			firstTracks.push_back( curTrack	);
	//		}
	//
	//		if(	curTrack->tout > tout+TIME_DELTA ) {
	//			lastTracks.clear();	 //	should really check	for	overlap..
	//			lastTracks.push_back( curTrack );
	//
	//			tout = curTrack->tout;
	//			toutd =	tout - curTrack->outdt;
	//		} else if( curTrack->tout >	toutd+TIME_DELTA ) {
	//			time_sec new_toutd = curTrack->tout-curTrack->outdt;
	//			if(	new_toutd <	toutd+TIME_DELTA ) // Shorter blend?
	//				lastTracks.clear();
	//			lastTracks.push_back( curTrack );
	//		}
	//	}
	//
	//	// unblend within request	track to give full blend at	actor scheduler	level
	//	vector<MeCtScheduler::Track*>::iterator	it = firstTracks.begin();
	//	for( curTrack =	*it; it	!= firstTracks.end(); ++it )
	//		curTrack->indt = 0;
	//	it = lastTracks.begin();
	//	for( curTrack =	*it; it	!= lastTracks.end(); ++it )
	//		curTrack->outdt	= 0;
	//
	//	// Request indt and outdt
	//	indt  =	tind - tin;
	//	outdt =	tout - toutd;
	//
	//	//	Schedule the BML request
	//	request->agent->scheduler_p->schedule( scheduler, mcu->time, (float)indt, (float)outdt,	MeCtScheduler::Once	);
	//}
	//
	/////////  End old code

	if( has_controllers ) {
		// Schedule a prune command to clear them out later.
		string command( "char " );
		command += request->agent->name;
		command += " prune";
		commands.push_back( new SbmCommand( command, 15 ) ); // 15 seconds later is a guess because tout is not properly set
	}

    // Build associated command sequence
	srCmdSeq *seq = new srCmdSeq(); //sequence file that holds the audio and visemes
	seq->offset( (float)( mcu->time ) );

	// Schedule SbmCommands
	VecOfSbmCommand::iterator commands_end = commands.end();
	for( VecOfSbmCommand::iterator i = commands.begin(); i != commands_end; ++i ) {
		SbmCommand* command = *i;
		if( seq->insert( command->time, command->command.c_str() ) != CMD_SUCCESS ) {
			cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
				"Failed to insert SbmCommand \""<<command->command<<"\" at time "<<command->time<<endl;
		}
	}
	
	// Schedule visemes
	//   visemes are stored in request->visemes as VisemeData objects (defined in bml.hpp)
	// add audioOffset to each viseme time,
    if( request->visemes.size() > 0 ) {
		//// Replaced by addition in next loop
	    //for( int i=0; i<(int)request->visemes.size(); i++ ) {
		//	request->visemes.at(i)->time+= audioOffset;
	    //}

        ostringstream command;
	    for (int i=0; i<(int)request->visemes.size(); i++) { //adds visemes for audio into sequence file
			VisemeData* v = request->visemes.at(i);
			float time = float( v->time() + audioOffset );

            command.str( "" );
            command << "char " << bpMsg.agentId << " viseme " << v->id() << ' ' << v->weight() << ' ' << v->duration();
			
            if( LOG_BML_VISEMES ) cout << "command (complete): " << command.str() << endl;
            //visemes get set a specified time
            if( seq->insert( time, (char*)(command.str().c_str()) )!=CMD_SUCCESS ) {
                cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
                    "Failed to insert viseme \""<<v->id()<<"\" @ "<<time<<endl;
            }
			if( LOG_BML_VISEMES ) {
		        ostringstream echo;
				echo.str( "echo LOG_BML_VISEMES: t" );
				echo << time << ":\t" << command.str();
				if( seq->insert( time, (char*)(echo.str().c_str()) )!=CMD_SUCCESS ) {
					cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
						"Failed to insert viseme echo \""<<v->id()<<"\" @ "<<time<<endl;
				}
			}
	    }
	} else if( request->speech_request ) {
		cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): request->spespeech_requestech but no viseme data."<<endl;
	}
	
    // Schedule audio
	if( request->audioPlay ) {
		if( LOG_AUDIO || LOG_BML_VISEMES )
			cout << "DEBUG: BodyPlannerImpl::realizeRequest(..): scheduling request->audioPlay: " << request->audioPlay << endl;
        // schedule for later
        if( seq->insert( (float)(audioOffset<0? 0: audioOffset), request->audioPlay ) != CMD_SUCCESS ) {
			printf( "ERROR: BodyPlannerImpl::realizeRequest: insert audio trigger into seq FAILED, msgId=%s\n", bpMsg.msgId ); 
	    }
	}

	//  Schedule vrAgentBML end
	{
        ostringstream command;
		//// vrSpoke is a misnomer, since not all BML acts include speech
        //command << "send vrSpoke " << bpMsg.agentId << " " << bpMsg.recipientId << " " << bpMsg.msgId << " FakeText";
        command << "send vrAgentBML " << bpMsg.agentId << " " << bpMsg.recipientId << " " << bpMsg.msgId << " end complete";

		if( seq->insert( (float)tout, (char*)(command.str().c_str()) )!=CMD_SUCCESS ) {
			cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
				"Failed to insert \""<<command<<"\" command."<<endl;
		}
	}


	//  Add Logging / Debugging commands to sequence
	if( log_synchpoints ) {
		ostringstream oss;
		oss << "echo ===\tAgent: " << bpMsg.agentId << "\tMsgId: " << bpMsg.msgId << "\tSynchPoint: ";
		string command_prefix = oss.str();

		BML::MapOfSynchPoint::iterator i = request->synch_points.begin();
		BML::MapOfSynchPoint::iterator synch_points_end = request->synch_points.end();

		for(; i!=synch_points_end; ++i ) {
			SynchPointPtr sp( i->second );
			string name = XMLString::transcode( sp->name );
			BML::time_sec time = sp->time;
			if( time != BML::TIME_UNSET ) {
				BML::time_sec seqTime = time;
				if( seqTime < 0 )
					seqTime = 0;

				oss.str("");  // clear the buffer
				oss << command_prefix << name << "\t@ " << time;
				if( seq->insert( (float)seqTime, (char*)(oss.str().c_str()) )!=CMD_SUCCESS ) {
					cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
						"Failed to insert echo timesynch_point message"<<endl;
				}
			}
		}
	}

	if( auto_print_controllers ) {
		ostringstream oss;
		oss << "print character "<< bpMsg.agentId << " schedule";
		string& cmd = oss.str();
		if( seq->insert( 0, (char*)(cmd.c_str()) )!=CMD_SUCCESS ) {
			cerr << "WARNING: BodyPlannerImpl::realizeRequest(..): msgId=\""<<bpMsg.msgId<<"\": "<<
				"Failed to insert \"" << cmd << "\" command"<<endl;
		}
	}

	if( auto_print_sequence ) {
		cout << "DEBUG: BML::Processor::realizeRequest(..): Sequence for message Id \"" << bpMsg.msgId<<"\":"<<endl;
		seq->print();
	}

	//  Trigger command sequence
	char* seqName = new char[strlen(bpMsg.msgId)+11];
	sprintf( seqName, "%s:seq-items", bpMsg.msgId );
	mcu->active_seq_map.remove( seqName );  // remove old sequence by this name
	if( mcu->active_seq_map.insert( seqName, seq ) != CMD_SUCCESS ) {
		printf( "WARNING: BodyPlannerImpl::realizeRequest: insert audio/viseme trigger seq into active_seq_map FAILED, msgId=%s\n", bpMsg.msgId ); 
	}
	delete [] seqName;


	{
		VecOfSbmCommand::iterator delete_commands_end = commands.end();
		for( VecOfSbmCommand::iterator i_delete = commands.begin(); i_delete != delete_commands_end; ++i_delete )
		{
			delete *i_delete;
		}
		commands.clear();
	}
}

// Cleanup Callback
int BML::Processor::vrSpoke( BodyPlannerMsg& bpMsg, mcuCBHandle *mcu ) {
	const char* requestId = bpMsg.requestId.c_str();
	MapOfBmlRequest::iterator find_result = requests.find( requestId );
	if( find_result == requests.end() ) {
		cerr << "ERROR: BodyPlannerImpl::vrSpoke(..): " << bpMsg.agentId << ": Unknown msgId=" << bpMsg.msgId << endl;
		return CMD_FAILURE;
	}

	// else ...
	// TODO: Clean-up circular references
	requests.erase( requestId );

	return CMD_SUCCESS;
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
			cerr << "WARNING: BmlProcessor: Unknown agent \"" << character_id << "\"." << endl;
		// Ignore
		return CMD_SUCCESS;
	}
	const char   *recipient_id = args.read_token();
	const char   *message_id   = args.read_token();
	const char   *command      = args.read_token();
	//cout << "DEBUG: vrAgentBML " << character_id << " " << recipientId << " " << messageId << endl;

	if( !character->is_initialized() ) {
		vrSpeakFailed( character_id, recipient_id, message_id, "Uninitialized SbmCharacter." );
		return CMD_FAILURE;
	}


	if( _stricmp( command, "request" )==0 ) {
		//  NOTE: "vrAgentBML ... request" currently mimics vrSpeak,
		//  until we can figure out how to support multiple requests in an message

		char       *xml          = args.read_remainder_raw();

		if( xml[0]=='\0' ) {
			vrSpeakFailed( character_id, recipient_id, message_id, "vrSpeak message incomplete (empty XML argument)." );
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
				vrSpeakFailed( character_id, recipient_id, message_id, "XML parser returned NULL document." );
				return CMD_FAILURE;
			}

			BodyPlannerMsg bpMsg( character_id, recipient_id, message_id, character, xmlDoc );
			bp.vrSpeak( bpMsg, mcu );

			return( CMD_SUCCESS );
		} catch( BodyPlannerException& e ) {
			ostringstream msg;
			msg << "BodyPlannerException: "<<e.message;
			vrSpeakFailed( character_id, recipient_id, message_id, msg.str().c_str() );
			return CMD_FAILURE;
		} catch( const std::exception& e ) {
			ostringstream msg;
			msg << "std::exception: "<<e.what();
			vrSpeakFailed( character_id, recipient_id, message_id, msg.str().c_str() );
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
			return bp.vrSpoke( BodyPlannerMsg( character_id, recipient_id, message_id, character, NULL ), mcu );
		} catch( BodyPlannerException& e ) {
			cerr << "vrSpeak: BodyPlannerException: "<<e.message<<endl;
			return CMD_FAILURE;
		//} catch( AssertException& e ) {
		//	cerr << "vrSpeak: AssertionException: "<<e.getMessage()<< endl;
		//	return CMD_FAILURE;
		} catch( const exception& e ) {
			cerr << "vrSpeak: std::exception: "<<e.what()<< endl;
			return CMD_FAILURE;
		//} catch( ... ) {
		//	cerr << "vrSpeak: Unknown exception."<< endl;
		//	//std::unexpected();
		//	return CMD_FAILURE;
		}
	} else {
		cerr << "ERROR: vrAgentBML: Unknown subcommand \"" << command << "\" in message:\n\t"
		     << "vrAgentBML " << character_id << " " << recipient_id << " " << message_id << " " << command << " " << args.read_remainder_raw() << endl;
		return CMD_FAILURE;
	}
}

int BML::Processor::vrSpeak_func( srArgBuffer& args, mcuCBHandle *mcu )	{
	Processor& bp = mcu->bml_processor;

	const char *agent_id     = args.read_token();
	const char *recipient_id = args.read_token();
	const char *message_id   = args.read_token();
	char       *xml          = args.read_remainder_raw();
	//cout << "DEBUG: vrSpeak " << agentId << " " << recipientId << " " << messageId << endl;

	try {
		if( xml[0]=='\0' ) {
			vrSpeakFailed( agent_id, recipient_id, message_id, "vrSpeak message incomplete (empty XML argument)." );
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
				cerr << "WARNING: BmlProcessor: Unknown agent \"" << agent_id << "\"." << endl;
			// Ignore
			return CMD_SUCCESS;
		}

		if( !agent->is_initialized() ) {
			vrSpeakFailed( agent_id, recipient_id, message_id, "Uninitialized agent." );
			return CMD_FAILURE;
		}

        DOMDocument *xmlDoc = xml_utils::parseMessageXml( bp.xmlParser.get(), xml );
		if( xmlDoc == NULL ) {
			vrSpeakFailed( agent_id, recipient_id, message_id, "XML parser returned NULL document." );
			return CMD_FAILURE;
		}

		BodyPlannerMsg bpMsg( agent_id, recipient_id, message_id, agent, xmlDoc );
		bp.vrSpeak( bpMsg, mcu );

		return( CMD_SUCCESS );
	} catch( BodyPlannerException& e ) {
		ostringstream msg;
		msg << "BodyPlannerException: "<<e.message;
		vrSpeakFailed( agent_id, recipient_id, message_id, msg.str().c_str() );
		return CMD_FAILURE;
	} catch( const std::exception& e ) {
		ostringstream msg;
		msg << "std::exception: "<<e.what();
		vrSpeakFailed( agent_id, recipient_id, message_id, msg.str().c_str() );
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
	try {
		char *agentId     = args.read_token();
		char *recipientId = args.read_token();
		char *messageId   = args.read_token();
		// Ignore rest

		//cout << "DEBUG: vrSpoke " << agentId << " " << recipientId << " " << messageId << endl;

		SbmCharacter *agent = mcu->character_map.lookup( agentId );
		if( agent==NULL ) {
			// Ignore unknown agent.  Probably managed by other SBM process.
			return CMD_SUCCESS;
		}

		BodyPlannerMsg bpMsg( agentId, recipientId, messageId, agent, NULL );
		return bp.vrSpoke( bpMsg, mcu );
	} catch( BodyPlannerException& e ) {
		cerr << "vrSpeak: BodyPlannerException: "<<e.message<<endl;
		return CMD_FAILURE;
	//} catch( AssertException& e ) {
	//	cerr << "vrSpeak: AssertionException: "<<e.getMessage()<< endl;
	//	return CMD_FAILURE;
	} catch( const exception& e ) {
		cerr << "vrSpeak: std::exception: "<<e.what()<< endl;
		return CMD_FAILURE;
	//} catch( ... ) {
	//	cerr << "vrSpeak: Unknown exception."<< endl;
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
		char* characterId = args.read_token();
		SbmCharacter* character = mcu->character_map.lookup( characterId );
		if( character==NULL ) {
			cerr << "WARNING: BML::Processor::bp_cmd_func(): Unknown character \"" << characterId << "\".  This is probably an error since the command \"bp speech_reply\" is not supposed to be sent over the network, thus it should not be coming from another SBM process." << endl;
			return CMD_SUCCESS;
		}

		char* requestIdStr = args.read_token(); // as string
		SmartBody::RequestId requestId = atoi( requestIdStr );

		char* status = args.read_token();
		char* errorMsg = NULL;
		if( strcmp( status, "SUCCESS" )!=0 ) {
			if( strcmp( status, "ERROR" )==0 ) {
				errorMsg = args.read_remainder_raw();
				if( errorMsg == NULL ) {
					errorMsg = "!!NO ERROR MESSAGE!!";
				}
			} else {
				errorMsg = "!!INVALID SPEECH CALLBACK SUBCOMMAND (bml_old_processor)!!";
				// TODO: include status in errorMsg without memory leak (use &std::String?)
			}
		}
		bp.speechReply( character, requestId, errorMsg, mcu );
		return CMD_SUCCESS;  // Errors are dealt with out of band
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
			cerr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\"."<< endl;
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
			cerr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\"."<< endl;
			return CMD_FAILURE;
		}
	} else if( attribute == "log_synch_points" ||
	           attribute == "log-synch-points" ) {
		string value = args.read_token();
		if( value == "on" ) {
			bp.log_synchpoints = true;
			return CMD_SUCCESS;
		} else if( value == "off" ) {
			bp.log_synchpoints = false;
			return CMD_SUCCESS;
		} else {
			cerr << "ERROR: BML::Processor::set_func(): expected \"on\" or \"off\" for " << attribute <<".  Found \""<<value<<"\"."<< endl;
			return CMD_FAILURE;
		}
	} else if( attribute == "gaze" ) {
		attribute = args.read_token();
		if( attribute == "speed" ) {
			float lumbar   = args.read_float();
			float cervical = args.read_float();
			float eyeball  = args.read_float();

			return BML::Gaze::set_gaze_speed( lumbar, cervical, eyeball );
		} else if( attribute == "smoothing" ) {
			float lumbar   = args.read_float();
			float cervical = args.read_float();
			float eyeball  = args.read_float();

			return BML::Gaze::set_gaze_smoothing( lumbar, cervical, eyeball );
		} else {
			cerr << "ERROR: BML::Processor::set_func(): Unknown gaze attribute \"" << attribute <<"\"."<< endl;
			return CMD_FAILURE;
		}
	} else {
		cerr << "ERROR: BML::Processor::set_func(): Unknown attribute \"" << attribute <<"\"."<< endl;
        return CMD_NOT_FOUND;
	}
}

int BML::Processor::print_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	Processor& bp = mcu->bml_processor;

	string attribute = args.read_token();
	if( attribute == "auto_print_controllers" ||
		attribute == "auto-print-controllers" ) {
		cout << "BodyPlanner auto_print_controllers: "<<
			(bp.auto_print_controllers? "on" : "off") << endl;
		return CMD_SUCCESS;
	} else if( attribute == "auto_print_sequence" ||
	           attribute == "auto-print-sequence" ) {
		cout << "BodyPlanner auto_print_sequence: "<<
			(bp.auto_print_sequence? "on" : "off") << endl;
		return CMD_SUCCESS;
	} else if( attribute == "log_synchpoints" ||
	           attribute == "log-synchpoints" ) {
		cout << "BodyPlanner log_synchpoints: "<<
			(bp.log_synchpoints? "on" : "off") << endl;
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
			cerr << "ERROR: BML::Processor::set_func(): Unknown gaze attribute \"" << attribute <<"\"."<< endl;
			return CMD_FAILURE;
		}
	} else {
		cerr << "ERROR: BML::Processor::print_func(): Unknown attribute \"" << attribute <<"\"."<< endl;
        return CMD_NOT_FOUND;
	}
}
