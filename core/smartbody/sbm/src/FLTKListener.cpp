#include "FLTKListener.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sbm/SBPawn.h>
#include <fltk_viewer.h>

FLTKListener::FLTKListener()
{
}

FLTKListener::~FLTKListener()
{
}

void FLTKListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = mcu.getCharacter(name);
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
	#endif
	character->dMesh_p->setSkeleton(character->getSkeleton());

}

void FLTKListener::OnCharacterDelete( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = mcu.getCharacter(name);
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

void FLTKListener::OnCharacterUpdate( const std::string & name, const std::string & objectClass )
{
	OnCharacterDelete(name);
	OnCharacterCreate(name, objectClass);
}

void FLTKListener::OnCharacterChanged( const std::string& name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* character = mcu.getCharacter(name);
	if (!character)
		return;

	OnCharacterDelete(name);
	OnCharacterCreate(name, character->getClassType());
}

void FLTKListener::OnPawnCreate( const std::string & name )
{
	if (name.find("light") == 0)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SbmPawn* pawn = mcu.getPawn(name);
		pawn->registerObserver(this);
	}
}

void FLTKListener::OnPawnDelete( const std::string & name )
{
	if (name.find("light") == 0)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SbmPawn* pawn = mcu.getPawn(name);
		pawn->unregisterObserver(this);
	}
}


void FLTKListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{
}


void FLTKListener::OnChannel( const std::string & name, const std::string & channelName, const float value)
{
}

void FLTKListener::OnReset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

}

void FLTKListener::notify(DSubject* subject)
{
	SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(subject);
	if (pawn)
	{
		const std::string& pawnName = pawn->getName();
		if (pawn->getName().find("light") == 0)
		{
			// adjust the lights based on the new position and color
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			FltkViewer* viewer = dynamic_cast<FltkViewer*>(mcu.viewer_p);
			if (viewer)
			{
				viewer->updateLights();
			}


		}

	}
}

