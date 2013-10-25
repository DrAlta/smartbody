#include "irrlichtsmartbodylistener.h"
#include <SB/SBSceneListener.h>
#include <IAnimatedMesh.h>
#include <vhcl.h>
#include <sstream>

IrrlichtSmartBodyListener::IrrlichtSmartBodyListener(irr::scene::ISceneManager* sceneMgr, std::map<std::string, int>* characterMap)
{
	mSceneMgr = sceneMgr;
	map = characterMap;
	id = 1;
}

IrrlichtSmartBodyListener::~IrrlichtSmartBodyListener()
{
}

void IrrlichtSmartBodyListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	std::stringstream strstr;
	strstr << "../irrlicht-1.8/media/" << objectClass<< ".mesh";

	irr::scene::ISkinnedMesh* mesh = (irr::scene::ISkinnedMesh*)mSceneMgr->getMesh(strstr.str().c_str());

	if (!mesh)
	{
		LOG("Cannot find mesh named '%s", objectClass.c_str());
		return;
	}

	irr::scene::IAnimatedMeshSceneNode* node = mSceneMgr->addAnimatedMeshSceneNode( mesh, NULL, id );
	
	//must set to allow manual joint control
	node->setJointMode(irr::scene::EJUOR_CONTROL);

	irr::scene::IAnimatedMesh* animMesh  = node->getMesh();
	irr::scene::ISkinnedMesh* skinmesh = dynamic_cast<irr::scene::ISkinnedMesh*> (animMesh);

	irr::core::array<irr::scene::ISkinnedMesh::SJoint*> jointssss  =skinmesh->getAllJoints();


	for(irr::u32 i = 1; i < jointssss.size(); i++)
	{
		//Clear all joint animation keys - model takes less space on disk
		jointssss[i]->PositionKeys.clear();
		jointssss[i]->RotationKeys.clear();
		jointssss[i]->ScaleKeys.clear();
	}



	(*map)[name] = id;
	id++;

	//set texture
	node->getMaterial(0).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_body.tga"));
	node->getMaterial(1).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_body.tga"));
	node->getMaterial(2).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_clothes.tga"));
	node->getMaterial(3).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_body.tga"));
	node->getMaterial(4).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_sword.tga"));
	node->getMaterial(5).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_clothes.tga"));
	node->getMaterial(6).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_clothes.tga"));
	node->getMaterial(7).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/sinbad_clothes.tga"));
	node->getMaterial(8).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8/media/irrlicht2_dn.jpg"));

	node->setPosition(irr::core::vector3df(0,-80,0));


	node->addShadowVolumeSceneNode();
	
	//mSceneMgr->setShadowColor(irr::video::SColor(150,0,0,0));

	node->setScale(irr::core::vector3df(10,10,10));
	node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS , true);

}

void IrrlichtSmartBodyListener::OnCharacterDelete( const std::string & name )
{
}

void IrrlichtSmartBodyListener::OnCharacterChanged( const std::string& name )
{

}

void IrrlichtSmartBodyListener::OnLogMessage( const std::string & message )
{
#ifdef WIN32
	LOG(message.c_str());
#endif
}


