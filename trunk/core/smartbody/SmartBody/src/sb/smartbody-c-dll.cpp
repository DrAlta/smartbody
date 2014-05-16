
#include "vhcl.h"

#include "smartbody-c-dll.h"
#include "sb/SBScene.h"
#include "sb/SBAssetManager.h"
#include "sb/SBDebuggerServer.h"
#include "sb/SBPython.h"
#include "sb/SBSceneListener.h"
#include "sb/SBSimulationManager.h"
#include "sb/SBSpeechManager.h"
#include "sb/SBVHMsgManager.h"
#include "sbm/local_speech.h"
#include "sbm/mcontrol_callbacks.h"
#include "sbm/sbm_constants.h"

#ifndef SB_NO_VHMSG
#include "vhmsg-tt.h"
#endif

#include <fstream>
#include <ios>
#include <string.h>
#include <map>


using std::string;


struct SBM_CallbackInfo
{
   string name;
   string objectClass;
   string visemeName;
   float weight;
   float blendTime;
   string logMessage;
   int logMessageType;

   SBM_CallbackInfo() : weight(0), blendTime(0), logMessageType(0) {}
   void operator = ( const SBM_CallbackInfo & l )
   {
      name = l.name;
      objectClass = l.objectClass;
      visemeName = l.visemeName;
      weight = l.weight;
      blendTime = l.blendTime;
      logMessage = l.logMessage;
      logMessageType = l.logMessageType;
   }
};


class LogMessageListener : public vhcl::Log::Listener
{
private:
   SBMHANDLE m_sbmHandle;

public:
   LogMessageListener( SBMHANDLE sbmHandle ) : m_sbmHandle(sbmHandle) {}
   ~LogMessageListener() {}

   virtual void OnMessage( const std::string & message );
};


class SBM_SmartbodyListener : public SmartBody::SBSceneListener
{
private:
   SBMHANDLE m_sbmHandle;

public:
   SBM_SmartbodyListener( SBMHANDLE sbmHandle )
   {
      m_sbmHandle = sbmHandle;
   }

   virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
   virtual void OnCharacterDelete( const std::string & name );
   virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime );
   virtual void OnChannel( const std::string & name, const std::string & channelName, const float value );
};


class Smartbody_c_Dll
{
public:
   SBMHANDLE m_handle;
   SBM_SmartbodyListener * m_listener;
   LogMessageListener * m_logListener;
   std::queue<SBM_CallbackInfo> m_createCallbackInfo;
   std::queue<SBM_CallbackInfo> m_deleteCallbackInfo;
   std::queue<SBM_CallbackInfo> m_changeCallbackInfo;
   std::queue<SBM_CallbackInfo> m_visemeCallbackInfo;
   std::queue<SBM_CallbackInfo> m_channelCallbackInfo;
   std::queue<SBM_CallbackInfo> m_logCallbackInfo;

public:
   Smartbody_c_Dll(SBMHANDLE handle)
   {
      m_handle = handle;
      m_listener = new SBM_SmartbodyListener(m_handle);
      m_logListener = new LogMessageListener(m_handle);
   }

   virtual ~Smartbody_c_Dll()
   {
      delete m_logListener;
      delete m_listener;
   }
};


std::map< int, Smartbody_c_Dll * > g_smartbodyDLLInstances;
int g_handleId_DLL = 0;


bool SBM_ReleaseCharacterJoints( SBM_CharacterFrameDataMarshalFriendly * character );
bool SBM_HandleExists( SBMHANDLE sbmHandle );
bool SBM_InitVHMsg();
void SBM_InitLocalSpeechRelay();


void LogMessageListener::OnMessage( const std::string & message )
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

    SBM_CallbackInfo info;
    info.logMessage = message;
    info.logMessageType = messageType;

    g_smartbodyDLLInstances[m_sbmHandle]->m_logCallbackInfo.push(info);
}


void SBM_SmartbodyListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
    SBM_CallbackInfo info;
    info.name = name;
    info.objectClass = objectClass;
    g_smartbodyDLLInstances[m_sbmHandle]->m_createCallbackInfo.push(info);
    //LOG("smartbody-c-dll : OnCharacterCreate, name = %s, objectClass = %s, number of callback info = %d",name.c_str(), objectClass.c_str(), g_CreateCallbackInfo[m_sbmHandle].size());
}

void SBM_SmartbodyListener::OnCharacterDelete( const std::string & name )
{
    SBM_CallbackInfo info;
    info.name = name;
    g_smartbodyDLLInstances[m_sbmHandle]->m_deleteCallbackInfo.push(info);
}

void SBM_SmartbodyListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{
    SBM_CallbackInfo info;
    info.name = name;
    info.visemeName = visemeName;
    info.weight = weight;
    info.blendTime = blendTime;
    g_smartbodyDLLInstances[m_sbmHandle]->m_visemeCallbackInfo.push(info);
}

void SBM_SmartbodyListener::OnChannel( const std::string & name, const std::string & channelName, const float value )
{
    SBM_CallbackInfo info;
    info.name = name;
    info.visemeName = channelName;
    info.weight = value;
    g_smartbodyDLLInstances[m_sbmHandle]->m_channelCallbackInfo.push(info);
}



#ifdef WIN_BUILD
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
   switch ( fdwReason )
   {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
      default:
         break;
   }

   //vhcl::Memory::EnableDebugFlags( vhcl::Memory::MEM_DEFAULT_FLAGS | vhcl::Memory::CHECK_EVERY_128_DF );  // enable heap checking every 128 allocs

   return TRUE;
}
#endif



SBAPI SBMHANDLE SBM_CreateSBM()
{
   g_handleId_DLL++;
   g_smartbodyDLLInstances[ g_handleId_DLL ] = new Smartbody_c_Dll(g_handleId_DLL);

   return g_handleId_DLL;
}


SBAPI bool SBM_Init( SBMHANDLE sbmHandle, const char * pythonLibPath, bool logToFile )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   // this is the first getScene() called by the system
   SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

   scene->addSceneListener(g_smartbodyDLLInstances[ g_handleId_DLL ]->m_listener);

   vhcl::Log::g_log.AddListener(g_smartbodyDLLInstances[ g_handleId_DLL ]->m_logListener);
   vhcl::Log::g_log.AddListener(new vhcl::Log::DebuggerListener());


   initPython(pythonLibPath);

   SBM_InitVHMsg();
   SBM_InitLocalSpeechRelay();

   srArgBuffer arg_buf( "" );
   mcu_vrAllCall_func( arg_buf, scene->getCommandManager() );

   if (logToFile)
   {
      scene->startFileLogging("./smartbody.log");
   }

   return true;
}


SBAPI bool SBM_Shutdown( SBMHANDLE sbmHandle )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

#ifndef SB_NO_VHMSG
   SmartBody::SBScene::getScene()->getVHMsgManager()->send("vrProcEnd sbm");

   //SmartBody::SBScene::getScene()->getVHMsgManager()->setEnable(false);
   vhmsg::ttu_close();
#endif

   XMLPlatformUtils::Terminate();

   std::map< int, Smartbody_c_Dll * >::iterator itdll = g_smartbodyDLLInstances.find( sbmHandle );
   Smartbody_c_Dll * sbmdll = g_smartbodyDLLInstances[ sbmHandle ];
   g_smartbodyDLLInstances.erase( itdll );

   SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
   scene->removeSceneListener(sbmdll->m_listener);
   vhcl::Log::g_log.RemoveListener(sbmdll->m_logListener);
   delete sbmdll;

   return true;
}


SBAPI bool SBM_LoadSkeleton( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * skeletonName )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   int ret = SmartBody::SBScene::getScene()->getAssetManager()->load_skeleton( data, sizeBytes, skeletonName );
   return ret == CMD_SUCCESS;
}


SBAPI bool SBM_LoadMotion( SBMHANDLE sbmHandle, const void * data, int sizeBytes, const char * motionName )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   int ret = SmartBody::SBScene::getScene()->getAssetManager()->load_motion( data, sizeBytes, motionName );
   return ret == CMD_SUCCESS;
}


SBAPI bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
   scene->getSimulationManager()->setTime(timeInSeconds);
   scene->update();
   return true;
}


SBAPI void SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   SmartBody::SBScene::getScene()->getDebuggerServer()->SetID( id );
}


SBAPI void SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
   SBDebuggerServer * debuggerServer = scene->getDebuggerServer();
   debuggerServer->m_cameraPos.x = x;
   debuggerServer->m_cameraPos.y = y;
   debuggerServer->m_cameraPos.z = z;
   debuggerServer->m_cameraRot.x = rx;
   debuggerServer->m_cameraRot.y = ry;
   debuggerServer->m_cameraRot.z = rz;
   debuggerServer->m_cameraRot.w = rw;
   debuggerServer->m_cameraFovY   = fov;
   debuggerServer->m_cameraAspect = aspect;
   debuggerServer->m_cameraZNear  = zNear;
   debuggerServer->m_cameraZFar   = zFar;
}


SBAPI void SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return;
   }

   SmartBody::SBScene::getScene()->getDebuggerServer()->m_rendererIsRightHanded = enabled;
}


SBAPI bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
   string s = string(op) + string(" ") + string(args);
   scene->command( s.c_str() );
   scene->getDebuggerServer()->ProcessVHMsgs(op, args);

   return true;
}


SBAPI bool SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

   character->m_name = new char[ strlen(name) + 1 ];
   strcpy( character->m_name, name );

   character->x  = 0;
   character->y  = 0;
   character->z  = 0;
   character->rw = 0;
   character->rx = 0;
   character->ry = 0;
   character->rz = 0;
   character->m_numJoints = 0;

   character->jname = NULL;
   character->jx  = NULL;
   character->jy  = NULL;
   character->jz  = NULL;
   character->jrw = NULL;
   character->jrx = NULL;
   character->jry = NULL;
   character->jrz = NULL;

   return true;
}


SBAPI bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_CharacterFrameDataMarshalFriendly * character )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      LOG("SBM_GetCharcter : Handle %d does not exist", sbmHandle);
      return false;
   }

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
   SmartBody::SBCharacter * sbcharacter = scene->getCharacter(name);

   const SBM_CharacterFrameDataMarshalFriendly & data = sbcharacter->GetFrameDataMarshalFriendly();

   if (character->m_numJoints == 0 || character->m_numJoints != data.m_numJoints)
   {
      SBM_ReleaseCharacterJoints(character);

      character->m_numJoints = data.m_numJoints;
      character->jname = new char * [ character->m_numJoints ];
      character->jx = new float [ character->m_numJoints ];
      character->jy = new float [ character->m_numJoints ];
      character->jz = new float [ character->m_numJoints ];
      character->jrw = new float [ character->m_numJoints ];
      character->jrx = new float [ character->m_numJoints ];
      character->jry = new float [ character->m_numJoints ];
      character->jrz = new float [ character->m_numJoints ];

      for ( size_t i = 0; i < character->m_numJoints; i++ )
      {
         character->jname[ i ] = new char[ strlen(data.jname[ i ]) + 1 ];
         strcpy( character->jname[ i ], data.jname[ i ] );
      }
   }

   character->x = data.x;
   character->y = data.y;
   character->z = data.z;
   character->rw = data.rw;
   character->rx = data.rx;
   character->ry = data.ry;
   character->rz = data.rz;

   for ( size_t i = 0; i < data.m_numJoints; i++ )
   {
      character->jx[i] = data.jx[i];
      character->jy[i] = data.jy[i];
      character->jz[i] = data.jz[i];
      character->jrw[i] = data.jrw[i];
      character->jrx[i] = data.jrx[i];
      character->jry[i] = data.jry[i];
      character->jrz[i] = data.jrz[i];
   }

   return true;
}


SBAPI bool SBM_ReleaseCharacter( SBM_CharacterFrameDataMarshalFriendly * character )
{
   if ( !character )
   {
      return false;
   }

   SBM_ReleaseCharacterJoints(character);

   delete [] character->m_name;
   character->m_name = NULL;

   return true;
}


bool SBM_ReleaseCharacterJoints( SBM_CharacterFrameDataMarshalFriendly * character )
{
   if ( !character )
   {
      return false;
   }

   for ( size_t i = 0; i < character->m_numJoints; i++ )
   {
      delete [] character->jname[ i ];
   }

   character->x  = 0;
   character->y  = 0;
   character->z  = 0;
   character->rw = 0;
   character->rx = 0;
   character->ry = 0;
   character->rz = 0;
   character->m_numJoints = 0;

   delete [] character->jname;
   delete [] character->jx;
   delete [] character->jy;
   delete [] character->jz;
   delete [] character->jrw;
   delete [] character->jrx;
   delete [] character->jry;
   delete [] character->jrz;

   character->jname = NULL;
   character->jx  = NULL;
   character->jy  = NULL;
   character->jz  = NULL;
   character->jrw = NULL;
   character->jrx = NULL;
   character->jry = NULL;
   character->jrz = NULL;

   return true;
}


bool SBM_HandleExists( SBMHANDLE sbmHandle )
{
   return g_smartbodyDLLInstances.find( sbmHandle ) != g_smartbodyDLLInstances.end();
}


SBAPI bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_createCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_createCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_createCallbackInfo.pop();
    strncpy(name, info.name.c_str(), maxNameLen);
    strncpy(objectClass, info.objectClass.c_str(), maxObjectClassLen);    
    return true;
}

SBAPI bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_deleteCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_deleteCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_deleteCallbackInfo.pop();
    strncpy(name, info.name.c_str(), maxNameLen);
    return true;
}

SBAPI bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_changeCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_changeCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_changeCallbackInfo.pop();    
    strncpy(name, info.name.c_str(), maxNameLen);   
    return true;
}

SBAPI bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_visemeCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_visemeCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_visemeCallbackInfo.pop();
    strncpy(name, info.name.c_str(), maxNameLen);
    strncpy(visemeName, info.visemeName.c_str(), maxVisemeNameLen);
    *weight = info.weight;
    *blendTime = info.blendTime;
    return true;
}

SBAPI bool SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value )
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_channelCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_channelCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_channelCallbackInfo.pop();
    strncpy(name, info.name.c_str(), maxNameLen);
    strncpy(channelName, info.visemeName.c_str(), maxChannelNameLen);
    *value = info.weight;
    return true;
}

SBAPI bool SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char * logMessage, int maxLogMessageLen, int * logMessageType)
{
    if ( !SBM_HandleExists( sbmHandle ) || g_smartbodyDLLInstances[sbmHandle]->m_logCallbackInfo.size() == 0)
    {
        return false;
    }

    SBM_CallbackInfo info = g_smartbodyDLLInstances[sbmHandle]->m_logCallbackInfo.front();
    g_smartbodyDLLInstances[sbmHandle]->m_logCallbackInfo.pop();
    strncpy(logMessage, info.logMessage.c_str(), maxLogMessageLen);
    *logMessageType = info.logMessageType;
    return true;
}


SBAPI bool SBM_PythonCommandVoid( SBMHANDLE sbmHandle, const char * command)
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

#ifndef SB_NO_PYTHON
   return SmartBody::SBScene::getScene()->run(command);
#else
   return false;
#endif
}

SBAPI bool SBM_PythonCommandBool( SBMHANDLE sbmHandle, const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return false;
   }

#ifndef SB_NO_PYTHON
   try
   {
      boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command, mainDict);
      bool result = boost::python::extract<bool>(mainDict["ret"]);
      return result;
   }
   catch (...)
   {
      PyErr_Print();
      return false;
   }
#else
   return false;
#endif
}

SBAPI int SBM_PythonCommandInt( SBMHANDLE sbmHandle, const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

#ifndef SB_NO_PYTHON
   try
   {
      boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command, mainDict);
      int result = boost::python::extract<int>(mainDict["ret"]);
      return result;
   }
   catch (...)
   {
      PyErr_Print();
      return 0;
   }
#else
   return 0;
#endif
}

SBAPI float SBM_PythonCommandFloat( SBMHANDLE sbmHandle, const char * command )
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

#ifndef SB_NO_PYTHON
   try
   {
      boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command, mainDict);
      float result = boost::python::extract<float>(mainDict["ret"]);
      return result;
   }
   catch (...)
   {
      PyErr_Print();
      return 0;
   }
#else
   return 0;
#endif
}

SBAPI char * SBM_PythonCommandString( SBMHANDLE sbmHandle, const char * command, char * output, int maxLen)
{
   if ( !SBM_HandleExists( sbmHandle ) )
   {
      return 0;
   }

#ifndef SB_NO_PYTHON
   try
   {
      boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command, mainDict);
      std::string result = boost::python::extract<std::string>(mainDict["ret"]);
      strncpy(output, result.c_str(), maxLen);
      return output;
   }
   catch (...)
   {
      PyErr_Print();
      return "";
   }
#else
   return "";
#endif
}


bool SBM_InitVHMsg()
{
#if defined(WIN_BUILD) || defined(MAC_BUILD)

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

   printf( "Starting VHMsg (DLL side)\n" );

   SmartBody::SBVHMsgManager * vhmsgManager = scene->getVHMsgManager();
   const char* envScope = getenv("VHMSG_SCOPE");
   const char* envServer = getenv("VHMSG_SERVER");
   if (envScope)
   {
      vhmsgManager->setScope(envScope);
   }
   if (envServer)
   {
      vhmsgManager->setServer(envServer);
   }

   //int err = vhmsg::ttu_open();
   //if (err == vhmsg::TTU_SUCCESS)
   vhmsgManager->setEnable(true);

#endif
   return true;
}


void SBM_InitLocalSpeechRelay()
{
   //AUDIO_Init();

#if defined(__ANDROID__)
   std::string festivalLibDir = "/sdcard/SBUnity/festival/lib/";
   std::string festivalCacheDir = "/sdcard/SBUnity/festival/cache/";
   std::string cereprocLibDir = "/sdcard/SBUnity/cerevoice/voices/";
#else
   std::string festivalLibDir = "./SBUnity/festival/lib/";
   std::string festivalCacheDir = "./SBUnity/festival/cache/";
   std::string cereprocLibDir = "./SBUnity/cerevoice/voices/";
#endif

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
   scene->getSpeechManager()->festivalRelay()->initSpeechRelay(festivalLibDir, festivalCacheDir);
   scene->getSpeechManager()->cereprocRelay()->initSpeechRelay(cereprocLibDir, festivalCacheDir);
}






#if 0
// stubs used for testing dll usage on other platforms
int unused = 0;
SBAPI SBMHANDLE SBM_CreateSBM() { unused++; return unused; }

SBAPI bool SBM_SetSpeechAudiofileBasePath( SBMHANDLE sbmHandle, const char * basePath ) { return true; }
SBAPI bool SBM_SetProcessId( SBMHANDLE sbmHandle, const char * processId ) { return true; }
SBAPI bool SBM_SetMediaPath( SBMHANDLE sbmHandle, const char * path ) { return true; }

SBAPI bool SBM_Init( SBMHANDLE sbmHandle, const char* pythonLibPath, bool logToFile ) { return true; }
SBAPI bool SBM_Shutdown( SBMHANDLE sbmHandle ) { return true; }

SBAPI bool SBM_SetListener( SBMHANDLE sbmHandle, SBM_OnCreateCharacterCallback createCB, SBM_OnCharacterDeleteCallback deleteCB, SBM_OnCharacterChangeCallback changedCB, SBM_OnVisemeCallback visemeCB, SBM_OnChannelCallback channelCB ) { return true; }

SBAPI bool SBM_Update( SBMHANDLE sbmHandle, double timeInSeconds ) { return true; }

SBAPI void SBM_SetDebuggerId( SBMHANDLE sbmHandle, const char * id ) { return; }
SBAPI void SBM_SetDebuggerCameraValues( SBMHANDLE sbmHandle, double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar ) { return; }
SBAPI void SBM_SetDebuggerRendererRightHanded( SBMHANDLE sbmHandle, bool enabled ) { return; }

SBAPI bool SBM_ProcessVHMsgs( SBMHANDLE sbmHandle, const char * op, const char * args ) { return true; }
SBAPI bool SBM_ExecutePython( SBMHANDLE sbmHandle, const char * command ) { return true; }

SBAPI int  SBM_GetNumberOfCharacters( SBMHANDLE sbmHandle ) { return 42; }
SBAPI bool SBM_InitCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character ) { return true; }
SBAPI bool SBM_GetCharacter( SBMHANDLE sbmHandle, const char * name, SBM_SmartbodyCharacter * character ) { return true; }
SBAPI bool SBM_ReleaseCharacter( SBM_SmartbodyCharacter * character ) { return true; }
SBAPI bool SBM_SetLogMessageCallback(LogMessageCallback cb) { return true; }
SBAPI void SBM_LogMessage(const char * message, int messageType) { return; }

// used for polling on iOS since callbacks aren't allowed
SBAPI bool SBM_IsCharacterCreated( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * objectClass, int maxObjectClassLen ) { return false; }
SBAPI bool SBM_IsCharacterDeleted( SBMHANDLE sbmHandle, char * name, int maxNameLen ) { return false; }
SBAPI bool SBM_IsCharacterChanged( SBMHANDLE sbmHandle, char * name, int maxNameLen ) { return false; }
SBAPI bool SBM_IsVisemeSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * visemeName, int maxVisemeNameLen, float * weight, float * blendTime ) { return false; }
SBAPI bool SBM_IsChannelSet( SBMHANDLE sbmHandle, char * name, int maxNameLen, char * channelName, int maxChannelNameLen, float * value ) { return false; }
SBAPI bool SBM_IsLogMessageWaiting( SBMHANDLE sbmHandle, char *logMessage, int maxLogMessageLen, int* logMessageType ) { return false; }

// python usage functions
// functions can't be distinguished by return type alone so they are named differently
SBAPI bool SBM_PythonCommandVoid( SBMHANDLE sbmHandle,  const char * command ) { return true; }
SBAPI bool SBM_PythonCommandBool( SBMHANDLE sbmHandle,  const char * command ) { return true; }
SBAPI int SBM_PythonCommandInt( SBMHANDLE sbmHandle,  const char * command ) { return 42; }
SBAPI float SBM_PythonCommandFloat( SBMHANDLE sbmHandle,  const char * command )  { return 42; }
SBAPI char * SBM_PythonCommandString( SBMHANDLE sbmHandle, const char * command, char * output, int maxLen) { return "test"; }

#endif
