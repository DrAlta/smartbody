/*
 *  bml_processor.hpp - part of SmartBody-lib
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
 */

#ifndef BML_PROCESSOR_HPP
#define BML_PROCESSOR_HPP

#include "sr_hash_map.h"
#include "xercesc_utils.hpp"
#include "bml.hpp"

// Use Boost Smart Point Impl until TR1 is finalized
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


#define BMLR_BML2ANIM  0


// Forward Declaration
class mcuCBHandle;


namespace BML {
	const bool LOG_SYNC_POINTS = false;
	const bool LOG_SPEECH      = false;

	const float CONTROLLER_SPEED_MIN_DEFAULT = 0.25;
	const float CONTROLLER_SPEED_MAX_DEFAULT = 4;


	class Processor {
		///////////////////////////////////////////////////////////////////////
		//  Private Data Structures
	public:
		struct BMLProcessorException {
			const char* message;

			BMLProcessorException( const char* message )
				: message(message)
			{}
		};

	private:

		struct BMLProcessorMsg {
			const char			*actorId;
#if USE_RECIPIENT
			const char			*recipientId;
#endif
			const char			*msgId;
			const SbmCharacter	*actor;
			const DOMDocument	*xml;

			const std::string   requestId;

			// Remaining arguments
			srArgBuffer   args;


			// Pass in both actorId and actor to allow future possibility of aliases
			// without loosing track of which name was originally used (for feedback messages)
#if USE_RECIPIENT
			BMLProcessorMsg( const char *actorId, const char *recipientId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, const char* args );
			BMLProcessorMsg( const char *actorId, const char *recipientId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, srArgBuffer& arg );
#else
			BMLProcessorMsg( const char *actorId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, const char* args );
			BMLProcessorMsg( const char *actorId, const char *msgId, const SbmCharacter *actor, DOMDocument *xml, srArgBuffer& arg );
#endif
			~BMLProcessorMsg();
		};

		//////////////////////////////////////////////////////////////////////////
		// Private Constants

		// Private Data
		HandlerBase*       xmlErrorHandler;
		MapOfBmlRequest    bml_requests;    // indexed by buildRequestId(..) string
		MapOfSpeechRequest speeches; // indexed by buildSpeechKey(..) string

		bool auto_print_controllers;
		bool auto_print_sequence;
		bool log_syncpoints;
		bool warn_unknown_agents;
#if BMLR_BML2ANIM
		std::string bml2animText; // [BMLR] Stores the bml 2 animation mapping file
#endif

		boost::shared_ptr<XercesDOMParser> xmlParser;

		float ct_speed_min;
		float ct_speed_max;

	public:
		//////////////////////////////////////////////////////////////////////////
		// Public Methods
		Processor();
		virtual ~Processor();

		void reset();

		bool get_auto_print_controllers() {
			return auto_print_controllers;
		}

		void set_auto_print_controllers( bool value ) {
			auto_print_controllers = value;
		}

		bool get_auto_print_sequence() {
			return auto_print_sequence;
		}

		void set_auto_print_sequence( bool value ) {
			auto_print_sequence = value;
		}

		void set_log_syncpoints( bool value ) {
			log_syncpoints = value;
		}

		void set_warn_unknown_agents( bool value ) {
			warn_unknown_agents = value;
		}

		/**
		 *  Handles "vrAgentBML .. request .." and "vrSpeak .." messages
		 */
		void bml_request( BMLProcessorMsg& bpMsg, mcuCBHandle *mcu );

		/**
		 *  Parses <BML> elements
		 */
		void parseBML( DOMElement *el, BML::BmlRequestPtr request, mcuCBHandle *mcu );

		/**
		 */
		void speechReply( SbmCharacter* character, SmartBody::RequestId requestId, srArgBuffer& response_args, mcuCBHandle *mcu );

//// Moved to BmlRequest::realize( Processor*, mcuCBHandle* )
//		/**
//		 *  Completes final timing calculations and triggers schedule
//		 */
//		void realizeRequest( BML::BmlRequestPtr request, BMLProcessorMsg& bpMsg, mcuCBHandle *mcu );

		/**
		 *  Begins interrupting a BML performance.
		 */
		int interrupt( SbmCharacter* actor, const std::string& performance_id, time_sec duration, mcuCBHandle* mcu );

		/**
		 *  Handles "vrAgentBML .. end .." messages
		 */
		int bml_end( BMLProcessorMsg& bpMsg, mcuCBHandle *mcu );




		///////////////////////////////////////////////////////////////////////
		//  Static Command and Message Hooks

		/**
		 *  Notify BodyPlanner of vrAgentBML commands/messages.
		 */
		static int vrAgentBML_cmd_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
		 *  Notify BodyPlanner of vrSpeak command/message.
		 */
		static int vrSpeak_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
		 *  Notify BodyPlanner of vrSpoke messages.
		 */
		static int vrSpoke_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
		 *  Notify BodyPlanner of completed speech request.
		 */
		static int bpSpeechReady_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
		 *  Notify BodyPlanner of request timings.
		 */
		static int bp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
		 *  Handles the command "set bodyplanner" or "set bp"
		 */
		static int set_func( srArgBuffer& args, mcuCBHandle *mcu );

		/**
 		 *  Handles the command "print bodyplanner" or "print bp"
		 */
		static int print_func( srArgBuffer& args, mcuCBHandle *mcu );

	protected:
		//////////////////////////////////////////////////////////////////////////
		// Protected Methods
#if USE_RECIPIENT
		BmlRequestPtr createBmlRequest( const SbmCharacter* agent, const std::string & actorId, const std::string & requestId, const std::string & recipientId, const std::string & msgId );
#else
		BmlRequestPtr createBmlRequest( const SbmCharacter* agent, const std::string & actorId, const std::string & requestId, const std::string & msgId );
#endif

		/**
		 *  Parses a group of behavior tags, such as <bml> or <required>.
		 *  The workhorse function of parseBML(..)
		 */
		void parseBehaviorGroup( DOMElement *el, BML::BmlRequestPtr request, mcuCBHandle *mcu, size_t&, bool );

		BehaviorRequestPtr parse_bml_body( DOMElement* elem, std::string& unique_id, SyncPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu );
		BehaviorRequestPtr parse_bml_head( DOMElement* elem, std::string& unique_id, SyncPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu );


///  Is the following necessary anymore?
//		friend BML::BehaviorRequest* BML::parse_bml_interrupt( DOMElement* elem, BML::SyncPoints& tms, BML::BmlRequestPtr request, mcuCBHandle *mcu );
#if BMLR_BML2ANIM
		BehaviorRequest* parse_bml_to_anim( DOMElement* elem, SyncPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ); // [BMLR]
#endif
#if BMLR_BML2ANIM
		BehaviorRequest* parse_bml_to_anim( DOMElement* elem, SynchPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ); // [BMLR]
#endif
	}; // class Processor
};  // end namespace BML


#endif // BML_PROCESSOR_HPP
