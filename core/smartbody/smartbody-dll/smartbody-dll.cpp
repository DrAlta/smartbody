
#include "vhcl.h"

#include "smartbody-dll.h"

#ifdef WIN_BUILD
#include <windows.h>
#include <mmsystem.h>
#endif

#include "sbm/SBScene.h"
#include "sbm/xercesc_utils.hpp"
#include "sbm/mcontrol_util.h"
#include "sbm/mcontrol_callbacks.h"
#include "sbm/sbm_character.hpp"
#include "sbm/sbm_test_cmds.hpp"
#include "sbm/resource_cmds.h"
#include "sbm/locomotion_cmds.hpp"
#include "sbm/SBPython.h"

#include "sbm/SbmDebuggerServer.h"


using std::string;


// TODO: this stuff should go in vhcl_platform.h
#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#define USE_SBSCENE  1


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
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: need a scene->clearAssetPath("audio");
   // TODO: need a scene->setPathPrefix(??);

   mcu.audio_paths = srPathList();
   mcu.audio_paths.setPathPrefix(mcu.getMediaPath());
   scene->addAssetPath("audio", basePath);


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   // clear the old audio path list
   mcu.audio_paths = srPathList();
   mcu.audio_paths.setPathPrefix(mcu.getMediaPath());
   mcu.audio_paths.insert((char*) basePath.c_str());
#endif
}


SMARTBODY_DLL_API void Smartbody_dll::SetFacebone( const bool enabled )
{
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: need a scene->setFacebone(enabled);
   scene;
   mcu.net_face_bones = enabled;


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.net_face_bones = enabled;
#endif
}


SMARTBODY_DLL_API void Smartbody_dll::SetProcessId( const std::string & processId )
{
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: need a scene->setProcessId(processId);
   scene;
   mcu.set_process_id( processId.c_str() );


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.set_process_id( processId.c_str() );
#endif
}


SMARTBODY_DLL_API void Smartbody_dll::SetMediaPath( const std::string & path )
{
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;
   scene->setMediaPath(path);


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.setMediaPath( path );
#endif
}


SMARTBODY_DLL_API bool Smartbody_dll::Init(const std::string& pythonLibPath, bool logToFile)
{
   m_internalListener = new Smartbody_dll_SBMCharacterListener_Internal( this );

   XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU

#if USE_SBSCENE
   // TODO: Replace with SBScene * g_scene = new SBScene();
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   // TODO: Replace with g_scene->SetCharacterListener(m_internalListener)
   mcu.sbm_character_listener = m_internalListener;


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.sbm_character_listener = m_internalListener;
#endif

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
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   // TODO: replace with SBScene->getSimulationManager()?
   scene;
   bool update_sim = mcu.update_timer( timeInSeconds );
   if( update_sim ) mcu.update();
   return true;


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   bool update_sim = mcu.update_timer( timeInSeconds );
   if( update_sim ) mcu.update();

   return true;
#endif
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerId( const std::string & id )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->SetID( id );
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerCameraValues( double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->m_camera.pos.x = x;
   mcu._scene->getDebuggerServer()->m_camera.pos.y = y;
   mcu._scene->getDebuggerServer()->m_camera.pos.z = z;
   mcu._scene->getDebuggerServer()->m_camera.rot.x = rx;
   mcu._scene->getDebuggerServer()->m_camera.rot.y = ry;
   mcu._scene->getDebuggerServer()->m_camera.rot.z = rz;
   mcu._scene->getDebuggerServer()->m_camera.rot.w = rw;
   mcu._scene->getDebuggerServer()->m_camera.fovY   = fov;
   mcu._scene->getDebuggerServer()->m_camera.aspect = aspect;
   mcu._scene->getDebuggerServer()->m_camera.zNear  = zNear;
   mcu._scene->getDebuggerServer()->m_camera.zFar   = zFar;
}


SMARTBODY_DLL_API void Smartbody_dll::SetDebuggerRendererRightHanded( bool enabled )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu._scene->getDebuggerServer()->m_rendererIsRightHanded = enabled;
}


SMARTBODY_DLL_API bool Smartbody_dll::ProcessVHMsgs( const char * op, const char * args )
{
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   string s = string(op) + string(" ") + string(args);
   scene->command( s.c_str() );

   return true;


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu.execute( op, (char *)args );


   mcu._scene->getDebuggerServer()->ProcessVHMsgs(op, args);


   return true;
#endif
}


SMARTBODY_DLL_API int Smartbody_dll::GetNumberOfCharacters()
{
#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;
   return scene->getNumCharacters();


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   return mcu.getNumCharacters();
#endif
}


SMARTBODY_DLL_API SmartbodyCharacter& Smartbody_dll::GetCharacter( const string & name )
{
#if USE_SBSCENE
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
            // 			if (i==136)
            // 			{
            // 				//sr_out << "eyeball quat = " << jointQ << srnl;
            // 				LOG("eyeball quat = %f %f %f %f\n",jointQ.x,jointQ.y,jointQ.z,jointQ.w);
            // 			}

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

#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   SbmCharacter * char_p = mcu.getCharacter(name );
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
            // 			if (i==136)
            // 			{
            // 				//sr_out << "eyeball quat = " << jointQ << srnl;
            // 				LOG("eyeball quat = %f %f %f %f\n",jointQ.x,jointQ.y,jointQ.z,jointQ.w);
            // 			}

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
#endif
}


bool Smartbody_dll::InitVHMsg()
{
#if !defined(__ANDROID__) && !defined(SBM_IPHONE)

#if USE_SBSCENE
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   SBScene * scene = mcu._scene;

   printf( "Starting VHMsg (DLL side)\n" );

   // TODO: need scene->SetVhmsgEnabled(true)
   scene;
   int err = vhmsg::ttu_open();
   if (err == vhmsg::TTU_SUCCESS)
      mcu.vhmsg_enabled = true;


#else
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   printf( "Starting VHMsg (DLL side)\n" );

   int err = vhmsg::ttu_open();
   if (err == vhmsg::TTU_SUCCESS)
      mcu.vhmsg_enabled = true;
#endif

#endif
   return true;
}


void Smartbody_dll::RegisterCallbacks()
{
  // place any additional commands here...

}
