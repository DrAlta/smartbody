
#include "vhcl.h"

#include "SbmDebuggerServer.h"

#include <stdio.h>
#ifdef WIN32
#include <conio.h>
#endif

#include "vhmsg-tt.h"

#include "sbm/SBScene.h"


using std::string;
using std::vector;


SbmDebuggerServer::SbmDebuggerServer()
{
   m_sbmId = "123";
   m_sbmId2 = "unity-123";
   m_connectResult = false;
   m_updateFrequencyS = 0;
   m_lastUpdate = m_timer.GetTime();
   m_scene = NULL;
}


SbmDebuggerServer::~SbmDebuggerServer()
{
}


void SbmDebuggerServer::Update()
{
   if (m_updateFrequencyS > 0)
   {
      double currentTime = m_timer.GetTime();
      if (currentTime > m_lastUpdate + m_updateFrequencyS)
      {
         m_lastUpdate = currentTime;

         if (m_scene)
         {
            vector<string> charNames = m_scene->getCharacterNames();
            for (size_t i = 0; i < charNames.size(); i++)
            {
               SBCharacter * c = m_scene->getCharacter(charNames[i]);

               size_t numBones = c->getSkeleton()->getNumJoints();

               string msg = vhcl::Format("sbmdebugger %s update", m_sbmId2.c_str());
               msg += vhcl::Format(" character %s bones %d\n", c->getName().c_str(), numBones);

               for (int j = 0; j < c->getSkeleton()->getNumJoints(); j++)
               {
                  SBJoint * joint = c->getSkeleton()->getJoint(j);

                  // beware of temporaries
                  float posx = joint->getPosition().x;
                  float posy = joint->getPosition().y;
                  float posz = joint->getPosition().z;
                  float rotx = joint->getQuaternion().x;
                  float roty = joint->getQuaternion().y;
                  float rotz = joint->getQuaternion().z;
                  float rotw = joint->getQuaternion().w;

                  msg += vhcl::Format("  %s pos %f %f %f rot %f %f %f %f\n", joint->getName().c_str(), posx, posy, posz, rotx, roty, rotz, rotw);
               }

               vhmsg::ttu_notify1(msg.c_str());
            }


            {
               string msg = vhcl::Format("sbmdebugger %s update camera\n", m_sbmId2.c_str());

               msg += vhcl::Format("pos %f %f %f\n", m_camera.pos.x, m_camera.pos.y, m_camera.pos.z);
               msg += vhcl::Format("rot %f %f %f %f\n", m_camera.rot.x, m_camera.rot.y, m_camera.rot.z, m_camera.rot.w);
               msg += vhcl::Format("persp %f %f %f %f\n", m_camera.fovY, m_camera.aspect, m_camera.zNear, m_camera.zFar);

               vhmsg::ttu_notify1(msg.c_str());
            }
         }
      }
   }
}


void SbmDebuggerServer::GenerateInitHierarchyMsg(SBJoint * root, string & msg, int tab)
{
   string name = root->getName();
   float posx = root->offset().x;
   float posy = root->offset().y;
   float posz = root->offset().z;
   const SrQuat & q = root->quat()->prerot();

   msg += string().assign(tab, ' ');
   msg += vhcl::Format("{ %s pos %f %f %f prerot %f %f %f %f\n", name.c_str(), posx, posy, posz, q.x, q.y, q.z, q.w);

   for (int i = 0; i < root->getNumChildren(); i++)
   {
      GenerateInitHierarchyMsg(root->getChild(i), msg, tab + 2);
   }

   msg += string().assign(tab, ' ');
   msg += vhcl::Format("}\n");
}


void SbmDebuggerServer::ProcessVHMsgs(const char * op, const char * args)
{
   string message = string(op) + " " + string(args);
   vector<string> split;
   vhcl::Tokenize(message, split, " \t\n");

   if (split.size() > 0)
   {
      if (split[0] == "sbmdebugger")
      {
         //LOG("SbmDebuggerServer::ProcessVHMsgs() - %s", message.c_str());

         if (split.size() > 1)
         {
            if (split[1] == m_sbmId2)
            {
               if (split.size() > 2)
               {
                  if (split[2] == "connect")
                  {
                     m_connectResult = true;
                     vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s connect_success", m_sbmId2.c_str()).c_str());
                  }
                  else if (split[2] == "send_init")
                  {
                     if (m_scene != NULL)
                     {
                        vector<string> charNames = m_scene->getCharacterNames();
                        for (size_t i = 0; i < charNames.size(); i++)
                        {
                           SBCharacter * c = m_scene->getCharacter(charNames[i]);

                           size_t numBones = c->getSkeleton()->getNumJoints();

                           string msg = vhcl::Format("sbmdebugger %s init", m_sbmId2.c_str());
                           msg += vhcl::Format(" character %s bones %d\n", c->getName().c_str(), numBones);

                           SBJoint * root = c->getSkeleton()->getJoint(0);

                           GenerateInitHierarchyMsg(root, msg, 4);

                           vhmsg::ttu_notify1(msg.c_str());
                        }
                     }
                  }
                  else if (split[2] == "start_update")
                  {
                     if (split.size() > 3)
                     {
                        m_updateFrequencyS = vhcl::ToDouble(split[3]);
                     }
                  }
               }
            }
            else if (split[1] == "queryids")
            {
               LOG("SbmDebuggerServer::ProcessVHMsgs() - queryids");

               vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s id", m_sbmId2.c_str()).c_str());
            }
         }
      }
   }
}
