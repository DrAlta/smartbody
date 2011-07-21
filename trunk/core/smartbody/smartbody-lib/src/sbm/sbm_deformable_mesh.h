#ifndef SBM_DEFORMABLE_MESH_H
#define SBM_DEFORMABLE_MESH_H

#include <vector>
#include <string>
#include <map>
#include <sr/sr_sn_shape.h>
#include <sk/sk_skeleton.h>
#include <sr/sr_model.h>

class SkinWeight
{
public:
	std::vector<std::string>	infJointName;	// name array
	std::vector<SkJoint*>		infJoint;         // corresponding joint for each infJointName
	std::vector<float>			bindWeight;		// weight array
	std::vector<SrMat>			bindPoseMat;	// each joint, binding pose transformation matrix
	SrMat						bindShapeMat;	// overall bind shape transformation matrix
	std::string					sourceMesh;		// skin Source Name
	std::vector<unsigned int>	numInfJoints;	// number of influenced joints for very vertex
	std::vector<unsigned int>	weightIndex;	// looking up the weight according to this index
	std::vector<unsigned int>	jointNameIndex;	// looking up the joint name according to this index

public:
	SkinWeight();
	~SkinWeight();
};

/* This class is used to simulate and represent deformed mesh
   for Smartbody Characters.
*/
class DeformableMesh
{
public:
	std::vector<SrSnModel*>		dMeshDynamic_p;
	std::vector<SrSnModel*>		dMeshStatic_p;
	std::vector<SkinWeight*>	skinWeights;
	std::map<std::string, std::vector<std::string> > morphTargets;
	SkSkeleton*					skeleton;			// pointer to current skeleton
	bool						binding;			// whether in deformable mesh mode

public:
	DeformableMesh();
	~DeformableMesh();
	void setSkeleton(SkSkeleton* skel);
	virtual void update();
	SkinWeight* getSkinWeight(std::string skinSourceName);
	int	getMesh(std::string meshName);				// get the postion given the mesh name
    /*! Set the visibility state of the deformable geometry,
        The integers mean 1:show, 0:hide, and -1:don't change the visibility state. */
	void set_visibility(int deformableMesh);
};

#endif
