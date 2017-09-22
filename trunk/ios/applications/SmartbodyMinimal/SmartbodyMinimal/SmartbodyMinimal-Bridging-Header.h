//
//  SmartbodyMinimal-Bridging-Header.h.h
//  sbminimal
//
//  Created by Anton Leuski on 9/9/17.
//

#ifndef SmartbodyMinimal_Bridging_Header_h_h
#define SmartbodyMinimal_Bridging_Header_h_h

void SBIOSInitialize(const char* path, void (*listener)(const char*));
void SBIOSSetupDrawing(int w, int h);
void SBIOSDrawFrame(void);
void SBIOSReloadTexture(void);
void SBUpdate(float t);
void SBExecuteCmd(const char* command);
void SBExecutePythonCmd(const char* command);
void SBCameraOperation(float dx, float dy, int cameraMode);

const char* SBGetStringAttribute(const char* inAttributeName);
int SBGetBoolAttribute(const char* inAttributeName);
double SBGetDoubleAttribute(const char* inAttributeName);
int SBGetIntAttribute(const char* inAttributeName);

#endif /* SmartbodyMinimal_Bridging_Header_h_h */
