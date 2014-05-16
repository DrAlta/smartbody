
#include "vhcl.h"

#ifndef __native_client__
#ifndef SB_NO_VHMSG
#include "vhmsg-tt.h"
#endif
#endif

#include "smartbody-dll.h"

#ifdef WIN_BUILD
#include <windows.h>
#include <mmsystem.h>
#endif

#pragma warning(push)
#pragma warning(disable:4121)  // needed for boost::python::extract<std::string>() below
#include "sb/SBScene.h"

#include "sbm/mcontrol_callbacks.h"
#include "sb/SBPython.h"
#include "sb/SBCharacter.h"
#include "sb/SBSkeleton.h"
#include "sb/SBAssetManager.h"
#include "sb/SBSpeechManager.h"
#include "sb/SBSimulationManager.h"
#include "sb/SBVHMsgManager.h"
#include "sb/SBSceneListener.h"
#include "sb/SBJointMap.h"
#include "sb/SBJointMapManager.h"
#include <sbm/remote_speech.h>
#include <sbm/local_speech.h>
#include <sbm/text_speech.h>
#include <sbm/sbm_speech_audiofile.hpp>
#pragma warning(pop)


#include "sb/SBDebuggerServer.h"


using std::string;


#if (NACL_BUILD) 
#define SB_NO_PYTHON  1
#endif


class Smartbody_dll_SBSceneListener_Internal : public SmartBody::SBSceneListener
{
   private:
      Smartbody_dll * m_dll;

   public:
      explicit Smartbody_dll_SBSceneListener_Internal( Smartbody_dll * dll ) :
         m_dll( dll )
      {
      }

      virtual ~Smartbody_dll_SBSceneListener_Internal()
      {
      }

      virtual void OnCharacterCreate( const string & name, const string & objectClass )
      {
      }

      virtual void OnCharacterDelete( const string & name )
      {
         {
            std::map<std::string,SmartbodyCharacter*>::iterator mi = m_dll->m_characters.find(name);
            SmartbodyCharacter* pc = NULL;
            if (mi != m_dll->m_characters.end())
            {
               pc = mi->second;
               delete pc;
               m_dll->m_characters.erase(mi);
            }
         }
      }

      virtual void OnPawnCreate( const std::string & name )
      {
      }

      virtual void OnViseme( const string & name, const string & visemeName, const float weight, const float blendTime )
      {
      }

      virtual void OnChannel( const string & name, const string & channelName, const float value )
      {
      }
};

SBAPI Smartbody_dll::Smartbody_dll() :
   m_internalListener( NULL )
{
   vhcl::Log::g_log.AddListener( new vhcl::Log::DebuggerListener() );
}


SBAPI Smartbody_dll::~Smartbody_dll()
{
}


SBAPI bool Smartbody_dll::Init(const std::string& pythonLibPath, bool logToFile)
{
   m_internalListener = new Smartbody_dll_SBSceneListener_Internal( this );
   
   //XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU


   // TODO: Replace with g_scene->addSceneListener(m_internalListener)
   SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
   scene->addSceneListener(m_internalListener);

   //SetSpeechAudiofileBasePath( "../../" );

   initPython(pythonLibPath);

   InitVHMsg();
   InitLocalSpeechRelay();

   srArgBuffer arg_buf( "" );
   mcu_vrAllCall_func( arg_buf, scene->getCommandManager() );

   if (logToFile)
   {
      scene->startFileLogging("./smartbody.log");
   }

   return true;
}


void Smartbody_dll::InitLocalSpeechRelay()
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

	SmartBody::SBScene::getScene()->getSpeechManager()->festivalRelay()->initSpeechRelay(festivalLibDir, festivalCacheDir);
	SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->initSpeechRelay(cereprocLibDir, festivalCacheDir);
}

SBAPI bool Smartbody_dll::Shutdown()
{
#ifndef SB_NO_VHMSG
   {
	   SmartBody::SBScene::getScene()->getVHMsgManager()->send("vrProcEnd sbm");
   }

   //SmartBody::SBScene::getScene()->getVHMsgManager()->setEnable(false);
   vhmsg::ttu_close();
#endif
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


SBAPI bool Smartbody_dll::LoadSkeleton( const void * data, int sizeBytes, const char * skeletonName )
{
    int ret = SmartBody::SBScene::getScene()->getAssetManager()->load_skeleton( data, sizeBytes, skeletonName );
    return ret == CMD_SUCCESS;
}


SBAPI bool Smartbody_dll::LoadMotion( const void * data, int sizeBytes, const char * motionName )
{
	int ret = SmartBody::SBScene::getScene()->getAssetManager()->load_motion( data, sizeBytes, motionName );
    return ret == CMD_SUCCESS;
}


SBAPI bool Smartbody_dll::Update( const double timeInSeconds )
{
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
	sim->setTime(timeInSeconds);
	//sim->stepDt(timeInSeconds);
	static double prevTime = -1;
	//if (timeInSeconds != prevTime)
	//if (timeInSeconds > 0.0)
	//{
		//LOG("SmartBody DLL Update, dt = %f, time = %f",timeInSeconds, sim->getTime());
		//prevTime = timeInSeconds;	
	scene->update();	
	//}   
	return true;
}


SBAPI void Smartbody_dll::SetDebuggerId( const std::string & id )
{
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

	scene->getDebuggerServer()->SetID( id );
}


SBAPI void Smartbody_dll::SetDebuggerCameraValues( double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar )
{
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.x = x;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.y = y;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraPos.z = z;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.x = rx;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.y = ry;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.z = rz;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraRot.w = rw;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraFovY   = fov;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraAspect = aspect;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZNear  = zNear;
   SmartBody::SBScene::getScene()->getDebuggerServer()->m_cameraZFar   = zFar;
}


SBAPI void Smartbody_dll::SetDebuggerRendererRightHanded( bool enabled )
{
  SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

   scene->getDebuggerServer()->m_rendererIsRightHanded = enabled;
}


SBAPI bool Smartbody_dll::ProcessVHMsgs( const char * op, const char * args )
{
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

   string s = string(op) + string(" ") + string(args);
   scene->command( s.c_str() );

   scene->getDebuggerServer()->ProcessVHMsgs(op, args);

   return true;
}

SBAPI bool Smartbody_dll::ExecutePython( const char * command )
{
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
	return scene->run(command);
}


SBAPI SmartbodyCharacter& Smartbody_dll::GetCharacter( const string & name )
{
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

   SmartBody::SBCharacter * char_p = scene->getCharacter(name);
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
            jointName = j->jointName();

         if (hasChar)
         {
            SmartbodyJoint& joint = c.m_joints[i];
            joint.m_name = jointName;			
			SrQuat jointQ = j->quat()->value();
			
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
#if defined(WIN_BUILD) || defined(MAC_BUILD)

   SmartBody::SBScene * scene = SmartBody::SBScene::getScene();

   printf( "Starting VHMsg (DLL side)\n" );

   // TODO: need scene->SetVhmsgEnabled(true)
   scene;
   SmartBody::SBVHMsgManager* vhmsgManager = scene->getVHMsgManager();
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
   SmartBody::SBScene::getScene()->getVHMsgManager()->setEnable(true);

#endif
   return true;
}


SBAPI bool Smartbody_dll::PythonCommandVoid( const std::string & command )
{
#ifndef SB_NO_PYTHON
	SmartBody::SBScene * scene = SmartBody::SBScene::getScene();
	return scene->run(command) == 1 ? true : false;
#else
	return false;
#endif
}

bool Smartbody_dll::PythonCommandBool( const std::string & command )
{
#ifndef SB_NO_PYTHON
   try
   {
      boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command.c_str(), mainDict);
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


int Smartbody_dll::PythonCommandInt( const std::string & command )
{
#ifndef SB_NO_PYTHON
   try
   {
		boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
		boost::python::object obj = boost::python::exec(command.c_str(), mainDict);
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

float Smartbody_dll::PythonCommandFloat( const std::string & command )
{
#ifndef SB_NO_PYTHON
   try
   {
	  boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command.c_str(), mainDict);
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

std::string Smartbody_dll::PythonCommandString( const std::string & command )
{
#ifndef SB_NO_PYTHON
   try
   {
	  boost::python::object mainDict = SmartBody::SBScene::getScene()->getPythonMainDict();
      boost::python::object obj = boost::python::exec(command.c_str(), mainDict);
      std::string result = boost::python::extract<std::string>(mainDict["ret"]);
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
