
#include "vhcl.h"

#include "SbmDebuggerCommon.h"

#include <string>
#include <vector>

#include <winsock.h>
#include <stdio.h>
#include <conio.h>

#include "vhmsg-tt.h"

using std::string;
using std::vector;

#define LERP(a, b, t) (a + (b - a) * t)
//#define TOLERANCE 0.00001f

//3.14159265358979323846





//http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

bool g_wsaStartupCalled = false;

string SocketGetHostname()
{
   char * hostname = new char [256];
   int ret = gethostname(hostname, 256);  // 256 is guaranteed to be long enough  http://msdn.microsoft.com/en-us/library/windows/desktop/ms738527(v=vs.85).aspx
   string hostnameStr = hostname;
   delete [] hostname;
   return hostnameStr;
}

bool SocketStartup()
{
   WSADATA wsaData;
   int err = WSAStartup( MAKEWORD(2,2), &wsaData );
   if ( err != 0 )
   {
      printf( "WSAStartup failed. Code: %d\n", err );
      return false;
   }

   g_wsaStartupCalled = true;
   return true;
}

bool SocketShutdown()
{
   if ( g_wsaStartupCalled )
   {
      WSACleanup();
      g_wsaStartupCalled = false;
   }
   return true;
}

void * SocketOpenTcp()
{
   SOCKET sockTCP = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if ( sockTCP == INVALID_SOCKET )
   {
      printf( "Couldn't create socket tcp.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      return NULL;
   }

   return (void *)sockTCP;
}

void * SocketAccept(void * socket)
{
   SOCKET sockTCP;
   sockaddr_in newToAddr;

   int i = sizeof( sockaddr_in );
   sockTCP = accept( (SOCKET)socket, (sockaddr *)&newToAddr, &i );

   //printf( "New Connection!\n" );
   //string clientIP = inet_ntoa( newToAddr.sin_addr );

   return (void *)sockTCP;  // TODO - check for errors
}

void SocketClose(void * socket)
{
   closesocket((SOCKET)socket);
}

bool SocketSetReuseAddress(void * socket, bool reuse)
{
   int reuseAddr = 1;
   setsockopt((SOCKET)socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(int));
   return true;  // TODO - check return code
}

bool SocketBind(void * socket, int port)
{
   sockaddr_in m_addrTCP;
   m_addrTCP.sin_family      = AF_INET;
   m_addrTCP.sin_addr.s_addr = INADDR_ANY;
   m_addrTCP.sin_port        = htons( port );
   memset(m_addrTCP.sin_zero, 0, sizeof(m_addrTCP.sin_zero));

   if (bind((SOCKET)socket, (sockaddr *)&m_addrTCP, sizeof(m_addrTCP)) == SOCKET_ERROR)
   {
      printf("bind() failed.\n");
      int errnum = WSAGetLastError();
      printf("socket error: %d\n", errnum);
      return false;
   }

   return true;
}

bool SocketSetBlocking(void * socket, bool blocking)
{
   u_long nonBlocking = blocking ? 0 : 1;
   ioctlsocket((SOCKET)socket, FIONBIO, &nonBlocking);
   return true;  // TODO - check return code
}

bool SocketListen(void * socket, int numBackLog)
{
   listen((SOCKET)socket, numBackLog);
   return true;  // TODO - check return code
}

bool SocketIsDataPending(void * socket)
{
   fd_set readfds;
   FD_ZERO(&readfds);
   FD_SET((SOCKET)socket, &readfds);
   timeval timeout = { 0, 0 };  // return immediately
   int error = select(0, &readfds, 0, 0, &timeout);   // 1st parameter ignored by winsock
   if ( error == SOCKET_ERROR )
   {
      return false;
   }
   else if ( error == 0 )
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool SocketSend(void * socket, const string & msg)
{
   int bytesSent = send((SOCKET)socket, msg.c_str(), msg.length(), 0);
   if (bytesSent < 0)
   {
      int errnum = WSAGetLastError();
      //printf( "socket error: %d\n", errnum );
      //fprintf( fp, "socket error: %d\n", errnum );

      return false;
   }
   else if ( bytesSent > 0 )
   {
      //printf( "send: %ld\n", bytesSent );
      //fprintf( fp, "send: %ld\n", bytesSent );
   }

   return true;
}

int SocketReceive(void * socket, char * buffer, int bufferSize)
{
   int bytesReceived = recv((SOCKET)socket, buffer, bufferSize - 1, 0);
   if (bytesReceived > 0)
   {
      buffer[bytesReceived] = 0;  // null terminate the buffer, since recv() doesn't
   }

   return bytesReceived;
}


Joint::Joint()
{
   posOrig.x = 0;
   posOrig.y = 0;
   posOrig.z = 0;
   rotOrig.x = 0;
   rotOrig.y = 0;
   rotOrig.z = 0;
   rotOrig.w = 0;

   pos.x = 0;
   pos.y = 0;
   pos.z = 0;
   rot.x = 0;
   rot.y = 0;
   rot.z = 0;
   rot.w = 0;

   m_parent = NULL;
}


Joint::~Joint()
{
}

Vector4::Vector4(double _x, double _y, double _z, double _w)
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
}

void Vector4::ToAxisAngle(Vector3& axis, float& angle)
{
   float scale = (float)sqrt(x * x + y * y + z * z);
   if (scale != 0)
   {
      axis.x = x / scale;
	   axis.y = y / scale;
	   axis.z = z / scale;
   }
	
	angle = ((float)acos(w) * 2.0f) * RAD_TO_DEG;
}


Vector3 Joint::GetWorldPosition() const
{
   Vector3 worldPosition = posOrig + pos;
   Joint* parent = m_parent;
   while (parent)
   {
      worldPosition += (parent->posOrig + parent->pos);
      parent = parent->m_parent;
   }

   return worldPosition;
}

Vector3 Joint::GetLocalPosition()  const
{
   return posOrig + pos;
}

Vector4 Joint::GetWorldRotation()  const
{
   Vector4 worldRotation = rotOrig * rot;
   Joint* parent = m_parent;
   while (parent)
   {
      worldRotation *= (parent->rotOrig * parent->rot);
      parent = parent->m_parent;
   }

   return worldRotation;
}

Vector4 Joint::GetLocalRotation()  const
{
   return rotOrig * rot;
}

std::string Joint::GetPositionAsString(bool worldPos)  const
{
   Vector3 pos = worldPos ? GetWorldPosition() : GetLocalPosition();
   return vhcl::Format("x: %.2f y: %.2f z: %.2f", pos.x, pos.y, pos.z);
}

std::string Joint::GetRotationAsString(bool worldRot)  const
{
   Vector4 rot = worldRot ? GetWorldRotation() : GetLocalRotation();
   return vhcl::Format("w: %.2f x: %.2f y: %.2f z: %.2f", rot.w, rot.x, rot.y, rot.z);
}

Vector3 Pawn::GetWorldPosition() const
{
   Joint* joint = FindJoint("world_offset");
   return joint ? joint->pos : Vector3();
}

Joint* Pawn::GetWorldOffset() const
{
   return FindJoint("world_offset");
}

Joint * Pawn::FindJoint(const string & name, const vector<Joint *> & joints)
{
   for (size_t i = 0; i < joints.size(); i++)
   {
      Joint * j = joints[i];
      if (j->m_name == name)
      {
         return j;
      }
   }

   for (size_t i = 0; i < joints.size(); i++)
   {
      Joint * j = joints[i];
      Joint * ret = FindJoint(name, j->m_joints);
      if (ret)
      {
         return ret;
      }
   }

   return NULL;
}


Character* Scene::FindCharacter(const std::string & name)
{
   for (unsigned int i = 0; i < m_characters.size(); i++)
   {
      if (name == m_characters[i].m_name)
      {
         return &m_characters[i];
      }
   }

   return NULL;
}

Pawn* Scene::FindPawn(const std::string & name)
{
   for (unsigned int i = 0; i < m_pawns.size(); i++)
   {
      if (name == m_pawns[i].m_name)
      {
         return &m_pawns[i];
      }
   }

   return NULL;
}

Pawn* Scene::FindSbmObject(const std::string & name)
{
   Pawn* object = FindCharacter(name);
   return object ? object : FindPawn(name);
}
