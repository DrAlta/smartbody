#pragma once
#include "SbmShader.h"
#include "VBOData.h"
#include "TBOData.h"
#include <sbm/sbm_deformable_mesh.h>

class SbmDeformableMeshGPU : public DeformableMesh
{
protected:
	static bool initShader;
	bool useGPU;
	SbmShaderProgram shaderProgram;
	int numTotalVtxs, numTotalTris;
	VBOVec3f *VBOPos, *VBONormal, *VBOOutPos;
	VBOVec3i *VBOTri;
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
	void drawVBO();
};
