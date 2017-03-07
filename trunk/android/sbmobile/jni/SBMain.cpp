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

#include <jni.h>
#include <vhcl.h>
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBCommandManager.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBAssetManager.h>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sbm/GPU/SbmTexture.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sys/time.h>
#include "SBWrapper.h"

#include "esUtil.h"

ESContext esContext;

#if 1
vhcl::Log::AndroidListener androidListener;
unsigned long prevTime = 0;
int curW, curH;
bool sbInit = false;
static unsigned long getCurrentTime(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec*1000000 + now.tv_nsec/1000;
}

#endif 

extern "C" {
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_init(JNIEnv * env, jobject obj);	
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_setup(JNIEnv * env, jobject obj, jstring mediaPath);
	
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_surfaceChanged(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_render(JNIEnv * env, jobject obj);	
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_renderCardboard(JNIEnv * env, jobject obj, jfloatArray arr);	
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_reloadTexture(JNIEnv * env, jobject obj);	

	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_openConnection(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_closeConnection(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_executeSB(JNIEnv * env, jobject obj, jstring sbmCmd);
	JNIEXPORT jstring JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getLog(JNIEnv * env, jobject obj);
    JNIEXPORT jstring JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getStringAttribute(JNIEnv * env, jobject obj, jstring attrName);
	JNIEXPORT jboolean JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getBoolAttribute(JNIEnv * env, jobject obj, jstring attrName);
	JNIEXPORT jdouble JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getDoubleAttribute(JNIEnv * env, jobject obj, jstring attrName);   
    };

JNIEXPORT jstring JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getStringAttribute(JNIEnv * env, jobject obj, jstring attrName)
{
	const char* attrNameStr = (env)->GetStringUTFChars( attrName , NULL ) ;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    std::string strVal = scene->getStringAttribute(attrNameStr);
    return env->NewStringUTF(strVal.c_str());
}

JNIEXPORT jboolean JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getBoolAttribute(JNIEnv * env, jobject obj, jstring attrName)
{
	const char* attrNameStr = (env)->GetStringUTFChars( attrName , NULL ) ;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    bool val = scene->getBoolAttribute(attrNameStr);
    return (jboolean) val;
}

JNIEXPORT jdouble JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getDoubleAttribute(JNIEnv * env, jobject obj, jstring attrName)
{
	const char* attrNameStr = (env)->GetStringUTFChars( attrName , NULL ) ;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    double val = scene->getDoubleAttribute(attrNameStr);
    return (jdouble) val;
}

JNIEXPORT jdouble JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getIntAttribute(JNIEnv * env, jobject obj, jstring attrName)
{
	const char* attrNameStr = (env)->GetStringUTFChars( attrName , NULL ) ;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    int val = scene->getIntAttribute(attrNameStr);
    return (jint) val;
}




JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_reloadTexture(JNIEnv * env, jobject obj)
{
	//LOG("Reload OpenGL Texture");
	SBInitGraphics(&esContext);
	SbmTextureManager& texm = SbmTextureManager::singleton();
	texm.reloadTexture();

}


JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_surfaceChanged(JNIEnv * env, jobject obj,  jint width, jint height)
{	
	SBSetupDrawing(width,height);
	curW = width;
	curH = height;
}

JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_setup(JNIEnv * env, jobject obj, jstring mediaPath )
{
	if (sbInit) // don't need to setup twice
		return;
	const char* mediaPathStr = (env)->GetStringUTFChars( mediaPath , NULL ) ;
	//SBSetup("/sdcard/vhdata/","setup.py");
	SBSetup(mediaPathStr,"setup.py");
}



JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_init(JNIEnv * env, jobject obj)
{	
	esInitContext(&esContext);	
	if (sbInit) // don't need to setup twice
		return;

	SBInitialize(); // initialize smartbody with media path
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();	
	scene->addAssetPath("script", "scripts");
	SBInitScene("init.py");		
	vhcl::Log::g_log.AddListener(&androidListener);	
	sbInit = true;
	LOG("Initializing Done");
}


JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_render(JNIEnv * env, jobject obj)
{	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if(!scene)
		return;
	if (!sbInit)
		return;
	SrMat id;
	//SBDrawFrame(VHEngine::curW, VHEngine::curH, id);
	SBDrawFrame_ES20(curW, curH, &esContext, id);
}

JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_renderCardboard(JNIEnv * env, jobject obj, jfloatArray arr)
{	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if(!scene)
		return;
	if (!sbInit)
		return;
	SrMat eyeViewMat;
	jfloat *body = env->GetFloatArrayElements(arr, 0);
	for (int i=0;i<16;i++)
		eyeViewMat[i] = body[i];
	//eyeViewMat.transpose();
	//SBDrawFrame(VHEngine::curW, VHEngine::curH, eyeViewMat);
	SBDrawFrame_ES20(curW, curH, &esContext, eyeViewMat);
}

JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_step(JNIEnv * env, jobject obj)
{	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if(!scene)
		return;
	if (!sbInit)
		return;
	unsigned long curTime = getCurrentTime();	
	
	#if 1
	if (!scene->getBoolAttribute("ready"))
	{
		prevTime = curTime;
	    return;
	}
	
	static long frameDeltaTime = 1000000/30; // 30 frames per second
	static int firstCount = 0;	
	//if (curTime - prevTime > frameDeltaTime)
	//if (firstCount < 10)
	{
		float dt = (float)(curTime-prevTime)/1000000.f;			
		SBUpdate(dt);		
		firstCount++;		
	}
	#endif
}


JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_executeSB(JNIEnv * env, jobject obj, jstring sbmCmd)
{
	if (!sbInit)
		return;
	const char* pyCmdStrConst = (env)->GetStringUTFChars( sbmCmd , NULL ) ;
	LOG("python cmd = %s",pyCmdStrConst);
	SBExecutePythonCmd(pyCmdStrConst);
}

JNIEXPORT jstring JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_getLog( JNIEnv * env, jobject obj )
{	
	if (!sbInit)
		return;
	std::string logStr = "No Text";
	logStr = androidListener.getLogs();	
	return env->NewStringUTF(logStr.c_str());
}



JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_openConnection( JNIEnv * env, jobject obj )
{
	return SBInitVHMSGConnection();
}

JNIEXPORT void JNICALL Java_edu_usc_ict_sbmobile_SBMobileLib_closeConnection( JNIEnv * env, jobject obj )
{	
	SBCloseVHMSGConnection();
}
