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

#include "vhcl.h"
#include <iostream>
#include <sstream>


#include "bml_event.hpp"
#include "bml_xml_consts.hpp"
#include "BMLDefs.h"
#include "xercesc_utils.hpp"

using namespace std;
using namespace BML;
using namespace xml_utils;

const bool LOG_EVENT_COMMAND = false;


BML::EventRequest::EventRequest( const std::string& unique_id, const std::string& localId, const char* message,	const BehaviorSyncPoints& syncs_in, std::string spName )
						:	SequenceRequest( unique_id, localId, syncs_in,
						    /* Default Timing */ 0, 0, 0, 0, 0 ),
							message( message ),
							syncPointName( spName )
{
}
	
void BML::EventRequest::realize_impl( BmlRequestPtr request, mcuCBHandle* mcu )
{
	time_sec strokeAt = behav_syncs.sync_stroke()->time();

	VecOfSbmCommand commands;

	ostringstream cmd;
	cmd << "send " << message;

	if( LOG_EVENT_COMMAND ) {
		cout << "DEBUG: EventRequest::realize_impl(): Scheduling \"" << unique_id << "\" command: " << endl << "\t" << cmd.str() << endl;

		ostringstream echo;
		echo << "echo DEBUG: EventRequest::realize_impl(): Sending \"" << unique_id << "\" command: " << endl << "\t" << cmd.str();
		string str = echo.str();
		commands.push_back( new SbmCommand( str, (float)strokeAt ) );
	}

	string str = cmd.str();
	commands.push_back( new SbmCommand( str, (float)strokeAt ) );

	realize_sequence( commands, mcu );
}

const std::string BML::EventRequest::getMessage() 
{ 
	return message; 
}

std::string BML::EventRequest::getSyncPointName() 
{ 
	return syncPointName; 
}


BehaviorRequestPtr BML::parse_bml_event( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {

    const XMLCh* tag      = elem->getTagName();
	const XMLCh* attrMesg = elem->getAttribute( BMLDefs::ATTR_MESSAGE );

	const XMLCh* attrStroke = elem->getAttribute( BMLDefs::ATTR_STROKE );
	std::string spName;
	if (attrStroke)
	{
		spName = XMLString::transcode(attrStroke);
	}

	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	std::string localId;
	if (id)
	{
		localId = XMLString::transcode(id);
	}


	if( attrMesg && attrMesg[0]!='\0' ) {
        return BehaviorRequestPtr( new EventRequest( unique_id, localId, XMLString::transcode( attrMesg ), behav_syncs, spName ) );
	} else {
		xml_parse_error( BMLDefs::ATTR_MESSAGE, elem );
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
