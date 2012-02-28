
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


#include <winsock.h>

static const int NETWORK_PORT_TCP = 15104;

bool m_wsaStartupCalled = false;
SOCKET m_sockTCP;
sockaddr_in m_addrTCP;
vector< SOCKET > m_sockConnectionsTCP;


void SbmDebuggerServer::Init()
{
   WSADATA wsaData;
   int err = WSAStartup( MAKEWORD(2,2), &wsaData );
   if ( err != 0 )
   {
      printf( "WSAStartup failed. Code: %d\n", err );
      //return false;
   }

   m_wsaStartupCalled = true;


   char * hostname = new char [256];
   int ret = gethostname(hostname, 256);  // 256 is guaranteed to be long enough  http://msdn.microsoft.com/en-us/library/windows/desktop/ms738527(v=vs.85).aspx
   m_hostname = hostname;
   delete [] hostname;


   m_sockTCP = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if ( m_sockTCP == INVALID_SOCKET )
   {
      printf( "Couldn't create socket tcp.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      m_sockTCP = NULL;
      WSACleanup();
      m_wsaStartupCalled = false;
      //return false;
   }


   int reuseAddr = 1;
   setsockopt( m_sockTCP, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof( int ) );

   int portToTry = NETWORK_PORT_TCP;
   int portMax = NETWORK_PORT_TCP + 10;

   while (portToTry < portMax)
   {
      m_addrTCP.sin_family      = AF_INET;
      m_addrTCP.sin_addr.s_addr = INADDR_ANY;
      m_addrTCP.sin_port        = htons( portToTry );
      memset( m_addrTCP.sin_zero, 0, sizeof( m_addrTCP.sin_zero ) );

      if ( bind( m_sockTCP, (sockaddr *)&m_addrTCP, sizeof( m_addrTCP ) ) == 0 )
      {
         break;
      }

      printf( "bind() failed. Trying next port up.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      portToTry++;
   }

   if (portToTry >= portMax)
   {
      printf( "bind() failed.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      closesocket( m_sockTCP );
      m_sockTCP = NULL;
      WSACleanup();
      m_wsaStartupCalled = false;
      //return false;
   }


   m_port = portToTry;

   m_fullId = vhcl::Format("%s:%d:%s", m_hostname.c_str(), m_port, m_sbmFriendlyName.c_str());


   {
   u_long nonBlocking = 1;
   ioctlsocket( m_sockTCP, FIONBIO, &nonBlocking );
   }


   listen( m_sockTCP, 10 );


   //return true;
}

void SbmDebuggerServer::Close()
{
   if ( m_sockTCP )
   {
      closesocket( m_sockTCP );
      m_sockTCP = NULL;
   }


   if ( m_wsaStartupCalled )
   {
      WSACleanup();
      m_wsaStartupCalled = false;
   }
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


               //vhmsg::ttu_notify1(msg.c_str());
               for ( size_t i = 0; i < m_sockConnectionsTCP.size(); i++ )
               {
                   //static int c = 0;
                   //printf("TCP Send %d\n", c++);


                  int bytesSent = send( m_sockConnectionsTCP[ i ], msg.c_str(), msg.length(), 0 );
                  if ( bytesSent < 0 )
                  {
                     int errnum = WSAGetLastError();
                     //printf( "socket error: %d\n", errnum );
                     //fprintf( fp, "socket error: %d\n", errnum );
                  }
                  if ( bytesSent > 0 )
                  {
                     //printf( "send: %ld\n", bytesSent );
                     //fprintf( fp, "send: %ld\n", bytesSent );
                  }
               }

            }


            {
               string msg = vhcl::Format("sbmdebugger %s update camera\n", m_fullId.c_str());

               msg += vhcl::Format("pos %.3f %.3f %.3f\n", m_camera.pos.x, m_camera.pos.y, m_camera.pos.z);
               msg += vhcl::Format("rot %.3f %.3f %.3f %.3f\n", m_camera.rot.x, m_camera.rot.y, m_camera.rot.z, m_camera.rot.w);
               msg += vhcl::Format("persp %.3f %.3f %.3f %.3f\n", m_camera.fovY, m_camera.aspect, m_camera.zNear, m_camera.zFar);

               vhmsg::ttu_notify1(msg.c_str());
            }
         }
      }
   }






   {
      fd_set readfds;
      FD_ZERO( &readfds );
      FD_SET( m_sockTCP, &readfds );
      timeval timeout = { 0, 0 };  // return immediately
      int error = select( 0, &readfds, 0, 0, &timeout );   // 1st parameter ignored by winsock
      if ( error == SOCKET_ERROR )
      {
         printf( "TCP - Error checking status\n" );
      }
      else if ( error != 0 )
      {
         SOCKET newSock;
         sockaddr_in newToAddr;

         int i = sizeof( sockaddr_in );
         newSock = accept( m_sockTCP, (sockaddr *)&newToAddr, &i );

         u_long nonBlocking = 1;
         ioctlsocket( newSock, FIONBIO, &nonBlocking );

         m_sockConnectionsTCP.push_back( newSock );

         //printf( "New Connection!\n" );

         //if ( m_onClientConnectFunc )
         {
            string clientIP = inet_ntoa( newToAddr.sin_addr );
            //m_onClientConnectFunc( clientIP, m_onClientConnectUserData );
         }
      }
   }


   for ( int i = 0; i < (int)m_sockConnectionsTCP.size(); i++ )
   {
      int tcpDataPending;

      SOCKET s = m_sockConnectionsTCP[ i ];

      fd_set readfds;
      FD_ZERO( &readfds );
      FD_SET( s, &readfds );
      timeval timeout = { 0, 0 };  // return immediately
      int error = select( 0, &readfds, 0, 0, &timeout );   // 1st parameter ignored by winsock
      if ( error == SOCKET_ERROR )
      {
         //printf( "TCP - Error checking status\n" );
         tcpDataPending = 0;
      }
      else if ( error == 0 )
      {
         tcpDataPending = 0;
      }
      else
      {
         tcpDataPending = 1;
      }


      std::string overflowData = "";
      while ( tcpDataPending )
      {
         tcpDataPending = 0;

         char str[ 1000 ];
         memset( str, 0, sizeof( char ) * 1000 );

         int bytesReceived = recv( (SOCKET)s, str, sizeof( str ) - 1, 0 );
         if ( bytesReceived > 0 )
         {
            str[ bytesReceived ] = 0;

            string recvStr = overflowData + str;

            //OutputDebugString( string( recvStr + "\n" ).c_str() );


            int lastFullCommandIndex = -1;
            if (recvStr.size() > 0 && recvStr[recvStr.size() - 1] != ';')
            {
               // retain all the characters up to the last semicolon
               for (int x = recvStr.size() - 1; x >= 0; x--)
               {
                  if (recvStr[x] == ';')
                  {
                     lastFullCommandIndex = x;
                     break;
                  }

                  if (x == 0)
                     lastFullCommandIndex = -1;
               }

               if (lastFullCommandIndex >= 0)
               {
                  overflowData = recvStr.substr(lastFullCommandIndex + 1);
                  recvStr = recvStr.substr(0, lastFullCommandIndex + 1);
                  tcpDataPending = 1;
               }
            }

            vector< string > tokens;
            vhcl::Tokenize( recvStr, tokens, ";" );

            for ( int t = 0; t < (int)tokens.size(); t++ )
            {
               vector< string > msgTokens;
               vhcl::Tokenize( tokens[ t ], msgTokens, "|" );

               if ( msgTokens.size() > 0 )
               {
                  if ( msgTokens[ 0 ] == "CreateActor" )
                  {
                     if ( msgTokens.size() > 4 )
                     {
                        string charIdStr = msgTokens[ 1 ];
                        int charId       = atoi( charIdStr.c_str() );
                        string uClass    = msgTokens[ 2 ];
                        string name      = msgTokens[ 3 ];
                        int skeletonType = atoi( msgTokens[ 4 ].c_str() );

                        //if ( m_onCreateCharacterFunc )
                        {
                           //m_onCreateCharacterFunc( charId, uClass, name, skeletonType, m_onClientConnectUserData );
                        }
                     }
                  }
               }
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
