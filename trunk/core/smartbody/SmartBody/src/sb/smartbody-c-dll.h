
#ifndef SMARTBODY_C_DLL_H
#define SMARTBODY_C_DLL_H

#include "vhcl_public.h"

#include "sb/SBTypes.h"
#include "sb/SBCharacterFrameData.h"


typedef intptr_t SBMHANDLE;


#ifdef __cplusplus
extern "C" {
#endif 


SBAPI SBMHANDLE SBM_CreateSBM();

SBAPI bool SBM_Init( SBMHANDLE sbmHandle, const char * pythonLibPath, bool logToFile );
SBAPI bool SBM_Shutdown( SBMHANDLE sbmHandle );

SBAPI bool SBM_LoadSkeleton( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * skeletonName );
SBAPI bool SBM_LoadMotion( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * motionName );

SBAPI bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds );

SBAPI void SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id );
SBAPI void SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );
SBAPI void SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled );

SBAPI bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args );

SBAPI bool SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character );
SBAPI bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character );
SBAPI bool SBM_ReleaseCharacter( SBM_CharacterFrameDataMarshalFriendly * character );

// used for polling events.  These must be called regularly, or else their queues will overflow
SBAPI bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen );
SBAPI bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen );
SBAPI bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen );
SBAPI bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime );
SBAPI bool SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value );
SBAPI bool SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char * logMessage, int maxLogMessageLen, int * logMessageType );

// python usage functions
SBAPI bool SBM_PythonCommandVoid( SBMHANDLE sbmHandle, const char * command );
SBAPI bool SBM_PythonCommandBool( SBMHANDLE sbmHandle, const char * command );
SBAPI int SBM_PythonCommandInt( SBMHANDLE sbmHandle, const char * command );
SBAPI float SBM_PythonCommandFloat( SBMHANDLE sbmHandle, const char * command );
SBAPI char * SBM_PythonCommandString( SBMHANDLE sbmHandle, const char * command, char * output, int maxLen);


#ifdef __cplusplus
}
#endif

#endif  // SMARTBODY_C_DLL_H
