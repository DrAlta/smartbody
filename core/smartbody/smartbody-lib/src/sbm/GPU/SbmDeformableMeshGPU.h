#pragma once
#include "SbmShader.h"
#include "VBOData.h"
#include <sbm/sbm_deformable_mesh.h>

class SbmDeformableMeshGPU : public DeformableMesh
{
protected:
	bool useGPU;
	SbmShaderProgram shaderProgram;
public:
	SbmDeformableMeshGPU(void);
	~SbmDeformableMeshGPU(void);

public:
	virtual void update();
protected:
	void initShaderProgram();
};
