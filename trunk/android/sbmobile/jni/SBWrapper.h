//
//  test.h
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __MINIMAL_WRAPPER__
#define __MINIMAL_WRAPPER__
#include <sr/sr_mat.h>
#include "esUtil.h"
#if __cplusplus
extern "C"
{
#endif
    //int cameraMode;

	void SBInitVHMSGConnection();
	void SBCloseVHMSGConnection();
	void SBSetBackground(const char* fileName, const char* textureName, int textureRotate);
    void SBSetup(const char* path, const char* setupScript);
    void SBInitialize();
    void SBInitGraphics(ESContext* esContext);
	void SBInitScene(const char* initScriptName);
    void SBSetupDrawing(int w, int h, ESContext* esContext);

	void SBDrawFrameAR(int w, int h, ESContext *esContext, SrMat modelViewMat, SrMat perspectiveMat);
    void SBDrawFrame_ES20(int w, int h, ESContext *esContext, SrMat eyeView, bool drawAR = false);
    void SBDrawFBOTex_ES20(int w, int h, ESContext *esContext, SrMat eyeView);
	void SBUpdateCharacterGPUSkin();
	void SBDrawCharacterGPUSkin(ESContext *esContext);
    void SBDrawCharacters_ES20(ESContext *esContext);
	void SBDrawBackground(ESContext* esContext);
    void SBDrawPawns(ESContext *esContext);
    void SBDrawGrid(ESContext *esContext);
#if 0
    void SBDrawFrame(int w, int h, SrMat eyeView);
    void SBDrawCharacters();
#endif
	
    void setupLights();
    void SBUpdate(float t);
    void SBExecuteCmd(const char* command);
    void SBExecutePythonCmd(const char* command);
    void SBCameraOperation(float dx, float dy);
#if __cplusplus
}
#endif
#endif

