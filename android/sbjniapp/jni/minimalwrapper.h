//
//  test.h
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __MINIMAL_WRAPPER__
#define __MINIMAL_WRAPPER__
#if __cplusplus
extern "C"
{
#endif
    //int cameraMode;

	void SBInitVHMSGConnection();
	void SBCloseVHMSGConnection();
    void SBInitialize(const char* path);
	void SBInitScene(const char* initScriptName);
    void SBSetupDrawing(int w, int h);
    void SBDrawFrame(int w, int h);
    void SBDrawCharacters();
	void SBDrawBackground();
    void drawLights();
    void SBUpdate(float t);
    void SBExecuteCmd(const char* command);
    void SBExecutePythonCmd(const char* command);
    void SBCameraOperation(float dx, float dy);
#if __cplusplus
}
#endif
#endif
