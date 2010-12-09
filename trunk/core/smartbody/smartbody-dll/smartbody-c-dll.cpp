
#include "vhcl.h"

#include "smartbody-c-dll.h"

#include <map>
#include <fstream>
#include <ios>

#include "smartbody-dll.h"


using std::string;


class SBM_SmartbodyListener : public SmartbodyListener
{
private:
   SBMHANDLE m_sbmHandle;
   SBM_OnCreateCharacterCallback m_createCharacterCallback;
   SBM_OnCharacterDeleteCallback m_deleteCharacterCallback;
   SBM_OnVisemeCallback m_viseme;

public:
   SBM_SmartbodyListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCharCallback, SBM_OnCharacterDeleteCallback deleteCharCallback, SBM_OnVisemeCallback visemeCallback )
   {
      m_sbmHandle = sbmHandle;
      m_createCharacterCallback = createCharCallback;
      m_deleteCharacterCallback = deleteCharCallback;
      m_viseme = visemeCallback;
   }

   virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass )
   {
#if 0
      std::ofstream myfile;
      try
      {
         myfile.open( "example.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app );
         myfile << "loading character: " << name.c_str() << '\n';
#endif

         m_createCharacterCallback( m_sbmHandle, name.c_str(), objectClass.c_str() );

#if 0
         myfile << "returned: " << hr << '\n';
         myfile << "finished loading character: " << name.c_str()  << '\n';
      }
      catch ( std::exception e )
      {
         myfile << "OnCharacterDelete Caught an error: " << e.what() << '\n';
      }

      myfile.close();
#endif
   }

   virtual void OnCharacterDelete( const std::string & name )
   {
#if 0
      std::ofstream myfile;
      try
      {
         myfile.open( "example.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app );
         myfile << "deleting character: " << name.c_str() << '\n';
#endif

         m_deleteCharacterCallback( m_sbmHandle, name.c_str() );

#if 0
         myfile << "finished deleting character: " << name.c_str()  << '\n';
      }
      catch ( std::exception & e )
      {
         myfile << "OnCharacterDelete Caught an error: " << e.what() << '\n';
      }

      myfile.close();
#endif
   }

   virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
   {
#if 0
      std::ofstream myfile;
      try
      {
         myfile.open( "example.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app );
         myfile << "OnViseme: " << name.c_str() << '\n';
#endif

         m_viseme( m_sbmHandle, name.c_str(), visemeName.c_str(), weight, blendTime );

#if 0
         myfile << "returned: " << hr << '\n';
         myfile << "finished OnViseme: " << name.c_str()  << '\n';
      }
      catch ( std::exception & e )
      {
         myfile << "OnViseme Caught an error: " << e.what() << '\n';
      }

      myfile.close();
#endif
   }
};


// prototypes for local functions

bool SBM_HandleExists( SBMHANDLE sbmHandle );
void SBM_CharToCSbmChar( const SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter * sbmCChar );


std::map< int, Smartbody_dll * > g_smartbodyInstances;
std::map< int, SBM_SmartbodyCharacter * > g_characters;


SMARTBODY_C_DLL_API SBMHANDLE SBM_CreateSBM()
{
   int currentSize = g_smartbodyInstances.size();
   g_smartbodyInstances[ currentSize ] = new Smartbody_dll();
   return currentSize;
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
   return retVal;
}


SMARTBODY_C_DLL_API bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnVisemeCallback visemeCB )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SBM_SmartbodyListener * listener = new SBM_SmartbodyListener( sbmHandle, createCB, deleteCB, visemeCB );
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

#if 0
   std::ofstream myfile;
   myfile.open( "example.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app );
   myfile << op << " " << args << '\n';
   myfile.close();
#endif

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

   SmartbodyCharacter dllChar = g_smartbodyInstances[ sbmHandle ]->GetCharacter( (string)name );

   SBM_CharToCSbmChar( &dllChar, character );

   return true;
}


SMARTBODY_C_DLL_API bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character )
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
   delete [] character->m_name;

   return true;
}


bool SBM_HandleExists( SBMHANDLE sbmHandle )
{
   return g_smartbodyInstances.find( sbmHandle ) != g_smartbodyInstances.end();
}


void SBM_CharToCSbmChar( const ::SmartbodyCharacter * sbmChar, SBM_SmartbodyCharacter * sbmCChar )
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
   sbmCChar->m_name = new char[ sbmChar->m_name.length() + 1 ];
   strcpy( sbmCChar->m_name, sbmChar->m_name.c_str() );

   // copy joint data
   sbmCChar->m_numJoints = 0;
   sbmCChar->m_joints = NULL;

   if ( sbmChar->m_joints.size() > 0 )
   {
      sbmCChar->m_numJoints = sbmChar->m_joints.size();
      sbmCChar->m_joints = new SBM_SmartbodyJoint[ sbmCChar->m_numJoints ];

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
         sbmCChar->m_joints[ i ].m_name = new char[ sbmChar->m_joints[ i ].m_name.length() + 1 ];
         strcpy( sbmCChar->m_joints[ i ].m_name, sbmChar->m_joints[ i ].m_name.c_str() );
      }
   }
}
