/*
   This file is part of VHMsg written by Edward Fast at 
   University of Southern California's Institute for Creative Technologies.
   http://www.ict.usc.edu
   Copyright 2008 Edward Fast, University of Southern California

   VHMsg is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   VHMsg is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with VHMsg.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vhcl.h"

#include <string>
#include <vector>

#if WIN_BUILD
#include <WinSock2.h>
#endif

using std::string;
using std::vector;


namespace vhcl
{


//http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

bool g_wsaStartupCalled = false;

string SocketGetHostname()
{
#if WIN_BUILD
   char * hostname = new char [256];
   /*int ret = */ gethostname(hostname, 256);  // 256 is guaranteed to be long enough  http://msdn.microsoft.com/en-us/library/windows/desktop/ms738527(v=vs.85).aspx
   string hostnameStr = hostname;
   delete [] hostname;
   return hostnameStr;
#else
   return "";
#endif
}

bool SocketStartup()
{
#if WIN_BUILD
   WSADATA wsaData;
   int err = WSAStartup( MAKEWORD(2,2), &wsaData );
   if ( err != 0 )
   {
      printf( "WSAStartup failed. Code: %d\n", err );
      return false;
   }

   g_wsaStartupCalled = true;
   return true;
#else
   return true;
#endif
}

bool SocketShutdown()
{
#if WIN_BUILD
   if ( g_wsaStartupCalled )
   {
      WSACleanup();
      g_wsaStartupCalled = false;
   }
   return true;
#else
   return true;
#endif
}

void * SocketOpenTcp()
{
#if WIN_BUILD
   SOCKET sockTCP = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if ( sockTCP == INVALID_SOCKET )
   {
      printf( "Couldn't create socket tcp.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      return NULL;
   }

   return (void *)sockTCP;
#else
   return NULL;
#endif
}

void * SocketAccept(void * socket)
{
#if WIN_BUILD
   SOCKET sockTCP;
   sockaddr_in newToAddr;

   int i = sizeof( sockaddr_in );
   sockTCP = accept( (SOCKET)socket, (sockaddr *)&newToAddr, &i );

   //printf( "New Connection!\n" );
   //string clientIP = inet_ntoa( newToAddr.sin_addr );

   return (void *)sockTCP;  // TODO - check for errors
#else
   return NULL;
#endif
}

void SocketClose(void * socket)
{
#if WIN_BUILD
   closesocket((SOCKET)socket);
#else
#endif
}

bool SocketSetReuseAddress(void * socket, bool reuse)
{
#if WIN_BUILD
   int reuseAddr = 1;
   setsockopt((SOCKET)socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(int));
   return true;  // TODO - check return code
#else
   return true;
#endif
}

bool SocketBind(void * socket, int port)
{
#if WIN_BUILD
   sockaddr_in m_addrTCP;
   m_addrTCP.sin_family      = AF_INET;
   m_addrTCP.sin_addr.s_addr = INADDR_ANY;
   m_addrTCP.sin_port        = htons( (uint16_t)port );
   memset(m_addrTCP.sin_zero, 0, sizeof(m_addrTCP.sin_zero));

   if (bind((SOCKET)socket, (sockaddr *)&m_addrTCP, sizeof(m_addrTCP)) == SOCKET_ERROR)
   {
      printf("bind() failed.\n");
      int errnum = WSAGetLastError();
      printf("socket error: %d\n", errnum);
      return false;
   }

   return true;
#else
   return true;
#endif
}

bool SocketConnect(void * socket, const std::string & server, int port)
{
#if WIN_BUILD
   sockaddr_in toAddrTCP;

   // see if we're specifying a host by name or by number
   if (isalpha(server[0]))
   {
      hostent * host = gethostbyname(server.c_str());
      if (host == NULL)
      {
         printf( "gethostbyname() failed.\n" );
         int errnum = WSAGetLastError();
         printf( "socket error: %d\n", errnum );
         return false;
      }

      toAddrTCP.sin_family = AF_INET;
      toAddrTCP.sin_addr = *((in_addr *)host->h_addr);
      toAddrTCP.sin_port = htons((uint16_t)port);
   }
   else
   {
      toAddrTCP.sin_family = AF_INET;
      toAddrTCP.sin_addr.s_addr = inet_addr(server.c_str());
      toAddrTCP.sin_port = htons((uint16_t)port);
   }


   int ret;
   ret = connect((SOCKET)socket, (SOCKADDR*)&toAddrTCP, sizeof(toAddrTCP));
   if (ret < 0)
   {
      printf( "connect() failed.\n" );
      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );
      return false;
   }

   return true;
#else
   return true;
#endif
}

bool SocketSetBlocking(void * socket, bool blocking)
{
#if WIN_BUILD
   u_long nonBlocking = blocking ? 0 : 1;
   ioctlsocket((SOCKET)socket, FIONBIO, &nonBlocking);
   return true;  // TODO - check return code
#else
   return true;
#endif
}

bool SocketListen(void * socket, int numBackLog)
{
#if WIN_BUILD
   listen((SOCKET)socket, numBackLog);
   return true;  // TODO - check return code
#else
   return true;
#endif
}

bool SocketIsDataPending(void * socket)
{
#if WIN_BUILD
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
#else
   return false;
#endif
}

bool SocketSend(void * socket, const string & msg)
{
#if WIN_BUILD
   int bytesSent = send((SOCKET)socket, msg.c_str(), msg.length(), 0);
   if (bytesSent < 0)
   {
      /*int errnum =*/ WSAGetLastError();
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
#else
   return true;
#endif
}

int SocketReceive(void * socket, char * buffer, int bufferSize)
{
#if WIN_BUILD
   int bytesReceived = recv((SOCKET)socket, buffer, bufferSize - 1, 0);
   if (bytesReceived > 0)
   {
      buffer[bytesReceived] = 0;  // null terminate the buffer, since recv() doesn't
   }

   return bytesReceived;
#else
   return 0;
#endif
}


};
