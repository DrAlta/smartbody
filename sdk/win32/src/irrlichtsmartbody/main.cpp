#include <irrlicht.h>
#include <string>
#include <map>

#include <sstream>

#include <iostream>
#include <sb/SBPython.h>
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSceneListener.h>

#include "irrlichtsmartbodylistener.h"


using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif



int main()
{
#ifdef WIN32
	std::string irrlichtMediaDirectory = "../irrlicht-1.8/media";
#else

	std::string irrlichtMediaDirectory = "../data/irrlichtmedia";
#endif
	const bool shadows = true;

#ifdef WIN32
		IrrlichtDevice *device =
			createDevice( video::EDT_DIRECT3D9, dimension2d<u32>(640, 480), 16,
		false, shadows);
#else
		IrrlichtDevice *device =
			createDevice( video::EDT_OPENGL, dimension2d<u32>(640, 480), 16,
		false, shadows);
#endif

	if (!device)
		return 1;


	device->setWindowCaption(L"SmartBody Irrlicht Demo");

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	std::map<std::string, int> characterMap;

	guienv->addStaticText(L"Hello World! This is the SmartBody Irrlicht Software renderer!",
		rect<s32>(10,10,260,22), true);


	// set up SmartBody
	// smartbody
	// the root path to SmartBody: change this to your own path
	std::string smartbodyRoot = "..";
	// set the following to the location of the Python libraries. 
	// if you downloaded SmartBody, it will be in core/smartbody/Python27/Lib
#ifdef WIN32
	initPython(smartbodyRoot + "/Python27/lib");
#else
	initPython("/usr/lib/python2.7");
#endif
	SmartBody::SBScene* m_pScene = SmartBody::SBScene::getScene();

	m_pScene->startFileLogging("smartbody.log");
	IrrlichtSmartBodyListener* listener = new IrrlichtSmartBodyListener(smgr, &characterMap);
	m_pScene->addSceneListener(listener);
	m_pScene->start();

	// sets the media path, or root of all the data to be used
	// other paths will be relative to the media path
	m_pScene->setMediaPath(smartbodyRoot + "/data");
	m_pScene->addAssetPath("script", ".");
	// the file 'irrlichtsmartbody.py' needs to be placed in the media path directory
	m_pScene->runScript("irrlichtsmartbody.py");


	// Create a terrain scennode
   IAnimatedMesh *terrain_model = smgr->addHillPlaneMesh("groundPlane", // Name of the scenenode
                           core::dimension2d<f32>(10.0f, 10.0f), // Tile size
                           core::dimension2d<u32>(100.0f, 100.0f), // Tile count
                           0, // Material
                           0.0f, // Hill height
                           core::dimension2d<f32>(0.0f, 0.0f), // countHills
                           core::dimension2d<f32>(10.0f, 10.0f)); // textureRepeatCount
                                          
   irr::scene::IAnimatedMeshSceneNode* terrain_node = smgr->addAnimatedMeshSceneNode(terrain_model);
   std::string textureLocation = irrlichtMediaDirectory + "/wall.jpg";
   terrain_node->setMaterialTexture(0, driver->getTexture(textureLocation.c_str()));   
   terrain_node->setMaterialFlag(EMF_LIGHTING, false);
   
   // Insert it into the scene
   terrain_node->setPosition(vector3df(0,-5,0));

	scene::ISceneNode* node = 0;
	scene::IAnimatedMesh* mesh = 0;


	// create light

	node = smgr->addLightSceneNode(0, core::vector3df(0,300,300),
	video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 800.0f);
	scene::ISceneNodeAnimator* anim = 0;


	// attach billboard to light
	node = smgr->addBillboardSceneNode(node, core::dimension2d<f32>(50, 50));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	std::string particleTexture = irrlichtMediaDirectory + "/particlewhite.bmp";
	node->setMaterialTexture(0, driver->getTexture(particleTexture.c_str()));

	smgr->setShadowColor(video::SColor(150,0,0,0));

	device->getCursorControl()->setVisible(true);

	smgr->addCameraSceneNodeFPS();
	smgr->addCameraSceneNode(0, vector3df(0,200,400), vector3df(0,0,0));
	smgr->getActiveCamera()->setFarValue(10000.0f);

	
	while(device->run())
	{

		//smartbody stuff
		SmartBody::SBSimulationManager* sim = m_pScene->getSimulationManager();
		sim->setTime(device->getTimer()->getTime() / 1000.0f);
		m_pScene->update();


		int numCharacters = m_pScene->getNumCharacters();
		if (numCharacters == 0)
			return true;

		const std::vector<std::string>& characterNames = m_pScene->getCharacterNames();

		for (size_t n = 0; n < characterNames.size(); n++)
		{

			SmartBody::SBCharacter* character = m_pScene->getCharacter(characterNames[n]);

			// find the Irrlicht node's id from the name
			std::map<std::string, int>::iterator iter = characterMap.find(character->getName());

			if (iter == characterMap.end())
				continue;

			irr::scene::ISceneNode* node = smgr->getSceneNodeFromId((*iter).second);
			irr::scene::IAnimatedMeshSceneNode* animatedMeshNode  = dynamic_cast<irr::scene::IAnimatedMeshSceneNode*>(node);


			if (!animatedMeshNode)
				continue;

			//must have for manual joint control 
			animatedMeshNode->animateJoints();	

			SrVec pos = character->getPosition();
			SrVec hpr = character->getHPR();

			//-x to rotate anbd move the whole mesh the correct way due to smartbody Right hand system
			// *10 for synchronizing scaling it by 10 initially
			animatedMeshNode->setPosition(irr::core::vector3df(-pos.x*10, pos.y*10,pos.z*10));
			animatedMeshNode->setRotation(irr::core::vector3df(hpr.z,-hpr.x, hpr.y ));


			// Update joints
			SmartBody::SBSkeleton* sbSkel = character->getSkeleton();

			int found = 0;

			int numJoints = sbSkel->getNumJoints();
			for (int j = 0; j < numJoints; j++)
			{
				SmartBody::SBJoint* joint = sbSkel->getJoint(j);
			
				IBoneSceneNode* iJoint = animatedMeshNode->getJointNode(joint->getName().c_str());


				if (!iJoint)
					continue;

				SrQuat jointQuat = joint->getQuaternion();
				irr::core::quaternion quat =  irr::core::quaternion(-jointQuat.x,jointQuat.y, jointQuat.z,-jointQuat.w);
				irr::core::vector3df irrlichrot;
				quat.toEuler(irrlichrot);

				irr::f32 x = irr::core::radToDeg(irrlichrot.X);
				irr::f32 y = irr::core::radToDeg(irrlichrot.Y);
				irr::f32 z = irr::core::radToDeg(irrlichrot.Z);

				iJoint->setRotation(irr::core::vector3df(x,y,z));	

			}		

		}//for

		//irrlicht stuff
		driver->beginScene(true, true, 0);
		smgr->drawAll();
		guienv->drawAll();
		driver->endScene();
	}


	device->drop();

	return 0;
}

