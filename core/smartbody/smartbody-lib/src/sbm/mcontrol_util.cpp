/*
 *  mcontrol_util.cpp - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 *      Andrew n marshall, USC
 *      Ashok Basawapatna, USC (no longer)
 *      Eric Forbell, USC
 *      Corne Versloot, USC
 *      Thomas Amundsen, USC
 *      
 */

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#include "sbm/lin_win.h"
#include "vhcl.h"
#include "sbm/mcontrol_util.h"

#include <stdlib.h>
#include <iostream>
#include <string>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif
#endif

#include "sbm_audio.h"
#include "sb/SBScene.h"
#include <sbm/mcontrol_callbacks.h>
#include <sbm/sbm_test_cmds.hpp>

#if USE_WSP
#include "wsp.h"
#endif

#ifndef __native_client__

#include <sb/SBPythonClass.h>
#include <sb/SBPython.h>

#ifndef SB_NO_PYTHON
#include <boost/python.hpp> // boost python support
#endif

#else
#ifndef SB_NO_PYTHON
#undef USE_PYTHON
#endif 
#endif

#include "sr/sr_model.h"

#if !defined (__ANDROID__) && !defined(SBM_IPHONE)  && !defined(__native_client__)// disable shader support
#ifndef __native_client__
#include "sbm/GPU/SbmShader.h"
#include "sbm/GPU/SbmTexture.h"
#include "sbm/GPU/SbmDeformableMeshGPU.h"
#endif
#endif

#if __native_client__
#include "sbm_test_cmds.hpp"
#include "resource_cmds.h"
#endif


#include "sbm_deformable_mesh.h"
#include <sbm/ODEPhysicsSim.h>
#include <sbm/locomotion_cmds.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sb/SBBoneBusManager.h>
#include <sb/SBScript.h>
#include <sb/SBServiceManager.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotion.h>
#include <sb/SBAssetManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBDebuggerServer.h>
#include <sb/SBDebuggerClient.h>
#include <controllers/me_ct_param_animation_utilities.h>
#include <sb/PABlend.h>
#include <controllers/me_ct_saccade.h>
#include <controllers/me_ct_data_receiver.h>
#include <controllers/me_ct_eyelid.h>
#include <controllers/me_ct_scheduler2.h>
#include <controllers/me_ct_breathing.h>
#include <controllers/me_ct_gaze.h>
#include <controllers/me_controller_tree_root.hpp>
#include <controllers/me_ct_example_body_reach.hpp>
#include <sbm/MiscCommands.h>
#include <sbm/remote_speech.h>
#include <sbm/text_speech.h>

#include "Heightfield.h"
#include "sb/sbm_pawn.hpp"
#include "sb/sbm_character.hpp"
#include <sb/nvbg.h>
#include <sb/SBScene.h>
#include <sbm/KinectProcessor.h>
#include <sb/SBJointMap.h>
#include <sb/SBEvent.h>
#include <sr/sr_viewer.h>
#include <sr/sr_camera.h>
#include <sb/SBCharacterListener.h>
#include <sb/SBAnimationTransition.h>


using namespace std;

#if USE_WSP
using namespace WSP;
#endif

const bool LOG_ABORTED_SEQ = false;

/////////////////////////////////////////////////////////////

#if 0
void mcuCBHandle::test_map( void )	{
	SbmPawn* pawn_p = NULL;

#if 1
	srHashMap <SbmPawn> pawn_map_cp( pawn_map );
#endif

	printf( "iterate ---\n" );

	pawn_map.reset();
	while( pawn_p = pawn_map.next() )	{

		printf( "iterate pawn: %s\n", pawn_p->name );
	
#if 1
		SbmPawn* pawn_cp_p = NULL;
		pawn_map_cp.reset();
		while( pawn_cp_p = pawn_map_cp.next() )	{
			printf( "  pawn: %s\n", pawn_cp_p->name );
		}
#endif	
	}
	printf( "---\n" );
}
#endif

/////////////////////////////////////////////////////////////
//  Singleton Instance
mcuCBHandle* mcuCBHandle::_singleton = NULL;

/////////////////////////////////////////////////////////////


SequenceManager::SequenceManager()
{
}

SequenceManager::~SequenceManager()
{
	clear();
}

void SequenceManager::clear()
{
	for (size_t x = 0; x < _sequences.size(); x++)
	{
		srCmdSeq* seq = _sequences[x].second;
		seq->reset();
		while(char* cmd = seq->pull() )	{
			delete [] cmd;
		}
		delete seq;
	}

	_sequenceSet.clear();
	_sequences.clear();
}

bool SequenceManager::addSequence(const std::string& seqName, srCmdSeq* seq)
{
	if (_sequenceSet.find(seqName) != _sequenceSet.end())
		return false;

	_sequenceSet.insert(seqName);
	_sequences.push_back(std::pair<std::string, srCmdSeq*>(seqName, seq));
	return true;
}

bool SequenceManager::removeSequence(const std::string& seqName, bool deleteSequence)
{
	std::set<std::string>::iterator iter = _sequenceSet.find(seqName);
	if (iter == _sequenceSet.end())
		return false;

	_sequenceSet.erase(iter);

	for (std::vector<std::pair<std::string, srCmdSeq*> >::iterator iter = _sequences.begin();
		iter != _sequences.end();
		iter++)
	{
		if ((*iter).first == seqName)
		{
			if (deleteSequence)
				delete (*iter).second;
			_sequences.erase(iter);
			return true;
		}
	}

	LOG("Could not find sequence in active sequence queue. Please check code - this should not happen.");
	return false;
}

srCmdSeq* SequenceManager::getSequence(const std::string& name)
{
	for (std::vector<std::pair<std::string, srCmdSeq*> >::iterator iter = _sequences.begin();
		iter != _sequences.end();
		iter++)
	{
		if ((*iter).first == name)
		{
			return (*iter).second;
		}
	}

	return NULL;
}

srCmdSeq* SequenceManager::getSequence(int num, std::string& name)
{
	if (_sequences.size() > (size_t) num)
	{
		name = _sequences[num].first;
		return _sequences[num].second;
	}
	else
	{
		return NULL;
	}
}

int SequenceManager::getNumSequences()
{
	return _sequences.size();
}

mcuCBHandle::mcuCBHandle()
:	net_world_offset_updates( true ),
	viewer_p( NULL ),
	ogreViewer_p( NULL ),
	camera_p( NULL ),
	root_group_p( new SrSnGroup() ),
	height_field_p( NULL ),
	viewer_factory ( new SrViewerFactory() ),
	ogreViewerFactory ( new SrViewerFactory() ),
	logListener(NULL),
	initPythonLibPath("")
{	
	root_group_p->ref();
	kinectProcessor = new KinectProcessor();
#if USE_WSP
	theWSP = WSP::create_manager();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );
#endif

}

/////////////////////////////////////////////////////////////

mcuCBHandle::~mcuCBHandle() {
	clear();

	// clean up factories and time profiler which are set externally

	viewer_factory = NULL;
	ogreViewerFactory = NULL;

	// clean up python
#ifndef SB_NO_PYTHON
	Py_Finalize();

#if defined(WIN_BUILD)
	{
		// According to the python docs, .pyd files are not unloaded during Py_Finalize().
		// This causes issues when trying to re-load the smartbody dll over and over.
		// So, we force unload these .pyd files.  This list is all the standard .pyd files included in the Python26 DLLs folder.
		// For reference:  http://docs.python.org/2/c-api/init.html  "Dynamically loaded extension modules loaded by Python are not unloaded"

		// initPythonLibPath - eg:  "../../../../core/smartbody/Python26/Lib"

		HMODULE hmodule;
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/bz2.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/pyexpat.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/select.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/unicodedata.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/winsound.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_bsddb.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes_test.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_elementtree.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_hashlib.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_msi.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_multiprocessing.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_socket.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_sqlite3.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ssl.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_testcapi.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_tkinter.pyd", initPythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
	}
#endif  // WIN_BUILD
#endif  // USE_PYTHON
}

void mcuCBHandle::reset( void )	
{
	// clear everything
	clear();

	// initialize everything
	net_bone_updates = false;
	net_world_offset_updates = true;
	root_group_p = new SrSnGroup();
	logListener = NULL;
	root_group_p->ref();
	kinectProcessor = new KinectProcessor();
#if USE_WSP
	theWSP = WSP::create_manager();
	theWSP->init( "SMARTBODY" );
#endif


}







 

/**
 *  Clears the contents of the mcuCBHandle.
 *  Used by reset and destructor.
 */
void mcuCBHandle::clear( void )	
{
	if (logListener)
	{
		vhcl::Log::g_log.RemoveListener(logListener);
		delete logListener;
		logListener = NULL;
	}
	
	if (kinectProcessor)
	{
		delete kinectProcessor;
		kinectProcessor = NULL;
	}

	//close_viewer();
	if (viewer_p)	
	{
		viewer_factory->reset(viewer_p);
		viewer_p = NULL;
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}


	ogreViewerFactory->remove(ogreViewer_p);
	if (ogreViewer_p)
	{
		delete ogreViewer_p;
		ogreViewer_p = NULL;
	}

	if (camera_p)
	{
		delete camera_p;
		camera_p = NULL;
	}

	if( root_group_p )	
	{
		root_group_p->unref();
		root_group_p = NULL;
	}

	if( height_field_p )	
	{
		delete height_field_p;
		height_field_p = NULL;
	}

/*	cmd_map.reset();


	while (cmd_map.getHashMap().pull() != NULL)
	{
	}
	set_cmd_map.reset();
	while (set_cmd_map.getHashMap().pull() != NULL)
	{
	}
	print_cmd_map.reset();
	while (print_cmd_map.getHashMap().pull() != NULL)
	{
	}
	test_cmd_map.reset();
	while (test_cmd_map.getHashMap().pull() != NULL)
	{
	}
*/



	for (std::map<std::string, DeformableMesh*>::iterator deformableIter = deformableMeshMap.begin();
		deformableIter != deformableMeshMap.end();
		deformableIter++)
	{
		DeformableMesh* deformableMesh = (*deformableIter).second;
	//	delete deformableMesh;
	}
	deformableMeshMap.clear();
	//SbmDeformableMeshGPU::initShader = false;
	//SbmShaderManager::destroy_singleton();

	cameraTracking.clear();


}

/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////

int mcuCBHandle::open_viewer( int width, int height, int px, int py )	{	
	
	if( viewer_p == NULL )	{
		if (!viewer_factory)
			return CMD_FAILURE;
		viewer_p = viewer_factory->create( px, py, width, height );
		viewer_p->label_viewer( "SBM Viewer - Local Mode" );
		camera_p = new SrCamera;
		viewer_p->set_camera( camera_p );
		//((FltkViewer*)viewer_p)->set_mcu(this);
		viewer_p->show_viewer();
		if( root_group_p )	{
			viewer_p->root( root_group_p );
		}
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(viewer_p);
#endif
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_viewer( void )	{

	if( viewer_p )	{
		viewer_factory->remove(viewer_p);
		viewer_p = NULL;
#if !defined (__ANDROID__) && !defined(SBM_IPHONE) && !defined(__native_client__)
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}
	if( camera_p )	{
		delete camera_p;
		camera_p = NULL;
	}
}

int mcuCBHandle::openOgreViewer( int width, int height, int px, int py )	{	

	if( ogreViewer_p == NULL )	{
		if (!ogreViewerFactory)
			return CMD_FAILURE;
		ogreViewer_p = ogreViewerFactory->create( px, py, width, height );
		ogreViewer_p->label_viewer( "SB Ogre Viewer" );
		camera_p = new SrCamera;
		ogreViewer_p->set_camera( camera_p );		
		ogreViewer_p->show_viewer();
		if( root_group_p )	{
			ogreViewer_p->root( root_group_p );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::closeOgreViewer( void )	{

	if( ogreViewer_p )	{
		ogreViewerFactory->remove(ogreViewer_p);
		ogreViewer_p = NULL;
	}	
}



int mcuCBHandle::add_scene( SrSnGroup *scene_p )	{

	if( root_group_p )	{
		if( scene_p )	{
			root_group_p->add( scene_p ); 
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

int mcuCBHandle::remove_scene( SrSnGroup *scene_p )	{

	if( root_group_p )	{
		if( scene_p )	{
			root_group_p->remove( scene_p ); 
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::set_net_host( const char * net_host )
{
	// EDF
	// Sets up the network connection for sending bone rotations over to Unreal
	SmartBody::SBScene::getScene()->getBoneBusManager()->setHost(net_host);
	SmartBody::SBScene::getScene()->getBoneBusManager()->setEnable(true);
	SmartBody::SBScene::getScene()->getBoneBusManager()->getBoneBus().UpdateAllCharacters();
}

int mcuCBHandle::map_skeleton( const char * mapName, const char * skeletonName )
{
	// ED - taken from skeletonmap_func()

	SmartBody::SBSkeleton* sbskeleton = SmartBody::SBScene::getScene()->getAssetManager()->getSkeleton(skeletonName);

	if (!sbskeleton)
	{
		LOG("Cannot find skeleton named %s.", skeletonName);
		return CMD_FAILURE;
	}
	
	// find the bone map name
	SmartBody::SBJointMap* jointMap = SmartBody::SBScene::getScene()->getJointMapManager()->getJointMap(mapName);
	if (!jointMap)
	{
		LOG("Cannot find joint map name '%s'.", mapName);
		return CMD_FAILURE;
	}

	// apply the map
	jointMap->applySkeleton(sbskeleton);

	LOG("Applied joint map %s to skeleton %s.", mapName, skeletonName);

	return CMD_SUCCESS;
}

int mcuCBHandle::map_motion( const char * mapName, const char * motionName )
{
	// taken from motionmap_func()

	SmartBody::SBMotion* sbmotion = SmartBody::SBScene::getScene()->getAssetManager()->getMotion(motionName);
	if (!sbmotion)
	{
		LOG("Cannot find motion name %s.", motionName);
		return CMD_FAILURE;
	}
	
	// find the bone map name
	SmartBody::SBJointMap* jointMap = SmartBody::SBScene::getScene()->getJointMapManager()->getJointMap(mapName);
	if (!jointMap)
	{
		LOG("Cannot find bone map name '%s'.", mapName);
		return CMD_FAILURE;
	}

	// apply the map
	jointMap->applyMotion(sbmotion);

	LOG("Applied bone map %s to motion %s.", mapName, motionName);

	return CMD_SUCCESS;
}

void mcuCBHandle::NetworkSendSkeleton( bonebus::BoneBusCharacter * character, SkSkeleton * skeleton, GeneralParamMap * param_map )
{
	if ( character == NULL )
	{
		return;
	}


	// Send the bone rotation for each joint in the skeleton
	const std::vector<SkJoint *> & joints  = skeleton->joints();

	character->IncrementTime();
	character->StartSendBoneRotations();

	std::vector<int> otherJoints;

	for ( size_t i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeJoint)
		{
			if (j->getJointType() == SkJoint::TypeOther)
				otherJoints.push_back(i); // collect the 'other' joins
			continue;
		}

		const SrQuat& q = j->quat()->value();

		character->AddBoneRotation( j->extName().c_str(), q.w, q.x, q.y, q.z, SmartBody::SBScene::getScene()->getSimulationManager()->getTime() );

		//printf( "%s %f %f %f %f\n", (const char *)j->name(), q.w, q.x, q.y, q.z );
	}

	character->EndSendBoneRotations();


	character->StartSendBonePositions();

	for ( size_t i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeJoint)
			continue;

		float posx = j->pos()->value( 0 );
		float posy = j->pos()->value( 1 );
		float posz = j->pos()->value( 2 );
		if ( false )
		{
			posx += j->offset().x;
			posy += j->offset().y;
			posz += j->offset().z;
		}

		//these coordinates are meant to mimic the setpositionbyname coordinates you give to move the character
		//so if you wanted to move a joint on the face in the x direction you'd do whatever you did to move the actor
		//itself further in the x position.
		character->AddBonePosition( j->extName().c_str(), posx, posy, posz, SmartBody::SBScene::getScene()->getSimulationManager()->getTime() );
	}

	character->EndSendBonePositions();

	if (otherJoints.size() > 0)
	{
		character->StartSendGeneralParameters();
		for (size_t i = 0; i < otherJoints.size(); i++)
		{
			SkJoint* joint = joints[otherJoints[i]];
			character->AddGeneralParameters(i, 1, joint->pos()->value( 0 ), i, SmartBody::SBScene::getScene()->getSimulationManager()->getTime());

		}
		character->EndSendGeneralParameters();
	}
	

/*
	// Passing General Parameters
	character->StartSendGeneralParameters();
<<<<<<< .mine
	for (size_t i = 0; i < joints.size(); i++)
=======
	int numFound = 0;
	for (int i = 0; i < joints.size(); i++)
>>>>>>> .r2317
	{
		SkJoint* j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeOther)
			continue;

		// judge whether it is joint for general parameters, usually should have a prefix as "param"
		string j_name = j->name();
		int name_end_pos = j_name.find_first_of("_");
		string test_prefix = j_name.substr( 0, name_end_pos );
		if( test_prefix == character->m_name )	
		{
			// if is, prepare adding data
			int index = 0;
			GeneralParamMap::iterator pos;
			for(pos = param_map->begin(); pos != param_map->end(); pos++)
			{
				for(int n = 0; n < (int)pos->second->char_names.size(); n++)
				{
					if( character->m_name == pos->second->char_names[n] )
					{
						index ++;
						for(int m = 0 ; m < pos->second->size; m++)
						{
							std::stringstream joint_name;
							joint_name << character->m_name << "_" << index << "_" << ( m + 1 );
							if(_stricmp( j->name().c_str(), joint_name.str().c_str()) == 0)
								character->AddGeneralParameters(index, pos->second->size, j->pos()->value(0), m, time);
						}
					}
				}
			}
		}
	}
	character->EndSendGeneralParameters();
*/
	
}

int mcuCBHandle::executePythonFile(const char* filename)
{
#ifndef SB_NO_PYTHON
	// add the .seq extension if necessary
	std::string candidateSeqName = filename;
	if (candidateSeqName.find(".py") == std::string::npos)
	{
		candidateSeqName.append(".py");
	}
	// current path containing .exe
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);

	std::string curFilename = SmartBody::SBScene::getScene()->getAssetManager()->findFileName("script", candidateSeqName);
	if (curFilename != "")
	{
		try {
			std::stringstream strstr;
			strstr << "execfile(\"" << curFilename << "\")";
			PyRun_SimpleString(strstr.str().c_str());
			PyErr_Print();
			PyErr_Clear();
			return CMD_SUCCESS;
		} catch (...) {
			PyErr_Print();
			return CMD_FAILURE;
		}
	}

	LOG("Could not find Python script '%s'", filename);
	return CMD_FAILURE;

#endif
	return CMD_FAILURE;
}

int mcuCBHandle::executePython(const char* command)
{
#ifndef SB_NO_PYTHON
	try {
		//LOG("executePython = %s",command);

		int result = PyRun_SimpleString(command);
		//LOG("cmd result = %d",result);

		return CMD_SUCCESS;
	} catch (...) {
		PyErr_Print();
	}
#endif
	return CMD_FAILURE;
}



std::map<std::string, SbmPawn*>& mcuCBHandle::getPawnMap()
{
	return pawn_map;
}

bool mcuCBHandle::addPawn(SbmPawn* pawn)
{
	SbmPawn* p = getPawn(pawn->getName());
	if (!p)
	{
		pawn_map[pawn->getName()] = pawn;
		return true;
	}
	else
	{
		return false;
	}

}

void mcuCBHandle::removePawn(const std::string& name)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(name);
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}
}

SbmPawn* mcuCBHandle::getPawn(const std::string& name)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(name);
	if (iter == pawn_map.end())
		return NULL;
	else
		return (*iter).second;
}

int mcuCBHandle::getNumPawns()
{
	return pawn_map.size();
}

std::map<std::string, SbmCharacter*>& mcuCBHandle::getCharacterMap()
{
	return character_map;
}

std::string mcuCBHandle::getValidName(const std::string& name)
{
	bool nameFound = true;
	int nameCounter = 0;
	std::string currentName = name;
	while (nameFound)
	{
		std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(currentName);
		if (iter == pawn_map.end())
		{
			nameFound = false;
		}
		else
		{
			std::stringstream strstr;
			strstr << name << nameCounter;
			nameCounter++;
			currentName = strstr.str();
		}
	}
	return currentName;
}




void mcuCBHandle::render()
{
	if( viewer_p ) { viewer_p->render(); }
	if (ogreViewer_p) { ogreViewer_p->render(); }
}


void mcuCBHandle::render_terrain( int renderMode ) {
			if( height_field_p )	{
				height_field_p->render(renderMode);
			}
		}

float mcuCBHandle::query_terrain( float x, float z, float *normal_p )	{
			if( height_field_p )	{
				return( height_field_p->get_elevation( x, z, normal_p ) );
			}
			if( normal_p )	{
				normal_p[ 0 ] = 0.0;
				normal_p[ 1 ] = 1.0;
				normal_p[ 2 ] = 0.0;
			}
			return( 0.0 );
		}

/////////////////////////////////////////////////////////////
