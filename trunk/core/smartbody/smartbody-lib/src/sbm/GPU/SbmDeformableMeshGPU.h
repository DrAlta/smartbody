#pragma once
#include "SbmShader.h"
#include "VBOData.h"
#include "TBOData.h"
#include <sbm/sbm_deformable_mesh.h>

class MeshSubset
{
public:	
	SrMaterial  material;
	std::string texName;
	std::string normalMapName;
	VBOVec3i* VBOTri;
	int       numTri;
public:	
};

class SbmDeformableMeshGPU : public DeformableMesh
{
public:
	static bool useGPUDeformableMesh;
	static bool useShadowPass;	
	static GLuint shadowMapID;
protected:
	static bool initShader;
	bool useGPU;	
	int numTotalVtxs, numTotalTris;
	VBOVec4f *VBOPos;
	VBOVec3f *VBOTangent, *VBOBiNormal;
	VBOVec3f *VBONormal, *VBOOutPos;
	VBOVec3f *VBOTexCoord;
	VBOVec3i *VBOTri;
	std::vector<MeshSubset*> meshSubset;
	VBOVec4f *VBOBoneID1,*VBOBoneID2, *VBOWeight1, *VBOWeight2;
	TBOData  *TBOTran; // bone transformation	
	std::vector<SkJoint*> boneJointList;
	std::vector<SrMat>    bindPoseMatList;
	ublas::vector<SrMat>  transformBuffer;	
public:
	SbmDeformableMeshGPU(void);
	~SbmDeformableMeshGPU(void);

public:
	virtual void update();
protected:
	bool initBuffer(); // initialize VBO and related GPU data buffer	
	static void initShaderProgram();
	void skinTransformGPU();
	void updateTransformBuffer();	
};

