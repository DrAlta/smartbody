
#ifndef SMARTBODY_C_DLL_H
#define SMARTBODY_C_DLL_H

#if WIN32
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
#include <map>

typedef intptr_t SBMHANDLE;

typedef void (__stdcall *LogMessageCallback)(const char* message, int messageType); //0 = normal, 1 = error, 2 = warning


// Listener callbacks to get Smartbody related notifications
typedef int (__stdcall *SBM_OnCreateCharacterCallback)( SBMHANDLE sbmHandle, const char * name, const char * objectClass );
typedef int (__stdcall *SBM_OnCharacterDeleteCallback)( SBMHANDLE sbmHandle, const char * name );
typedef int (__stdcall *SBM_OnCharacterChangeCallback)( SBMHANDLE sbmHandle, const char * name );
typedef int (__stdcall *SBM_OnVisemeCallback)( SBMHANDLE sbmHandle, const char * name, const char * visemeName, float weight, float blendTime );
typedef int (__stdcall *SBM_OnChannelCallback)( SBMHANDLE sbmHandle, const char * name, const char * channelName, float value );

struct SBM_CallbackInfo
{
    char * name;
    char * objectClass;
    char * visemeName;
    float weight;
    float blendTime;

   SBM_CallbackInfo()
   {
      name = NULL;
      objectClass = NULL;
      visemeName = NULL;
   }

   ~SBM_CallbackInfo()
   {
      if (name)
      {
         delete name;
         name = NULL;
      }
      if (objectClass)
      {
         delete objectClass;
         objectClass = NULL;
      }
      if (visemeName)
      {
         delete visemeName;
         visemeName = NULL;
      }
   }
};

#ifdef __cplusplus
extern "C" {
#endif 


// helper class for receiving individual joint data
struct SBM_SmartbodyJoint
{
   char * m_name;
   float x;
   float y;
   float z;
   float rw;
   float rx;
   float ry;
   float rz;
};


// helper class for receiving character data including all the joints
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
   SBM_SmartbodyJoint * m_joints;
};


SMARTBODY_C_DLL_API SBMHANDLE SBM_CreateSBM();

SMARTBODY_C_DLL_API bool SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath );
SMARTBODY_C_DLL_API bool SBM_SetFacebone( SBMHANDLE sbmHandle, bool enabled );
SMARTBODY_C_DLL_API bool SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId );
SMARTBODY_C_DLL_API bool SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path );

SMARTBODY_C_DLL_API bool SBM_Init( SBMHANDLE sbmHandle, const char* pythonLibPath );
SMARTBODY_C_DLL_API bool SBM_Shutdown( SBMHANDLE sbmHandle );

SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changedCB, SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB );

SMARTBODY_C_DLL_API bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds );

SMARTBODY_C_DLL_API void SBM_SetCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );

SMARTBODY_C_DLL_API bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args );

SMARTBODY_C_DLL_API int  SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle );
SMARTBODY_C_DLL_API bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_ReleaseCharacterJoints( SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_SetLogMessageCallback(LogMessageCallback cb);
SMARTBODY_C_DLL_API void SBM_LogMessage(const char* message, int messageType);

// used for polling on iOS since callbacks aren't allowed
SMARTBODY_C_DLL_API bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** objectClass );
SMARTBODY_C_DLL_API bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, int * numCharacters, char *** name );
SMARTBODY_C_DLL_API bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, int * numCharacters, char *** name );
SMARTBODY_C_DLL_API bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** visemeName, float** weight, float** blendTime );
SMARTBODY_C_DLL_API bool SBM_IsChannelSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** channelName, float ** value );

// helper functions
void DeleteCallbacks(SBMHANDLE sbmHandle, std::map< int, std::vector<SBM_CallbackInfo*> >& callbackData);

#ifdef __cplusplus
}
#endif

#endif  // SMARTBODY_C_DLL_H
