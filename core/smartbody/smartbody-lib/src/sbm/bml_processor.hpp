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

#include "xercesc_utils.hpp"
#include "bml.hpp"
#include "sr_hash_map.h"

#include <boost/shared_ptr.hpp>


// Forward Declaration
class mcuCBHandle;


namespace BML {
	class Processor {
		///////////////////////////////////////////////////////////////////////
		//  Private Data Structures
	public:
		struct BodyPlannerException {
			const char* message;

			BodyPlannerException( const char* message )
				: message(message)
			{}
		};

	private:
		struct BodyPlannerMsg {
			const char			*agentId;
			const char			*recipientId;
			const char			*msgId;
			const SbmCharacter	*agent;
			const DOMDocument	*xml;

			const std::string   requestId;


			BodyPlannerMsg( const char *agentId, const char *recipientId, const char *msgId, const SbmCharacter *agent, DOMDocument *xml );
			~BodyPlannerMsg();
		};

		//////////////////////////////////////////////////////////////////////////
		// Private Constants

		// Private Data
		HandlerBase * xmlErrorHandler;
		srHashMap<BML::BmlRequest> requests;     // indexed by msgId
		srHashMap<BML::SpeechRequest> speeches; // indexed by RequestId

		bool auto_print_controllers;
		bool auto_print_sequence;
		bool log_synchpoints;
		bool warn_unknown_agents;

		boost::shared_ptr<XercesDOMParser> xmlParser;

	public:
		//////////////////////////////////////////////////////////////////////////
		// Public Methods
		Processor();
		virtual ~Processor();

		void reset();

		void set_auto_print_controllers( bool value ) {
			auto_print_controllers = value;
		}

		void set_auto_print_sequence( bool value ) {
			auto_print_sequence = value;
		}

		void set_log_synchpoints( bool value ) {
			log_synchpoints = value;
		}

		/**
		*  Handles vrSpeak messages from agent
		*/
		void vrSpeak( BodyPlannerMsg& bpMsg, mcuCBHandle *mcu );

		/**
		*  Parses <BML> elements
		*/
		void parseBML( DOMElement *el, BML::BmlRequest* request, mcuCBHandle *mcu );

		/**
		*/
		void speechReply( SbmCharacter* character, SmartBody::RequestId requestId, const char* error, mcuCBHandle *mcu );

		/**
		*  Completes final timing calculations and triggers schedule
		*/
		void realizeRequest( BML::BmlRequest* request, BodyPlannerMsg& bpMsg, mcuCBHandle *mcu );

		/**
		*  Handles vrSpoke messages from agent, cleans up old BmlRequest obj.
		*/
		int vrSpoke( BodyPlannerMsg& bpMsg, mcuCBHandle *mcu );





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
		BehaviorRequest* parse_bml_body( DOMElement* elem, SynchPoints& tms, BmlRequest* request, mcuCBHandle *mcu );
		BehaviorRequest* parse_bml_event( DOMElement* elem, SynchPoints& tms, BmlRequest* request, mcuCBHandle *mcu );
		BehaviorRequest* parse_bml_head( DOMElement* elem, SynchPoints& tms, BmlRequest* request, mcuCBHandle *mcu );
	}; // namespace BodyPlanner
};  // end namespace BML


#endif // BML_PROCESSOR_HPP
