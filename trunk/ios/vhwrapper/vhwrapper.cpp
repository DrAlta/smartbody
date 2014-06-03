
#include "vhcl.h"

#include "vhwrapper.h"

#include <map>
#include <vector>

#include <stdio.h>

#ifdef WIN_BUILD
#include <windows.h>
#endif

#ifdef ENABLE_VHMSG_WRAPPER
#include "vhmsg-tt.h"
#endif

#include "vhcl_audio.h"


using vhcl::Audio;
using vhcl::Sound;


bool VHCL_AUDIO_HandleExists( const AUDIOHANDLE handle );


#ifdef WIN_BUILD
HINSTANCE g_SBM_HINST = NULL;
#endif

#ifdef ENABLE_VHMSG_WRAPPER
std::map<VHMSGHANDLE, vhmsg::Client*> g_vhmsgInstances;
std::vector<wchar_t*> g_vhmsgQueuedMessages;
int g_vhmsgHandleId = 0;
#endif

std::map<AUDIOHANDLE, Audio*> g_audioInstances;
int g_audioHandleId = 0;


typedef SBMHANDLE (*SBM_CreateSBM_DEF)();
typedef bool (*SBM_Init_DEF)( SBMHANDLE, const char *, bool );
typedef bool (*SBM_Shutdown_DEF)( SBMHANDLE );
typedef bool (*SBM_Update_DEF)(SBMHANDLE, double);
typedef bool (*SBM_ProcessVHMsgs_DEF)(SBMHANDLE, const char *, const char*);
typedef bool (*SBM_InitCharacter_DEF)( SBMHANDLE sbmHandle, const char*, SBM_CharacterFrameDataMarshalFriendly * );
typedef bool (*SBM_GetCharacter_DEF)( SBMHANDLE sbmHandle, const char*, SBM_CharacterFrameDataMarshalFriendly * );
typedef bool (*SBM_ReleaseCharacter_DEF)(SBM_CharacterFrameDataMarshalFriendly *);
typedef bool (*SBM_IsCharacterCreated_DEF)( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen );
typedef bool (*SBM_IsLogMessageWaiting_DEF)( SBMHANDLE sbmHandle, char *logMessage, int maxLogMessageLen, int * messageType );
typedef bool (*SBM_IsCharacterDeleted_DEF)( SBMHANDLE sbmHandle, char * name, int maxNameLen );
typedef bool (*SBM_IsCharacterChanged_DEF)( SBMHANDLE sbmHandle, char * name, int maxNameLen );
typedef bool (*SBM_IsVisemeSet_DEF)( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime );
typedef bool (*SBM_IsChannelSet_DEF)( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value );
typedef bool (*SBM_PythonCommandVoid_DEF)(SBMHANDLE sbmHandle, const char * command);
typedef bool (*SBM_PythonCommandBool_DEF)(SBMHANDLE sbmHandle, const char * command);
typedef int  (*SBM_PythonCommandInt_DEF)(SBMHANDLE sbmHandle, const char *command);
typedef float (*SBM_PythonCommandFloat_DEF)(SBMHANDLE sbmHandle, const char * command);
typedef char * (*SBM_PythonCommandString_DEF)(SBMHANDLE sbmHandle, const char * command, char * output, int maxLen);
typedef bool (*SBM_SBAssetManager_LoadSkeleton_DEF)( SBMHANDLE, const void *, int, const char * );
typedef bool (*SBM_SBAssetManager_LoadMotion_DEF)( SBMHANDLE, const void *, int, const char * );
typedef bool (*SBM_SBDebuggerServer_SetID_DEF)(SBMHANDLE, const char * );
typedef bool (*SBM_SBDebuggerServer_SetCameraValues_DEF)(SBMHANDLE, double, double, double, double, double, double, double, double, double, double, double );
typedef bool (*SBM_SBDebuggerServer_SetRendererIsRightHanded_DEF)(SBMHANDLE, bool );
typedef void (*SBM_SBMotion_AddChannel_DEF)(SBMHANDLE sbmHandle, const char * motionName, const char * channelName, const char * channelType);
typedef void (*SBM_SBMotion_AddFrame_DEF)(SBMHANDLE sbmHandle, const char * motionName, float frameTime, const float * frameData, int numFrameData);
typedef void (*SBM_SBMotion_SetSyncPoint_DEF)(SBMHANDLE sbmHandle, const char * motionName, const char * syncTag, double time);
typedef void (*SBM_SBJointMap_GetMapTarget_DEF)(SBMHANDLE sbmHandle, const char * jointMap, const char * jointName, char * mappedJointName, int maxMappedJointName);
typedef void (*SBM_SBVHMsgManager_SetServer_DEF)(SBMHANDLE sbmHandle, const char * server);
typedef void (*SBM_SBVHMsgManager_SetScope_DEF)(SBMHANDLE sbmHandle, const char * scope);
typedef void (*SBM_SBVHMsgManager_SetPort_DEF)(SBMHANDLE sbmHandle, const char * port);
typedef void (*SBM_SBVHMsgManager_SetEnable_DEF)(SBMHANDLE sbmHandle, bool enable);

SBM_CreateSBM_DEF                  g_SBM_CreateSBM = NULL;
SBM_Init_DEF                       g_SBM_Init = NULL;
SBM_Shutdown_DEF                   g_SBM_Shutdown = NULL;
SBM_Update_DEF                     g_SBM_Update = NULL;
SBM_ProcessVHMsgs_DEF              g_SBM_ProcessVHMsgs = NULL;
SBM_InitCharacter_DEF              g_SBM_InitCharacter = NULL;
SBM_GetCharacter_DEF               g_SBM_GetCharacter = NULL;
SBM_ReleaseCharacter_DEF           g_SBM_ReleaseCharacter = NULL;
SBM_IsCharacterCreated_DEF         g_SBM_IsCharacterCreated = NULL;
SBM_IsLogMessageWaiting_DEF        g_SBM_IsLogMessageWaiting = NULL;
SBM_IsCharacterDeleted_DEF         g_SBM_IsCharacterDeleted = NULL;
SBM_IsCharacterChanged_DEF         g_SBM_IsCharacterChanged = NULL;
SBM_IsVisemeSet_DEF                g_SBM_IsVisemeSet = NULL;
SBM_IsChannelSet_DEF               g_SBM_IsChannelSet = NULL;
SBM_PythonCommandVoid_DEF          g_SBM_PythonCommandVoid = NULL;
SBM_PythonCommandBool_DEF          g_SBM_PythonCommandBool = NULL;
SBM_PythonCommandInt_DEF           g_SBM_PythonCommandInt = NULL;
SBM_PythonCommandFloat_DEF         g_SBM_PythonCommandFloat = NULL;
SBM_PythonCommandString_DEF        g_SBM_PythonCommandString = NULL;
SBM_SBAssetManager_LoadSkeleton_DEF g_SBM_SBAssetManager_LoadSkeleton = NULL;
SBM_SBAssetManager_LoadMotion_DEF  g_SBM_SBAssetManager_LoadMotion = NULL;
SBM_SBDebuggerServer_SetID_DEF     g_SBM_SBDebuggerServer_SetID = NULL;
SBM_SBDebuggerServer_SetCameraValues_DEF g_SBM_SBDebuggerServer_SetCameraValues = NULL;
SBM_SBDebuggerServer_SetRendererIsRightHanded_DEF g_SBM_SBDebuggerServer_SetRendererIsRightHanded = NULL;
SBM_SBMotion_AddChannel_DEF        g_SBM_SBMotion_AddChannel = NULL;
SBM_SBMotion_AddFrame_DEF          g_SBM_SBMotion_AddFrame = NULL;
SBM_SBMotion_SetSyncPoint_DEF      g_SBM_SBMotion_SetSyncPoint = NULL;
SBM_SBJointMap_GetMapTarget_DEF    g_SBM_SBJointMap_GetMapTarget = NULL;
SBM_SBVHMsgManager_SetServer_DEF   g_SBM_SBVHMsgManager_SetServer = NULL;
SBM_SBVHMsgManager_SetScope_DEF    g_SBM_SBVHMsgManager_SetScope = NULL;
SBM_SBVHMsgManager_SetPort_DEF     g_SBM_SBVHMsgManager_SetPort = NULL;
SBM_SBVHMsgManager_SetEnable_DEF   g_SBM_SBVHMsgManager_SetEnable = NULL;


VHWRAPPERDLL_API SBMHANDLE WRAPPER_SBM_CreateSBM(const bool releaseMode)
{
#ifdef WIN_BUILD
   if (releaseMode)
   {
      g_SBM_HINST = LoadLibrary(TEXT("SmartBody.dll"));
   }
   else
   {
      g_SBM_HINST = LoadLibrary(TEXT("SmartBody_d.dll"));
   }

   if (g_SBM_HINST == NULL)
   {
      //WRAPPER_SBM_LogMessage(vhcl::Format("ERROR: Failed to LoadLibrary '%s'", releaseMode ? "SmartBody.dll" : "SmartBody_d.dll").c_str(), 1);
      return -1;
   }

   g_SBM_CreateSBM                  = (SBM_CreateSBM_DEF)GetProcAddress(g_SBM_HINST, "SBM_CreateSBM");
   g_SBM_Init                       = (SBM_Init_DEF)GetProcAddress(g_SBM_HINST, "SBM_Init");
   g_SBM_Shutdown                   = (SBM_Shutdown_DEF)GetProcAddress(g_SBM_HINST, "SBM_Shutdown");
   g_SBM_Update                     = (SBM_Update_DEF)GetProcAddress(g_SBM_HINST, "SBM_Update");
   g_SBM_ProcessVHMsgs              = (SBM_ProcessVHMsgs_DEF)GetProcAddress(g_SBM_HINST, "SBM_ProcessVHMsgs");
   g_SBM_InitCharacter              = (SBM_InitCharacter_DEF)GetProcAddress(g_SBM_HINST, "SBM_InitCharacter");
   g_SBM_GetCharacter               = (SBM_GetCharacter_DEF)GetProcAddress(g_SBM_HINST, "SBM_GetCharacter");
   g_SBM_ReleaseCharacter           = (SBM_ReleaseCharacter_DEF)GetProcAddress(g_SBM_HINST, "SBM_ReleaseCharacter");
   g_SBM_IsCharacterCreated         = (SBM_IsCharacterCreated_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterCreated");
   g_SBM_IsLogMessageWaiting        = (SBM_IsLogMessageWaiting_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsLogMessageWaiting");
   g_SBM_IsCharacterDeleted         = (SBM_IsCharacterDeleted_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterDeleted");
   g_SBM_IsCharacterChanged         = (SBM_IsCharacterChanged_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsCharacterChanged");
   g_SBM_IsVisemeSet                = (SBM_IsVisemeSet_DEF )GetProcAddress(g_SBM_HINST, "SBM_IsVisemeSet");
   g_SBM_IsChannelSet               = (SBM_IsChannelSet_DEF)GetProcAddress(g_SBM_HINST, "SBM_IsChannelSet");
   g_SBM_PythonCommandVoid          = (SBM_PythonCommandVoid_DEF)GetProcAddress(g_SBM_HINST, "SBM_PythonCommandVoid");
   g_SBM_PythonCommandBool          = (SBM_PythonCommandBool_DEF)GetProcAddress(g_SBM_HINST, "SBM_PythonCommandBool");
   g_SBM_PythonCommandInt           = (SBM_PythonCommandInt_DEF)GetProcAddress(g_SBM_HINST, "SBM_PythonCommandInt");
   g_SBM_PythonCommandFloat         = (SBM_PythonCommandFloat_DEF)GetProcAddress(g_SBM_HINST, "SBM_PythonCommandFloat");
   g_SBM_PythonCommandString        = (SBM_PythonCommandString_DEF)GetProcAddress(g_SBM_HINST, "SBM_PythonCommandString");
   g_SBM_SBAssetManager_LoadSkeleton = (SBM_SBAssetManager_LoadSkeleton_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBAssetManager_LoadSkeleton");
   g_SBM_SBAssetManager_LoadMotion   = (SBM_SBAssetManager_LoadMotion_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBAssetManager_LoadMotion");
   g_SBM_SBDebuggerServer_SetID                    = (SBM_SBDebuggerServer_SetID_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBDebuggerServer_SetID");
   g_SBM_SBDebuggerServer_SetCameraValues          = (SBM_SBDebuggerServer_SetCameraValues_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBDebuggerServer_SetCameraValues");
   g_SBM_SBDebuggerServer_SetRendererIsRightHanded = (SBM_SBDebuggerServer_SetRendererIsRightHanded_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBDebuggerServer_SetRendererIsRightHanded");
   g_SBM_SBMotion_AddChannel        = (SBM_SBMotion_AddChannel_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBMotion_AddChannel");
   g_SBM_SBMotion_AddFrame          = (SBM_SBMotion_AddFrame_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBMotion_AddFrame");
   g_SBM_SBMotion_SetSyncPoint      = (SBM_SBMotion_SetSyncPoint_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBMotion_SetSyncPoint");
   g_SBM_SBJointMap_GetMapTarget    = (SBM_SBJointMap_GetMapTarget_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBJointMap_GetMapTarget");
   g_SBM_SBVHMsgManager_SetServer   = (SBM_SBVHMsgManager_SetServer_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBVHMsgManager_SetServer");
   g_SBM_SBVHMsgManager_SetScope    = (SBM_SBVHMsgManager_SetScope_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBVHMsgManager_SetScope");
   g_SBM_SBVHMsgManager_SetPort     = (SBM_SBVHMsgManager_SetPort_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBVHMsgManager_SetPort");
   g_SBM_SBVHMsgManager_SetEnable   = (SBM_SBVHMsgManager_SetEnable_DEF)GetProcAddress(g_SBM_HINST, "SBM_SBVHMsgManager_SetEnable");

   if (g_SBM_CreateSBM)
   {
      return g_SBM_CreateSBM();
   }

   return -1;
#else
   return SBM_CreateSBM();
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_Init( SBMHANDLE sbmHandle, const char * pythonPath, bool logToFile )
{
#ifdef WIN_BUILD
   if (g_SBM_Init)
   {
      return g_SBM_Init(sbmHandle, pythonPath, logToFile);
   }
   return false;
#else
   return SBM_Init(sbmHandle, pythonPath, logToFile);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_Shutdown( SBMHANDLE sbmHandle )
{
#ifdef WIN_BUILD
   bool retVal = false;
   if (g_SBM_Shutdown)
   {
      retVal = g_SBM_Shutdown(sbmHandle);
   }

   BOOL freeSuccessful = FreeLibrary(g_SBM_HINST);
   g_SBM_HINST = NULL;
   if (!freeSuccessful)
   {
      //WRAPPER_SBM_LogMessage("ERROR: Failed to FreeLibrary SmartBody.dll", 1);
      return false;
   }
   else
   {
      //WRAPPER_SBM_LogMessage("SUCCESS!: FreeLibrary SmartBody.dll", 0);
   }

   return retVal;
#else
   return SBM_Shutdown(sbmHandle);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds )
{
#ifdef WIN_BUILD
   if (g_SBM_Update)
   {
      return g_SBM_Update(sbmHandle, timeInSeconds);
   }
   return false;
#else
   return SBM_Update(sbmHandle, timeInSeconds);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args )
{
#ifdef WIN_BUILD
   if (g_SBM_ProcessVHMsgs)
   {
      return g_SBM_ProcessVHMsgs(sbmHandle, op, args);
   }
   return false;
#else
   return SBM_ProcessVHMsgs(sbmHandle, op, args);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character )
{
#ifdef WIN_BUILD
   if (g_SBM_InitCharacter)
   {
      return g_SBM_InitCharacter(sbmHandle, name, character);
   }
   return false;
#else
   return SBM_InitCharacter(sbmHandle, name, character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character )
{
#ifdef WIN_BUILD
   if (g_SBM_GetCharacter)
   {
      return g_SBM_GetCharacter(sbmHandle, name, character);
   }
   return false;
#else
   return SBM_GetCharacter(sbmHandle, name, character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_ReleaseCharacter( SBM_CharacterFrameDataMarshalFriendly * character )
{
#ifdef WIN_BUILD
   if (g_SBM_ReleaseCharacter)
   {
      return g_SBM_ReleaseCharacter(character);
   }
   return false;
#else
   return SBM_ReleaseCharacter(character);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen )
{
#ifdef WIN_BUILD
   if (g_SBM_IsCharacterCreated)
   {
      return g_SBM_IsCharacterCreated(sbmHandle, name, maxNameLen, objectClass, maxObjectClassLen);
   }
   return false;
#else
   return SBM_IsCharacterCreated(sbmHandle, name, maxNameLen, objectClass, maxObjectClassLen);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char * logMessage, int maxLogMessageLen, int * messageType)
{
#ifdef WIN_BUILD
    if (g_SBM_IsLogMessageWaiting)
    {
        return g_SBM_IsLogMessageWaiting(sbmHandle, logMessage, maxLogMessageLen, messageType);
    }
    return false;
#else
    return SBM_IsLogMessageWaiting(sbmHandle, logMessage, maxLogMessageLen, messageType);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen )
{
#ifdef WIN_BUILD
   if (g_SBM_IsCharacterDeleted)
   {
      return g_SBM_IsCharacterDeleted(sbmHandle, name, maxNameLen);
   }
   return false;
#else
   return SBM_IsCharacterDeleted(sbmHandle, name, maxNameLen);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen )
{
#ifdef WIN_BUILD
   if (g_SBM_IsCharacterChanged)
   {
      return g_SBM_IsCharacterChanged(sbmHandle, name, maxNameLen);
   }
   return false;
#else
   return SBM_IsCharacterChanged(sbmHandle, name, maxNameLen);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime )
{
#ifdef WIN_BUILD
   if (g_SBM_IsVisemeSet)
   {
      return g_SBM_IsVisemeSet(sbmHandle, name, maxNameLen, visemeName, maxVisemeNameLen, weight, blendTime);
   }
   return false;
#else
   return SBM_IsVisemeSet(sbmHandle, name, maxNameLen, visemeName, maxVisemeNameLen, weight, blendTime);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value )
{
#ifdef WIN_BUILD
   if (g_SBM_IsChannelSet)
   {
      return g_SBM_IsChannelSet(sbmHandle, name, maxNameLen, channelName, maxChannelNameLen, value);
   }
   return false;
#else
   return SBM_IsChannelSet(sbmHandle, name, maxNameLen, channelName, maxChannelNameLen, value);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_PythonCommandVoid(SBMHANDLE sbmHandle, const char * command)
{
#ifdef WIN_BUILD
   if (g_SBM_PythonCommandVoid)
   {
      return g_SBM_PythonCommandVoid(sbmHandle, command);
   }
   return false;
#else
   return SBM_PythonCommandVoid(sbmHandle, command);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_PythonCommandBool(SBMHANDLE sbmHandle, const char * command)
{
#ifdef WIN_BUILD
   if (g_SBM_PythonCommandBool)
   {
      return g_SBM_PythonCommandBool(sbmHandle, command);
   }
   return false;
#else
   return SBM_PythonCommandBool(sbmHandle, command);
#endif
}

VHWRAPPERDLL_API int WRAPPER_SBM_PythonCommandInt(SBMHANDLE sbmHandle, const char * command)
{
#ifdef WIN_BUILD
   if (g_SBM_PythonCommandInt)
   {
      return g_SBM_PythonCommandInt(sbmHandle, command);
   }
   return 0;
#else
   return SBM_PythonCommandInt(sbmHandle, command);
#endif
}

VHWRAPPERDLL_API float WRAPPER_SBM_PythonCommandFloat(SBMHANDLE sbmHandle, const char *command)
{
#ifdef WIN_BUILD
   if (g_SBM_PythonCommandFloat)
   {
      return g_SBM_PythonCommandFloat(sbmHandle, command);
   }
   return 0;
#else
    return SBM_PythonCommandFloat(sbmHandle, command);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_PythonCommandString(SBMHANDLE sbmHandle, const char * command, char * output, int maxLen)
{
#ifdef WIN_BUILD
   if (g_SBM_PythonCommandString)
   {
      g_SBM_PythonCommandString(sbmHandle, command, output, maxLen);
   }
#else
    SBM_PythonCommandString(sbmHandle, command, output, maxLen);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SBAssetManager_LoadSkeleton( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * skeletonName )
{
#ifdef WIN_BUILD
   if (g_SBM_SBAssetManager_LoadSkeleton)
   {
      return g_SBM_SBAssetManager_LoadSkeleton(sbmHandle, data, sizeBytes, skeletonName);
   }
   return false;
#else
   return SBM_SBAssetManager_LoadSkeleton(sbmHandle, data, sizeBytes, skeletonName);
#endif
}

VHWRAPPERDLL_API bool WRAPPER_SBM_SBAssetManager_LoadMotion( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * motionName )
{
#ifdef WIN_BUILD
   if (g_SBM_SBAssetManager_LoadMotion)
   {
      return g_SBM_SBAssetManager_LoadMotion(sbmHandle, data, sizeBytes, motionName);
   }
   return false;
#else
   return SBM_SBAssetManager_LoadMotion(sbmHandle, data, sizeBytes, motionName);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBDebuggerServer_SetID( SBMHANDLE sbmHandle, const char * id )
{
#ifdef WIN_BUILD
   if (g_SBM_SBDebuggerServer_SetID)
   {
      g_SBM_SBDebuggerServer_SetID(sbmHandle, id );
   }
#else
   SBM_SBDebuggerServer_SetID(sbmHandle, id);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBDebuggerServer_SetCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
#ifdef WIN_BUILD
   if (g_SBM_SBDebuggerServer_SetCameraValues)
   {
      g_SBM_SBDebuggerServer_SetCameraValues(sbmHandle, x, y, z, rx, ry, rz, rw, fov, aspect, zNear, zFar );
   }
#else
   SBM_SBDebuggerServer_SetCameraValues(sbmHandle, x, y, z, rx, ry, rz, rw, fov, aspect, zNear, zFar );
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBDebuggerServer_SetRendererIsRightHanded( SBMHANDLE sbmHandle, bool enabled )
{
#ifdef WIN_BUILD
   if (g_SBM_SBDebuggerServer_SetRendererIsRightHanded)
   {
      g_SBM_SBDebuggerServer_SetRendererIsRightHanded(sbmHandle, enabled );
   }
#else
   SBM_SBDebuggerServer_SetRendererIsRightHanded(sbmHandle, enabled );
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBMotion_AddChannel(SBMHANDLE sbmHandle, const char * motionName, const char * channelName, const char * channelType)
{
#ifdef WIN_BUILD
   if (g_SBM_SBMotion_AddChannel)
   {
      g_SBM_SBMotion_AddChannel(sbmHandle, motionName, channelName, channelType);
   }
#else
    SBM_SBMotion_AddChannel(sbmHandle, motionName, channelName, channelType);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBMotion_AddFrame(SBMHANDLE sbmHandle, const char * motionName, float frameTime, const float * frameData, int numFrameData)
{
#ifdef WIN_BUILD
   if (g_SBM_SBMotion_AddFrame)
   {
      g_SBM_SBMotion_AddFrame(sbmHandle, motionName, frameTime, frameData, numFrameData);
   }
#else
    SBM_SBMotion_AddFrame(sbmHandle, motionName, frameTime, frameData, numFrameData);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBMotion_SetSyncPoint(SBMHANDLE sbmHandle, const char * motionName, const char * syncTag, double time)
{
#ifdef WIN_BUILD
   if (g_SBM_SBMotion_SetSyncPoint)
   {
      g_SBM_SBMotion_SetSyncPoint(sbmHandle, motionName, syncTag, time);
   }
#else
    SBM_SBMotion_SetSyncPoint(sbmHandle, motionName, syncTag, time);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBJointMap_GetMapTarget(SBMHANDLE sbmHandle, const char * jointMap, const char * jointName, char * mappedJointName, int maxMappedJointName)
{
#ifdef WIN_BUILD
   if (g_SBM_SBJointMap_GetMapTarget)
   {
      g_SBM_SBJointMap_GetMapTarget(sbmHandle, jointMap, jointName, mappedJointName, maxMappedJointName);
   }
#else
    SBM_SBJointMap_GetMapTarget(sbmHandle, jointMap, jointName, mappedJointName, maxMappedJointName);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBVHMsgManager_SetServer( SBMHANDLE sbmHandle, const char * server )
{
#ifdef WIN_BUILD
   if (g_SBM_SBVHMsgManager_SetServer)
   {
      g_SBM_SBVHMsgManager_SetServer(sbmHandle, server);
   }
#else
    SBM_SBVHMsgManager_SetServer(sbmHandle, server);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBVHMsgManager_SetScope( SBMHANDLE sbmHandle, const char * scope )
{
#ifdef WIN_BUILD
   if (g_SBM_SBVHMsgManager_SetScope)
   {
      g_SBM_SBVHMsgManager_SetScope(sbmHandle, scope);
   }
#else
    SBM_SBVHMsgManager_SetScope(sbmHandle, scope);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBVHMsgManager_SetPort( SBMHANDLE sbmHandle, const char * port )
{
#ifdef WIN_BUILD
   if (g_SBM_SBVHMsgManager_SetPort)
   {
      g_SBM_SBVHMsgManager_SetPort(sbmHandle, port);
   }
#else
    SBM_SBVHMsgManager_SetPort(sbmHandle, port);
#endif
}

VHWRAPPERDLL_API void WRAPPER_SBM_SBVHMsgManager_SetEnable( SBMHANDLE sbmHandle, bool enable )
{
#ifdef WIN_BUILD
   if (g_SBM_SBVHMsgManager_SetEnable)
   {
      g_SBM_SBVHMsgManager_SetEnable(sbmHandle, enable);
   }
#else
    SBM_SBVHMsgManager_SetEnable(sbmHandle, enable);
#endif
}

////////////////////////////////////////////////////////////////////////////


#if !defined(MAC_BUILD) && !defined(IPHONE_BUILD) && !defined(ANDROID_BUILD)

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

   g_audioInstances[handle]->PauseAllSounds();

}


VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_StopAllSounds(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return;
   }

   g_audioInstances[handle]->StopAllSounds();

}

VHWRAPPERDLL_API void WRAPPER_VHCL_AUDIO_UnpauseAllSounds(AUDIOHANDLE handle)
{
   if (!VHCL_AUDIO_HandleExists(handle))
   {
      return;
   }

   g_audioInstances[handle]->UnpauseAllSounds();

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

#endif  // !defined(MAC_BUILD)


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



// stubs for testing library loading on different platforms
#if 0
#include "vhwrapper.h"


SBMHANDLE WRAPPER_SBM_CreateSBM(const bool releaseMode)
{
   return 42;
   //return SBM_CreateSBM();
}

#endif
