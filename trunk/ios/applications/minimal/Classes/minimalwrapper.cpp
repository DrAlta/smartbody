//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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

using namespace boost::filesystem;

#if __cplusplus
extern "C"
{
#endif

void SBInitialize(const char* mediapath)
{
    printf("media path %s\n", mediapath);
    XMLPlatformUtils::Initialize();
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    initPython("../../Python26/Libs");
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->setupTimer();
    printf("Timer setup!\n");
    scene->setMediaPath(mediapath);
	scene->addAssetPath("seq", "sbm-common/scripts");
	scene->runScript("default-init.py");
    printf("Run default-init.py\n");
}
    
void getBoneData()
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SmartBody::SBSkeleton* sk = scene->getCharacter("brad")->getSkeleton();
    SmartBody::SBSkeleton* sk1 = scene->getCharacter("doctor")->getSkeleton();
    sk->update_global_matrices(); 
    sk1->update_global_matrices();
    std::map<int,int> indexMap;
    numJoints = sk->joints().size();
    for (int i=0;i<sk->joints().size();i++)
    {
        SkJoint* joint = sk->joints()[i];
        SkJoint* joint1 = sk1->joints()[i];
        SrVec pos = joint->gmat().get_translation();
        SrVec pos1 = joint1->gmat().get_translation();
        jointPos[i * 3 + 0] = pos.x;
        jointPos[i * 3 + 1] = pos.y;
        jointPos[i * 3 + 2] = pos.z;
        jointPos1[i * 3 + 0] = pos1.x;
        jointPos1[i * 3 + 1] = pos1.y;
        jointPos1[i * 3 + 2] = pos1.z;        
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
}
    
void SBMUpdateX(float t)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->update();
    scene->update();
}
    
void SBMExecuteCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->command(command);
}
    
void SBMExecutePythonCmd(const char* command)
{
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->run(command);
}
    
float* rotatePoint(float* pointF, float* originF, float* directionF, float angle)
{
    SrVec point = SrVec(pointF[0], pointF[1], pointF[2]);
    SrVec origin = SrVec(originF[0], originF[1], originF[2]);
    SrVec direction = SrVec(directionF[0], directionF[1], directionF[2]);
    SrVec v = direction;
    SrVec o = origin;
    SrVec p = point;
    float c = cos(angle);
    float s = sin(angle);
    float C = 1.0f - c;
    
    SrMat mat;
    mat.e11() = v[0] * v[0] * C + c;
    mat.e12() = v[0] * v[1] * C - v[2] * s;
    mat.e13() = v[0] * v[2] * C + v[1] * s;
    mat.e21() = v[1] * v[0] * C + v[2] * s;
    mat.e22() = v[1] * v[1] * C + c;
    mat.e23() = v[1] * v[2] * C - v[0] * s;
    mat.e31() = v[2] * v[0] * C - v[1] * s;
    mat.e32() = v[2] * v[1] * C + v[0] * s;
    mat.e33() = v[2] * v[2] * C + c;
    
    mat.transpose();
    
    SrVec result = origin + mat * (point - origin);
    return result;
}

void getCamera(float x, float y, float prevX, float prevY, float curX, float curY, int mode)
{
    static SrCamera _camera;
    if (mode == -1)
    {
        _camera.setUpVector(SrVec(0, 1, 0));
        _camera.setCenter(0, 92, 0);
        _camera.setEye(0, 166, 300);
        _camera.setScale(1);
    }   
    if (mode == 0)  // dollying
    {
        printf("camera dollying, %f, %f\n", x, y);
        float amount = x / 100.0f;
        SrVec cameraPos(_camera.getEye());
        SrVec targetPos(_camera.getCenter());
        SrVec diff = targetPos - cameraPos;
        float distance = diff.len();
        diff.normalize();
        
        if (amount >= distance)
            amount = distance - .000001f;
        
        SrVec diffVector = diff;
        SrVec adjustment = diffVector * distance * amount;
        cameraPos += adjustment;
        SrVec oldEyePos = _camera.getEye();
        _camera.setEye(cameraPos.x, cameraPos.y, cameraPos.z);
        SrVec cameraDiff = _camera.getEye() - oldEyePos;
        SrVec temp = _camera.getCenter() + cameraDiff;
         _camera.setCenter(temp.x, temp.y, temp.z);
     }
    
    if (mode == 1)  // camera rotation
    {   
 		float deltaX = -x / swidth;
		float deltaY = -y / sheight;
        deltaX *= 3.0f;
        deltaY *= 3.0f;

		SrVec origUp = _camera.getUpVector();
		SrVec origCenter = _camera.getCenter();
		SrVec origCamera = _camera.getEye();
        
		SrVec dirX = origUp;
		SrVec  dirY;
		dirY.cross(origUp, (origCenter - origCamera));
		dirY /= dirY.len();
        
		SrVec camera = rotatePoint(origCamera, origCenter, dirX, -deltaX * float(M_PI));
		camera = rotatePoint(camera, origCenter, dirY, deltaY * float(M_PI));
        
		_camera.setEye(camera.x, camera.y, camera.z);
    }
        
    
    SrMat mat;
    _camera.get_perspective_mat(mat);
    SrMat mat1;
    _camera.get_view_mat(mat1);
    for (int i = 0; i < 16; i++)
    {
        projection[i] = mat.get(i);
        modelview[i] = mat1.get(i);
    }
}
    
#if __cplusplus
}
#endif