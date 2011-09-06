
#include "vhcl.h"

#include "smartbody-c-dll.h"

#include <map>
#include <fstream>
#include <ios>
#include <string.h>

#include "smartbody-dll.h"


using std::string;

std::map< int, std::vector<SBM_CallbackInfo*> > g_CallbackInfo;
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

public:
   SBM_SmartbodyListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCharCallback, SBM_OnCharacterDeleteCallback deleteCharCallback, SBM_OnCharacterChangeCallback changeCharCallback, SBM_OnVisemeCallback visemeCallback )
   {
      m_sbmHandle = sbmHandle;
      m_createCharacterCallback = createCharCallback;
      m_deleteCharacterCallback = deleteCharCallback;
      m_changeCharacterCallback = changeCharCallback;
      m_viseme = visemeCallback;
   }

   virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass )
   {
#ifdef WIN32
         m_createCharacterCallback( m_sbmHandle, name.c_str(), objectClass.c_str() );
#else
          SBM_CallbackInfo* info = new SBM_CallbackInfo();
          info->name = new char[name.length() + 1];
          strcpy(info->name, name.c_str());
          
          info->objectClass = new char[objectClass.length() + 1];
          strcpy(info->objectClass, objectClass.c_str());
          g_CallbackInfo[m_sbmHandle].push_back(info);
#endif
   }

   virtual void OnCharacterDelete( const std::string & name )
   {
      m_deleteCharacterCallback( m_sbmHandle, name.c_str() );
   }

   virtual void OnCharacterChange( const std::string & name )
   {
      m_changeCharacterCallback( m_sbmHandle, name.c_str() );
   }

   virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
   {
      m_viseme( m_sbmHandle, name.c_str(), visemeName.c_str(), weight, blendTime );
   }
};


bool SBM_HandleExists( SBMHANDLE sbmHandle );
void SBM_CharToCSbmChar( const SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter * sbmCChar );


std::map< int, Smartbody_dll * > g_smartbodyInstances;
int g_handleId = 0;
std::map< int, SBM_SmartbodyCharacter * > g_characters;


SMARTBODY_C_DLL_API SBMHANDLE SBM_CreateSBM()
{
   g_handleId++;
   g_smartbodyInstances[ g_handleId ] = new Smartbody_dll();
   return g_handleId;
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


SMARTBODY_C_DLL_API bool SBM_SetFacebone( SBMHANDLE sbmHandle, bool enabled )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   g_smartbodyInstances[ sbmHandle ]->SetFacebone( enabled );
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


SMARTBODY_C_DLL_API bool SBM_Init( SBMHANDLE sbmHandle )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->Init();
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


SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changedCB, SBM_OnVisemeCallback visemeCB )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SBM_SmartbodyListener * listener = new SBM_SmartbodyListener( sbmHandle, createCB, deleteCB, changedCB, visemeCB );
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


SMARTBODY_C_DLL_API bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   return g_smartbodyInstances[ sbmHandle ]->ProcessVHMsgs( op, args );
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


SMARTBODY_C_DLL_API bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, int * numCharacters, const char * name, const char * objectClass )
{
    if ( !SBM_HandleExists( sbmHandle ) )
    {
        return false;
    }
    
    for (unsigned int i = 0; i < g_CallbackInfo[sbmHandle].size(); i++)
    {
        //if (strncasecmp(name, g_CallbackInfo[sbmHandle][i]->name, strlen(name) == 0
        //    && strncasecmp(objectClass, g_CallbackInfo[sbmHandle][i]->objectClass, strlen(objectClass))))
        {
            name = new char[strlen(g_CallbackInfo[sbmHandle][i]->name) + 1];
            objectClass = new char[strlen(g_CallbackInfo[sbmHandle][i]->objectClass) + 1];

            delete g_CallbackInfo[sbmHandle][i];
            g_CallbackInfo[sbmHandle].erase(g_CallbackInfo[sbmHandle].begin() + i);
            return true;
        }
    }
    
    return false;
}

SMARTBODY_C_DLL_API bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, const char * name)
{
    if ( !SBM_HandleExists( sbmHandle ) )
    {
        return false;
    }
    
    return false;
}

SMARTBODY_C_DLL_API bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, const char * name)
{
    if ( !SBM_HandleExists( sbmHandle ) )
    {
        return false;
    }
    return false;
}

SMARTBODY_C_DLL_API bool SBM_VisemeSet( SBMHANDLE sbmHandle, const char * name, const char * visemeName, float weight, float blendTime)
{
    if ( !SBM_HandleExists( sbmHandle ) )
    {
        return false;
    }
    return false;
}