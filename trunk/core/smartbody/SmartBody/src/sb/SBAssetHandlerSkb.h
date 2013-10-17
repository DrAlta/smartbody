#ifndef _SBASSETHANDLERSSKB_H_
#define _SBASSETHANDLERSSKB_H_

#include <sb/SBTypes.h>
#include <vector>
#include <sb/SBAsset.h>
#include <sb/SBAssetHandler.h>


namespace SmartBody {

class SBAssetHandlerSkb: public SBAssetHandler
{
	public:
		SBAPI SBAssetHandlerSkb();
		SBAPI virtual ~SBAssetHandlerSkb();

		SBAPI virtual std::vector<SBAsset*> getAssets(const std::string& path);

};


}

#endif