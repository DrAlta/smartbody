#pragma once
#include <sb/SBTypes.h>
#include "SbmShader.h"
#include "VBOData.h"
#include "TBOData.h"
#include <sbm/sbm_deformable_mesh.h>
//Zengrui: for Smartbody javascript, simply give each function an empty implementation
#if !defined(EMSCRIPTEN)
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
	SBAPI static bool disableRendering;
	SBAPI static bool useGPUDeformableMesh;
	SBAPI static bool useShadowPass;	
	SBAPI static GLuint shadowMapID;
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
	bool initGPUVertexBuffer;
public:
	SBAPI SbmDeformableMeshGPU(void);
	~SbmDeformableMeshGPU(void);	
public:
	virtual void update();
	virtual bool buildVertexBufferGPU();	
	void skinTransformGPU(DeformableMeshInstance* meshInstance);
	static void initShaderProgram();	
	VBOVec3f* getPosVBO() { return VBOPos; }
	VBOVec3f* getNormalVBO() { return VBONormal; }
	VBOVec3f* getTangentVBO() { return VBOTangent; }
	VBOVec3f* getBiNormalVBO() { return VBOBiNormal; }
	VBOVec2f* getTexCoordVBO() { return VBOTexCoord; }
	VBOVec3i* getTriVBO() { return VBOTri; }
	std::vector<VBOVec3i*>& getVBOSubMeshTris() {
		return subMeshTris;
	}
protected:
	bool initBuffer(); // initialize VBO and related GPU data buffer	
	bool initBuffer1();	
	void updateTransformBuffer();	
};

class SbmDeformableMeshGPUInstance : public DeformableMeshInstance
{
protected:	
	TBOData  *TBOTran; // bone transformation	
	VBOVec3f *VBODeformPos;
	bool     bufferReady;
public:
	SBAPI SbmDeformableMeshGPUInstance();
	~SbmDeformableMeshGPUInstance();
	
	SBAPI virtual void update();			
	SBAPI virtual void setDeformableMesh(DeformableMesh* mesh);
	SBAPI bool initBuffer();

	SBAPI std::vector<SrMat>& getTransformBuffer() { return transformBuffer; }
	SBAPI TBOData*            getTBOTransforBuffer() { return TBOTran; }
	SBAPI VBOVec3f*           getVBODeformPos() { return VBODeformPos; }
protected:
	void gpuBlendShape();	
	void cleanBuffer();	
};

#elif defined(EMSCRIPTEN)
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
	~MeshSubset(){};
};

class SbmDeformableMeshGPUInstance;

class SbmDeformableMeshGPU : public DeformableMesh
{
public:
	SBAPI static bool disableRendering;
	SBAPI static bool useGPUDeformableMesh;
	SBAPI static bool useShadowPass;	
	SBAPI static GLuint shadowMapID;
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
	bool initGPUVertexBuffer;
public:
	SBAPI SbmDeformableMeshGPU(void){};
	~SbmDeformableMeshGPU(void){};	
public:
	virtual void update(){};
	virtual bool buildVertexBufferGPU(){};	
	void skinTransformGPU(DeformableMeshInstance* meshInstance){};
	static void initShaderProgram(){};	
	VBOVec3f* getPosVBO() { return VBOPos; }
protected:
	bool initBuffer(){return true;}; // initialize VBO and related GPU data buffer	
	bool initBuffer1(){return true;};	
	void updateTransformBuffer(){};	
};

class SbmDeformableMeshGPUInstance : public DeformableMeshInstance
{
protected:	
	TBOData  *TBOTran; // bone transformation	
	bool     bufferReady;
public:
	SBAPI SbmDeformableMeshGPUInstance(){};
	~SbmDeformableMeshGPUInstance(){};
	
	virtual void update(){};			
	virtual void setDeformableMesh(DeformableMesh* mesh){};
	std::vector<SrMat>& getTransformBuffer() { return transformBuffer; }
	TBOData*            getTBOTransforBuffer() { return TBOTran; }
protected:
	void gpuBlendShape(){};
	bool initBuffer(){return true;};	
	void cleanBuffer(){};	
};
#endif