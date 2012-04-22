#ifndef _SBBONEBUSMANAGER_
#define _SBBONEBUSMANAGER_

#include <sbm/SBService.h>
#include "bonebus.h"

namespace SmartBody {

class SBBoneBusManager : public SmartBody::SBService
{
	public:
		SBBoneBusManager();
		~SBBoneBusManager();

		virtual void setEnable(bool val);
		void setHost(const std::string& host);
		const std::string& getHost();

		virtual void start();
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();

		bonebus::BoneBusClient& getBoneBus();

		virtual void notify(SBSubject* subject);

	private:
		bonebus::BoneBusClient _boneBus;
		std::string _host;

};

}

#endif
