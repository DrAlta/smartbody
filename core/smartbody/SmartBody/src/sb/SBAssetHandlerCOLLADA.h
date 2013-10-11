#ifndef _SBASSETHANDLERCOLLADA_H_

#include <sb/SBTypes.h>
#include <vector>
#include <sb/SBAsset.h>

namespace SmartBody {

class SBAssetHandlerCOLLADA
{
	public:
		SBAPI SBAssetHandlerCOLLADA();
		SBAPI virtual ~SBAssetHandlerCOLLADA();

		SBAPI virtual std::vector<SBAsset*> getAssets(const std::string& path);

	protected:
		std::vector<std::string> assetTypes;
};


}

#endif