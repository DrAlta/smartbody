//
//  test.h
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include <iostream>
void mcu_register_callbacks();
void MCUInitialize();
void initConnection(const char* serverName, const char* portName);
void endConnection();
void SBInitialize(const char* mediaPath);
void SBUpdateX(float t);
void SBExecuteCmd(const char* command);
void getCharacterWo(const char* name, float& x, float& y, float& z, float& qw, float& qx, float& qy, float& qz);
void getJointRotation(const char* charname, const char* jointname, float& q, float& x, float& y, float& z);
void getJointInfo(const char* charname, float* positions, float* bones, std::string* jnames, int n);
