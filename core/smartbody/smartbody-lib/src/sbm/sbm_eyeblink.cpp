/*
 *  sbm_eyeblink.cpp - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Ed Fast, USC
 *      Andrew n marshall, USC
 */

// sbm_eyeblink.cxx

#include <stdio.h>
#include <stdlib.h>

#include "sbm_eyeblink.h"


int auto_eyeblink_on = 1;


void SBM_eyeblink_update( BoneBusCharacter * character, double frame_time )
{
   // EDF - Only send these messages on the server machine.  The rest of the clients will respond to these messages
   //       This block of code should be done either in Unreal Script, or a stand-alone module, or incorporated into the agent code.
   if ( auto_eyeblink_on )
   {
      if ( character == NULL )
      {
         return;
      }

      // automatic blinking routine
      static const double blink_down_time = 0.04;       // how long the eyes should stay closed. ~1 frame
      static const double min_blink_repeat_time = 4.0;  // how long to wait until the next blink
      static const double max_blink_repeat_time = 8.0;  // will pick a random number between these min/max
      
      static double last_blink = frame_time;
      static double blink_repeat_time = min_blink_repeat_time;
      static bool eye_closed = false;

      if ( !eye_closed )
      {
         if ( frame_time - last_blink > blink_repeat_time )
         {
            // close the eyes
            // EDF - vhmsg is too slow to work as separate messages, sigh.
            //ttu_notify2( "dimr", "agent doctor viseme blink_rt 1" );
            //ttu_notify2( "dimr", "agent doctor viseme blink_lf 1" );
            //SendWinsockSetViseme( "blink", .9f );
            //NetworkSetViseme( handle, "blink", 0.9f, 0.001f );
            character->SetViseme( "blink", 0.9f, 0.001f );

            last_blink = frame_time;
            eye_closed = true;
         }
      }
      else
      {
         if ( frame_time - last_blink > blink_down_time )
         {
            // open the eyes
            // EDF - vhmsg is too slow to work as separate messages, sigh.
            //ttu_notify2( "dimr", "agent doctor viseme blink_rt 0" );
            //ttu_notify2( "dimr", "agent doctor viseme blink_lf 0" );
            //SendWinsockSetViseme( "blink", 0 );
            //NetworkSetViseme( handle, "blink", 0, 0.001f );
            character->SetViseme( "blink", 0, 0.001f );

            last_blink = frame_time;
            eye_closed = false;

            // compute when to close them again
            double fraction = (double)rand() / (double)RAND_MAX;
            blink_repeat_time = ( fraction * ( max_blink_repeat_time - min_blink_repeat_time ) ) + min_blink_repeat_time;
         }
      }
   }
}


void SBM_enable_auto_eyeblink()
{
   // EDF - NOTE!  This is a global setting, not per character. Will have to be modified to support multiple chars.

   auto_eyeblink_on = 1;
}


void SBM_disable_auto_eyeblink( mcuCBHandle * mcu )
{
   // EDF - NOTE!  This is a global setting, not per character. Will have to be modified to support multiple chars.

   auto_eyeblink_on = 0;

   SbmCharacter * char_p;
   mcu->character_map.reset();
   while ( char_p = mcu->character_map.next() )
   {
      // EDF - vhmsg is too slow to work as separate messages, sigh.
      //ttu_notify2( "dimr", "agent doctor viseme blink_rt 0" );
      //ttu_notify2( "dimr", "agent doctor viseme blink_lf 0" );
      //ttu_notify2( "sbm", "char doctor viseme blink 0" );
      //SendWinsockSetViseme( "blink", 0 );
      //NetworkSetViseme( char_p->net_handle, "blink", 0, 0.001f );
      char_p->bonebusCharacter->SetViseme( "blink", 0, 0.001f );
   }
}
