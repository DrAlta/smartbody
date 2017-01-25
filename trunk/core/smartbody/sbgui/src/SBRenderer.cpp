#include "vhcl.h"
#if !defined(__FLASHPLAYER__) && !defined(__ANDROID__) && !defined(EMSCRIPTEN)
#include "external/glew/glew.h"
#endif

#include "SBRenderer.h"
#include <sb/SBObject.h>
#include <sb/SBAttribute.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPawn.h>
#include <sb/SBCharacter.h>
#include <sr/sr_camera.h>
#include <sr/sr_mat.h>
#include <sr/sr_random.h>


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
	else if (gbufferDebug == "texCoordTex")
		debugTex = gbuffer.texCoordTex;
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

	SBFrameBufferObject::drawTextureQuad(debugTex);
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
		LOG("Error ! Can't find tex name = %s", texName.c_str());		
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
		LOG("Kernel %d, vec = %s", i, ssaoKernel[i].toString().c_str());
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

void SBRenderer::drawTestDeferred(std::vector<SrLight>& lights)
{
	gbuffer.bindFBO();
	glUseProgram(gbufferShader->getShaderProgram());
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = SmartBody::SBScene::getScene()->getActiveCamera();
	// if there is no active camera, then only show the blank screen
	if (!cam)
		return;	

	SrMat mat;
	//----- Set Projection ----------------------------------------------
	cam->setAspectRatio((float)width / (float)height);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(cam->get_perspective_mat(mat));

	//----- Set Visualisation -------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrix(cam->get_view_mat(mat));
	glScalef(cam->getScale(), cam->getScale(), cam->getScale());

	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	std::vector<std::string> pawnNames = scene->getPawnNames();
	for (unsigned int i = 0; i < pawnNames.size(); i++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(pawnNames[i]);
		DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
		float alphaThreshold = (float)pawn->getDoubleAttribute("alphaThreshold");
		glUniform1f(glGetUniformLocation(gbufferShader->getShaderProgram(), "alphaThreshold"), alphaThreshold);
		if (meshInstance)
		{
			if (!meshInstance->isStaticMesh())
				meshInstance->updateFast();
			renderMesh(meshInstance);
		}
	}
	glUseProgram(0);
	gbuffer.unbindFBO();

	std::string gbufferDebug = SmartBody::SBScene::getScene()->getStringAttribute("Renderer.gbufferDebug");
	drawTestSSAO();
	//drawLightPass(lights);
	drawIBLPass();
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



void SBRenderer::drawIBLPass()
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

	glUniformMatrix4fv(glGetUniformLocation(iblShader->getShaderProgram(), "uViewToWorldMatrix"), 1, GL_FALSE, (GLfloat*)&invViewMat);
	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "uTanHalfFov"), tanHalfFov);
	glUniform1f(glGetUniformLocation(iblShader->getShaderProgram(), "uAspectRatio"), aspectRatio);

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
	//LOG("Proj Mat = %s", projMatrix.toString().c_str());
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
		LOG("SBRenderer::renderMesh ERR: no deformable mesh found!");
		return; // no deformable mesh
	}


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

	std::vector<SbmSubMesh*>& subMeshList = mesh->subMeshList;	
	for (unsigned int i = 0; i < subMeshList.size(); i++)
	{
		SbmSubMesh* subMesh = subMeshList[i];		
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
		SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, subMesh->texName.c_str());
		if (tex)
		{			
			glEnable(GL_TEXTURE_2D);
			GLint activeTexture = -1;
			glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
			if (activeTexture != GL_TEXTURE0)
				glActiveTexture(GL_TEXTURE0);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, tex->getID());			
		}
		glDrawElements(GL_TRIANGLES, subMesh->triBuf.size() * 3, GL_UNSIGNED_INT, &subMesh->triBuf[0]);
	}
}

void SBRenderer::registerGUI()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	std::vector<std::string> GBufferOptions;
	GBufferOptions.push_back("none");
	GBufferOptions.push_back("posTex");
	GBufferOptions.push_back("normalTex");
	GBufferOptions.push_back("diffuseTex");
	GBufferOptions.push_back("texCoordTex");
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
}



SBGBuffer::SBGBuffer()
{
	posTex = normalTex = diffuseTex = texCoordTex = depthTex = NULL;
}

void SBGBuffer::initBuffer(int w, int h)
{
	LOG("Init SBGBuffer");
	initFBO("gbuffer_fbo");

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	posTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_posTex");
	normalTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_normalTex");
	diffuseTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_diffuseTex");
	texCoordTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_texCoordTex");
	depthTex = texManager.createTexture(SbmTextureManager::TEXTURE_RENDER_TARGET, "gbuffer_depthTex");

	createBufferTex(w, h, true);	
	attachTexture(posTex, GL_COLOR_ATTACHMENT0);
	attachTexture(normalTex, GL_COLOR_ATTACHMENT1);
	attachTexture(diffuseTex, GL_COLOR_ATTACHMENT2);
	attachTexture(texCoordTex, GL_COLOR_ATTACHMENT3);
	attachTexture(depthTex, GL_DEPTH_ATTACHMENT);
	setDrawBufferDefault();	
}

void SBGBuffer::resize(int w, int h)
{
	LOG("Resize SBGBuffer");
	createBufferTex(w, h, false);
}


void SBGBuffer::createBufferTex(int w, int h, bool rebuild)
{
	if (!posTex)
		return;
	
	posTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	normalTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	diffuseTex->createEmptyTexture(w, h, 4, GL_FLOAT);
	texCoordTex->createEmptyTexture(w, h, 3, GL_FLOAT);
	depthTex->createEmptyTexture(w, h, 1, GL_FLOAT);

	posTex->buildTexture(false, rebuild);
	normalTex->buildTexture(false, rebuild);
	diffuseTex->buildTexture(false, rebuild);
	texCoordTex->buildTexture(false, rebuild);
	depthTex->buildTexture(false, rebuild);
}

