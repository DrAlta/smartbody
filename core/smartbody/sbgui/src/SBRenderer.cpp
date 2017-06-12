/*************************************************************
Copyright (C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/


#if !defined(__FLASHPLAYER__) && !defined(__ANDROID__) && !defined(EMSCRIPTEN)
#include "external/glew/glew.h"
#endif

#include "SBRenderer.h"
#include "fltk_viewer.h"

#include <sb/SBObject.h>
#include <sb/SBAttribute.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPawn.h>
#include <sb/SBCharacter.h>
#include <sb/SBUtilities.h>
#include <sr/sr_camera.h>
#include <sr/sr_mat.h>
#include <sr/sr_random.h>

#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sbm/GPU/VBOData.h>

#include <algorithm>


SBRenderer* SBRenderer::_singleton = NULL;

SBRenderer::SBRenderer()
{
	ssaoOutput = ssaoNoise = ssaoBlurOutput = lightPassOutput = NULL;
}


SBRenderer::~SBRenderer()
{
}

SBRenderer& SBRenderer::singleton()
{
	if (!_singleton)
		_singleton = new SBRenderer();
	return *_singleton;
}

void SBRenderer::destroy_singleton()
{
	delete _singleton;
	_singleton = NULL;
}


void SBRenderer::drawDebugFBO()
{
	std::string gbufferDebug = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.gbufferDebug");
	if (gbufferDebug == "" || gbufferDebug == "none")
		return;
	SbmTexture* debugTex = NULL;
	if (gbufferDebug == "posTex")
		debugTex = gbuffer.posTex;
	else if (gbufferDebug == "normalTex")
		debugTex = gbuffer.normalTex;
	else if (gbufferDebug == "diffuseTex")
		debugTex = gbuffer.diffuseTex;
	else if (gbufferDebug == "specularTex")
		debugTex = gbuffer.specularTex;
	else if (gbufferDebug == "glossyTex")
		debugTex = gbuffer.glossyTex;
	else if (gbufferDebug == "depthTex")
		debugTex = gbuffer.depthTex;
	else if (gbufferDebug == "ssaoTex")
		debugTex = ssaoOutput;
	else if (gbufferDebug == "ssaoBlurTex")
		debugTex = ssaoBlurOutput;
	else if (gbufferDebug == "lightTex")
		debugTex = lightPassOutput;
	else if (gbufferDebug == "envMapTex")
	{
		debugTex = getCurEnvMap();
	}
	else
		debugTex = gbuffer.posTex; // draw posTex by default

	if (!debugTex)
	{		
		return;
	}

	//SBFrameBufferObject::drawTextureQuad(debugTex);
	drawTextureQuadWithDepth(debugTex, gbuffer.depthTex);
}

SbmTexture* SBRenderer::getCurEnvMap(bool diffuseMap)
{
	SbmTexture* debugTex = NULL;
	std::string texName = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.envMapName");
	if (diffuseMap)
		texName = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.envDiffuseMapName");
	SbmTextureManager& texManager = SbmTextureManager::singleton();
	debugTex = texManager.findTexture(SbmTextureManager::TEXTURE_HDR_MAP, texName.c_str());
	if (!debugTex)
		SmartBody::util::log("Error ! Can't find tex name = %s", texName.c_str());		
	return debugTex;
}

void SBRenderer::initRenderer(int w, int h)
{
	width = w;
	height = h;
	gbuffer.initBuffer(w, h);

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	std::string shaderPath = scene->getMediaPath() + "/shaders/";
	SbmShaderManager& shaderManager = SbmShaderManager::singleton();
	shaderManager.addShader("gbuffer_shader", shaderPath + "gbuffer.vert", shaderPath + "gbuffer.frag", true);
	gbufferShader = shaderManager.getShader("gbuffer_shader");

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	lightPassFBO.initFBO("lightPassFBO");
	lightPassOutput = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "lightPassOutput");
	lightPassOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
	lightPassOutput->buildTexture(false, true);
	lightPassFBO.attachTexture(lightPassOutput, GL_COLOR_ATTACHMENT0);
	lightPassFBO.setDrawBufferDefault();

	shaderManager.addShader("lightPass_shader", shaderPath + "lighting.vert", shaderPath + "lighting.frag", true);
	lightPassShader = shaderManager.getShader("lightPass_shader");

	shaderManager.addShader("ibl_shader", shaderPath + "ibl.vert", shaderPath + "ibl.frag", true);
	iblShader = shaderManager.getShader("ibl_shader");

	shaderManager.addShader("skinning_shader", shaderPath + "skinning.vert", "", true);
	skinningShader = shaderManager.getShader("skinning_shader");		

	shaderManager.addShader("depthQuad_shader", shaderPath + "depthQuad.vert", shaderPath + "depthQuad.frag", true);
	depthQuadShader = shaderManager.getShader("depthQuad_shader");
	

	texManager.createColorTexture("white_tex", SrColor::white);
	texManager.createColorTexture("black_tex", SrColor::black);
	texManager.createColorTexture("gray_tex", SrColor(0.2f, 0.2f, 0.2f, 0.2f));
	texManager.createColorTexture("defaultNormal_tex", SrColor(0.5f, 0.5f, 1.0f));
	texManager.updateEnvMaps();
}

void SBRenderer::initSSAO(int w, int h)
{
	// create ssao render target
	SbmTextureManager& texManager = SbmTextureManager::singleton();	
	ssaoFBO.initFBO("ssaoFBO");

	ssaoOutput = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "ssaoOutput");
	ssaoOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
	ssaoOutput->buildTexture(false, true);
	ssaoFBO.attachTexture(ssaoOutput, GL_COLOR_ATTACHMENT0);
	ssaoFBO.setDrawBufferDefault();

	ssaoBlurFBO.initFBO("ssaoBlurFBO");
	ssaoBlurOutput = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "ssaoBlurOutput");
	ssaoBlurOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
	ssaoBlurOutput->buildTexture(false, true);
	ssaoBlurFBO.attachTexture(ssaoBlurOutput, GL_COLOR_ATTACHMENT0);
	ssaoBlurFBO.setDrawBufferDefault();

	SrRandom random;
	random.seed(clock());
	ssaoNoiseSize = 4;
	ssaoKernelSize = 30;
	
	// create ssaoKernel
	ssaoKernel.resize(ssaoKernelSize);
	for (unsigned int i = 0; i < ssaoKernel.size(); i++)
	{
		// randomly sample a point inside a unit hemi-sphere
		ssaoKernel[i] = SrVec(random.get(-1.f, 1.f), random.get(-1.f, 1.f), random.get(0.f, 1.f));
		ssaoKernel[i].normalize();
		// favor sample points that are closer to the vertex position, rescale the samples based on a distance cut-off curve
		float ratio = (float)i / (float)ssaoKernel.size();
		float rr = ratio*ratio;
		float kernelWeight = 0.1*(1.f - rr) + 1.f*rr;
		ssaoKernel[i] *= kernelWeight;
		SmartBody::util::log("Kernel %d, vec = %s", i, ssaoKernel[i].toString().c_str());
	}
	
	

	// create ssao noise texture
	int noiseSqr = ssaoNoiseSize*ssaoNoiseSize;
	ssaoNoise = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "ssaoNoise");
	ssaoNoise->createEmptyTexture(ssaoNoiseSize, ssaoNoiseSize, 3, GL_FLOAT);	
	std::vector<SrVec> noiseTexData(noiseSqr);	
	for (unsigned int i = 0; i < noiseTexData.size(); i++)
	{
		noiseTexData[i] = SrVec(random.get(-1.f, 1.f), random.get(-1.f, 1.f), 0.f);
		noiseTexData[i].normalize();
	}
	ssaoNoise->setBuffer((unsigned char*)&noiseTexData[0], noiseTexData.size() * sizeof(SrVec));
	ssaoNoise->buildTexture(false, true);

	

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string shaderPath = scene->getMediaPath() + "/shaders/";
	SbmShaderManager& shaderManager = SbmShaderManager::singleton();
	shaderManager.addShader("ssao_shader", shaderPath + "ssao.vert", shaderPath + "ssao.frag", true);
	ssaoShader = shaderManager.getShader("ssao_shader");
	ssaoShader->buildShader();

	shaderManager.addShader("ssaoBlur_shader", shaderPath + "blur.vert", shaderPath + "blur.frag", true);
	ssaoBlurShader = shaderManager.getShader("ssaoBlur_shader");
	ssaoBlurShader->buildShader();


	glUseProgram(ssaoShader->getShaderProgram());
	glUniform1i(glGetUniformLocation(ssaoShader->getShaderProgram(), "uKernelSize"), ssaoKernelSize);
	glUniform3fv(glGetUniformLocation(ssaoShader->getShaderProgram(), "uKernelOffsets"), ssaoKernelSize, (GLfloat*)&ssaoKernel[0]);
	glUseProgram(0);	


	glUseProgram(ssaoBlurShader->getShaderProgram());
	glUniform1i(glGetUniformLocation(ssaoBlurShader->getShaderProgram(), "uBlurSize"), ssaoNoiseSize);
	glUseProgram(0);
}



void SBRenderer::resize(int w, int h)
{
	gbuffer.resize(w, h);
	if (ssaoOutput)
	{
		ssaoOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
		ssaoOutput->buildTexture(false, false);
	}

	if (ssaoBlurOutput)
	{
		ssaoBlurOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
		ssaoBlurOutput->buildTexture(false, false);
	}

	if (lightPassOutput)
	{
		lightPassOutput->createEmptyTexture(w, h, 4, GL_FLOAT);
		lightPassOutput->buildTexture(false, false);
	}
	
	width = w;
	height = h;
}

void SBRenderer::drawTestDeferred(std::vector<SrLight>& lights, FltkViewerData* viewData)
{	
	// update skinning transform

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> pawnNames = scene->getPawnNames();
#if 1
	for (unsigned int i = 0; i < pawnNames.size(); i++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(pawnNames[i]);
		DeformableMeshInstance* meshInstance = pawn->getActiveMesh();		
		if (meshInstance)
		{
			if (!meshInstance->isStaticMesh())
			{
				SbmDeformableMeshGPUInstance* gpuMeshInstance = dynamic_cast<SbmDeformableMeshGPUInstance*>(meshInstance);				
				GPUMeshUpdate(meshInstance);
			}			
		}
	}
#endif

	gbuffer.bindFBO();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(gbufferShader->getShaderProgram());	
	SrCamera* cam = SmartBody::SBScene::getScene()->getActiveCamera();
	// if there is no active camera, then only show the blank screen
	if (!cam)
		return;	

	SrMat mat, modelViewMat, modelViewProjMat;
	//----- Set Projection ----------------------------------------------
	cam->setAspectRatio((float)width / (float)height);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(cam->get_perspective_mat(mat));

	//----- Set Visualisation -------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrix(cam->get_view_mat(mat));
	glScalef(cam->getScale(), cam->getScale(), cam->getScale());
#endif
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	cam->get_view_mat(modelViewMat);
	cam->get_perspective_mat(modelViewProjMat);
	modelViewProjMat = modelViewMat*modelViewProjMat;

	glUniformMatrix4fv(glGetUniformLocation(gbufferShader->getShaderProgram(), "modelViewMat"), 1, GL_FALSE, (GLfloat*)&modelViewMat);
	glUniformMatrix4fv(glGetUniformLocation(gbufferShader->getShaderProgram(), "modelViewProjMat"), 1, GL_FALSE, (GLfloat*)&modelViewProjMat);
	
	for (unsigned int i = 0; i < pawnNames.size(); i++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(pawnNames[i]);
		DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
		float alphaThreshold = (float)pawn->getDoubleAttribute("alphaThreshold");
		glUniform1f(glGetUniformLocation(gbufferShader->getShaderProgram(), "alphaThreshold"), alphaThreshold);
		if (meshInstance)
		{				
			renderMesh(meshInstance);
		}
	}

	drawFloor(viewData);
	glUseProgram(0);
	gbuffer.unbindFBO();

	std::string gbufferDebug = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.gbufferDebug");
	drawTestSSAO();
	//drawLightPass(lights);
	drawIBLPass(lights);

	drawDebugFBO();
}

void SBRenderer::drawLightPass(std::vector<SrLight>& lights)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = SmartBody::SBScene::getScene()->getActiveCamera();
	// if there is no active camera, then only show the blank screen
	if (!cam)
		return;

	lightPassFBO.bindFBO();	
	glUseProgram(lightPassShader->getShaderProgram());

	SrMat viewMat;
	cam->get_view_mat(viewMat);
	viewMat = viewMat.get_rotation();
	std::vector<SrVec> lightDirs, lightColors;
	float lcolor[4];
	// update light direction
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		SrVec ldir = lights[i].position*viewMat;
		ldir.normalize();
		lightDirs.push_back(ldir);
		lights[i].diffuse.get(lcolor);
		lightColors.push_back(SrVec(lcolor[0], lcolor[1], lcolor[2]));
		//lightColors.push_back(SrVec(1,1,1));
	}
	
	glUniform1i(glGetUniformLocation(lightPassShader->getShaderProgram(), "numLights"), lights.size());
	glUniform3fv(glGetUniformLocation(lightPassShader->getShaderProgram(), "lightDir"), lights.size(), (GLfloat*)&lightDirs[0]);
	glUniform3fv(glGetUniformLocation(lightPassShader->getShaderProgram(), "lightColor"), lights.size(), (GLfloat*)&lightColors[0]);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// binding the textures
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurOutput->getID());
	//glBindTexture(GL_TEXTURE_2D, gbuffer.posTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuseTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



	SBFrameBufferObject::drawScreenQuad();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);	


	glUseProgram(0);
	lightPassFBO.unbindFBO();	
}



void SBRenderer::drawIBLPass(std::vector<SrLight>& lights)
{
	std::string texName = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.envMapName");
	SbmTextureManager& texManager = SbmTextureManager::singleton();
	SbmTexture* envMap = texManager.findTexture(SbmTextureManager::TEXTURE_HDR_MAP, texName.c_str());
	if (!envMap) return;

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = SmartBody::SBScene::getScene()->getActiveCamera();
	// if there is no active camera, then only show the blank screen
	if (!cam)
		return;

	lightPassFBO.bindFBO();
	glUseProgram(iblShader->getShaderProgram());

	SrMat viewMat;
	cam->get_view_mat(viewMat);
	viewMat = viewMat.get_rotation();
	SrMat invViewMat = viewMat.inverse();

	float tanHalfFov = tanf(cam->getFov() / 2.f);
	float aspectRatio = cam->getAspectRatio();

	SbmTexture* curEnvMap = getCurEnvMap(false);
	SbmTexture* curEnvDiffuseMap = getCurEnvMap(true);
	GLuint envMapTexID = -1, envDiffuseTexId = -1;
	if (curEnvMap)
		envMapTexID = curEnvMap->getID();
	if (curEnvDiffuseMap)
		envDiffuseTexId = curEnvDiffuseMap->getID();

	float roughness = 0.f;
	float metallic = 0.f;
	
	roughness = scene->getDoubleAttribute("Renderer.roughness");
	roughness = std::min(1.f, std::max(roughness, 0.f));
	metallic = scene->getDoubleAttribute("Renderer.metallic");
	metallic = std::min(1.f, std::max(metallic, 0.f));

	float exposure = 1.f;
	float gamma = 1.f;
	exposure = scene->getDoubleAttribute("Renderer.exposure");
	exposure = std::min(100.f, std::max(exposure, 0.f));
	//gamma = scene->getDoubleAttribute("Renderer.gamma");
	//gamma = std::min(10.f, std::max(gamma, 0.f));

	// update lights to shaders
	std::vector<SrVec> lightDirs, lightColors;
	float lcolor[4];
	// update light direction
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		SrVec ldir = lights[i].position;
		ldir.normalize();
		lightDirs.push_back(ldir);
		lights[i].diffuse.get(lcolor);
		lightColors.push_back(SrVec(lcolor[0], lcolor[1], lcolor[2]));		
	}

	glUniform1i(glGetUniformLocation(iblShader->getShaderProgram(), "showEnvMap"), scene->getBoolAttribute("Renderer.showEnvMap") ? 1 : 0);
	glUniform1i(glGetUniformLocation(iblShader->getShaderProgram(), "numLights"), lights.size());
	glUniform3fv(glGetUniformLocation(iblShader->getShaderProgram(), "lightDir"), lights.size(), (GLfloat*)&lightDirs[0]);
	glUniform3fv(glGetUniformLocation(iblShader->getShaderProgram(), "lightColor"), lights.size(), (GLfloat*)&lightColors[0]);
	

	glUniformMatrix4fv(glGetUniformLocation(iblShader->getShaderProgram(), "uViewToWorldMatrix"), 1, GL_FALSE, (GLfloat*)&invViewMat);
 	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "uTanHalfFov"), tanHalfFov);
	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "uAspectRatio"), aspectRatio);
	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "roughnessVal"), roughness);
	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "metallic"), metallic);

	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "exposure"), exposure);
	//glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "gamma"), gamma);
	//glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "shininess"), shininess);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// binding the textures
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, envMapTexID);
	//glBindTexture(GL_TEXTURE_2D, gbuffer.posTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, envDiffuseTexId);
	//glBindTexture(GL_TEXTURE_2D, gbuffer.posTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuseTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurOutput->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gbuffer.specularTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



	SBFrameBufferObject::drawScreenQuad();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, 0);


	glUseProgram(0);
	lightPassFBO.unbindFBO();

}

void SBRenderer::drawTestSSAO()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = SmartBody::SBScene::getScene()->getActiveCamera();
	// if there is no active camera, then only show the blank screen
	if (!cam)
		return;

	ssaoFBO.bindFBO();
	float tanHalfFov = tanf(cam->getFov()/2.f);
	float aspectRatio = cam->getAspectRatio();
	float radius = scene->getDoubleAttribute("Renderer.ssaoRadius");
	float power = scene->getDoubleAttribute("Renderer.ssaoPower");
	glUseProgram(ssaoShader->getShaderProgram());

	SrMat projMatrix;
	cam->get_perspective_mat(projMatrix);
	//SmartBody::util::log("Proj Mat = %s", projMatrix.toString().c_str());
	glUniformMatrix4fv(glGetUniformLocation(ssaoShader->getShaderProgram(), "uProjectionMatrix"), 1, GL_FALSE, (GLfloat*)&projMatrix);
	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "uTanHalfFov"), tanHalfFov);
	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "uAspectRatio"), aspectRatio);
	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "farPlane"), cam->getFarPlane());
	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "nearPlane"), cam->getNearPlane());

	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "uRadius"), radius);
	glUniform1f(glGetUniformLocation(ssaoShader->getShaderProgram(), "uPower"), power);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// binding the textures
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbuffer.depthTex->getID());
	//glBindTexture(GL_TEXTURE_2D, gbuffer.posTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssaoNoise->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#if 1
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuseTex->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
	

	SBFrameBufferObject::drawScreenQuad();
#if 0
	glBegin(GL_QUADS);	
#if 1
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
#else
	glVertex2f(-1.0f, 1.0f);
	glVertex2f(-1.0f, -1.0f);
	glVertex2f(1.0f, -1.0f);
	glVertex2f(1.0f, 1.0f);
#endif	
	glEnd();
#endif
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);


	glUseProgram(0);
	ssaoFBO.unbindFBO();


	ssaoBlurFBO.bindFBO();
	glUseProgram(ssaoBlurShader->getShaderProgram());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoOutput->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SBFrameBufferObject::drawScreenQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	ssaoBlurFBO.unbindFBO();
}


void SBRenderer::renderMesh(DeformableMeshInstance* meshInstance)
{

	DeformableMesh* mesh = meshInstance->getDeformableMesh();
	if (!mesh)
	{
		SmartBody::util::log("SBRenderer::renderMesh ERR: no deformable mesh found!");
		return; // no deformable mesh
	}

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	SbmTexture* whiteTex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, "white_tex");
	SbmTexture* blackTex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, "black_tex");
	SbmTexture* grayTex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, "gray_tex");
	SbmTexture* blueTex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, "defaultNormal_tex");

#if 0
	if (meshInstance->_deformPosBuf.size() > 0)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (GLfloat*)&meshInstance->_deformPosBuf[0]);
	}
	if (mesh->normalBuf.size() > 0)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, (GLfloat*)&mesh->normalBuf[0]);
	}


	if (mesh->texCoordBuf.size() > 0)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (GLfloat*)&mesh->texCoordBuf[0]);
	}	

	if (mesh->tangentBuf.size() > 0)
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat*)&mesh->tangentBuf[0]);
	}
#else // use vertex attribute array
	
	SbmDeformableMeshGPUInstance* gpuMeshInstance = (SbmDeformableMeshGPUInstance*)meshInstance;
	if (!gpuMeshInstance->getVBODeformPos())
		gpuMeshInstance->initBuffer();

	glDisable(GL_CULL_FACE);
	SbmDeformableMeshGPU* gpuMesh = (SbmDeformableMeshGPU*)gpuMeshInstance->getDeformableMesh();
	//VBOVec3f* posVBO = gpuMeshInstance->getVBODeformPos();
	//posVBO->VBO()->UpdateWithData(meshInstance->_deformPosBuf);


	glEnableVertexAttribArray(0);
	gpuMeshInstance->getVBODeformPos()->VBO()->BindBuffer();
	//gpuMesh->getPosVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	gpuMeshInstance->getVBODeformNormal()->VBO()->BindBuffer();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	gpuMesh->getTexCoordVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(3);
	gpuMeshInstance->getVBODeformTangent()->VBO()->BindBuffer();
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

// 	glEnableVertexAttribArray(4);
// 	gpuMesh->getBiNormalVBO()->VBO()->BindBuffer();
// 	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

#endif

	std::vector<SbmSubMesh*>& subMeshList = mesh->subMeshList;
	std::vector<VBOVec3i*>& subMeshTris = gpuMesh->getVBOSubMeshTris();
	for (unsigned int i = 0; i < subMeshList.size(); i++)
	{
		SbmSubMesh* subMesh = subMeshList[i];	
		VBOVec3i* subMeshVBO = subMeshTris[i];
		glMaterial(subMesh->material);		
		if (subMesh->material.useAlphaBlend)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}

		SmartBody::SBSkeleton* skel = meshInstance->getSkeleton();
		SmartBody::SBPawn* pawn = skel->getPawn();		
		
		SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, subMesh->texName.c_str());
		SbmTexture* normalTex = texManager.findTexture(SbmTextureManager::TEXTURE_NORMALMAP, subMesh->normalMapName.c_str());	
		SbmTexture* specularTex = texManager.findTexture(SbmTextureManager::TEXTURE_SPECULARMAP, subMesh->specularMapName.c_str());
		//SbmTexture* glossyTex = texManager.findTexture(SbmTextureManager::TEXTURE_SPECULARMAP, subMesh->specularMapName.c_str());

		if (!tex)
			tex = whiteTex;
		if (!normalTex)
			normalTex = blueTex; // to use only original normal

		if (!specularTex)
			specularTex = grayTex;

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, tex->getID());		

		glActiveTexture(GL_TEXTURE1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, normalTex->getID());

		glActiveTexture(GL_TEXTURE2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, specularTex->getID());
		
		subMeshVBO->VBO()->BindBuffer();
		//glDrawElements(GL_TRIANGLES, subMesh->triBuf.size() * 3, GL_UNSIGNED_INT, &subMesh->triBuf[0]);
		glDrawElements(GL_TRIANGLES, subMesh->triBuf.size() * 3, GL_UNSIGNED_INT, 0);
		subMeshVBO->VBO()->UnbindBuffer();
	}

	gpuMeshInstance->getVBODeformPos()->VBO()->UnbindBuffer();
	gpuMeshInstance->getVBODeformNormal()->VBO()->UnbindBuffer();
	gpuMesh->getTexCoordVBO()->VBO()->UnbindBuffer();
	gpuMeshInstance->getVBODeformTangent()->VBO()->UnbindBuffer();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);	
}

void SBRenderer::registerGUI()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	std::vector<std::string> GBufferOptions;
	GBufferOptions.push_back("none");
	GBufferOptions.push_back("posTex");
	GBufferOptions.push_back("normalTex");
	GBufferOptions.push_back("diffuseTex");
	GBufferOptions.push_back("specularTex");
	GBufferOptions.push_back("glossyTex");
	GBufferOptions.push_back("depthTex");
	GBufferOptions.push_back("ssaoTex");
	GBufferOptions.push_back("ssaoBlurTex");
	GBufferOptions.push_back("lightTex");
	GBufferOptions.push_back("envMapTex");

	SmartBody::StringAttribute* gbufferOptionsAttr = scene->createStringAttribute("Renderer.gbufferDebug", "none", true, "Renderer", 60, false, false, false, "Debug gbuffer");
	gbufferOptionsAttr->setValidValues(GBufferOptions);

	scene->createStringAttribute("Renderer.envMapName", "noname", true, "Renderer", 60, false, false, false, "Name of env texture for background and IBL lighting");
	scene->createStringAttribute("Renderer.envDiffuseMapName", "noname", true, "Renderer", 60, false, false, false, "Name of diffuse convoluted env texture for background and IBL lighting");

		
	SmartBody::DoubleAttribute* ssaoRadiusAttr = scene->createDoubleAttribute("Renderer.ssaoRadius", 0.15, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");
	SmartBody::DoubleAttribute* ssaoPowerAttr = scene->createDoubleAttribute("Renderer.ssaoPower", 1.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");

	SmartBody::DoubleAttribute* roughnessAttr = scene->createDoubleAttribute("Renderer.roughness", 0.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");
	SmartBody::DoubleAttribute* metallicAttr = scene->createDoubleAttribute("Renderer.metallic", 0.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");
	SmartBody::DoubleAttribute* exposureAttr = scene->createDoubleAttribute("Renderer.exposure", 5.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");
	SmartBody::BoolAttribute* showEnvMap = scene->createBoolAttribute("Renderer.showEnvMap", true, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");

	//SmartBody::DoubleAttribute* gammaAttr = scene->createDoubleAttribute("Renderer.gamma", 1.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");



	//SmartBody::DoubleAttribute* roughnessAttr = scene->createDoubleAttribute("Renderer.shininess", 0.0, true, "Renderer", 60, false, false, false, "SSAO Kernel Size");
}



void SBRenderer::drawFloor(FltkViewerData* viewerData)
{
	if (viewerData->showFloor)
	{
		static GLfloat mat_emissin[] = { 0.f,  0.f,    0.f,    1.f };
		static GLfloat mat_ambient[] = { 0.f,  0.f,    0.f,    1.f };
		static GLfloat mat_diffuse[] = { 0.5f,  0.5f,    0.5f,    1.f };
		static GLfloat mat_speclar[] = { 0.f,  0.f,    0.f,    1.f };
		std::string defaultTexName = "white_tex";
		std::string defaultNormalTex = "defaultNormal_tex";
		std::string defaultGlossyTex = "gray_tex";
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emissin);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

		viewerData->floorColor.get(mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_speclar);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
		glEnable(GL_LIGHTING);
		
		SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, defaultTexName.c_str());
		SbmTexture* blueTex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, defaultNormalTex.c_str());
		SbmTexture* grayTex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, defaultGlossyTex.c_str());
		//	If we are using blended textures
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		if (tex)
			glBindTexture(GL_TEXTURE_2D, tex->getID());

		glActiveTexture(GL_TEXTURE1);
		if (blueTex)
			glBindTexture(GL_TEXTURE_2D, blueTex->getID());

		glActiveTexture(GL_TEXTURE2);
		if (grayTex)
			glBindTexture(GL_TEXTURE_2D, grayTex->getID());


		SrVec upVec = SrVec(0.f, 1.f, 0.01f);
		float floorSize = 1200;
		float planeY = -0.0f;
		glBegin(GL_QUADS);
		glVertexAttrib2f(2, 0, 0);
		glVertexAttrib3f(1, upVec[0], upVec[1], upVec[2]);
		glVertex3f(-floorSize, planeY, floorSize);
		glVertexAttrib2f(2, 0, 1);
		glVertexAttrib3f(1, upVec[0], upVec[1], upVec[2]);
		glVertex3f(floorSize, planeY, floorSize);
		glVertexAttrib2f(2, 1, 1);
		glVertexAttrib3f(1, upVec[0], upVec[1], upVec[2]);
		glVertex3f(floorSize, planeY, -floorSize);
		glVertexAttrib2f(2, 1, 0);
		glVertexAttrib3f(1, upVec[0], upVec[1], upVec[2]);
		glVertex3f(-floorSize, planeY, -floorSize);
		glEnd();
		// unbind texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SBRenderer::GPUMeshUpdate(DeformableMeshInstance* meshInstance)
{
	SbmDeformableMeshGPUInstance* gpuMeshInstance = (SbmDeformableMeshGPUInstance*)meshInstance;	
	SbmDeformableMeshGPU* gpuMesh = (SbmDeformableMeshGPU*)gpuMeshInstance->getDeformableMesh();

	if (!gpuMeshInstance->getVBODeformPos())
		gpuMeshInstance->initBuffer();
	
	// update blendshapes
	gpuMeshInstance->gpuBlendShape();

	static GLuint queryName = -1;
	if (queryName == -1)
		glGenQueries(1, &queryName);
	// setup transform feedback
	const char* attr[3] = { "deformPos", "deformNormal", "deformTangent" };
	glTransformFeedbackVaryings(skinningShader->getShaderProgram(), 3, attr, GL_SEPARATE_ATTRIBS);
	glLinkProgram(skinningShader->getShaderProgram());	

	glEnableVertexAttribArray(0);
	gpuMesh->getPosVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	gpuMesh->getNormalVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	gpuMesh->getTangentVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
	gpuMesh->getBoneIDVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);
	gpuMesh->getBoneWeightVBO()->VBO()->BindBuffer();
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glEnable(GL_RASTERIZER_DISCARD);
	gpuMeshInstance->updateTransformBuffer();
	std::vector<SrMat>& transBuffer = gpuMeshInstance->getTransformBuffer();
	//float meshScale = 
	SrVec meshScale = meshInstance->getMeshScale();
	
	glUseProgram(skinningShader->getShaderProgram());	
	glUniformMatrix4fv(glGetUniformLocation(skinningShader->getShaderProgram(), "Transform"), transBuffer.size(), true, (GLfloat*)getPtr(transBuffer));
	glUniform1f(glGetUniformLocation(skinningShader->getShaderProgram(), "meshScale"), meshScale[0]);
	// bind transform feedback buffer
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gpuMeshInstance->getVBODeformPos()->VBO()->m_iVBO_ID);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, gpuMeshInstance->getVBODeformNormal()->VBO()->m_iVBO_ID);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, gpuMeshInstance->getVBODeformTangent()->VBO()->m_iVBO_ID);

	
	//glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queryName);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, gpuMeshInstance->_deformPosBuf.size());
	glEndTransformFeedback();
	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);	

	//GLuint PrimitivesWritten = 0;
	//glGetQueryObjectuiv(queryName, GL_QUERY_RESULT, &PrimitivesWritten);
	//SmartBody::util::log("Output transform feedback = %d", PrimitivesWritten);

	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisable(GL_RASTERIZER_DISCARD);
}

void SBRenderer::drawTextureQuadWithDepth(SbmTexture* tex, SbmTexture* depthTex)
{
	if (!tex || !depthTex) return;

	glUseProgram(depthQuadShader->getShaderProgram());
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, tex->getID());

	glActiveTexture(GL_TEXTURE1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, depthTex->getID());

	//drawScreenQuad();
	SBFrameBufferObject::drawScreenQuad();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glEnable(GL_LIGHTING);
}

SBGBuffer::SBGBuffer()
{
	posTex = normalTex = diffuseTex = specularTex = glossyTex = depthTex = NULL;
}

void SBGBuffer::initBuffer(int w, int h)
{
	SmartBody::util::log("Init SBGBuffer");
	initFBO("gbuffer_fbo");

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	posTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_posTex");
	normalTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_normalTex");
	diffuseTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_diffuseTex");
	specularTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_specularTex");
	glossyTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_glossyTex");
	depthTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_depthTex");

	createBufferTex(w, h, true);	
	attachTexture(posTex, GL_COLOR_ATTACHMENT0);
	attachTexture(normalTex, GL_COLOR_ATTACHMENT1);
	attachTexture(diffuseTex, GL_COLOR_ATTACHMENT2);
	attachTexture(specularTex, GL_COLOR_ATTACHMENT3);
	attachTexture(glossyTex, GL_COLOR_ATTACHMENT4);
	attachTexture(depthTex, GL_DEPTH_ATTACHMENT);
	setDrawBufferDefault();	
}

void SBGBuffer::resize(int w, int h)
{
	//SmartBody::util::log("Resize SBGBuffer");
	createBufferTex(w, h, false);
}


void SBGBuffer::createBufferTex(int w, int h, bool rebuild)
{
	if (!posTex)
		return;
	
	posTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	normalTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	diffuseTex->createEmptyTexture(w, h, 4, GL_FLOAT);
	specularTex->createEmptyTexture(w, h, 4, GL_FLOAT);
	glossyTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	depthTex->createEmptyTexture(w, h, 1, GL_FLOAT);

	posTex->buildTexture(false, rebuild);
	normalTex->buildTexture(false, rebuild);
	diffuseTex->buildTexture(false, rebuild);
	specularTex->buildTexture(false, rebuild);
	glossyTex->buildTexture(false, rebuild);
	depthTex->buildTexture(false, rebuild);
}

