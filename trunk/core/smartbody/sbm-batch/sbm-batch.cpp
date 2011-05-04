/*
 *  sbm-batch.cpp - part of SBM: SmartBody Module
 *  Copyright (C) 2008  University of Southern California
 *
 *  SBM is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SBM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SBM.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcus Thiebaux, USC
 */


#include "vhcl.h"

#include <cstdio>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <set>
#include <iostream>

#include "smartbody-dll.h"
#include "vhmsg-tt.h"


using std::string;
using std::vector;


Smartbody_dll * sbm = NULL;
vector< string > characters;


class SBMListener : public SmartbodyListener
{
   public:
      virtual void OnCharacterCreate( const string & name )
      {
         printf( "Character Create!\n" );

         characters.push_back( name );
      }

      virtual void OnCharacterDelete( const string & name )
      {
         printf( "Character Delete!\n" );

         for ( uint32_t i = 0; i < characters.size(); i++ )
         {
            if ( characters[ i ].compare( name ) == 0 )
            {
               characters.erase( characters.begin() + i );
               break;
            }
         }
      }
};


void tt_client_callback( const char * op, const char * args, void * user_data )
{
   printf( "received - '%s %s'\n", op, args );

   sbm->ProcessVHMsgs( op, args );
}


double get_time()
{
   return( (double)timeGetTime() / 1000.0 );
}


int main( int argc, char ** argv )
{
   printf( "Starting VHMsg\n" );

   vhmsg::ttu_set_client_callback( tt_client_callback );
   vhmsg::ttu_open();

   // sbm related vhmsgs
   vhmsg::ttu_register( "sbm" );
   vhmsg::ttu_register( "vrAgentBML" );
   vhmsg::ttu_register( "vrSpeak" );
   vhmsg::ttu_register( "RemoteSpeechReply" );
   vhmsg::ttu_register( "PlaySound" );
   vhmsg::ttu_register( "StopSound" );
   vhmsg::ttu_register( "CommAPI" );
   vhmsg::ttu_register( "object-data" );
   vhmsg::ttu_register( "wsp" );

   vhmsg::ttu_report_version( "sbm-batch", "all", "all" );


   printf( "Starting SBM\n" );

   SBMListener listener;
   sbm = new Smartbody_dll;

   sbm->Init();
   sbm->SetListener( &listener );


   printf( "Starting main loop, hit 'q' to quit\n" );

   bool loop = true;
   while ( loop )
   {

	   static bool once = false;
	   if (!once)
	   {
		   for (int i = 1; i < argc; i++)
		   {
			   sbm->ProcessVHMsgs("sbm", argv[i]);
		   }
		   once = true;
	   }  

      sbm->Update( get_time() );
      vhmsg::ttu_poll();


      for ( uint32_t i = 0; i < characters.size(); i++ )
      {
         SmartbodyCharacter c = sbm->GetCharacter( characters[ i ] );

         printf( "Character %s: %5.2f %5.2f %5.2f\n", c.m_name.c_str(), c.x, c.y, c.z );
      }

      if ( _kbhit() && _getch() == 'q' )
         loop = false;
   }


   vhmsg::ttu_close();
   sbm->Shutdown();
   delete sbm;
}

/*
// Batch processing using experimental DLL interface
int main( int argc, char ** argv )
{
	printf( "Starting VHMsg\n" );

	vhmsg::ttu_set_client_callback( tt_client_callback );
	vhmsg::ttu_open();

	// sbm related vhmsgs
	vhmsg::ttu_register( "sbm" );
	vhmsg::ttu_register( "vrAgentBML" );
	vhmsg::ttu_register( "vrSpeak" );
	vhmsg::ttu_register( "RemoteSpeechReply" );
	vhmsg::ttu_register( "PlaySound" );
	vhmsg::ttu_register( "StopSound" );
	vhmsg::ttu_register( "CommAPI" );
	vhmsg::ttu_register( "object-data" );
	vhmsg::ttu_register( "wsp" );

	vhmsg::ttu_report_version( "sbm-batch", "all", "all" );

	printf( "Starting SBM\n" );

	Init();
	SetFacebone();

	printf( "Starting main loop, hit 'q' to quit\n" );

	std::set<std::string> characters;

	bool loop = true;
	while ( loop )
	{
		Update( get_time() );
		vhmsg::ttu_poll();


		std::string characterName;
		std::string objectClass;
		bool hasNew =  HasCharacterCreated(characterName, objectClass);
		if (hasNew)
		{
			std::cout << "Character created with name: " << characterName << std::endl;
			characters.insert(characterName);
		}

		bool hasDeleted = HasCharacterDeleted(characterName);
		if (hasDeleted)
		{
			std::cout << "Character deleted with name: " << characterName << std::endl;
			characters.erase(characterName);
		}


		for (std::set<std::string>::iterator iter = characters.begin();
			iter != characters.end();
			iter++)
		{
			// get the character information
			float x;
			float y;
			float z;
			float rw;
			float rx;
			float ry;
			float rz;
			GetCharacterInfo((*iter), x, y, z, rw, rx, ry, rz);
			std::cout << "Character : " << (*iter) << " " << x << " " << y << " " << z << std::endl;

			// get the joint information
			int numJoints = GetNumJoints((*iter));
			std::string jointName;
			for (int j = 0; j < numJoints; j++)
			{
				GetCharacterJointInfo((*iter), j, jointName, x, y, z, rw, rx, ry, rz);
			}
		}
		
		if ( _kbhit() && _getch() == 'q' )
			loop = false;
	}

	vhmsg::ttu_close();
	Shutdown();
}
*/

