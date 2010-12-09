
#ifndef SMARTBODY_C_DLL_H
#define SMARTBODY_C_DLL_H


#ifdef SMARTBODY_C_DLL_EXPORTS
#define SMARTBODY_C_DLL_API __declspec(dllexport)
#else
#define SMARTBODY_C_DLL_API __declspec(dllimport)
#endif


#include "vhcl_public.h"


typedef intptr_t SBMHANDLE;


// Listener callbacks to get Smartbody related notifications
typedef int (__stdcall *SBM_OnCreateCharacterCallback)( SBMHANDLE sbmHandle, const char * name, const char * objectClass );
typedef int (__stdcall *SBM_OnCharacterDeleteCallback)( SBMHANDLE sbmHandle, const char * name );
typedef int (__stdcall *SBM_OnVisemeCallback)( SBMHANDLE sbmHandle, const char * name, const char * visemeName, float weight, float blendTime );


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

SMARTBODY_C_DLL_API bool SBM_Init( SBMHANDLE sbmHandle );
SMARTBODY_C_DLL_API bool SBM_Shutdown( SBMHANDLE sbmHandle );

SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnVisemeCallback visemeCB );

SMARTBODY_C_DLL_API bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds );

SMARTBODY_C_DLL_API bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args );

SMARTBODY_C_DLL_API int  SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle );
SMARTBODY_C_DLL_API bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character );
SMARTBODY_C_DLL_API bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character );


#endif  // SMARTBODY_C_DLL_H
