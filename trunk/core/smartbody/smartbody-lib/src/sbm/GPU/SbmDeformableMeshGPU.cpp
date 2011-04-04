#include "SbmDeformableMeshGPU.h"
#include <string>

const char* ShaderDir = "../src/sbm/GPU/shaderFiles/";
const char* VSName = "vs_skin_pos.vert";
const std::string shaderName = "MeshSkin";


SbmDeformableMeshGPU::SbmDeformableMeshGPU(void)
{
	useGPU = false;
}

SbmDeformableMeshGPU::~SbmDeformableMeshGPU(void)
{

}

void SbmDeformableMeshGPU::initShaderProgram()
{
	std::string vsPathName = ShaderDir;
	vsPathName += VSName;
	//shaderProgram.initShaderProgram(vsPathName.c_str(),NULL);	
	SbmShaderManager::singleton().addShader(shaderName.c_str(),vsPathName.c_str(),NULL);
}

void SbmDeformableMeshGPU::update()
{	
	SbmShaderProgram* program = SbmShaderManager::singleton().getShader(shaderName.c_str());	
	if (!useGPU && program && program->finishBuild())
	{
		// initialize 
		useGPU = true;
	}

	if (!useGPU)
	{							
		DeformableMesh::update();		
	}	
	else
	{
		// GPU update and rendering
		printf("GPU Deformable Model Update\n");

	}
}