
#include <string>
#include <vector>

#include "SbmDebuggerCommon.h"


class SbmDebuggerClient
{
private:
   //vhmsg::Client m_vhmsg;
   std::vector<std::string> m_processIdList;
   std::string m_sbmId;
   bool m_connectResult;


   Scene m_scene;

public:
   SbmDebuggerClient();
   virtual ~SbmDebuggerClient();

   Scene* GetScene() { return &m_scene; }

   void QuerySbmProcessIds();
   const std::vector<std::string> & GetSbmProcessIds() const { return m_processIdList; }

   void Connect(const std::string & id);
   void Disconnect();
   bool GetConnectResult() { return m_connectResult; }

   void Init();
   void Update();
   void StartUpdates(double updateFrequencyS);
   void EndUpdates();

   void GetResourcePaths();

   void SendSBMCommand(const std::string & command);

   void ProcessVHMsgs(const char * op, const char * args);
};
