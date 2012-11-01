#ifndef BML_NOISE_HPP
#define BML_NOISE_HPP

#include "bml.hpp"


// Forward Declaration
class mcuCBHandle;

namespace BML {

	BML::BehaviorRequestPtr parse_bml_noise( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu );
};


#endif // BML_SACCADE_HPP
