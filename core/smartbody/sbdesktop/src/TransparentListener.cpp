#include "TransparentListener.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sb/SBPawn.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSimulationManager.h>
#include "TransparentViewer.h"


TransparentListener::TransparentListener()
{
}

TransparentListener::~TransparentListener()
{
}

void TransparentListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	// remove any existing scene
	if (character->scene_p)
	{
		if( SmartBody::SBScene::getScene()->getRootGroup() )
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove( character->scene_p ); 
		}
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
	if( SmartBody::SBScene::getScene()->getRootGroup() )
	{
		SmartBody::SBScene::getScene()->getRootGroup()->add( character->scene_p ); 
	}


	// remove any existing deformable mesh
	

	#if defined(__ANDROID__) || defined(SBM_IPHONE)
		character->dMesh_p = new DeformableMesh();
	#else		
		character->dMeshInstance_p =  new SbmDeformableMeshGPUInstance();
	#endif
	SmartBody::SBSkeleton* sbSkel = character->getSkeleton();	
	character->dMeshInstance_p->setSkeleton(sbSkel);
}

void TransparentListener::OnCharacterDelete( const std::string & name )
{
	

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;

	// remove any existing scene
	if (character->scene_p)
	{
		if( SmartBody::SBScene::getScene()->getRootGroup() )
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove( character->scene_p ); 
		}
		character->scene_p->unref();
		character->scene_p = NULL;
	}
	// remove any existing deformable mesh		
}

void TransparentListener::OnCharacterUpdate( const std::string & name)
{
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
		return;
	std::string classType = character->getClassType();

	OnCharacterDelete(name);
	OnCharacterCreate(name, classType);
}

void TransparentListener::OnPawnCreate( const std::string & name )
{
	
	SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(name);
	if (!pawn)
		return;

	if (name.find("light") == 0)
	{
		pawn->registerObserver(this);
		TransparentViewer* window = dynamic_cast<TransparentViewer*>(SmartBody::SBScene::getScene()->getViewer());
		if (window)
		{
			//window->updateLights();
		}
	}

	// remove any existing scene
	if (pawn->scene_p)
	{
		if( SmartBody::SBScene::getScene()->getRootGroup() )
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove( pawn->scene_p ); 
		}
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}
	// remove any existing deformable mesh	
	
	if (pawn->dMeshInstance_p)
	{
		delete pawn->dMeshInstance_p;
		pawn->dMeshInstance_p = NULL;
	}

	//pawn->dMesh_p =  new SbmDeformableMeshGPU();
	pawn->dMeshInstance_p = new SbmDeformableMeshGPUInstance();

	pawn->scene_p = new SkScene();
	pawn->scene_p->ref();
	pawn->scene_p->init(pawn->getSkeleton());
	bool visible = pawn->getBoolAttribute("visible");
	if (visible)
		pawn->scene_p->visible(true);
	else
		pawn->scene_p->visible(false);
	if( SmartBody::SBScene::getScene()->getRootGroup() )
	{
		SmartBody::SBScene::getScene()->getRootGroup()->add( pawn->scene_p ); 
	}
}

void TransparentListener::OnPawnDelete( const std::string & name )
{
	
	SbmPawn* pawn =SmartBody::SBScene::getScene()->getPawn(name);
	if (!pawn)
		return;
	pawn->unregisterObserver(this);

	// remove any existing scene
	if (pawn->scene_p)
	{
		if( SmartBody::SBScene::getScene()->getRootGroup() )
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove( pawn->scene_p ); 
		}
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
			
			TransparentViewer* window = dynamic_cast<TransparentViewer*>(SmartBody::SBScene::getScene()->getViewer());
			if (window)
			{
				//window->updateLights();
			}


		}

	}
}

void TransparentListener::OnLogMessage(const std::string& message)
{
	LOG(message.c_str());
}


