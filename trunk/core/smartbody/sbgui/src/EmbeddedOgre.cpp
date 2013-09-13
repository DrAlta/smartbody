#include "vhcl.h"
#include <sb/SBScene.h>
#include <sb/SBJoint.h>
#include <sb/SBSkeleton.h>
#include <sb/SBCharacter.h>
#include <sb/SBAttribute.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmTexture.h>
#if !defined(WIN32)
#include <GL/glx.h>
#endif
#include "OgreFrameListener.h"
#include "SBOgreListener.h"
#include "EmbeddedOgre.h"

#include <boost/lexical_cast.hpp>

#ifdef INTMAX_C 
#undef INTMAX_C
#endif
#ifdef UINTMAX_C
#undef UINTMAX_C
#endif

#define USE_RTSHADER_SYSTEM

using namespace Ogre;

EmbeddedOgre::EmbeddedOgre(void)
{
	ogreRoot = NULL;
	ogreWnd  = NULL;
	ogreSceneMgr = NULL;
	ogreFrameListener = NULL;
	ogreCharacterVisible = false;
}

EmbeddedOgre::~EmbeddedOgre(void)
{
}


void EmbeddedOgre::setupResource()
{
	// Load resource paths from config file
	ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;

			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}

	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}


void EmbeddedOgre::resetOgreScene()
{
	Ogre::TextureManager& ogreTexManager = Ogre::TextureManager::getSingleton();	
	Ogre::MeshManager& meshManager = Ogre::MeshManager::getSingleton();	
	Ogre::SkeletonManager& skelManager = Ogre::SkeletonManager::getSingleton();
	
	ogreSceneMgr->getRootSceneNode()->detachAllObjects();
	ogreSceneMgr->destroyAllEntities();	
	ogreSceneMgr->clearScene();
	
	meshManager.unloadAll();
	meshManager.removeAll();
	ogreTexManager.unloadAll();
	skelManager.unloadAll();
	skelManager.removeAll();		

// 	std::vector<std::string> characterNames = SmartBody::SBScene::getScene()->getCharacterNames();
// 	for (unsigned int i=0;i<characterNames.size();i++)
// 	{
// 		ogreListener->OnCharacterUpdate(characterNames[i],"character");
// 	}		

	createDefaultScene();
}

void EmbeddedOgre::setCharacterVisibility( bool bVisible )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> charNames = scene->getCharacterNames();
	ogreCharacterVisible = bVisible;
	for (unsigned int i=0;i<charNames.size();i++)
	{
		std::string name = charNames[i];
		SmartBody::SBCharacter* sbChar = scene->getCharacter(charNames[i]);
		if (sbChar)
		{
			sbChar->dMeshInstance_p->setVisibility(true);
		}
		if (ogreSceneMgr->hasSceneNode(name))
		{
			SceneNode* node = ogreSceneMgr->getSceneNode(name);
			node->setVisible(bVisible);
		}		
	}
}



void EmbeddedOgre::setCharacterVisible( bool bVisible, std::string charName )
{
	if (ogreSceneMgr->hasSceneNode(charName))
	{
		SceneNode* node = ogreSceneMgr->getSceneNode(charName);
		Ogre::Entity* ogreChar = dynamic_cast<Ogre::Entity*>(node->getAttachedObject(charName));
		node->setVisible(bVisible);

	}		
}

bool EmbeddedOgre::getCharacterVisiblility()
{
	return ogreCharacterVisible;
}


void EmbeddedOgre::updateOgreLights()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	float inverseScale = float(1.0/scene->getScale());
	const std::vector<std::string>& pawnNames =  SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		iter != pawnNames.end();
		iter++)
	{
		SmartBody::SBPawn* sbpawn = SmartBody::SBScene::getScene()->getPawn(*iter);
		const std::string& name = sbpawn->getName();
		if (name.find("light") == 0)
		{
			if (firstTime)
			{
				ogreSceneMgr->destroyAllLights();
				firstTime = false;
			}
			Light * light;
			SrVec pos = sbpawn->getPosition();
			try {
				light =  ogreSceneMgr->getLight(name);
			} catch( Ogre::Exception& e ) {
				light = ogreSceneMgr->createLight( name );
			}
			SmartBody::BoolAttribute* directionalAttr = dynamic_cast<SmartBody::BoolAttribute*>(sbpawn->getAttribute("lightIsDirectional"));
			if (directionalAttr)
			{
				if (directionalAttr->getValue())
				{
					if (light->getType() != Light::LT_DIRECTIONAL)
						light->setType(Light::LT_DIRECTIONAL);
					
					const Ogre::Vector3& direction = light->getDirection();
					if (direction.x != -pos.x || 
						direction.y != -pos.y || 
						direction.z != -pos.z)
						light->setDirection(-pos.x,-pos.y,-pos.z);
				}
				else
				{
					light->setType(Light::LT_POINT);
					light->setPosition(pos.x,pos.y,pos.z);
				}			
			}
			else
			{
				light->setType(Light::LT_POINT);
				light->setDirection(pos.x,pos.y,pos.z);				
			}

			SmartBody::BoolAttribute* castShadowAttr = dynamic_cast<SmartBody::BoolAttribute*>(sbpawn->getAttribute("lightCastShadow"));
			if (castShadowAttr)
			{
				if (light->getCastShadows() != castShadowAttr->getValue())
					light->setCastShadows(castShadowAttr->getValue());				
			}

			SmartBody::IntAttribute* shadowMapSize = dynamic_cast<SmartBody::IntAttribute*>(sbpawn->getAttribute("lightShadowMapSize"));
			if (shadowMapSize)
			{
				int numShadowTextures = ogreSceneMgr->getShadowTextureCount();
				for (int s = 0; s < numShadowTextures; s++)
				{
					const Ogre::TexturePtr& texturePtr = ogreSceneMgr->getShadowTexture(s);
					int textureSize = (*texturePtr).getSize();
					if (textureSize != shadowMapSize->getValue())
					{
						ogreSceneMgr->setShadowTextureSize(shadowMapSize->getValue());
					}
				}

				//ogreSceneMgr->setShadowTextureSize(shadowMapSize->getValue());			
			}

			SmartBody::Vec3Attribute* diffuseColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightDiffuseColor"));
			if (diffuseColorAttr)
			{
				const SrVec& color = diffuseColorAttr->getValue();				
				const Ogre::ColourValue& diffuse = light->getDiffuseColour();
				Ogre::ARGB rgb = diffuse.getAsARGB();
				if (diffuse.r != color[0] || 
					diffuse.g != color[1] || 
					diffuse.b != color[2])
					light->setDiffuseColour(color.x,color.y,color.z);
			}
			else
			{
				const Ogre::ColourValue& diffuse = light->getDiffuseColour();
				Ogre::ARGB rgb = diffuse.getAsARGB();
				if (diffuse.r != 1.0f || 
					diffuse.g != 0.95f || 
					diffuse.b != 0.8f )
					light->setDiffuseColour( 1.0f, 0.95f, 0.8f );
			}			
			SmartBody::Vec3Attribute* specularColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightSpecularColor"));
			if (specularColorAttr)
			{
				const SrVec& color = diffuseColorAttr->getValue();				
				const Ogre::ColourValue& specular = light->getSpecularColour();
				Ogre::ARGB rgb = specular.getAsARGB();
				if (specular.r != color[0] || 
					specular.g != color[1] || 
					specular.b != color[2])
					light->setSpecularColour(color.x,color.y,color.z);
			}
			else
			{
				const SrVec& color = diffuseColorAttr->getValue();				
				const Ogre::ColourValue& specular = light->getSpecularColour();
				Ogre::ARGB rgb = specular.getAsARGB();
				if (specular.r != 0.0f || 
					specular.g != 0.0f || 
					specular.b != 0.0f)
				light->setSpecularColour( 0.0f, 0.0f, 0.0f );
			}					
		}
	}
	if (ogreSceneMgr->getShadowFarDistance() != 30.f*inverseScale)
		ogreSceneMgr->setShadowFarDistance(30.f*inverseScale);
	if (ogreSceneMgr->getShadowDirectionalLightExtrusionDistance() != 30.f*inverseScale)
		ogreSceneMgr->setShadowDirectionalLightExtrusionDistance(30.f*inverseScale);
}

void EmbeddedOgre::createDefaultScene()
{
	
	ogreSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_MODULATIVE );
	//ogreSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_ADDITIVE );
	//ogreSceneMgr->setShadowTechnique( SHADOWTYPE_STENCIL_MODULATIVE );
	//ogreSceneMgr->setShadowTechnique( SHADOWTYPE_STENCIL_ADDITIVE );
	//ogreSceneMgr->setShadowTechnique( SHADOWTYPE_NONE );
	//ogreSceneMgr->setShadowTextureCount(1);
	//ogreSceneMgr->setShadowTextureSize( 1024 );
	//ogreSceneMgr->setShadowColour( ColourValue( 0.3f, 0.3f, 0.3f ) );	
	
	// Setup animation default
	Animation::setDefaultInterpolationMode( Animation::IM_LINEAR );
	Animation::setDefaultRotationInterpolationMode( Animation::RIM_LINEAR );

	// Set ambient light
	ogreSceneMgr->setAmbientLight( ColourValue( 0.2f, 0.2f, 0.2f ) );
	// Give it a little ambience with lights

#if 1
	Light * light1;
	Vector3 dir;

	light1 = ogreSceneMgr->createLight( "WhiteLight" );
	light1->setType( Light::LT_DIRECTIONAL );
	light1->setDirection( -1.5f, -4.5f, -8.0f );
	//light1->setSpotlightRange(Degree(30.0f), Degree(50.0f));
	//l->setCastShadows( true );
	//light1->setPowerScale( 1.0 );

	//dir = -light1->getPosition();
	//dir = Vector3( 15, 50, 0 );
	//dir.normalise();
	//light1->setDirection( dir );
	light1->setDiffuseColour( 1.0f, 1.0f, 1.0f );
	light1->setSpecularColour(1.0f, 1.0f, 1.0f);
	
// 	Light * mR_FillLight;
// 	mR_FillLight = ogreSceneMgr->createLight("R_FillLight");
// 	mR_FillLight->setType(Light::LT_SPOTLIGHT);
// 	mR_FillLight->setPosition(-150.0f, 450.0f, 200.0f);
// 	mR_FillLight->setSpotlightRange(Degree(30), Degree(30));
// 	dir = -mR_FillLight->getPosition();
// 	dir.normalise();
// 	mR_FillLight->setDirection(dir);
// 	mR_FillLight->setDiffuseColour(0.32f, 0.37f, 0.4f);
// 	mR_FillLight->setSpecularColour(0.32f, 0.37f, 0.4f);

// 	Light * mL_FillLight;
// 	mL_FillLight = ogreSceneMgr->createLight("L_FillLight");
// 	mL_FillLight->setType(Light::LT_SPOTLIGHT);
// 	mL_FillLight->setPosition(-1500.0f,100.0f,-100.0f);
// 	mL_FillLight->setSpotlightRange(Degree(30.0f), Degree(50.0f));
// 	dir = -mL_FillLight->getPosition();
// 	dir.normalise();
// 	mL_FillLight->setDirection(dir);
// 	mL_FillLight->setDiffuseColour(0.45f, 0.42f, 0.40f);
// 	mL_FillLight->setSpecularColour(0.45f, 0.42f, 0.40f);
// 
// 	Light * mBounceLight;
// 	mBounceLight = ogreSceneMgr->createLight("BounceLight");
// 	mBounceLight->setType(Light::LT_SPOTLIGHT);
// 	mBounceLight->setPosition(-50.0f,-500.0f,400.0f);
// 	mBounceLight->setSpotlightRange(Degree(30.0f), Degree(50.0f));
// 	dir = -mBounceLight->getPosition();
// 	dir.normalise();
// 	mBounceLight->setDirection(dir);
// 	mBounceLight->setDiffuseColour(0.37f, 0.37f, 0.36f);
// 	mBounceLight->setSpecularColour(0.37f, 0.37f, 0.36f);
	

	Ogre::Light* directionalLight = ogreSceneMgr->createLight("directionalLight");
	directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionalLight->setDiffuseColour(Ogre::ColourValue(1.f, 1.f, 1.f));
	directionalLight->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setDirection(Ogre::Vector3( 1.f, -2.f, 5.f ));		
	
#endif

	// Position the camera
	ogreCamera->setPosition( 0, 180, 225 );
	ogreCamera->lookAt( 0, 92, 0 );
	//adding plane entity to the scene
	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
    
	Ogre::MeshPtr planeMesh = MeshManager::getSingleton().createPlane( "Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 3500, 3500, 100, 100, true, 1, 60, 60, Vector3::UNIT_Z );
	std::string materialName = "MyplaneMat";
    
	Ogre::MaterialManager& matManager = Ogre::MaterialManager::getSingleton();
	Ogre::MaterialPtr ogreMat = matManager.create(materialName, "General", true);
	Ogre::Pass* pass = ogreMat->getTechnique(0)->getPass(0);			
	pass->setDiffuse(0.3f,0.3f,0.3f,1.f);	
	pass->setShadingMode(SO_PHONG);
	for (int i=0;i<planeMesh->getNumSubMeshes();i++)
	{
		SubMesh* subMesh = planeMesh->getSubMesh(i);
		subMesh->setMaterialName(materialName);
	}
    
	Entity * pPlaneEnt = ogreSceneMgr->createEntity( "plane", "Myplane" );	
	//pPlaneEnt->setMaterialName( "Ogre/DepthShadowmap/Receiver/RockWall" );
	//pPlaneEnt->setMaterialName( "Ogre/Compositor/HDR/GaussianBloom" );
	//pPlaneEnt->setMaterialName( "Examples/ShowNormals" );	
	pPlaneEnt->setCastShadows( false );	
	ogreSceneMgr->getRootSceneNode()->createChildSceneNode("plane_node", Vector3( 0, 0, 0 ) )->attachObject( pPlaneEnt );
	ogreSceneMgr->getSceneNode("plane_node")->setVisible(true);

	firstTime = true;
}


unsigned long EmbeddedOgre::getCurrentGLContext()
{
#ifdef WIN32
	return (unsigned long)wglGetCurrentContext();
#else
	return (unsigned long)glXGetCurrentContext();
#endif
}

void EmbeddedOgre::createOgreWindow( void* windowHandle, void* parentHandle, unsigned long glContext, int width, int height, std::string windowName )
{
	// initialize Ogre3D
	size_t winHandle = (size_t)windowHandle;
	unsigned long oldGLContext = getCurrentGLContext();//glContext;
	//LOG("embeddedOgre, current GL context = %lu, input GL context = %lu, winHandle = %lu",oldGLContext, glContext, winHandle);
	try 
	{		
		ogreRoot = OGRE_NEW Ogre::Root();

		Ogre::MaterialManager& matManager = Ogre::MaterialManager::getSingleton();

		ogreRoot->restoreConfig();
		//LogManager::getSingletonPtr()->setLogDetail(LL_BOREME);
		Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_LOW);
#ifdef WIN32
		Ogre::String pluginName = "RenderSystem_GL";	
		Ogre::String sceneManagerPlugin = "Plugin_OctreeSceneManager";
		bool lIsInDebugMode = OGRE_DEBUG_MODE;
		if(lIsInDebugMode)
		{
			pluginName.append("_d");
			sceneManagerPlugin.append("_d");
		}
#else
		Ogre::String sceneManagerPlugin = "/usr/lib/x86_64-linux-gnu/OGRE-1.8.0/Plugin_OctreeSceneManager";
		Ogre::String pluginName = "/usr/lib/x86_64-linux-gnu/OGRE-1.8.0/RenderSystem_GL";	
#endif
		ogreRoot->loadPlugin(pluginName);		
		ogreRoot->loadPlugin(sceneManagerPlugin);
#if 1 //OGRE_VERSION_MINOR > 7 && OGRE_VERSION_MAJOR == 1
		const Ogre::RenderSystemList& lRenderSystemList = (ogreRoot->getAvailableRenderers());
		Ogre::RenderSystem *lRenderSystem = lRenderSystemList[0];		
#else
                const Ogre::RenderSystemList* lRenderSystemList = (ogreRoot->getAvailableRenderers());
                Ogre::RenderSystem *lRenderSystem = (*lRenderSystemList)[0];
#endif

		ogreRoot->setRenderSystem(lRenderSystem);

#ifdef WIN32
		ogreRoot->addResourceLocation("../../../../lib/OgreSDK/media/materials/programs","FileSystem");
		ogreRoot->addResourceLocation("../../../../lib/OgreSDK/media/materials/scripts","FileSystem");
		ogreRoot->addResourceLocation("../../../../lib/OgreSDK/media/materials/textures","FileSystem");
		ogreRoot->addResourceLocation("../../../../lib/OgreSDK/media/RTShaderLib/","FileSystem");
		ogreRoot->addResourceLocation("../../../../lib/OgreSDK/media/RTShaderLib/materials/","FileSystem");
#else
		ogreRoot->addResourceLocation("/usr/share/OGRE-1.8.0/media/materials/programs","FileSystem");
		ogreRoot->addResourceLocation("/usr/share/OGRE-1.8.0/media/materials/scripts","FileSystem");
		ogreRoot->addResourceLocation("/usr/share/OGRE-1.8.0/media/materials/textures","FileSystem");
		ogreRoot->addResourceLocation("/usr/share/OGRE-1.8.0/media/RTShaderLib/","FileSystem");
		ogreRoot->addResourceLocation("/usr/share/OGRE-1.8.0/media/RTShaderLib/materials/","FileSystem");
#endif

		ogreWnd = ogreRoot->initialise( false );
		Ogre::NameValuePairList params;
		//if (parentHandle)
		//	params["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)parentHandle);	
        
        char charHandle[256];
        sprintf(charHandle,"%ld",winHandle);
        Ogre::String strHandle = Ogre::StringConverter::toString((size_t)winHandle);
        
		//printf("window handle = %lu, strHandle = %s, charHandle = %s\n",winHandle, strHandle.c_str(), charHandle);
#if defined(__APPLE__)        
        params["macAPI"] = "cocoa";	
        params["macAPICocoaUseNSView"] = "true";	
#elif defined(linux)
        params["currentGLContext"] = String("true");
#else
        params["externalGLControl"] = Ogre::StringConverter::toString( true );
		params["externalGLContext"] = Ogre::StringConverter::toString( (unsigned long)getCurrentGLContext() );
        
#endif
		params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)winHandle);	
		
		ogreWnd = ogreRoot->createRenderWindow( windowName, width, height, false, &params );
		ogreGLContext = (unsigned long)getCurrentGLContext();

		
#if defined(USE_RTSHADER_SYSTEM) 	
		if (Ogre::RTShader::ShaderGenerator::initialize())
		{
			ResourceGroupManager::getSingleton().initialiseAllResourceGroups();		
			// Grab the shader generator pointer.
			ogreShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
			//Add the hardware skinning to the shader generator default render state
			ogreSrsHardwareSkinning = ogreShaderGenerator->createSubRenderState(Ogre::RTShader::HardwareSkinning::Type);
			Ogre::RTShader::RenderState* renderState = ogreShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			renderState->addTemplateSubRenderState(ogreSrsHardwareSkinning);

			Ogre::MaterialManager& matManager = Ogre::MaterialManager::getSingleton();

			Ogre::MaterialPtr pCast1 = matManager.getByName("Ogre/RTShader/shadow_caster_dq_skinning_1weight");
			Ogre::MaterialPtr pCast2 = matManager.getByName("Ogre/RTShader/shadow_caster_dq_skinning_2weight");
			Ogre::MaterialPtr pCast3 = matManager.getByName("Ogre/RTShader/shadow_caster_dq_skinning_3weight");
			Ogre::MaterialPtr pCast4 = matManager.getByName("Ogre/RTShader/shadow_caster_dq_skinning_4weight");			
			Ogre::RTShader::HardwareSkinningFactory::getSingleton().setCustomShadowCasterMaterials(
				Ogre::RTShader::ST_DUAL_QUATERNION, pCast1, pCast2, pCast3, pCast4);
		}

#endif
		

		// setup scene manager
		ogreSceneMgr= ogreRoot->createSceneManager("OctreeSceneManager", "MyFirstSceneManager");
		//ogreSceneMgr= ogreRoot->createSceneManager("DefaultSceneManager", "MyFirstSceneManager");

		LOG("Scene Manager Type = %s",ogreSceneMgr->getTypeName().c_str());
		// The 'root SceneNode' is the only scenenode at the beginning in the SceneManager.
		// The SceneNodes can be seen as 'transformation' containers <=> it contains scale/position/rotation
		// of the objects. There is only 1 root scenenode, and all other scenenode are 
		// its direct or indirect children.
		Ogre::SceneNode* lRootSceneNode = ogreSceneMgr->getRootSceneNode();

		// I create a camera. It represent a 'point of view' in the scene.		
		ogreCamera = ogreSceneMgr->createCamera("PlayerCam");
		// Position it at 500 in Z direction
		ogreCamera->setPosition(Vector3(0,0,500));
		// Look back along -Z
		ogreCamera->lookAt(Vector3(0,0,-300));
		ogreCamera->setNearClipDistance(5);
		ogreCamera->setAutoAspectRatio(true);

		// We create a viewport on a part of the window.
		// A viewport is the link between 1 camera and 1 drawing surface (here the window).
		// I can then call 'update();' on it to make it draw the Scene from the camera.
		// You can have several viewports on 1 window.
		// Check API for details on parameters.
		float lViewportWidth = 1.f;
		float lViewportHeight = 1.f;
		float lViewportLeft	= (1.0f - lViewportWidth) * 0.5f;
		float lViewportTop = (1.0f - lViewportHeight) * 0.5f;
		unsigned short lMainViewportZOrder = 100;
		Ogre::Viewport * vp = ogreWnd->addViewport(ogreCamera, lMainViewportZOrder, lViewportLeft, lViewportTop, lViewportWidth, lViewportHeight);		
		vp->setBackgroundColour(Ogre::ColourValue(0.63f,0.63f,0.63f));
		// I choose the visual ratio of the camera. To make it looks real, I want it the same as the viewport.
		float ratio = float(vp->getActualWidth()) / float(vp->getActualHeight());
		ogreCamera->setAspectRatio(ratio);
		// I choose the clipping far& near planes. if far/near>2000, you can get z buffer problem.
		// eg : far/near = 10000/5 = 2000 . it's ok.
		// If (far/near)>2000 then you will likely get 'z fighting' issues.
		ogreCamera->setNearClipDistance(1.5f);
		ogreCamera->setFarClipDistance(3000.0f); 

		// I want my window to be active
		ogreWnd->setActive(true);
		// I want to update myself the content of the window, not automatically.
		ogreWnd->setAutoUpdated(false);	
		
		//setupResource();
		//createDefaultScene();
	}
	catch( Ogre::Exception& e )
	{
		Ogre::String s = "FLTKOgreWindow::createOgreWindow() - Exception:\n" + e.getFullDescription() +  "\n"; 
		Ogre::LogManager::getSingleton().logMessage( s, Ogre::LML_CRITICAL );		
	}
	//LOG("Finish setup resource");
	// create frame listener
	ogreFrameListener = new OgreFrameListener(ogreWnd,ogreCamera,"Create Ogre Frame Listener", ogreSceneMgr, this);
	ogreRoot->addFrameListener(ogreFrameListener);
	//LOG("Before setup default scene");
	createDefaultScene();	
	//LOG("After setup default scene");	

}

void EmbeddedOgre::update()
{
	ogreWnd->update(false);	
	
	// This update some internal counters and listeners.
	// Each render surface (window/rtt/mrt) that is 'auto-updated' has got its 'update' function called.
	ogreRoot->renderOneFrame();
}

void EmbeddedOgre::finishRender()
{
	bool lVerticalSynchro = true;	
	ogreWnd->swapBuffers(lVerticalSynchro);		
}

Ogre::Entity* EmbeddedOgre::createOgreCharacter( SmartBody::SBCharacter* sbChar )
{
	Ogre::Entity* outChar = NULL;
	std::string skeletonName = sbChar->getSkeleton()->getName();
	std::string meshName = sbChar->getStringAttribute("deformableMesh");
	if (meshName == "") meshName = sbChar->getName();

	SmartBody::SBSkeleton* charSkel = SmartBody::SBScene::getScene()->getSkeleton(skeletonName);
	addSBSkeleton(charSkel);
	addDeformableMesh(meshName, sbChar->dMesh_p);
	Ogre::SkeletonPtr ogreSkel = Ogre::SkeletonManager::getSingleton().getByName(skeletonName);
	Ogre::MeshPtr     ogreMesh = Ogre::MeshManager::getSingleton().getByName(meshName);	
	if (ogreSkel.isNull() || ogreMesh.isNull()) return NULL;
   

	DeformableMesh* deformMesh = sbChar->dMesh_p;	
 	ogreMesh->setSkeletonName(skeletonName);	
 	ogreMesh->_notifySkeleton(ogreSkel);
	// combine skeleton and mesh with skinning information	
#if 0
	for (unsigned int i=0;i<deformMesh->skinWeights.size();i++)
	{
		// generate bone indices
		SkinWeight* skinWeight = deformMesh->skinWeights[i];
		if (deformMesh->getMesh(skinWeight->sourceMesh) == -1) continue;
		Ogre::SubMesh* subMesh = ogreMesh->getSubMesh(skinWeight->sourceMesh);		
		int globalCounter = 0;
		for (unsigned int j=0;j<subMesh->vertexData->vertexCount;j++)
		{
			Ogre::VertexBoneAssignment vba;
			// The index of the vertex in the vertex buffer
			vba.vertexIndex = static_cast<unsigned int>(j);			
			for (unsigned int k=0;k<skinWeight->numInfJoints[j];k++)
			{
				std::string jName = skinWeight->infJointName[skinWeight->jointNameIndex[globalCounter]];
				vba.boneIndex = ogreSkel->getBone(jName)->getHandle();
				vba.weight    = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
				//if (k < 4)
				subMesh->addBoneAssignment(vba);
				globalCounter++;
			}		
		}	
		subMesh->_compileBoneAssignments();
	}	
#else	
	ogreMesh->clearBoneAssignments();
	for (unsigned int j=0;j<ogreMesh->sharedVertexData->vertexCount;j++)
	{
		for (unsigned int m=0;m<1;m++)
		{
			for (unsigned int k=0;k<4;k++)
			{
				Ogre::VertexBoneAssignment vba;
				// The index of the vertex in the vertex buffer
				vba.vertexIndex = static_cast<unsigned int>(j);							
				int origBoneID = deformMesh->boneIDBuf[m][j][k];
				std::string jName = deformMesh->boneJointList[origBoneID]->getName();
				//vba.boneIndex = (ogreSkel->getBone(jName)->getHandle()+2)%ogreSkel->getNumBones();				o
				vba.boneIndex = ogreSkel->getBone(jName)->getHandle();
				//if (origBoneID != vba.boneIndex)
				//vba.boneIndex = origBoneID;
				vba.weight    = deformMesh->boneWeightBuf[m][j][k];			
				if (vba.weight > 0)
				{
					ogreMesh->addBoneAssignment(vba);		
					//LOG("jName = %s, origID = %d, ogreBoneID = %d, weight = %f",jName.c_str(),origBoneID,vba.boneIndex, vba.weight);
				}
			}	
		}			
	}
	ogreMesh->_compileBoneAssignments();	
#endif
	ogreMesh->load();
	outChar = ogreSceneMgr->createEntity(sbChar->getName(),meshName);	

#if 1	//outChar->setMaterialName("Ogre/RTShader/shadow_caster_dq_skinning_4weight");	
/*
	for (unsigned int i=0;i<outChar->getNumSubEntities();i++)
	{
		MaterialPtr entityMaterial = outChar->getSubEntity(i)->getMaterial();
		if(!entityMaterial.isNull())
		{
			Technique* bestTechnique = entityMaterial->getBestTechnique();
			if(bestTechnique)
			{
				Pass* pass = bestTechnique->getPass(0);
				if (pass && pass->hasVertexProgram() && pass->getVertexProgram()->isSkeletalAnimationIncluded()) 
				{
					LOG("Entity %d, Has Hardware Skinning",i);
				}
				else
				{
					LOG("Entity %d, Has Software Skinning",i);
				}
			}
		}
	}

	if (outChar->isHardwareAnimationEnabled())
	{
		LOG("outChar has hardware skinning");
	}
	else
	{
		LOG("outChar has software skinning");
	}
	*/
// 	RTShader::HardwareSkinningFactory::getSingleton().setMaxCalculableBoneCount(150);
// 	RTShader::HardwareSkinningFactory::getSingleton().prepareEntityForSkinning(outChar, Ogre::RTShader::ST_DUAL_QUATERNION, true, false);
// 	//The following line is needed only because the Jaiqua model material has shaders and
// 	//as such is not automatically reflected in the RTSS system
// 	for (unsigned int i=0;i<outChar->getNumSubEntities();i++)
// 	{
// 		RTShader::ShaderGenerator::getSingleton().createShaderBasedTechnique(
// 			outChar->getSubEntity(i)->getMaterialName(),
// 			Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
// 			Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
// 			true);
// 		ogreShaderGenerator->validateMaterial(Ogre::MaterialManager::DEFAULT_SCHEME_NAME,outChar->getSubEntity(i)->getMaterialName());
// 		outChar->getSubEntity(i)->getMaterial()->compile();		
//  	}			
#endif
	//outChar->setDisplaySkeleton(true);
	return outChar;
}

void EmbeddedOgre::addSBSkeleton( SmartBody::SBSkeleton* skel )
{
	if (!skel) return;
 	Ogre::String skelName = skel->getName();
 	Ogre::SkeletonManager& skelManager = Ogre::SkeletonManager::getSingleton();
	if (!skelManager.getByName(skelName).isNull()) return; // skeleton already exists
	Ogre::SkeletonPtr ogreSkel = skelManager.create(skelName,"General",true);
	
	// setup the initial state 
	for (int i=0;i<skel->getNumJoints();i++)
	{
		SmartBody::SBJoint* joint = skel->getJoint(i);
		Bone* ogBone = ogreSkel->createBone(joint->getName());
		SrVec p = joint->getOffset();
		ogBone->setPosition(p.x,p.y,p.z);
		SrQuat q = joint->quat()->orientation()*joint->quat()->prerot();
		ogBone->setOrientation(q.w,q.x,q.y,q.z);
		ogBone->setManuallyControlled(true);
		ogBone->setInitialState();		
	}
	// setup the hierarchy
	for (int i=0;i<skel->getNumJoints();i++)
	{
		SmartBody::SBJoint* joint = skel->getJoint(i);
		Bone* ogBone = ogreSkel->getBone(joint->getName());
		for (int k=0;k<joint->getNumChildren();k++)
		{
			Bone* childBone = ogreSkel->getBone(joint->getChild(k)->getName());
			ogBone->addChild(childBone);
		}
	}
	ogreSkel->load(); // load the skeleton	
}

void EmbeddedOgre::addDeformableMesh( std::string meshName, DeformableMesh* mesh )
{
#if 0
	Ogre::MeshManager& meshManager = Ogre::MeshManager::getSingleton();	
	Ogre::MeshPtr ogreMesh = meshManager.create(meshName,"General",true);
	for (unsigned int i=0;i<mesh->dMeshStatic_p.size();i++)
	{
		SrModel& subModel = mesh->dMeshStatic_p[i]->shape();
		std::string subModelName = subModel.name;
		Ogre::SubMesh* ogSubMesh = ogreMesh->createSubMesh(subModelName);
		Ogre::VertexData* vtxData = new Ogre::VertexData();
		ogSubMesh->useSharedVertices = false;
		ogSubMesh->vertexData = vtxData;
		vtxData->vertexCount = subModel.V.size();
		Ogre::VertexDeclaration* decl = vtxData->vertexDeclaration;
		size_t offset = 0;
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		// store texture coordinate in a separate element
		//decl->addElement(1, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
		
		// create vertex buffer 0 : position and normal
		Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			decl->getVertexSize(0),                     // This value is the size of a vertex in memory
			vtxData->vertexCount,                       // The number of vertices you'll put into this buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
			);	
		float* tempFloatArray;
		tempFloatArray = new float[vtxData->vertexCount*6];
		for (int j=0;j<subModel.V.size();j++)
		{
			for (int k=0;k<3;k++)
			{
				tempFloatArray[j*6+k] = subModel.V.get(j)[k];
				tempFloatArray[j*6+3+k] = subModel.N.get(j)[k];
			}			
		}
		vbuf->writeData(0, vbuf->getSizeInBytes(), tempFloatArray, true);
		Ogre::VertexBufferBinding* bind = vtxData->vertexBufferBinding;
		bind->setBinding(0, vbuf);

		// create vertex buffer 1 : texture coordinate
// 		Ogre::HardwareVertexBufferSharedPtr tbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
// 			decl->getVertexSize(1),                     // This value is the size of a vertex in memory
// 			vtxData->vertexCount,                       // The number of vertices you'll put into this buffer
// 			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
// 			);			
// 		tbuf->writeData(0, tbuf->getSizeInBytes(), &subModel.T[0], true);		
// 		bind->setBinding(1, tbuf);

		// create index buffer
		unsigned int* tempUIntArray;
		tempUIntArray = new unsigned int[subModel.F.size()*3];
		for (int j=0;j<subModel.F.size();j++)
		{
			tempUIntArray[j*3] = (unsigned int)subModel.F[j].a;
			tempUIntArray[j*3+1] = (unsigned int)subModel.F[j].b;
			tempUIntArray[j*3+2] = (unsigned int)subModel.F[j].c;
		}

				
		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_32BIT,        // You can use several different value types here
			subModel.F.size()*3,                       // The number of indices you'll put in that buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
			);
		ibuf->writeData(0, ibuf->getSizeInBytes(), tempUIntArray, true);
		ogSubMesh->indexData->indexBuffer = ibuf;
		ogSubMesh->indexData->indexCount  = subModel.F.size()*3;
		ogSubMesh->indexData->indexStart  = 0;
	}
	ogreMesh->_setBounds(AxisAlignedBox(-100,-100,-100,100,100,100));
	ogreMesh->_setBoundingSphereRadius(Math::Sqrt(3*100*100));
	ogreMesh->load(); // load the mesh

#else 
	// do not create anything if there is no skin weights or meshes
	if (mesh->skinWeights.size() == 0 || mesh->dMeshStatic_p.size() == 0) return; 

	Ogre::MeshManager& meshManager = Ogre::MeshManager::getSingleton();	
	if (!meshManager.getByName(meshName).isNull()) return; // mesh already exists

	Ogre::MeshPtr ogreMesh = meshManager.create(meshName,"General",true);
	mesh->buildVertexBuffer(); // if not already built
	Ogre::VertexData* vtxData = new Ogre::VertexData();
	ogreMesh->sharedVertexData = vtxData;
	vtxData->vertexCount = mesh->posBuf.size();

	Ogre::VertexDeclaration* decl = vtxData->vertexDeclaration;
	size_t offset = 0;
	decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
	// store texture coordinate in a separate element
	//offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	decl->addElement(1, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);

	// create vertex buffer 0 : position and normal
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0),                     // This value is the size of a vertex in memory
		vtxData->vertexCount,                       // The number of vertices you'll put into this buffer
		Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, // Properties
		true
		);	
	float* tempFloatArray;
	tempFloatArray = new float[vtxData->vertexCount*6];
	SrVec bbMax, bbMin;
	bbMax = SrVec((float)-1e20,(float)-1e20,(float)-1e20);
	bbMin = SrVec((float)1e20,(float)1e20,(float)1e20);
	for (unsigned int j=0;j<vtxData->vertexCount;j++)
	{
		for (int k=0;k<3;k++)
		{
			tempFloatArray[j*6+k] = mesh->posBuf[j][k];
			tempFloatArray[j*6+3+k] = mesh->normalBuf[j][k];
			if (bbMax[k] < mesh->posBuf[j][k])
				bbMax[k] = mesh->posBuf[j][k];
			if (bbMin[k] > mesh->posBuf[j][k])
				bbMin[k] = mesh->posBuf[j][k];
		}		
		//LOG("vtx %d, mesh normal = %f %f %f", j, mesh->normalBuf[j][0], mesh->normalBuf[j][1], mesh->normalBuf[j][2]);
	}
	vbuf->writeData(0, vbuf->getSizeInBytes(), tempFloatArray, true);
	Ogre::VertexBufferBinding* bind = vtxData->vertexBufferBinding;
	bind->setBinding(0, vbuf);
	delete [] tempFloatArray;

	// create vertex buffer 1 : texture coordinate
	Ogre::HardwareVertexBufferSharedPtr tbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(1),                     // This value is the size of a vertex in memory
		vtxData->vertexCount,                       // The number of vertices you'll put into this buffer
		Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, // Properties
		true
		);		
	tempFloatArray = new float[vtxData->vertexCount*2];
	for (unsigned int j=0;j<vtxData->vertexCount;j++)
	{
		tempFloatArray[j*2] = mesh->texCoordBuf[j][0];
		tempFloatArray[j*2+1] = mesh->texCoordBuf[j][1];
	}

	tbuf->writeData(0, tbuf->getSizeInBytes(), tempFloatArray, true);		
 	bind->setBinding(1, tbuf);
	for (unsigned int i=0;i<mesh->subMeshList.size();i++)
	{
		SbmSubMesh* subModel = mesh->subMeshList[i];		
		Ogre::SubMesh* ogSubMesh = ogreMesh->createSubMesh();		
		ogSubMesh->useSharedVertices = true;		
		// create index buffer
		unsigned int* tempUIntArray;
		tempUIntArray = new unsigned int[subModel->numTri*3];
		for (int j=0;j<subModel->numTri;j++)
		{
			tempUIntArray[j*3] = (unsigned int)subModel->triBuf[j][0];
			tempUIntArray[j*3+1] = (unsigned int)subModel->triBuf[j][1];
			tempUIntArray[j*3+2] = (unsigned int)subModel->triBuf[j][2];
		}
		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_32BIT,        // You can use several different value types here
			subModel->numTri*3,                       // The number of indices you'll put in that buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, // Properties
			true
			);
		ibuf->writeData(0, ibuf->getSizeInBytes(), tempUIntArray, true);
		ogSubMesh->indexData->indexBuffer = ibuf;
		ogSubMesh->indexData->indexCount  = subModel->numTri*3;
		ogSubMesh->indexData->indexStart  = 0;

		// create material for the mesh
 		addTexture(subModel->texName);

		std::string materialName = meshName + boost::lexical_cast<std::string>(i) + "Mat";
		Ogre::MaterialPtr ogreMat = Ogre::MaterialManager::getSingleton().create(materialName, "General");
		Ogre::Pass* pass = ogreMat->getTechnique(0)->getPass(0);
 		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().getByName(subModel->texName);
		if (!texPtr.isNull())
		{
 			Ogre::TextureUnitState* texUnit = pass->createTextureUnitState();
 			texUnit->setTextureName(texPtr->getName());	
		}
 		//pass->setDiffuse(1.0,1.0,1.0,1.0);
		SrMaterial& mat = subModel->material;
		//LOG("diffuse material = %f %f %f %f",mat.diffuse.r,mat.diffuse.g,mat.diffuse.b,mat.diffuse.a);
		float color[4];
		mat.ambient.get(color);	
		//LOG("ambient color = %f %f %f",color[0],color[1],color[2]);
		//pass->setAmbient(color[0],color[1],color[2]);
		mat.diffuse.get(color);	
		//LOG("diffuse color = %f %f %f %f",color[0],color[1],color[2],color[3]);
		pass->setDiffuse(color[0],color[1],color[2],color[3]);
		mat.specular.get(color);	
		//LOG("specular color = %f %f %f %f",color[0],color[1],color[2],color[3]);
		pass->setSpecular(color[0],color[1],color[2],color[3]);
		pass->setShininess(mat.shininess);
		pass->setAlphaRejectSettings(CMPF_GREATER,0);
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);	
		pass->setSceneBlending(SBF_SOURCE_ALPHA,SBF_ONE_MINUS_SOURCE_ALPHA);
		pass->setShadingMode(SO_PHONG);
		//pass->setShadowCasterVertexProgram("Ogre/RTShader/shadow_caster_dq_skinning_4weight_vs");
		//pass->setVertexProgram("Ogre/RTShader/shadow_caster_dq_skinning_4weight_vs");		
		//pass->setVertexProgram("Ogre/HardwareSkinningFourWeightsGLSL");
		//pass->setShadowCasterVertexProgram("Ogre/HardwareSkinningFourWeightsShadowCasterGLSL");
 		ogSubMesh->setMaterialName(materialName);				
		//ogSubMesh->setMaterialName("smartbody");
	}
	ogreMesh->_setBounds(AxisAlignedBox(bbMin[0],bbMin[1],bbMin[2],bbMax[0],bbMax[1],bbMax[2]));
	ogreMesh->_setBoundingSphereRadius(Math::Sqrt(3*10*10));	
	ogreMesh->load(); // load the mesh
#endif	
}

void EmbeddedOgre::addTexture( std::string texName )
{
	
	SbmTextureManager& texManager = SbmTextureManager::singleton();
	SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE,texName.c_str());
	Ogre::TextureManager& ogreTexManager = Ogre::TextureManager::getSingleton();	
	Ogre::TexturePtr ogreTex = ogreTexManager.getByName(texName);	
	if (!ogreTex.isNull()) return; // the texture already exist in ogre
	if (!tex) return; // the texture not exist in SmartBody
	PixelFormat pixelFormat = PF_R8G8B8;
	if (tex->getNumChannels() == 4) pixelFormat = PF_R8G8B8A8;
	// initialze the texture
	ogreTex = TextureManager::getSingleton().createManual(
		texName, // name
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D,      // type
		tex->getWidth(), tex->getHeight(),         // width & height
		0,                // number of mipmaps
		pixelFormat,     // pixel format
		TU_STATIC );  	
	HardwarePixelBufferSharedPtr pixelBuffer = ogreTex->getBuffer();
	//LOG("texture format = %d, buffer format = %d",ogreTex->getFormat(), pixelBuffer->getFormat());
	pixelBuffer->lock(HardwareBuffer::HBL_NORMAL);
	const PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	uint8* pDest = static_cast<uint8*>(pixelBox.data);
	// our texture buffer format is in RGB(A), but Ogre internal format is always XBGR ( or ABGR )
	// so the following hassle is needed to convert the data to ogre format
	for (int i=0;i<tex->getHeight();i++)
		for (int j=0;j<tex->getWidth();j++)
		{
			int idx = i*tex->getWidth()*tex->getNumChannels()+j*tex->getNumChannels();
			int idx1 = i*tex->getWidth()*4+j*4;
			pDest[idx1+3] = 255;
			if (tex->getNumChannels() == 4) pDest[idx1+3] = tex->getBuffer()[idx+3];
			pDest[idx1] = tex->getBuffer()[idx+2];
			pDest[idx1+1] = tex->getBuffer()[idx+1];
			pDest[idx1+2] = tex->getBuffer()[idx+0];
		}	
	pixelBuffer->unlock();
	ogreTex->load();	
	
}

void EmbeddedOgre::updateOgreCharacterRenderMode()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> charNames = scene->getCharacterNames();
	for (unsigned int i=0;i<charNames.size();i++)
	{
		SmartBody::SBCharacter* sbChar = scene->getCharacter(charNames[i]);
		if (sbChar)
		{
			setCharacterVisible(sbChar->dMeshInstance_p->getVisibility(),charNames[i]);			
		}
	}

}
