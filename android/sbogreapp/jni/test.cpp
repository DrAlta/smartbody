//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "test.h"
#include <sbm/mcontrol_util.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/resource_cmds.h>
#include <sbm/sbm_test_cmds.hpp>
#include <sbm/locomotion_cmds.hpp>
#include <sr/sr_camera.h>


#define ANDROID_PYTHON
#ifdef ANDROID_PYTHON
#include <sb/SBPython.h>
#endif

//XERCES_CPP_NAMESPACE_USE

/*
int sbm_main_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	
{
    
    const char* token = args.read_token();
    if( strcmp(token,"id")==0 ) {  // Process specific
        token = args.read_token(); // Process id
        const char* process_id = mcu_p->process_id.c_str();
        if( ( mcu_p->process_id == "" )        // If process id unassigned
           || strcmp( token, process_id )!=0 ) // or doesn't match
            return CMD_SUCCESS;                 // Ignore.
        token = args.read_token(); // Sub-command
    }
    
    const char* args_raw = args.read_remainder_raw();
    srArgBuffer argsRawBuff(args_raw);
    int result = mcu_p->execute( token, argsRawBuff );
    switch( result ) {
        case CMD_NOT_FOUND:
            LOG("SBM ERR: command NOT FOUND: '%s %s' ", token, args_raw );
            break;
        case CMD_FAILURE:
            LOG("SBM ERR: command FAILED: '%s %s' ", token, args_raw );
            break;
        case CMD_SUCCESS:
            break;
    }
    return CMD_SUCCESS;
}
*/

    void mcu_register_callbacks() 
    {

        mcuCBHandle& mcu = mcuCBHandle::singleton();
/*
        mcu.insert( "sbm",			sbm_main_func );
        mcu.insert( "help",			mcu_help_func );
        
        mcu.insert( "path",			mcu_filepath_func );
        mcu.insert( "seq",			mcu_sequence_func );
        mcu.insert( "seq-chain",	mcu_sequence_chain_func );
        
        
        //  cmd prefixes "set" and "print"
        mcu.insert( "set",          mcu_set_func );
        mcu.insert( "print",        mcu_print_func );
        mcu.insert( "test",			mcu_test_func );
        
        mcu.insert( "viewer",		mcu_viewer_func );
        mcu.insert( "bmlviewer",    mcu_bmlviewer_func);
        mcu.insert( "panimviewer",  mcu_panimationviewer_func);
        mcu.insert( "cbufviewer",	mcu_channelbufferviewer_func); // deprecated
        mcu.insert( "dataviewer",	mcu_channelbufferviewer_func);
        mcu.insert( "resourceviewer",	mcu_resourceViewer_func);	
        mcu.insert( "faceviewer",	mcu_faceViewer_func);
        
        mcu.insert( "camera",		mcu_camera_func );
        mcu.insert( "terrain",		mcu_terrain_func );
        mcu.insert( "time",			mcu_time_func );
        mcu.insert( "tip",			mcu_time_ival_prof_func );
        
        mcu.insert( "panim",		mcu_panim_cmd_func );	
        mcu.insert( "physics",		mcu_physics_cmd_func );	
        mcu.insert( "mirror",       mcu_motion_mirror_cmd_func);
        mcu.insert( "motionplayer", mcu_motion_player_func);
        
        mcu.insert( "load",			mcu_load_func );
        mcu.insert( "pawn",			SbmPawn::pawn_cmd_func );
        mcu.insert( "char",			SbmCharacter::character_cmd_func );
        
        mcu.insert( "ctrl",			mcu_controller_func );
        mcu.insert( "sched",		mcu_sched_controller_func );
        mcu.insert( "motion",		mcu_motion_controller_func );
        mcu.insert( "stepturn",		mcu_stepturn_controller_func );
        mcu.insert( "quickdraw",	mcu_quickdraw_controller_func );
        mcu.insert( "gaze",			mcu_gaze_controller_func );	
        mcu.insert( "gazelimit",	mcu_gaze_limit_func );
        mcu.insert( "snod",			mcu_snod_controller_func );
        mcu.insert( "lilt",			mcu_lilt_controller_func );
        mcu.insert( "divulge",		mcu_divulge_content_func );
        mcu.insert( "wsp",			mcu_wsp_cmd_func );
        mcu.insert( "create_remote_pawn", SbmPawn::create_remote_pawn_func );
        
        mcu.insert( "vrAgentBML",   BML_PROCESSOR::vrAgentBML_cmd_func );
        mcu.insert( "bp",		    BML_PROCESSOR::bp_cmd_func );
        mcu.insert( "vrSpeak",		BML_PROCESSOR::vrSpeak_func );
        mcu.insert( "vrExpress",  mcu_vrExpress_func );
        
        mcu.insert( "receiver",		mcu_joint_datareceiver_func );
        
        mcu.insert( "net_reset",           mcu_net_reset );
        mcu.insert( "net_check",           mcu_net_check );
        mcu.insert( "RemoteSpeechReply",   remoteSpeechResult_func );
        mcu.insert( "RemoteSpeechTimeOut", remoteSpeechTimeOut_func);  // internally routed message
        mcu.insert( "joint_logger",        joint_logger::start_stop_func );
        mcu.insert( "J_L",                 joint_logger::start_stop_func );  // shorthand
        //mcu.insert( "locomotion",          locomotion_cmd_func );
        //mcu.insert( "loco",                locomotion_cmd_func ); // shorthand
        mcu.insert( "resource",            resource_cmd_func );
        mcu.insert( "syncpolicy",          mcu_syncpolicy_func );
        mcu.insert( "check",			   mcu_check_func);		// check matching between .skm and .sk
        mcu.insert( "python",			   mcu_python_func);
        mcu.insert( "adjustmotion",		   mcu_adjust_motion_function);
        mcu.insert( "mediapath",		   mcu_mediapath_func);
        mcu.insert( "bml",				   test_bml_func );
        mcu.insert( "triggerevent",		   triggerevent_func );
        mcu.insert( "addevent",			   addevent_func );
        mcu.insert( "removeevent",		   removeevent_func );
        mcu.insert( "enableevents",	       enableevents_func );
        mcu.insert( "disableevents",	   disableevents_func );
        mcu.insert( "registerevent",       registerevent_func );
        mcu.insert( "unregisterevent",     unregisterevent_func );
        mcu.insert( "setmap",			   setmap_func );
        mcu.insert( "motionmap",		   motionmap_func );
        mcu.insert( "skeletonmap",		   skeletonmap_func );
        mcu.insert( "steer",			   mcu_steer_func);	
        mcu.insert( "characters",		   showcharacters_func );
        mcu.insert( "pawns",			   showpawns_func );
        mcu.insert( "RemoteSpeechReplyRecieved", remoteSpeechReady_func);  // TODO: move to test commands
        mcu.insert( "syncpoint",		   syncpoint_func);
        mcu.insert( "pawnbonebus",		   pawnbonebus_func);
#ifdef USE_GOOGLE_PROFILER
        mcu.insert( "startprofile",			   startprofile_func );
        mcu.insert( "stopprofile",			   stopprofile_func );
#endif
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
        
        
        mcu.insert( "net",	mcu_net_func );
        
        mcu.insert( "PlaySound", mcu_play_sound_func );
        mcu.insert( "StopSound", mcu_stop_sound_func );
        
        mcu.insert( "uscriptexec", mcu_uscriptexec_func );
        
        mcu.insert( "CommAPI", mcu_commapi_func );
        
        mcu.insert( "vrKillComponent", mcu_vrKillComponent_func );
        mcu.insert( "vrAllCall", mcu_vrAllCall_func );
        mcu.insert("vrQuery", mcu_vrQuery_func );
        
        mcu.insert( "text_speech", text_speech::text_speech_func ); // [BMLR]
*/
    }    
  
void sbm_vhmsg_callback( const char *op, const char *args, void * user_data )
{
	LOG("vhmsg callback op=%s, args=%s\n",op,args);

    // Replace singleton with a user_data pointer
    switch( mcuCBHandle::singleton().execute( op, (char *)args ) ) {
        case CMD_NOT_FOUND:
            LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
            break;
        case CMD_FAILURE:
            LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
            break;
    }
} 

void endConnection()
{
	vhmsg::ttu_close();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.vhmsg_enabled = false;
	LOG("TTU Closed, Disable VHMsg.");
}

void initConnection(const char* serverName, const char* portName)
{    
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    const char* scope = "DEFAULT_SCOPE";    
    int err;
    int openConnection = vhmsg::ttu_open(serverName,scope,portName);
    if( openConnection == vhmsg::TTU_SUCCESS )
    {
        vhmsg::ttu_set_client_callback( sbm_vhmsg_callback );
        err = vhmsg::ttu_register( "sbm" );
        err = vhmsg::ttu_register( "vrAgentBML" );
        err = vhmsg::ttu_register( "vrExpress" );
        err = vhmsg::ttu_register( "vrSpeak" );
        err = vhmsg::ttu_register( "RemoteSpeechReply" );
        err = vhmsg::ttu_register( "PlaySound" );
        err = vhmsg::ttu_register( "StopSound" );
        err = vhmsg::ttu_register( "CommAPI" );
        err = vhmsg::ttu_register( "object-data" );
        err = vhmsg::ttu_register( "vrAllCall" );
        err = vhmsg::ttu_register( "vrKillComponent" );
        err = vhmsg::ttu_register( "wsp" );
        err = vhmsg::ttu_register( "receiver" );
        mcu.vhmsg_enabled = true;
        LOG("TTU Open Success : server = %s, scope = %s, port = %s",serverName,scope,portName);
    }
    else
    {
        LOG("TTU Open Failed : server = %s, scope = %s, port = %s",serverName,scope,portName);
    }    

}

void initSBPython()
{
#ifdef ANDROID_PYTHON
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	//LOGI("Before init Python");
	initPython(python_lib_path);
#endif
}


void MCUInitialize()
{
    XMLPlatformUtils::Initialize();
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu_register_callbacks();

    LOG("Init MCU");
    const char* serverName = "172.16.33.21";
    const char* port = "61616";
    LOG("Before Init Connection");
    initConnection(serverName,port);
    initSBPython();
    //SBMExecuteCmd("time resume");
}
        
void SBInitialize(const char* mediaPath)
{
    LOG("before add asset path");
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.executePython("scene.addAssetPath('script', '/sdcard/sbogredir/')");
    LOG("before execute python file");
    mcu.executePythonFile("initOgre.py");
    LOG("after execute python file");


	//SBMExecuteCmd("path seq /sdcard/SbmOgre/");
	//SBMExecuteCmd("seq initOgre.seq");
	/*
    SBMExecuteCmd("char brad init common.sk"); 
    SBMExecuteCmd("set character brad world_offset x -35 y 102 h -17");
    SBMExecuteCmd("char doctor init common.sk");
    SBMExecuteCmd("set character doctor world_offset x 35 y 102 h -17");    
    SBMExecuteCmd("bml char brad <body posture=\"HandsAtSide_Motex\"/>");
    SBMExecuteCmd("bml char doctor <body posture=\"HandsAtSide_Motex\"/>");
    */
}
       
void SBUpdateX(float t)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    bool updateSim = mcu.update_timer(t);
    
    int err = vhmsg::ttu_poll();
    if( err == vhmsg::TTU_ERROR )	
    {
        fprintf( stderr, "ttu_poll ERROR\n" );
    }    
    
    if (updateSim)        
        mcu.update();
}
    
void SBExecuteCmd(const char* command)
{
    //if (!mcuInit) return;
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.executePython((char*) command);
}


void getCharacterWo(const char* name, float& x, float& y, float& z, float& qw, float& qx, float& qy, float& qz)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmCharacter* char_p = mcu.getCharacter(name);
    if (!char_p) return;
    const SkJoint * joint = char_p->get_world_offset_joint();
    const SkJointPos * pos = joint->const_pos();
    x = pos->value( SkJointPos::X );
    y = pos->value( SkJointPos::Y );
    z = pos->value( SkJointPos::Z );
    const SrQuat & q = ((SkJoint *)joint)->quat()->value();
    qw = q.w;
    qx = q.x;
    qy = q.y;
    qz = q.z;
}

void getJointRotation(const char* charname, const char* jointname, float& w, float& x, float& y, float& z)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmCharacter* char_p = mcu.getCharacter(charname);
    if (!char_p)    return;
    const SkJoint * joint = char_p->getSkeleton()->search_joint(jointname);
    if (joint)
    {
        const SrQuat & q = ((SkJoint *)joint)->quat()->value();
        w = q.w;
        x = q.x;
        y = q.y;
        z = q.z;
    }
}

void getJointInfo(const char* charname, float* positions, float* bones, std::string* jnames, int n)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmCharacter* char_p = mcu.getCharacter(charname);
    if (!char_p)    return;
    
    const SkJoint * wj = char_p->get_world_offset_joint();
    const SkJointPos * pos = wj->const_pos();
    positions[0] = pos->value( SkJointPos::X );
    positions[1] = pos->value( SkJointPos::Y );
    positions[2] = pos->value( SkJointPos::Z );
    const SrQuat & q = ((SkJoint *)wj)->quat()->value();
    positions[3] = q.w;
    positions[4] = q.x;
    positions[5] = q.y;
    positions[6] = q.z;

    for (int i = 0; i < n; i++)
    {
        std::string& name = jnames[i];
        const SkJoint * joint = char_p->getSkeleton()->search_joint(name.c_str());
        if (joint)
        {
            const SrQuat & q = ((SkJoint *)joint)->quat()->value();
            bones[i * 4 + 0] = q.w;
            bones[i * 4 + 1] = q.x;
            bones[i * 4 + 2] = q.y;
            bones[i * 4 + 3] = q.z;
        }
        else
        {
            bones[i * 4 + 0] = 1;
            bones[i * 4 + 1] = 0;
            bones[i * 4 + 2] = 0;
            bones[i * 4 + 3] = 0;        
        }
    }
}
