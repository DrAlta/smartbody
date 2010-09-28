/*
 *  bml_animation.hpp - part of SmartBody-lib
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

#ifndef BML_ANIMATION_HPP
#define BML_ANIMATION_HPP

#include "bml.hpp"


// Forward Declaration
class mcuCBHandle;

namespace BML {
	const XMLCh TAG_SBM_ANIMATION[] = L"sbm:animation";
	const XMLCh TAG_SBM_PANIMATION[] = L"sbm:panimation";
	const XMLCh ATTR_SPEED[]        = L"ME:twarp";  // MotionEngine:time warp
	const XMLCh ATTR_PVALUE[]        = L"sbm:value";

	BML::BehaviorRequestPtr parse_bml_animation( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu );
	BML::BehaviorRequestPtr parse_bml_panimation( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu );
};


#endif // BML_ANIMATION_HPP
