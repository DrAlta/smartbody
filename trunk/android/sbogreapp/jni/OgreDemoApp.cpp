#include "OgreDemoApp.h"
#include "SBListener.h"
#include "test.h"
#include <iostream>
#include <sb/SBScene.h>
#include <android/log.h>

#ifndef ANDROID_LOG
#define ANDROID_LOG
#define LOG_TAG    "OgreKit"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_FOOT   LOGI("%s %s %d", __FILE__, __FUNCTION__, __LINE__)
#endif

DemoApp::DemoApp()
{
	m_pCubeNode		= 0;
	m_pCubeEntity   = 0;

	resX = 800;
	resY = 1280;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

DemoApp::~DemoApp()
{
#ifdef USE_RTSHADER_SYSTEM
    mShaderGenerator->removeSceneManager(OgreFramework::getSingletonPtr()->m_pSceneMgr);
    
    finalizeRTShaderSystem();
#endif
    
    delete OgreFramework::getSingletonPtr();
}

//|||||||||||||||||||||||||||||||||||||||||||||||

#ifdef USE_RTSHADER_SYSTEM

/*-----------------------------------------------------------------------------
 | Initialize the RT Shader system.	
 -----------------------------------------------------------------------------*/
bool DemoApp::initializeRTShaderSystem(Ogre::SceneManager* sceneMgr)
{			
    if (Ogre::RTShader::ShaderGenerator::initialize())
    {
        mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        
        mShaderGenerator->addSceneManager(sceneMgr);
        
        // Setup core libraries and shader cache path.
        Ogre::StringVector groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
        Ogre::StringVector::iterator itGroup = groupVector.begin();
        Ogre::StringVector::iterator itGroupEnd = groupVector.end();
        Ogre::String shaderCoreLibsPath;
        Ogre::String shaderCachePath;
        
        for (; itGroup != itGroupEnd; ++itGroup)
        {
            Ogre::ResourceGroupManager::LocationList resLocationsList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*itGroup);
            Ogre::ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
            Ogre::ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
            bool coreLibsFound = false;
            
            // Try to find the location of the core shader lib functions and use it
            // as shader cache path as well - this will reduce the number of generated files
            // when running from different directories.
            for (; it != itEnd; ++it)
            {
                if ((*it)->archive->getName().find("RTShaderLib") != Ogre::String::npos)
                {
                    shaderCoreLibsPath = (*it)->archive->getName() + "/";
                    shaderCachePath = shaderCoreLibsPath;
                    coreLibsFound = true;
                    break;
                }
            }
            // Core libs path found in the current group.
            if (coreLibsFound) 
                break; 
        }
        
        // Core shader libs not found -> shader generating will fail.
        if (shaderCoreLibsPath.empty())			
            return false;			
        
        // Create and register the material manager listener.
        mMaterialMgrListener = new ShaderGeneratorTechniqueResolverListener(mShaderGenerator);				
        Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
    }
    
    return true;
}

/*-----------------------------------------------------------------------------
 | Finalize the RT Shader system.	
 -----------------------------------------------------------------------------*/
void DemoApp::finalizeRTShaderSystem()
{
    // Restore default scheme.
    Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);
    
    // Unregister the material manager listener.
    if (mMaterialMgrListener != NULL)
    {			
        Ogre::MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
        delete mMaterialMgrListener;
        mMaterialMgrListener = NULL;
    }
    
    // Finalize RTShader system.
    if (mShaderGenerator != NULL)
    {				
        Ogre::RTShader::ShaderGenerator::finalize();
        mShaderGenerator = NULL;
    }
}
#endif // USE_RTSHADER_SYSTEM

OIS::AndroidInputManager* DemoApp::getInputManager()
{
	return m_pInputManagerRef;
}

void DemoApp::startDemo(int width, int height)
{
	//LOGI("startDemo");
	new OgreFramework();
	if(!OgreFramework::getSingletonPtr()->initOgre("DemoApp v1.0", this, 0,width,height))
		return;
	//LOGI("after new OgreFramework()");
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Demo initialized!");
	//LOGI("after logMessage");
	m_bShutdown = false;    	
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Demo initialized!");
	m_pInputManagerRef = OgreFramework::getSingletonPtr()->m_pInputMgr;
	
#ifdef USE_RTSHADER_SYSTEM
    initializeRTShaderSystem(OgreFramework::getSingletonPtr()->m_pSceneMgr);
    Ogre::MaterialPtr baseWhite = Ogre::MaterialManager::getSingleton().getByName("BaseWhite", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);				
    baseWhite->setLightingEnabled(false);
    mShaderGenerator->createShaderBasedTechnique(
                                                 "BaseWhite", 
                                                 Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
                                                 Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);	
    mShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, 
                                       "BaseWhite");
    baseWhite->getTechnique(0)->getPass(0)->setVertexProgram(
                                                             baseWhite->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
    baseWhite->getTechnique(0)->getPass(0)->setFragmentProgram(
                                                               baseWhite->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());
    
    // creates shaders for base material BaseWhiteNoLighting using the RTSS
    mShaderGenerator->createShaderBasedTechnique(
                                                 "BaseWhiteNoLighting", 
                                                 Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
                                                 Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);	
    mShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, 
                                       "BaseWhiteNoLighting");
    Ogre::MaterialPtr baseWhiteNoLighting = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    baseWhiteNoLighting->getTechnique(0)->getPass(0)->setVertexProgram(
                                                                       baseWhiteNoLighting->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
    baseWhiteNoLighting->getTechnique(0)->getPass(0)->setFragmentProgram(
                                                                         baseWhiteNoLighting->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());
#endif
    
	setupDemoScene();
//#if !((OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__)
	runDemo();
//#endif
}

//|||||||||||||||||||||||||||||||||||||||||||||||

void DemoApp::setupDemoScene()
{   
    OgreFramework::getSingletonPtr()->m_pSceneMgr->setSkyBox(true, "Examples/SceneSkyBox1");
    
    OgreFramework::getSingletonPtr()->m_pSceneMgr->setShadowTechnique( Ogre::SHADOWTYPE_TEXTURE_MODULATIVE );
//	OgreFramework::getSingletonPtr()->m_pSceneMgr->setShadowTextureSize( 4048 );
	OgreFramework::getSingletonPtr()->m_pSceneMgr->setShadowColour( Ogre::ColourValue( 0.3f, 0.3f, 0.3f ) );   
    OgreFramework::getSingletonPtr()->m_pSceneMgr->setAmbientLight( Ogre::ColourValue( 0.2f, 0.2f, 0.2f ) );
    Ogre::Light * l;
    Ogre::Vector3 dir;	
    l = OgreFramework::getSingletonPtr()->m_pSceneMgr->createLight("WhiteLight");
    l->setType( Ogre::Light::LT_SPOTLIGHT );
    l->setPosition( -150.0f, 450.0f, 200.0f );
    l->setCastShadows( true );
    l->setPowerScale( 1.0 );
    
    dir = -l->getPosition();
    //dir = Vector3( 15, 50, 0 );
    dir.normalise();
    l->setDirection( dir );
    l->setDiffuseColour( 1.24f, 1.22f, 1.15f );
    l->setSpecularColour(0.8f, 0.8f, 0.9f);
    
    Ogre::Light * mR_FillLight;
    mR_FillLight = OgreFramework::getSingletonPtr()->m_pSceneMgr->createLight("R_FillLight");
    mR_FillLight->setType(Ogre::Light::LT_SPOTLIGHT);
    mR_FillLight->setPosition(1500,100,200);
    mR_FillLight->setSpotlightRange(Ogre::Degree(30), Ogre::Degree(50));
    dir = -mR_FillLight->getPosition();
    dir.normalise();
    mR_FillLight->setDirection(dir);
    mR_FillLight->setDiffuseColour(0.32f, 0.37f, 0.4f);
    mR_FillLight->setSpecularColour(0.32f, 0.37f, 0.4f);
    mR_FillLight->setCastShadows(true);
    
    Ogre::Light * mL_FillLight;
    mL_FillLight = OgreFramework::getSingletonPtr()->m_pSceneMgr->createLight("L_FillLight");
    mL_FillLight->setType(Ogre::Light::LT_SPOTLIGHT);
    mL_FillLight->setPosition(-1500.0f,100.0f,-100.0f);
    mL_FillLight->setSpotlightRange(Ogre::Degree(30.0f), Ogre::Degree(50.0f));
    dir = -mL_FillLight->getPosition();
    dir.normalise();
    mL_FillLight->setDirection(dir);
    mL_FillLight->setDiffuseColour(0.45f, 0.42f, 0.40f);
    mL_FillLight->setSpecularColour(0.45f, 0.42f, 0.40f);
    mL_FillLight->setCastShadows(true);
    
    Ogre::Light * mBounceLight;
    mBounceLight = OgreFramework::getSingletonPtr()->m_pSceneMgr->createLight("BounceLight");
    mBounceLight->setType(Ogre::Light::LT_SPOTLIGHT);
    mBounceLight->setPosition(-50.0f,-500.0f,400.0f);
    mBounceLight->setSpotlightRange(Ogre::Degree(30.0f), Ogre::Degree(50.0f));
    dir = -mBounceLight->getPosition();
    dir.normalise();
    mBounceLight->setDirection(dir);
    mBounceLight->setDiffuseColour(0.37f, 0.37f, 0.36f);
    mBounceLight->setSpecularColour(0.37f, 0.37f, 0.36f);    
    mBounceLight->setCastShadows(true);
    
    m_pCubeNode = OgreFramework::getSingletonPtr()->m_pSceneMgr->getRootSceneNode()->createChildSceneNode("world_scene_vh");
    m_pCubeEntity = OgreFramework::getSingletonPtr()->m_pSceneMgr->createEntity("world_entity_vh","VH_Defaultlevel_Ogre.mesh");
    m_pCubeNode->attachObject(m_pCubeEntity);
    m_pCubeNode->setVisible(true);  


    m_sbListener = new SBListener(OgreFramework::getSingletonPtr());
    SmartBody::SBScene::getScene()->addSceneListener(m_sbListener);
     
    SBInitialize("");  
}

void DemoApp::renderDemo(int width, int height)
{
	double timeSinceLastFrame = 0;
	double startTime = 0;
	/*
	//if (width != resX || height != resY)
	{
		// change render window resolution
		resX = width;
		resY = height;
		OgreFramework::getSingletonPtr()->m_pRenderWnd->resize(resX,resY);
	}
	*/
	if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isActive())
	{
		startTime = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU();            
		OgreFramework::getSingletonPtr()->m_pMouse->capture();
       OgreFramework::getSingletonPtr()->updateOgre(timeSinceLastFrame);
		OgreFramework::getSingletonPtr()->m_pRoot->renderOneFrame();        	
	}
}
//|||||||||||||||||||||||||||||||||||||||||||||||

void DemoApp::runDemo()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Start main loop...");
	
	double timeSinceLastFrame = 0;
	double startTime = 0;
    
    OgreFramework::getSingletonPtr()->m_pRenderWnd->resetStatistics();
    /*
#if (!defined(OGRE_IS_IOS)) && !((OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__)
	while(!m_bShutdown && !OgreFramework::getSingletonPtr()->isOgreToBeShutDown()) 
	{
		if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isClosed())m_bShutdown = true;
        
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
		Ogre::WindowEventUtilities::messagePump();
#endif	
		if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isActive())
		{
			startTime = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU();
            
#if !OGRE_IS_IOS
			OgreFramework::getSingletonPtr()->m_pKeyboard->capture();
#endif
			OgreFramework::getSingletonPtr()->m_pMouse->capture();
            
			OgreFramework::getSingletonPtr()->updateOgre(timeSinceLastFrame);
			OgreFramework::getSingletonPtr()->m_pRoot->renderOneFrame();
            
			timeSinceLastFrame = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU() - startTime;
		}
		else
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
		}
	}
#endif
*/    
}

//|||||||||||||||||||||||||||||||||||||||||||||||

bool DemoApp::keyPressed(const OIS::KeyEvent &keyEventRef)
{
#if !defined(__ANDROID__)
	OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
	
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_F))
	{
        //do something
	}
#endif
	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

bool DemoApp::keyReleased(const OIS::KeyEvent &keyEventRef)
{
#if !defined(__ANDROID__)
	OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
#endif

	return true;
}

//|||||||||||||||||||||||||||||||||||||||||||||||
