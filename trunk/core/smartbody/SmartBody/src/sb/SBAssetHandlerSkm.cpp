#include "SBAssetHandlerSkm.h"
#include <vhcl.h>
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>

namespace SmartBody {

SBAssetHandlerSkm::SBAssetHandlerSkm()
{
	assetTypes.push_back("skm");
}

SBAssetHandlerSkm::~SBAssetHandlerSkm()
{
}

std::vector<SBAsset*> SBAssetHandlerSkm::getAssets(const std::string& path)
{
	std::vector<SBAsset*> assets;

	boost::filesystem::path pathname(path);
	if( !boost::filesystem::exists( pathname ) )
	{
#if (BOOST_VERSION > 104400)
		LOG("Asset path \"%s\" not found.",  pathname.string().c_str());
#else
		LOG("Asset path \"%s\" not found.", pathname.native_file_string().c_str());
#endif
		return assets;
	}

	if( boost::filesystem::is_directory( pathname ) ) // path indicates a directory
	{
		#if (BOOST_VERSION > 104400)
		LOG("Asset path \"%s\" is a directory.",  pathname.string().c_str());
#else
		LOG("Asset path \"%s\" is a directory.", pathname.native_file_string().c_str());
#endif
		return assets;
	}

	SmartBody::SBMotion* motion = new SmartBody::SBMotion();
	bool parseSuccessful = false;

	std::string convertedPath = pathname.string();
#ifdef WIN32
	boost::replace_all(convertedPath, "\\", "/");
#endif
	SrInput in( convertedPath.c_str(), "rt" );
	SrString fullin_string;
	in.getall( fullin_string );
	SrInput fullin( (const char *)fullin_string );
	fullin.filename( convertedPath.c_str() ); // copy filename for error message
			
	double scale = SmartBody::SBScene::getScene()->getDoubleAttribute("globalMotionScale");
	parseSuccessful = motion->load( fullin, scale );
	if (parseSuccessful)
		assets.push_back(motion);
	else
		LOG("Could not .skm file %s", convertedPath.c_str());

	return assets;
}

};
