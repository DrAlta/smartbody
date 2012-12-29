//
//  test.cpp
//  sbmwrapper
//
//  Created by Yuyu Xu on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "test.h"
#include <SB/SBScene.h>
#include <SB/SBSimulationManager.h>
#include <SB/SBCharacter.h>

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
#include <sr/sr_camera.h>


#define ANDROID_PYTHON
#ifdef ANDROID_PYTHON
#include <sb/SBPython.h>
#endif

//XERCES_CPP_NAMESPACE_USE


void sb_vhmsg_callback( const char *op, const char *args, void * user_data )
{
	LOG("vhmsg callback op=%s, args=%s\n",op,args);

    // Replace singleton with a user_data pointer
    switch( mcuCBHandle::singleton().execute( op, (char *)args ) ) {
        case CMD_NOT_FOUND:
            LOG("SB ERR: command NOT FOUND: '%s' + '%s'", op, args );
            break;
        case CMD_FAILURE:
            LOG("SB ERR: command FAILED: '%s' + '%s'", op, args );
            break;
    }
} 

void endConnection()
{
	vhmsg::ttu_close();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.vhmsg_enabled = false;
	LOG("TTU Closed, Disable VHMsg.");
}

void initConnection(const char* serverName, const char* portName)
{
    mcuCBHandle& mcu = mcuCBHandle::singleton();
    const char* scope = "DEFAULT_SCOPE";    
    int err;
    int openConnection = vhmsg::ttu_open(serverName,scope,portName);
    if( openConnection == vhmsg::TTU_SUCCESS )
    {
        vhmsg::ttu_set_client_callback( sb_vhmsg_callback );
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
        mcu.vhmsg_enabled = true;
        LOG("TTU Open Success : server = %s, scope = %s, port = %s",serverName,scope,portName);
    }
    else
    {
        LOG("TTU Open Failed : server = %s, scope = %s, port = %s",serverName,scope,portName);
    }    

}

void initSBPython()
{
#ifdef ANDROID_PYTHON
	std::string python_lib_path = "/sdcard/sbmmedia/python";
	//LOGI("Before init Python");
	initPython(python_lib_path);
#endif
}

        
void SBInitialize(const char* mediaPath)
{
	/*
    const char* serverName = "172.16.33.21";
    const char* port = "61616";
    initConnection(serverName,port);
    */

	initSBPython();
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->getSimulationManager()->setTime(0.0);
	scene->addAssetPath("script", "/sdcard/sbogreappdir");
	scene->runScript("initOgre.py");
	scene->getSimulationManager()->start();
}
       
void SBUpdateX(float t)
{

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	int err = vhmsg::ttu_poll();
	if( err == vhmsg::TTU_ERROR )
	{
		fprintf( stderr, "ttu_poll ERROR\n" );
	}

	scene->getSimulationManager()->setTime(t);
	scene->getSimulationManager()->update();
}
    
void SBExecuteCmd(const char* command)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	scene->run(command);
}


void getCharacterWo(const char* name, float& x, float& y, float& z, float& qw, float& qx, float& qy, float& qz)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBCharacter* sbchar = scene->getCharacter(name);

    if (!sbchar)
    	return;

    const SkJoint * joint = sbchar->get_world_offset_joint();
    const SkJointPos * pos = joint->const_pos();
    x = pos->value( SkJointPos::X );
    y = pos->value( SkJointPos::Y );
    z = pos->value( SkJointPos::Z );
    const SrQuat & q = ((SkJoint *)joint)->quat()->value();
    qw = q.w;
    qx = q.x;
    qy = q.y;
    qz = q.z;
}

void getJointRotation(const char* charname, const char* jointname, float& w, float& x, float& y, float& z)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBCharacter* sbchar = scene->getCharacter(charname);

	if (!sbchar)
		return;

    const SkJoint * joint = sbchar->getSkeleton()->search_joint(jointname);
    if (joint)
    {
        const SrQuat & q = ((SkJoint *)joint)->quat()->value();
        w = q.w;
        x = q.x;
        y = q.y;
        z = q.z;
    }
}

void getJointInfo(const char* charname, float* positions, float* bones, std::string* jnames, int n)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBCharacter* sbchar = scene->getCharacter(charname);

	if (!sbchar)
		return;
    
    const SkJoint * wj = sbchar->get_world_offset_joint();
    const SkJointPos * pos = wj->const_pos();
    positions[0] = pos->value( SkJointPos::X );
    positions[1] = pos->value( SkJointPos::Y );
    positions[2] = pos->value( SkJointPos::Z );
    const SrQuat & q = ((SkJoint *)wj)->quat()->value();
    positions[3] = q.w;
    positions[4] = q.x;
    positions[5] = q.y;
    positions[6] = q.z;

    for (int i = 0; i < n; i++)
    {
        std::string& name = jnames[i];
        const SkJoint * joint = sbchar->getSkeleton()->search_joint(name.c_str());
        if (joint)
        {
            const SrQuat & q = ((SkJoint *)joint)->quat()->value();
            bones[i * 4 + 0] = q.w;
            bones[i * 4 + 1] = q.x;
            bones[i * 4 + 2] = q.y;
            bones[i * 4 + 3] = q.z;
        }
        else
        {
            bones[i * 4 + 0] = 1;
            bones[i * 4 + 1] = 0;
            bones[i * 4 + 2] = 0;
            bones[i * 4 + 3] = 0;        
        }
    }
}
