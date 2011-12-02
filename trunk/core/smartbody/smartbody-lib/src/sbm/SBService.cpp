#include "SBService.h"

namespace SmartBody {

SBService::SBService() : SBObject()
{
	_enabled = false;
}

SBService::~SBService()
{
}

void SBService::setEnable(bool val)
{
	_enabled = val;
}

bool SBService::isEnable()
{
	return _enabled;
}

}
