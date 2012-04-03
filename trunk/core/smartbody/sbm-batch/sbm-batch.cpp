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

#ifdef WIN32
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#else
#define _stdcall 
#endif

#include <set>
#include <iostream>
#include <cstring>

#include "smartbody-dll.h"
#include "smartbody-c-dll.h"
#include "vhmsg-tt.h"


using std::string;
using std::vector;


Smartbody_dll * sbm = NULL;
vector< string > characters;
vector<SBM_SmartbodyCharacter*> instances;


class SBMListener : public SmartbodyListener
{
   public:
      virtual void OnCharacterCreate( const string & name )
      {	     
         printf( "Character Create!\n" );

         characters.push_back( name );
		 
      }

	  virtual void OnCharacterCreate( const string & name, const string & objectClass )
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

	  virtual void OnCharacterChange( const string & name )
      {
         printf( "Character Changed!\n" );

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


 int _stdcall OnCreateCharacterCallback( SBMHANDLE sbmHandle, const char * name, const char * objectClass )
{
	 characters.push_back( name );
	 SBM_SmartbodyCharacter* character = new SBM_SmartbodyCharacter();
	  // copy name
	   character->m_name = new char[ strlen(name) + 1 ];
	   strcpy( character->m_name, name  );
	 instances.push_back(character);
	 return 0;
}

 int _stdcall OnCharacterDeleteCallback( SBMHANDLE sbmHandle, const char * name )
{
	 printf( "Character Delete!\n" );

     for (uint32_t i = 0; i < characters.size(); i++ )
     {
        if ( characters[ i ].compare( name ) == 0 )
        {
           characters.erase( characters.begin() + i );
		   SBM_SmartbodyCharacter* c = instances[i];
		   delete c;
		   instances.erase(instances.begin() + i);
           break;
        }
     }

	 return 0;
}
 int _stdcall OnCharacterChangeCallback( SBMHANDLE sbmHandle, const char * name )
{
	 printf( "Character Delete!\n" );

     for (uint32_t i = 0; i < characters.size(); i++ )
     {
        if ( characters[ i ].compare( name ) == 0 )
        {
			SBM_SmartbodyCharacter* c = instances[i];
			c->m_numJoints = 0;
			delete [] c->m_joints;
           break;
        }
     }

	 return 0;
}

 int _stdcall OnVisemeCallback( SBMHANDLE sbmHandle, const char * name, const char * visemeName, float weight, float blendTime )
{
	return 0;
}

  int _stdcall OnChannelCallback( SBMHANDLE sbmHandle, const char * name, const char * channelName, float value )
{
	return 0;
}

static void tt_client_callback( const char * op, const char * args, void * user_data )
{
   printf( "received - '%s %s'\n", op, args );

   sbm->ProcessVHMsgs( op, args );
}

#ifdef WIN32_LEAN_AND_MEAN

#else
#include <sys/time.h>
#endif

double get_time()
{
#if WIN32
   return( (double)timeGetTime() / 1000.0 );
#else
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return( tv.tv_sec + ( tv.tv_usec / 1000000.0 ) );
#endif
}


int main( int argc, char ** argv )
{
   printf( "Starting VHMsg\n" );

   vhmsg::ttu_set_client_callback( tt_client_callback );
   vhmsg::ttu_open();

   // sbm related vhmsgs
   vhmsg::ttu_register( "sb" );
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
	// register the log listener
	vhcl::Log::StdoutListener* logListener = new vhcl::Log::StdoutListener();
	vhcl::Log::g_log.AddListener(logListener);

   SBMListener listener;
   //sbm = new Smartbody_dll;
   //sbm->Init();
   //sbm->SetListener( &listener );
   
   SBMHANDLE sbmHandle = SBM_CreateSBM();
   SBM_Init(sbmHandle, "../../Python26/Lib", true);
   SBM_SetListener(sbmHandle, 
	   (SBM_OnCreateCharacterCallback) OnCreateCharacterCallback, 
	   (SBM_OnCharacterDeleteCallback) OnCharacterDeleteCallback, 
	   (SBM_OnCharacterChangeCallback) OnCharacterChangeCallback, 
	   (SBM_OnVisemeCallback) OnVisemeCallback,
	   (SBM_OnChannelCallback) OnChannelCallback);

   printf( "Starting main loop...\n");
#if WIN32
	printf("Hit 'q' to quit\n");
#endif

	bool once = false;
   bool loop = true;
   while ( loop )
   {
	   static bool once = false;
	   if (!once)
	   {
		   for (int i = 1; i < argc; i++)
		   {
			   //sbm->ProcessVHMsgs("sbm", argv[i]);
			    SBM_ProcessVHMsgs(sbmHandle, "sbm", argv[i]);
		   }
		   once = true;
	   }  

      //sbm->Update( get_time() );
		SBM_Update( sbmHandle, get_time());
		vhmsg::ttu_poll();


	  //LOG("character size = %d\n",characters.size());
      for ( uint32_t i = 0; i < characters.size(); i++ )
      {
          //SmartbodyCharacter& c = sbm->GetCharacter( characters[ i ] );
		  SBM_GetCharacter(sbmHandle, characters[ i ].c_str(), instances[i] );
		  
         //printf( "Character %s: %5.2f %5.2f %5.2f\n", c.m_name.c_str(), c.x, c.y, c.z );
      }

////////////////////////////////////////////////////  00985ttq08ergijefvonad;kfjbv;sdfjv;owourehcv[ vqiiw[fo jws
#if WIN32
      if ( _kbhit() && _getch() == 'q' )
         loop = false;
#else
	loop = true;
//	loop = false;
#endif
   }


   vhmsg::ttu_close();
   SBM_Shutdown(sbmHandle);
   delete sbm;
}



