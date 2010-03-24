/*
 *  bml_event.cpp - part of SmartBody-lib
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
 */

#include <iostream>
#include <sstream>

#include "bml_event.hpp"


using namespace std;
using namespace BML;

const bool LOG_EVENT_COMMAND = false;


namespace BML {
	class EventRequest : public SequenceRequest {
	protected:
		const std::string message;
	
	public:
		EventRequest( const std::string& unique_id, const char* message,
			          const BehaviorSyncPoints& syncs_in )
		:	SequenceRequest( unique_id, syncs_in,
							 /* Default Timing */ 0, 0, 0, 0, 0 ),
			message( message )
		{}
	
		void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
		{
			time_sec strokeAt = behav_syncs.sync_stroke()->time();

			VecOfSbmCommand commands;

			ostringstream cmd;
			cmd << "send " << message;

			if( LOG_EVENT_COMMAND ) {
				cout << "DEBUG: EventRequest::realize_impl(): Scheduling \"" << unique_id << "\" command: " << endl << "\t" << cmd.str() << endl;

				ostringstream echo;
				echo << "echo DEBUG: EventRequest::realize_impl(): Sending \"" << unique_id << "\" command: " << endl << "\t" << cmd.str();
				commands.push_back( new SbmCommand( echo.str(), (float)strokeAt ) );
			}

			commands.push_back( new SbmCommand( cmd.str(), (float)strokeAt ) );

			realize_sequence( commands, mcu );
		}
	};
};  // end namespace BML

BehaviorRequestPtr BML::parse_bml_event( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
    const XMLCh* attrMesg = elem->getAttribute( ATTR_MESSAGE );

	if( attrMesg && attrMesg[0]!='\0' ) {
        return BehaviorRequestPtr( new EventRequest( unique_id, XMLString::transcode( attrMesg ), behav_syncs ) );
	} else {
		// TODO: Use exception?
        wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_MESSAGE<<"= attribute.  Behavior ignored."<< endl;
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
