#ifndef SBM_DEFORMABLE_MESH_H
#define SBM_DEFORMABLE_MESH_H

#include <sb/SBTypes.h>
#include <vector>
#include <string>
#include <map>
#include <sr/sr_sn_shape.h>
#include <sk/sk_skeleton.h>
#include <sr/sr_model.h>
#include <sb/SBAsset.h>
#include <sb/SBCharacter.h>

#include "external/glm/glm/glm.hpp"
#include "external/nanoflann/nanoflann.hpp"

#define USE_SKIN_WEIGHT_SIZE_8 0

#ifdef WIN32
	//#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <wingdi.h>
	#include <GL/gl.h>
#elif defined(SB_IPHONE)
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#elif defined(__APPLE__) || defined(__APPLE_CC__)
       #include <OpenGL/gl.h>
//       #include <Carbon/Carbon.h>
       #define APIENTRY
#elif defined(__FLASHPLAYER__)
	#include <GL/gl.h>
#elif defined(EMSCRIPTEN)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#elif defined(__ANDROID__)
	//#include <GLES/gl.h>
	#include <GLES2/gl2.h>
	//#include "wes_gl.h"
#else
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

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

	// skin weight buffer for easier access to skinning
	std::vector<SrVec4i> boneIDs;
	std::vector<SrVec4>  boneWeights;

public:
	SBAPI SkinWeight();
	
	SBAPI ~SkinWeight();
	SBAPI void normalizeWeights();
	SBAPI void copyWeights(SkinWeight* copy, const std::string& morphName);
	SBAPI void initWeights(std::string sourceMesh, std::vector<SrVec4i>& boneID, std::vector<SrVec4>& boneWeights, std::vector<std::string>& boneJointNameList, std::vector<SrMat>& bindPoseMatList);

	SBAPI void addWeight(SkinWeight* weight);
	SBAPI void mergeRedundantWeight(std::vector<int>& vtxIdxMap);
	SBAPI void buildSkinWeightBuf();
};

class SbmSubMesh
{
public:
	SrMaterial  material;
	bool isHair;
	std::string modelName; // the name of original SrModel
	std::string matName;
	std::string texName;
	std::string normalMapName;
	std::string specularMapName;
	int numTri;
	std::vector<SrVec3i> triBuf;
	std::vector<int> faceIdxList;
};



struct MeshPointCloud
{	
	std::vector<SrVec>  pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline float kdtree_distance(const float *p1, const size_t idx_p2,size_t size) const
	{
		SrVec v1 = SrVec(p1[0],p1[1],p1[2]);		
		return (v1-pts[idx_p2]).norm2();
	}

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline float kdtree_get_pt(const size_t idx, int dim) const
	{
		return pts[idx][dim];
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }
};
typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, MeshPointCloud>, MeshPointCloud, 3> MeshKDTree;


class DeformableMeshInstance;
namespace SmartBodyBinary
{
	class StaticMesh;
}

struct BlendShapeData
{
	std::vector<std::pair<int, SrVec> > diffV;
	std::vector<std::pair<int, SrVec> > diffN;
};

/* This class is used to simulate and represent deformed mesh
   for Smartbody Characters.
*/
class DeformableMesh : public SmartBody::SBAsset
{
public:
	std::vector<SrSnModel*>		dMeshDynamic_p;
	std::vector<SrSnModel*>		dMeshStatic_p;
	std::vector<SkinWeight*>	skinWeights;
	std::map<std::string, std::vector<SrSnModel*> > blendShapeMap;	// the key store the base shape name, vector stores morph target SrModels. first one in the vector is always the base one
	std::map<int, std::vector<int> > blendShapeNewVtxIdxMap; 
	std::map<std::string, std::vector<std::string> > morphTargets;	// stores a vector of morph target names, first one is always the base one
	std::vector<BlendShapeData> optimizedBlendShapeData;						// stores optimized information when calculating blend shapes; list of vertices affected, and their differential vector and normal amounts
	
	std::string                 skeletonName;						// binding skeleton for this deformable model
	SkSkeleton*					skeleton;							// pointer to current skeleton
	
	// unrolled all vertices into a single buffer for faster GPU rendering
	bool initStaticVertexBuffer, initSkinnedVertexBuffer;	
	std::vector<SrVec>          posBuf;	
	std::vector<SrVec>          normalBuf;
	std::vector<SrVec>          tangentBuf;
	std::vector<SrVec>          binormalBuf;
	std::vector<SrVec>          skinColorBuf;
	std::vector<SrVec>          meshColorBuf;
	std::vector<SrVec2>         texCoordBuf;	
	std::vector<SrVec3i>        triBuf;
	std::vector<SbmSubMesh*>    subMeshList;

	std::vector<int>			boneCountBuf;
	std::vector<SrVec>          boneColorMap;
	std::vector<SrVec4i>        boneIDBuf[2];
	std::vector<SrVec4>         boneIDBuf_f[2];
	std::vector<SrVec4>         boneWeightBuf[2];
	std::map<std::string,int>   boneJointIdxMap;
	std::vector<SkJoint*>		boneJointList;	
	std::vector<std::string>    boneJointNameList;
	std::vector<SrMat>          bindPoseMatList;	

	std::vector<int> meshIndexList;
	std::map<int,std::vector<int> > vtxNewVtxIdxMap;

	bool hasVertexColor;	
	bool hasTexCoord;

	SrModel _emptyModel;
public:
	SBAPI DeformableMesh();
	SBAPI virtual ~DeformableMesh();	
	SBAPI void initDeformMesh(std::vector<SrModel*>& meshVec);
	SkinWeight* getSkinWeight(const std::string& skinSourceName);
	SBAPI int getNumMeshes();
	SBAPI const std::string getMeshName(int index);
	SBAPI SrModel& getStaticModel(int index);
	SBAPI int	getMesh(const std::string& meshName);				// get the position given the mesh name
	int getValidSkinMesh(const std::string& meshName);
    
	SBAPI void rebuildVertexBuffer(bool rebuild);

	SBAPI virtual bool buildSkinnedVertexBuffer(); // unrolled all models inside this deformable mesh into a GPU-friendly format
	SBAPI virtual bool buildBlendShapes();
	SBAPI void updateVertexBuffer(); // update the values in the vertex buffer based on dMeshStatic_p	
	SBAPI bool isSkinnedMesh();
	SBAPI bool saveToSmb(std::string inputFileName);
	SBAPI bool saveToDmb(std::string inputFileName);
	SBAPI bool readFromSmb(std::string inputFileName);
	SBAPI bool readFromDmb(std::string inputFileName);
	// helper function
	void saveToStaticMeshBinary(SmartBodyBinary::StaticMesh* mesh);
	void readFromStaticMeshBinary(SmartBodyBinary::StaticMesh* mesh);
	void loadAllFoundTextures(std::string textureDirectory);
	SBAPI SrVec computeCenterOfMass();
	SBAPI SrBox computeBoundingBox();

	SBAPI void translate(SrVec trans);
	SBAPI void rotate(SrVec trans);
	SBAPI void scale(float factor);
	SBAPI void addTransform(const SrMat& transform);
	SBAPI void centralize();
	SBAPI void computeNormals();
	SBAPI void copySkinWeights(DeformableMesh* fromMesh, const std::string& morphName);

};

class DeformableMeshInstance
{
protected:
	DeformableMesh* _mesh;
	//std::vector<SrSnModel*>	dynamicMesh; 
	SkSkeleton*				_skeleton;
	SmartBody::SBCharacter*	_character;		// pointer to current character
	SmartBody::SBPawn*      _pawn;
	bool				  _updateMesh;
	std::vector<SkJointList> _boneJointList;
	SrVec _meshScale;
	int  meshVisibleType;
	bool _recomputeNormal;
	bool _isStaticMesh;

public:
	std::vector<SrVec> _restPosBuf;
	std::vector<SrVec> _deformPosBuf;	
	std::vector<SrMat>  transformBuffer;	

	GLuint _tempTex;
	GLuint _tempFBO;

	GLuint * _tempTexPairs;
	GLuint * _tempFBOPairs;

	GLuint * _tempTexWithMask;
	GLuint * _tempFBOTexWithMask;

public:
	SBAPI DeformableMeshInstance();
	SBAPI virtual ~DeformableMeshInstance();	
	SBAPI virtual void setDeformableMesh(DeformableMesh* mesh);
	SBAPI void updateJointList();
	SBAPI virtual void setPawn(SmartBody::SBPawn* pawn);
	SBAPI virtual void setVisibility(int deformableMesh);
	SBAPI virtual void setMeshScale(SrVec scale);
	SBAPI SrVec   getMeshScale() { return _meshScale; }
	SBAPI int    getVisibility();
	SBAPI void    setToStaticMesh(bool isStatic);
	SBAPI bool    isStaticMesh();
	SBAPI SmartBody::SBSkeleton* getSkeleton();	
	SBAPI virtual void update();
	SBAPI virtual void updateFast();

	SBAPI void blendShapeStaticMesh();
	SBAPI virtual void GPUblendShapes(glm::mat4x4, glm::mat4x4);
	SBAPI virtual void blendShapes();
	SBAPI DeformableMesh* getDeformableMesh() { return _mesh; }
	SBAPI SmartBody::SBCharacter* getCharacter() { return _character; }
	SBAPI SmartBody::SBPawn* getPawn() { return _pawn; }
	SBAPI void updateTransformBuffer();
	void updateSkin(const std::vector<SrVec>& restPos, std::vector<SrVec>& deformPos);
protected:
	void cleanUp();
};

#endif
