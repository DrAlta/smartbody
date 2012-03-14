
#include "vhcl.h"

#include "SbmDebuggerServer.h"

#include <stdio.h>

#include "vhmsg-tt.h"

#include "sbm/SBScene.h"


using std::string;
using std::vector;


SbmDebuggerServer::SbmDebuggerServer()
{
   m_sbmFriendlyName = "sbm";
   m_connectResult = false;
   m_updateFrequencyS = 0;
   m_lastUpdate = m_timer.GetTime();
   m_scene = NULL;
   m_rendererIsRightHanded = true;
}


SbmDebuggerServer::~SbmDebuggerServer()
{
}


static const int NETWORK_PORT_TCP = 15104;

void * m_sockTCP;
vector< void * > m_sockConnectionsTCP;


void SbmDebuggerServer::Init()
{
   bool ret = SocketStartup();
   if (!ret)
   {
      printf("SocketStartup() failed\n");
   }


   m_hostname = SocketGetHostname();


   m_sockTCP = SocketOpenTcp();
   if (m_sockTCP == NULL)
   {
      printf( "SocketOpenTcp() failed\n" );
      SocketShutdown();
      return;
   }


   //ret = SocketSetReuseAddress(m_sockTCP, true);


   int portToTry = NETWORK_PORT_TCP;
   int portMax = NETWORK_PORT_TCP + 100;

   while (portToTry < portMax)
   {
      ret = SocketBind(m_sockTCP, portToTry);
      if (ret)
      {
         break;
      }

      printf( "SocketBind() failed. Trying next port up.\n" );
      portToTry++;
   }

   if (portToTry >= portMax)
   {
      printf( "SocketBind() failed.\n" );
      SocketClose(m_sockTCP);
      m_sockTCP = NULL;
      SocketShutdown();
      return;
   }


   m_port = portToTry;

   m_fullId = vhcl::Format("%s:%d:%s", m_hostname.c_str(), m_port, m_sbmFriendlyName.c_str());


   SocketSetBlocking(m_sockTCP, false);

   SocketListen(m_sockTCP);


   //return true;
}

void SbmDebuggerServer::Close()
{
   if ( m_sockTCP )
   {
      SocketClose(m_sockTCP);
      m_sockTCP = NULL;
   }

   SocketShutdown();
}

void SbmDebuggerServer::SetID(const std::string & id)
{
   m_sbmFriendlyName = id;
   m_fullId = vhcl::Format("%s:%d:%s", m_hostname.c_str(), m_port, m_sbmFriendlyName.c_str());
}

void SbmDebuggerServer::Update()
{
   if (m_updateFrequencyS > 0)
   {
      double currentTime = m_timer.GetTime();
      if (currentTime > m_lastUpdate + m_updateFrequencyS)
      {
         m_lastUpdate = currentTime;

         // I have to send pawn, camera, and character updates all on one SocketSend
         // call otherwise there is a massive delay on the receiving end
         bool sentCamUpdate = false;
         bool sentPawnUpdates = false;
         if (m_scene)
         {
            vector<string> charNames = m_scene->getCharacterNames();
            for (size_t i = 0; i < charNames.size(); i++)
            {
               SBCharacter * c = m_scene->getCharacter(charNames[i]);

               size_t numBones = c->getSkeleton()->getNumJoints();

               string msg = vhcl::Format("sbmdebugger %s update", m_fullId.c_str());
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

                  msg += vhcl::Format("  %s pos %.3f %.3f %.3f rot %.3f %.3f %.3f %.3f\n", joint->getName().c_str(), posx, posy, posz, rotx, roty, rotz, rotw);
               }

               msg += ";";

               if (!sentCamUpdate)
               {
                  // camera update
                  msg += vhcl::Format("sbmdebugger %s update camera\n", m_fullId.c_str());

                  msg += vhcl::Format("pos %.3f %.3f %.3f\n", m_camera.pos.x, m_camera.pos.y, m_camera.pos.z);
                  msg += vhcl::Format("rot %.3f %.3f %.3f %.3f\n", m_camera.rot.x, m_camera.rot.y, m_camera.rot.z, m_camera.rot.w);
                  msg += vhcl::Format("persp %.3f %.3f %.3f %.3f\n", m_camera.fovY, m_camera.aspect, m_camera.zNear, m_camera.zFar);
                  msg += ";";
                  sentCamUpdate = true;
               }

               if (!sentPawnUpdates)
               {
                  // sbmdebugger <sbmid> update pawn <name> pos <x y z> rot <x y z w> geom <s> size <s> 
                  vector<string> pawnNames = m_scene->getPawnNames();
                  for (size_t i = 0; i < pawnNames.size(); i++)
                  {
                     SBPawn* p = m_scene->getPawn(pawnNames[i]);
                     msg += vhcl::Format("sbmdebugger %s update pawn %s", m_fullId.c_str(), p->getName().c_str());
                     SrVec pos = p->getPosition();
                     SrQuat rot = p->getOrientation();
                     
                     msg += vhcl::Format(" pos %.3f %.3f %.3f rot %.3f %.3f %.3f %.3f", 
                        pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, rot.w); 

                     SbmGeomObject* geom = p->getGeomObject();
                     if (geom)
                        msg += vhcl::Format(" geom %s size %.3f", geom->geomType(), geom->getGeomSize().x);    
                     else 
                        msg += vhcl::Format(" geom %s size %.3f", "sphere", 10.0f);
                                        
                     msg += ";";
                  }
                  sentPawnUpdates = true;
               }

               for ( size_t i = 0; i < m_sockConnectionsTCP.size(); i++ )
               {
                  //static int c = 0;
                  //printf("TCP Send %d\n", c++);
                  SocketSend(m_sockConnectionsTCP[ i ], msg);
               }
            }
         }
      }
   }





   if (SocketIsDataPending(m_sockTCP))
   {
      void * socket = SocketAccept(m_sockTCP);
      if (socket)
      {
         SocketSetBlocking(socket, false);
         m_sockConnectionsTCP.push_back(socket);
      }
   }


   for ( size_t i = 0; i < m_sockConnectionsTCP.size(); i++ )
   {
      void * s = m_sockConnectionsTCP[i];

      bool tcpDataPending;
      tcpDataPending = SocketIsDataPending(s);

      std::string overflowData = "";
      while ( tcpDataPending )
      {
         tcpDataPending = 0;

         char str[ 1000 ];
         memset( str, 0, sizeof( char ) * 1000 );

         int bytesReceived = SocketReceive(s, str, sizeof( str ) - 1);
         if ( bytesReceived > 0 )
         {
            string recvStr = overflowData + str;

            vector< string > tokens;
            vhcl::Tokenize( recvStr, tokens, ";" );

            for ( int t = 0; t < (int)tokens.size(); t++ )
            {
               vector< string > msgTokens;
               vhcl::Tokenize( tokens[ t ], msgTokens, "|" );

               if ( msgTokens.size() > 0 )
               {
                  if ( msgTokens[ 0 ] == "TestMessage" )
                  {
                     if ( msgTokens.size() > 4 )
                     {
                        string s = msgTokens[ 1 ];
                     }
                  }
               }
            }
         }
         else if (bytesReceived < 0)
         {
            m_sockConnectionsTCP.erase(m_sockConnectionsTCP.begin() + i);
            tcpDataPending = false;
            continue;
         }

         tcpDataPending = SocketIsDataPending(s);
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
            if (split[1] == m_fullId)
            {
               if (split.size() > 2)
               {
                  if (split[2] == "connect")
                  {
                     m_connectResult = true;
                     vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s connect_success", m_fullId.c_str()).c_str());
                  }
                  else if (split[2] == "disconnect")
                  {
                     m_sockConnectionsTCP.clear();

                     m_updateFrequencyS = 0;
                     m_connectResult = false;
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

                           string msg = vhcl::Format("sbmdebugger %s init", m_fullId.c_str());
                           msg += vhcl::Format(" character %s bones %d\n", c->getName().c_str(), numBones);

                           SBJoint * root = c->getSkeleton()->getJoint(0);

                           GenerateInitHierarchyMsg(root, msg, 4);

                           vhmsg::ttu_notify1(msg.c_str());


                           msg = vhcl::Format("sbmdebugger %s init", m_fullId.c_str());
                           msg += vhcl::Format(" renderer right_handed %d\n", m_rendererIsRightHanded ? 1 : 0);
                           vhmsg::ttu_notify1(msg.c_str());
                        }

                        vector<string> pawnNames = m_scene->getPawnNames();
                        for (size_t i = 0; i < pawnNames.size(); i++)
                        {
                           SBPawn* p = m_scene->getPawn(pawnNames[i]);
                           string msg = vhcl::Format("sbmdebugger %s init pawn %s", m_fullId.c_str(), p->getName().c_str());
                           SrVec pos = p->getPosition();
                           SrQuat rot = p->getOrientation();
                           msg += vhcl::Format(" pos %.3f %.3f %.3f rot %.3f %.3f %.3f %.3f geom %s size %.3f", 
                              pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, rot.w, "sphere", 10.0f);

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
                  else if (split[2] == "end_update")
                  {
                     m_updateFrequencyS = 0;
                  }
               }
            }
            else if (split[1] == "queryids")
            {
               LOG("SbmDebuggerServer::ProcessVHMsgs() - queryids");

               vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s id", m_fullId.c_str()).c_str());
            }
         }
      }
   }
}
