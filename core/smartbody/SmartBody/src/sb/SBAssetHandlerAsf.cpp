#include "SBAssetHandlerAsf.h"
#include <vhcl.h>
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sbm/ParserASFAMC.h>

namespace SmartBody {

SBAssetHandlerAsf::SBAssetHandlerAsf()
{
	assetTypes.push_back("asf");
}

SBAssetHandlerAsf::~SBAssetHandlerAsf()
{
}

std::vector<SBAsset*> SBAssetHandlerAsf::getAssets(const std::string& path)
{
	std::vector<SBAsset*> assets;

	std::string convertedPath = checkPath(path);
	if (convertedPath == "")
		return assets;

	boost::filesystem::path p(convertedPath);
	std::string fileName = boost::filesystem::basename( p );
	std::string extension =  boost::filesystem::extension( p );

	std::ifstream filestream(convertedPath.c_str());
	std::ifstream datastream("");
	SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();			
	skeleton->skfilename(convertedPath.c_str());
	skeleton->setName(fileName + extension);

	double scale = 1.0;
	if (SmartBody::SBScene::getScene()->getAttribute("globalSkeletonScale"))
		scale = SmartBody::SBScene::getScene()->getDoubleAttribute("globalSkeletonScale");

	SkMotion motion;
	bool ok = ParserASFAMC::parse(*skeleton, motion, filestream, datastream, float(scale));
	if (ok)
	{
		assets.push_back(skeleton);
	}

	return assets;
}

};
