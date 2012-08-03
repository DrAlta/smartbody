#ifndef _SBJOINT_H_
#define _SBJOINT_H_

#include <sk/sk_joint.h>
#include <sbm/Physics/SbmColObject.h>
#include <string>

#ifdef __native_client__
#include <sb/SBSkeleton.h>
#endif

namespace SmartBody {

#ifndef __native_client__
class SBSkeleton;
#endif

class SBJoint : public SkJoint, public SbmTransformObjInterface
{
	public:
		SBJoint();
		SBJoint( SkSkeleton* sk, SkJoint* parent, RotType rtype, int i );

		void setName(std::string& name);
		const std::string& getName();

		int getIndex();
		
		void setParent(SBJoint* parent);
		SBJoint* getParent();
		int getNumChildren();
		SBJoint* getChild(int index);
		void addChild(SBJoint* child);
		std::vector<SBJoint*> getDescendants();

		void setSkeleton(SBSkeleton* skeleton);
		SBSkeleton* getSkeleton();

		SrVec getOffset();
		void setOffset(SrVec vec);

		SrVec getPosition();
		SrQuat getQuaternion();
		SrMat getMatrixGlobal();
		SrMat getMatrixLocal();

		void setUseRotation(bool val);
		bool isUseRotation();
		void setUsePosition(int index, bool val);
		bool isUsePosition(int index);

		void calculateLocalCenter();
		const SrVec& getLocalCenter();

		void setMass(float mass);
		float getMass();

		void setPrerotation(SrQuat& quat);
		SrQuat getPrerotation();
		void setPostrotation(SrQuat& quat);
		SrQuat getPostrotation();

		virtual SbmTransform& getGlobalTransform();
		virtual void setGlobalTransform(SbmTransform& newGlobalTransform);

	protected:
		SrVec _localCenter;
		SbmTransform globalTransform;
};

};
#endif