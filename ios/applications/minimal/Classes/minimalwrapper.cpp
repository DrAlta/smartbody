//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "vhcl.h"
#include "minimalwrapper.h"
#include <sbm/mcontrol_callbacks.h>
#include <sb/SBScene.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <sbm/xercesc_utils.hpp>
#include <sr/sr_camera.h>
#include <sb/SBPython.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmTexture.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_gl.h>
#include <sr/sr_light.h>
#include <sr/sr_camera.h>
#include <sr/sr_gl_render_funcs.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

using namespace boost::filesystem;
/*
    Known issue for smartbody ios:
    - vhcl log OnMessage doesn't seem to work inside static libraries, it only works on this application level, need to look more into it. Now I used printf before OnMessage function inside vhcl_log.cpp
 */


#if __cplusplus
extern "C"
{
#endif

static SrLight light1;
static SrLight light2;
static SrCamera* cameraReset;
static vhcl::Log::StdoutListener listener;
void SBSetupDrawing(int w, int h)
{   
    glViewport(0, 0, w, h);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
    
	// light
	light1.directional = true;
	light1.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light1.position = SrVec( 0.0, 5.0, 10.0f );
	//light1.constant_attenuation = 1.0f;
    
	light2 = light1;
	light2.directional = true;
	light2.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light2.position = SrVec( 0.0, -5.0f, 10.0f );
}
    
void initCharacterScene()
{
 	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	// for some reason couldn't get flash listener working properly, following code is supposed to replace character listener
	std::vector<std::string> charNames = scene->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(charNames[i]);
		if (!character)
			continue;
        
        LOG("init character %s", charNames[i].c_str());
        
		// add skeletons and mesh
		
		if (character->scene_p)
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove(character->scene_p);
			character->scene_p->unref();
			character->scene_p = NULL;
		}
		LOG("Character %s's skeleton added to the scene.", charNames[i].c_str());
        
		if (character->dMesh_p)
		{
			for (size_t i = 0; i < character->dMesh_p->dMeshDynamic_p.size(); i++)
			{
				SmartBody::SBScene::getScene()->getRootGroup()->remove( character->dMesh_p->dMeshDynamic_p[i] );
			}
			delete character->dMesh_p;
			character->dMesh_p = NULL;
		}
		character->dMesh_p = new DeformableMesh();
		character->dMeshInstance_p =  new DeformableMeshInstance();
		character->dMesh_p->setSkeleton(character->getSkeleton());
		character->dMeshInstance_p->setSkeleton(character->getSkeleton());
		LOG("Character %s's deformable mesh reset.", charNames[i].c_str());
        
		std::string dMeshAttrib = character->getStringAttribute("deformableMesh");
		character->setStringAttribute("deformableMesh", dMeshAttrib);
		LOG("Character %s's deformable mesh %s added to the scene.", charNames[i].c_str(), dMeshAttrib.c_str());
	}
}
    
void SBInitialize(const char* mediapath)
{
    vhcl::Log::g_log.AddListener(&listener);
    XMLPlatformUtils::Initialize();
    LOG("media path%s", mediapath);
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SrCamera& cam = *scene->createCamera("activeCamera");
    cam.init();
    initPython("../../Python26/Libs");
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->setupTimer();
    scene->setMediaPath(mediapath);
	scene->addAssetPath("seq", "sbm-common/scripts");
	scene->runScript("default-init.py");
    initCharacterScene();
    
    // store the camera information for resetting
    cameraReset = new SrCamera(cam);
}
    
void SBDrawFrame(int width, int height)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SrCamera& cam = *scene->getActiveCamera();
    
    // lighting
	glLight(0, light1);
	glLight(1, light2);

    
	// clear background
	glClearColor(0.0f,0.0f,0.0f,1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    // setup view
    cam.setAspectRatio(float(width) / float(height));
	SrMat mat;
	glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
	glLoadMatrixf ( (const float*)cam.get_perspective_mat(mat) );
	glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity();
	glLoadMatrixf ( (const float*)cam.get_view_mat(mat) );
	glScalef ( cam.getScale(), cam.getScale(), cam.getScale());
    glViewport( 0, 0, width, height);

    // update texture
    glEnable(GL_TEXTURE_2D);
    SbmTextureManager& texm = SbmTextureManager::singleton();
    texm.updateTexture();
    
    /*
    // draw a ground plane
	glDisable(GL_LIGHTING);
	float planeSize  = 300.f;
	SrVec quad[4] = { SrVec(planeSize, 0.f, planeSize), SrVec(-planeSize, 0.f, planeSize), SrVec(-planeSize,0.f,-planeSize), SrVec(planeSize, 0.f, -planeSize) };
	SrVec quadN[4] = { SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f), SrVec(0.f, 1.f, 0.f) };
	GLfloat quadColor[16] = { 0.2f,0.2f, 0.2f, 1.f , 0.3f,0.3f,0.3f, 1.f, 0.5f,0.5f,0.5f,1.f, 0.25f,0.25f,0.25f,1.f };
	unsigned short indices[] = {0,1,2, 0,2,3};
	glShadeModel(GL_SMOOTH);
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLfloat*)&quad[0]);
	glColorPointer(4, GL_FLOAT, 0, quadColor);
	glNormalPointer(GL_FLOAT, 0, (GLfloat*)&quadN[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_LIGHTING);
     */
    
    // draw characters
    SBDrawCharacters();
}
    
void SBDrawCharacters()
{
    
#if 0   // draw bone figure
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
        glPointSize(2.0f);
        glLineWidth(1.0f);
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
		if(pawn->dMesh_p && pawn->dMeshInstance_p)
		{
			pawn->dMesh_p->set_visibility(1);
			pawn->dMeshInstance_p->setVisibility(1);
			pawn->dMeshInstance_p->update();
			SrGlRenderFuncs::renderDeformableMesh(pawn->dMeshInstance_p);
		}
	}
#endif
}
    
void SBUpdate(float t)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->update();
    scene->update();
}
    
void SBExecuteCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->command(command);
}
    
void SBExecutePythonCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->run(command);
}
    
    
void SBCameraOperation(float dx, float dy)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* cam = scene->getActiveCamera();
    
    float camDx = dx * cam->getAspectRatio() * 0.01f;
    float camDy = dy * cam->getAspectRatio() * 0.01f;
    
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
        
        SrVec cameraPoint = rotatePoint(origCamera, origCenter, dirX, -camDx * float(M_PI));
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
#endif