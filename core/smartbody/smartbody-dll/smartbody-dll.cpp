
#include "vhcl.h"

#if __native_client__
#include "vhmsg-tt.h"
#endif

#include "smartbody-dll.h"

#ifdef WIN_BUILD
#include <windows.h>
#include <mmsystem.h>
#endif

#pragma warning(push)
#pragma warning(disable:4121)  // needed for boost::python::extract<std::string>() below
#include "sb/SBScene.h"
#include "sbm/xercesc_utils.hpp"
#include "sbm/mcontrol_util.h"
#include "sbm/mcontrol_callbacks.h"
#include "sbm/sbm_character.hpp"
#include "sbm/sbm_test_cmds.hpp"
#include "sbm/resource_cmds.h"
#include "sbm/locomotion_cmds.hpp"
#include "sb/SBPython.h"
#include "sb/SBCharacter.h"
#include "sb/SBSkeleton.h"
#pragma warning(pop)


#include "sbm/SbmDebuggerServer.h"


using std::string;


#if (NACL_BUILD) 
#define USE_SBPYTHON  0
#else
#define USE_SBPYTHON  1
#endif


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


class Smartbody_dll_SBMCharacterListener_Internal : public SBMCharacterListener
{
   private:
      Smartbody_dll * m_dll;

   public:
      explicit Smartbody_dll_SBMCharacterListener_Internal( Smartbody_dll * dll ) :
         m_dll( dll )
      {
      }

      virtual ~Smartbody_dll_SBMCharacterListener_Internal()
      {
      }

      virtual void OnCharacterCreate( const string & name, const string & objectClass )
      {
         if ( m_dll->m_listener )
         {
            m_dll->m_listener->OnCharacterCreate( name, objectClass );
         }
      }

      virtual void OnCharacterDelete( const string & name )
      {
         if ( m_dll->m_listener )
         {
            std::map<std::string,SmartbodyCharacter*>::iterator mi = m_dll->m_characters.find(name);
            SmartbodyCharacter* pc = NULL;
            if (mi != m_dll->m_characters.end())
            {
               pc = mi->second;
               delete pc;
               m_dll->m_characters.erase(mi);
            }
            m_dll->m_listener->OnCharacterDelete( name );
         }
      }

      virtual void OnPawnCreate( const std::string & name )
      {
         if ( m_dll->m_listener )
         {
            m_dll->m_listener->OnPawnCreate( name );
         }
      }

      virtual void OnViseme( const string & name, const string & visemeName, const float weight, const float blendTime )
      {
         if ( m_dll->m_listener )
         {
            m_dll->m_listener->OnViseme( name, visemeName, weight, blendTime );
         }
      }

      virtual void OnChannel( const string & name, const string & channelName, const float value )
      {
         if ( m_dll->m_listener )
         {
            m_dll->m_listener->OnChannel( name, channelName, value );
         }
      }

      virtual void OnCharacterChanged( const std::string& name ) 
      {	 
         if ( m_dll->m_listener )
         {
            std::map<std::string,SmartbodyCharacter*>::iterator mi = m_dll->m_characters.find(name);
            SmartbodyCharacter* pc = NULL;
            if (mi != m_dll->m_characters.end())
            {
               pc = mi->second;
               delete pc;
               m_dll->m_characters.erase(mi);
            }
            m_dll->m_listener->OnCharacterChanged(name);
         }
      }
};

SMARTBODY_DLL_API Smartbody_dll::Smartbody_dll() :
   m_internalListener( NULL )
{
   vhcl::Log::g_log.AddListener( new vhcl::Log::DebuggerListener() );
}


SMARTBODY_DLL_API Smartbody_dll::~Smartbody_dll()
{
}


SMARTBODY_DLL_API void Smartbody_dll::SetSpeechAudiofileBasePath( const std::string & basePath )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: need a scene->clearAssetPath("audio");
   // TODO: need a scene->setPathPrefix(??);

   // clear the old audio path list
   mcu.audio_paths = srPathList();
   mcu.audio_paths.setPathPrefix(mcu.getMediaPath());
   scene->addAssetPath("audio", basePath);
}

SMARTBODY_DLL_API void Smartbody_dll::SetProcessId( const std::string & processId )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: need a scene->setProcessId(processId);
   scene;
   mcu.set_process_id( processId.c_str() );
}


SMARTBODY_DLL_API void Smartbody_dll::SetMediaPath( const std::string & path )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;
   scene->setMediaPath(path);
}


SMARTBODY_DLL_API bool Smartbody_dll::Init(const std::string& pythonLibPath, bool logToFile)
{
   m_internalListener = new Smartbody_dll_SBMCharacterListener_Internal( this );
   
   XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU


   // TODO: Replace with SBScene * g_scene = new SBScene();
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   // TODO: Replace with g_scene->SetCharacterListener(m_internalListener)
   mcu.sbm_character_listener = m_internalListener;
   
   SetSpeechAudiofileBasePath( "../../" );

   initPython(pythonLibPath);

   InitVHMsg();
   RegisterCallbacks();

   srArgBuffer arg_buf( "" );
   mcu_vrAllCall_func( arg_buf, &mcu );

   if (logToFile)
   {
      vhcl::Log::Listener* listener = new vhcl::Log::FileListener("./smartbody.log");
      vhcl::Log::g_log.AddListener(listener);
   }

   return true;
}


SMARTBODY_DLL_API bool Smartbody_dll::Shutdown()
{
   {
      mcuCBHandle & mcu = mcuCBHandle::singleton();
      mcu.vhmsg_send( "vrProcEnd sbm" );
   }

   mcuCBHandle::destroy_singleton();

   XMLPlatformUtils::Terminate();

   std::map<std::string,SmartbodyCharacter*>::iterator mi; // m_dll->m_characters.find(name);
   for (mi  = m_characters.begin();
        mi != m_characters.end();
        mi++)
   {
      SmartbodyCharacter* pc = mi->second;
      delete pc;
   }
   m_characters.clear();

   delete m_internalListener;
   m_internalListener = NULL;

   return true;
}


SMARTBODY_DLL_API void Smartbody_dll::SetListener( SmartbodyListener * listener )
{
   m_listener = listener;
}


SMARTBODY_DLL_API bool Smartbody_dll::Update( const double timeInSeconds )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: replace with SBScene->getSimulationManager()?
   scene;
   bool update_sim = mcu.update_timer( timeInSeconds );
   if( update_sim ) mcu.update();
   return true;
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerId( const std::string & id )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->SetID( id );
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerCameraValues( double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->m_cameraPos.x = x;
   mcu._scene->getDebuggerServer()->m_cameraPos.y = y;
   mcu._scene->getDebuggerServer()->m_cameraPos.z = z;
   mcu._scene->getDebuggerServer()->m_cameraRot.x = rx;
   mcu._scene->getDebuggerServer()->m_cameraRot.y = ry;
   mcu._scene->getDebuggerServer()->m_cameraRot.z = rz;
   mcu._scene->getDebuggerServer()->m_cameraRot.w = rw;
   mcu._scene->getDebuggerServer()->m_cameraFovY   = fov;
   mcu._scene->getDebuggerServer()->m_cameraAspect = aspect;
   mcu._scene->getDebuggerServer()->m_cameraZNear  = zNear;
   mcu._scene->getDebuggerServer()->m_cameraZFar   = zFar;
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerRendererRightHanded( bool enabled )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->m_rendererIsRightHanded = enabled;
}


SMARTBODY_DLL_API bool Smartbody_dll::ProcessVHMsgs( const char * op, const char * args )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   string s = string(op) + string(" ") + string(args);
   scene->command( s.c_str() );

   scene->getDebuggerServer()->ProcessVHMsgs(op, args);

   return true;
}

SMARTBODY_DLL_API bool Smartbody_dll::ExecutePython( const char * command )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   int ret = mcu.executePython(command);
   if (ret == CMD_SUCCESS)
      return true;
   else
      return false;
}



SMARTBODY_DLL_API int Smartbody_dll::GetNumberOfCharacters()
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;
   return scene->getNumCharacters();
}


SMARTBODY_DLL_API SmartbodyCharacter& Smartbody_dll::GetCharacter( const string & name )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   SBCharacter * char_p = scene->getCharacter(name);
   if ( char_p )
   {
      std::map<std::string,SmartbodyCharacter*>::iterator mi = m_characters.find(name);
      SmartbodyCharacter* pc = NULL;
      bool hasChar = false;
      if (mi != m_characters.end())
      {
         pc = mi->second;
         hasChar = true;
      }
      else
      {
         pc = new SmartbodyCharacter();
         hasChar = false;
         m_characters[name] = pc;
      }

      SmartbodyCharacter& c = *pc;

      const SkJoint * joint = char_p->get_world_offset_joint();

      const SkJointPos * pos = joint->const_pos();
      float x = pos->value( SkJointPos::X );
      float y = pos->value( SkJointPos::Y );
      float z = pos->value( SkJointPos::Z );
      //LOG("world_offset = %f %f %f",x,y,z);

      SkJoint::RotType rot_type = joint->rot_type();
      if ( rot_type != SkJoint::TypeQuat )
      {
         //cerr << "ERROR: Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")"<<endl;
      }

      // const_cast because the SrQuat does validation (no const version of value())
      const SrQuat & q = ((SkJoint *)joint)->quat()->value();

      c.m_name = char_p->getName();
      c.x = x;
      c.y = y;
      c.z = z;
      c.rw = q.w;
      c.rx = q.x;
      c.ry = q.y;
      c.rz = q.z;


      const std::vector<SkJoint *> & joints  = char_p->getSkeleton()->joints();

      for ( size_t i = 0; i < joints.size(); i++ )
      {
         // const_cast because the SrQuat does validation (no const version of value())
         SkJoint * j = joints[i];

         SrQuat q = j->quat()->value();

         //printf( "%s %f %f %f %f\n", (const char *)j->name(), q.w, q.x, q.y, q.z );

         float posx = j->pos()->value( 0 );
         float posy = j->pos()->value( 1 );
         float posz = j->pos()->value( 2 );
         if ( false )
         {
            posx += j->offset().x;
            posy += j->offset().y;
            posz += j->offset().z;
         }

         std::string jointName;
         if (j->extName() != "")
            jointName = j->extName();
         else
            jointName = j->name();

         if (hasChar)
         {
            SmartbodyJoint& joint = c.m_joints[i];
            joint.m_name = jointName;			
            SrQuat jointQ = j->quat()->value();
            // if (i==136)
            // {
            //    //sr_out << "eyeball quat = " << jointQ << srnl;
            //    LOG("eyeball quat = %f %f %f %f\n",jointQ.x,jointQ.y,jointQ.z,jointQ.w);
            // }

            joint.x = posx;
            joint.y = posy;
            joint.z = posz;
            joint.rw = q.w;
            joint.rx = q.x;
            joint.ry = q.y;
            joint.rz = q.z;
         }
         else
         {
            SmartbodyJoint joint;
            joint.m_name = jointName;
            joint.x = posx;
            joint.y = posy;
            joint.z = posz;
            joint.rw = q.w;
            joint.rx = q.x;
            joint.ry = q.y;
            joint.rz = q.z;
            c.m_joints.push_back( joint );
         }
      }
      return c;
   }
   else
   {
      return m_emptyCharacter;
   }
}

bool Smartbody_dll::InitVHMsg()
{
#if !defined(ANDROID_BUILD) && !defined(IPHONE_BUILD)

   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   printf( "Starting VHMsg (DLL side)\n" );

   // TODO: need scene->SetVhmsgEnabled(true)
   scene;
   int err = vhmsg::ttu_open();
   if (err == vhmsg::TTU_SUCCESS)
      mcu.vhmsg_enabled = true;

#endif
   return true;
}


SMARTBODY_DLL_API bool Smartbody_dll::PythonCommandVoid( const std::string & command )
{
#if USE_SBPYTHON
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   return mcu.executePython(command.c_str()) == 1 ? true : false;
#else
   return false;
#endif
}

bool Smartbody_dll::PythonCommandBool( const std::string & command )
{
#if USE_SBPYTHON
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   try
   {
      boost::python::object obj = boost::python::exec(command.c_str(), mcu.mainDict);
      bool result = boost::python::extract<bool>(mcu.mainDict["ret"]);
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


int Smartbody_dll::PythonCommandInt( const std::string & command )
{
#if USE_SBPYTHON
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   try
   {
      boost::python::object obj = boost::python::exec(command.c_str(),mcu.mainDict);
      int result = boost::python::extract<int>(mcu.mainDict["ret"]);
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

float Smartbody_dll::PythonCommandFloat( const std::string & command )
{
#if USE_SBPYTHON
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   try
   {
      boost::python::object obj = boost::python::exec(command.c_str(), mcu.mainDict);
      float result = boost::python::extract<float>(mcu.mainDict["ret"]);
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

std::string Smartbody_dll::PythonCommandString( const std::string & command )
{
#if USE_SBPYTHON
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   try
   {
      boost::python::object obj = boost::python::exec(command.c_str(), mcu.mainDict);
      std::string result = boost::python::extract<std::string>(mcu.mainDict["ret"]);
      return result;
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

void Smartbody_dll::RegisterCallbacks()
{
  // place any additional commands here...
}
