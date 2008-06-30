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
*/

#include "vhcl.h"

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <winsock.h>

#include <string>
#include <vector>

#include "tt_utils.h"
#include "cepstral_tts.h"


cepstral_tts * tts = NULL;


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
   std::string voice = tokens.at( 3 );
   std::string file_name = tokens.at( 4 );
   size_t prefix_length = message_c.find( file_name, 0 ) + file_name.length() + 1;
   std::string utterance = message_c.substr( prefix_length );  // strip off the prefix, only using the xml

   // remove anything after </speech> tag
   size_t postfix_pos = utterance.rfind( "</speech>" );
   if ( postfix_pos != std::string::npos )
      utterance = utterance.substr( 0, postfix_pos + 9 );

   std::string xml = tts->tts( utterance.c_str() );

   std::string reply = agent_name;
   reply += " ";
   reply += message_id;
   reply += " OK: <?xml version=\"1.0\" encoding=\"UTF-8\"?>";
   reply += xml;

   printf( "REPLY: %s\n", reply.c_str() );

   ttu_notify2( "RemoteSpeechReply", reply.c_str() );
}


void elvin_callback( char * op, char * args, void * userData )
{
   printf( "received -- op: %s args: %s\n", op, args );

   if ( strcmp( op, "RemoteSpeechCmd" ) == 0 )
   {
      process_message( args );
   }
}


int main( int argc, char * argv[] )
{
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
      printf( "unable to connect to message server, aborting\n" );
      return -1;
   }

   ttu_register( "RemoteSpeechCmd" );

   tts = new cepstral_tts();

   tts->init();

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
