/*
 *  sbm_audio.cpp - part of SmartBody-lib
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
 */

// sbm_audio.cpp

#include <windows.h>

#include <stdio.h>


// EDF - we want to be as unobtrusive into the SBM project as possible.
// so I simply take the defines I use and define them here as opposed to
// bringing in all the SDL headers, adding include paths to the project settings, etc.

// taken from SDL.h
#define SDL_INIT_AUDIO		0x00000010

// taken from SDL_types.h
typedef unsigned short	Uint16;
typedef unsigned int	Uint32;

// taken from SDL_audio.h
#define AUDIO_S16LSB	0x8010	/* Signed 16-bit samples */
#define AUDIO_S16	AUDIO_S16LSB


// define these structs as void since we only use them as parameters
typedef void * Mix_Chunk;
typedef void * SDL_RWops;


// from SDL.h
typedef int (__cdecl * SDL_Init_func_type)( Uint32 flags );
typedef void (__cdecl * SDL_Quit_func_type)();

// from SDL_rwops.h
typedef SDL_RWops * (__cdecl * SDL_RWFromFile_func_type)( const char * file, const char * mode ); 

// from SDL_error.h
typedef char * (__cdecl * SDL_GetError_func_type)();

// fom SDL_mixer.h
typedef int (__cdecl * Mix_OpenAudio_func_type)( int frequency, Uint16 format, int channels, int chunksize ); 
typedef void (__cdecl * Mix_CloseAudio_func_type)();
typedef Mix_Chunk * (__cdecl * Mix_LoadWAV_RW_func_type)( SDL_RWops * src, int freesrc );
typedef int (__cdecl * Mix_PlayChannelTimed_func_type)( int channel, Mix_Chunk * chunk, int loops, int ticks );

#define Mix_LoadWAV_func(file)	Mix_LoadWAV_RW_func(SDL_RWFromFile_func(file, "rb"), 1)
#define Mix_PlayChannel_func(channel,chunk,loops) Mix_PlayChannelTimed_func(channel,chunk,loops,-1)
#define Mix_GetError_func	SDL_GetError_func



SDL_Init_func_type             SDL_Init_func = NULL;
SDL_Quit_func_type             SDL_Quit_func = NULL;
SDL_RWFromFile_func_type       SDL_RWFromFile_func = NULL;
SDL_GetError_func_type         SDL_GetError_func = NULL;
Mix_OpenAudio_func_type        Mix_OpenAudio_func = NULL;
Mix_CloseAudio_func_type       Mix_CloseAudio_func = NULL;
Mix_LoadWAV_RW_func_type       Mix_LoadWAV_RW_func = NULL;
Mix_PlayChannelTimed_func_type Mix_PlayChannelTimed_func = NULL;



HINSTANCE  hSDL;
HINSTANCE  hSDL_mixer;


bool AUDIO_Init()
{
   hSDL = LoadLibrary( "SDL.dll" );
   if ( hSDL == NULL )
   {
      return false;
   }

   hSDL_mixer = LoadLibrary( "SDL_mixer.dll" );
   if ( hSDL_mixer == NULL )
   {
      FreeLibrary( hSDL );
      return false;
   }

   SDL_Init_func = (SDL_Init_func_type)GetProcAddress( hSDL, "SDL_Init" );
   SDL_Quit_func = (SDL_Quit_func_type)GetProcAddress( hSDL, "SDL_Quit" );
   SDL_RWFromFile_func = (SDL_RWFromFile_func_type)GetProcAddress( hSDL, "SDL_RWFromFile" );
   SDL_GetError_func = (SDL_GetError_func_type)GetProcAddress( hSDL, "SDL_GetError" );
   Mix_OpenAudio_func = (Mix_OpenAudio_func_type)GetProcAddress( hSDL_mixer, "Mix_OpenAudio" );
   Mix_CloseAudio_func = (Mix_CloseAudio_func_type)GetProcAddress( hSDL_mixer, "Mix_CloseAudio" );
   Mix_LoadWAV_RW_func = (Mix_LoadWAV_RW_func_type)GetProcAddress( hSDL_mixer, "Mix_LoadWAV_RW" );
   Mix_PlayChannelTimed_func = (Mix_PlayChannelTimed_func_type)GetProcAddress( hSDL_mixer, "Mix_PlayChannelTimed" );

   if ( SDL_Init_func == NULL ||
        SDL_Quit_func == NULL ||
        SDL_RWFromFile_func == NULL ||
        SDL_GetError_func == NULL ||
        Mix_OpenAudio_func == NULL ||
        Mix_CloseAudio_func == NULL ||
        Mix_LoadWAV_RW_func == NULL ||
        Mix_PlayChannelTimed_func == NULL )
   {
      FreeLibrary( hSDL );
      FreeLibrary( hSDL_mixer );
      return false;
   }



   if ( SDL_Init_func( SDL_INIT_AUDIO ) < 0 )
   {
      FreeLibrary( hSDL );
      FreeLibrary( hSDL_mixer );
      return false;
   }

   int audio_rate = 44100;
   Uint16 audio_format = AUDIO_S16; 
   int audio_channels = 1;
   int audio_buffers = 4096;

   if ( Mix_OpenAudio_func( audio_rate, audio_format, audio_channels, audio_buffers ) < 0 )
   {
      FreeLibrary( hSDL );
      FreeLibrary( hSDL_mixer );
      return false;
   }

   return true;
}


void AUDIO_Play( const char * audio_file )
{
   Mix_Chunk * chunk;
   int channel;

   chunk = Mix_LoadWAV_func( audio_file );
   if ( !chunk ) printf( "ERROR: %s '%s'\n", Mix_GetError_func(), audio_file );
   channel = Mix_PlayChannel_func( -1, chunk, 0 );
}


void AUDIO_Close()
{
   Mix_CloseAudio_func();
   SDL_Quit_func();


   FreeLibrary( hSDL_mixer );
   FreeLibrary( hSDL );
}
