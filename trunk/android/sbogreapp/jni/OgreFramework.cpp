#include "OgreFramework.h"

#include "test.h"
#include <android/log.h>

using namespace Ogre; 

#ifndef ANDROID_LOG
#define ANDROID_LOG
#define LOG_TAG    "OgreKit"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_FOOT   LOGI("%s %s %d", __FILE__, __FUNCTION__, __LINE__)
#endif


namespace Ogre
{
    template<> OgreFramework* Ogre::Singleton<OgreFramework>::ms_Singleton = 0;
};

bool OgreFramework::m_ShowDeformModel = false;

OgreFramework::OgreFramework()
{
	m_MoveSpeed			= 0.1f;
	m_RotateSpeed       = 0.3f;
    
	m_bShutDownOgre     = false;
	m_iNumScreenShots   = 0;
    
	m_pRoot				= 0;
	m_pSceneMgr			= 0;
	m_pRenderWnd        = 0;
	m_pCamera			= 0;
	m_pViewport			= 0;
	m_pLog				= 0;
	m_pTimer			= 0;
    
	m_pInputMgr			= 0;
	m_pKeyboard			= 0;
	m_pMouse			= 0;
    
#ifdef __ANDROID__
    m_ResourcePath = "/sdcard/sbogreappdir/ogre/";
#else
    m_ResourcePath = "";
    m_pTrayMgr          = 0;
#endif    
    m_FrameEvent        = Ogre::FrameEvent();
}

//|||||||||||||||||||||||||||||||||||||||||||||||
#if defined(__ANDROID__)
bool OgreFramework::initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener, OIS::MultiTouchListener *pMouseListener, int resX, int resY)
#else
bool OgreFramework::initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener, OIS::MouseListener *pMouseListener)
#endif
{
	//if (!Ogre::LogManager::getSingletonPtr())
	//    new Ogre::LogManager();

	m_pLog = Ogre::LogManager::getSingleton().getDefaultLog();//Ogre::LogManager::getSingleton().createLog("OgreLogfile.log", true, true, false);
	LOGI("m_pLog = %d",m_pLog);
	//m_pLog->setDebugOutputEnabled(true);
    
    	String pluginsPath;
    // only use plugins.cfg if not static
    	String cfgFile = m_ResourcePath  + "ogre.cfg";
	String logFile = m_ResourcePath  + "ogre.log";
	LOGI("Before create Root");
	LOGI("pluginPath = %s, cfg = %s, log = %s",pluginsPath.c_str(), cfgFile.c_str(), logFile.c_str());
	m_pRoot = new Ogre::Root(pluginsPath, cfgFile, logFile);
	LOGI("After create Root");
	m_StaticPluginLoader.load();
    	LOGI("Before Init Root");
	if(!m_pRoot->showConfigDialog())
		return false;
	m_pRoot->initialise(false);
	LOGI("After Init Root");

	LOGI("before createRenderWindow");
	m_pRenderWnd = Ogre::Root::getSingleton().createRenderWindow("SmartBody Ogre App",resX,resY,true);
    	LOGI("before createSceneManager");
	m_pSceneMgr = m_pRoot->createSceneManager(ST_GENERIC, "SceneManager");
	m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f, 0.7f, 0.7f));

	
	
	m_pCamera = m_pSceneMgr->createCamera("Camera");
	m_pCamera->setPosition(Vector3(0, 300, 300));
	m_pCamera->lookAt(Vector3(0, 166, 0));
	m_pCamera->setFarClipDistance(1000);
	m_pCamera->setNearClipDistance(5);
    	LOGI("before addViewPort");
	m_pViewport = m_pRenderWnd->addViewport(m_pCamera);
	m_pViewport->setBackgroundColour(ColourValue(0.8f, 0.7f, 0.6f, 1.0f));
    
	m_pCamera->setAspectRatio(Real(m_pViewport->getActualWidth()) / Real(m_pViewport->getActualHeight()));
	
	m_pViewport->setCamera(m_pCamera);
       m_pViewport->setOrientationMode(Ogre::OR_PORTRAIT);
	unsigned long hWnd = 0;
       OIS::ParamList paramList;
       m_pRenderWnd->getCustomAttribute("WINDOW", &hWnd);
    
	paramList.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));
    
	//m_pInputMgr = OIS::InputManager::createInputSystem(paramList);
	m_pInputMgr = new OIS::AndroidInputManager();
    	LOGI("after create AndroidInputManager");
#if !defined(__ANDROID__)
       m_pKeyboard = static_cast<OIS::Keyboard*>(m_pInputMgr->createInputObject(OIS::OISKeyboard, true));
	m_pMouse = static_cast<OIS::Mouse*>(m_pInputMgr->createInputObject(OIS::OISMouse, true));
    
	m_pMouse->getMouseState().height = m_pRenderWnd->getHeight();
	m_pMouse->getMouseState().width	 = m_pRenderWnd->getWidth();
#else
	m_pMouse = static_cast<OIS::MultiTouch*>(m_pInputMgr->createInputObject(OIS::OISMultiTouch, true));
#endif
    
#if !defined(__ANDROID__)
	if(pKeyListener == 0)
		m_pKeyboard->setEventCallback(this);
	else
		m_pKeyboard->setEventCallback(pKeyListener);
#endif
    
	if(pMouseListener == 0)
		m_pMouse->setEventCallback(this);
	else
		m_pMouse->setEventCallback(pMouseListener);
    
	Ogre::String secName, typeName, archName;
	Ogre::ConfigFile cf;
    cf.load(m_ResourcePath + "resources.cfg");
    
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || defined(OGRE_IS_IOS)
            // OS X does not set the working directory relative to the app,
            // In order to make things portable on OS X we need to provide
            // the loading with it's own bundle path location
            if (!Ogre::StringUtil::startsWith(archName, "/", false)) // only adjust relative dirs
                archName = Ogre::String(m_ResourcePath + archName);
#endif
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
//	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    
	m_pTimer = OGRE_NEW Ogre::Timer();
	m_pTimer->reset();
#ifndef __ANDROID__	
	m_pTrayMgr = new OgreBites::SdkTrayManager("TrayMgr", m_pRenderWnd, m_pMouse, this);
    m_pTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
 //   m_pTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    m_pTrayMgr->hideCursor();
//    m_pTrayMgr->createTextBox(OgreBites::TL_BOTTOM, "Command", "Command", 120, 100);
#endif
    
    
	m_pRenderWnd->setActive(true);

        // init SmartBody Listener to update the character   
	return true;
}

OgreFramework::~OgreFramework()
{
    if(m_pInputMgr) OIS::InputManager::destroyInputSystem(m_pInputMgr);
#ifndef __ANDROID__
    if(m_pTrayMgr)  delete m_pTrayMgr;
#endif
    m_StaticPluginLoader.unload();
    if(m_pRoot)     delete m_pRoot;
}

bool OgreFramework::keyPressed(const OIS::KeyEvent &keyEventRef)
{
#if !defined(__ANDROID__)
	
	if(m_pKeyboard->isKeyDown(OIS::KC_ESCAPE))
	{
        m_bShutDownOgre = true;
        return true;
	}
    
	if(m_pKeyboard->isKeyDown(OIS::KC_SYSRQ))
	{
		m_pRenderWnd->writeContentsToTimestampedFile("BOF_Screenshot_", ".png");
		return true;
	}
    
	if(m_pKeyboard->isKeyDown(OIS::KC_M))
	{
		static int mode = 0;
		
		if(mode == 2)
		{
			m_pCamera->setPolygonMode(PM_SOLID);
			mode = 0;
		}
		else if(mode == 0)
		{
            m_pCamera->setPolygonMode(PM_WIREFRAME);
            mode = 1;
		}
		else if(mode == 1)
		{
			m_pCamera->setPolygonMode(PM_POINTS);
			mode = 2;
		}
	}
    
	if(m_pKeyboard->isKeyDown(OIS::KC_O))
	{
		if(m_pTrayMgr->isLogoVisible())
        {
            m_pTrayMgr->hideLogo();
            m_pTrayMgr->hideFrameStats();
        }
        else
        {
            m_pTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
            m_pTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
        }
	}
    
#endif
	return true;
}

bool OgreFramework::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

#if defined(__ANDROID__)
bool OgreFramework::touchMoved(const OIS::MultiTouchEvent &evt)
{
    OIS::MultiTouchState state = evt.state;
    int origTransX = 0, origTransY = 0;
#if !OGRE_NO_VIEWPORT_ORIENTATIONMODE
    switch(m_pCamera->getViewport()->getOrientationMode())
    {
        case Ogre::OR_LANDSCAPELEFT:
            origTransX = state.X.rel;
            origTransY = state.Y.rel;
            state.X.rel = -origTransY;
            state.Y.rel = origTransX;
            break;
            
        case Ogre::OR_LANDSCAPERIGHT:
            origTransX = state.X.rel;
            origTransY = state.Y.rel;
            state.X.rel = origTransY;
            state.Y.rel = origTransX;
            break;
            
            // Portrait doesn't need any change
        case Ogre::OR_PORTRAIT:
        default:
            break;
    }
#endif
	m_pCamera->yaw(Degree(state.X.rel * -0.1));
	m_pCamera->pitch(Degree(state.Y.rel * -0.1));
	
	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

bool OgreFramework::touchPressed(const OIS:: MultiTouchEvent &evt)
{
#pragma unused(evt)
	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

bool OgreFramework::touchReleased(const OIS:: MultiTouchEvent &evt)
{
#pragma unused(evt)
	return true;
}

bool OgreFramework::touchCancelled(const OIS:: MultiTouchEvent &evt)
{
#pragma unused(evt)
	return true;
}
#else
bool OgreFramework::mouseMoved(const OIS::MouseEvent &evt)
{
	m_pCamera->yaw(Degree(evt.state.X.rel * -0.1f));
	m_pCamera->pitch(Degree(evt.state.Y.rel * -0.1f));
	
	return true;
}

bool OgreFramework::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}

bool OgreFramework::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}
#endif

void OgreFramework::updateOgre(double timeSinceLastFrame)
{
	m_MoveScale = m_MoveSpeed   * (float)timeSinceLastFrame;
	m_RotScale  = m_RotateSpeed * (float)timeSinceLastFrame;

#if OGRE_VERSION >= 0x10800
    m_pSceneMgr->setSkyBoxEnabled(true);
#endif
    
	m_TranslateVector = Vector3::ZERO;
    
	getInput();
	moveCamera();
    
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;
    //m_pTrayMgr->frameRenderingQueued(m_FrameEvent);
    
    // smartbody update
    
    static double timer = 0.0;
    timer += .033;
	 SBUpdateX(timer);

     for (int i=0;i<m_characterNameList.size();i++)
      {
    	 std::string charName = m_characterNameList[i];
    	 Ogre::SceneNode* characterNode = (SceneNode *)m_pSceneMgr->getRootSceneNode()->getChild(charName);
    	 Ogre::SceneNode* skelNode = (SceneNode *)m_pSceneMgr->getRootSceneNode()->getChild(charName+"_skel");
    	 Ogre::Entity* entity = (Entity *)characterNode->getAttachedObject(charName);
    	 Ogre::Skeleton* skeleton = entity->getSkeleton();
    	 for (int i = 0; i < skeleton->getNumBones(); i++)
    	 {
    		 Ogre::Bone* bone = skeleton->getBone(i);
    		 bone->setManuallyControlled(true);
    	 }
    	 Ogre::Node* character = m_pSceneMgr->getRootSceneNode()->getChild(charName);
    	 float x, y, z, qw, qx, qy, qz;
    	 getCharacterWo(charName.c_str(), x, y, z, qw, qx, qy, qz);
    	 character->setPosition(x, y, z);
    	 character->setOrientation(Quaternion(qw, qx, qy, qz));
    	 skelNode->setPosition(x, y, z);
    	 skelNode->setOrientation(Quaternion(qw, qx, qy, qz));
    	 for (int i = 0; i < skeleton->getNumBones(); i++)
    	 {
    		 Ogre::Bone* bone = skeleton->getBone(i);
    		 if (bone)
    		 {
    	             float q = 1;
    	             float x = 0;
    	             float y = 0;
    	             float z = 0;
    	             Quaternion quat;
    	             getJointRotation(charName.c_str(), bone->getName().c_str(), q, x, y, z);
    	             quat.w = q;
    	             quat.x = x;
    	             quat.y = y;
    	             quat.z = z;
    	             bone->setOrientation(quat);
    	    }
    	}

    	 for (int i = 0; i < skeleton->getNumBones(); i++)
    	 {
    		 Ogre::Bone* bone = skeleton->getBone(i);
    		 bone->_update(true,true);
    		 bone->needUpdate();
    	 }
    	 //bradSkel->_updateTransforms();
    	 //bradSkel->_notifyManualBonesDirty();
    	 for (int i = 0; i < skeleton->getNumBones(); i++)
    	 {
    	     		Ogre::Bone* bone = skeleton->getBone(i);
    	     		Ogre::String jointName = charName+"_joint#"+Ogre::StringConverter::toString(i);
    	     		Ogre::SceneNode* jointNode = (SceneNode *)skelNode->getChild(jointName);
    	     		jointNode->setOrientation(bone->_getDerivedOrientation());
    	     		jointNode->setPosition(bone->_getDerivedPosition());
    	 }
    	 //
    	 if (m_ShowDeformModel)
    	 {
    		 characterNode->setVisible(true);
    		 skelNode->setVisible(false);
    	 }
    	 else
    	 {
    		 characterNode->setVisible(false);
    		 skelNode->setVisible(true);
    	 }
      }
    
     /*
    Ogre::SceneNode* bradNode = (SceneNode *)m_pSceneMgr->getRootSceneNode()->getChild("Brad");
    Ogre::Entity* bradEnt = (Entity *)bradNode->getAttachedObject("Brad");
    Ogre::Skeleton* bradSkel = bradEnt->getSkeleton();
    
    Ogre::SceneNode* doctorNode = (SceneNode *)m_pSceneMgr->getRootSceneNode()->getChild("Doctor");
    Ogre::Entity* doctorEnt = (Entity *)doctorNode->getAttachedObject("Doctor");
    Ogre::Skeleton* doctorSkel = doctorEnt->getSkeleton();    
    
    // setup once
    static bool setupId = true;
    if (setupId)
    {
        // update bone rotations
        for (int i = 0; i < bradSkel->getNumBones(); i++)
        {
            Ogre::Bone* bradBone = bradSkel->getBone(i);
            jointNames[i] = bradBone->getName();
        }
        numberJoints = bradSkel->getNumBones();
        setupId = false;
        
        for (int i = 0; i < bradSkel->getNumBones(); i++)
        {
            Ogre::Bone* bradBone = bradSkel->getBone(i);
            Ogre::Bone* doctorBone = doctorSkel->getBone(i); 
            bradBone->setManuallyControlled(true);
            doctorBone->setManuallyControlled(true);
        }
    }
    */
    
    // update root joint
    //Ogre::Node* bradCharacter = m_pSceneMgr->getRootSceneNode()->getChild("Brad");
    //Ogre::Node* doctorCharacter = m_pSceneMgr->getRootSceneNode()->getChild("Doctor");
 /*   
    
    getJointInfo("brad", characterPosition, boneData, jointNames, numberJoints);
    bradCharacter->setPosition(characterPosition[0], characterPosition[1], characterPosition[2]);
    bradCharacter->setOrientation(characterPosition[3], characterPosition[4], characterPosition[5], characterPosition[6]);   
    for (int i = 0; i < bradSkel->getNumBones(); i++)
    {
        Ogre::Bone* bradBone = bradSkel->getBone(i);
        bradBone->setManuallyControlled(true);
        bradBone->setOrientation(boneData[i * 4 + 0], boneData[i * 4 + 1], boneData[i * 4 + 2], boneData[i * 4 + 3]);
    }
    
    getJointInfo("doctor", characterPosition, boneData, jointNames, numberJoints);
    doctorCharacter->setPosition(characterPosition[0], characterPosition[1], characterPosition[2]);
    doctorCharacter->setOrientation(characterPosition[3], characterPosition[4], characterPosition[5], characterPosition[6]);  
    for (int i = 0; i < doctorSkel->getNumBones(); i++)
    {
        Ogre::Bone* bradBone = doctorSkel->getBone(i);
        bradBone->setManuallyControlled(true);
        bradBone->setOrientation(boneData[i * 4 + 0], boneData[i * 4 + 1], boneData[i * 4 + 2], boneData[i * 4 + 3]);
    }  
 */
     /*
    float x, y, z, qw, qx, qy, qz;
    getCharacterWo("brad", x, y, z, qw, qx, qy, qz);
    bradCharacter->setPosition(x, y, z);
    bradCharacter->setOrientation(Quaternion(qw, qx, qy, qz));  
    getCharacterWo("doctor", x, y, z, qw, qx, qy, qz);   
    doctorCharacter->setPosition(x, y, z);
    doctorCharacter->setOrientation(Quaternion(qw, qx, qy, qz));
    */
    
    // update bone rotations
     /*
    for (int i = 0; i < bradSkel->getNumBones(); i++)
    {
        Ogre::Bone* bradBone = bradSkel->getBone(i);
        Ogre::Bone* doctorBone = doctorSkel->getBone(i);
        if (bradBone && doctorBone)
        {
    //        bradBone->setManuallyControlled(true);
    //        doctorBone->setManuallyControlled(true);
            float q = 1;
            float x = 0;
            float y = 0;
            float z = 0;
            Quaternion quat;
            getJointRotation("brad", bradBone->getName().c_str(), q, x, y, z);
            quat.w = q;
            quat.x = x;
            quat.y = y;
            quat.z = z;
            bradBone->setOrientation(quat);
            getJointRotation("doctor", doctorBone->getName().c_str(), q, x, y, z);
            quat.w = q;
            quat.x = x;
            quat.y = y;
            quat.z = z;            
            doctorBone->setOrientation(quat);
        }       
    }
    */
    //bradSkel->_updateTransforms();
    //bradEnt->getAllAnimationStates()->_notifyDirty();
    //bradEnt->_updateAnimation();
}

void OgreFramework::moveCamera()
{
#if !defined(__ANDROID__)
	if(m_pKeyboard->isKeyDown(OIS::KC_LSHIFT)) 
		m_pCamera->moveRelative(m_TranslateVector);
	else
#endif

		m_pCamera->moveRelative(m_TranslateVector / 10);
}

void OgreFramework::getInput()
{
#if !defined(__ANDROID__)
	if(m_pKeyboard->isKeyDown(OIS::KC_A))
		m_TranslateVector.x = -m_MoveScale;
	
	if(m_pKeyboard->isKeyDown(OIS::KC_D))
		m_TranslateVector.x = m_MoveScale;
	
	if(m_pKeyboard->isKeyDown(OIS::KC_W))
		m_TranslateVector.z = -m_MoveScale;
	
	if(m_pKeyboard->isKeyDown(OIS::KC_S))
		m_TranslateVector.z = m_MoveScale;
#endif
}
