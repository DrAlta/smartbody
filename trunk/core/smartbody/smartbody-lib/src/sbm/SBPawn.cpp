#include "SBPawn.h"

namespace SmartBody {

SBPawn::SBPawn() : SbmPawn()
{
	createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
}

SBPawn::SBPawn(const char* name) : SbmPawn(name)
{
	createDoubleAttribute("posX", 0.0, true, "transform", 10, false, false, false, "X position");
	createDoubleAttribute("posY", 0.0, true, "transform", 20, false, false, false, "Y position");
	createDoubleAttribute("posZ", 0.0, true, "transform", 30, false, false, false, "Z position");
	createDoubleAttribute("rotX", 0.0, true, "transform", 40, false, false, false, "X rotation");
	createDoubleAttribute("rotY", 0.0, true, "transform", 50, false, false, false, "Y rotation");
	createDoubleAttribute("rotZ", 0.0, true, "transform", 60, false, false, false, "Z rotation");
}

SBPawn::~SBPawn()
{
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


void SBPawn::notify(SBSubject* subject)
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "posX")
		{
			double val = this->getDoubleAttribute(this->getName());
			SrVec position = this->getPosition();
			position.x = (float) val;
			this->setPosition(position);
		}
		else if (attribute->getName() == "posY")
		{
			double val = this->getDoubleAttribute(this->getName());
			SrVec position = this->getPosition();
			position.y = (float) val;
			this->setPosition(position);
		}
		else if (attribute->getName() == "posZ")
		{
			double val = this->getDoubleAttribute(this->getName());
			SrVec position = this->getPosition();
			position.z = (float) val;
			this->setPosition(position);
		}
	}
}

};

