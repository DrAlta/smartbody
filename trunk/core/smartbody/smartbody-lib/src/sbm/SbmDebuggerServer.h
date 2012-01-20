
#include <string>
#include <vector>

#include "SbmDebuggerCommon.h"


namespace SmartBody { class SBScene; }
namespace SmartBody { class SBJoint; }


class SbmDebuggerServer
{
private:
   std::vector<std::string> m_processIdList;
   std::string m_sbmId;
   std::string m_sbmId2;
   bool m_connectResult;
   double m_updateFrequencyS;
   double m_lastUpdate;
   vhcl::Timer m_timer;
   SmartBody::SBScene * m_scene;

public:
   DebuggerCamera m_camera;
   bool m_rendererIsRightHanded;


public:
   SbmDebuggerServer();
   virtual ~SbmDebuggerServer();


   void Init() {}

   void SetSBScene(SmartBody::SBScene * scene) { m_scene = scene; }
   void SetID(const std::string & id) { m_sbmId2 = id; }

   void Update();

   void GenerateInitHierarchyMsg(SmartBody::SBJoint * root, std::string & msg, int tab);

   void ProcessVHMsgs(const char * op, const char * args);
};
