#include "SBJoint.h"
#include "SBSkeleton.h"
#include "SBCharacter.h"
#include <sk/sk_skeleton.h>
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBJoint::SBJoint() : SkJoint()
{
	_localCenter.set(0, 0, 0);
}

SBJoint::SBJoint( SkSkeleton* sk, SkJoint* parent, RotType rtype, int i ) : SkJoint(sk, parent, rtype, i)
{
}

void SBJoint::setName(std::string& str)
{
	name(str);
}

const std::string& SBJoint::getName()
{
	return name();
}

void SBJoint::setParent(SBJoint* parent)
{
	SkJoint::set_parent(parent);
}

SBJoint* SBJoint::getParent()
{
	SkJoint* joint = SkJoint::parent();
	SBJoint* sbJoint = dynamic_cast<SBJoint*>(joint);
	return sbJoint;
}

int SBJoint::getNumChildren()
{
	return num_children();
}

SBJoint* SBJoint::getChild(int index)
{
	if (num_children() > index && index >= 0)
	{
		SkJoint* skchild = child(index);
		SBJoint* sbchild = dynamic_cast<SBJoint*>(skchild);
		return sbchild;
	}
	else
	{
		return NULL;
	}
}

void SBJoint::addChild(SBJoint* child)
{
	add_child(child);

	SBSkeleton* skel = getSkeleton();
	if (skel)
		skel->update();
}

void SBJoint::setSkeleton(SBSkeleton* skeleton)
{
	SkJoint::skeleton(skeleton);

	SBSkeleton* skel = getSkeleton();
	if (skel)
		skel->update();
}

SBSkeleton* SBJoint::getSkeleton()
{
	SkSkeleton* skskel = SkJoint::skeleton();
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}

SrVec SBJoint::getOffset()
{
	return offset();
}

void SBJoint::setOffset(SrVec vec)
{
	offset(vec);

	SBSkeleton* skel = getSkeleton();
	if (skel)
		skel->update();
}

int SBJoint::getIndex()
{
	return index();
}

SrVec SBJoint::getPosition()
{
	update_gmat();
	return pos()->value();
}

SrQuat SBJoint::getQuaternion()
{
	update_gmat();
	return quat()->value();
}

SrMat SBJoint::getMatrixGlobal()
{
	update_gmat();
	return gmat();
}

SrMat SBJoint::getMatrixLocal()
{
	update_lmat();
	return lmat();
}

void SBJoint::setUseRotation(bool val)
{
	if (val)
		quat()->activate();
	else
		quat()->deactivate();

	SBSkeleton* skel = getSkeleton();
	if (skel)
		skel->update();
}

bool SBJoint::isUseRotation()
{
	return quat()->active();
}

void SBJoint::setUsePosition(int index, bool val)
{
	if (index < 0 || index > 3)
		return;

	pos()->limits(index, val);
	SBSkeleton* skel = getSkeleton();
	if (skel)
		skel->update();
}

bool SBJoint::isUsePosition(int index)
{
	if (index < 0 || index > 3)
		return false;

	return pos()->limits(index);

}

void SBJoint::calculateLocalCenter()
{
	_localCenter = SrVec(0,0,0);
	if (this->getNumChildren() == 0) 
		return;

	for (int i=0;i<this->getNumChildren();i++)
	{
		_localCenter += this->getChild(i)->offset()*0.5f;
	}
	_localCenter /= (float)this->getNumChildren();
}

const SrVec& SBJoint::getLocalCenter()
{
	return _localCenter;
}

};
