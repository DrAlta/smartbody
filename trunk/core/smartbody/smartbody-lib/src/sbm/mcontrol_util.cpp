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
:	viewer_p( NULL ),
	ogreViewer_p( NULL ),
	camera_p( NULL ),
	viewer_factory ( new SrViewerFactory() ),
	ogreViewerFactory ( new SrViewerFactory() )
{	


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
		std::string pythonLibPath = Py_GetPythonHome();
		HMODULE hmodule;
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/bz2.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/pyexpat.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/select.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/unicodedata.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/winsound.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_bsddb.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ctypes_test.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_elementtree.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_hashlib.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_msi.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_multiprocessing.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_socket.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_sqlite3.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_ssl.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_testcapi.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
		hmodule = GetModuleHandle(vhcl::Format("%s/../DLLs/_tkinter.pyd", pythonLibPath.c_str()).c_str());
		FreeLibrary(hmodule);
	}
#endif  // WIN_BUILD
#endif  // USE_PYTHON
}

void mcuCBHandle::reset( void )	
{
	// clear everything
	clear();

}







 

/**
 *  Clears the contents of the mcuCBHandle.
 *  Used by reset and destructor.
 */
void mcuCBHandle::clear( void )	
{

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
		if( SmartBody::SBScene::getScene()->getRootGroup() )	{
			viewer_p->root( SmartBody::SBScene::getScene()->getRootGroup() );
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
		if( SmartBody::SBScene::getScene()->getRootGroup() )	{
			ogreViewer_p->root( SmartBody::SBScene::getScene()->getRootGroup() );
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




void mcuCBHandle::render()
{
	if( viewer_p ) { viewer_p->render(); }
	if (ogreViewer_p) { ogreViewer_p->render(); }
}


/////////////////////////////////////////////////////////////
