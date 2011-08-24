/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include "vhcl_log.h"
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include <boost/filesystem/operations.hpp>
#include <sr/sr_camera.h>
#include <sbm/SbmPython.h>

#include <jni.h>
#include <android/log.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libsbmjni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct Engine
{
	SrCamera camera;
	TimeRegulator timer;
	vhcl::Log::AndroidListener androidListener;
};

Engine engine;
bool mcuInit = false;

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

void mcu_register_callbacks( void ) {

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	//mcu.insert( "sbm",			sbm_main_func );
	mcu.insert( "help",			mcu_help_func );

	//mcu.insert( "q",			mcu_quit_func );
	//mcu.insert( "quit",			mcu_quit_func );
	//mcu.insert( "reset",		mcu_reset_func );
	//mcu.insert( "echo",			mcu_echo_func );

	mcu.insert( "path",			mcu_filepath_func );
	mcu.insert( "seq",			mcu_sequence_func );
	mcu.insert( "seq-chain",	mcu_sequence_chain_func );
	//mcu.insert( "send",			sbm_vhmsg_send_func );

	//mcu.insert( "snapshot",		mcu_snapshot_func );

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
	//mcu.insert( "resource",            resource_cmd_func );
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
	//mcu.insert_set_cmd( "test",           sbm_set_test_func );

	mcu.insert_print_cmd( "bp",           BML_PROCESSOR::print_func );
	mcu.insert_print_cmd( "pawn",         SbmPawn::print_cmd_func );
	mcu.insert_print_cmd( "character",    SbmCharacter::print_cmd_func );
	mcu.insert_print_cmd( "char",         SbmCharacter::print_cmd_func );
	mcu.insert_print_cmd( "face",         mcu_print_face_func );
	mcu.insert_print_cmd( "joint_logger", joint_logger::print_func );
	mcu.insert_print_cmd( "J_L",          joint_logger::print_func );  // shorthand
	mcu.insert_print_cmd( "mcu",          mcu_divulge_content_func );
	//mcu.insert_print_cmd( "test",         sbm_print_test_func );

	//mcu.insert_test_cmd( "args", test_args_func );
	mcu.insert_test_cmd( "bml",  test_bml_func );
	//mcu.insert_test_cmd( "fml",  test_fml_func );
	//mcu.insert_test_cmd( "locomotion", test_locomotion_cmd_func );
	//mcu.insert_test_cmd( "loco",       test_locomotion_cmd_func );  // shorthand
	mcu.insert_test_cmd( "rhet", remote_speech_test);
	//mcu.insert_test_cmd( "bone_pos", test_bone_pos_func );


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

void drawSBM(mcuCBHandle& mcu)
{
	static SrVec jointPos[200];
	static unsigned short boneIdx[400];
	//LOGI("Num of Pawns = %d\n",mcu.getPawnMap().size());
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		SbmCharacter* char_p = mcu.getCharacter(pawn->getName() );
		if( char_p )
		{
			SkSkeleton* sk = char_p->_skeleton;
			sk->update_global_matrices();
			std::map<int,int> indexMap;
			for (int i=0;i<sk->joints().size();i++)
			{
				SkJoint* joint = sk->joints()[i];
				SrVec pos = joint->gmat().get_translation();
				jointPos[i] = pos;
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

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, (const GLfloat*)&jointPos[0]);
			//glDrawArrays(GL_POINTS, 0, sk->joints().size());
			glDrawElements(GL_LINES,sk->joints().size()*2,GL_UNSIGNED_SHORT, boneIdx);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}
}


bool setupGraphics(int w, int h) {

	SrCamera& cam = engine.camera;
	cam.init();
	cam.center = SrVec(0, 92, 0);
	cam.up = SrVec::j;
	cam.eye.set ( 0, 166, 185 );
	cam.scale = 1.f;

    glViewport(0, 0, w, h);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

    return true;
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };

void renderFrame() {

	SrCamera& cam = engine.camera;

	// Just fill the screen with a color.
	glClearColor(0.03f,0.03f,0.03f,1);
	//glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
	//        ((float)engine->state.y)/engine->height, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	SrMat mat;
	glMatrixMode ( GL_PROJECTION );
	glLoadMatrixf ( (const float*)cam.get_perspective_mat(mat) );

	//----- Set Visualisation -------------------------------------------
	glMatrixMode ( GL_MODELVIEW );
	glLoadMatrixf ( (const float*)cam.get_view_mat(mat) );

	//glScalef ( cam.scale, cam.scale, cam.scale );
	//glDisable(GL_LIGHTING);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	// draw a ground plane
	float planeSize  = 300.f;
	SrVec quad[4] = { SrVec(planeSize, 0.f, planeSize), SrVec(-planeSize, 0.f, planeSize), SrVec(-planeSize,0.f,-planeSize), SrVec(planeSize, 0.f, -planeSize) };
	GLfloat quadColor[16] = { 0.2f,0.2f, 0.2f, 1.f , 0.3f,0.3f,0.3f, 1.f, 0.5f,0.5f,0.5f,1.f, 0.25f,0.25f,0.25f,1.f };
	unsigned short indices[] = {0,1,2, 0,2,3};

	glShadeModel(GL_SMOOTH);
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLfloat*)&quad[0]);
	glColorPointer(4, GL_FLOAT, 0, quadColor);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	glDisableClientState(GL_COLOR_ARRAY);
	drawSBM(mcu);
	//eglSwapBuffers(engine->display, engine->surface);    
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executeSbm(JNIEnv * env, jobject obj, jstring sbmCmd);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd);
	JNIEXPORT jstring JNICALL Java_com_android_sbmjni_SbmJNILib_getLog(JNIEnv * env, jobject obj);
};

void initPython()
{
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	LOGI("Before init Python");
	initPython(python_lib_path);
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	//LOGI("Starting Sbm Android");
	//LOGI("Initialize XMLPlatformUtils\n");
	//sleep(8);

	

	XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU
	//LOGI("Start Initialize mcu\n");
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu_register_callbacks();
	
	vhcl::Log::g_log.AddListener(&engine.androidListener);
#ifdef USE_PYTHON
	initPython();
#endif
	
	TimeRegulator& timer = engine.timer;

	mcu.register_timer( timer );
	TimeIntervalProfiler* profiler = new TimeIntervalProfiler();
	mcu.register_profiler(*profiler);

    mcu.execute("path seq /sdcard/sbmmedia/");
    mcu.execute("seq default.seq");
    //mcu.execute("load skeletons -R /sdcard/sbmmedia/");
    //mcu.execute("load motions -R /sdcard/sbmmedia/");
    //mcu.execute("char doctor init common.sk");
    //mcu.execute("bml char doctor <body posture=\"test\"/>");
	mcu.execute("pawn foo init");
	timer.start();	
    setupGraphics(width, height);

	mcuInit = true;
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_step(JNIEnv * env, jobject obj)
{
	if (!mcuInit) return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	bool update_sim = mcu.update_timer();		
	if( update_sim ) {
		mcu.update();
	}
    renderFrame();
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executeSbm(JNIEnv * env, jobject obj, jstring sbmCmd)
{
	if (!mcuInit) return;

	const char* sbmCmdStrConst = (env)->GetStringUTFChars( sbmCmd , NULL ) ;
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	char* sbmCmdStr = const_cast<char*>(sbmCmdStrConst);
	mcu.execute(sbmCmdStr);
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd)
{	
#ifdef USE_PYTHON
	if (!mcuInit) return;
	const char* pyCmdStrConst = (env)->GetStringUTFChars( pythonCmd , NULL ) ;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	char* pyCmdStr = const_cast<char*>(pyCmdStrConst);
	mcu.executePython(pyCmdStr);
#else // if there is no python, then run the sbm command. 
	//Java_com_android_sbmjni_SbmJNILib_executeSbm(env,obj,pythonCmd);
	LOG("Python now supported");
#endif
}

JNIEXPORT jstring JNICALL Java_com_android_sbmjni_SbmJNILib_getLog( JNIEnv * env, jobject obj )
{
	std::string logStr = engine.androidListener.getLogs();
	//std::string logStr = "Getting Log through JNI";
	return env->NewStringUTF(logStr.c_str());
}