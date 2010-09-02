/*
 *  bml_interrupt.cpp - part of SmartBody-lib
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
#include <string>

#include "vhcl_log.h"
#include "mcontrol_util.h"
#include "bml_processor.hpp"
#include "bml_interrupt.hpp"
#include "xercesc_utils.hpp"
#include "bml_xml_consts.hpp"


////// XML ATTRIBUTES
const XMLCh ATTR_ACT[] = L"act";



using namespace std;
using namespace BML;
using namespace xml_utils;


namespace BML {
	const bool ECHO_BP_INTERRUPT_COMMAND = false;

	// Handle interrupt as a sequence, in case it is interrupted
	class InterruptBehavior : public SequenceRequest {
	protected:
		std::string performance_id;
	
	public:
		InterruptBehavior( const std::string& unique_id,
						   const std::string& localId,
		                   const std::string& performance_id,
			               const BehaviorSyncPoints& behav_syncs )
		:	SequenceRequest( unique_id, localId, behav_syncs, 0, 0, 0, 0.5, 0.5 ),  // half second default duration
			performance_id( performance_id )
		{}

		void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu  )
		{
			// Get times from BehaviorSyncPoints
			time_sec strokeAt = behav_syncs.sync_stroke()->time();
			time_sec relaxAt  = behav_syncs.sync_relax()->time();

			VecOfSbmCommand commands;

			// TODO: replace stroke with stroke_start, relax with stroke_end
			time_sec duration = relaxAt - strokeAt;

			ostringstream out;
			out << "bp interrupt " << request->actorId << ' ' << performance_id << ' ' << duration;
			commands.push_back( new SbmCommand( out.str(), (float)strokeAt ) );

			if( ECHO_BP_INTERRUPT_COMMAND ) {
				ostringstream out2;
				out2 << "echo " << out.str();
				commands.push_back( new SbmCommand( out2.str(), (float)strokeAt ) );
			}

			realize_sequence( commands, mcu );
		}
	};
};  // end namespace BML


BehaviorRequestPtr BML::parse_bml_interrupt( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
    const XMLCh* tag      = elem->getTagName();

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);

	const XMLCh* performanceId = elem->getAttribute( ATTR_ACT );
	if( performanceId && *performanceId != 0 ) {
		// performanceId is ASCII, not Unicode
		char* temp_ascii_id = XMLString::transcode( performanceId );
		BehaviorRequestPtr interrupt( new InterruptBehavior( unique_id, localId, temp_ascii_id, behav_syncs ) );
		XMLString::release( &temp_ascii_id );

		return interrupt;
    } else {
		std::wstringstream wstrstr;
        wstrstr << "WARNING: BodyPlannerImpl::parseBML(): <"<<tag<<"> BML tag missing "<<ATTR_ACT<<"= attribute." << endl;
		LOG(convertWStringToString(wstrstr.str()).c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
    }
}
