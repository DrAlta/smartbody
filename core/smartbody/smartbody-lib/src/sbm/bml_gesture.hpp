#ifndef BML_GESTURE_HPP
#define BML_GESTURE_HPP

#include "bml.hpp"
#include "BMLDefs.h"

// Forward Declaration
class mcuCBHandle;

namespace BML
{
	BML::BehaviorRequestPtr parse_bml_gesture( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu );
};

#endif