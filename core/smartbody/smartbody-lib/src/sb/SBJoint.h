#ifndef _SBJOINT_H_
#define _SBJOINT_H_

#include <sb/SBTypes.h>
#include <sk/sk_joint.h>
#include <sbm/Physics/SBColObject.h>
#include <string>

#ifdef __native_client__
#include <sb/SBSkeleton.h>
#endif

namespace SmartBody {

#ifndef __native_client__
class SBSkeleton;
#endif

class SBJoint : public SkJoint, public SBTransformObjInterface
{
	public:
		SBAPI SBJoint();
		SBAPI SBJoint( SkSkeleton* sk, SkJoint* parent, RotType rtype, int i );

		SBAPI void setName(const std::string& name);
		SBAPI const std::string& getName();

		SBAPI int getIndex();
		
		SBAPI void setParent(SBJoint* parent);
		SBAPI SBJoint* getParent();
		SBAPI int getNumChildren();
		SBAPI SBJoint* getChild(int index);
		SBAPI void addChild(SBJoint* child);
		SBAPI std::vector<SBJoint*> getDescendants();

		SBAPI void setSkeleton(SBSkeleton* skeleton);
		SBAPI SBSkeleton* getSkeleton();

		SBAPI SrVec getOffset();
		SBAPI void setOffset(SrVec vec);

		SBAPI SrVec getPosition();
		SBAPI SrQuat getQuaternion();
		SBAPI SrMat getMatrixGlobal();
		SBAPI SrMat getMatrixLocal();

		SBAPI void setUseRotation(bool val);
		SBAPI bool isUseRotation();
		SBAPI void setUsePosition(int index, bool val);
		SBAPI bool isUsePosition(int index);

		SBAPI void calculateLocalCenter();
		SBAPI const SrVec& getLocalCenter();

		SBAPI void setMass(float mass);
		SBAPI float getMass();

		SBAPI void setPrerotation(SrQuat& quat);
		SBAPI SrQuat getPrerotation();
		SBAPI void setPostrotation(SrQuat& quat);
		SBAPI SrQuat getPostrotation();

		SBAPI virtual SBTransform& getGlobalTransform();
		SBAPI virtual void setGlobalTransform(SBTransform& newGlobalTransform);

	protected:
		SrVec _localCenter;
		SBTransform globalTransform;
};

};
#endif