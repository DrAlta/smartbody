
#include "vhcl.h"

#include "smartbody-c-dll.h"


#include <fstream>
#include <ios>
#include <string.h>

#include "smartbody-dll.h"


using std::string;

std::map< int, std::vector<SBM_CallbackInfo*> > g_CreateCallbackInfo;
std::map< int, std::vector<SBM_CallbackInfo*> > g_DeleteCallbackInfo;
std::map< int, std::vector<SBM_CallbackInfo*> > g_ChangeCallbackInfo;
std::map< int, std::vector<SBM_CallbackInfo*> > g_VisemeCallbackInfo;
std::map< int, std::vector<SBM_CallbackInfo*> > g_ChannelCallbackInfo;

LogMessageCallback LogMessageFunc = NULL;

class LogMessageListener : public vhcl::Log::Listener
{
public:
   LogMessageListener() {}
   ~LogMessageListener() {}

   virtual void OnMessage( const std::string & message )
   {
      int messageType = 0;
      if (message.find("WARNING") != std::string::npos)
      {
         messageType = 2;
      }
      else if (message.find("ERROR") != std::string::npos)
      {
         messageType = 1;
      }
      SBM_LogMessage(message.c_str(), messageType);
   }
};
LogMessageListener* g_pLogMessageListener = NULL;


SMARTBODY_C_DLL_API bool SBM_SetLogMessageCallback(LogMessageCallback cb)
{
   LogMessageFunc = cb;

   if (g_pLogMessageListener == NULL)
   {
      g_pLogMessageListener = new LogMessageListener();
      vhcl::Log::g_log.AddListener(g_pLogMessageListener);
      return true;
   }

   return false;
}


SMARTBODY_C_DLL_API void SBM_LogMessage(const char* message, int messageType)
{
   // 0 = normal, 1 = error, 2 = warning
   if (LogMessageFunc)
   {
      LogMessageFunc(message, messageType);
   }
}

class SBM_SmartbodyListener : public SmartbodyListener
{
private:
   SBMHANDLE m_sbmHandle;
   SBM_OnCreateCharacterCallback m_createCharacterCallback;
   SBM_OnCharacterDeleteCallback m_deleteCharacterCallback;
   SBM_OnCharacterChangeCallback m_changeCharacterCallback;
   SBM_OnVisemeCallback m_viseme;
   SBM_OnChannelCallback m_channel;

public:
   SBM_SmartbodyListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCharCallback, SBM_OnCharacterDeleteCallback deleteCharCallback, SBM_OnCharacterChangeCallback changeCharCallback, SBM_OnVisemeCallback visemeCallback, SBM_OnChannelCallback channelCallback )
   {
      m_sbmHandle = sbmHandle;
      m_createCharacterCallback = createCharCallback;
      m_deleteCharacterCallback = deleteCharCallback;
      m_changeCharacterCallback = changeCharCallback;
      m_viseme = visemeCallback;
	  m_channel = channelCallback;
   }

   virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass )
   {
#if !defined(IPHONE_BUILD)
         m_createCharacterCallback( m_sbmHandle, name.c_str(), objectClass.c_str() );
#else
          SBM_CallbackInfo* info = new SBM_CallbackInfo();
          info->name = new char[name.length() + 1];
          strcpy(info->name, name.c_str());
          
          info->objectClass = new char[objectClass.length() + 1];
          strcpy(info->objectClass, objectClass.c_str());
          g_CreateCallbackInfo[m_sbmHandle].push_back(info);
#endif
   }

   virtual void OnCharacterDelete( const std::string & name )
   {
#if !defined(IPHONE_BUILD)
      m_deleteCharacterCallback( m_sbmHandle, name.c_str() );
#else
      SBM_CallbackInfo* info = new SBM_CallbackInfo();
      info->name = new char[name.length() + 1];
      strcpy(info->name, name.c_str());
      g_DeleteCallbackInfo[m_sbmHandle].push_back(info);
#endif
   }

   virtual void OnCharacterChange( const std::string & name )
   {
#if !defined(IPHONE_BUILD)
      m_changeCharacterCallback( m_sbmHandle, name.c_str() );
#else
      SBM_CallbackInfo* info = new SBM_CallbackInfo();
      info->name = new char[name.length() + 1];
      strcpy(info->name, name.c_str());
      g_ChangeCallbackInfo[m_sbmHandle].push_back(info);
#endif
   }

   virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
   {
#if !defined(IPHONE_BUILD)
      m_viseme( m_sbmHandle, name.c_str(), visemeName.c_str(), weight, blendTime );
#else
      SBM_CallbackInfo* info = new SBM_CallbackInfo();
      info->name = new char[name.length() + 1];
      strcpy(info->name, name.c_str());

      info->visemeName = new char[visemeName.length() + 1];
      strcpy(info->visemeName, visemeName.c_str());

      info->weight = weight;
      info->blendTime = blendTime;

      g_VisemeCallbackInfo[m_sbmHandle].push_back(info);
#endif
   }

   virtual void OnChannel( const std::string & name, const std::string & channelName, const float value )
   {
#if !defined(IPHONE_BUILD)
      m_channel( m_sbmHandle, name.c_str(), channelName.c_str(), value );
#else
      SBM_CallbackInfo* info = new SBM_CallbackInfo();
      info->name = new char[name.length() + 1];
      strcpy(info->name, name.c_str());

      info->visemeName = new char[channelName.length() + 1];
      strcpy(info->visemeName, channelName.c_str());

      info->weight = value;

      g_ChannelCallbackInfo[m_sbmHandle].push_back(info);
#endif
   }
};


bool SBM_HandleExists( SBMHANDLE sbmHandle );
void SBM_CharToCSbmChar( const SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter * sbmCChar );
void SBM_CharToCSbmChar2( const SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter2 * sbmCChar );


std::map< int, Smartbody_dll * > g_smartbodyInstances;
int g_handleId_DLL = 0;
std::map< int, SBM_SmartbodyCharacter * > g_characters;


SMARTBODY_C_DLL_API SBMHANDLE SBM_CreateSBM()
{
   g_handleId_DLL++;
   g_smartbodyInstances[ g_handleId_DLL ] = new Smartbody_dll();
   return g_handleId_DLL;
}


SMARTBODY_C_DLL_API bool SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   g_smartbodyInstances[ sbmHandle ]->SetSpeechAudiofileBasePath( basePath );
   return true;
}

SMARTBODY_C_DLL_API bool SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   g_smartbodyInstances[ sbmHandle ]->SetProcessId( processId );
   return true;
}


SMARTBODY_C_DLL_API bool SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   g_smartbodyInstances[ sbmHandle ]->SetMediaPath( path );
   return true;
}


SMARTBODY_C_DLL_API bool SBM_Init( SBMHANDLE sbmHandle, const char* pythonLibPath, bool logToFile )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->Init(pythonLibPath, logToFile);
}


SMARTBODY_C_DLL_API bool SBM_Shutdown( SBMHANDLE sbmHandle )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   std::map< int, Smartbody_dll * >::iterator it = g_smartbodyInstances.find( sbmHandle );
   Smartbody_dll * sbm = g_smartbodyInstances[ sbmHandle ];
   g_smartbodyInstances.erase( it );
   bool retVal = sbm->Shutdown();
   delete sbm;

   // release the logger
   if (g_pLogMessageListener)
   {
      vhcl::Log::g_log.RemoveListener(g_pLogMessageListener);
      delete g_pLogMessageListener;
      g_pLogMessageListener = NULL;
   }

   return retVal;
}


SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changedCB, SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SBM_SmartbodyListener * listener = new SBM_SmartbodyListener( sbmHandle, createCB, deleteCB, changedCB, visemeCB, channelCB );
   g_smartbodyInstances[ sbmHandle ]->SetListener( listener );
   return true;
}


SMARTBODY_C_DLL_API bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->Update( timeInSeconds );
}


SMARTBODY_C_DLL_API void SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   g_smartbodyInstances[ sbmHandle ]->SetDebuggerId( id );
}


SMARTBODY_C_DLL_API void SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   g_smartbodyInstances[ sbmHandle ]->SetDebuggerCameraValues( x, y, z, rx, ry, rz, rw, fov, aspect, zNear, zFar );
}


SMARTBODY_C_DLL_API void SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   g_smartbodyInstances[ sbmHandle ]->SetDebuggerRendererRightHanded( enabled );
}


SMARTBODY_C_DLL_API bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->ProcessVHMsgs( op, args );
}

SMARTBODY_C_DLL_API bool SBM_ExecutePython( SBMHANDLE sbmHandle, const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->ExecutePython( command );
}


SMARTBODY_C_DLL_API int SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return -1;
   }

   return g_smartbodyInstances[ sbmHandle ]->GetNumberOfCharacters();
}


SMARTBODY_C_DLL_API bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SmartbodyCharacter& dllChar = g_smartbodyInstances[ sbmHandle ]->GetCharacter( (string)name );

   SBM_CharToCSbmChar( &dllChar, character );

   return true;
}


SMARTBODY_C_DLL_API bool SBM_GetCharacter2( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter2 * character )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SmartbodyCharacter& dllChar = g_smartbodyInstances[ sbmHandle ]->GetCharacter( (string)name );

   SBM_CharToCSbmChar2( &dllChar, character );

   return true;
}


SMARTBODY_C_DLL_API bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character )
{
   if ( !character )
   {
      return false;
   }

   SBM_ReleaseCharacterJoints(character);
   
   delete [] character->m_name;

   return true;
}


SMARTBODY_C_DLL_API bool SBM_ReleaseCharacterJoints( SBM_SmartbodyCharacter * character )
{
   if ( !character )
   {
      return false;
   }

   for ( size_t i = 0; i < character->m_numJoints; i++ )
   {
      delete [] character->m_joints[ i ].m_name;
   }

   delete [] character->m_joints;
   return true;
}


bool SBM_HandleExists( SBMHANDLE sbmHandle )
{
   return g_smartbodyInstances.find( sbmHandle ) != g_smartbodyInstances.end();
}


void SBM_CharToCSbmChar( const::SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter * sbmCChar )
{
   // copy transformation data
   sbmCChar->x = sbmChar->x;
   sbmCChar->y = sbmChar->y;
   sbmCChar->z = sbmChar->z;
   sbmCChar->rw = sbmChar->rw;
   sbmCChar->rx = sbmChar->rx;
   sbmCChar->ry = sbmChar->ry;
   sbmCChar->rz = sbmChar->rz;


   // copy name
   // NOTE: name should be copied during callback and character creation
   //sbmCChar->m_name = new char[ sbmChar->m_name.length() + 1 ];
   //strcpy( sbmCChar->m_name, sbmChar->m_name.c_str() );


   if ( sbmChar->m_joints.size() > 0 )
   {
      bool initJoints = false;
      if (sbmCChar->m_numJoints == 0)
      {
         //SBM_LogMessage("CREATING JOINTS!", 2);
         sbmCChar->m_numJoints = sbmChar->m_joints.size();
         sbmCChar->m_joints = new SBM_SmartbodyJoint[ sbmCChar->m_numJoints ];
         initJoints = true;
      }

      for ( size_t i = 0; i < sbmCChar->m_numJoints; i++ )
      {
         // copy transformation data
         sbmCChar->m_joints[ i ].x = sbmChar->m_joints[ i ].x;
         sbmCChar->m_joints[ i ].y = sbmChar->m_joints[ i ].y;
         sbmCChar->m_joints[ i ].z = sbmChar->m_joints[ i ].z;
         sbmCChar->m_joints[ i ].rw = sbmChar->m_joints[ i ].rw;
         sbmCChar->m_joints[ i ].rx = sbmChar->m_joints[ i ].rx;
         sbmCChar->m_joints[ i ].ry = sbmChar->m_joints[ i ].ry;
         sbmCChar->m_joints[ i ].rz = sbmChar->m_joints[ i ].rz;

         // copy name
         if (initJoints)
         {
            // only initialize joints if this is the first time
            sbmCChar->m_joints[ i ].m_name = new char[ sbmChar->m_joints[ i ].m_name.length() + 1 ];
            strcpy( sbmCChar->m_joints[ i ].m_name, sbmChar->m_joints[ i ].m_name.c_str() );
         }
      }
   }
}


void SBM_CharToCSbmChar2( const::SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter2 * sbmCChar )
{
   // copy transformation data
   sbmCChar->x = sbmChar->x;
   sbmCChar->y = sbmChar->y;
   sbmCChar->z = sbmChar->z;
   sbmCChar->rw = sbmChar->rw;
   sbmCChar->rx = sbmChar->rx;
   sbmCChar->ry = sbmChar->ry;
   sbmCChar->rz = sbmChar->rz;


   // copy name
   // NOTE: name should be copied during callback and character creation
   //sbmCChar->m_name = new char[ sbmChar->m_name.length() + 1 ];
   //strcpy( sbmCChar->m_name, sbmChar->m_name.c_str() );

   if ( sbmChar->m_joints.size() > 0 )
   {
      bool initJoints = false;
      if (sbmCChar->m_numJoints == 0)
      {
         //SBM_LogMessage("CREATING JOINTS!", 2);
         sbmCChar->m_numJoints = sbmChar->m_joints.size();
         //sbmCChar->m_joints = new SBM_SmartbodyJoint[ sbmCChar->m_numJoints ];
         sbmCChar->jname = new char * [ sbmCChar->m_numJoints ];
         sbmCChar->jx = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jy = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jz = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jrw = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jrx = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jry = new float [ sbmCChar->m_numJoints ];
         sbmCChar->jrz = new float [ sbmCChar->m_numJoints ];
         initJoints = true;
      }

      for ( size_t i = 0; i < sbmCChar->m_numJoints; i++ )
      {
         // copy transformation data
         //sbmCChar->m_joints[ i ].x = sbmChar->m_joints[ i ].x;
         //sbmCChar->m_joints[ i ].y = sbmChar->m_joints[ i ].y;
         //sbmCChar->m_joints[ i ].z = sbmChar->m_joints[ i ].z;
         //sbmCChar->m_joints[ i ].rw = sbmChar->m_joints[ i ].rw;
         //sbmCChar->m_joints[ i ].rx = sbmChar->m_joints[ i ].rx;
         //sbmCChar->m_joints[ i ].ry = sbmChar->m_joints[ i ].ry;
         //sbmCChar->m_joints[ i ].rz = sbmChar->m_joints[ i ].rz;
         sbmCChar->jx[ i ] = sbmChar->m_joints[ i ].x;
         sbmCChar->jy[ i ] = sbmChar->m_joints[ i ].y;
         sbmCChar->jz[ i ] = sbmChar->m_joints[ i ].z;
         sbmCChar->jrw[ i ] = sbmChar->m_joints[ i ].rw;
         sbmCChar->jrx[ i ] = sbmChar->m_joints[ i ].rx;
         sbmCChar->jry[ i ] = sbmChar->m_joints[ i ].ry;
         sbmCChar->jrz[ i ] = sbmChar->m_joints[ i ].rz;

         // copy name
         if (initJoints)
         {
            // only initialize joints if this is the first time
            //sbmCChar->m_joints[ i ].m_name = new char[ sbmChar->m_joints[ i ].m_name.length() + 1 ];
            //strcpy( sbmCChar->m_joints[ i ].m_name, sbmChar->m_joints[ i ].m_name.c_str() );
            sbmCChar->jname[ i ] = new char[ sbmChar->m_joints[ i ].m_name.length() + 1 ];
            strcpy( sbmCChar->jname[ i ], sbmChar->m_joints[ i ].m_name.c_str() );
         }
      }
   }
}

SMARTBODY_C_DLL_API bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** objectClass )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_CreateCallbackInfo[sbmHandle].size() == 0)
    {
        return false;
    }
    
    *numCharacters = g_CreateCallbackInfo[sbmHandle].size();
    (*name) = new char*[*numCharacters];
    (*objectClass) = new char*[*numCharacters];
    for (unsigned int i = 0; i < g_CreateCallbackInfo[sbmHandle].size(); i++)
    {
         int length = strlen(g_CreateCallbackInfo[sbmHandle][i]->name) + 1;
         (*name)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*name)[i], length, g_CreateCallbackInfo[sbmHandle][i]->name);
#else
		 strncpy((*name)[i], g_CreateCallbackInfo[sbmHandle][i]->name, length);
#endif
         length = strlen(g_CreateCallbackInfo[sbmHandle][i]->objectClass) + 1;
         (*objectClass)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*objectClass)[i], length, g_CreateCallbackInfo[sbmHandle][i]->objectClass);  
#else
		 strncpy((*objectClass)[i], g_CreateCallbackInfo[sbmHandle][i]->objectClass, length);
#endif
    }

    DeleteCallbacks(sbmHandle, g_CreateCallbackInfo);
    return true;
}

SMARTBODY_C_DLL_API bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, int * numCharacters, char *** name)
{
    if ( !SBM_HandleExists( sbmHandle ) || g_DeleteCallbackInfo[sbmHandle].size() == 0)
    {
        return false;
    }

    *numCharacters = g_DeleteCallbackInfo[sbmHandle].size();
    (*name) = new char*[*numCharacters];
    for (unsigned int i = 0; i < g_DeleteCallbackInfo[sbmHandle].size(); i++)
    {
         int length = strlen(g_DeleteCallbackInfo[sbmHandle][i]->name) + 1;
         (*name)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*name)[i], length, g_DeleteCallbackInfo[sbmHandle][i]->name); 
#else
		 strncpy((*name)[i], g_DeleteCallbackInfo[sbmHandle][i]->name, length);
#endif
    }
    
    DeleteCallbacks(sbmHandle, g_DeleteCallbackInfo);
    return true;
}

SMARTBODY_C_DLL_API bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, int * numCharacters, char *** name)
{
    if ( !SBM_HandleExists( sbmHandle ) || g_ChangeCallbackInfo[sbmHandle].size() == 0)
    {
        return false;
    }

    *numCharacters = g_ChangeCallbackInfo[sbmHandle].size();
    (*name) = new char*[*numCharacters];
    for (unsigned int i = 0; i < g_ChangeCallbackInfo[sbmHandle].size(); i++)
    {
         int length = strlen(g_ChangeCallbackInfo[sbmHandle][i]->name) + 1;
         (*name)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*name)[i], length, g_ChangeCallbackInfo[sbmHandle][i]->name);
#else
		 strncpy((*name)[i], g_ChangeCallbackInfo[sbmHandle][i]->name, length);
#endif
    }

    DeleteCallbacks(sbmHandle, g_ChangeCallbackInfo);

    return true;
}

SMARTBODY_C_DLL_API bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** visemeName, float** weight, float ** blendTime)
{
    if ( !SBM_HandleExists( sbmHandle ) || g_VisemeCallbackInfo[sbmHandle].size() == 0)
    {
        return false;
    }

    *numCharacters = g_VisemeCallbackInfo[sbmHandle].size();
    (*name) = new char*[*numCharacters];
    (*visemeName) = new char*[*numCharacters];
    (*weight) = new float[*numCharacters];
    (*blendTime) = new float[*numCharacters];
    for (unsigned int i = 0; i < g_VisemeCallbackInfo[sbmHandle].size(); i++)
    {
         int length = strlen(g_VisemeCallbackInfo[sbmHandle][i]->name) + 1;
         (*name)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*name)[i], length, g_VisemeCallbackInfo[sbmHandle][i]->name); 
#else
		 strncpy((*name)[i], g_VisemeCallbackInfo[sbmHandle][i]->name, length); 
#endif
         length = strlen(g_VisemeCallbackInfo[sbmHandle][i]->visemeName) + 1;
         (*visemeName)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*visemeName)[i], length, g_VisemeCallbackInfo[sbmHandle][i]->visemeName);  
#else
		 strncpy((*visemeName)[i], g_VisemeCallbackInfo[sbmHandle][i]->visemeName, length);  
#endif
         (*weight)[i] = g_VisemeCallbackInfo[sbmHandle][i]->weight;
         (*blendTime)[i] = g_VisemeCallbackInfo[sbmHandle][i]->blendTime;
    }

    DeleteCallbacks(sbmHandle, g_VisemeCallbackInfo);

    return true;
}

SMARTBODY_C_DLL_API bool SBM_IsChannelSet( SBMHANDLE sbmHandle, int * numCharacters, char *** name, char *** channelName, float ** value)
{
    if ( !SBM_HandleExists( sbmHandle ) || g_ChannelCallbackInfo[sbmHandle].size() == 0)
    {
        return false;
    }

    *numCharacters = g_ChannelCallbackInfo[sbmHandle].size();
    (*name) = new char*[*numCharacters];
    (*channelName) = new char*[*numCharacters];
    (*value) = new float[*numCharacters];
    for (unsigned int i = 0; i < g_ChannelCallbackInfo[sbmHandle].size(); i++)
    {
         int length = strlen(g_ChannelCallbackInfo[sbmHandle][i]->name) + 1;
         (*name)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*name)[i], length, g_ChannelCallbackInfo[sbmHandle][i]->name); 
#else
		 strncpy((*name)[i], g_ChannelCallbackInfo[sbmHandle][i]->name, length); 
#endif
         // channel name is stored in visemeName
         length = strlen(g_ChannelCallbackInfo[sbmHandle][i]->visemeName) + 1;
         (*channelName)[i] = new char[length];
#ifdef _WIN32
         strcpy_s((*channelName)[i], length, g_ChannelCallbackInfo[sbmHandle][i]->visemeName);  
#else
		 strncpy((*channelName)[i], g_ChannelCallbackInfo[sbmHandle][i]->visemeName, length); 
#endif
         (*value)[i] = g_ChannelCallbackInfo[sbmHandle][i]->weight;
    }

    DeleteCallbacks(sbmHandle, g_ChannelCallbackInfo);

    return true;
}


void DeleteCallbacks(SBMHANDLE sbmHandle, std::map< int, std::vector<SBM_CallbackInfo*> >& callbackData)
{
    if ( !SBM_HandleExists( sbmHandle ) )
    {
        return;
    }

    for (unsigned int i = 0; i < callbackData.size(); i++)
    {
         delete callbackData[sbmHandle][i]; // char* memory is deleted in the destructor
    }

    callbackData.clear();
}

SMARTBODY_C_DLL_API bool SBM_PythonCommandVoid( SBMHANDLE sbmHandle, const char * command)
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->PythonCommandVoid( command );
}

SMARTBODY_C_DLL_API bool SBM_PythonCommandBool( SBMHANDLE sbmHandle,  const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->PythonCommandBool( command );
}

SMARTBODY_C_DLL_API int SBM_PythonCommandInt( SBMHANDLE sbmHandle,  const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

   return g_smartbodyInstances[ sbmHandle ]->PythonCommandInt( command );
}

SMARTBODY_C_DLL_API float SBM_PythonCommandFloat( SBMHANDLE sbmHandle,  const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

   return g_smartbodyInstances[ sbmHandle ]->PythonCommandFloat( command );
}

SMARTBODY_C_DLL_API char* SBM_PythonCommandString( SBMHANDLE sbmHandle,  const char * command, char* output, int maxLen)
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

   std::string temp = g_smartbodyInstances[ sbmHandle ]->PythonCommandString( command ).c_str();
   #ifdef WIN32
   strcpy_s(output, maxLen, temp.c_str());
   #else
   strncpy(output, temp.c_str(), maxLen);
   #endif
   return output;
}

