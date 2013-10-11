#include "SBAssetHandler.h"

namespace SmartBody {

SBAssetHandler::SBAssetHandler()
{
}

SBAssetHandler::~SBAssetHandler()
{
}

std::vector<std::string> SBAssetHandler::getAssetTypes()
{
	return assetTypes;
}

std::vector<SBAsset*> SBAssetHandler::getAssets(const std::string& path)
{
	return std::vector<SBAsset*>();
}

};

