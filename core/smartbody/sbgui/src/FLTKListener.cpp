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

	attr = pawn->getAttribute("meshScale");
	if (attr)
		attr->registerObserver(this);

	attr = pawn->getAttribute("deformableMesh");
	if (attr)
		attr->registerObserver(this);

	attr = pawn->getAttribute("deformableMeshScale");
	if (attr)
		attr->registerObserver(this);

	attr = pawn->getAttribute("displayType");
	if (attr)
		attr->registerObserver(this);

	FLTKListener::OnCharacterUpdate(name);
	
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
#if 0
	if (pawn->dMesh_p)
	{
		for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
		{
			scene->getRootGroup()->remove( pawn->dMesh_p->dMeshDynamic_p[i] );
		}
		//delete character->dMesh_p; // AS 1/28/13 causing crash related to mesh instances
		pawn->dMesh_p = NULL;
	}
#endif 

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
	FLTKListener::OnCharacterCreate(name, "");
}

void FLTKListener::OnPawnDelete( const std::string & name )
{
	FLTKListener::OnCharacterDelete(name);
}

void FLTKListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{
}

void FLTKListener::OnChannel( const std::string & name, const std::string & channelName, const float value)
{
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
		SmartBody::SBCharacter* character = dynamic_cast<SmartBody::SBCharacter*>(attribute->getObject());
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
#if 0
		if (name == "mesh")
		{
		}
		else 
#endif
		if ( name == "deformableMeshScale" || name == "meshScale")
		{
			//LOG("name = deformableMeshScale");
			bool useDeformableMesh = (name == "deformableMeshScale");
			SmartBody::DoubleAttribute* doubleAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			if (doubleAttribute)
			{
				if (!pawn->dMeshInstance_p && useDeformableMesh)
					pawn->dMeshInstance_p = new SbmDeformableMeshGPUInstance();
				else if (!pawn->dStaticMeshInstance_p && !useDeformableMesh)
					pawn->dStaticMeshInstance_p = new SbmDeformableMeshGPUInstance();
				
				DeformableMeshInstance* meshInstance = useDeformableMesh ? pawn->dMeshInstance_p : pawn->dStaticMeshInstance_p;
				meshInstance->setMeshScale((float) doubleAttribute->getValue());
				//LOG("Set mesh scale = %f",doubleAttribute->getValue());
			}			
		}
		else if (name == "deformableMesh" || name == "mesh")
		{
			bool useDeformableMesh = (name == "deformableMesh");
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			if (strAttribute)
			{
				const std::string& value = strAttribute->getValue();
				// clean up any old meshes

				if (value == "")
					return;

				SmartBody::SBAssetManager* assetManager = scene->getAssetManager();
				DeformableMesh* mesh = assetManager->getDeformableMesh(value);
				if (!mesh)
				{
					int index = value.find(".");
					if (index != std::string::npos)
					{
						std::string prefix = value.substr(0, index);
						const std::vector<std::string>& meshPaths = assetManager->getAssetPaths("mesh");
						for (size_t x = 0; x < meshPaths.size(); x++)
						{
							assetManager->loadAsset(meshPaths[x] + "/" + prefix + "/" + value);
						}
					}
					mesh = assetManager->getDeformableMesh(value);
				}
		
				
				if (mesh)
				{
					if (!pawn->dMeshInstance_p && useDeformableMesh)
					{
						pawn->dMeshInstance_p = new SbmDeformableMeshGPUInstance();
						pawn->dMeshInstance_p->setToStaticMesh(false);
					}
					else if (!pawn->dStaticMeshInstance_p && !useDeformableMesh)
					{
						pawn->dStaticMeshInstance_p = new SbmDeformableMeshGPUInstance();
						pawn->dStaticMeshInstance_p->setToStaticMesh(true);
					}
					else if (!pawn->dStaticMeshInstance_p && name == "mesh")
					{
						pawn->dStaticMeshInstance_p = new SbmDeformableMeshGPUInstance();
						pawn->dStaticMeshInstance_p->setToStaticMesh(true);
					}

					// create attributes for all the blend shapes if there's any
					std::map<std::string, std::vector<SrSnModel*> > ::iterator iter = mesh->blendShapeMap.begin();
					for (; iter != mesh->blendShapeMap.end(); ++iter)
					{
						for (size_t c = 0; c < iter->second.size(); ++c)
						{
							if (strcmp(iter->first.c_str(), (const char*)iter->second[c]->shape().name) == 0)	// you don't create it for base shape
								continue;

							std::stringstream ss;
							ss << "blendShape.channelName." << (const char*)iter->second[c]->shape().name;
							character->createStringAttribute(ss.str().c_str(), "", true, "Blend Shape", c + 1, false, false, false, "blend shape channel name");
							character->createActionAttribute("updateChannel", true, "Blend Shape", 0, false, false, false, "update blend shape channel");
						}
					}

					DeformableMeshInstance* meshInsance = useDeformableMesh ? pawn->dMeshInstance_p : pawn->dStaticMeshInstance_p;
					meshInsance->setDeformableMesh(mesh);
					//meshInsance->setSkeleton(pawn->getSkeleton());	
					meshInsance->setPawn(pawn);
					
#if 0
					for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
					{
						scene->getRootGroup()->add( pawn->dMesh_p->dMeshDynamic_p[i] );
					}
#endif
				}
			}
		}
		else if (name == "displayType")
		{
			SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			if (strAttribute)
			{
				const std::string& value = strAttribute->getValue();
				if (value == "bones")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(1,0,0,0);
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(0);
				}
				else if (value == "visgeo")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(0,1,0,0);
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(0);
				}
				else if (value == "colgeo")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(0,0,1,0);
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(0);
				}
				else if (value == "axis")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(0,0,0,1);
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(0);
				}
				else if (value == "mesh")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(0,0,0,0);
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(1);
 #if !defined(__ANDROID__) && !defined(__FLASHPLAYER__) && !defined(SB_IPHONE)						
					SbmDeformableMeshGPU::useGPUDeformableMesh = false;
#endif          
				}
				else if (value == "GPUmesh")
				{
					if (pawn->scene_p)
						pawn->scene_p->set_visibility(0,0,0,0);
#if !defined(__ANDROID__) && !defined(__FLASHPLAYER__) && !defined(SB_IPHONE)
					SbmDeformableMeshGPU::useGPUDeformableMesh = true;
#endif
					if (pawn->dMeshInstance_p)
						pawn->dMeshInstance_p->setVisibility(1);

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

