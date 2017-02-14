/*************************************************************
Copyright (C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/

// sbm_audio.cpp

#include "vhcl.h"

#include "sbm_audio.h"

#include "vhcl_audio.h"
#include <sstream>
#include <cstdlib>


vhcl::Audio * g_audio = NULL;


bool AUDIO_Init()
{
	LOG("Initializing AUDIO...");
	if (g_audio)
	{
		LOG("Could not initialize AUDIO...");
		return false;
	}

	g_audio = new vhcl::Audio();
	bool ret = g_audio->Open();
	if (!ret)
		LOG("Could not perform AUDIO_Init()");
	return ret;
}


void AUDIO_Play( const char * audio_file )
{
	
	vhcl::Sound * sound = g_audio->CreateSoundLibSndFile( audio_file, audio_file );
	if ( sound )
	{
		sound->Play();
	}
	else
	{
		LOG("no sound available from %s", audio_file);
	}

}

void AUDIO_Stop( const char * audio_file )
{
	if (g_audio)
		g_audio->DestroySound(audio_file);
}


void AUDIO_Close()
{
	if (!g_audio)
		return;
	g_audio->Close();
	delete g_audio;
	g_audio = NULL;
}
