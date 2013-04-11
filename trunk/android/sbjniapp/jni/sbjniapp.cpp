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
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBCommandManager.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <boost/filesystem/operations.hpp>
#include <sbm/time_regulator.h>
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

#define  LOG_TAG    "libsbjniapp"
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

void sb_vhmsg_callback( const char *op, const char *args, void * user_data ) {
	// Replace singleton with a user_data pointer
	if (!mcuInit) return;
	//LOG("VHMSG Callback : op = %s ,args = %s\n",op,args);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	switch( scene->getCommandManager()->execute( op, (char *)args ) ) {
		case CMD_NOT_FOUND:
			LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
			break;
		case CMD_FAILURE:
			LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
			break;
	}
}

void drawSB()
{
	static SrVec jointPos[200];
	static unsigned short boneIdx[400];

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> characterNames = scene->getCharacterNames();

	for (std::vector<std::string>::iterator iter = characterNames.begin();
		iter != characterNames.end();
		iter++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(*iter);

		SmartBody::SBSkeleton* skeleton = character->getSkeleton();
		skeleton->update_global_matrices();
		std::map<int,int> indexMap;
		for (int i=0;i<skeleton->joints().size();i++)
		{
			SkJoint* joint = skeleton->joints()[i];
			SrVec pos = joint->gmat().get_translation();
			jointPos[i] = pos;
			indexMap[joint->index()] = i;
			boneIdx[i*2+0] = joint->index();
			if (joint->parent())
				boneIdx[i*2+1] = joint->parent()->index();
			else
				boneIdx[i*2+1] = joint->index();
		}

		for (int i=0;i<skeleton->joints().size();i++)
		{
			boneIdx[i*2] = indexMap[boneIdx[i*2]];
			boneIdx[i*2+1] = indexMap[boneIdx[i*2+1]];
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (const GLfloat*)&jointPos[0]);
		//glDrawArrays(GL_POINTS, 0, sk->joints().size());
		glDrawElements(GL_LINES,skeleton->joints().size()*2,GL_UNSIGNED_SHORT, boneIdx);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void initConnection()
{
	if (!mcuInit) return;	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const char* serverName = "172.16.33.14";
	const char* scope = "DEFAULT_SCOPE";
	const char* port = "61616";
	int err;
	int openConnection = vhmsg::ttu_open(serverName,scope,port);
	if( openConnection == vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( sb_vhmsg_callback );
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
		scene->getVHMsgManager()->setEnable(true);
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
	cam.setCenter(0, 92, 0);
	cam.setUpVector(SrVec::j);
	cam.setEye( 0, 166, 185 );
	cam.setScale(1.f);

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
	drawSB();
	//eglSwapBuffers(engine->display, engine->surface);    
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_restart(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_openConnection(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_closeConnection(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_executeSB(JNIEnv * env, jobject obj, jstring sbmCmd);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd);
	JNIEXPORT jstring JNICALL Java_com_android_sbjniapp_SBJNIAppLib_getLog(JNIEnv * env, jobject obj);
};

void initSmartBody()
{
	mcuInit = true;
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	initPython(python_lib_path);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	scene->getSimulationManager()->setupTimer();
	scene->getSimulationManager()->setTime(0.0);
	scene->getSimulationManager()->start();

	scene->addAssetPath("script", "/sdcard/sbjniappdir");
	scene->runScript("default.py");
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	if (mcuInit)
		return;

	initSmartBody();
	vhcl::Log::g_log.AddListener(&engine.androidListener);
	
	setupGraphics(width, height);    

}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_step(JNIEnv * env, jobject obj)
{
	if (!mcuInit) return;

	int err = vhmsg::ttu_poll();
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->getSimulationManager()->update();
    renderFrame();
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_executeSB(JNIEnv * env, jobject obj, jstring sbmCmd)
{
	if (!mcuInit) return;
	const char* pyCmdStrConst = (env)->GetStringUTFChars( sbmCmd , NULL ) ;
	//LOG("python cmd = %s",pyCmdStrConst);
	char* pyCmdStr = const_cast<char*>(pyCmdStrConst);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->run(pyCmdStr);
}

JNIEXPORT jstring JNICALL Java_com_android_sbjniapp_SBJNIAppLib_getLog( JNIEnv * env, jobject obj )
{
	std::string logStr = engine.androidListener.getLogs();	
	return env->NewStringUTF(logStr.c_str());
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_restart( JNIEnv * env, jobject obj )
{
	/*
	if (!mcuInit) return;
	mcuInit = false;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBScene::destroyScene();
	initSmartBody();	
	mcuInit = true;
	*/
}



JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_openConnection( JNIEnv * env, jobject obj )
{
	return initConnection();
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_closeConnection( JNIEnv * env, jobject obj )
{	
	vhmsg::ttu_close();
}
