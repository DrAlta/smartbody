#include "SBPawn.h"

#include <sbm/mcontrol_callbacks.h>
#include <sb/SBAttribute.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBColObject.h>
#include <sb/SBPhysicsSim.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBScene.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sr/sr_model.h>
#include <sbm/gwiz_math.h>


namespace SmartBody {

SBPawn::SBPawn() : SbmPawn()
{
	setAttributeGroupPriority("transform", 80);
	setAttributeGroupPriority("Display", 100);
	setAttributeGroupPriority("Physics", 300);

	createBoolAttribute("visible", true, true, "Display", 5, false, false, false, "");
	_posX = createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	_posY = createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	_posZ = createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	_rotX = createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	_rotY = createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	_rotZ = createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
	
	createVec3Attribute("meshScale", 1.0, 1.0f, 1.0f, true, "Display", 410, false, false, false, "Scale of geometry/mesh");
	createStringAttribute("mesh", "", true, "Display", 405, false, false, false, "Geometry/mesh");
	// since this is a pawn, show static mesh by default
	createBoolAttribute("showStaticMesh", true, true, "Display", 415, false, false, false, "Whether the object is visible.");
	createBoolAttribute("blendTexturesWithLighting", true, true, "Display", 405, false, false, false, "Whether the object is visible.");
	createVec3Attribute("meshTranslate", 0.0, 0.0, 0.0, true, "Display", 420, false, false, false, "Mesh translation offset");
	createVec3Attribute("meshRotation", 0.0, 0.0, 0.0, true, "Display", 430, false, false, false,  "Mesh rotation offset");
	createActionAttribute("createPhysics", true, "Physics", 300, false, false, false, "Initializes the pawn as a physics object.");
	createBoolAttribute("enablePhysics", false, true, "Physics", 310, false, false, false, "Enables or disables physics for this pawn.");
	std::vector<std::string> shapes;
	shapes.push_back("null");
	shapes.push_back("sphere");
	shapes.push_back("box");
	shapes.push_back("capsule");
	SmartBody::StringAttribute* shapeAttr = createStringAttribute("collisionShape", "null", true, "Physics", 350, false, false, false, "Initializes the pawn as a physics object.");
	shapeAttr->setValidValues(shapes);
	SrVec defaultScale(1.0f, 1.0f, 1.0f);
	createVec3Attribute("collisionShapeScale", defaultScale[0], defaultScale[1], defaultScale[2], true, "Physics", 360, false, false, false, "Scaling of physics-based shape.");
	createBoolAttribute("showCollisionShape", true, true, "Physics", 370, false, false, false, "Whether the collision shape is visible.");
	smoothTargetHPR = false;
	smoothTargetPos = false;
	posStartTime = 0.f;
	posEndTime = 0.f;
	hprStartTime = 0.f;
	hprEndTime = 0.f;	
}

SBPawn::SBPawn(const char* name) : SbmPawn(name)
{
	setAttributeGroupPriority("transform", 80);
	setAttributeGroupPriority("Display", 100);
	setAttributeGroupPriority("Physics", 300);
	
	createBoolAttribute("visible", true, true, "Display", 5, false, false, false, "Whether the object is visible.");
	createVec3Attribute("color", 1.0, 0.0, 0.0, true, "Display", 6, false, false, false, "Object color.");
	_posX = createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	_posY = createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	_posZ = createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	_rotX = createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	_rotY = createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	_rotZ = createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
	createStringAttribute("mesh", "", true, "Display", 400, false, false, false, "Geometry/mesh");
	// since this is a pawn, show static mesh by default
	createBoolAttribute("showStaticMesh", true, true, "Display", 405, false, false, false, "Whether the object is visible.");
	createBoolAttribute("blendTexturesWithLighting", true, true, "Display", 405, false, false, false, "Whether the object is visible.");

	createVec3Attribute("meshScale", 1.0, 1.0f, 1.0f, true, "Display", 410, false, false, false, "Scale of geometry/mesh");
	createVec3Attribute("meshTranslation", 0.0, 0.0, 0.0, true, "Display", 420, false, false, false, "Mesh translation offset");
	createVec3Attribute("meshRotation", 0.0, 0.0, 0.0, true, "Display", 430, false, false, false,  "Mesh rotation offset");
	createVec3Attribute("meshPivot", 0.0, 0.0, 0.0, true, "Display", 440, false, false, false,  "Mesh pivot offset");
	createActionAttribute("createPhysics", true, "Physics", 300, false, false, false, "Initializes the pawn as a physics object.");
	createBoolAttribute("enablePhysics", false, true, "Physics", 310, false, false, false, "Enables or disables physics for this pawn.");
	std::vector<std::string> shapes;
	shapes.push_back("null");
	shapes.push_back("sphere");
	shapes.push_back("box");
	shapes.push_back("capsule");
	SmartBody::StringAttribute* shapeAttr = createStringAttribute("collisionShape", "null", true, "Physics", 350, false, false, false, "Initializes the pawn as a physics object.");
	shapeAttr->setValidValues(shapes);
	SrVec defaultScale(1.0f, 1.0f, 1.0f);
	createVec3Attribute("collisionShapeScale", defaultScale[0], defaultScale[1], defaultScale[2], true, "Physics", 360, false, false, false, "Scaling of physics-based shape.");
	createBoolAttribute("showCollisionShape", true, true, "Physics", 370, false, false, false, "Whether the collision shape is visible.");
	smoothTargetHPR = false;
	smoothTargetPos = false;
	posStartTime = 0.f;
	posEndTime = 0.f;
	hprStartTime = 0.f;
	hprEndTime = 0.f;
}

SBPawn::~SBPawn()
{
}

void SBPawn::setName(const std::string& name)
{
	std::string oldName = getName();

	SBObject::setName(name);
	SmartBody::SBScene::getScene()->updatePawnNames();
}

SBSkeleton* SBPawn::getSkeleton()
{
	SkSkeleton* skskel = SbmPawn::getSkeleton();
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}

void SBPawn::setSkeleton(SBSkeleton* skel)
{
	if (!skel)
	{
		LOG("Skeleton is nonexistent. Skeleton not set on pawn %s", this->getName().c_str());
		return;
	}

	SrVec position;
	SrVec hpr;	
	SBSkeleton* sk = this->getSkeleton();
	if (sk)
	{		
		position = this->getPosition();
		hpr = this->getHPR();
	}	

	SbmPawn::setSkeleton(skel);	
	setup();
	std::string pawnName = this->getName();		
	skel->setPawnName(pawnName);

	if (sk)
	{		
		this->setPosition(position);
		this->setHPR(hpr);			
	}
}

SrVec SBPawn::getPosition()
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	return SrVec(x, y, z);
}

SrQuat SBPawn::getOrientation()
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);

	gwiz::quat_t q = gwiz::euler_t(p, h, r);
	SrQuat quat(float(q.w()), float(q.x()), float(q.y()), float(q.z()));
	return quat;
}

void SBPawn::setPosition(const SrVec& pos)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	set_world_offset(pos.x, pos.y, pos.z, h, p, r);	
}

void SBPawn::setOrientation(const SrQuat& quat)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	gwiz::euler_t euler = gwiz::euler_t(gwiz::quat_t(quat.w, quat.x, quat.y, quat.z));
	set_world_offset(x, y, z, float(euler.h()), float(euler.p()), float(euler.r()));
}

void SBPawn::setHPR(const SrVec& hpr)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	set_world_offset(x, y, z, hpr[0], hpr[1], hpr[2]);
}

SrVec SBPawn::getHPR()
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);

	SrVec hpr(h, p, r);
	return hpr;
}

void SBPawn::setPositionSmooth( const SrVec& pos, float smoothTime )
{
	initialPos = getPosition(); // start position
	targetPos  = pos;
	posStartTime = (float) SmartBody::SBScene::getScene()->getSimulationManager()->getTime();
	posEndTime = posStartTime + smoothTime;
	smoothTargetPos = true;
}

void SBPawn::setHPRSmooth( const SrVec& hpr, float smoothTime )
{
	initialHPR = getHPR(); // start position
	targetHPR  = hpr;
	hprStartTime = (float) SmartBody::SBScene::getScene()->getSimulationManager()->getTime();
	hprEndTime = hprStartTime + smoothTime;
	smoothTargetHPR = true;
}

void SBPawn::afterUpdate(double time)
{	
	float x, y, z, h, p, r;	
	if (smoothTargetPos)
	{
		if (time >= posStartTime && time <= posEndTime && posStartTime != posEndTime)
		{
			float weight = (float)(time-posStartTime)/(float)(posEndTime - posStartTime);
			SrVec newPos = initialPos*(1.f-weight) + targetPos*(weight);
			setPosition(newPos);
		}
		else
		{
			smoothTargetPos = false;
		}
	}

	if (smoothTargetHPR)
	{
		if (time >= hprStartTime && time <= hprEndTime && hprStartTime != hprEndTime)
		{
			float weight = (float)(time-hprStartTime)/(float)(hprEndTime - hprStartTime);
			gwiz::quat_t q1 = gwiz::euler_t(initialHPR.y, initialHPR.x, initialHPR.z);
			gwiz::quat_t q2 = gwiz::euler_t(targetHPR.y, targetHPR.x, targetHPR.z);
			gwiz::quat_t qNew; qNew = qNew.lerp(weight,q1,q2);			
			//SrVec newHPR = initialHPR*(1.f-weight) + targetHPR*(weight);
			gwiz::euler_t euler = gwiz::euler_t(gwiz::quat_t(qNew.w(), qNew.x(), qNew.y(), qNew.z()));
			SrVec newHPR = SrVec(float(euler.h()), float(euler.p()), float(euler.r()));
			setHPR(newHPR);
		}
		else
		{
			smoothTargetHPR = false;
		}
	}
	get_world_offset(x, y, z, h, p, r);	
/*	
	setDoubleAttribute("posX",x);
	setDoubleAttribute("posY",y);
	setDoubleAttribute("posZ",z);

	setDoubleAttribute("rotX",p);
	setDoubleAttribute("rotY",h);
	setDoubleAttribute("rotZ",r);
	*/
 	_posX->setValueFast(x);
 	_posY->setValueFast(y);
 	_posZ->setValueFast(z);
 	_rotX->setValueFast(p);
 	_rotY->setValueFast(h);
 	_rotZ->setValueFast(r);
}

void SBPawn::notify(SBSubject* subject)
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "posX")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec position = this->getPosition();
			position.x = (float) val;
			this->setPosition(position);
		}
		else if (attribute->getName() == "posY")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec position = this->getPosition();
			position.y = (float) val;
			this->setPosition(position);
		}
		else if (attribute->getName() == "posZ")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec position = this->getPosition();
			position.z = (float) val;
			this->setPosition(position);
		}
		if (attribute->getName() == "rotX")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec hpr = this->getHPR();
			hpr.y = (float) val;
			this->setHPR(hpr);
		}
		else if (attribute->getName() == "rotY")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec hpr = this->getHPR();
			hpr.x = (float) val;
			this->setHPR(hpr);
		}
		else if (attribute->getName() == "rotZ")
		{
			double val = this->getDoubleAttribute(attribute->getName());
			SrVec hpr = this->getHPR();
			hpr.z = (float) val;
			this->setHPR(hpr);
		}
		else if (attribute->getName() == "collisionShape")
		{
			SBGeomObject* geomObject = getGeomObject();
			std::string shapeName = getStringAttribute("collisionShape");
//			LOG("collisionShape = %s",shapeName.c_str());
			if (shapeName != geomObject->geomType())
			{
				SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollisionManager();
				SrVec size = getVec3Attribute("collisionShapeScale");
				SBGeomObject* obj = colManager->createCollisionObject(collisionObjName,shapeName,size);
				if (obj) 
				{
					obj->attachToObj(this);				
					SmartBody::SBAttribute* attr = this->getAttribute("color");
					if (attr)
					{
						SmartBody::Vec3Attribute* colorAttribute = dynamic_cast<SmartBody::Vec3Attribute*>(attr);
						if (colorAttribute)
						{
							const SrVec& color = colorAttribute->getValue();
							obj->color.set(color.x, color.y, color.z, 1.0f);
						}
					}
				}
				
			}
			else
			{
				// do nothing, already is the shape
			}
		}
		else if (attribute->getName() == "collisionShapeScale")
		{
			SrVec scale = getVec3Attribute("collisionShapeScale");
			SBGeomObject* object = getGeomObject();
			object->setGeomSize(scale);
		}
		else if (attribute->getName() == "enablePhysics")
		{
			SmartBody::BoolAttribute* physicsAttr = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			if (getPhysicsObject())
				getPhysicsObject()->enablePhysicsSim(physicsAttr->getValue());
		}
		else if (attribute->getName() == "createPhysics")
		{
			SmartBody::SBPhysicsManager* manager = SmartBody::SBScene::getScene()->getPhysicsManager();
			manager->createPhysicsPawn(this->getName(), this->getStringAttribute("collisionShape"), this->getVec3Attribute("collisionShapeScale"));
			SmartBody::BoolAttribute* physicsAttr = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			//setPhysicsSim(physicsAttr->getValue());
		}
		else if (attribute->getName() == "mesh")
		{
		}
#if 0
		else if (attribute->getName() == "meshScale")
		{
			SmartBody::DoubleAttribute* meshAttr = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			if (this->dMesh_p)
			{
				for (size_t x = 0; x < this->dMesh_p->dMeshStatic_p.size(); x++)
				{
					SrSnModel* srSnmodel = this->dMesh_p->dMeshStatic_p[x];
					SrModel& model = srSnmodel->shape();
					if (model.VOrig.size() == 0)
						model.saveOriginalVertices();
					else
						model.restoreOriginalVertices();
					model.scale((float) meshAttr->getValue());
				}
			}
		}
		else if (attribute->getName() == "meshTranslation")
		{
			SmartBody::Vec3Attribute* meshAttr = dynamic_cast<SmartBody::Vec3Attribute*>(attribute);
			if (this->dMesh_p)
			{
				for (size_t x = 0; x < this->dMesh_p->dMeshStatic_p.size(); x++)
				{
					SrSnModel* srSnmodel = this->dMesh_p->dMeshStatic_p[x];
					SrModel& model = srSnmodel->shape();
					model.translate(meshAttr->getValue());
				}
			}
		}
		else if (attribute->getName() == "meshRotation")
		{
			SmartBody::Vec3Attribute* meshAttr = dynamic_cast<SmartBody::Vec3Attribute*>(attribute);
			if (this->dMesh_p)
			{
				SrVec r(meshAttr->getValue());
				r *= 3.14159f / 180.0f;
				LOG("Rotating by %f %f %f", r[0], r[1], r[2]);
				for (size_t x = 0; x < this->dMesh_p->dMeshStatic_p.size(); x++)
				{
					SrSnModel* srSnmodel = this->dMesh_p->dMeshStatic_p[x];
					SrModel& model = srSnmodel->shape();				
					model.rotate(r);
				}
			}
		}
#endif
		else if (attribute->getName() == "visible")
		{
			/*
			if (!this->scene_p)
				return;
			SmartBody::BoolAttribute* visibleAttr = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
			if (visibleAttr->getValue())
				this->scene_p->visible(true);
			else
				this->scene_p->visible(false);
				*/
		}
		else if (attribute->getName() == "color")
		{
			if (this->getGeomObject())
			{
				SmartBody::Vec3Attribute* colorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(attribute);
				const SrVec& c = colorAttr->getValue();
				this->getGeomObject()->color.set(c.x, c.y, c.z); 
			}
		}
	}
}

SBPhysicsObj* SBPawn::getPhysicsObject()
{
#ifndef SB_NO_ODE_PHYSICS
	SBPhysicsSim* phyEngine = SBPhysicsSim::getPhysicsEngine();
	return phyEngine->getPhysicsPawn(getName());	
#else
	return NULL;
#endif
}

void SBPawn::copy( SBPawn* orignalPawn )
{
	SbmPawn::copy(orignalPawn);
	SBObject::copyAllAttributes(orignalPawn);		
}

DeformableMeshInstance* SBPawn::getActiveMesh()
{
	bool showStaticMesh = getBoolAttribute("showStaticMesh");
	if (showStaticMesh)
	{
		return dStaticMeshInstance_p;
	}
	else
	{
		return dMeshInstance_p;
	}
}

void SBPawn::createMeshFromCollisionSurface(std::string name, SrVec color)
{
	SBGeomObject* geomObject = getGeomObject();

	SBGeomBox* boxObject = dynamic_cast<SBGeomBox*>(geomObject);
	if (!boxObject)
		return;

	SrVec boxSize = boxObject->getGeomSize();
#if !defined (__ANDROID__) && !defined(SB_IPHONE) &&  !defined(__FLASHPLAYER__) && !defined(__native_client__)
	SbmDeformableMeshGPU* mesh = new SbmDeformableMeshGPU();
#else
	DeformableMesh* mesh = new DeformableMesh();
#endif	
	mesh->setName(name);

	SrModel* model = new SrModel();

	model->V.push_back(SrPnt(  1.0f * boxSize[0], -1.0f * boxSize[1], -1.0f * boxSize[2]  ));  // 0
	model->V.push_back(SrPnt(  1.0f * boxSize[0],  1.0f * boxSize[1], -1.0f * boxSize[2]  ));  // 1
	model->V.push_back(SrPnt(  1.0f * boxSize[0],  1.0f * boxSize[1],  1.0f * boxSize[2]  ));  // 2
	model->V.push_back(SrPnt(  1.0f * boxSize[0], -1.0f * boxSize[1],  1.0f * boxSize[2]  ));  // 3

	model->V.push_back(SrPnt( -1.0f * boxSize[0], -1.0f * boxSize[1],  1.0f * boxSize[2] ));  // 4
	model->V.push_back(SrPnt( -1.0f * boxSize[0],  1.0f * boxSize[1],  1.0f * boxSize[2]  ));  // 5
	model->V.push_back(SrPnt( -1.0f * boxSize[0],  1.0f * boxSize[1], -1.0f * boxSize[2]  ));  // 6
	model->V.push_back(SrPnt( -1.0f * boxSize[0], -1.0f * boxSize[1], -1.0f * boxSize[2] ));  // 7

	
	// CW
	model->F.push_back(SrModel::Face( 7, 1, 0));
	model->F.push_back(SrModel::Face(6, 1, 7));
	model->F.push_back(SrModel::Face(1, 2, 3));
	model->F.push_back(SrModel::Face(0, 1, 3));
	model->F.push_back(SrModel::Face(6, 2, 1));
	model->F.push_back(SrModel::Face(5, 2, 6));
	model->F.push_back(SrModel::Face(7, 5, 6));
	model->F.push_back(SrModel::Face(5, 7, 4));
	model->F.push_back(SrModel::Face(2, 4, 3));
	model->F.push_back(SrModel::Face(2, 5, 4));
	model->F.push_back(SrModel::Face(7, 0, 3));
	model->F.push_back(SrModel::Face(4, 7, 3));

	// edges
	SrArray<int> edges;
	edges.push(0);
	edges.push(1);
	edges.push(6);
	edges.push(7);
	edges.push(3);
	edges.push(2);
	edges.push(4);
	edges.push(5);
	edges.push(6);
	edges.push(5);
	edges.push(5);
	edges.push(2);
	edges.push(2);
	edges.push(1);
	edges.push(1);
	edges.push(6);
	edges.push(7);
	edges.push(0);
	edges.push(0);
	edges.push(3);
	edges.push(3);
	edges.push(4);
	edges.push(4);
	edges.push(7);
	model->make_edges(edges);


	int numFaces = model->F.size();
	model->Fm.resize(numFaces);

	model->computeNormals(); 

	model->M.size(1);
	model->M[0] = SrMaterial();
	model->M[0].diffuse.set(color[0], color[1], color[2], 1.0f);
	model->mtlnames.push("unknown");
	model->set_one_material(model->M[0]);
	
	SrSnModel* srSnModelStatic = new SrSnModel();
	srSnModelStatic->shape(*model);
	if (model->name.len() > 0)
		srSnModelStatic->shape().name = model->name;
	mesh->dMeshStatic_p.push_back(srSnModelStatic);
	srSnModelStatic->ref();

	SrSnModel* srSnModelDynamic = new SrSnModel();
	srSnModelDynamic->shape(*model);
	srSnModelDynamic->changed(true);
	srSnModelDynamic->visible(false);
	srSnModelDynamic->shape().name = model->name;
	mesh->dMeshDynamic_p.push_back(srSnModelDynamic);
	srSnModelDynamic->ref();



	SmartBody::SBScene::getScene()->getAssetManager()->addDeformableMesh(name, mesh);
	
}


};

