/*
 * Copyright (C) 2010 The Android Open Source Project
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
 *
 */

//BEGIN_INCLUDE(all)
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include <boost/filesystem/operations.hpp>
#include <sr/sr_camera.h>

#include <jni.h>
#include <errno.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sbm-log", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "sbm-warn", __VA_ARGS__))

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    SrCamera           camera;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    float deltaX, deltaY;
    struct saved_state state;
};

unsigned char getASCII(int32_t aKey )
{
	unsigned char result = ' ';
	switch (aKey)
	{
	case AKEYCODE_0 :
			result = '0';
			break;
	case AKEYCODE_1 :
			result = '1';
			break;
	case AKEYCODE_2 :
			result = '2';
			break;
	case AKEYCODE_3 :
			result = '3';
			break;
	case AKEYCODE_4 :
			result = '4';
			break;
	case AKEYCODE_5 :
			result = '5';
			break;
	case AKEYCODE_6 :
			result = '6';
			break;
	case AKEYCODE_7 :
			result = '7';
			break;
	case AKEYCODE_8 :
			result = '8';
			break;
	case AKEYCODE_9 :
			result = '9';
			break;
	case AKEYCODE_STAR :
			result = '*';
			break;
	case AKEYCODE_POUND :
			result = '#';
			break;
	case AKEYCODE_A :
			result = 'a';
			break;
	case AKEYCODE_B :
			result = 'b';
			break;
	case AKEYCODE_C :
		result = 'c';
		break;
	case AKEYCODE_D :
			result = 'd';
			break;
	case AKEYCODE_E :
			result = 'e';
			break;
	case AKEYCODE_F :
			result = 'f';
			break;
	case AKEYCODE_G :
			result = 'g';
			break;
	case AKEYCODE_H :
			result = 'h';
			break;
	case AKEYCODE_I :
			result = 'i';
			break;
	case AKEYCODE_J :
			result = 'j';
			break;
	case AKEYCODE_L :
			result = 'l';
			break;
	case AKEYCODE_M :
			result = 'm';
			break;
	case AKEYCODE_N :
			result = 'n';
			break;
	case AKEYCODE_O :
			result = 'o';
			break;
	case AKEYCODE_P :
			result = 'p';
			break;
	case AKEYCODE_Q :
			result = 'q';
			break;
	case AKEYCODE_R :
			result = 'r';
			break;
	case AKEYCODE_S :
			result = 's';
			break;
	case AKEYCODE_T :
			result = 't';
			break;
	case AKEYCODE_U :
			result = 'u';
			break;
	case AKEYCODE_V :
			result = 'v';
			break;
	case AKEYCODE_W :
			result = 'w';
			break;
	case AKEYCODE_X :
			result = 'x';
			break;
	case AKEYCODE_Y :
			result = 'y';
			break;
	case AKEYCODE_Z :
			result = 'z';
			break;
	case AKEYCODE_COMMA :
			result = ',';
			break;
	case AKEYCODE_PERIOD :
			result = '.';
			break;
	case AKEYCODE_TAB :
			result = 'c';
			break;
	case AKEYCODE_SPACE :
			result = ' ';
			break;
	case AKEYCODE_ENTER :
			result = 'c';
			break;
	case AKEYCODE_GRAVE :
			result = '`';
			break;
	case AKEYCODE_MINUS :
			result = '-';
			break;
	case AKEYCODE_PLUS :
			result = '-';
			break;
	case AKEYCODE_EQUALS :
			result = '=';
			break;
	case AKEYCODE_BACKSLASH :
			result = '/';
			break;
	case AKEYCODE_SLASH :
			result = '\\';
			break;
	case AKEYCODE_SEMICOLON :
			result = ';';
			break;
	case AKEYCODE_APOSTROPHE :
			result = '\'';
			break;
	default :
			result = ' ';
			break;
	}
	return result;
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


/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    engine->camera.init();
    engine->camera.center = SrVec(0, 92, 0);
    engine->camera.up = SrVec::j;
    engine->camera.eye.set ( 0, 166, 185 );
    engine->camera.scale = 1.f;
    // Initialize GL state.
    LOGI("viewport size w = %d, h = %d\n",w,h);
    glViewport(0,0,w,h);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    SrCamera& cam = engine->camera;

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
    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->deltaX = (AMotionEvent_getX(event,0) - engine->state.x)/engine->width;
        engine->deltaY = (AMotionEvent_getY(event,0) - engine->state.y)/engine->height;

        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);

        // rotate the camera based on touch motion
        SrCamera& cam = engine->camera;
        SrVec origUp = cam.up;
        SrVec origCenter = cam.center;
        SrVec origCamera = cam.eye;

        SrVec dirX = origUp;
        SrVec  dirY;
        dirY.cross(origUp, (origCenter - origCamera));
        dirY /= dirY.len();
        SrVec camera = rotatePoint(origCamera, origCenter, dirX, -engine->deltaX * float(M_PI));
        camera = rotatePoint(camera, origCenter, dirY, engine->deltaY * float(M_PI));
        //cam.eye = camera;

        return 1;
    }
    else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
    		 AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
    {
    	unsigned char key = getASCII(AKeyEvent_getKeyCode(event));
    	//LOGI("Keycode = %d, Key Pressed = %c\n",AKeyEvent_getKeyCode(event), key);
    	SrCamera& cam = engine->camera;
    	if (AKeyEvent_getKeyCode(event) == AKEYCODE_W)
    	{
    		cam.eye += SrVec(0,0,-2);
    		cam.center += SrVec(0,0,0.-2);
    	}
    	else if (AKeyEvent_getKeyCode(event) == AKEYCODE_S)
    	{
    		cam.eye += SrVec(0,0,2);
    		cam.center += SrVec(0,0,2);
    	}
    	else if (AKeyEvent_getKeyCode(event) == AKEYCODE_A)
    	{
    		cam.eye += SrVec(-2,0,0);
    		cam.center += SrVec(-2,0,0);
    	}
    	else if (AKeyEvent_getKeyCode(event) == AKEYCODE_D)
    	{
    		cam.eye += SrVec(2,0,0);
    		cam.center += SrVec(2,0,0);
    	}
    	else if (AKeyEvent_getKeyCode(event) == AKEYCODE_X)
    	{
    		cam.eye += SrVec(0,2,0);
    	    cam.center += SrVec(0,2,0);
    	}
       	else if (AKeyEvent_getKeyCode(event) == AKEYCODE_Z)
       	{
       		cam.eye += SrVec(0,-2,0);
       		cam.center += SrVec(0,-2,0);
       	}

    	return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }
    /*
    boost::filesystem2::path curPath = boost::filesystem2::current_path();
    LOGI("current path = %s\n",curPath.directory_string().c_str());
    boost::filesystem2::directory_iterator end;
    for (boost::filesystem2::directory_iterator iter(curPath); iter != end; iter++)
    {
    	std::string fileName = (*iter).string();
    	LOGI("next file = %s\n",fileName.c_str());
    }
    */
    LOGI("Starting Sbm Android");
    LOGI("Initialize XMLPlatformUtils\n");
    XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU
    LOGI("Start Initialize mcu\n");
    mcuCBHandle& mcu = mcuCBHandle::singleton();

    mcu_register_callbacks();

    TimeRegulator timer;
    mcu.register_timer( timer );
    TimeIntervalProfiler* profiler = new TimeIntervalProfiler();
    mcu.register_profiler(*profiler);

    mcu.execute("path seq /sdcard/sbmmedia/");
    mcu.execute("seq default.seq");
    //mcu.execute("load skeletons -R /sdcard/sbmmedia/");
    //mcu.execute("load motions -R /sdcard/sbmmedia/");
    //mcu.execute("char doctor init common.sk");
    //mcu.execute("bml char doctor <body posture=\"test\"/>");

    //mcu.execute("load skeletons -R ../../../../data/sbm-common/common-sk");

    //mcu.execute("bml char * <saccade mode=\"listen\"/>");

    timer.start();
    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        //LOGI("Looping for Sbm-Android");

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
        // update SmartBody
        mcu.update_profiler();
        bool update_sim = mcu.update_timer();
        mcu.mark( "main", 0, "fltk-check" );
        if( update_sim ) {
        	//LOGI("Update mcu...");
        	mcu.update();
        }


        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
