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


namespace BML {
	class EventRequest : public SequenceRequest {
	protected:
		const std::string message;
	
	public:
		EventRequest( const std::string& unique_id, const char* message,
			          const SyncPoints& syncs_in )
		:	SequenceRequest( unique_id, syncs_in,
							 /* Default Timing */ 0, 0, 0, 0, 0 ),
			message( message )
		{}
	
		void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
		{
			time_sec strokeAt = syncs.sp_stroke->time;

			VecOfSbmCommand commands;

			ostringstream out;
			out << "send " << message;
			commands.push_back( new SbmCommand( out.str(), (float)strokeAt ) );

			realize_sequence( commands, mcu );
		}
	};
};  // end namespace BML

BehaviorRequestPtr BML::parse_bml_event( DOMElement* elem, const std::string& unique_id, SyncPoints& tms, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();
    const XMLCh* attrMesg = elem->getAttribute( ATTR_MESSAGE );

	if( attrMesg && attrMesg[0]!='\0' ) {
        return BehaviorRequestPtr( new EventRequest( unique_id, XMLString::transcode( attrMesg ), tms ) );
	} else {
		// TODO: Use exception?
        wcerr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_MESSAGE<<"= attribute.  Behavior ignored."<< endl;
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
