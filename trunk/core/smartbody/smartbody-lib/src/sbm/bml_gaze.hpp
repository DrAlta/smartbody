/*
 *  bml_gaze.hpp - part of SmartBody-lib
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

#ifndef BML_GAZE_HPP
#define BML_GAZE_HPP

#include "bml.hpp"


// Forward Declaration
class mcuCBHandle;

namespace BML {
	const XMLCh TAG_GAZE[]      = L"gaze";

	namespace Gaze {
		int set_gaze_speed( float lumbar, float cervical, float eyeball );
		void print_gaze_speed();
		int set_gaze_smoothing( float lumbar, float cervical, float eyeball );
		void print_gaze_smoothing();
	};

	BML::BehaviorRequest* parse_bml_gaze( DOMElement* elem, BML::SynchPoints& tms, BML::BmlRequestPtr request, mcuCBHandle *mcu );
};


#endif // BML_GAZE_HPP
