//
// Created by feng on 10/14/2015.
//

#include "AppListener.h"
#include <sb/SBVHMsgManager.h>
#include <sb/SBPython.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAttribute.h>
#include <sb/SBAssetManager.h>


AppListener::AppListener()
{
}

AppListener::~AppListener()
{
}

void AppListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
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

    attr = pawn->getAttribute("deformableMeshScale");
    if (attr)
        attr->registerObserver(this);

    attr = pawn->getAttribute("displayType");
    if (attr)
        attr->registerObserver(this);

    OnCharacterUpdate(name);
}

void AppListener::OnCharacterDelete( const std::string & name )
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

}

void AppListener::OnCharacterUpdate( const std::string & name)
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

void AppListener::OnPawnCreate( const std::string & name )
{
    OnCharacterCreate(name, "");
}

void AppListener::OnPawnDelete( const std::string & name )
{
    OnCharacterDelete(name);
}

void AppListener::notify(SmartBody::SBSubject* subject)
{
    SmartBody::SBScene* scene =	SmartBody::SBScene::getScene();
    SmartBody::SBPawn* pawn = dynamic_cast<SmartBody::SBPawn*>(subject);

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
#if 0
        if (name == "mesh")
        {
        }
        else
#endif
        if ( name == "deformableMeshScale")
        {
            //LOG("name = deformableMeshScale");
            //SmartBody::Vec3Attribute* vec3Attribute = dynamic_cast<SmartBody::Vec3Attribute*>(attribute);
            SmartBody::DoubleAttribute* doubleAttribute = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
            //if (vec3Attribute)
            if (doubleAttribute)
            {
                if (!pawn->dMeshInstance_p)
                    pawn->dMeshInstance_p = new DeformableMeshInstance();
                double val = doubleAttribute->getValue();
                pawn->dMeshInstance_p->setMeshScale(SrVec(val,val,val));
                //pawn->dMeshInstance_p->setMeshScale(vec3Attribute->getValue());
                //LOG("Set mesh scale = %f",doubleAttribute->getValue());
            }
        }
        else if (name == "deformableMesh" || name == "mesh")
        {
            SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
            if (strAttribute)
            {
                const std::string& value = strAttribute->getValue();
                // clean up any old meshes
#if 0
                if (pawn->dMesh_p)
                {
                    for (size_t i = 0; i < pawn->dMesh_p->dMeshDynamic_p.size(); i++)
                    {
                        scene->getRootGroup()->remove( pawn->dMesh_p->dMeshDynamic_p[i] );
                    }
                }
#endif
                if (pawn->dMeshInstance_p)
                {
                    //delete pawn->dMeshInstance_p;
                }
                if (value == "")
                    return;

                SmartBody::SBAssetManager* assetManager = scene->getAssetManager();
                DeformableMesh* mesh = assetManager->getDeformableMesh(value);
                if (!mesh)
                {
                    LOG("Can't find mesh '%s'", value.c_str());
                    int index = value.find(".");
                    if (index != std::string::npos)
                    {
                        std::string prefix = value.substr(0, index);
                        const std::vector<std::string>& meshPaths = assetManager->getAssetPaths("mesh");
                        for (size_t x = 0; x < meshPaths.size(); x++)
                        {
                            std::string assetName = meshPaths[x] + "/" + prefix + "/" + value;
                            LOG("Try to load mesh from '%s'", assetName.c_str());
                            assetManager->loadAsset(assetName);
                        }
                    }
                    mesh = assetManager->getDeformableMesh(value);
                }


                if (mesh)
                {
                    if (!pawn->dMeshInstance_p)
                        pawn->dMeshInstance_p = new DeformableMeshInstance();
                    pawn->dMeshInstance_p->setDeformableMesh(mesh);
                    pawn->dMeshInstance_p->setPawn(pawn);
                    if (name == "mesh") // setting static mesh
                    {
                        pawn->dMeshInstance_p->setToStaticMesh(true);
                    }
                    LOG("mesh '%s', vertex count = %d", value.c_str(),  mesh->posBuf.size());

                    // add blendshapes
                    {
                        // if there are no blendshapes, but there are blendShape.channelName attributes,
                        // then add the morph targets
                        std::vector<SmartBody::StringAttribute*> shapeAttributes;
                        std::map<std::string, SmartBody::SBAttribute*>& attributes = pawn->getAttributeList();
                        for (std::map<std::string, SmartBody::SBAttribute*>::iterator iter = attributes.begin();
                             iter != attributes.end();
                             iter++)
                        {
                            SmartBody::SBAttribute* attribute = (*iter).second;
                            const std::string& attrName = attribute->getName();
                            size_t pos = attrName.find("blendShape.channelName.");
                            if (pos != std::string::npos)
                            {
                                SmartBody::StringAttribute* strAttribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
                                shapeAttributes.push_back(strAttribute);
                            }
                        }

                        int numShapeAttributes = shapeAttributes.size();
                        if (numShapeAttributes > 0)
                        {
                            // make space for all the attributes
                            mesh->morphTargets.insert(std::pair<std::string, std::vector<std::string> >("face", std::vector<std::string>() ));
                            std::map<std::string, std::vector<std::string> >::iterator shapeIter = mesh->morphTargets.begin();
                            (*shapeIter).second.resize(numShapeAttributes);


                            bool hasNeutral = false;
                            for (std::vector<SmartBody::StringAttribute*>::iterator iter = shapeAttributes.begin();
                                 iter != shapeAttributes.end();
                                 iter++)
                            {
                                const std::string& attrName = (*iter)->getName();
                                // get the shape name and value
                                std::string shapeName = attrName.substr(23);

                                std::string shapeChannel = (*iter)->getValue();
                                if (shapeChannel == "Neutral")
                                {
                                    DeformableMesh* neutralMesh = SmartBody::SBScene::getScene()->getAssetManager()->getDeformableMesh(shapeName);
                                    mesh->blendShapeMap.insert(std::pair<std::string, std::vector<SrSnModel*> >(neutralMesh->getName(), std::vector<SrSnModel*>() ));
                                    std::map<std::string, std::vector<SrSnModel*> >::iterator blendshapeIter = mesh->blendShapeMap.begin();
                                    (*blendshapeIter).second.resize(numShapeAttributes);
                                    SrSnModel* staticModel = neutralMesh->dMeshStatic_p[0];
                                    SrSnModel* model = new SrSnModel();
                                    model->shape(staticModel->shape());
                                    model->shape().name = staticModel->shape().name;
                                    model->changed(true);
                                    model->visible(false);
                                    (*blendshapeIter).second[0] = model;
                                    model->ref();
                                    hasNeutral = true;
                                }

                            }

                            std::map<std::string, std::vector<SrSnModel*> >::iterator blendshapeIter = mesh->blendShapeMap.begin();
                            if (blendshapeIter !=  mesh->blendShapeMap.end())
                            {
                                (*blendshapeIter).second.resize(numShapeAttributes);

                                int count = 1;
                                if (hasNeutral)
                                {
                                    for (std::vector<SmartBody::StringAttribute*>::iterator iter = shapeAttributes.begin();
                                         iter != shapeAttributes.end();
                                         iter++)
                                    {
                                        const std::string& attrName = (*iter)->getName();
                                        // get the shape name and value
                                        std::string shapeName = attrName.substr(23);
                                        std::string shapeChannel = (*iter)->getValue();
                                        if (shapeChannel == "Neutral")
                                            continue;
                                        if (blendshapeIter !=  mesh->blendShapeMap.end())
                                            (*shapeIter).second[count] = shapeName;
                                        DeformableMesh* shapeModel = SmartBody::SBScene::getScene()->getAssetManager()->getDeformableMesh(shapeName);
                                        if (shapeModel)
                                        {
                                            (*blendshapeIter).second[count] = shapeModel->dMeshStatic_p[0];
                                            shapeModel->dMeshStatic_p[0]->ref();
                                        }
                                        else
                                        {
                                            (*blendshapeIter).second[count] = NULL;
                                        }
                                        count++;
                                    }
                                }
                            }
                        }
                    }
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

void AppListener::OnSimulationUpdate()
{
    //return;
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

    const std::vector<std::string>& pawns = scene->getPawnNames();
    for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
         pawnIter != pawns.end();
         pawnIter++)
    {
        SmartBody::SBPawn* pawn = scene->getPawn((*pawnIter));
        if (pawn->scene_p)
        {
            pawn->scene_p->update();
            if (pawn->dMeshInstance_p)
            {
                pawn->dMeshInstance_p->blendShapeStaticMesh();
                pawn->dMeshInstance_p->update();
            }
        }
    }

}
