/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2010 zcube(JiSeop Moon).

    Contributor(s): harkon.kr.
-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include <jni.h>
#include <vhcl.h>
#include <stdlib.h>
#include "AndroidLogListener.h"
#include "test.h"
#include <android/log.h>
#include <boost/lexical_cast.hpp>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include "Ogre.h"
#include "OgreGLES2Plugin.h"

#include "OgreDemoApp.h"

#define LOG_TAG    "OgreKit"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_FOOT   LOGI("%s %s %d", __FILE__, __FUNCTION__, __LINE__)

Ogre::Log* ogreLog = NULL;
Ogre::Root* root = NULL;
Ogre::Plugin* renderSystem = NULL;
Ogre::SceneManager* sceneManager = NULL;
Ogre::RenderWindow* renderWindow = NULL;
Ogre::Viewport* viewport = NULL;
Ogre::Camera* camera = NULL;

bool mcuInit = false;
AndroidLogListener gLogListener;
DemoApp ogreApp;

vhcl::Log::AndroidListener sbLogListener;


jboolean init(JNIEnv* env, jobject thiz, jstring arg, jint width, jint height)
{
	LOG_FOOT;
	new Ogre::LogManager();
	ogreLog = Ogre::LogManager::getSingleton().createLog("OgreLogFile.log",true,true,false);
	Ogre::LogManager::getSingleton().getDefaultLog()->addListener(&gLogListener);
	ogreApp.startDemo(width, height);
	vhcl::Log::g_log.AddListener(&sbLogListener);
	mcuInit = true;

/*
	LOGI("Init sbOgreApp");
	root = new Ogre::Root("","");
	renderSystem = new Ogre::GLES2Plugin();
	root->installPlugin(renderSystem);
	const Ogre::RenderSystemList& renderers = root->getAvailableRenderers();
	root->setRenderSystem(renderers[0]);
	root->initialise(false);
	LOGI("Finish initialize OgreRoot");
	renderWindow = Ogre::Root::getSingleton().createRenderWindow("sbOgreApp",1024,768,false);
	LOGI("Finish create OgreWindow");
	sceneManager = root->createSceneManager(Ogre::ST_GENERIC,"SceneManager");
	if (sceneManager == NULL)
		LOGI("Fail create scene manager");
	LOGI("Finish create SceneManager");
	sceneManager->setAmbientLight(Ogre::ColourValue(0.7f,0.7f,0.7f));
	LOGI("Finish setAmbientLight Camera");
	camera = sceneManager->createCamera("Camera");
	LOGI("Finish create Camera");
	camera->setPosition(Ogre::Vector3(0,60,60));
	camera->lookAt(Ogre::Vector3(0,0,0));
	camera->setNearClipDistance(1);

	LOGI("Finish create OgreCamera");

	
	viewport = renderWindow->addViewport(camera);
	viewport->setBackgroundColour(Ogre::ColourValue(0.8f,0.7f,0.6f,1.0f));
	viewport->setCamera(camera);	
*/	
	return JNI_TRUE;
}

jboolean executeSB(JNIEnv* env, jobject thiz, jstring arg)
{
	if (!mcuInit) return false;
	const char* sbCmd = (env)->GetStringUTFChars(arg,NULL);
	SBExecuteCmd(sbCmd);
	return true;
}

jboolean openConnection(JNIEnv* env, jobject thiz)//, jstring serverName, jstring portName)
{
	//if (!mcuInit) return false;
	LOGI("open vhmsg connection");
	const char* server = "172.16.33.21";//(env)->GetStringUTFChars(serverName,NULL);
	const char* port = "61616";//(env)->GetStringUTFChars(portName,NULL);
	initConnection(server,port);
	return true;
}

void closeConnection(JNIEnv* env, jobject thiz)
{
	endConnection();
}

void setDeformable(JNIEnv* env, jobject thiz, jint isDeformable)
{
	if (isDeformable == 1)
		OgreFramework::m_ShowDeformModel = true;
	else
		OgreFramework::m_ShowDeformModel = false;
}


jboolean render(JNIEnv* env, jobject thiz, jint drawWidth, jint drawHeight, jboolean forceRedraw)
{
	//LOG_FOOT;
	//LOGI("Update sbOgreApp");
	//root->renderOneFrame();

	ogreApp.renderDemo(drawWidth,drawHeight);
	

	return JNI_TRUE;	
}

void cleanup(JNIEnv* env)
{
	LOG_FOOT;
}

jstring getLogStr(JNIEnv* env, jobject thiz)
{
	std::string logStr;
	std::list<std::string>::iterator vi;
	int msgCounter = 0;
	std::string headStr = "#";
	for ( vi = sbLogListener.logList.begin();
		  vi != sbLogListener.logList.end();
		  vi++)
	{
		std::string logLine = headStr + boost::lexical_cast<std::string>(msgCounter++) + " " + (*vi) + "\n\n";
		logStr += logLine;
	}
	return env->NewStringUTF(logStr.c_str());
}

jboolean inputEvent(JNIEnv* env, jobject thiz, jint action, jfloat mx, jfloat my)
{
	LOG_FOOT;
	OIS::AndroidInputManager* inputManager = ogreApp.getInputManager();
	if (inputManager)
	    inputManager->injectTouch(action,mx,my);

	return JNI_TRUE;
}

jboolean keyEvent(JNIEnv* env, jobject thiz, jint action, jint unicodeChar, jint keyCode, jobject keyEvent)
{
	LOG_FOOT;

	return JNI_TRUE;  
}

void setOffsets(JNIEnv* env, jobject thiz, jint x, jint y)
{
	LOGI("%s %d %d", __FUNCTION__, x, y);
	OIS::AndroidInputManager* inputManager = ogreApp.getInputManager();
	if (inputManager)
	    inputManager->setOffsets(x,y);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
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
            "init",
            "(Ljava/lang/String;II)Z",
            (void *) init
        },
        {
        	"render",
			"(IIZ)Z",
			(void *) render
        },
        {
			"inputEvent",
			"(IFFLandroid/view/MotionEvent;)Z",
			(void *) inputEvent

        },
        {
            "keyEvent",
            "(IIILandroid/view/KeyEvent;)Z",
            (void *) keyEvent
        },
        {
            "cleanup",
            "()V",
            (void *) cleanup
        },
		{
			"setOffsets",
			"(II)V",
			(void *) setOffsets
		},
		{
		            "executeSB",
		            "(Ljava/lang/String;)Z",
		            (void *) executeSB
		},
		{
		            "openConnection",
		            "()V",
		            (void *) openConnection
		},
		{
		            "closeConnection",
		            "()V",
		            (void *) closeConnection
		},
		{
				            "setDeformable",
				            "(I)V",
				            (void *) setDeformable
		},
		{
		            "getLogStr",
		            "()Ljava/lang/String;",
		            (void *) getLogStr
		},
    };
    jclass k;
    k = (env)->FindClass ("com/android/sbogreapp/Main");
    (env)->RegisterNatives(k, methods, 11);

    return JNI_VERSION_1_4;
}
