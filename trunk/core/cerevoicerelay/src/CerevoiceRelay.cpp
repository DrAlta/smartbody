/*
   Part of SBM: SmartBody Module
   Copyright (C) 2008  University of Southern California

   SBM is free software: you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public License
   as published by the Free Software Foundation, version 3 of the
   license.

   SBM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   Lesser GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public
   License along with SBM.  If not, see:
       http://www.gnu.org/licenses/lgpl-3.0.txt

   CONTRIBUTORS:
      Edward Fast, USC
      Thomas Amundsen, USC
      Arno Hartholt, USC
*/

#include "vhcl.h"

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <winsock.h>

#include <string>
#include <vector>
#include <iostream>

#include "tt_utils.h"
#include "cerevoice_tts.h"


const int NETWORK_PORT_TCP = 1314;
const char * host_name;
cerevoice_tts * tts;
SOCKET sock_tcp = NULL;
sockaddr_in toAddr_tcp;
int toSize_tcp = sizeof(toAddr_tcp);
bool bServerMode;


int InitWinsock( const char * network_host )
{
   WSADATA WSAData;
   INT Code = WSAStartup( MAKEWORD(2,2), &WSAData );

   if ( Code != 0 )
   {
      return -1;
   }

   sock_tcp = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

   if ( sock_tcp == INVALID_SOCKET )
   {
      printf( "Couldn't create socket2.\n" );

      int errnum = WSAGetLastError();
      printf( "socket error: %d\n", errnum );

      sock_tcp = NULL;
      WSACleanup();

      return -1;
   }

   // see if we're specifying a host by name or by number
   if ( isalpha( network_host[ 0 ] ) )
   {
      hostent * host = gethostbyname( network_host );

      if ( host == NULL )
      {
         printf( "gethostbyname() failed.\n" );

         int errnum = WSAGetLastError();
         printf( "socket error: %d\n", errnum );

         closesocket( sock_tcp );
         sock_tcp = NULL;
         WSACleanup();

         return -1;
      }

      toAddr_tcp.sin_family = AF_INET;
      toAddr_tcp.sin_addr = *( ( in_addr* )host->h_addr );
      toAddr_tcp.sin_port = htons( NETWORK_PORT_TCP );
   }
   else
   {
      toAddr_tcp.sin_family = AF_INET;
      toAddr_tcp.sin_addr.s_addr = inet_addr( network_host );
      toAddr_tcp.sin_port = htons( NETWORK_PORT_TCP );
   }

   {
      int ret;
      ret = connect( sock_tcp, (SOCKADDR*)&toAddr_tcp, toSize_tcp );

      if ( ret < 0 )
      {
         printf( "connect() failed.\n" );

         int errnum = WSAGetLastError();
         printf( "socket error: %d\n", errnum );

         closesocket( sock_tcp );
         sock_tcp = NULL;
         WSACleanup();

         return -1;
      }
   }

   return 0;
}


void CloseWinsock()
{
   if ( sock_tcp )
   {
      closesocket( sock_tcp );
      sock_tcp = NULL;
   }

   WSACleanup();
}


void process_message( const char * message )
{
   std::string message_c = message;

   // parse the string
   std::vector< std::string > tokens;
   const std::string delimiters = " ";
   vhcl::Tokenize( message_c, tokens, delimiters );

   std::string command = tokens.at( 0 );

   std::string agent_name = tokens.at( 1 );
   std::string message_id = tokens.at( 2 );
   std::string voice_id = tokens.at( 3 );
   std::string file_name = tokens.at( 4 );
   size_t prefix_length = message_c.find( file_name, 0 ) + file_name.length() + 1;
   std::string utterance = message_c.substr( prefix_length );  // strip off the prefix, only using the xml

   // remove anything after </speech> tag
   size_t postfix_pos = utterance.rfind( "</speech>" );
   if ( postfix_pos != std::string::npos )
      utterance = utterance.substr( 0, postfix_pos + 9 );

   //parse out just the sound file name and give it a .wav file type
   int pos = file_name.find( ".aiff" );
   int pos2 = file_name.find( "utt" );
   file_name = file_name.substr( pos2, pos - pos2 ) + ".wav";

   // Create file name relative to cerevoice relay
   std::string cereproc_file_name = tts->temp_audio_dir_cereproc + file_name;
   std::string player_file_name = tts->temp_audio_dir_player + file_name;

   std::string xml = tts->tts( utterance.c_str(), cereproc_file_name.c_str(), player_file_name.c_str(), voice_id );

   // Only send out a reply when result is not empty, ignore otherwise as a nother voice relay might pick up the request
   if ( xml.compare("") != 0 )
   {
      std::string reply = agent_name;
      reply += " ";
      reply += message_id;
      reply += " OK: <?xml version=\"1.0\" encoding=\"UTF-8\"?>";
      reply += xml;
   
      printf( "REPLY: %s\n", reply.c_str() );

      ttu_notify2( "RemoteSpeechReply", reply.c_str() );
   }
}


std::string remove_spaces_and_double_quotes ( char * c )
{
   std::string s = c;

   while(s.find(" ") != std::string::npos)
   {
      s.replace(s.find(" "), 1, "");
   }
  
   while(s.find("\"") != std::string::npos)
   {
      s.replace(s.find("\""), 1, "");
   }

   return s;
}


void elvin_callback( char * op, char * args, void * userData )
{
   printf( "received -- op: %s args: %s\n", op, args );

   if ( strcmp( op, "RemoteSpeechCmd" ) == 0 )
   {
      if ( bServerMode )
      {
         std::string query = args;

         if ( InitWinsock( host_name ) != 0 )
         {
            fprintf( stderr, "\n InitWinsock Failed!\n" );
         }

         int bytesSent = send( sock_tcp, query.c_str(), (int)query.size(), 0 );
         //printf( "SENT %d bytes\n", bytesSent );
         if ( bytesSent < 0 )
         {
            int errnum = WSAGetLastError();
            fprintf( stderr, "socket error: %d\n", errnum );
         }

         char recv_b[ 4096 ];
         int bytesReceived = recv( sock_tcp, recv_b, 4096, 0 );

         //close TCP connection
         CloseWinsock();

         if ( bytesReceived == 0 )
         {
            printf( "Connection closed\n" );
         }
         else if ( bytesReceived < 0 )
         {
            fprintf( stderr, "recv failed: %d\n", WSAGetLastError() );
         }
         else
         {
            recv_b[ bytesReceived ] = '\0';
            //printf("Bytes received: %d\n%s\n", bytesReceived, recv_b);

            //initializing vector value
            char * hit_actor;
            hit_actor = strtok( recv_b, "," );
            hit_actor = strtok( NULL, "," );
         }
      }
	  else
      {
         process_message( args );
      }
   }
   else if ( strcmp( op, "vrAllCall" ) == 0 )
   {
      ttu_notify2( "vrComponent", "rvoice cerevoicerelay" );
   }
   else if ( strcmp( op, "vrKillComponent" ) == 0 )
   {
      std::string strArgs = remove_spaces_and_double_quotes(args);
      
      if ( _stricmp( strArgs.c_str(), "rvoice" ) == 0 || _stricmp( strArgs.c_str(), "all" ) == 0 )
      {
         printf( "Kill message received." );
         ttu_notify2( "vrProcEnd", "rvoice cerevoicerelay" );
         exit(0);
	  }
   }
}


void register_messages ()
{
	ttu_register( "RemoteSpeechCmd" );
	ttu_register( "vrKillComponent" );
   ttu_register( "vrAllCall" );
}


int main( int argc, char * argv[] )
{
   std::vector<char *> voices;
   bServerMode = true;
    
   // Messaging set up
   char * elvish_session_host = getenv( "elvish_session_host" );
   char * elvish_scope = getenv( "elvish_scope" );

   if ( elvish_session_host == NULL )
   {
      elvish_session_host = getenv( "COMPUTERNAME" );
   }

   if ( elvish_scope == NULL )
   {
      elvish_scope = strcat( getenv( "COMPUTERNAME" ), "_SCOPE" );
   }

   ttu_set_client_callback( elvin_callback );
   int err = ttu_open( elvish_session_host );
   if ( err != TTU_SUCCESS )
   {
      printf( "Unable to connect to message server.\n\nPress any key to exit.\n");
      _getch();
      exit(1);
   }

   register_messages();

   // Parse parameters
   for (int i = 1; i < argc; i += 2)
   {
      if ( argc <= i+1)
      {
         printf( "Missing argument for parameter %s.\n\nPress any key to exit.\n", argv[ i ] );
         _getch();
         exit(1);
      } 
      else if ( strcmp( argv[ i ], "-host" ) == 0 )
      {
         bServerMode = true;
         host_name = argv[ i+1 ];
         printf( "Host parameter found. The server / client mode has currently not been implemented. " 
            "Please restart without host parameter to run as stand-alone application.\n\n" 
            "Press any key to exit.\n");
         _getch();
         exit(1);
      } 
      else if ( strcmp( argv[ i ], "-voice" ) == 0 )
      {
         printf( "Voice paramater: %s\n", argv[ i+1 ] );
         bServerMode = false;     
         voices.push_back( argv[ i+1 ] );
      } 
      else 
      {
         printf( "Unknown argument '%s'. \n\n"
            "Known arguments:\n"
            "-host <server_name>, for running CereVoiceRelay in client mode,\n   connecting to <server_name>\n"
            "-voice <voice_id>, for loading voice <voice_id>;\n   multiple '-voice <voice_id>' pairs can be given.\n\n"
            "Press any key to exit.\n", argv[ i ] );
         _getch();
         exit(1);
      }
   }

   // Check if we have any voices defined
   if ( voices.size() < 1 )
   {
      printf( "No voices have been defined. Please specify one or more voices \n"
         "using one or more '-voice <voice_id>' parameters.\n\nPress any key to exit.\n" );
      _getch();
      exit(1);
   }

   // Initialize cerevoice if running as stand-alone application
   if ( !bServerMode )
   {
      tts = new cerevoice_tts();
      tts->init( voices );

      // Notify that we're online
      ttu_notify2( "vrComponent", "rvoice cerevoicerelay" );
   }

   // Main loop
   for ( ;; )
   {
      if ( _kbhit() )
      {
         int c = _getch();
         if ( c == 'q' )
         {
            break;
         }
      }

      ttu_poll();
      Sleep( 10 );
   }
}

