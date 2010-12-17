
#include "vhcl.h"

#include "smartbody-dll.h"

#include <windows.h>
#include <mmsystem.h>

#include "sbm/xercesc_utils.hpp"
#include "sbm/mcontrol_util.h"
#include "sbm/mcontrol_callbacks.h"
#include "sbm/sbm_character.hpp"
#include "sbm/sbm_test_cmds.hpp"
#include "sbm/resource_cmds.h"
#include "sbm/locomotion_cmds.hpp"

using std::string;


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

   return TRUE;
}



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
};


// static function taken from sbm_main.cpp
int sbm_main_func( srArgBuffer & args, mcuCBHandle * mcu_p )
{
   const char * token = args.read_token();
   if ( strcmp( token, "id" ) == 0 )
   {  // Process specific
      token = args.read_token(); // Process id
      const char * process_id = mcu_p->process_id.c_str();
      if( ( mcu_p->process_id == "" )         // If process id unassigned
         || strcmp( token, process_id ) !=0 ) // or doesn't match
         return CMD_SUCCESS;                  // Ignore.
      token = args.read_token(); // Sub-command
   }

   const char * args_raw = args.read_remainder_raw();
   int result = mcu_p->execute( token, srArgBuffer( args_raw ) );
   switch( result )
   {
      case CMD_NOT_FOUND:
         fprintf( stdout, "SBM ERR: command NOT FOUND: '%s %s'\n> ", token, args_raw );
         break;
      case CMD_FAILURE:
         fprintf( stdout, "SBM ERR: command FAILED: '%s %s'\n> ", token, args_raw );
         break;
      case CMD_SUCCESS:
         break;
      default:
         break;
   }

   return CMD_SUCCESS;
}


// static function taken from sbm_main.cpp
int mcu_echo_func( srArgBuffer & args, mcuCBHandle * mcu_p )
{
   fprintf( stdout, "%s\n> ", args.read_remainder_raw() );

   return CMD_SUCCESS;
}


// static function taken from sbm_main.cpp
int mcu_reset_func( srArgBuffer & args, mcuCBHandle * mcu_p )
{
   // TODO: If arg, call as init, else call previous init
   mcu_p->reset();
   return CMD_SUCCESS;
}


// static function taken from sbm_main.cpp
int sbm_vhmsg_send_func( srArgBuffer & args, mcuCBHandle * mcu_p )
{
   const char * cmdName = args.read_token();
   const char * cmdArgs = args.read_remainder_raw();
   return mcu_p->vhmsg_send( cmdName, cmdArgs );
}


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
   mcu.speech_audiofile_base_path = basePath;
}


SMARTBODY_DLL_API void Smartbody_dll::SetFacebone( const bool enabled )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.net_face_bones = enabled;
}


SMARTBODY_DLL_API void Smartbody_dll::SetProcessId( const std::string & processId )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.set_process_id( processId.c_str() );
}


SMARTBODY_DLL_API void Smartbody_dll::SetMediaPath( const std::string & path )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.setMediaPath( path );
}


SMARTBODY_DLL_API bool Smartbody_dll::Init()
{
   m_internalListener = new Smartbody_dll_SBMCharacterListener_Internal( this );

   XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU

   mcuCBHandle & mcu = mcuCBHandle::singleton();
   mcu.sbm_character_listener = m_internalListener;
   SetMediaPath( "../../data/art" );
   SetSpeechAudiofileBasePath( "../../" );

   InitVHMsg();
   RegisterCallbacks();


   mcu_vrAllCall_func( srArgBuffer(""), &mcu );

   vhcl::Log::Listener* listener = new vhcl::Log::FileListener("./smartbody.log");
   vhcl::Log::g_log.AddListener(listener);

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

   bool update_sim = mcu.update_timer( timeInSeconds );
   if( update_sim ) mcu.update();
   return true;
}


SMARTBODY_DLL_API bool Smartbody_dll::ProcessVHMsgs( const char * op, const char * args )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu.execute( op, (char *)args );

   return true;
}


SMARTBODY_DLL_API int Smartbody_dll::GetNumberOfCharacters()
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   return mcu.character_map.get_num_entries();
}


SMARTBODY_DLL_API SmartbodyCharacter Smartbody_dll::GetCharacter( const string & name )
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   SmartbodyCharacter c;

   const SbmCharacter * char_p = mcu.character_map.lookup( name );
   if ( char_p )
   {
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

      c.m_name = char_p->name;
      c.x = x;
      c.y = y;
      c.z = z;
      c.rw = q.w;
      c.rx = q.x;
      c.ry = q.y;
      c.rz = q.z;


      const SrArray<SkJoint *> & joints  = char_p->skeleton_p->joints();

      for ( int i = 0; i < joints.size(); i++ )
      {
         // const_cast because the SrQuat does validation (no const version of value())
         SkJoint * j = (SkJoint *)joints.const_get( i );

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


         SmartbodyJoint joint;
         joint.m_name = j->name();
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


bool Smartbody_dll::InitVHMsg()
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   printf( "Starting VHMsg (DLL side)\n" );

   vhmsg::ttu_open();

   mcu.vhmsg_enabled = true;

   return true;
}


void Smartbody_dll::RegisterCallbacks()
{
   mcuCBHandle & mcu = mcuCBHandle::singleton();

   mcu.insert( "sbm",   sbm_main_func );
   mcu.insert( "help",			mcu_help_func );

   mcu.insert( "reset", mcu_reset_func );
   mcu.insert( "echo",  mcu_echo_func );

   mcu.insert( "path",      mcu_filepath_func );
   mcu.insert( "seq",       mcu_sequence_func );
   mcu.insert( "seq-chain", mcu_sequence_chain_func );
   mcu.insert( "send",      sbm_vhmsg_send_func );

   mcu.insert( "set",   mcu_set_func );
   mcu.insert( "print", mcu_print_func );
   mcu.insert( "test",  mcu_test_func );

   mcu.insert( "viewer", mcu_viewer_func );
   mcu.insert( "bmlviewer", mcu_bmlviewer_func );
   mcu.insert( "panimviewer",  mcu_panimationviewer_func);
   mcu.insert( "cbufviewer", mcu_channelbufferviewer_func);
   mcu.insert( "camera", mcu_camera_func );
   mcu.insert( "terrain",	mcu_terrain_func );
   mcu.insert( "time",   mcu_time_func );
   mcu.insert( "tip",	mcu_time_ival_prof_func );

   mcu.insert( "panim",		mcu_panim_keys_func );	

   mcu.insert( "load",   mcu_load_func );
   mcu.insert( "pawn",   SbmPawn::pawn_cmd_func );
   mcu.insert( "char",   SbmCharacter::character_cmd_func );

   mcu.insert( "ctrl",      mcu_controller_func );
   mcu.insert( "sched",     mcu_sched_controller_func );
   mcu.insert( "motion",    mcu_motion_controller_func );
   mcu.insert( "stepturn",  mcu_stepturn_controller_func );
   mcu.insert( "quickdraw", mcu_quickdraw_controller_func );
   mcu.insert( "gaze",      mcu_gaze_controller_func );
   mcu.insert( "gazelimit",	mcu_gaze_limit_func );
   mcu.insert( "snod",      mcu_snod_controller_func );
   mcu.insert( "lilt",      mcu_lilt_controller_func );
   mcu.insert( "divulge",   mcu_divulge_content_func );
   mcu.insert( "wsp",       mcu_wsp_cmd_func );
   mcu.insert( "create_remote_pawn",	SbmPawn::create_remote_pawn_func );

   mcu.insert( "vrAgentBML",  BML_PROCESSOR::vrAgentBML_cmd_func );
   mcu.insert( "bp",          BML_PROCESSOR::bp_cmd_func );
   mcu.insert( "vrSpeak",     BML_PROCESSOR::vrSpeak_func );

   mcu.insert( "net_reset",           mcu_net_reset );
   mcu.insert( "RemoteSpeechReply",   remoteSpeechResult_func );
   mcu.insert( "RemoteSpeechTimeOut", remoteSpeechTimeOut_func);  // internally routed message
   mcu.insert( "joint_logger",        joint_logger::start_stop_func );
   mcu.insert( "J_L",                 joint_logger::start_stop_func );  // shorthand
   mcu.insert( "locomotion",          locomotion_cmd_func );
   mcu.insert( "loco",                locomotion_cmd_func ); // shorthand
   mcu.insert( "resource",	resource_cmd_func );
   mcu.insert( "syncpolicy",			mcu_syncpolicy_func );
   mcu.insert( "check",                mcu_check_func ); // shorthand
   mcu.insert( "mediapath",		   mcu_mediapath_func);

   mcu.insert( "RemoteSpeechReplyRecieved", remoteSpeechReady_func);  // TODO: move to test commands

   mcu.insert_set_cmd( "bp",             BML_PROCESSOR::set_func );
   mcu.insert_set_cmd( "pawn",           SbmPawn::set_cmd_func );
   mcu.insert_set_cmd( "character",      SbmCharacter::set_cmd_func );
   mcu.insert_set_cmd( "char",           SbmCharacter::set_cmd_func );
   mcu.insert_set_cmd( "face",           mcu_set_face_func );
   mcu.insert_set_cmd( "joint_logger",   joint_logger::set_func );
   mcu.insert_set_cmd( "J_L",            joint_logger::set_func );  // shorthand
   mcu.insert_set_cmd( "test",           sbm_set_test_func );

   mcu.insert_print_cmd( "bp",           BML_PROCESSOR::print_func );
   mcu.insert_print_cmd( "pawn",         SbmPawn::print_cmd_func );
   mcu.insert_print_cmd( "character",    SbmCharacter::print_cmd_func );
   mcu.insert_print_cmd( "char",         SbmCharacter::print_cmd_func );
   mcu.insert_print_cmd( "face",         mcu_print_face_func );
   mcu.insert_print_cmd( "joint_logger", joint_logger::print_func );
   mcu.insert_print_cmd( "J_L",          joint_logger::print_func );  // shorthand
   mcu.insert_print_cmd( "mcu",          mcu_divulge_content_func );
   mcu.insert_print_cmd( "test",         sbm_print_test_func );

   mcu.insert_test_cmd( "args", test_args_func );
   mcu.insert_test_cmd( "bml",  test_bml_func );
   mcu.insert_test_cmd( "fml",  test_fml_func );
   mcu.insert_test_cmd( "locomotion", test_locomotion_cmd_func );
   mcu.insert_test_cmd( "loco",       test_locomotion_cmd_func );  // shorthand
   mcu.insert_test_cmd( "rhet", remote_speech_test);
   mcu.insert_test_cmd( "bone_pos", test_bone_pos_func );

   mcu.insert( "net", mcu_net_func );

   mcu.insert( "PlaySound", mcu_play_sound_func );
   mcu.insert( "StopSound", mcu_stop_sound_func );

   mcu.insert( "uscriptexec", mcu_uscriptexec_func );

   mcu.insert( "CommAPI", mcu_commapi_func );

   mcu.insert( "vrKillComponent", mcu_vrKillComponent_func );
   mcu.insert( "vrAllCall",       mcu_vrAllCall_func );

   mcu.insert( "text_speech", text_speech::text_speech_func ); // [BMLR]
}
