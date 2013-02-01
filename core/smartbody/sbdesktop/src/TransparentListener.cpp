#include "TransparentListener.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sb/SBPawn.h>
#include <sb/SBSkeleton.h>
#include "TransparentViewer.h"
#include <sbm/mcontrol_util.h>

TransparentListener::TransparentListener()
{
}

TransparentListener::~TransparentListener()
{
}

void TransparentListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	// remove any existing scene
	if (character->scene_p)
	{
		mcu.remove_scene(character->scene_p);
		character->scene_p->unref();
		character->scene_p = NULL;
	}

	character->scene_p = new SkScene();
	character->scene_p->ref();
	character->scene_p->init(character->getSkeleton());
	bool visible = character->getBoolAttribute("visible");
	if (visible)
		character->scene_p->visible(true);
	else
		character->scene_p->visible(false);
	mcu.add_scene(character->scene_p);


	// remove any existing deformable mesh
	if (character->dMesh_p)
	{
		for (size_t i = 0; i < character->dMesh_p->dMeshDynamic_p.size(); i++)
		{
			mcu.root_group_p->remove( character->dMesh_p->dMeshDynamic_p[i] );
		}
		delete character->dMesh_p;
		character->dMesh_p = NULL;
	}

	#if defined(__ANDROID__) || defined(SBM_IPHONE)
		character->dMesh_p = new DeformableMesh();
	#else
		character->dMesh_p =  new SbmDeformableMeshGPU();
		character->dMeshInstance_p =  new SbmDeformableMeshGPUInstance();
	#endif
	SmartBody::SBSkeleton* sbSkel = character->getSkeleton();
	character->dMesh_p->setSkeleton(sbSkel);
	character->dMeshInstance_p->setSkeleton(sbSkel);
}

void TransparentListener::OnCharacterDelete( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	// remove any existing scene
	if (character->scene_p)
	{
		mcu.remove_scene(character->scene_p);
		character->scene_p->unref();
		character->scene_p = NULL;
	}
	// remove any existing deformable mesh
	if (character->dMesh_p)
	{
		for (size_t i = 0; i < character->dMesh_p->dMeshDynamic_p.size(); i++)
		{
			mcu.root_group_p->remove( character->dMesh_p->dMeshDynamic_p[i] );
		}
		delete character->dMesh_p;
		character->dMesh_p = NULL;
	}

	
}

void TransparentListener::OnCharacterUpdate( const std::string & name, const std::string & objectClass )
{
	OnCharacterDelete(name);
	OnCharacterCreate(name, objectClass);
}

void TransparentListener::OnCharacterChanged( const std::string& name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	OnCharacterDelete(name);
	OnCharacterCreate(name, character->getClassType());
}

void TransparentListener::OnPawnCreate( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (!pawn)
		return;

	if (name.find("light") == 0)
	{
		pawn->registerObserver(this);
		TransparentViewer* window = dynamic_cast<TransparentViewer*>(mcu.viewer_p);
		if (window)
		{
			//window->updateLights();
		}
	}

	// remove any existing scene
	if (pawn->scene_p)
	{
		mcu.remove_scene(pawn->scene_p);
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}
	// remove any existing deformable mesh	
	if (pawn->dMesh_p)
	{
		delete pawn->dMesh_p;
		pawn->dMesh_p = NULL;
	}	
	if (pawn->dMeshInstance_p)
	{
		delete pawn->dMeshInstance_p;
		pawn->dMeshInstance_p = NULL;
	}

	pawn->dMesh_p =  new SbmDeformableMeshGPU();
	pawn->dMeshInstance_p = new SbmDeformableMeshGPUInstance();

	pawn->scene_p = new SkScene();
	pawn->scene_p->ref();
	pawn->scene_p->init(pawn->getSkeleton());
	bool visible = pawn->getBoolAttribute("visible");
	if (visible)
		pawn->scene_p->visible(true);
	else
		pawn->scene_p->visible(false);
	mcu.add_scene(pawn->scene_p);
}

void TransparentListener::OnPawnDelete( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (!pawn)
		return;
	pawn->unregisterObserver(this);

	// remove any existing scene
	if (pawn->scene_p)
	{
		mcu.remove_scene(pawn->scene_p);
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}
}


void TransparentListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{
}


void TransparentListener::OnChannel( const std::string & name, const std::string & channelName, const float value)
{
}

void TransparentListener::OnReset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void TransparentListener::notify(SmartBody::SBSubject* subject)
{
	SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(subject);
	if (pawn)
	{
		const std::string& pawnName = pawn->getName();
		if (pawn->getName().find("light") == 0)
		{
			// adjust the lights based on the new position and color
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			TransparentViewer* window = dynamic_cast<TransparentViewer*>(mcu.viewer_p);
			if (window)
			{
				//window->updateLights();
			}


		}

	}
}

