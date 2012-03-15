#include "SBBoneBusManager.h"
#include "vhcl.h"

namespace SmartBody {

SBBoneBusManager::SBBoneBusManager()
{
	setName("BoneBus");
	_host = "";

	createStringAttribute("host", "", true, "", 10, false, false, false, "Host where the BoneBus data will be sent. If not set, will use 'localhost'");
}

SBBoneBusManager::~SBBoneBusManager()
{
}

bonebus::BoneBusClient& SBBoneBusManager::getBoneBus()
{
	return _boneBus;
}

void SBBoneBusManager::setEnable(bool val)
{
	SBService::setEnable(val);

	if (val)
	{
		if (_boneBus.IsOpen())
		{
			LOG("Closing Bone Bus connection.");
			_boneBus.CloseConnection();
		}

		std::string host = this->getStringAttribute("host");
		if (host == "")
			host = "localhost";
		bool success = _boneBus.OpenConnection(host.c_str());
		if (!success)
		{
			LOG("Could not open Bone Bus connection to %s", host.c_str());
			SmartBody::BoolAttribute* enableAttribute = dynamic_cast<SmartBody::BoolAttribute*>(getAttribute("enable"));
			if (enableAttribute)
				enableAttribute->setValueFast(false);
			return;
		}
		else
		{
			LOG("Connected Bone Bus to %s", host.c_str());
		}

	}
	else
	{
		if (_boneBus.IsOpen())
		{
			LOG("Closing Bone Bus connection.");
			_boneBus.CloseConnection();
		}
	}

	SmartBody::BoolAttribute* enableAttribute = dynamic_cast<SmartBody::BoolAttribute*>(getAttribute("enable"));
	if (enableAttribute)
		enableAttribute->setValueFast(val);
}

void SBBoneBusManager::setHost(const std::string& host)
{
	_host = host;
	SmartBody::StringAttribute* hostAttribute = dynamic_cast<SmartBody::StringAttribute*>(getAttribute("host"));
	if (hostAttribute)
		hostAttribute->setValueFast(host);
}

const std::string& SBBoneBusManager::getHost()
{
	return _host;
}

void SBBoneBusManager::start()
{
}

void SBBoneBusManager::beforeUpdate(double time)
{
}

void SBBoneBusManager::update(double time)
{
}

void SBBoneBusManager::afterUpdate(double time)
{
}

void SBBoneBusManager::stop()
{
}

void SBBoneBusManager::notify(SBSubject* subject)
{
	SBService::notify(subject);

	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (!attribute)
	{
		return;
	}

	const std::string& name = attribute->getName();
	if (name == "enable")
	{
		bool val = getBoolAttribute("enable");
		setEnable(val);
		return;
	}
	else if (name == "host")
	{
		setHost(getStringAttribute("host"));
		return;
	}

}

}


