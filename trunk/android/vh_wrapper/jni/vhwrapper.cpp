
#include "vhcl.h"

#include "vhwrapper.h"

#include <map>
#include <vector>

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
HINSTANCE g_SBM_HINST;
#endif

#ifdef ENABLE_VHMSG_WRAPPER
#include "vhmsg-tt.h"
#endif


#ifdef ENABLE_VHMSG_WRAPPER
std::map<VHMSGHANDLE, vhmsg::Client*> g_vhmsgInstances;
std::vector<wchar_t*> g_vhmsgQueuedMessages;
int g_vhmsgHandleId = 0;
#endif

#include "vhcl_audio.h"

using vhcl::Audio;
using vhcl::Sound;
bool VHCL_AUDIO_HandleExists( const AUDIOHANDLE handle );
std::map<AUDIOHANDLE, Audio*> g_audioInstances;
int g_audioHandleId = 0;


typedef SBMHANDLE (__stdcall *SBM_CreateSBM_DEF)();
typedef bool (__stdcall *SBM_SetSpeechAudiofileBasePath_DEF)(SBMHANDLE, const char *);
typedef bool (__stdcall *SBM_SetFacebone_DEF)(SBMHANDLE, bool);
typedef bool (__stdcall *SBM_SetProcessId_DEF)( SBMHANDLE, const char * );
typedef bool (__stdcall *SBM_SetMediaPath_DEF)( SBMHANDLE, const char * );
typedef bool (__stdcall *SBM_Init_DEF)( SBMHANDLE, const char * );
typedef bool (__stdcall *SBM_Shutdown_DEF)( SBMHANDLE );
typedef bool (__stdcall *SBM_SetListener_DEF)( SBMHANDLE, SBM_OnCreateCharacterCallback, SBM_OnCharacterDeleteCallback, SBM_OnCharacterChangeCallback, SBM_OnVisemeCallback, SBM_OnChannelCallback);
typedef bool (__stdcall *SBM_Update_DEF)(SBMHANDLE, double);
typedef bool (__stdcall *SBM_ProcessVHMsgs_DEF)(SBMHANDLE, const char*, const char*);
typedef int  (__stdcall *SBM_GetNumberOfCharacters_DEF)(SBMHANDLE sbmHandle);
typedef bool (__stdcall *SBM_GetCharacter_DEF)( SBMHANDLE sbmHandle, const char*, SBM_SmartbodyCharacter* );
typedef bool (__stdcall *SBM_ReleaseCharacter_DEF)(SBM_SmartbodyCharacter *);
typedef bool (__stdcall *SBM_ReleaseCharacterJoints_DEF)(SBM_SmartbodyCharacter *);
typedef bool (__stdcall *SBM_SetLogMessageCallback_DEF)(LogMessageCallback);
typedef void (__stdcall *SBM_LogMessage_DEF)(const char*, int);
typedef bool (__stdcall *SBM_IsCharacterCreated_DEF)( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** objectClass );
typedef bool (__stdcall *SBM_IsCharacterDeleted_DEF)( SBMHANDLE sbmHandle, int * numCharacters, char *** name );
typedef bool (__stdcall *SBM_IsCharacterChanged_DEF)( SBMHANDLE sbmHandle, int * numCharacters, char *** name );
typedef bool (__stdcall *SBM_IsVisemeSet_DEF)( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** visemeName, float** weight, float** blendTime );
typedef bool (__stdcall *SBM_IsChannelSet_DEF)( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** channelName, float ** value );

SBM_CreateSBM_DEF                  g_SBM_CreateSBM_DEF = NULL;
SBM_SetSpeechAudiofileBasePath_DEF g_SBM_SetSpeechAudiofileBasePath_DEF = NULL;
SBM_SetFacebone_DEF                g_SBM_SetFacebone_DEF = NULL;
SBM_SetProcessId_DEF               g_SBM_SetProcessId_DEF = NULL;
SBM_SetMediaPath_DEF               g_SBM_SetMediaPath_DEF = NULL;
SBM_Init_DEF                       g_SBM_Init_DEF = NULL;
SBM_Shutdown_DEF                   g_SBM_Shutdown_DEF = NULL;
SBM_SetListener_DEF                g_SBM_SetListener_DEF = NULL;
SBM_Update_DEF                     g_SBM_Update_DEF = NULL;
SBM_ProcessVHMsgs_DEF              g_SBM_ProcessVHMsgs_DEF = NULL;
SBM_GetNumberOfCharacters_DEF      g_SBM_GetNumberOfCharacters_DEF = NULL;
SBM_GetCharacter_DEF               g_SBM_GetCharacter_DEF = NULL;
SBM_ReleaseCharacter_DEF           g_SBM_ReleaseCharacter_DEF = NULL;
SBM_ReleaseCharacterJoints_DEF     g_SBM_ReleaseCharacterJoints_DEF = NULL;
SBM_SetLogMessageCallback_DEF      g_SBM_SetLogMessageCallback_DEF = NULL;
SBM_LogMessage_DEF                 g_SBM_LogMessage_DEF = NULL;
SBM_IsCharacterCreated_DEF         g_SBM_IsCharacterCreated_DEF = NULL;
SBM_IsCharacterDeleted_DEF           g_SBM_IsCharacterDeleted_DEF = NULL;
SBM_IsCharacterChanged_DEF         g_SBM_IsCharacterChanged_DEF = NULL;
SBM_IsVisemeSet_DEF                g_SBM_IsVisemeSet_DEF = NULL;
SBM_IsChannelSet_DEF               g_SBM_IsChannelSet_DEF = NULL;




VHWRAPPERDLL_API SBMHANDLE WRAPPER_SBM_CreateSBM(const bool releaseMode)
{
#ifdef WIN32
   if (releaseMode)
   {
      g_SBM_HINST = LoadLibrary(TEXT("smartbody-dll.dll")); 
   }
   else
   {
      g_SBM_HINST = LoadLibrary(TEXT("smartbody-dll_d.dll")); 
   }

   if (g_SBM_HINST == NULL)
   {
      WRAPPER_SBM_LogMessage(vhcl::Format("ERROR: Failed to LoadLibrary '%s'", releaseMode ? "smartbody-dll.dll" : "smartbody-dll_d.dll").c_str(), 1);
      return -1;
   }

   g_SBM_CreateSBM_DEF                  = (SBM_CreateSBM_DEF)GetProcAddress(g_SBM_HINST, "SBM_CreateSBM");
   g_SBM_SetSpeechAudiofileBasePath_DEF = (SBM_SetSpeechAudiofileBasePath_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetSpeechAudiofileBasePath");
   g_SBM_SetFacebone_DEF                = (SBM_SetFacebone_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetFacebone");
   g_SBM_SetProcessId_DEF               = (SBM_SetProcessId_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetProcessId");
   g_SBM_SetMediaPath_DEF               = (SBM_SetMediaPath_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetMediaPath");
   g_SBM_Init_DEF                       = (SBM_Init_DEF)GetProcAddress(g_SBM_HINST, "SBM_Init");
   g_SBM_Shutdown_DEF                   = (SBM_Shutdown_DEF)GetProcAddress(g_SBM_HINST, "SBM_Shutdown");
   g_SBM_SetListener_DEF                = (SBM_SetListener_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetListener");
   g_SBM_Update_DEF                     = (SBM_Update_DEF)GetProcAddress(g_SBM_HINST, "SBM_Update");
   g_SBM_ProcessVHMsgs_DEF              = (SBM_ProcessVHMsgs_DEF)GetProcAddress(g_SBM_HINST, "SBM_ProcessVHMsgs");
   g_SBM_GetNumberOfCharacters_DEF      = (SBM_GetNumberOfCharacters_DEF)GetProcAddress(g_SBM_HINST, "SBM_GetNumberOfCharacters");
   g_SBM_GetCharacter_DEF               = (SBM_GetCharacter_DEF)GetProcAddress(g_SBM_HINST, "SBM_GetCharacter");
   g_SBM_ReleaseCharacter_DEF           = (SBM_ReleaseCharacter_DEF)GetProcAddress(g_SBM_HINST, "SBM_ReleaseCharacter");
   g_SBM_ReleaseCharacterJoints_DEF     = (SBM_ReleaseCharacterJoints_DEF)GetProcAddress(g_SBM_HINST, "SBM_ReleaseCharacterJoints");
   g_SBM_SetLogMessageCallback_DEF      = (SBM_SetLogMessageCallback_DEF)GetProcAddress(g_SBM_HINST, "SBM_SetLogMessageCallback");
   g_SBM_LogMessage_DEF                 = (SBM_LogMessage_DEF)GetProcAddress(g_SBM_HINST, "SBM_LogMessage");
   g_SBM_IsCharacterCreated_DEF         = (SBM_IsCharacterCreated_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterCreated");
   g_SBM_IsCharacterDeleted_DEF         = (SBM_IsCharacterDeleted_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterDeleted");
   g_SBM_IsCharacterChanged_DEF         = (SBM_IsCharacterChanged_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterChanged");
   g_SBM_IsVisemeSet_DEF                = (SBM_IsVisemeSet_DEF )GetProcAddress(g_SBM_HINST, "SBM_IsVisemeSet");
   g_SBM_IsChannelSet_DEF               = (SBM_IsChannelSet_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsChannelSet");

   if (g_SBM_CreateSBM_DEF)
   {
      return g_SBM_CreateSBM_DEF();
   }

   return -1;
#else
   return SBM_CreateSBM();
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath )
{
#ifdef WIN32
   if (g_SBM_SetSpeechAudiofileBasePath_DEF)
   {
      return g_SBM_SetSpeechAudiofileBasePath_DEF(sbmHandle, basePath);
   }
   return false;
#else
   return SBM_SetSpeechAudiofileBasePath(sbmHandle, basePath); 
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SetFacebone( SBMHANDLE sbmHandle, bool enabled )
{
#ifdef WIN32
   if (g_SBM_SetFacebone_DEF)
   {
      return g_SBM_SetFacebone_DEF(sbmHandle, enabled);
   }
   return false;
#else
   return SBM_SetFacebone(sbmHandle, enabled);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId )
{
#ifdef WIN32
   if (g_SBM_SetProcessId_DEF)
   {
      return g_SBM_SetProcessId_DEF(sbmHandle, processId);
   }
   return false;
#else
   return SBM_SetProcessId(sbmHandle, processId);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path )
{
#ifdef WIN32
   if (g_SBM_SetMediaPath_DEF)
   {
      return g_SBM_SetMediaPath_DEF(sbmHandle, path);
   }
   return false;
#else
   return SBM_SetMediaPath(sbmHandle, path); 
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_Init( SBMHANDLE sbmHandle,  const char* pythonLib )
{
#ifdef WIN32
   if (g_SBM_Init_DEF)
   {
      return g_SBM_Init_DEF(sbmHandle, pythonLib);
   }
   return false;
#else
   return SBM_Init(sbmHandle, pythonLib);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_Shutdown( SBMHANDLE sbmHandle )
{
#ifdef WIN32
   bool retVal = false;
   if (g_SBM_Shutdown_DEF)
   {
      retVal = g_SBM_Shutdown_DEF(sbmHandle);
   }

   BOOL freeSuccessful = FreeLibrary(g_SBM_HINST);
   g_SBM_HINST = NULL;
   if (!freeSuccessful)
   {
      WRAPPER_SBM_LogMessage("ERROR: Failed to FreeLibrary smartbody-dll.dll", 1);
      return false;
   }
   else
   {
      //WRAPPER_SBM_LogMessage("SUCCESS!: FreeLibrary smartbody-dll.dll", 0);
   }

   return retVal;
#else
   return SBM_Shutdown(sbmHandle);
#endif
}

#ifdef WIN32
VHWRAPPERDLL_API bool WRAPPER_SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB,
                                              SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changeCB,
                                              SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB  )
{
   if (g_SBM_SetListener_DEF)
   {
      return g_SBM_SetListener_DEF(sbmHandle, createCB, deleteCB, changeCB, visemeCB, channelCB);
   }
   return false;
}
#else
VHWRAPPERDLL_API bool WRAPPER_SBM_SetListener( SBMHANDLE sbmHandle)
{
    return SBM_SetListener(sbmHandle, NULL, NULL, NULL, NULL, NULL);
}
#endif

VHWRAPPERDLL_API bool WRAPPER_SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds )
{
#ifdef WIN32
   if (g_SBM_Update_DEF)
   {
      return g_SBM_Update_DEF(sbmHandle, timeInSeconds);
   }
   return false;
#else
   return SBM_Update(sbmHandle, timeInSeconds);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args )
{
#ifdef WIN32
   if (g_SBM_ProcessVHMsgs_DEF)
   {
      return g_SBM_ProcessVHMsgs_DEF(sbmHandle, op, args);
   }
   return false;
#else
   return SBM_ProcessVHMsgs(sbmHandle, op, args);
#endif
}

VHWRAPPERDLL_API int WRAPPER_SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle )
{
#ifdef WIN32
   if (g_SBM_GetNumberOfCharacters_DEF)
   {
      return g_SBM_GetNumberOfCharacters_DEF(sbmHandle);
   }
   return -1;
#else
   return SBM_GetNumberOfCharacters(sbmHandle);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character )
{
#ifdef WIN32
   if (g_SBM_GetCharacter_DEF)
   {
      return g_SBM_GetCharacter_DEF(sbmHandle, name, character);
   }
   return false;
#else
   return SBM_GetCharacter(sbmHandle, name, character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character )
{
#ifdef WIN32
   if (g_SBM_ReleaseCharacter_DEF)
   {
      return g_SBM_ReleaseCharacter_DEF(character);
   }
   return false;
#else
   return SBM_ReleaseCharacter(character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_ReleaseCharacterJoints( SBM_SmartbodyCharacter * character )
{
#ifdef WIN32
   if (g_SBM_ReleaseCharacter_DEF)
   {
      return g_SBM_ReleaseCharacter_DEF(character);
   }
   return false;
#else
   return SBM_ReleaseCharacterJoints(character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SetLogMessageCallback( LogMessageCallback cb )
{
#ifdef WIN32
   if (g_SBM_SetLogMessageCallback_DEF)
   {
      return g_SBM_SetLogMessageCallback_DEF(cb);
   }
   return false;
#else
   return SBM_SetLogMessageCallback(cb);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_LogMessage(const char* message, int messageType)
{
#ifdef WIN32
   if (g_SBM_LogMessage_DEF)
   {
      g_SBM_LogMessage_DEF(message, messageType);
   }
#else
   SBM_LogMessage(message, messageType);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterCreated( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** objectClass )
{
#ifdef WIN32
   return g_SBM_IsCharacterCreated_DEF(sbmHandle, numCharacters, name, objectClass);
#else
    return SBM_IsCharacterCreated(sbmHandle, numCharacters, name, objectClass);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, int * numCharacters, char *** name)
{
#ifdef WIN32
   return g_SBM_IsCharacterDeleted_DEF(sbmHandle, numCharacters, name);
#else
    return SBM_IsCharacterDeleted(sbmHandle, numCharacters, name);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterChanged( SBMHANDLE sbmHandle, int * numCharacters, char *** name)
{
#ifdef WIN32
   return g_SBM_IsCharacterChanged_DEF(sbmHandle, numCharacters, name);
#else
    return SBM_IsCharacterChanged(sbmHandle, numCharacters, name);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsVisemeSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** visemeName, float** weight, float** blendTime)
{
#ifdef WIN32
   return g_SBM_IsVisemeSet_DEF(sbmHandle, numCharacters, name, visemeName, weight, blendTime);
#else
    return SBM_IsVisemeSet(sbmHandle, numCharacters, name, visemeName, weight, blendTime);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsChannelSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** channelName, float ** value)
{
#ifdef WIN32
   return g_SBM_IsChannelSet_DEF(sbmHandle, numCharacters, name, channelName, value);
#else
    return SBM_IsChannelSet(sbmHandle, numCharacters, name, channelName, value);
#endif
}


////////////////////////////////////////////////////////////////////////////



VHWRAPPERDLL_API AUDIOHANDLE WRAPPER_VHCL_AUDIO_CreateAudio()
{
   g_audioHandleId++;
   g_audioInstances[g_audioHandleId] = new Audio();
   return g_audioHandleId;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Open(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   bool retVal = g_audioInstances[handle]->Open();
   return retVal;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Close(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   std::map<AUDIOHANDLE, Audio*>::iterator it = g_audioInstances.find( handle );
   Audio * aud = g_audioInstances[ handle ];
   g_audioInstances.erase( it );
   // close() gets called in the destructor
   delete aud;
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetListenerPos(AUDIOHANDLE handle, const float x, const float y, const float z )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->SetListenerPos(x, y, z);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_GetListenerPos(AUDIOHANDLE handle, float & x, float & y, float & z )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->GetListenerPos(x, y, z);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetListenerRot(AUDIOHANDLE handle, const float targetx, const float targety, const float targetz, const float upx, const float upy, const float upz )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->SetListenerRot(targetx, targety, targetz, upx, upy, upz);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_GetListenerRot(AUDIOHANDLE handle, float & targetx, float & targety, float & targetz, float & upx, float & upy, float & upz )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->GetListenerRot(targetx, targety, targetz, upx, upy, upz);
   return true;
}

VHWRAPPERDLL_API Sound* WRAPPER_VHCL_AUDIO_CreateSound(AUDIOHANDLE handle, const char* fileName, const char* name )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return NULL;
   }

   return g_audioInstances[handle]->CreateSound(fileName, name);
}

VHWRAPPERDLL_API Sound* WRAPPER_VHCL_AUDIO_PlaySound(AUDIOHANDLE handle, const char* fileName, const char* name, 
                                                             float posX, float posY, float posZ, bool looping )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return NULL;
   }

   // first look to see if it already exists
   Sound* sound = WRAPPER_VHCL_AUDIO_FindSound(handle, fileName);
   if (sound == NULL)
   {
      // it didn't exist, create it
      sound = WRAPPER_VHCL_AUDIO_CreateSound(handle, fileName, name);
   }

   if (sound != NULL)
   {
      WRAPPER_VHCL_AUDIO_AttachSoundToFreeChannel(handle, sound);
      sound->SetPosition(posX, posY, posZ);
      sound->SetLooping(looping);
      sound->Play();
   }


   return sound;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_StopSound(AUDIOHANDLE handle, const char* fileName )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   // first look to see if it already exists
   Sound* sound = WRAPPER_VHCL_AUDIO_FindSound(handle, fileName);
   if (sound != NULL)
   {
      sound->Stop();
   }

   return true;
}


VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_PauseAllSounds(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return;
   }
#ifndef __ANDROID__
   g_audioInstances[handle]->PauseAllSounds();
#endif

}


VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_StopAllSounds(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return;
   }
#ifndef __ANDROID__
   g_audioInstances[handle]->StopAllSounds();
#endif

}

VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_UnpauseAllSounds(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return;
   }
#ifndef __ANDROID__
   g_audioInstances[handle]->UnpauseAllSounds();
#endif

}







VHWRAPPERDLL_API Sound* WRAPPER_VHCL_AUDIO_CreateSoundLibSndFile(AUDIOHANDLE handle, const char* fileName, const char* name )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return NULL;
   }

   return g_audioInstances[handle]->CreateSoundLibSndFile(fileName, name);
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_DestroySound(AUDIOHANDLE handle, const char* name )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->DestroySound(name);
   return true;
}

VHWRAPPERDLL_API Sound* WRAPPER_VHCL_AUDIO_FindSound(AUDIOHANDLE handle, const char* name )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return NULL;
   }

   return g_audioInstances[handle]->FindSound(name);
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Update(AUDIOHANDLE handle, const float frameTime )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->Update(frameTime);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_AttachSoundToFreeChannel(AUDIOHANDLE handle, Sound * sound )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->AttachSoundToFreeChannel(sound);
   return true;
}  

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_ReleaseSoundFromChannel(AUDIOHANDLE handle, Sound * sound )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   g_audioInstances[handle]->ReleaseSoundFromChannel(sound);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetSoundHardwareChannel(AUDIOHANDLE handle, const char* fileName, const char* channelName )
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   // first look to see if it already exists
   Sound* sound = WRAPPER_VHCL_AUDIO_FindSound(handle, fileName);
   if (sound == NULL)
   {
      return false;
   }

   sound->SetHardwareChannel(channelName);
   return true;
}

VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SoundExists(AUDIOHANDLE handle, const char* fileName)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return false;
   }

   Sound* sound = WRAPPER_VHCL_AUDIO_FindSound(handle, fileName);
   return sound != NULL;
}

bool VHCL_AUDIO_HandleExists( const AUDIOHANDLE handle )
{
   return g_audioInstances.find( handle ) != g_audioInstances.end();
}

#ifdef ENABLE_VHMSG_WRAPPER
///VHMSG c++ WRAPPER FUNCTIONS////////////////////////////////
VHWRAPPERDLL_API VHMSGHANDLE WRAPPER_VHMSG_CreateVHMsg()
{
   g_vhmsgHandleId++;
   g_vhmsgInstances[g_vhmsgHandleId] = new vhmsg::Client();
   return g_vhmsgHandleId;
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_OpenConnection(const VHMSGHANDLE handle )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }

   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->OpenConnection();
   if (retVal)
   {
      vhmsg::ttu_open();
      vhmsg::ttu_set_client_callback( &WRAPPER_tt_client_callback, &g_vhmsgQueuedMessages );
   }
   return retVal;
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_OpenConnection2( const VHMSGHANDLE handle, const char * server, const char * port )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }

   return g_vhmsgInstances[g_vhmsgHandleId]->OpenConnection(server, port);
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_CloseConnection(const VHMSGHANDLE handle)
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   vhmsg::ttu_close();
   g_vhmsgInstances[g_vhmsgHandleId]->CloseConnection();
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_Send( const VHMSGHANDLE handle, const char * message )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }
   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->Send(message);
   vhmsg::ttu_notify1( message );
   return retVal;
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_Send2( const VHMSGHANDLE handle, const wchar_t * message )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }
   
   char* m = ConvertWCharToChar(message);
   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->Send(m);
   vhmsg::ttu_notify1( m );
   delete m;
   return retVal;
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_EnablePollingMethod(const VHMSGHANDLE handle)
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   g_vhmsgInstances[g_vhmsgHandleId]->EnablePollingMethod();
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_EnableImmediateMethod(const VHMSGHANDLE handle)
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   g_vhmsgInstances[g_vhmsgHandleId]->EnableImmediateMethod();
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_SetListener( const VHMSGHANDLE handle, vhmsg::Listener * listener )
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   g_vhmsgInstances[g_vhmsgHandleId]->SetListener(listener);
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_Subscribe( const VHMSGHANDLE handle, const char * req )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }

   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->Subscribe(req);
   vhmsg::ttu_register( req );
   return retVal;
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_Subscribe2( const VHMSGHANDLE handle, const wchar_t * req )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }

   char* m = ConvertWCharToChar(req);
   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->Subscribe(m);
   vhmsg::ttu_register( m );
   delete m;
   return retVal;
}

VHWRAPPERDLL_API bool WRAPPER_VHMSG_Unsubscribe( const VHMSGHANDLE handle, const wchar_t * req )
{
   if (!VHMSG_HandleExists(handle))
   {
      return false;
   }

   char* m = ConvertWCharToChar(req);
   bool retVal = g_vhmsgInstances[g_vhmsgHandleId]->Unsubscribe(m);
   vhmsg::ttu_unregister( m );
   delete m;
   return retVal;
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_Poll(const VHMSGHANDLE handle)
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   g_vhmsgInstances[g_vhmsgHandleId]->Poll();
   vhmsg::ttu_poll();
}

VHWRAPPERDLL_API void WRAPPER_VHMSG_WaitAndPoll( const VHMSGHANDLE handle, const double waitTimeSeconds )
{
   if (!VHMSG_HandleExists(handle))
   {
      return;
   }

   g_vhmsgInstances[g_vhmsgHandleId]->WaitAndPoll(waitTimeSeconds);
}

VHWRAPPERDLL_API wchar_t* WRAPPER_VHMSG_GetMessages( )
{
   /*for (unsigned int i = 0; i < g_vhmsgQueuedMessages.size(); i++)
   {
      
   }*/
   wchar_t* retVal = NULL;
   if (g_vhmsgQueuedMessages.size() >= 1)
   {
      retVal = g_vhmsgQueuedMessages[0];
      g_vhmsgQueuedMessages.erase(g_vhmsgQueuedMessages.begin());
   }
   //g_vhmsgQueuedMessages.clear();

   return retVal;
}

VHWRAPPERDLL_API int WRAPPER_VHMSG_GetNumQueuedMessages( )
{
   return g_vhmsgQueuedMessages.size();
}

char* ConvertWCharToChar(const wchar_t * wc)
{
   size_t convertedChars = 0;
   size_t  sizeInBytes = ((wcslen(wc) + 1) * 2);
   errno_t err = 0;
   char    *ch = (char *)malloc(sizeInBytes);
   err = wcstombs_s(&convertedChars, ch, sizeInBytes, wc, sizeInBytes);
   return ch;
}

wchar_t* ConvertCharToWChar(const char* c)
{
   size_t convertedChars = 0;
   size_t  sizeInBytes = ((strlen(c) + 1) * 2);
   errno_t err = 0;
   wchar_t    *wc = (wchar_t *)malloc(sizeInBytes);
   err = mbstowcs_s(&convertedChars, wc, sizeInBytes, c, sizeInBytes);
   return wc;
}

bool VHMSG_HandleExists( const VHMSGHANDLE handle )
{
   return g_vhmsgInstances.find( handle ) != g_vhmsgInstances.end();
}

void WRAPPER_tt_client_callback(const char * op, const char * args, void * user_data )
{
   std::vector<wchar_t*>* queuedMessages = reinterpret_cast<std::vector<wchar_t*>* >(user_data);
   std::string msg = op;
   msg += " ";
   msg += args;
   wchar_t* msgwc = ConvertCharToWChar(msg.c_str());
   queuedMessages->push_back(msgwc);
}
#endif

//////////////////////////////////////////////////////////////
