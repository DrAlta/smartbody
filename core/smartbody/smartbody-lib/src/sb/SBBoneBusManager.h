#ifndef _SBBONEBUSMANAGER_
#define _SBBONEBUSMANAGER_

#include <sb/SBTypes.h>
#include <sb/SBService.h>
#include "bonebus.h"

namespace SmartBody {

class SBBoneBusManager : public SmartBody::SBService
{
	public:
		SBAPI SBBoneBusManager();
		SBAPI ~SBBoneBusManager();

		SBAPI virtual void setEnable(bool val);
		SBAPI void setHost(const std::string& host);
		SBAPI const std::string& getHost();

		SBAPI virtual void start();
		SBAPI virtual void beforeUpdate(double time);
		SBAPI virtual void update(double time);
		SBAPI virtual void afterUpdate(double time);
		SBAPI virtual void stop();

		SBAPI bonebus::BoneBusClient& getBoneBus();

		SBAPI virtual void notify(SBSubject* subject);

	private:
		bonebus::BoneBusClient _boneBus;
		std::string _host;

};

}

#endif
