#ifndef _SBASSETHANDLERSHDR_H_
#define _SBASSETHANDLERSHDR_H_

#include <sb/SBTypes.h>
#include <vector>
#include <sb/SBAsset.h>
#include <sb/SBAssetHandler.h>


namespace SmartBody {

class SBAssetHandlerHdr: public SBAssetHandler
{
	public:
		SBAPI SBAssetHandlerHdr();
		SBAPI virtual ~SBAssetHandlerHdr();

		SBAPI virtual std::vector<SBAsset*> getAssets(const std::string& path);

		
};


}

#endif