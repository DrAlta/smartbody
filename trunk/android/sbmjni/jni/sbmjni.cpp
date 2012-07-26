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

#include <vhcl_log.h>
#include <vhcl_string.h>
#include <vhmsg-tt.h>
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>
#include <boost/filesystem/operations.hpp>
#include <sr/sr_camera.h>

#define ANDROID_PYTHON
#ifdef ANDROID_PYTHON
#include <sb/SBPython.h>
#endif

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

void sbm_vhmsg_callback( const char *op, const char *args, void * user_data ) {
	// Replace singleton with a user_data pointer
	if (!mcuInit) return;
	//LOG("VHMSG Callback : op = %s ,args = %s\n",op,args);
	switch( mcuCBHandle::singleton().execute( op, (char *)args ) ) {
		case CMD_NOT_FOUND:
			LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
			break;
		case CMD_FAILURE:
			LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
			break;
	}
}
/*
int sbm_main_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{

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

void mcu_register_callbacks( void ) {

	mcuCBHandle& mcu = mcuCBHandle::singleton();
/*
	mcu.insert( "q",			mcu_quit_func );
	mcu.insert( "quit",			mcu_quit_func );

	mcu.insert( "snapshot",		mcu_snapshot_func );
	mcu.insert( "viewer",		mcu_viewer_func );
	mcu.insert( "bmlviewer",    mcu_bmlviewer_func);
	mcu.insert( "panimviewer",  mcu_panimationviewer_func);
	mcu.insert( "cbufviewer",	mcu_channelbufferviewer_func); // deprecated
	mcu.insert( "dataviewer",	mcu_channelbufferviewer_func);
	mcu.insert( "resourceviewer",	mcu_resourceViewer_func);	
	mcu.insert( "faceviewer",	mcu_faceViewer_func);
*/
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

void initConnection()
{
	if (!mcuInit) return;	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	const char* serverName = "172.16.33.14";
	const char* scope = "DEFAULT_SCOPE";
	const char* port = "61616";
	int err;
	int openConnection = vhmsg::ttu_open(serverName,scope,port);
	if( openConnection == vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( sbm_vhmsg_callback );
		err = vhmsg::ttu_register( "sb" );
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
	glClearColor(1.03f,0.03f,0.03f,1);
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
	JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_restart(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_openConnection(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_closeConnection(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executeSbm(JNIEnv * env, jobject obj, jstring sbmCmd);
    JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd);
	JNIEXPORT jstring JNICALL Java_com_android_sbmjni_SbmJNILib_getLog(JNIEnv * env, jobject obj);
};




void initPython()
{
#ifdef ANDROID_PYTHON
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	//LOGI("Before init Python");
	initPython(python_lib_path);
#endif
}

void initSmartBody()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.executePython("scene.addAssetPath('seq', '/sdcard/sbmjniData/')");
	mcu.executePythonFile("default.py");
//	mcu.execute("path seq /sdcard/sbmjniData/");
//	mcu.execute("seq default.seq");	
//	mcu.executePython("print 'aaa'");	
	TimeRegulator& timer = engine.timer;
	timer.reset();
	timer.start();	
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	if (mcuInit)
		return;	
	XMLPlatformUtils::Initialize();  // Initialize Xerces before creating MCU	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu_register_callbacks();	
	vhcl::Log::g_log.AddListener(&engine.androidListener);
#ifdef ANDROID_PYTHON
	initPython();
#endif
	
	TimeRegulator& timer = engine.timer;

	mcu.register_timer( timer );
	TimeIntervalProfiler* profiler = new TimeIntervalProfiler();
	mcu.register_profiler(*profiler);
	initSmartBody();

	setupGraphics(width, height);    
	mcuInit = true;
	initConnection();
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_step(JNIEnv * env, jobject obj)
{
	if (!mcuInit) return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	bool update_sim = mcu.update_timer();	
	int err = vhmsg::ttu_poll();
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
	//mcu.vhmsg_send("sbm",sbmCmdStr);
	//mcu.vhmsg_send("sbm","testing sbm message");
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd)
{	
#ifdef ANDROID_PYTHON
	if (!mcuInit) return;
	const char* pyCmdStrConst = (env)->GetStringUTFChars( pythonCmd , NULL ) ;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//LOG("python cmd = %s",pyCmdStrConst);
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
	return env->NewStringUTF(logStr.c_str());
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_restart( JNIEnv * env, jobject obj )
{
	/*
	if (!mcuInit) return;
	mcuInit = false;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.reset();
	initSmartBody();	
	mcuInit = true;
	*/
}



JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_openConnection( JNIEnv * env, jobject obj )
{
	return initConnection();
}

JNIEXPORT void JNICALL Java_com_android_sbmjni_SbmJNILib_closeConnection( JNIEnv * env, jobject obj )
{	
	vhmsg::ttu_close();
}
