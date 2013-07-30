#include "SBScript.h"

namespace SmartBody {

SBScript::SBScript() : SBAsset()
{
	_enabled = true;
}

SBScript::~SBScript()
{
}

void SBScript::setEnable(bool val)
{
	_enabled = val;
}

bool SBScript::isEnable()
{
	return _enabled;
}

}
