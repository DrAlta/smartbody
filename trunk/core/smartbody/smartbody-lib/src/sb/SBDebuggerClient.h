#ifndef SBM_DEBUGGER_CLIENT_H_
#define SBM_DEBUGGER_CLIENT_H_

#include <vhcl_socket.h>
#include <string>
#include <vector>

#include "sbm/NetRequest.h"

class SBDebuggerClient
{
private:
   //vhmsg::Client m_vhmsg;
   std::vector<std::string> m_processIdList;
   std::string m_sbmId;
   bool m_connectResult;
   bool m_initFinish;
   vhcl::socket_t m_sockTCP_client;

   NetRequestManager m_netRequestManager;

public:
   SBDebuggerClient();
   virtual ~SBDebuggerClient();

   void QuerySbmProcessIds();
   const std::vector<std::string> & GetSbmProcessIds() const { return m_processIdList; }

   void Connect(const std::string & id);
   void Disconnect();
   bool GetConnectResult() { return m_connectResult; }

   void Init();
   void Update();
   void StartUpdates(double updateFrequencyS);
   void EndUpdates();

   void SendSBMCommand(int requestId, const std::string & command);
   void SendSBMCommand(int requestId, const std::string & returnValue, const std::string & functionNameandParams,
      NetRequest::RequestCallback cb, void* callbackOwner = NULL);

   void ProcessVHMsgs(const char * op, const char * args);
};

#endif
