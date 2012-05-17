#include "SBPawn.h"
#include <sbm/mcontrol_util.h>
#include <sbm/mcontrol_callbacks.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBCollisionManager.h>
#include <sbm/Physics/SbmColObject.h>


namespace SmartBody {

SBPawn::SBPawn() : SbmPawn()
{
	_posX = createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	_posY = createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	_posZ = createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	_rotX = createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	_rotY = createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	_rotZ = createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
	createStringAttribute("mesh", "", true, "Basic", 400, false, false, false, "Geometry/mesh");
	createDoubleAttribute("meshScale", 1.0, true, "Basic", 410, false, false, false, "Scale of geometry/mesh");
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
}

SBPawn::SBPawn(const char* name) : SbmPawn(name)
{
	_posX = createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	_posY = createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	_posZ = createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	_rotX = createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	_rotY = createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	_rotZ = createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
	createStringAttribute("mesh", "", true, "Basic", 400, false, false, false, "Geometry/mesh");
	createDoubleAttribute("meshScale", 1.0, true, "Basic", 410, false, false, false, "Scale of geometry/mesh");
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
}

SBPawn::~SBPawn()
{
}


void SBPawn::addMesh(std::string mesh)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu_load_mesh( getName().c_str(), mesh.c_str(), &mcu );
}

SBSkeleton* SBPawn::getSkeleton()
{
	SkSkeleton* skskel = SbmPawn::getSkeleton();
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}

void SBPawn::setSkeleton(SBSkeleton* skel)
{
	SbmPawn::setSkeleton(skel);
	setup();
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

void SBPawn::setPosition(SrVec pos)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	set_world_offset(pos.x, pos.y, pos.z, h, p, r);	
}

void SBPawn::setOrientation(SrQuat quat)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
	gwiz::euler_t euler = gwiz::euler_t(gwiz::quat_t(quat.w, quat.x, quat.y, quat.z));
	set_world_offset(x, y, z, float(euler.h()), float(euler.p()), float(euler.r()));
}

void SBPawn::setHPR(SrVec hpr)
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

void SBPawn::afterUpdate(double time)
{
	float x, y, z, h, p, r;
	get_world_offset(x, y, z, h, p, r);
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
			SbmGeomObject* object = getGeomObject();
			std::string shapeName = getStringAttribute("collisionShape");
			LOG("collisionShape = %s",shapeName.c_str());
			if (shapeName != object->geomType())
			{
				SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollisionManager();
				SrVec size = getVec3Attribute("collisionShapeScale");
				SbmGeomObject* obj = colManager->createCollisionObject(collisionObjName,shapeName,size);
				if (obj) obj->attachToObj(this);
			}
			else
			{
				// do nothing, already is the shape
			}
		}
		else if (attribute->getName() == "collisionShapeScale")
		{
			SrVec scale = getVec3Attribute("collisionShapeScale");
			SbmGeomObject* object = getGeomObject();
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
			SmartBody::StringAttribute* meshAttr = dynamic_cast<SmartBody::StringAttribute*>(attribute);
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			mcu_load_mesh(getName().c_str(), meshAttr->getValue().c_str(), &mcu, "");
		}
		else if (attribute->getName() == "meshScale")
		{
			SmartBody::DoubleAttribute* meshAttr = dynamic_cast<SmartBody::DoubleAttribute*>(attribute);
			if (this->dMesh_p)
			{
				for (size_t x = 0; x < this->dMesh_p->dMeshStatic_p.size(); x++)
				{
					SrSnModel* srSnmodel = this->dMesh_p->dMeshStatic_p[x];
					SrModel& model = srSnmodel->shape();
					model.scale((float) meshAttr->getValue());
				}
			}
		}
	}
}

};

