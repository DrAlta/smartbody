//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "vhcl.h"
#include <vhmsg-tt.h>
#include "SBWrapper.h"

#include <sb/SBScene.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBPython.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAttribute.h>
#include <sb/SBSceneListener.h>
#include <sb/SBAssetManager.h>

#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmTexture.h>
#include <sbm/xercesc_utils.hpp>

#include <sr/sr_camera.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_gl.h>
#include <sr/sr_light.h>
#include <sr/sr_camera.h>
#include <sr/sr_gl_render_funcs.h>
#include <sr/sr_euler.h>
#include <sbm/gwiz_math.h>


#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include "AppListener.h"
#include "Shader.h"

#if defined(ANDROID_BUILD)
//#include <EGL/egl.h>
//#include <GLES/gl.h>
#include <GLES3/gl3.h>
//#include "wes.h"
//#include "wes_gl.h"
#elif defined(IPHONE_BUILD)
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif

using namespace boost::filesystem;
using namespace SmartBody;
/*
    Known issue for smartbody ios:
    - vhcl log OnMessage doesn't seem to work inside static libraries, it only works on this application level, need to look more into it. Now I used printf before OnMessage function inside vhcl_log.cpp
 */


#if __cplusplus
extern "C"
{
#endif

static SrCamera* cameraReset;
static vhcl::Log::StdoutListener listener;
#if USE_GL_FIXED_PIPELINE
std::vector<SrLight> _lights;
#endif

void setupLights()
{
	#if USE_GL_FIXED_PIPELINE
    _lights.clear();
	int numLightsInScene = 0;
	const std::vector<std::string>& pawnNames =  SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		 iter != pawnNames.end();
         iter++)
	{
		SmartBody::SBPawn* sbpawn = SmartBody::SBScene::getScene()->getPawn(*iter);
		const std::string& name = sbpawn->getName();
		if (name.find("light") == 0)
		{
			numLightsInScene++;
			SmartBody::BoolAttribute* enabledAttr = dynamic_cast<SmartBody::BoolAttribute*>(sbpawn->getAttribute("enabled"));
			if (enabledAttr && !enabledAttr->getValue())
			{
				continue;
			}
			SrLight light;
            
			light.position = sbpawn->getPosition();
            
			SrQuat orientation = sbpawn->getOrientation();
			SmartBody::BoolAttribute* directionalAttr = dynamic_cast<SmartBody::BoolAttribute*>(sbpawn->getAttribute("lightIsDirectional"));
			if (directionalAttr)
			{
				light.directional = directionalAttr->getValue();
			}
			else
			{
				light.directional = true;
			}
			if (light.directional)
			{
				light.position = -SrVec(0, 1, 0) * orientation;
			}
			
			SmartBody::Vec3Attribute* diffuseColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightDiffuseColor"));
			if (diffuseColorAttr)
			{
				const SrVec& color = diffuseColorAttr->getValue();
				light.diffuse = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.diffuse = SrColor( 1.0f, 0.95f, 0.8f );
			}
			SmartBody::Vec3Attribute* ambientColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightAmbientColor"));
			if (ambientColorAttr)
			{
				const SrVec& color = ambientColorAttr->getValue();
				light.ambient = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.ambient = SrColor( 0.0f, 0.0f, 0.0f );
			}
			SmartBody::Vec3Attribute* specularColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightSpecularColor"));
			if (specularColorAttr)
			{
				const SrVec& color = specularColorAttr->getValue();
				light.specular = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.specular = SrColor( 0.0f, 0.0f, 0.0f );
			}
			SmartBody::DoubleAttribute* spotExponentAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightSpotExponent"));
			if (spotExponentAttr)
			{
				light.spot_exponent = (float) spotExponentAttr->getValue();
			}
			else
			{
				light.spot_exponent = 0.0f;
			}
			SmartBody::Vec3Attribute* spotDirectionAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightSpotDirection"));
			if (spotDirectionAttr)
			{
				const SrVec& direction = spotDirectionAttr->getValue();
				light.spot_direction = direction;
				// override the explicit direction with orientation
				light.spot_direction = SrVec(0, 1, 0) * orientation;
			}
			else
			{
				light.spot_direction = SrVec( 0.0f, 0.0f, -1.0f );
			}
			SmartBody::DoubleAttribute* spotCutOffAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightSpotCutoff"));
			if (spotExponentAttr)
			{
				if (light.directional)
					light.spot_cutoff = 180.0f;
				else
					light.spot_cutoff = (float) spotCutOffAttr->getValue();
                
			}
			else
			{
				light.spot_cutoff = 180.0f;
			}
			SmartBody::DoubleAttribute* constantAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightConstantAttenuation"));
			if (constantAttentuationAttr)
			{
				light.constant_attenuation = (float) constantAttentuationAttr->getValue();
			}
			else
			{
				light.constant_attenuation = 1.0f;
			}
			SmartBody::DoubleAttribute* linearAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightLinearAttenuation"));
			if (linearAttentuationAttr)
			{
				light.linear_attenuation = (float) linearAttentuationAttr->getValue();
			}
			else
			{
				light.linear_attenuation = 0.0f;
			}
			SmartBody::DoubleAttribute* quadraticAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightQuadraticAttenuation"));
			if (quadraticAttentuationAttr)
			{
				light.quadratic_attenuation = (float) quadraticAttentuationAttr->getValue();
			}
			else
			{
				light.quadratic_attenuation = 0.0f;
			}
			
			_lights.push_back(light);
		}
	}

    //LOG("light size = %d\n",_lights.size());
	
	if (_lights.size() == 0 && numLightsInScene == 0)
	{
		SrLight light;		
		light.directional = true;
		light.diffuse = SrColor( 1.0f, 0.0f, 0.0f );
		SrMat mat;
		sr_euler_mat_xyz (mat, SR_TORAD(0), SR_TORAD(0), SR_TORAD(135	));
		SrQuat orientation(mat);
		SrVec up(0,1,0);
		SrVec lightDirection = -up * orientation;
		light.position = SrVec( lightDirection.x, lightDirection.y, lightDirection.z);
	//	light.constant_attenuation = 1.0f/cam.scale;
		light.constant_attenuation = 1.0f;
		_lights.push_back(light);

		SrLight light2 = light;
		light2.directional = true;
		light2.diffuse = SrColor( 0.8f, 0.0f, 0.0f );
		sr_euler_mat_xyz (mat, SR_TORAD(0), SR_TORAD(0), SR_TORAD(-135));
		SrQuat orientation2(mat);
		lightDirection = -up * orientation2;
		light2.position = SrVec( lightDirection.x, lightDirection.y, lightDirection.z);
	//	light2.constant_attenuation = 1.0f;
	//	light2.linear_attenuation = 2.0f;
		_lights.push_back(light2);
	}
    
    
    myGLEnable(GL_LIGHTING);
	int maxLight = -1;
	for (size_t x = 0; x < _lights.size(); x++)
	{
		glLight ( x, _lights[x] );
		maxLight++;
	}
    
	if (maxLight < 0)
	{
		myGLDisable(GL_LIGHT0);
	}
	if (maxLight < 1)
	{
		myGLDisable(GL_LIGHT1);
	}
	if (maxLight < 2)
	{
		myGLDisable(GL_LIGHT2);
	}
	if (maxLight < 3)
	{
		myGLDisable(GL_LIGHT3);
	}
	if (maxLight < 4)
	{
		myGLDisable(GL_LIGHT4);
	}
	if (maxLight < 5)
	{
		myGLDisable(GL_LIGHT5);
	}
    
	if (maxLight > 0)
	{
		myGLEnable(GL_LIGHT0);
	}
	if (maxLight > 1)
	{
		myGLEnable(GL_LIGHT1);
	}
	if (maxLight > 2)
	{
		myGLEnable(GL_LIGHT2);
	}
	if (maxLight > 3)
	{
		myGLEnable(GL_LIGHT3);
	}
	if (maxLight > 4)
	{
		myGLEnable(GL_LIGHT4);
	}
	if (maxLight > 5)
	{
		myGLEnable(GL_LIGHT5);
	}
	#endif
}

void SBSetupDrawing(int w, int h, ESContext *esContext)
{   
	glViewport(0, 0, w, h);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	//myGLEnable(GL_CULL_FACE);
	//glShadeModel(GL_SMOOTH);
	//myGLDisable(GL_DEPTH_TEST);
}

void SBSetup(const char* mediapath, const char* setupScript)
{
	vhcl::Log::g_log.AddListener(&listener);
	XMLPlatformUtils::Initialize();
	LOG("media path%s", mediapath);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string mpath = mediapath;
	initPython(mpath + "pythonLib_27/Lib");
	scene->setMediaPath(mediapath);
	scene->addAssetPath("script", ".");
	scene->runScript(setupScript);
}
    
void SBInitialize()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	AppListener* appListener = new AppListener();
	scene->addSceneListener(appListener);
	SrCamera& cam = *scene->createCamera("activeCamera");
	cam.init();
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->setupTimer();
    scene->createBoolAttribute("enableAlphaBlend", true, true, "", 5, false, false, false, "enable alpha blending?");
}

void SBInitGraphics(ESContext *esContext) {
	LOG("Before shaderInit");
	shaderInit(esContext);
	LOG("After shaderInit");
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();	
	scene->createIntAttribute("renderer.fboTexID",- 1, true,"",40,false,false,false,"texture id attached with the current fbo. For fast video recording.");
	scene->setIntAttribute("renderer.fboTexID", esContext->fboTexID);
	LOG("fboTexID = %d", esContext->fboTexID);
}


void SBInitScene( const char* initScriptName )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->addAssetPath("script", ".");
	scene->runScript(initScriptName);
	SrCamera& cam = *scene->getCamera("activeCamera");
	cameraReset = new SrCamera(cam);
}

void SBDrawFrame_ES20(int width, int height, ESContext *esContext, SrMat eyeView) {
	//LOG("draw!");
	static bool initShader = false;
	if (!initShader)
	{
		LOG("Shader not initialized, run SBInitGraphics.");
		SBInitGraphics(esContext);
        initShader = true;
	}


	UserData *userData = (UserData*) esContext->userData;
	ShapeData *shapeData = (ShapeData*) esContext->shapeData;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	// setup textures
	SbmTextureManager& texm = SbmTextureManager::singleton();
	texm.updateTexture();


	bool useRenderTarget = false;
	if (useRenderTarget)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, esContext->fboID);
	SrCamera& cam = *scene->getActiveCamera();
	// clear background
	//glClearColor( 0.0f,0.0f,0.0f,1.0f );
	//glClearColor ( 0.6f, 0.6f, 0.6f, 1.0f );
	SBSetupDrawing(width, height, esContext);
	glClearColor(0.33f, 0.78f, 0.95f, 1.f);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// draw background
	SBDrawBackground(esContext);
	
	//cam.print();
	// setup view
	cam.setAspectRatio(float(width) / float(height));
	SrMat matPerspective, matModelView;
	cam.get_perspective_mat(matPerspective);
	cam.get_view_mat(matModelView);
	matModelView *= cam.getScale();
	matModelView = matModelView*eyeView;
	glViewport( 0, 0, width, height);
	SrMat matMVP = matModelView * matPerspective;
	//use the shape program object
	//LOG("Before use program");
	glUseProgram (shapeData->programObject );
	//LOG("After use program");
	glUniformMatrix4fv(shapeData->mvpLoc, 1, GL_FALSE, (GLfloat *)matMVP.pt(0));
	glUniformMatrix4fv(shapeData->mvLoc, 1, GL_FALSE, (GLfloat *)matModelView.pt(0));
	//LOG("Before draw grid");
	//drawGrid(esContext);
	//LOG("Before draw pawns");
	SBDrawPawns(esContext);
	// Use the program object
	//LOG("Before userData useProgram");
	glUseProgram ( userData->programObject );
	glUniformMatrix4fv(userData->mvLoc, 1, GL_FALSE, (GLfloat *)matModelView.pt(0));
	glUniformMatrix4fv(userData->mvpLoc, 1, GL_FALSE, (GLfloat *)matMVP.pt(0));
	//LOG("Before SBDrawCharacters_ES20");
	SBDrawCharacters_ES20(esContext);
	//draw lights
	//LOG("After SBDrawCharacters_ES20");
	drawLights(esContext);
	if (useRenderTarget)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		/*
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawBackgroundTexID(esContext->fboTexID, esContext);		
		*/
		SBDrawFBOTex_ES20(width, height, esContext, eyeView);
	}
	//LOG("After drawLights");
}

void SBDrawFBOTex_ES20(int w, int h, ESContext *esContext, SrMat eyeView)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport( 0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawBackgroundTexID(esContext->fboTexID, esContext);
}

void SBDrawCharacters_ES20(ESContext *esContext) {
	static bool meshUpdated = false;
	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	//printf("draw pawns::numOfPawns: %d\n", pawns.size());
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		 pawnIter != pawns.end();
		 pawnIter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		//LOG("draw pawn %s", pawn->getName().c_str());
		//DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
		DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
		if(meshInstance)
		{	/*
			if (!meshInstance->isStaticMesh())
			{
				//LOG("pawn %s is deformable mesh", pawn->getName().c_str());
				//meshInstance->blendShapeStaticMesh();
				meshInstance->setVisibility(1);
				//if (!meshInstance->isStaticMesh() && !meshUpdated)
				meshInstance->update();
			}
			else
			{
				//LOG("pawn %s is static mesh", pawn->getName().c_str());
			}
			*/
			//if(pawn->dMeshInstance_p->getDeformableMesh())
			drawMeshStatic(meshInstance, esContext, false);
			//else
             //   drawSkeleton(esContext);
		}
	}
	meshUpdated = true;
}

void SBDrawPawns(ESContext *esContext) {

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const std::vector<std::string>& pawnNames = scene->getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		 iter != pawnNames.end();
		 iter++)
	{

		SmartBody::SBPawn *pawn = scene->getPawn((*iter));

		if (!pawn->getSkeleton())
			continue;

		SmartBody::SBCharacter *character = dynamic_cast<SmartBody::SBCharacter*>(pawn);
		if (character)
			continue;

		const bool isVisible = pawn->getBoolAttribute("visible");
		if (!isVisible)
			continue;
		SrMat gmat = pawn->get_world_offset();//pawn->get_world_offset_joint()->gmat();
		if (pawn->dMeshInstance_p) {
			pawn->dMeshInstance_p->setVisibility(1);
			if (pawn->dMeshInstance_p->getDeformableMesh())
				drawMeshStatic(pawn->dMeshInstance_p, esContext, false);
		}
	}
}

void SBDrawGrid(ESContext *esContext) {
    drawGrid(esContext);
}


    
void SBDrawFrame(int width, int height, SrMat eyeViewMat)
{
#if USE_GL_FIXED_PIPELINE
	static bool initWes = false;
	if (!initWes)
	{
		wes_init(NULL);
		initWes = true;
	}

	
	//LOG("Getting Scene");
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if(!scene)
		return;
	//LOG("Got Scene");

#if 0
	SmartBody::SBAttribute* attribute = scene->getAttribute("appStatus");
	if (!attribute)
		return;

	SmartBody::StringAttribute* strattribute = dynamic_cast<SmartBody::StringAttribute*>(attribute);
	if (!strattribute)
		return;

	if (strattribute->getValue() != "finished")
		return;

	if (scene->getSimulationManager()->getTime() < .033)
		return;
#endif

    SrCamera& cam = *scene->getActiveCamera();
    
	// clear background
	glViewport( 0, 0, width, height);
	glClearColor(0.4f,0.4f,0.4f,1);
	myGLEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//LOG("Preparing to DrawCharacter");
    // setup view
    cam.setAspectRatio(float(width) / float(height));
	SrMat mat;
	glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
	cam.get_perspective_mat(mat);
	glLoadMatrixf ( (float*) mat);
	glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity();
	cam.get_view_mat(mat);
	mat = mat*eyeViewMat;
	glLoadMatrixf ( (float*) mat);
	glScalef ( cam.getScale(), cam.getScale(), cam.getScale());

	//LOG("Set camera");
#if 1
    // draw lights
    setupLights();
    
    // update texture
    myGLEnable(GL_TEXTURE_2D);
    SbmTextureManager& texm = SbmTextureManager::singleton();
    texm.updateTexture();
    
    /*
    // draw a ground plane
	myGLDisable(GL_LIGHTING);
	float planeSize  = 300.f;
	SrVec quad[4] = { SrVec(planeSize, 0.f, planeSize), SrVec(-planeSize, 0.f, planeSize), SrVec(-planeSize,0.f,-planeSize), SrVec(planeSize, 0.f, -planeSize) };
	SrVec quadN[4] = { SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f) };
	GLfloat quadColor[16] = { 0.2f,0.2f, 0.2f, 1.f , 0.3f,0.3f,0.3f, 1.f, 0.5f,0.5f,0.5f,1.f, 0.25f,0.25f,0.25f,1.f };
	unsigned short indices[] = {0,1,2, 0,2,3};
	glShadeModel(GL_SMOOTH);
	myGLDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLfloat*)&quad[0]);
	glColorPointer(4, GL_FLOAT, 0, quadColor);
	glNormalPointer(GL_FLOAT, 0, (GLfloat*)&quadN[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	myGLEnable(GL_LIGHTING);
     */
    
    // draw characters
    //LOG("Drawing Character");
	SBDrawBackground();
    SBDrawCharacters();
#endif
#endif
}
    

void SBDrawBackground(ESContext* esContext)
{
#if 0
	SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE,"background_img");
	if (!tex)
	{
		//LOG("cannot find texture image .....");
		return; // no background image
	}

	//LOG("texture image id = %d", tex->getID());
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity ();

	glMatrixMode (GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1);

	
	myGLEnable(GL_TEXTURE_2D);	
	glBindTexture(GL_TEXTURE_2D, tex->getID());	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texIDs[0]);
	//LOG("before bindAttrib");
	float z_max = -(1.f - gwiz::epsilon10());
	SrVec4 quad[4] = { SrVec4(-1.0, 1.0f, z_max, 1.f), SrVec4(-1.0f, -1.0f, z_max, 1.f), SrVec4(1.0f, -1.0f, z_max, 1.f), SrVec4(1.0f, 1.0f, z_max, 1.f) };
	SrVec4 quadT[4] = { SrVec4(0.f, 1.f, 0.f, 0.f), SrVec4(0.f, 0.f, 0.f, 0.f), SrVec4(1.f, 0.f, 0.f, 0.f), SrVec4(1.f, 1.f, 0.f, 0.f) };
	unsigned short indices[] = {0,1,2, 0,2,3};
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, (GLfloat*)&quad[0]);  
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  	
	glTexCoordPointer(4, GL_FLOAT, 0, (GLfloat*)&quadT[0]); 

	glDrawElements_wes(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	myGLDisable(GL_TEXTURE_2D);	

	glMatrixMode (GL_PROJECTION);
	glPopMatrix(); 

	glMatrixMode (GL_MODELVIEW);
	glPopMatrix();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
	drawBackground("background_img", esContext);
#endif
}


void SBDrawCharacters()
{

#define DRAW_DEBUG_BONE 0
#if DRAW_DEBUG_BONE   // draw bone figure
    const std::vector<std::string>& chars = SmartBody::SBScene::getScene()->getCharacterNames();
	for (std::vector<std::string>::const_iterator charIter = chars.begin();
         charIter != chars.end();
         charIter++)
	{
		SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter((*charIter));
        float jointPos[600];
        unsigned short boneIdx[400];
        int numJoints;
        SmartBody::SBSkeleton* sk = character->getSkeleton();
        sk->update_global_matrices();
        std::map<int,int> indexMap;
        numJoints = sk->joints().size();
        for (int i=0;i<sk->joints().size();i++)
        {
            SkJoint* joint = sk->joints()[i];
            SrVec pos = joint->gmat().get_translation();
            jointPos[i * 3 + 0] = pos.x;
            jointPos[i * 3 + 1] = pos.y;
            jointPos[i * 3 + 2] = pos.z;
			//LOG("Joint %d, Position = %f %f %f", i, pos.x,pos.y,pos.z);
            indexMap[joint->index()] = i;
            boneIdx[i*2+0] = joint->index();
            if (joint->parent())
                boneIdx[i*2+1] = joint->parent()->index();
            else
                boneIdx[i*2+1] = joint->index();
        }
        for (int i=0;i<sk->joints().size();i++)
        {
            boneIdx[i*2] = indexMap[boneIdx[i*2]];
            boneIdx[i*2+1] = indexMap[boneIdx[i*2+1]];
        }
        //glPointSize(2.0f);
        //glLineWidth(1.0f);
        glColor4f(1, 1, 1, 1);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, (const GLfloat*)&jointPos[0]);
        glDrawArrays(GL_POINTS, 0, numJoints);
        glDrawElements(GL_LINES,numJoints*2,GL_UNSIGNED_SHORT, boneIdx);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
#else   // draw CPU deformable mesh

	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
         pawnIter != pawns.end();
         pawnIter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		if(pawn->dMeshInstance_p)
		{
			pawn->dMeshInstance_p->setVisibility(1);
			//pawn->dMeshInstance_p->blendShapeStaticMesh();
			SrGlRenderFuncs::renderDeformableMesh(pawn->dMeshInstance_p);
		}
	}
#endif
}
    
void SBUpdate(float t)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    if((!scene)  || (!sim))
    	return;
	if (SmartBody::SBScene::getScene()->getVHMsgManager()->isEnable())
	{
		int err = SmartBody::SBScene::getScene()->getVHMsgManager()->poll();
		if( err == CMD_FAILURE )   {
			LOG("ttu_poll ERROR\n" );
		}
	}
    //sim->update();
    bool update_sim = scene->getSimulationManager()->updateTimer();
    if (update_sim)
    {
    	scene->update();

    	// update mesh position
		const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
		//printf("draw pawns::numOfPawns: %d\n", pawns.size());
		for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
			 pawnIter != pawns.end();
			 pawnIter++)
		{
			SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
			//LOG("draw pawn %s", pawn->getName().c_str());
			//DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
			DeformableMeshInstance* meshInstance = pawn->getActiveMesh();
			if(meshInstance)
			{
				if (!meshInstance->isStaticMesh())
				{
					//LOG("pawn %s is deformable mesh", pawn->getName().c_str());
					meshInstance->blendShapeStaticMesh();
					//meshInstance->setVisibility(1);
					//if (!meshInstance->isStaticMesh() && !meshUpdated)
					meshInstance->update();
				}
			}
		}
    }


    
}
    
void SBExecuteCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->command(command);
    LOG("%s\n", command);
}
    
void SBExecutePythonCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->run(command);
    LOG("%s\n", command);
}


void sb_vhmsg_callback( const char *op, const char *args, void * user_data ) {
	// Replace singleton with a user_data pointer
	//if (!mcuInit) return;
	//LOG("VHMSG Callback : op = %s ,args = %s\n",op,args);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	switch( scene->getCommandManager()->execute( op, (char *)args ) ) {
	case CMD_NOT_FOUND:
		LOG("SBM ERR: command NOT FOUND: '%s' + '%s'", op, args );
		break;
	case CMD_FAILURE:
		LOG("SBM ERR: command FAILED: '%s' + '%s'", op, args );
		break;
	}
}


void SBInitVHMSGConnection()
{
	//if (!mcuInit) return;	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const char* serverName = "172.16.33.14";
	const char* scope = "DEFAULT_SCOPE";
	const char* port = "61616";
	int err;
	int openConnection = vhmsg::ttu_open(serverName,scope,port);
	if( openConnection == vhmsg::TTU_SUCCESS )
	{
		vhmsg::ttu_set_client_callback( sb_vhmsg_callback );
		err = vhmsg::ttu_register( "sb" );
		err = vhmsg::ttu_register( "sbm" );
		err = vhmsg::ttu_register( "vrAgentBML" );
		err = vhmsg::ttu_register( "vrExpress" );
		err = vhmsg::ttu_register( "vrSpeak" );
		err = vhmsg::ttu_register( "RemoteSpeechReply" );
		err = vhmsg::ttu_register( "PlaySound" );
		err = vhmsg::ttu_register( "StopSound" );
		err = vhmsg::ttu_register( "CommAPI" );
		err = vhmsg::ttu_register( "object-data" );
		err = vhmsg::ttu_register( "vrAllCall" );
		err = vhmsg::ttu_register( "vrKillComponent" );
		err = vhmsg::ttu_register( "wsp" );
		err = vhmsg::ttu_register( "receiver" );
		scene->getVHMsgManager()->setEnable(true);
		LOG("TTU Open Success : server = %s, scope = %s, port = %s",serverName,scope,port);
	}
	else
	{
		LOG("TTU Open Failed : server = %s, scope = %s, port = %s",serverName,scope,port);
	}
}
    
    
void SBCameraOperation(float dx, float dy)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = scene->getActiveCamera();
    
    float camDx = dx * cam->getAspectRatio() ;
    float camDy = dy * cam->getAspectRatio() ;
    int cameraMode = 1; // hard coded for now
    if (cameraMode == 0) // zoom
    {
		float tmpFov = cam->getFov() + (-camDx + camDy);
		cam->setFov(SR_BOUND(tmpFov, 0.001f, srpi));
    }
    else if (cameraMode == 1) // rotation
    {   
        SrVec origUp = cam->getUpVector();
		SrVec origCenter = cam->getCenter();
		SrVec origCamera = cam->getEye();
		
        SrVec dirX = origUp;
        SrVec dirY;
        dirY.cross(origUp, (origCenter - origCamera));
        dirY /= dirY.len();
        
        SrVec cameraPoint = rotatePoint(origCamera, origCenter, dirX, camDx * float(M_PI));
        //cameraPoint = rotatePoint(origCamera, origCenter, dirX, -camDy * float(M_PI));
        cam->setEye(cameraPoint.x, cameraPoint.y, cameraPoint.z);
    }
    else if (cameraMode == 2) // reset
    {
        cam->copyCamera(cameraReset);
    }
    else if (cameraMode == 3) // dolly
    {
        float amount = camDx - camDy;
        SrVec cameraPos(cam->getEye());
        SrVec targetPos(cam->getCenter());
        SrVec diff = targetPos - cameraPos;
        float distance = diff.len();
        diff.normalize();
        
        if (amount >= distance);
        amount = distance - 0.00001f;
        
        SrVec diffVector = diff;
        SrVec adjustment = diffVector * distance * amount;
        cameraPos += adjustment;
        SrVec oldEyePos = cam->getEye();
        cam->setEye(cameraPos.x, cameraPos.y, cameraPos.z);
        SrVec cameraDiff = cam->getEye() - oldEyePos;
        SrVec tmpCenter = cam->getCenter();
        tmpCenter += cameraDiff;
        cam->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
    }
}
#if __cplusplus
}

void SBCloseVHMSGConnection()
{
	vhmsg::ttu_close();
}


#endif
