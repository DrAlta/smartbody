#include "vhcl.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include "FLTKListener.h"
#include <sb/SBPawn.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBWSPManager.h>
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
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBPawn* pawn = scene->getPawn(name);
	if (!pawn)
		return;

	// add attribute observations
	SmartBody::SBAttribute* attr = pawn->getAttribute("mesh");
	if (attr)
		attr->registerObserver(this);
	attr = pawn->getAttribute("deformableMesh");
	if (attr)
		attr->registerObserver(this);

	OnCharacterUpdate(name);
	
	if (otherListener)
		otherListener->OnCharacterCreate(name,objectClass);

	if (name.find("light") == 0)
	{
		pawn->registerObserver(this);
		BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
		if (window)
		{
			window->fltkViewer->updateLights();
		}
	
	}
	
	// if this is a camera, update the camera list in the main window
	SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
	if (camera)
	{
		BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
		if (window)
		{
			window->updateCameraList();
			window->redraw();
		}
	}
}

void FLTKListener::OnCharacterDelete( const std::string & name )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(name);
	if (!pawn)
		return;

	// remove any existing scene
	if (pawn->scene_p)
	{
		if( scene->getRootGroup() )
		{
			scene->getRootGroup()->remove( pawn->scene_p ); 
		}
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}
	// remove any existing deformable mesh
	if (pawn->dMesh_p)
	{
		for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
		{
			scene->getRootGroup()->remove( pawn->dMesh_p->dMeshDynamic_p[i] );
		}
		//delete character->dMesh_p; // AS 1/28/13 causing crash related to mesh instances
		pawn->dMesh_p = NULL;
	}
#if 1 //!USE_OGRE_VIEWER
	// make sure the character isn't associated with the viewer

	
	BaseWindow* window = dynamic_cast<BaseWindow*>(scene->getViewer());
	if (window)
	{
		if (window->fltkViewer->_objManipulator.get_selected_pawn() == pawn)
		{
			window->fltkViewer->_objManipulator.set_selected_pawn(NULL);
			window->fltkViewer->_objManipulator.get_active_control()->detach_pawn();
			window->fltkViewer->_objManipulator.removeActiveControl();
		}
	}

	// if this is a camera, update the camera list in the main window
	SrCamera* camera = dynamic_cast<SrCamera*>(pawn);
	if (camera)
	{
		if (window)
			window->updateCameraList();
	}
#endif

	if (window)
	{
		if (window->fltkViewer->_objManipulator.get_selected_pawn() == pawn)
		{
			window->fltkViewer->_objManipulator.set_selected_pawn(NULL);
			window->fltkViewer->_objManipulator.get_active_control()->detach_pawn();
			window->fltkViewer->_objManipulator.removeActiveControl();
		}
	}

	if (otherListener)
		otherListener->OnCharacterDelete(name);
}

void FLTKListener::OnCharacterUpdate( const std::string & name)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(name);
	if (!pawn)
		return;
	
	// remove any existing scene
	if (pawn->scene_p)
	{
		if( scene->getRootGroup() )
		{
			scene->getRootGroup()->remove( pawn->scene_p ); 
		}
		pawn->scene_p->unref();
		pawn->scene_p = NULL;
	}

	pawn->scene_p = new SkScene();
	pawn->scene_p->ref();
	pawn->scene_p->init(pawn->getSkeleton());
	bool visible = pawn->getBoolAttribute("visible");
	if (visible)
		pawn->scene_p->visible(true);
	else
		pawn->scene_p->visible(false);


	if( scene->getRootGroup() )
	{
		scene->getRootGroup()->add( pawn->scene_p ); 
	}
}

void FLTKListener::OnPawnCreate( const std::string & name )
{
	OnCharacterCreate(name, "");
}

void FLTKListener::OnPawnDelete( const std::string & name )
{
	OnCharacterDelete(name);
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

void FLTKListener::OnObjectSelected(const std::string& objectName)
{
	SmartBody::SBScene* scene =	SmartBody::SBScene::getScene();
	BaseWindow* window = dynamic_cast<BaseWindow*>(scene->getViewer());
	if (window->resourceWindow)
	{
		window->resourceWindow->selectPawn(objectName);
	}

	if (window->fltkViewer)
	{
		PawnControl* tempControl = window->fltkViewer->_objManipulator.getPawnControl(window->fltkViewer->_transformMode);
		window->fltkViewer->_objManipulator.active_control = tempControl;
		window->fltkViewer->_objManipulator.set_selected_pawn(scene->getPawn(objectName));
	}
}

void FLTKListener::notify(SmartBody::SBSubject* subject)
{
	SmartBody::SBScene* scene =	SmartBody::SBScene::getScene();
	SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(subject);
	if (pawn)
	{
		const std::string& pawnName = pawn->getName();
		if (pawn->getName().find("light") == 0)
		{
			// adjust the lights based on the new position and color
			BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
			if (window)
			{
#if 1 //!USE_OGRE_VIEWER
				window->fltkViewer->updateLights();
#endif
			}


		}

	}
	
	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (attribute)
	{
		SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(attribute->getObject());
		const std::string& name = attribute->getName();
		if (name == "visible")
		{
			SmartBody::BoolAttribute* boolAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			if (boolAttribute)
			{
				if (!pawn->scene_p)
					return;
				if (boolAttribute->getValue())
					pawn->scene_p->visible(true);
				else
					pawn->scene_p->visible(false);
			}
		}
		if (name == "mesh")
		{
		}
		else if (name == "deformableMesh")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			if (strAttribute)
			{
				const std::string& value = strAttribute->getValue();
				// clean up any old meshes
				if (pawn->dMesh_p)
				{
					for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
					{
						scene->getRootGroup()->remove( pawn->dMesh_p->dMeshDynamic_p[i] );
					}
				}
				if (pawn->dMeshInstance_p)
				{
					//delete pawn->dMeshInstance_p;
				}

				SmartBody::SBAssetManager* assetManager = scene->getAssetManager();
				DeformableMesh* mesh = assetManager->getDeformableMesh(value);
				if (!mesh)
				{
					// load the assets from the mesh directories
					std::vector<std::string> meshPaths = assetManager->getAssetPaths("mesh");
					for (size_t m = 0; m < meshPaths.size(); m++)
					{
						assetManager->loadAssetsFromPath(meshPaths[m] + "/" + value);
					}
				}
		
				mesh = assetManager->getDeformableMesh(value);
				if (mesh)
				{
					pawn->dMesh_p = mesh;
					if (!pawn->dMeshInstance_p)
						pawn->dMeshInstance_p = new DeformableMeshInstance();
					pawn->dMeshInstance_p->setDeformableMesh(mesh);
					pawn->dMeshInstance_p->setSkeleton(pawn->getSkeleton());
					
					for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
					{
						scene->getRootGroup()->add( pawn->dMesh_p->dMeshDynamic_p[i] );
					}
				}
			}
		}
		
	}
}

void FLTKListener::setOtherListener( SmartBody::SBSceneListener* listener )
{
	otherListener = listener;
}

void FLTKListener::OnSimulationStart()
{
}

void FLTKListener::OnSimulationEnd()
{
}

void FLTKListener::OnSimulationUpdate()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	const std::vector<std::string>& pawns = scene->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SmartBody::SBPawn* pawn = scene->getPawn((*pawnIter));
 		if (pawn->scene_p)
 			pawn->scene_p->update();	
	}

	scene->updateTrackedCameras();
		
	BaseWindow* rootWindow = dynamic_cast<BaseWindow*>(scene->getViewer());

	if(rootWindow && rootWindow->dataViewerWindow && rootWindow->dataViewerWindow->shown())
	{
		rootWindow->dataViewerWindow->update();
	}

	if (rootWindow && rootWindow->panimationWindow && rootWindow->panimationWindow->shown())
	{
			rootWindow->panimationWindow->update_viewer();
	}
	if (rootWindow && rootWindow->visemeViewerWindow && rootWindow->visemeViewerWindow->shown())
	{
		rootWindow->visemeViewerWindow->update();
	}

	if (scene->getViewer())
		scene->getViewer()->render();
	if (scene->getOgreViewer())
		scene->getOgreViewer()->render();
}

void FLTKListener::OnLogMessage( const std::string& message )
{
#ifdef WIN32
	LOG(message.c_str());
#endif
}

