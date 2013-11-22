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

#include <jni.h>


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
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmTexture.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_gl.h>
#include <sr/sr_light.h>
#include <sr/sr_camera.h>
#include <sr/sr_gl_render_funcs.h>

#define ANDROID_PYTHON
#ifdef ANDROID_PYTHON
#include <sb/SBPython.h>
#endif

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <android/log.h>
#define  LOG_TAG    "libsbjniapp"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOG_FOOT   LOGI("%s %s %d", __FILE__, __FUNCTION__, __LINE__)

#if 1
struct Engine
{
	//SrCamera camera;
	TimeRegulator timer;
	vhcl::Log::AndroidListener androidListener;
};

Engine engine;
static SrLight light1;
static SrLight light2;
static int curH = -1, curW = -1;
#endif 

#if 1
static bool mcuInit = false;
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
#endif

#if 1
void drawSB()
{
#if DRAW_STICK_FIGURE
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
#endif
// update and render deformable mesh

	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		if(pawn->dMeshInstance_p)
		{			
			pawn->dMeshInstance_p->setVisibility(1);
			pawn->dMeshInstance_p->update();
			SrGlRenderFuncs::renderDeformableMesh(pawn->dMeshInstance_p);
		}
	}

	//static SrSaGlRender* render_action = NULL;
	//if (!render_action)
	//     render_action = new SrSaGlRender();	
	//render_action->apply(SmartBody::SBScene::getScene()->getRootGroup());
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

	
	//SrCamera& cam = engine.camera;	
	//camera.setAspectRatio(0.966897);
	curW = w;
	curH = h;
        glViewport(0, 0, w, h);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	// light
	light1.directional = true;
	light1.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light1.position = SrVec( 100.0, 250.0, 400.0 );
	//light1.constant_attenuation = 1.0f;

	light2 = light1;
	light2.directional = true;
	light2.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light2.position = SrVec( 100.0, 500.0, -1000.0 );

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();	
	SrCamera& camera = *scene->createCamera("defaultCamera");	
	camera.init();
/*
	cam.setCenter(0, 92, 0);
	cam.setUpVector(SrVec::j);
	cam.setEye( 0, 166, 185 );
	cam.setScale(1.f);
*/
	camera.setCenter(0.0, 1.60887, 0.0);
	camera.setUpVector(SrVec(0, 1, 0));
	camera.setEye(0, 1.0478, 3.69259);
	camera.setScale(1);
	camera.setFov(1.0);
	camera.setFarPlane(100);
	camera.setNearPlane(0.1);
	float aspectRatio = ((float)w)/h;
	camera.setAspectRatio(aspectRatio);	
        return true;
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };

void renderFrame() {
	//LOG("renderFrame");
	if (!mcuInit) 
	{
		LOG("mcu is not initialized now. skip rendering");
		return;
	}
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* defaultCam = scene->getCamera("defaultCamera");
	if (!defaultCam) return;

	SrCamera& cam = *defaultCam;

	// Just fill the screen with a color.
	glClearColor(0.6f,0.6f,0.6f,1);
	//glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
	//        ((float)engine->state.y)/engine->height, 1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	
	glLight(0, light1);
	glLight(1, light2);
	glEnable(GL_TEXTURE_2D);

	SrMat mat;
	glMatrixMode ( GL_PROJECTION );
	glLoadMatrixf ( (const float*)cam.get_perspective_mat(mat) );

	//----- Set Visualisation -------------------------------------------
	glMatrixMode ( GL_MODELVIEW );
	glLoadMatrixf ( (const float*)cam.get_view_mat(mat) );

        SbmTextureManager& texm = SbmTextureManager::singleton();
        texm.updateTexture();

	//glScalef ( cam.scale, cam.scale, cam.scale );
	//glDisable(GL_LIGHTING);
	// draw a ground plane
	float planeSize  = 300.f;
	SrVec quad[4] = { SrVec(planeSize, 0.f, planeSize), SrVec(-planeSize, 0.f, planeSize), SrVec(-planeSize,0.f,-planeSize), SrVec(planeSize, 0.f, -planeSize) };
	SrVec quadN[4] = { SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f) };
	GLfloat quadColor[16] = { 0.2f,0.2f, 0.2f, 1.f , 0.3f,0.3f,0.3f, 1.f, 0.5f,0.5f,0.5f,1.f, 0.25f,0.25f,0.25f,1.f };
	unsigned short indices[] = {0,1,2, 0,2,3};

	glShadeModel(GL_SMOOTH);
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLfloat*)&quad[0]);
	glColorPointer(4, GL_FLOAT, 0, quadColor);
	glNormalPointer(GL_FLOAT, 0, (GLfloat*)&quadN[0]);
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	//glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glEnable(GL_LIGHTING);
	drawSB();
	//eglSwapBuffers(engine->display, engine->surface);    
}


extern "C" {
    //JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_test(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_restart(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_openConnection(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_closeConnection(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_executeSB(JNIEnv * env, jobject obj, jstring sbmCmd);
    //JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_executePython(JNIEnv * env, jobject obj, jstring pythonCmd);
    JNIEXPORT jstring JNICALL Java_com_android_sbjniapp_SBJNIAppLib_getLog(JNIEnv * env, jobject obj);   
    JNIEXPORT jboolean JNICALL Java_com_android_sbjniapp_SBJNIAppLib_handleInputEvent(JNIEnv* env, jobject thiz, jint action, jfloat mx, jfloat my);
};

#endif

#if 0
void closeConnection(JNIEnv* env, jobject thiz)
{
	LOGI("Close connection");
	//endConnection();
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOG_FOOT;
    JNIEnv *env;

    LOGI("JNI_OnLoad called");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
    	LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }

    JNINativeMethod methods[] =
    {
		{
		            "closeConnection",
		            "()V",
		            (void *) closeConnection
		},		
    };
    jclass k;
    k = (env)->FindClass ("com/android/sbjniapp/SBJNIAppLib");
    (env)->RegisterNatives(k, methods, 1);

    return JNI_VERSION_1_4;
}
#endif

#if 1

void initCharacterScene()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	// for some reason couldn't get flash listener working properly, following code is supposed to replace character listener
	std::vector<std::string> charNames = scene->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(charNames[i]);
		if (!character)
			continue;

		// add skeletons and mesh
		
		if (character->scene_p)
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove(character->scene_p);
			character->scene_p->unref();
			character->scene_p = NULL;
		}
		/*
		character->scene_p = new SkScene();
		character->scene_p->ref();
		character->scene_p->init(character->getSkeleton());
		bool visible = character->getBoolAttribute("visible");
		if (visible)
			character->scene_p->visible(true);
		else
			character->scene_p->visible(false);
		scene->getRootGroup()->add(character->scene_p);
		*/
		LOG("Character %s's skeleton added to the scene.", charNames[i].c_str());		
		character->dMeshInstance_p =  new DeformableMeshInstance();
		//character->dMeshInstance_p->setSkeleton(character->getSkeleton());
		character->dMeshInstance_p->setPawn(character);
		LOG("Character %s's deformable mesh reset.", charNames[i].c_str());

		std::string dMeshAttrib = character->getStringAttribute("deformableMesh");
		character->setStringAttribute("deformableMesh", dMeshAttrib);
		LOG("Character %s's deformable mesh %s added to the scene.", charNames[i].c_str(), dMeshAttrib.c_str());
	}
}


void initSmartBody()
{
	mcuInit = true;
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	LOGI("Before init python");
	initPython(python_lib_path);
	LOGI("Before init SBScene");
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	scene->getSimulationManager()->setupTimer();
	scene->getSimulationManager()->setTime(0.0);
	scene->getSimulationManager()->start();

	scene->addAssetPath("script", "/sdcard/sbjniappdir");
	scene->runScript("brad.py");
	initCharacterScene();	
	//if (curW != -1 && curH != -1)
	//    setupGraphics(curW,curH);	
}

bool touchEvent(int action, float x, float y)
{
	static bool firstTime = true;
	static float prevx = 0.f, prevy = 0.f;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = scene->getCamera("defaultCamera");
	if (!cam) return false;
	float deltaX, deltaY;
	deltaX = x - prevx; 
	deltaY = y - prevy;
	if (firstTime)
	{
		deltaX = 0.f;
		deltaY = 0.f;
		firstTime = false;		
	}
	prevx = x; 
	prevy = y;

	float dx = deltaX * cam->getAspectRatio() * 0.01;
	float dy = deltaY * cam->getAspectRatio() * 0.01;

	enum { Touch_Pressed = 0, Touch_Released, Touch_Moved, Touch_Cancelled, Touch_None };
	int touchAction = Touch_None;
	switch(action)
	{
	case 0:
		//state.touchType = OIS::MT_Pressed;
		touchAction = Touch_Pressed;
		break;
	case 1:
		//state.touchType = OIS::MT_Released;
		touchAction = Touch_Released;
		break;
	case 2:
		//state.touchType = OIS::MT_Moved;
		touchAction = Touch_Moved;
		break;
	case 3:
		//state.touchType = OIS::MT_Cancelled;
		touchAction = Touch_Cancelled;
		break;
	default:
		//state.touchType = OIS::MT_None;
		touchAction = Touch_None;
		break;
	}
	
#if 0
	if (touchAction == Touch_Moved)	// zoom
	{
		LOG("touchAction = Touch_Moved");
		float tmpFov = cam->getFov() + (-dx + dy);
		cam->setFov(SR_BOUND(tmpFov, 0.001f, srpi));
	}
#endif
	static SrVec origUp, origCenter, origCamera;
	static float origX, origY;
	if (touchAction == Touch_Pressed)
	{
		//LOG("touchAction = Pressed");
		origUp = cam->getUpVector();
		origCenter = cam->getCenter();
		origCamera = cam->getEye();		
		origX = x;
		origY = y;
	}
	else if (touchAction == Touch_Released)
	{
		origX = -1;
		origY = -1;		
	}
	else if (touchAction == Touch_Moved) // rotate
	{
		//LOG("touchAction = Moved");
		float camDx = (x-origX) * cam->getAspectRatio() * 0.01;
		float camDy = (y-origY) * cam->getAspectRatio() * 0.01;
		SrVec forward = origCenter - origCamera; 		   
		SrQuat q = SrQuat(origUp, vhcl::DEG_TO_RAD()*camDx*20.f);			   
		forward = forward*q;
		SrVec tmp = cam->getEye() + forward;
		cam->setCenter(tmp.x, tmp.y, tmp.z);

		SrVec cameraRight = cross(forward,origUp);
		cameraRight.normalize();		   
		q = SrQuat(cameraRight, vhcl::DEG_TO_RAD()*camDy*20.f);	
		cam->setUpVector(origUp*q);
		forward = forward*q;
		SrVec tmpCenter = cam->getEye() + forward;
		cam->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
	}
	
	
}

JNIEXPORT jboolean JNICALL Java_com_android_sbjniapp_SBJNIAppLib_handleInputEvent(JNIEnv* env, jobject thiz, jint action, jfloat mx, jfloat my)
{
	return touchEvent(action,mx,my);
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	if (mcuInit)
		return;

	setupGraphics(width, height);    
	initSmartBody();
	vhcl::Log::g_log.AddListener(&engine.androidListener);
	
	

}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_step(JNIEnv * env, jobject obj)
{
	if (!mcuInit) return;

	int err = vhmsg::ttu_poll();
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->getSimulationManager()->updateTimer();
	scene->update();
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
#if 1
	
	if (!mcuInit) return;
	LOG("before Restart");
	mcuInit = false;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	LOG("before destroyScene");
	SmartBody::SBScene::destroyScene();
	LOG("after destroyScene");
	initSmartBody();	
	setupGraphics(curW,curH);
	mcuInit = true;
	LOG("after Restart");
#endif	
}



JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_openConnection( JNIEnv * env, jobject obj )
{
	return initConnection();
}

JNIEXPORT void JNICALL Java_com_android_sbjniapp_SBJNIAppLib_closeConnection( JNIEnv * env, jobject obj )
{	
	vhmsg::ttu_close();
}

#endif
