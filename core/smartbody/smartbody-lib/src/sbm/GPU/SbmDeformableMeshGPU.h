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
	MeshSubset() {}
	~MeshSubset();
};

class SbmDeformableMeshGPUInstance;

class SbmDeformableMeshGPU : public DeformableMesh
{
public:
	static bool disableRendering;
	static bool useGPUDeformableMesh;
	static bool useShadowPass;	
	static GLuint shadowMapID;
	static bool initShader;
protected:	
	bool useGPU;	
	VBOVec3f *VBOPos;
	VBOVec3f *VBOTangent, *VBOBiNormal;
	VBOVec3f *VBONormal, *VBOOutPos;
	VBOVec2f *VBOTexCoord;
	VBOVec3i *VBOTri;
	std::vector<MeshSubset*> meshSubset;
	VBOVec4f *VBOBoneID1,*VBOBoneID2;
	VBOVec4f *VBOWeight1, *VBOWeight2;
	TBOData  *TBOTran; // bone transformation		
	std::vector<VBOVec3i*> subMeshTris;
	std::vector<SrMat>  transformBuffer;	
public:
	SbmDeformableMeshGPU(void);
	~SbmDeformableMeshGPU(void);	
public:
	virtual void update();
	bool buildGPUVertexBuffer();	
	void skinTransformGPU(std::vector<SrMat>& tranBuffer, TBOData* tranTBO);
	static void initShaderProgram();	
protected:
	bool initBuffer(); // initialize VBO and related GPU data buffer	
	bool initBuffer1();	
	void updateTransformBuffer();	
};

class SbmDeformableMeshGPUInstance : public DeformableMeshInstance
{
protected:
	std::vector<SrMat>  transformBuffer;	
	TBOData  *TBOTran; // bone transformation	
	bool     bufferReady;
public:
	SbmDeformableMeshGPUInstance();
	~SbmDeformableMeshGPUInstance();
	void updateTransformBuffer();
	virtual void update();			
	virtual void setDeformableMesh(DeformableMesh* mesh);
protected:
	bool initBuffer();	
	void cleanBuffer();	
};

