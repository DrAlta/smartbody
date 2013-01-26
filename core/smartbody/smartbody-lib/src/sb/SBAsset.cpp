#include "SBAsset.h"

namespace SmartBody {

SBAsset::SBAsset()
{
	_loaded = false;
}

SBAsset::~SBAsset()
{
}


bool SBAsset::isLoaded()
{
	return _loaded;
}

void SBAsset::load()
{
}

void SBAsset::unload()
{
}

}

