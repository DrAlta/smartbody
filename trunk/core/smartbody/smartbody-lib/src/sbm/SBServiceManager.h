#ifndef _SBSERVICEMANAGER_H_
#define _SBSERVICEMANAGER_H_

#include <vhcl.h>
#include <sbm/SBService.h>

namespace SmartBody {

class SBServiceManager : public SBObject
{
	public:
		SBServiceManager();
		~SBServiceManager();

		int getNumServices();
		std::vector<std::string> getServiceNames();

		SBService* getService(const std::string& name);
		void addService(SBService* service);
		void removeService(const std::string& name);

		std::map<std::string, SBService*>& getServices();
		
	protected:
		std::map<std::string, SBService*> _services;

};

}


#endif