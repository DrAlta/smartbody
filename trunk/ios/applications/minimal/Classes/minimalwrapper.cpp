//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "minimalwrapper.h"
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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <sr/sr_camera.h>
#include "vhmsg.h"

XERCES_CPP_NAMESPACE_USE
using namespace boost::filesystem;

#if __cplusplus
extern "C"
{
#endif
   
    
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
    
    
    void mcu_register_callbacks() 
    {
        mcuCBHandle& mcu = mcuCBHandle::singleton();
        
        
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
    }    
    
    void sbm_vhmsg_callback( const char *op, const char *args, void * user_data )
    {
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
    
void MCUInitialize()
{
    XMLPlatformUtils::Initialize();
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu_register_callbacks();
    
    const char* serverName = "172.16.33.21";
    const char* scope = "DEFAULT_SCOPE";
    const char* port = "61616";
    int err;
    int openConnection = vhmsg::ttu_open(serverName,scope,port);
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
        LOG("TTU Open Success : server = %s, scope = %s, port = %s",serverName,scope,port);
    }
    else
    {
        LOG("TTU Open Failed : server = %s, scope = %s, port = %s",serverName,scope,port);
    }    
}
        
void SBMInitialize(const char* mediaPath)
{
    SBMExecuteCmd("char brad init common.sk"); 
    SBMExecuteCmd("set character brad world_offset x -35 y 102 h -17");
    SBMExecuteCmd("char doctor init common.sk");
    SBMExecuteCmd("set character doctor world_offset x 35 y 102 h -17");    
    SBMExecuteCmd("bml char brad <body posture=\"HandsAtSide_Motex\"/>");
    SBMExecuteCmd("bml char doctor <body posture=\"HandsAtSide_Motex\"/>");
}
    
void SBMLoad(const char* p)
{
    path pathname(p);
    std::string command = "load skeletons -R " + pathname.root_directory();
    printf("loading command is %s\n", command.c_str());
    SBMExecuteCmd(command.c_str());
}
    
void getBoneData()
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmCharacter* char_p = mcu.getCharacter("brad");
    SkSkeleton* sk = char_p->_skeleton;
    SkSkeleton* sk1 = mcu.getCharacter("doctor")->_skeleton;
    sk->update_global_matrices(); 
    sk1->update_global_matrices();
    std::map<int,int> indexMap;
    numJoints = sk->joints().size();
    for (int i=0;i<sk->joints().size();i++)
    {
        SkJoint* joint = sk->joints()[i];
        SkJoint* joint1 = sk1->joints()[i];
        SrVec pos = joint->gmat().get_translation();
        SrVec pos1 = joint1->gmat().get_translation();
        jointPos[i * 3 + 0] = pos.x;
        jointPos[i * 3 + 1] = pos.y;
        jointPos[i * 3 + 2] = pos.z;
        jointPos1[i * 3 + 0] = pos1.x;
        jointPos1[i * 3 + 1] = pos1.y;
        jointPos1[i * 3 + 2] = pos1.z;        
        indexMap[joint->index()] = i;
        boneIdx[i*2+0] = joint->index();
        if (joint->parent())
            boneIdx[i*2+1] = joint->parent()->index();
        else
            boneIdx[i*2+1] = joint->index();
    }
   
    for (int i=0;i<sk->joints().size();i++)
    {
        boneIdx[i*2] = indexMap[boneIdx[i*2]];
        boneIdx[i*2+1] = indexMap[boneIdx[i*2+1]];
    }
}
    
void SBMUpdateX(float t)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    bool updateSim = mcu.update_timer(t);
    vhmsg::ttu_poll();
    if (updateSim)        
        mcu.update();
}
    
void SBMExecuteCmd(const char* command)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.execute((char*) command);
}
    
void SBMExecutePythonCmd(const char* command)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.executePython((char*) command);    
}
    
float SBMGetCharacterWo(const char* character)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmPawn* c = mcu.getCharacter(character);
    if (!c)
        return -1.0f;
    
    float x, y, z, p, h, r;
    c->get_world_offset(x, y, z, p, h, r);
    return y;
}
    
    float* rotatePoint(float* pointF, float* originF, float* directionF, float angle)
    {
        SrVec point = SrVec(pointF[0], pointF[1], pointF[2]);
        SrVec origin = SrVec(originF[0], originF[1], originF[2]);
        SrVec direction = SrVec(directionF[0], directionF[1], directionF[2]);
        SrVec v = direction;
        SrVec o = origin;
        SrVec p = point;
        float c = cos(angle);
        float s = sin(angle);
        float C = 1.0f - c;
        
        SrMat mat;
        mat.e11() = v[0] * v[0] * C + c;
        mat.e12() = v[0] * v[1] * C - v[2] * s;
        mat.e13() = v[0] * v[2] * C + v[1] * s;
        mat.e21() = v[1] * v[0] * C + v[2] * s;
        mat.e22() = v[1] * v[1] * C + c;
        mat.e23() = v[1] * v[2] * C - v[0] * s;
        mat.e31() = v[2] * v[0] * C - v[1] * s;
        mat.e32() = v[2] * v[1] * C + v[0] * s;
        mat.e33() = v[2] * v[2] * C + c;
        
        mat.transpose();
        
        SrVec result = origin + mat * (point - origin);
        return result;
    }
    
void getCamera(float x, float y, float prevX, float prevY, float curX, float curY, int mode)
{
    static SrCamera _camera;
    if (mode == -1)
    {
        _camera.init();
        _camera.up = SrVec::j;    
        _camera.center.set(0, 92, 0);
        _camera.eye.set(0, 166, 300);
        _camera.scale = 1.0f;
    }   
    if (mode == 0)  // dollying
    {
        printf("camera dollying, %f, %f\n", x, y);
        float amount = x / 100.0f;
        SrVec cameraPos(_camera.eye);
        SrVec targetPos(_camera.center);
        SrVec diff = targetPos - cameraPos;
        float distance = diff.len();
        diff.normalize();
        
        if (amount >= distance)
            amount = distance - .000001f;
        
        SrVec diffVector = diff;
        SrVec adjustment = diffVector * distance * amount;
        cameraPos += adjustment;
        SrVec oldEyePos = _camera.eye;
        _camera.eye = cameraPos;
        SrVec cameraDiff = _camera.eye - oldEyePos;
        _camera.center += cameraDiff;
     }
    
    if (mode == 1)  // camera rotation
    {   
 		float deltaX = -x / swidth;
		float deltaY = -y / sheight;
        deltaX *= 3.0f;
        deltaY *= 3.0f;

		SrVec origUp = _camera.up;
		SrVec origCenter = _camera.center;
		SrVec origCamera = _camera.eye;
        
		SrVec dirX = origUp;
		SrVec  dirY;
		dirY.cross(origUp, (origCenter - origCamera));
		dirY /= dirY.len();
        
		SrVec camera = rotatePoint(origCamera, origCenter, dirX, -deltaX * float(M_PI));
		camera = rotatePoint(camera, origCenter, dirY, deltaY * float(M_PI));
        
		_camera.eye = camera;    
    }
        
    
    SrMat mat;
    _camera.get_perspective_mat(mat);
    SrMat mat1;
    _camera.get_view_mat(mat1);
    for (int i = 0; i < 16; i++)
    {
        projection[i] = mat.get(i);
        modelview[i] = mat1.get(i);
    }
}
    
#if __cplusplus
}
#endif