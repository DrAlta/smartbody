
#ifndef SMARTBODY_C_DLL_H
#define SMARTBODY_C_DLL_H

#ifdef WIN32
#ifdef SMARTBODY_C_DLL_EXPORTS
#define SMARTBODY_C_DLL_API __declspec(dllexport)
#else
#define SMARTBODY_C_DLL_API __declspec(dllimport)
#endif
#else
#define SMARTBODY_C_DLL_API 
#define __stdcall
#endif


#include "vhcl_public.h"


typedef intptr_t SBMHANDLE;

typedef void (__stdcall *LogMessageCallback)(const char* message, int messageType);  // 0 = normal, 1 = error, 2 = warning

// Listener callbacks to get Smartbody related notifications
typedef int (__stdcall *SBM_OnCreateCharacterCallback)( SBMHANDLE sbmHandle, const char * name, const char * objectClass );
typedef int (__stdcall *SBM_OnCharacterDeleteCallback)( SBMHANDLE sbmHandle, const char * name );
typedef int (__stdcall *SBM_OnCharacterChangeCallback)( SBMHANDLE sbmHandle, const char * name );
typedef int (__stdcall *SBM_OnVisemeCallback)( SBMHANDLE sbmHandle, const char * name, const char * visemeName, float weight, float blendTime );
typedef int (__stdcall *SBM_OnChannelCallback)( SBMHANDLE sbmHandle, const char * name, const char * channelName, float value );


#ifdef __cplusplus
extern "C" {
#endif 


// keeping the joint data in separate arrays to help with marshalling
struct SBM_SmartbodyCharacter
{
   char * m_name;
   float x;
   float y;
   float z;
   float rw;
   float rx;
   float ry;
   float rz;
   size_t m_numJoints;

   char ** jname;
   float * jx;
   float * jy;
   float * jz;
   float * jrw;
   float * jrx;
   float * jry;
   float * jrz;
};


SMARTBODY_C_DLL_API SBMHANDLE SBM_CreateSBM();

SMARTBODY_C_DLL_API bool SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath );
SMARTBODY_C_DLL_API bool SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId );
SMARTBODY_C_DLL_API bool SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path );

SMARTBODY_C_DLL_API bool SBM_Init( SBMHANDLE sbmHandle, const char* pythonLibPath, bool logToFile );
SMARTBODY_C_DLL_API bool SBM_Shutdown( SBMHANDLE sbmHandle );

SMARTBODY_C_DLL_API bool SBM_LoadSkeleton( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * skeletonName );
SMARTBODY_C_DLL_API bool SBM_LoadMotion( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * motionName );

SMARTBODY_C_DLL_API bool SBM_MapSkeleton( SBMHANDLE sbmHandle, const char * mapName, const char * skeletonName );
SMARTBODY_C_DLL_API bool SBM_MapMotion( SBMHANDLE sbmHandle, const char * mapName, const char * motionName );

SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changedCB, SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB );

SMARTBODY_C_DLL_API bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds );

SMARTBODY_C_DLL_API void SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id );
SMARTBODY_C_DLL_API void SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );
SMARTBODY_C_DLL_API void SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled );

SMARTBODY_C_DLL_API bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args );
SMARTBODY_C_DLL_API bool SBM_ExecutePython( SBMHANDLE sbmHandle, const char * command );

SMARTBODY_C_DLL_API int  SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle );
SMARTBODY_C_DLL_API bool SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_SetLogMessageCallback(LogMessageCallback cb);
SMARTBODY_C_DLL_API void SBM_LogMessage(const char * message, int messageType);

// used for polling on iOS since callbacks aren't allowed
SMARTBODY_C_DLL_API bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen );
SMARTBODY_C_DLL_API bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen );
SMARTBODY_C_DLL_API bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen );
SMARTBODY_C_DLL_API bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime );
SMARTBODY_C_DLL_API bool SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value );
SMARTBODY_C_DLL_API bool SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char *logMessage, int maxLogMessageLen, int* logMessageType );

// python usage functions
// functions can't be distinguished by return type alone so they are named differently
SMARTBODY_C_DLL_API bool SBM_PythonCommandVoid( SBMHANDLE sbmHandle,  const char * command );
SMARTBODY_C_DLL_API bool SBM_PythonCommandBool( SBMHANDLE sbmHandle,  const char * command );
SMARTBODY_C_DLL_API int SBM_PythonCommandInt( SBMHANDLE sbmHandle,  const char * command );
SMARTBODY_C_DLL_API float SBM_PythonCommandFloat( SBMHANDLE sbmHandle,  const char * command );
SMARTBODY_C_DLL_API char * SBM_PythonCommandString( SBMHANDLE sbmHandle, const char * command, char * output, int maxLen);


#ifdef __cplusplus
}
#endif

#endif  // SMARTBODY_C_DLL_H
