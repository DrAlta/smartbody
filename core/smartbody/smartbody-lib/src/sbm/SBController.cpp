#include "SBController.h"

namespace SmartBody {

SBController::SBController() : MeController()
{
	controllerType = "SBController";
}

SBController::SBController(const SBController& controller)
{
	// ?
}

SBController::~SBController()
{
}

const std::string& SBController::getType()
{
	return controller_type();
}

void SBController::setIgnore(bool val)
{
	setEnable(!val);
	BoolAttribute* enableAttribute = dynamic_cast<BoolAttribute*>(getAttribute("enable"));

	if (enableAttribute)
	{
		enableAttribute->setValueFast(!val);
	}
}

bool SBController::isIgnore()
{
	return !isEnabled();
}

void SBController::setDebug(bool val)
{
	record_buffer_changes(val);
}

bool SBController::isDebug()
{
	return is_record_buffer_changes();
}

double SBController::getDuration()
{
	return controller_duration();
}

};

