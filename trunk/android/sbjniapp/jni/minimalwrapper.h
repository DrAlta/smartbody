//
//  test.h
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __MINIMAL_WRAPPER__
#define __MINIMAL_WRAPPER__
#include <sb/SBSceneListener.h>

class AppListener : public SmartBody::SBSceneListener, public SmartBody::SBObserver
{
   public:
	  AppListener();
	  ~AppListener();

      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
      virtual void OnCharacterDelete( const std::string & name );
	  virtual void OnCharacterUpdate( const std::string & name );
      virtual void OnPawnCreate( const std::string & name );
      virtual void OnPawnDelete( const std::string & name );

	  virtual void OnSimulationUpdate();

	  virtual void notify(SmartBody::SBSubject* subject);
};

#if __cplusplus
extern "C"
{
#endif
    //int cameraMode;
    void SBInitialize(const char* path);
    void SBSetupDrawing(int w, int h);
    void SBDrawFrame(int w, int h);
    void SBDrawCharacters();
    void drawLights();
    void SBUpdate(float t);
    void SBExecuteCmd(const char* command);
    void SBExecutePythonCmd(const char* command);
    void SBCameraOperation(float dx, float dy);
#if __cplusplus
}
#endif
#endif
