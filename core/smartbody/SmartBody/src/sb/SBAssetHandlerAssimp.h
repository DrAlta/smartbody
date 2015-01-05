#ifndef _SBASSETHANDLERASSIMP_H_

#include <sb/SBTypes.h>
#include <vector>
#include <sb/SBAsset.h>
#include <sb/SBAssetHandler.h>

namespace SmartBody {

class SBAssetHandlerAssimp : public SBAssetHandler
{
	public:
		SBAPI SBAssetHandlerAssimp();
		SBAPI virtual ~SBAssetHandlerAssimp();

		SBAPI virtual std::vector<SBAsset*> getAssets(const std::string& path);

};


}

#endif