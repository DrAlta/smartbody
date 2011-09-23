//
//  test.h
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#if __cplusplus
extern "C"
{
#endif
    float swidth;
    float sheight;
    int cameraMode;
    float jointPos[600];
    unsigned short boneIdx[400];
    float jointPos1[600];
    int numJoints;
    void mcu_register_callbacks();
    void MCUInitialize();
    void SBMInitialize(const char* mediaPath);
    void SBMUpdateX(float t);
    void SBMExecuteCmd(const char* command);
    void SBMExecutePythonCmd(const char* command);
    float SBMGetCharacterWo(const char* character);
    void getBoneData();
    void getCamera(float x, float y, float prevX, float prevY, float curX, float curY, int mode);
    float* rotatePoint(float* point, float* origin, float* direction, float angle);
    float projection[16];
    float modelview[16];
#if __cplusplus
}
#endif