//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "minimalwrapper.h"
#include <sbm/mcontrol_util.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <sbm/mcontrol_callbacks.h>
#include <sbm/resource_cmds.h>
#include <sbm/sbm_test_cmds.hpp>
#include <sbm/locomotion_cmds.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <sr/sr_camera.h>
#include "vhmsg.h"

XERCES_CPP_NAMESPACE_USE
using namespace boost::filesystem;

#if __cplusplus
extern "C"
{
#endif

void MCUInitialize()
{
    XMLPlatformUtils::Initialize();
    mcuCBHandle& mcu = mcuCBHandle::singleton();
}
        
void SBMInitialize(const char* mediaPath)
{
    SBMExecuteCmd("char brad init common.sk"); 
    SBMExecuteCmd("set character brad world_offset x -35 y 102 h -17");
    SBMExecuteCmd("char doctor init common.sk");
    SBMExecuteCmd("set character doctor world_offset x 35 y 102 h -17");    
    SBMExecuteCmd("bml char brad <body posture=\"HandsAtSide_Motex\"/>");
    SBMExecuteCmd("bml char doctor <body posture=\"HandsAtSide_Motex\"/>");
}
    
void SBMLoad(const char* p)
{
    path pathname(p);
    std::string command = "load skeletons -R " + pathname.root_directory();
    printf("loading command is %s\n", command.c_str());
    SBMExecuteCmd(command.c_str());
}
    
void getBoneData()
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmCharacter* char_p = mcu.getCharacter("brad");
    SkSkeleton* sk = char_p->_skeleton;
    SkSkeleton* sk1 = mcu.getCharacter("doctor")->_skeleton;
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
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    bool updateSim = mcu.update_timer(t);
    vhmsg::ttu_poll();
    if (updateSim)        
        mcu.update();
}
    
void SBMExecuteCmd(const char* command)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.execute((char*) command);
}
    
void SBMExecutePythonCmd(const char* command)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    mcu.executePython((char*) command);    
}
    
float SBMGetCharacterWo(const char* character)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    SbmPawn* c = mcu.getCharacter(character);
    if (!c)
        return -1.0f;
    
    float x, y, z, p, h, r;
    c->get_world_offset(x, y, z, p, h, r);
    return y;
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
        _camera.init();
        _camera.up = SrVec::j;    
        _camera.center.set(0, 92, 0);
        _camera.eye.set(0, 166, 300);
        _camera.scale = 1.0f;
    }   
    if (mode == 0)  // dollying
    {
        printf("camera dollying, %f, %f\n", x, y);
        float amount = x / 100.0f;
        SrVec cameraPos(_camera.eye);
        SrVec targetPos(_camera.center);
        SrVec diff = targetPos - cameraPos;
        float distance = diff.len();
        diff.normalize();
        
        if (amount >= distance)
            amount = distance - .000001f;
        
        SrVec diffVector = diff;
        SrVec adjustment = diffVector * distance * amount;
        cameraPos += adjustment;
        SrVec oldEyePos = _camera.eye;
        _camera.eye = cameraPos;
        SrVec cameraDiff = _camera.eye - oldEyePos;
        _camera.center += cameraDiff;
     }
    
    if (mode == 1)  // camera rotation
    {   
 		float deltaX = -x / swidth;
		float deltaY = -y / sheight;
        deltaX *= 3.0f;
        deltaY *= 3.0f;

		SrVec origUp = _camera.up;
		SrVec origCenter = _camera.center;
		SrVec origCamera = _camera.eye;
        
		SrVec dirX = origUp;
		SrVec  dirY;
		dirY.cross(origUp, (origCenter - origCamera));
		dirY /= dirY.len();
        
		SrVec camera = rotatePoint(origCamera, origCenter, dirX, -deltaX * float(M_PI));
		camera = rotatePoint(camera, origCenter, dirY, deltaY * float(M_PI));
        
		_camera.eye = camera;    
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