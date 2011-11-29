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

#ifndef VHCL_PLATFORM_H
#define VHCL_PLATFORM_H


// Platform identification.
#if defined(_WIN32)

   // Predefined Macros - http://msdn.microsoft.com/en-us/library/b0084kay(v=VS.90).aspx

   // defined for all Windows variants
   #define WIN_BUILD 1

   #if defined(_XBOX)
      #define XBOX_BUILD 1
   #else
      #if defined(_WIN64)
         #define WIN64_BUILD 1
      #else
         #define WIN32_BUILD 1
      #endif

      #if defined(_CONSOLE)
         #define WIN_CONSOLE_BUILD 1
      #elif defined(_USRDLL) // | defined(_DLL)
         #define WIN_DLL_BUILD 1
      #elif defined( NIAPP )
         #define WIN_NI_BUILD 1
      #endif
   #endif
#elif defined(__APPLE__)
   #define MAC_BUILD 1
#elif defined(linux) || defined(__linux)
   #define LINUX_BUILD 1
#else
   #define UNKNOWN_BUILD 1
#endif

#if defined(UNKNOWN_BUILD)
   #error Platform type not detected!
#endif



// Build type
#if defined(_DEBUG)
   #define DEBUG_BUILD 1
#elif defined(NDEBUG)
   #define RELEASE_BUILD 1
#else
   #if defined(LINUX_BUILD) || defined(MAC_BUILD)
      #define RELEASE_BUILD 1
   #else
      #define UNKNOWN_BUILD 1
   #endif
#endif

#if defined(UNKNOWN_BUILD)
   #error Build type not detected!
#endif


#endif  // VHCL_PLATFORM_H
