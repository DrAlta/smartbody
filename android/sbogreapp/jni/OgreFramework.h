//|||||||||||||||||||||||||||||||||||||||||||||||

#ifndef OGRE_FRAMEWORK_H
#define OGRE_FRAMEWORK_H

//|||||||||||||||||||||||||||||||||||||||||||||||

#include "SBListener.h"
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include "android/AndroidInputManager.h"

#ifdef OGRE_STATIC_LIB
#  define OGRE_STATIC_GL
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define OGRE_STATIC_Direct3D9
// dx10 will only work on vista, so be careful about statically linking
#    if OGRE_USE_D3D10
#      define OGRE_STATIC_Direct3D10
#    endif
#  endif
#  define OGRE_STATIC_CgProgramManager
#  ifdef OGRE_USE_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#  endif
#  if OGRE_VERSION >= 0x10800
#    if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#      define OGRE_IS_IOS 1
#    endif
#  else
#    if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#      define OGRE_IS_IOS 1
#    endif
#  endif
#  ifdef OGRE_IS_IOS
#    undef OGRE_STATIC_CgProgramManager
#    undef OGRE_STATIC_GL
#    define OGRE_STATIC_GLES 1
#    ifdef OGRE_USE_GLES2
#      define OGRE_STATIC_GLES2 1
#      define USE_RTSHADER_SYSTEM
#      undef OGRE_STATIC_GLES
#    endif
#    ifdef __OBJC__
#      import <UIKit/UIKit.h>
#    endif
#  endif
#  include "OgreStaticPluginLoader.h"
#endif

#ifdef __ANDROID__
#   include <OISMultiTouch.h>
#endif

#ifndef __ANDROID__
#include <SdkTrays.h>
#endif

//|||||||||||||||||||||||||||||||||||||||||||||||

#ifdef __ANDROID__
class OgreFramework : public Ogre::Singleton<OgreFramework>, OIS::KeyListener, OIS::MultiTouchListener
#else
class OgreFramework : public Ogre::Singleton<OgreFramework>, OIS::KeyListener, OIS::MouseListener, OgreBites::SdkTrayListener
#endif
{
public:
	OgreFramework();
	~OgreFramework();
    
#ifdef __ANDROID__
    bool initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MultiTouchListener *pMouseListener = 0, int resX = 800, int resY = 1280);
#else
	bool initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MouseListener *pMouseListener = 0, int resX = 800, int resY = 1280);
#endif

	Ogre::SceneManager* getSceneManager() { return m_pSceneMgr; }
	void updateOgre(double timeSinceLastFrame);
	void moveCamera();
	void getInput();
    
	bool isOgreToBeShutDown()const{return m_bShutDownOgre;}
    
	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

#ifdef __ANDROID__
	bool touchMoved(const OIS::MultiTouchEvent &evt);
	bool touchPressed(const OIS::MultiTouchEvent &evt); 
	bool touchReleased(const OIS::MultiTouchEvent &evt);
	bool touchCancelled(const OIS::MultiTouchEvent &evt);
#else
	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id); 
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
#endif
	
	static bool                 m_ShowDeformModel;
	std::vector<std::string>    m_characterNameList;
	Ogre::Root*                 m_pRoot;
	Ogre::SceneManager*			m_pSceneMgr;
	Ogre::RenderWindow*			m_pRenderWnd;
	Ogre::Camera*				m_pCamera;
	Ogre::Viewport*				m_pViewport;
	Ogre::Log*                  m_pLog;
	Ogre::Timer*				m_pTimer;
	
	OIS::AndroidInputManager*			m_pInputMgr;
	OIS::Keyboard*			m_pKeyboard;
#ifdef __ANDROID__
	OIS::MultiTouch*			m_pMouse;
#else
	OIS::Mouse*					m_pMouse;
#endif

public:
   // Added for Mac compatibility
   Ogre::String                 m_ResourcePath;
     

// skeleton info
    std::string jointNames[200];
    float characterPosition[7];
    float boneData[800];        // quaternion data
    int numberJoints;
    
private:
	OgreFramework(const OgreFramework&);
	OgreFramework& operator= (const OgreFramework&);
#ifndef __ANDROID__    
	OgreBites::SdkTrayManager*  m_pTrayMgr;
#endif
       Ogre::FrameEvent            m_FrameEvent;
	int                         m_iNumScreenShots;
    
	bool                        m_bShutDownOgre;
	
	Ogre::Vector3				m_TranslateVector;
	Ogre::Real                  m_MoveSpeed; 
	Ogre::Degree				m_RotateSpeed; 
	float                       m_MoveScale; 
	Ogre::Degree				m_RotScale;
       Ogre::StaticPluginLoader    m_StaticPluginLoader;
};

//|||||||||||||||||||||||||||||||||||||||||||||||

#endif 

//|||||||||||||||||||||||||||||||||||||||||||||||
