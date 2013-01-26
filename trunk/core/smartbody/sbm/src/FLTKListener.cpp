#include "vhcl.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include "FLTKListener.h"
#include <sb/SBPawn.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <RootWindow.h>
#include <fltk_viewer.h>


FLTKListener::FLTKListener()
{
	otherListener = NULL;
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
		character->dMeshInstance_p =  new DeformableMeshInstance();
	#else
		character->dMesh_p         =  new SbmDeformableMeshGPU();
		character->dMeshInstance_p =  new SbmDeformableMeshGPUInstance();		
	#endif
	SmartBody::SBSkeleton* sbSkel = character->getSkeleton();
	character->dMesh_p->setSkeleton(sbSkel);
	character->dMeshInstance_p->setSkeleton(sbSkel);
	
	if (otherListener)
		otherListener->OnCharacterCreate(name,objectClass);
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
#if 1 //!USE_OGRE_VIEWER
	// make sure the character isn't associated with the viewer
	BaseWindow* window = dynamic_cast<BaseWindow*>(mcu.viewer_p);
	if (window)
	{
		if (window->fltkViewer->_objManipulator.get_selected_pawn() == character)
		{
			window->fltkViewer->_objManipulator.set_selected_pawn(NULL);
			window->fltkViewer->_objManipulator.get_active_control()->detach_pawn();
			window->fltkViewer->_objManipulator.removeActiveControl();
		}
	}
#endif

	if (otherListener)
		otherListener->OnCharacterDelete(name);
}

void FLTKListener::OnCharacterUpdate( const std::string & name, const std::string & objectClass )
{
	OnCharacterDelete(name);
	OnCharacterCreate(name, objectClass);

	if (otherListener)
	{
		otherListener->OnCharacterDelete(name);
		otherListener->OnCharacterCreate(name, objectClass);
	}
}

void FLTKListener::OnCharacterChanged( const std::string& name )
{
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	OnCharacterDelete(name);
	OnCharacterCreate(name, character->getClassType());

	if (otherListener)
	{
		otherListener->OnCharacterDelete(name);
		otherListener->OnCharacterCreate(name, character->getClassType());
	}
}


void FLTKListener::OnCharacterChangeMesh( const std::string& name )
{
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;	

	if (otherListener)
	{
		otherListener->OnCharacterChangeMesh(name);		
	}	
}

void FLTKListener::OnPawnCreate( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (!pawn)
		return;

#if 1 //!USE_OGRE_VIEWER
	if (name.find("light") == 0)
	{
		pawn->registerObserver(this);
		BaseWindow* window = dynamic_cast<BaseWindow*>(mcu.viewer_p);
		if (window)
		{
			window->fltkViewer->updateLights();
		}
	}
#endif
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

	pawn->dMesh_p         =  new SbmDeformableMeshGPU();
	pawn->dMeshInstance_p =  new SbmDeformableMeshGPUInstance();	

	float sceneScale = 0.01f/SmartBody::SBScene::getScene()->getScale();

	pawn->scene_p = new SkScene();
	pawn->scene_p->ref();
	pawn->scene_p->init(pawn->getSkeleton(), sceneScale);
	bool visible = pawn->getBoolAttribute("visible");
	if (visible)
		pawn->scene_p->visible(true);
	else
		pawn->scene_p->visible(false);
	mcu.add_scene(pawn->scene_p);

	if (otherListener)
		otherListener->OnPawnCreate(name);

	// if this is a camera, update the camera list in the main window
	SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
	if (camera)
	{
		BaseWindow* window = dynamic_cast<BaseWindow*>(mcu.viewer_p);
		if (window)
		{
			window->updateCameraList();
		}
	}

}

void FLTKListener::OnPawnDelete( const std::string & name )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (!pawn)
		return;
	pawn->unregisterObserver(this);
#if 1 //!USE_OGRE_VIEWER
	BaseWindow* window = dynamic_cast<BaseWindow*>(mcu.viewer_p);
	if (window)
	{
		if (window->fltkViewer->_objManipulator.get_selected_pawn() == pawn)
		{
			window->fltkViewer->_objManipulator.set_selected_pawn(NULL);
			window->fltkViewer->_objManipulator.get_active_control()->detach_pawn();
			window->fltkViewer->_objManipulator.removeActiveControl();
		}
	}
#endif
	// remove any existing scene
	if (pawn->scene_p)
	{
		mcu.remove_scene(pawn->scene_p);
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}

	if (otherListener)
	{
		otherListener->OnPawnDelete(name);
	}

	// if this is a camera, update the camera list in the main window
	SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
	if (camera)
	{
		window->updateCameraList();
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
}

void FLTKListener::notify(SmartBody::SBSubject* subject)
{
	SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(subject);
	if (pawn)
	{
		const std::string& pawnName = pawn->getName();
		if (pawn->getName().find("light") == 0)
		{
			// adjust the lights based on the new position and color
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			BaseWindow* window = dynamic_cast<BaseWindow*>(mcu.viewer_p);
			if (window)
			{
#if 1 //!USE_OGRE_VIEWER
				window->fltkViewer->updateLights();
#endif
			}


		}

	}
}

void FLTKListener::setOtherListener( SmartBody::SBCharacterListener* listener )
{
	otherListener = listener;
}
