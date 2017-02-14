#include "SBAssetHandlerHDR.h"
#include <vhcl.h>
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <sbm/GPU/SbmTexture.h>
#include <sb/SBScene.h>
#include <sb/SBAttribute.h>

namespace SmartBody {

SBAssetHandlerHdr::SBAssetHandlerHdr()
{
	assetTypes.push_back("hdr");
}

SBAssetHandlerHdr::~SBAssetHandlerHdr()
{
}

std::vector<SBAsset*> SBAssetHandlerHdr::getAssets(const std::string& path)
{
	std::vector<SBAsset*> assets;

	std::string convertedPath = checkPath(path);
	if (convertedPath == "")
		return assets;

	boost::filesystem::path p(convertedPath);
	std::string fileName = boost::filesystem::basename( p );
	std::string extension =  boost::filesystem::extension( p );

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	std::string textureName = fileName + extension;
	texManager.loadTexture(SbmTextureManager::TEXTURE_HDR_MAP, textureName.c_str(), convertedPath.c_str());
	SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_HDR_MAP, textureName.c_str());
	if (tex)
	{		
		assets.push_back(tex);		
	}

	texManager.updateEnvMaps();

	return assets;
}


};
