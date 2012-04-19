#ifndef SBM_DEFORMABLE_MESH_H
#define SBM_DEFORMABLE_MESH_H

#include <vector>
#include <string>
#include <map>
#include <sr/sr_sn_shape.h>
#include <sk/sk_skeleton.h>
#include <sr/sr_model.h>

typedef std::vector<SkJoint*> SkJointList;

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

class SbmSubMesh
{
public:
	SrMaterial  material;
	std::string texName;
	std::string normalMapName;
	int numTri;
	std::vector<SrVec3i> triBuf;
};

class DeformableMeshInstance;

/* This class is used to simulate and represent deformed mesh
   for Smartbody Characters.
*/
class DeformableMesh
{
public:
	std::string                 meshName;
	std::vector<SrSnModel*>		dMeshDynamic_p;
	std::vector<SrSnModel*>		dMeshStatic_p;
	std::vector<SkinWeight*>	skinWeights;
	std::map<std::string, std::vector<std::string> > morphTargets;
	SkSkeleton*					skeleton;			// pointer to current skeleton
	bool						binding;			// whether in deformable mesh mode		
	// unrolled all vertices into a single buffer for faster GPU rendering
	bool initVertexBuffer;	
	std::vector<SrVec>          posBuf;	
	std::vector<SrVec>          normalBuf;
	std::vector<SrVec>          tangentBuf;
	std::vector<SrVec>          binormalBuf;
	std::vector<SrVec2>         texCoordBuf;	
	std::vector<SrVec3i>        triBuf;
	std::vector<SbmSubMesh*>    subMeshList;

	std::vector<SrVec4i>        boneIDBuf[2];
	std::vector<SrVec4>         boneIDBuf_f[2];
	std::vector<SrVec4>         boneWeightBuf[2];
	std::map<std::string,int>   boneJointIdxMap;
	std::vector<SkJoint*>    boneJointList;	
	std::vector<SrMat>          bindPoseMatList;

public:
	DeformableMesh();
	~DeformableMesh();	
	void setSkeleton(SkSkeleton* skel);
	virtual void update();
	SkinWeight* getSkinWeight(const std::string& skinSourceName);
	int	getMesh(const std::string& meshName);				// get the postion given the mesh name
    /*! Set the visibility state of the deformable geometry,
        The integers mean 1:show, 0:hide, and -1:don't change the visibility state. */
	void set_visibility(int deformableMesh);
	virtual bool buildVertexBuffer(); // unrolled all models inside this deformable mesh into a GPU-friendly format
};

class DeformableMeshInstance
{
protected:
	DeformableMesh* _mesh;
	std::vector<SrSnModel*> dynamicMesh; 
	SkSkeleton*			  _skeleton;
	bool				  _updateMesh;
	std::vector<SkJointList> _boneJointList;
public:
	DeformableMeshInstance();
	~DeformableMeshInstance();

	virtual void setDeformableMesh(DeformableMesh* mesh);

	void updateJointList();
	virtual void setSkeleton(SkSkeleton* skel);	
	virtual void setVisibility(int deformableMesh);
	virtual void update();

protected:
	void cleanUp();
};

#endif
