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

#ifndef VHCL_SOCKET_H
#define VHCL_SOCKET_H


namespace vhcl
{

std::string SocketGetHostname();
bool SocketStartup();
bool SocketShutdown();
void * SocketOpenTcp();
void * SocketAccept(void * socket);
void SocketClose(void * socket);
bool SocketSetReuseAddress(void * socket, bool reuse);
bool SocketBind(void * socket, int port);
bool SocketConnect(void * socket, const std::string & server, int port);
bool SocketSetBlocking(void * socket, bool blocking);
bool SocketListen(void * socket, int numBackLog = 10);
bool SocketIsDataPending(void * socket);
bool SocketSend(void * socket, const std::string & msg);
int SocketReceive(void * socket, char * buffer, int bufferSize);

};


#endif  // VHCL_SOCKET_H
