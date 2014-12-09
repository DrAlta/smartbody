#ifndef __OgreSmartBody_h_
#define __OgreSmartBody_h_
 
#include <OGRE/Ogre.h>
#include <OIS/OIS.h>
// smartbody
#ifndef NDEBUG
#define NDEBUG
#endif

#include <sb/SBScene.h>

class OgreSmartBody : public Ogre::WindowEventListener, public Ogre::FrameListener
{
public:
    OgreSmartBody(void);
    virtual ~OgreSmartBody(void);
    bool go(void);

	Ogre::SceneManager* getSceneManager();
 
protected:
    // Ogre::WindowEventListener
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);
 
    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	
 
private:
    Ogre::Root* mRoot;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    Ogre::RenderWindow* mWindow;
    Ogre::SceneManager* mSceneMgr;
    Ogre::Camera* mCamera;
 
    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;
	double mStartTime;

	// smartbody
	SmartBody::SBScene* m_pScene;
};
 
#endif // #ifndef __OgreSmartBody_h_

