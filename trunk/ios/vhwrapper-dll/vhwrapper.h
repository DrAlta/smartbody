
// This is a C-style wrapper for VH libraries that don't already have C-style headers.
//    - Message Callbacks - for passing log messages across the C/C# border
//    - SBM DLL - has it's own interface, but needs wrapping in order to be unloaded in Unity
//    - vhcl_audio - doesn't have a C-stlye interface, and doesn't exist as a dll.

//#define ENABLE_SBM

#include "vhcl_public.h"

#include "sb/smartbody-c-dll.h"

#ifdef ENABLE_VHMSG_WRAPPER
#include "vhmsg.h"
#endif


#if defined(WIN_BUILD)
#ifdef VHWRAPPERDLL_EXPORTS
#define VHWRAPPERDLL_API __declspec(dllexport)
#else
#define VHWRAPPERDLL_API __declspec(dllimport)
#endif
#else
#define VHWRAPPERDLL_API 
#define __stdcall
#endif


#ifdef __cplusplus
extern "C" {
#endif


VHWRAPPERDLL_API void __stdcall SetLogMessageCallback(LogMessageCallback cb);

// SBM
VHWRAPPERDLL_API SBMHANDLE WRAPPER_SBM_CreateSBM(const bool releaseMode);
VHWRAPPERDLL_API bool WRAPPER_SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath );
VHWRAPPERDLL_API bool WRAPPER_SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId );
VHWRAPPERDLL_API bool WRAPPER_SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path );
VHWRAPPERDLL_API bool WRAPPER_SBM_Init( SBMHANDLE sbmHandle, const char * pythonPath, bool logToFile );
VHWRAPPERDLL_API bool WRAPPER_SBM_Shutdown( SBMHANDLE sbmHandle );
VHWRAPPERDLL_API bool WRAPPER_SBM_LoadSkeleton( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * skeletonName );
VHWRAPPERDLL_API bool WRAPPER_SBM_LoadMotion( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * motionName );
VHWRAPPERDLL_API bool WRAPPER_SBM_MapSkeleton( SBMHANDLE sbmHandle, const char * mapName, const char * skeletonName );
VHWRAPPERDLL_API bool WRAPPER_SBM_MapMotion( SBMHANDLE sbmHandle, const char * mapName, const char * motionName );
VHWRAPPERDLL_API bool WRAPPER_SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB,
                                               SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changeCB,
                                               SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB );
VHWRAPPERDLL_API bool WRAPPER_SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds );
VHWRAPPERDLL_API void WRAPPER_SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id );
VHWRAPPERDLL_API void WRAPPER_SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );
VHWRAPPERDLL_API void WRAPPER_SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled );
VHWRAPPERDLL_API bool WRAPPER_SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args );
VHWRAPPERDLL_API int  WRAPPER_SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle );
VHWRAPPERDLL_API bool WRAPPER_SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
VHWRAPPERDLL_API bool WRAPPER_SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
VHWRAPPERDLL_API bool WRAPPER_SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character );
VHWRAPPERDLL_API bool WRAPPER_SBM_SetLogMessageCallback( LogMessageCallback cb );
VHWRAPPERDLL_API void WRAPPER_SBM_LogMessage( const char * message, int messageType );

// used for polling on iOS since callbacks aren't allowed
VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen );
VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen );
VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen );
VHWRAPPERDLL_API bool WRAPPER_SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime );
VHWRAPPERDLL_API bool WRAPPER_SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value );
VHWRAPPERDLL_API bool WRAPPER_SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char * logMessage, int maxLogMessageLen, int * messageType );

// functions can't be distinguished by return type alone so they are named differently
VHWRAPPERDLL_API bool   WRAPPER_SBM_PythonCommandVoid(SBMHANDLE sbmHandle, const char * command);
VHWRAPPERDLL_API bool   WRAPPER_SBM_PythonCommandBool(SBMHANDLE sbmHandle, const char * command);
VHWRAPPERDLL_API int    WRAPPER_SBM_PythonCommandInt(SBMHANDLE sbmHandle, const char * command);
VHWRAPPERDLL_API float  WRAPPER_SBM_PythonCommandFloat(SBMHANDLE sbmHandle, const char * command);
VHWRAPPERDLL_API char * WRAPPER_SBM_PythonCommandString(SBMHANDLE sbmHandle, const char * command, char * output, int maxLen);


// VHCL AUDIO

namespace vhcl { class Sound; }

typedef intptr_t AUDIOHANDLE;

VHWRAPPERDLL_API AUDIOHANDLE WRAPPER_VHCL_AUDIO_CreateAudio();
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Open(AUDIOHANDLE handle);
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Close(AUDIOHANDLE handle);
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetListenerPos(AUDIOHANDLE handle, const float x, const float y, const float z );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_GetListenerPos(AUDIOHANDLE handle, float & x, float & y, float & z );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetListenerRot(AUDIOHANDLE handle, const float targetx, const float targety, const float targetz, const float upx, const float upy, const float upz );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_GetListenerRot(AUDIOHANDLE handle, float & targetx, float & targety, float & targetz, float & upx, float & upy, float & upz );
VHWRAPPERDLL_API vhcl::Sound* WRAPPER_VHCL_AUDIO_CreateSound(AUDIOHANDLE handle, const char* fileName, const char* name );
VHWRAPPERDLL_API vhcl::Sound* WRAPPER_VHCL_AUDIO_PlaySound(AUDIOHANDLE handle, const char* fileName, const char* name, float posX, float posY, float posZ, bool looping );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_StopSound(AUDIOHANDLE handle, const char* fileName );
VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_PauseAllSounds(AUDIOHANDLE handle);
VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_UnpauseAllSounds(AUDIOHANDLE handle);
VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_StopAllSounds(AUDIOHANDLE handle);
VHWRAPPERDLL_API vhcl::Sound* WRAPPER_VHCL_AUDIO_CreateSoundLibSndFile(AUDIOHANDLE handle, const char* fileName, const char* name );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_DestroySound(AUDIOHANDLE handle, const char* name );
VHWRAPPERDLL_API vhcl::Sound* WRAPPER_VHCL_AUDIO_FindSound(AUDIOHANDLE handle, const char* name );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_Update(AUDIOHANDLE handle, const float frameTime );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_AttachSoundToFreeChannel(AUDIOHANDLE handle, vhcl::Sound * sound );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_ReleaseSoundFromChannel(AUDIOHANDLE handle, vhcl::Sound * sound );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SetSoundHardwareChannel(AUDIOHANDLE handle, const char* fileName, const char* channelName );
VHWRAPPERDLL_API bool WRAPPER_VHCL_AUDIO_SoundExists(AUDIOHANDLE handle, const char* fileName);


#ifdef ENABLE_VHMSG_WRAPPER
///VHMSG c++ WRAPPER FUNCTIONS////////////////////////////////
typedef int/*intptr_t*/ VHMSGHANDLE;
VHWRAPPERDLL_API VHMSGHANDLE WRAPPER_VHMSG_CreateVHMsg();
VHWRAPPERDLL_API bool WRAPPER_VHMSG_OpenConnection(const VHMSGHANDLE handle );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_OpenConnection2(const VHMSGHANDLE handle, const char * server, const char * port );
VHWRAPPERDLL_API void WRAPPER_VHMSG_CloseConnection(const VHMSGHANDLE handle );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_Send( const VHMSGHANDLE handle, const char * message );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_Send2( const VHMSGHANDLE handle, const wchar_t * message );
VHWRAPPERDLL_API void WRAPPER_VHMSG_EnablePollingMethod(const VHMSGHANDLE handle);
VHWRAPPERDLL_API void WRAPPER_VHMSG_EnableImmediateMethod(const VHMSGHANDLE handle);
VHWRAPPERDLL_API void WRAPPER_VHMSG_SetListener( const VHMSGHANDLE handle, vhmsg::Listener * listener );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_Subscribe( const VHMSGHANDLE handle, const char * req );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_Subscribe2( const VHMSGHANDLE handle, const wchar_t * req );
VHWRAPPERDLL_API bool WRAPPER_VHMSG_Unsubscribe( const VHMSGHANDLE handle, const wchar_t * req );
VHWRAPPERDLL_API void WRAPPER_VHMSG_Poll(const VHMSGHANDLE handle);
VHWRAPPERDLL_API void WRAPPER_VHMSG_WaitAndPoll( const VHMSGHANDLE handle, const double waitTimeSeconds );
VHWRAPPERDLL_API wchar_t* WRAPPER_VHMSG_GetMessages( );
VHWRAPPERDLL_API int WRAPPER_VHMSG_GetNumQueuedMessages( );

char* ConvertWCharToChar(const wchar_t * wc);
bool VHMSG_HandleExists( const VHMSGHANDLE handle );
void WRAPPER_tt_client_callback(const char * op, const char * args, void * user_data);
//////////////////////////////////////////////////////////////
#endif


#ifdef __cplusplus
}
#endif



// stubs for testing library loading on different platforms
#if 0

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t SBMHANDLE;
SBMHANDLE WRAPPER_SBM_CreateSBM(const bool releaseMode);

#ifdef __cplusplus
}
#endif

#endif
