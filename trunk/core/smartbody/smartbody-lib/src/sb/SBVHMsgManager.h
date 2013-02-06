#ifndef _SBVHMSGMANAGER_H_
#define _SBVHMSGMANAGER_H_

#include <vhcl.h>
#include <sb/SBTypes.h>
#include <sb/SBService.h>
#include <string>

namespace SmartBody {

class SBVHMsgManager : public SBService
{
	public:
		SBAPI SBVHMsgManager();
		SBAPI ~SBVHMsgManager();

		SBAPI virtual void setEnable(bool val);
		SBAPI virtual bool isEnable();

		SBAPI bool connect();
		SBAPI void disconnect();

		SBAPI int send(const char *op, const char* message);
		SBAPI int send(const char* message);

		SBAPI void setPort(const std::string& port);
		SBAPI const std::string& getPort();
		SBAPI void setServer(const std::string& server);
		SBAPI const std::string& getServer();
		SBAPI void setScope(const std::string& scope);
		SBAPI const std::string& getScope();

	protected:
		static void vhmsgCallback( const char *op, const char *args, void * user_data );

		std::string _port;
		std::string _server;
		std::string _scope;
};

}

#endif