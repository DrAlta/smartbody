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

#include "lin_win.h"
#include "vhcl.h"
#include "mcontrol_util.h"

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

#include "me_utilities.hpp"

#if USE_WSP
#include "wsp.h"
#endif
#include <sbm/SbmPythonClass.h>
#include <sbm/SbmPython.h>
#ifdef USE_PYTHON
#include <boost/python.hpp> // boost python support
#endif
#include "sr/sr_model.h"

#ifndef __ANDROID__ // disable shader support
#include "sbm/GPU/SbmShader.h"
#include "sbm/GPU/SbmTexture.h"
#endif

#include "sbm_deformable_mesh.h"
#include "sbm/Physics/SbmPhysicsSimODE.h"

#include <boost/algorithm/string/replace.hpp>

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


mcuCBHandle::mcuCBHandle()
:	loop( true ),
	vhmsg_enabled( false ),	
	internal_timer_p( NULL ),
	external_timer_p( NULL ),
	timer_p( NULL ),
	time( 0.0 ),
	time_dt( 0.0 ),
	physicsTime(0.0),
	internal_profiler_p( NULL ),
	external_profiler_p( NULL ),
	profiler_p( NULL ),
	net_bone_updates( true ),
	net_world_offset_updates( true ),
	net_face_bones( false ),
	net_host( NULL ),
	sbm_character_listener( NULL ),
	play_internal_audio( false ),
	resourceDataChanged( false ),
	skScale( 1.0 ),
	skmScale( 1.0 ),
	viewer_p( NULL ),
	bmlviewer_p( NULL ),
	panimationviewer_p( NULL ),
	channelbufferviewer_p( NULL ),
	resourceViewer_p( NULL ),
	velocityViewer_p( NULL ),
	faceViewer_p( NULL ),
	camera_p( NULL ),
	root_group_p( new SrSnGroup() ),
	height_field_p( NULL ),
	logger_p( new joint_logger::EvaluationLogger() ),
	test_character_default( "" ),
	test_recipient_default( "ALL" ),
	queued_cmds( 0 ),
	use_locomotion( false ),
	use_param_animation( false ),
	use_data_receiver( false ),
	updatePhysics( false ),
	locomotion_type( Basic ),
	viewer_factory ( new SrViewerFactory() ),
	bmlviewer_factory ( new GenericViewerFactory() ),
	panimationviewer_factory ( new GenericViewerFactory() ),
	channelbufferviewer_factory ( new GenericViewerFactory() ),
	commandviewer_factory ( new GenericViewerFactory() ),
	resourceViewerFactory ( new GenericViewerFactory() ),
	velocityViewerFactory ( new GenericViewerFactory() ),
	faceViewerFactory ( new GenericViewerFactory() ),
	resource_manager(ResourceManager::getResourceManager()),
	snapshot_counter( 1 ),
	use_python( false ),
	delay_behaviors(true),
	media_path("."),
	_interactive(true),
	sendPawnUpdates(false)
	//physicsEngine(NULL)
{	
	root_group_p->ref();
	logger_p->ref();
	kinectProcessor = new KinectProcessor();
#if USE_WSP
	theWSP = WSP::create_manager();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );
#endif
	createDefaultControllers();
	// initialize the default face motion mappings
	FaceDefinition* faceDefinition = new FaceDefinition();
	faceDefinition->setName("_default_");
	face_map["_default_"] = faceDefinition;
	physicsEngine = new SbmPhysicsSimODE();
	physicsEngine->initSimulation();

}

/////////////////////////////////////////////////////////////

mcuCBHandle::~mcuCBHandle() {
	clear();

	// clean up python
#ifdef USE_PYTHON
	Py_Finalize();
#endif
}

void mcuCBHandle::reset( void )	{
	clear();

	// reset initial variables to match the constructor.
	loop = true;
	time = 0.0;
	net_bone_updates = true;
	use_locomotion = false;
	use_data_receiver = false;
	root_group_p = new SrSnGroup();
	root_group_p->ref();
	logger_p = new joint_logger::EvaluationLogger();
	logger_p->ref();

#if USE_WSP
	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );
#endif

	if ( net_host )
		bonebus.OpenConnection( net_host );
}

 void mcuCBHandle::createDefaultControllers()
 {
	 _defaultControllers.push_back(new MeCtEyeLidRegulator());
	 _defaultControllers.push_back(new MeCtSaccade(NULL));
	 std::map<int, MeCtReachEngine*> reachMap;
	 _defaultControllers.push_back(new MeCtExampleBodyReach(reachMap));
 }


 FILE* mcuCBHandle::open_sequence_file( const char *seq_name, std::string& fullPath ) {

	FILE* file_p = NULL;

	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];

	// add the .seq extension if necessary
	std::string candidateSeqName = seq_name;
	if (candidateSeqName.find(".seq") == std::string::npos)
	{
		candidateSeqName.append(".seq");
	}
	sprintf( label, "%s", candidateSeqName.c_str());
	// current path containing .exe
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);

	seq_paths.reset();
	std::string filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
	//filename = mcn_return_full_filename_func( CurrentPath, filename );
	
	while(filename.size() > 0)	{
		file_p = fopen( filename.c_str(), "r" );
		if( file_p != NULL ) {
	
			// add the file resource
			FileResource* fres = new FileResource();
			std::stringstream stream;
			stream << filename;
			fres->setFilePath(stream.str());
			resource_manager->addResource(fres);
			fullPath = filename;			
			break;
		}
		filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension	
		sprintf( label, "%s.seq", seq_name );
		seq_paths.reset();
		filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
		while( filename.size() > 0 )	{
			if( ( file_p = fopen( filename.c_str(), "r" ) ) != NULL ) {
				
				// add the file resource
				FileResource* fres = new FileResource();				
				std::stringstream stream;
				stream << filename;
				fres->setFilePath(stream.str());
				resource_manager->addResource(fres);
				fullPath = filename;
				break;
			}
			filename = seq_paths.next_filename( buffer, candidateSeqName.c_str() );
			//filename = mcn_return_full_filename_func( CurrentPath, filename );
		}
	}

	// return empty string if file not found
	return file_p;
}

/**
 *  Clears the contents of the mcuCBHandle.
 *  Used by reset and destructor.
 */
void mcuCBHandle::clear( void )	{

	for (std::map<std::string, FaceDefinition*>::iterator iter = this->face_map.begin();
		iter != face_map.end();
		iter++)
	{
		FaceDefinition* face = (*iter).second;
		delete face;
	}

	for (size_t x = 0; x < this->cameraTracking.size(); x++)
	{
		delete this->cameraTracking[x];
	}

	if( height_field_p )	{
		delete height_field_p;
		height_field_p = NULL;
	}

	srCmdSeq* seq_p;
	pending_seq_map.reset();
	while( seq_p = pending_seq_map.pull() )	{
		char *cmd;
		seq_p->reset();
		while( cmd = seq_p->pull() )	{
			delete [] cmd;
		}
		delete seq_p;
	}
	active_seq_map.reset();
	while( seq_p = active_seq_map.pull() )	{
		char *cmd;
		seq_p->reset();
		while( cmd = seq_p->pull() )	{
			delete [] cmd;
		}
		delete seq_p;
	}

	getCharacterMap().clear();
	getPawnMap().clear();


	for (std::map<std::string, SkPosture*>::iterator postureIter = pose_map.begin();
		postureIter != pose_map.end();
		postureIter++)
	{
		SkPosture* posture = (*postureIter).second;
		delete posture;
	}
	
	for (std::map<std::string, SkMotion*>::iterator motionIter = motion_map.begin();
		 motionIter != motion_map.end();
		 motionIter++)
	{

		//SkMotion* motion = (*motionIter).second;
		//motion->unref(); // need to cleanup motions - fix

	}

	for (std::map<std::string, SkSkeleton*>::iterator skelIter = skeleton_map.begin();
		 skelIter != skeleton_map.end();
		 skelIter++)
	{
		SkSkeleton* skeleton = (*skelIter).second;
		delete skeleton;
	}

	// remove the parameterized animation states
	for (std::vector<PAStateData*>::iterator iter = param_anim_states.begin();
	     iter != param_anim_states.end();
	     iter++)
	{
		delete (*iter);
	}
	param_anim_states.clear();

	// remove the transition maps
	for (std::vector<PATransitionData*>::iterator iter = param_anim_transitions.begin();
	     iter != param_anim_transitions.end();
	     iter++)
	{
		delete (*iter);
	}
	param_anim_transitions.clear();
	
	/*
	MeCtPose* pose_ctrl_p;
	pose_ctrl_map.reset();
	while( pose_ctrl_p = pose_ctrl_map.pull() )	{
		pose_ctrl_p->unref();
	}
	
	MeCtMotion* mot_ctrl_p;
	motion_ctrl_map.reset();
	while( mot_ctrl_p = motion_ctrl_map.pull() )	{
		mot_ctrl_p->unref();
	}
	
	MeCtStepTurn* stepturn_ctrl_p;
	stepturn_ctrl_map.reset();
	while( stepturn_ctrl_p = stepturn_ctrl_map.pull() )	{
		stepturn_ctrl_p->unref();
	}

	MeCtQuickDraw* qdraw_ctrl_p;
	quickdraw_ctrl_map.reset();
	while( qdraw_ctrl_p = quickdraw_ctrl_map.pull() )	{
		qdraw_ctrl_p->unref();
	}
		
	MeCtGaze* gaze_ctrl_p;
	gaze_ctrl_map.reset();
	while( gaze_ctrl_p = gaze_ctrl_map.pull() )	{
		gaze_ctrl_p->unref();
	}
	
	MeCtSimpleNod* snod_ctrl_p;
	snod_ctrl_map.reset();
	while( snod_ctrl_p = snod_ctrl_map.pull() )	{
		snod_ctrl_p->unref();
	}
	
	MeCtAnkleLilt* lilt_ctrl_p;
	lilt_ctrl_map.reset();
	while( lilt_ctrl_p = lilt_ctrl_map.pull() ){
		lilt_ctrl_p->unref();
	}
	
	MeCtEyeLid* eyelid_ctrl_p;
	eyelid_ctrl_map.reset();
	while( eyelid_ctrl_p = eyelid_ctrl_map.pull() ){
		eyelid_ctrl_p->unref();
	}
	
	MeCtScheduler2* sched_ctrl_p;
	sched_ctrl_map.reset();
	while( sched_ctrl_p = sched_ctrl_map.pull() )	{
		sched_ctrl_p->unref();
	}
	
	MeController* ctrl_p;
	controller_map.reset();
	while( ctrl_p = controller_map.pull() )	{
		ctrl_p->unref();
	}
	*/
	
	if( root_group_p )	{
		root_group_p->unref();
		root_group_p = NULL;
	}

	if( logger_p ) {
		logger_p->unref();
		logger_p = NULL;
	}

	close_viewer();

	if ( net_host )
		bonebus.CloseConnection();

#if USE_WSP
	theWSP->shutdown();
#endif
	if (kinectProcessor)
		delete kinectProcessor;
	kinectProcessor = NULL;
}

/////////////////////////////////////////////////////////////

std::string mcuCBHandle::cmdl_tab_callback( std::string cmdl_str )	{

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	srHashMapBase* map = NULL;

	// get the current partial command
	std::string partialCommand( cmdl_str );
	std::string commandPrefix = "";

	// only use tab completion on the first word
	size_t index = partialCommand.find_first_of(" ");
	if( index != std::string::npos )
	{

		// if the command matches 'set', 'print' or 'test' use those maps
		std::string firstToken = partialCommand.substr(0, index);
		if (firstToken == "set")
		{
			map = &mcu.set_cmd_map.getHashMap();
			partialCommand = partialCommand.substr(index + 1);
			commandPrefix = "set ";
		}
		else if (firstToken == "print")
		{
			map = &mcu.print_cmd_map.getHashMap();
			partialCommand = partialCommand.substr(index + 1);
			commandPrefix = "print ";
		}
		else if (firstToken == "test")
		{
			map = &mcu.test_cmd_map.getHashMap();
			partialCommand = partialCommand.substr(index + 1);
			commandPrefix = "test ";
		}
		else
		{
			// transform tabs into a space
			cmdl_str += " ";
		}

	}

	// find a match against the current list of commands

	if( !map )
		map = &mcu.cmd_map.getHashMap();
	int numEntries = map->get_num_entries();
	map->reset();
	int numMatches = 0;
	char* key = NULL;
	int numChecked = 0;
	map->next( &key );
	std::vector<std::string> options;

	while( key )
	{
		bool match = false;
		std::string keyString = key;
		numChecked++;
		if( partialCommand.size() <= keyString.size() )
		{
			match = true;
			for( size_t a = 0; a < partialCommand.size() && a < keyString.size(); a++ )
			{
				if( partialCommand[ a ] != keyString[ a ] )
				{
					match = false;
					break;
				}
			}
			if( match )
			{
				options.push_back( keyString );
				numMatches++;
			}
		}
		map->next( &key );
		std::string nextKey = key;
		if( nextKey == keyString )
			break; // shouldn't map.next(key) make key == NULL? This doesn't seem to happen.
	}

	if( numMatches == 1 )
	{
		cmdl_str = commandPrefix + options[0] + " ";
	}
	else 
	if (numMatches > 1)
	{ // more than one match, show the options on the line below

		fprintf( stdout, "\n");
		std::sort( options.begin(), options.end() );
		for( size_t x = 0; x < options.size(); x++ )
		{
			fprintf( stdout, "%s ", options[x].c_str() );
		}
	}
	else 
	if( numMatches == 0 )
	{
		// transform tabs into a space
		cmdl_str += " ";
	}

	return( cmdl_str );
}

/////////////////////////////////////////////////////////////

int mcuCBHandle::open_viewer( int width, int height, int px, int py )	{	
	
	if( viewer_p == NULL )	{
		if (!viewer_factory)
			return CMD_FAILURE;
		viewer_p = viewer_factory->create( px, py, width, height );
		viewer_p->label_viewer( "SBM Viewer" );
		camera_p = new SrCamera;
		viewer_p->set_camera( *camera_p );
		//((FltkViewer*)viewer_p)->set_mcu(this);
		viewer_p->show_viewer();
		if( root_group_p )	{
			viewer_p->root( root_group_p );
		}
#ifndef __ANDROID__
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
#ifndef __ANDROID__
		SbmShaderManager::singleton().setViewer(NULL);
#endif
	}
	if( camera_p )	{
		delete camera_p;
		camera_p = NULL;
	}
}

int mcuCBHandle::open_bml_viewer( int width, int height, int px, int py )	{
	
	if( bmlviewer_p == NULL )	{
		bmlviewer_p = bmlviewer_factory->create( px, py, width, height );
		bmlviewer_p->label_viewer( "SBM BML Viewer" );
		bmlviewer_p->show_viewer();
		
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_bml_viewer( void )	{

	if( bmlviewer_p )	{
		bmlviewer_factory->destroy(bmlviewer_p);
		bmlviewer_p = NULL;
	}
}

int mcuCBHandle::open_panimation_viewer( int width, int height, int px, int py )
{
	if( panimationviewer_p == NULL )	{
		panimationviewer_p = panimationviewer_factory->create( px, py, width, height );
		panimationviewer_p->label_viewer( "Parameterized Animation Viewer" );
		panimationviewer_p->show_viewer();
		
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_panimation_viewer( void )
{
	if( panimationviewer_p )	{
		panimationviewer_factory->destroy(panimationviewer_p);
		panimationviewer_p = NULL;
	}
}

int mcuCBHandle::open_channelbuffer_viewer( int width, int height, int px, int py )
{
	if( channelbufferviewer_p == NULL )	{
		channelbufferviewer_p = channelbufferviewer_factory->create( px, py, width, height );
		channelbufferviewer_p->label_viewer( "Channel Buffer Viewer" );
		channelbufferviewer_p->show_viewer();
		
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_channelbuffer_viewer( void )
{
	if( channelbufferviewer_p )	{
		channelbufferviewer_factory->destroy(channelbufferviewer_p);
		channelbufferviewer_p = NULL;
	}
}


int mcuCBHandle::openResourceViewer( int width, int height, int px, int py )
{
	if( resourceViewer_p == NULL )	{
		resourceViewer_p = resourceViewerFactory->create( px, py, width, height );
		resourceViewer_p->label_viewer( "Resource Viewer" );
		resourceViewer_p->show_viewer();

		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}


void mcuCBHandle::closeResourceViewer( void )
{
	if( resourceViewer_p )	{
		resourceViewerFactory->destroy(resourceViewer_p);
		resourceViewer_p = NULL;
	}
}

int mcuCBHandle::openVelocityViewer( int width, int height, int px, int py )
{
	if( velocityViewer_p == NULL )	{
		velocityViewer_p = velocityViewerFactory->create( px, py, width, height );
		velocityViewer_p->label_viewer( "Velocity Viewer" );
		velocityViewer_p->show_viewer();

		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::closeVelocityViewer( void )
{
	if( velocityViewer_p )	{
		velocityViewerFactory->destroy(velocityViewer_p);
		velocityViewer_p = NULL;
	}
}

int mcuCBHandle::openFaceViewer( int width, int height, int px, int py )
{
	if( faceViewer_p == NULL )	{
		faceViewer_p = faceViewerFactory->create( px, py, width, height );
		faceViewer_p->label_viewer( "Face Viewer" );
		faceViewer_p->show_viewer();

		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}


void mcuCBHandle::closeFaceViewer( void )
{
	if( resourceViewer_p )	{
		faceViewerFactory->destroy(resourceViewer_p);
		faceViewer_p = NULL;
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

void mcuCBHandle::update( void )	{

#if 0
	static int c = 3;
	if( c )	{
		test_map();
		c--;
	}
#endif

	// updating steering engine
	if (steerEngine.isInitialized())
	{
		if (!this->steerEngine.isDone())
		{
			if (this->steerEngine.getStartTime() == 0.0f)

				this->steerEngine.setStartTime(float(this->time));
	
			for (std::map<std::string, SbmCharacter*>::iterator iter = getCharacterMap().begin();
				iter != getCharacterMap().end();
				iter++)
			{
				SbmCharacter* character = (*iter).second;
				character->steeringAgent->evaluate();
			}

			bool running = this->steerEngine._engine->update(false, true, float(this->time) - this->steerEngine.getStartTime());
			if (!running)
				this->steerEngine.setDone(true);
		}
	}

	if (physicsEngine && physicsEngine->getBoolAttribute("enable"))
	{		
		static float dt = 0.005f;//timeStep*0.03f;
		//elapseTime += time_dt;
		while (physicsTime < this->time)		
		{
			//printf("elapse time = %f\n",elapseTime);
			physicsEngine->updateSimulation(dt);
			physicsTime += dt;
			//curDt -= dt;
		}		
	}

	srCmdSeq* seq_p;
	char *seq_name = NULL;
	active_seq_map.reset();



	while( seq_p = active_seq_map.next( & seq_name ) )	{

		char *cmd;
		while( cmd = seq_p->pop( (float)time ) )	{

#if 0
			// the parent resource is associated with seq_name
			CmdResource* cmdResource = resource_manager->getCmdResource(seq_name);
			if(cmdResource)	{
				resource_manager->addParent(cmdResource);
			}
#endif
			int err = execute( cmd );
			if( err != CMD_SUCCESS )	{
				LOG( "mcuCBHandle::update ERR: execute FAILED: '%s'\n", cmd );
			}
			else
			{
				// we assume that every command execution could potentially change the resource data
				// therefore the data changed flag is set.
				// This is kind of rough, but should work fine.
				resourceDataChanged = true; 
			}
			delete [] cmd;
		}
		if( seq_p->get_count() < 1 )	{
			seq_p = active_seq_map.remove( seq_name );
			delete seq_p;
		}
#if 0
		if (cmdResource)	{
			resource_manager->removeParent();
		}
#endif
	}

#ifndef __ANDROID__
	SbmShaderManager& ssm = SbmShaderManager::singleton();
	SbmTextureManager& texm = SbmTextureManager::singleton();
	bool hasOpenGL        = ssm.initOpenGL();
	bool hasShaderSupport = false;

	// init OpenGL extension
	if (hasOpenGL)
	{
		hasShaderSupport = ssm.initGLExtension();		
	}
	// update the shader map
	if (hasShaderSupport)
	{
		ssm.buildShaders();
		texm.updateTexture();
	}	
#endif
	
	bool isClosingBoneBus = false;
	for (std::map<std::string, SbmPawn*>::iterator iter = getPawnMap().begin();
		iter != getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		pawn->reset_all_channels();
		pawn->ct_tree_p->evaluate( time );
		pawn->ct_tree_p->applyBufferToAllSkeletons();

		if (pawn->hasPhysicsSim() && physicsEngine->getBoolAttribute("enable"))
		{
			pawn->updateFromColObject();
		}
		else
		{			
			pawn->updateToColObject();
			pawn->updateToSteeringSpaceObject();
		}

		SbmCharacter* char_p = getCharacter(pawn->getName().c_str() );
		if (!char_p)
		{
			if (net_bone_updates)
			{
				if (!isClosingBoneBus && !pawn->bonebusCharacter && bonebus.IsOpen() && sendPawnUpdates)
				{
					// bonebus was connected after character creation, create it now
					pawn->bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( pawn->getName().c_str(), "pawn" , false );
				}
				if (sendPawnUpdates)
					NetworkSendSkeleton( pawn->bonebusCharacter, pawn->getSkeleton(), &param_map );
				if (pawn->bonebusCharacter && pawn->bonebusCharacter->GetNumErrors() > 3)
				{
					// connection is bad, remove the bonebus character 
					LOG("BoneBus cannot connect to server. Removing pawn %s", pawn->getName().c_str());
					bool success = bonebus.DeleteCharacter(pawn->bonebusCharacter);
					char_p->bonebusCharacter = NULL;
					isClosingBoneBus = true;
					if (bonebus.GetNumCharacters() == 0)
					{
						bonebus.CloseConnection();
					}
				}
			}
		}
		if( char_p ) {

			char_p->forward_visemes( time );	
			if (char_p->scene_p)
				char_p->scene_p->update();	
			char_p->updateJointPhyObjs();
			char_p->_skeleton->update_global_matrices();
			//char_p->dMesh_p->update();

			if ( net_bone_updates && char_p->getSkeleton() && char_p->bonebusCharacter ) {
				NetworkSendSkeleton( char_p->bonebusCharacter, char_p->getSkeleton(), &param_map );

				if ( net_world_offset_updates ) {

					const SkJoint * joint = char_p->get_world_offset_joint();

					const SkJointPos * pos = joint->const_pos();
					float x = pos->value( SkJointPos::X );
					float y = pos->value( SkJointPos::Y );
					float z = pos->value( SkJointPos::Z );

					SkJoint::RotType rot_type = joint->rot_type();
					if ( rot_type != SkJoint::TypeQuat ) {
						//strstr << "ERROR: Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")"<<endl;
					}

					// const_cast because the SrQuat does validation (no const version of value())
					const SrQuat & q = ((SkJoint *)joint)->quat()->value();

					char_p->bonebusCharacter->SetPosition( x, y, z, time );
					char_p->bonebusCharacter->SetRotation( (float)q.w, (float)q.x, (float)q.y, (float)q.z, time );

					if (char_p->bonebusCharacter->GetNumErrors() > 3)
					{
						// connection is bad, remove the bonebus character 
						isClosingBoneBus = true;
						LOG("BoneBus cannot connect to server. Removing all characters");
					}
				}
			}
			else if (!isClosingBoneBus && !char_p->bonebusCharacter && bonebus.IsOpen())
			{
				// bonebus was connected after character creation, create it now
				char_p->bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( char_p->getName().c_str(), char_p->getClassType().c_str(), this->net_face_bones );
			}
		}  // end of char_p processing
	} // end of loop

	if (isClosingBoneBus)
	{
		for (std::map<std::string, SbmPawn*>::iterator iter = getPawnMap().begin();
			iter != getPawnMap().end();
			iter++)
		{
			SbmPawn* pawn = (*iter).second;
			if (pawn->bonebusCharacter)
			{
				bool success = bonebus.DeleteCharacter(pawn->bonebusCharacter);
				pawn->bonebusCharacter = NULL;
			}
		}

		bonebus.CloseConnection();
	}


	if (panimationviewer_p)
		panimationviewer_p->update_viewer();

	// update any tracked cameras
	for (size_t x = 0; x < this->cameraTracking.size(); x++)
	{
		// move the camera relative to the joint
		SkJoint* joint = this->cameraTracking[x]->joint;
		joint->skeleton()->update_global_matrices();
		joint->update_gmat();
		const SrMat& jointGmat = joint->gmat();
		SrVec jointLoc(jointGmat[12], jointGmat[13], jointGmat[14]);
		SrVec newJointLoc = jointLoc;
		if (fabs(jointGmat[13] - this->cameraTracking[x]->yPos) < this->cameraTracking[x]->threshold)
			newJointLoc.y = (float)this->cameraTracking[x]->yPos;
		SrVec cameraLoc = newJointLoc + this->cameraTracking[x]->jointToCamera;
		this->camera_p->eye.set(cameraLoc.x, cameraLoc.y, cameraLoc.z);
		SrVec targetLoc = cameraLoc - this->cameraTracking[x]->targetToCamera;
		this->camera_p->center.set( targetLoc.x, targetLoc.y, targetLoc.z);
		this->viewer_p->set_camera(*( this->camera_p ));
	}	
}

srCmdSeq* mcuCBHandle::lookup_seq( const char* seq_name ) {
	int err = CMD_FAILURE;
	
	// Remove previous activation of sequence.
	// Anm: Why?  Need clear distrinction (and efinition) between pending and active.
	abort_seq( seq_name );

	srCmdSeq* seq_p = pending_seq_map.remove( seq_name );
	if( seq_p == NULL ) {
		// Sequence not found.  Load new instance from file.
		std::string fullPath;
		FILE* file_p = open_sequence_file( seq_name, fullPath );
		if( file_p ) {
			seq_p = new srCmdSeq();
			err = seq_p->read_file( file_p );
			fclose( file_p );

			if( err != CMD_SUCCESS ) {
				LOG("ERROR: mcuCBHandle::lookup_seq(..): '%s' PARSE FAILED\n", seq_name ); 

				delete seq_p;
				seq_p = NULL;
			}
		} else {
			LOG("ERROR: mcuCBHandle::lookup_seq(..): '%s' NOT FOUND\n", seq_name ); 
		}
	}
	
	return( seq_p );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq ) {
	ostringstream seq_id;
	seq_id << "execute_seq-" << (++queued_cmds);

	return execute_seq( seq, seq_id.str().c_str() );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq_p, const char* seq_id ) {

//	printf( "mcuCBHandle::execute_seq: id: '%s'\n", seq_id );
//	seq_p->print();

	if ( active_seq_map.insert( seq_id, seq_p ) != CMD_SUCCESS ) {
		LOG("ERROR: mcuCBHandle::execute_seq(..): Failed to insert srCmdSeq \"%s\"into active_seq_map.", seq_id );
		return CMD_FAILURE;
	}	
	resourceDataChanged = true;
	return CMD_SUCCESS;
}

int mcuCBHandle::execute_seq_chain( const vector<string>& seq_names, const char* error_prefix ) {
	vector<string>::const_iterator it  = seq_names.begin();
	vector<string>::const_iterator end = seq_names.end();

	if( it == end ) {
		// No sequences -> NOOP
		return CMD_SUCCESS;
	}

	const string& first_seq_name = *it;  // convenience reference
	std::string fullPath;
	FILE* first_file_p = open_sequence_file( first_seq_name.c_str(), fullPath );
	if( first_file_p == NULL ) {
		if( error_prefix )
			LOG("%s Cannot find sequence \"%s\". Aborting seq-chain.", error_prefix, first_seq_name.c_str());
		return CMD_FAILURE;
	}

	srCmdSeq* seq_p = new srCmdSeq();
	int parse_result = seq_p->read_file( first_file_p );
	fclose( first_file_p );
	if( parse_result != CMD_SUCCESS ) {
		if( error_prefix )
			LOG("%s Unable to parse sequence\"%s\".", error_prefix, first_seq_name.c_str());

		delete seq_p;
		seq_p = NULL;

		return CMD_FAILURE;
	}

	// Test remaining sequence names, error early if invalid
	vector<string>::const_iterator second = ++it;
	for( ; it != end; ++it ) {
		const string& next_seq = *it;  // convenience reference

		std::string fullPath;
		FILE* file = open_sequence_file( next_seq.c_str(), fullPath );
		if( file == NULL ) {
			if( error_prefix )
				LOG("%s Cannot find sequence \"%s\". Aborting seq-chain.", error_prefix, next_seq.c_str() );
			return CMD_FAILURE;
		} else {
			fclose( file );
		}
	}

	if( second != end ) {  // has more than one seq_name
		// Append new seq-chian command of remaining seq_names at end of seq_p
		float time = seq_p->duration();

		// Start from second
		it = second;

		// build command
		ostringstream oss;
		oss << "seq-chain";
		for( ; it != end; ++it )
			oss << ' ' << (*it);

		// insert command or error with cleanup
		int result = seq_p->insert( time, oss.str().c_str() );
		if( result != CMD_SUCCESS ) {
			if( error_prefix )
				LOG("%s Failed to insert seq-chain command at time %f", error_prefix, time);

			delete seq_p;
			seq_p = NULL;

			return CMD_FAILURE;
		}
	}

	execute_seq( seq_p, first_seq_name.c_str() );

	return CMD_SUCCESS;
}

int mcuCBHandle::execute_later( const char* command, float seconds ) {
	srCmdSeq *temp_seq = new srCmdSeq();
	temp_seq->insert( (float)time+seconds, command );

	ostringstream seqName;
	seqName << "execute_later-" << (++queued_cmds);

	return execute_seq( temp_seq, seqName.str().c_str() );;
}

int mcuCBHandle::abort_seq( const char* seq_name ) {
	srCmdSeq* seq_p = active_seq_map.remove( seq_name );
	if( seq_p == NULL )	{
		return CMD_FAILURE;  // Not Found
	}

	if( LOG_ABORTED_SEQ ) {
		cout << "DEBUG: mcuCBHandle::abort_seq(..): Aborting seq \"" << seq_name << "\" @ " << time << endl;
		if( seq_p->get_count() > 0 ) {
			cout << "\tRemaining commands:" << endl;

			const float offset = seq_p->offset();
			float time = 0;

			const char* cmd = seq_p->pull( &time );
			while( cmd != NULL ) {
				// print offseted time, so the values are comparable to the MCU abort time
				cout << "\ttime " << (time+offset) << ":\t"<< cmd << endl;
				cmd = seq_p->pull( &time );
				delete [] cmd;
			}
		}
	}

	srCmdSeq* pending_p = pending_seq_map.lookup( seq_name );
	if( pending_p != seq_p )	{
		delete seq_p;
	}

	return CMD_SUCCESS;  // Aborted successfully
}


int mcuCBHandle::delete_seq( const char* seq_name ) {
	int result = abort_seq( seq_name );

	srCmdSeq* seq_p = pending_seq_map.remove( seq_name );
	if( seq_p != NULL )	{
		delete seq_p;
		result = CMD_SUCCESS;
	}

	return result;
}

void mcuCBHandle::set_net_host( const char * net_host )
{
	// EDF
	// Sets up the network connection for sending bone rotations over to Unreal
	this->net_host = net_host;
	bonebus.OpenConnection( net_host );
	bonebus.UpdateAllCharacters();
}

void mcuCBHandle::set_process_id( const char * process_id )
{
	this->process_id = process_id;
}

int mcuCBHandle::vhmsg_send( const char *op, const char* message ) {
#if LINK_VHMSG_CLIENT
	//std::cout<<"Sending :" << cmdName << ' ' << cmdArgs <<std::endl;

	if( vhmsg_enabled ) {
		int err = vhmsg::ttu_notify2( op, message );
		if( err != vhmsg::TTU_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify2 failed on message \"" << op << "  " << message << "\"." << std::endl;
			LOG(strstr.str().c_str());
		}
	} else {
		// append to command queue if header token has callback function
		srArgBuffer tokenizer( message );
		char* token = tokenizer.read_token();
		if( cmd_map.is_command( op ) ) {
			// Append to command queue
			ostringstream command;
			command << op << " " << message;
			execute_later( command.str().c_str() );
		}
	}
#else
	// append to command queue if header token has callback function
	srArgBuffer tokenizer( message );
	char* token = tokenizer.read_token();
	if( cmd_map.is_command( op ) ) {
		// Append to command queue
		ostringstream command;
		command << op << " " << message;
		execute_later( command.str().c_str() );
	}
#endif
	return( CMD_SUCCESS );
}

int mcuCBHandle::vhmsg_send( const char* message ) {
#if LINK_VHMSG_CLIENT
	//std::cout<<"Sending :" << cmdName << ' ' << cmdArgs <<std::endl;

	if( vhmsg_enabled ) {
		int err = vhmsg::ttu_notify1( message );
		if( err != vhmsg::TTU_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify1 failed on message \"" << message << "\"." << std::endl;
			LOG(strstr.str().c_str());
		}
	} else {
		// append to command queue if header token has callback function
		srArgBuffer tokenizer( message );
		char* token = tokenizer.read_token();
		if( cmd_map.is_command( token ) ) {
			// Append to command queue
			execute_later( message );
		}
	}
#else
	// append to command queue if header token has callback function
	srArgBuffer tokenizer( message );
	char* token = tokenizer.read_token();
	if( cmd_map.is_command( token ) ) {
		// Append to command queue
		execute_later( message );
	}
#endif
	return( CMD_SUCCESS );
}

int mcuCBHandle::load_motions( const char* pathname, bool recursive ) {
	return load_me_motions( pathname, motion_map, recursive, resource_manager, skmScale );
}

int mcuCBHandle::load_skeletons( const char* pathname, bool recursive ) {
	return load_me_skeletons( pathname, skeleton_map, recursive, resource_manager, skmScale );
}

int mcuCBHandle::load_poses( const char* pathname, bool recursive ) {
	return load_me_postures( pathname, pose_map, recursive, resource_manager, skmScale );
}

//  Usage example: mcu_p->lookup_ctrl( ctrl_name, "ERROR: ctrl <controller name>: " );
MeController* mcuCBHandle::lookup_ctrl( const string& ctrl_name, const char* print_error_prefix  ) {
	MeController* ctrl_p;
	if( ctrl_name[0]=='~' ) {  // Referenced relative to a character
		string::size_type index = ctrl_name.find( "/" );
		if( index == string::npos ) {
			if( print_error_prefix )
				LOG("%s Invalid controller name \"%s\".  Missing '/' after character name.", print_error_prefix, ctrl_name.c_str());
			return NULL;
		}
		const string char_name( ctrl_name, 1, index-1 );
		if( char_name.length() == 0 ) {
			if( print_error_prefix )
				LOG("%s Invalid controller name \"%s\".  Empty character name.", print_error_prefix, ctrl_name.c_str() );
			return NULL;
		}

		SbmCharacter* char_p = getCharacter(char_name);
		if( char_p == NULL ) {
			if( print_error_prefix )
				LOG("%s Unknown character \"%s\" in controller reference \"%s\"", print_error_prefix, char_name.c_str(), ctrl_name.c_str());
			return NULL;
		}

		++index; // character after slash
		if( index == ctrl_name.length() ) {  // slash was the last character
			if( print_error_prefix )
				LOG("%s Invalid controller name \"%s\". Missing controller name after character.", print_error_prefix, ctrl_name.c_str());
			return NULL;
		}
		const string ctrl_subname( ctrl_name, index );

		if( ctrl_subname == "posture_sched" ) {
			ctrl_p = char_p->posture_sched_p;
		} else if( ctrl_subname == "motion_sched" ) {
			ctrl_p = char_p->motion_sched_p;
		} else if( ctrl_subname == "gaze_sched" ) {
			ctrl_p = char_p->gaze_sched_p;
		} else if( ctrl_subname == "head_sched" ) {
			ctrl_p = char_p->head_sched_p;
		} else if( ctrl_subname == "param_sched" ) {
			ctrl_p = char_p->param_sched_p;
		} else {
			// TODO: Character specific hash map?

			if( print_error_prefix )
			{
				std::stringstream strstr;
				strstr <<print_error_prefix<<"Unknown controller \""<<ctrl_subname<<"\" relative to character \""<<char_name<<"\".";
				LOG(strstr.str().c_str());
			}
				
			return NULL;
		}
	} else {
		ctrl_p = controller_map.lookup( ctrl_name.c_str() );
		if( ctrl_p == NULL ) {
			if( print_error_prefix )
				LOG("%s Unknown controller %s.", print_error_prefix, ctrl_name.c_str());
			return NULL;
		}
	}
	return ctrl_p;
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

	for ( size_t i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];
		if (j->getJointType() != SkJoint::TypeJoint)
			continue;

		const SrQuat& q = j->quat()->value();

		character->AddBoneRotation( j->name().c_str(), q.w, q.x, q.y, q.z, time );

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
		character->AddBonePosition( j->name().c_str(), posx, posy, posz, time );
	}

	character->EndSendBonePositions();

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
#ifdef USE_PYTHON
	try {
		FILE* file = fopen(filename, "r");
		if (file)
		{
			PyRun_SimpleFile(file, filename);
			PyErr_Print();
			PyErr_Clear();
			fclose(file);
			return CMD_SUCCESS;
		}
		else
		{
			if (file)
				fclose(file);
			return CMD_FAILURE;
		}
	} catch (...) {
		PyErr_Print();
	}
#endif
	return CMD_FAILURE;
}

int mcuCBHandle::executePython(const char* command)
{
#ifdef USE_PYTHON
	try {
		CmdResource* resource = new CmdResource();
		resource->setChildrenLimit(resource_manager->getLimit());	// assuming the limit of total resources( path, motion, file, command) is the same with the limit of children ( command resource only) number
		resource->setCommand(command);
		resource_manager->addResource(resource);

		PyRun_SimpleString(command);

		return CMD_SUCCESS;
	} catch (...) {
		PyErr_Print();
	}
#endif
	return CMD_FAILURE;
}


void mcuCBHandle::setMediaPath(std::string path)
{
	media_path = path;
	// update all the paths with the media path prefix
	seq_paths.setPathPrefix(media_path);
	me_paths.setPathPrefix(media_path);
	audio_paths.setPathPrefix(media_path);
	mesh_paths.setPathPrefix(media_path);
}

const std::string& mcuCBHandle::getMediaPath()
{
	return media_path;
}

SkMotion* mcuCBHandle::lookUpMotion( const char* motionName )
{
	SkMotion* anim_p = NULL;
	std::map<std::string, SkMotion*>::iterator animIter = motion_map.find(motionName);
	if (animIter != motion_map.end())
		anim_p = (*animIter).second;
	return anim_p;
}

PAStateData* mcuCBHandle::lookUpPAState(std::string stateName)
{
	for (size_t i = 0; i < param_anim_states.size(); i++)
	{
		if (param_anim_states[i]->stateName == stateName)
			return param_anim_states[i];
	}
	return NULL;
}

void mcuCBHandle::addPAState(PAStateData* state)
{
	if (!lookUpPAState(state->stateName))
		param_anim_states.push_back(state);
}

PATransitionData* mcuCBHandle::lookUpPATransition(std::string fromStateName, std::string toStateName)
{
	for (size_t i = 0; i < param_anim_transitions.size(); i++)
	{
		if (param_anim_transitions[i]->fromState->stateName == fromStateName && param_anim_transitions[i]->toState->stateName == toStateName)
			return param_anim_transitions[i];
	}
	return NULL;	
}

void mcuCBHandle::addPATransition(PATransitionData* transition)
{
	if (!lookUpPATransition(transition->fromState->stateName, transition->toState->stateName))
		param_anim_transitions.push_back(transition);
}

bool mcuCBHandle::checkExamples()
{
	std::vector<std::string> requiredStates;
	requiredStates.push_back("UtahLocomotion");
	requiredStates.push_back("UtahStartingLeft");
	requiredStates.push_back("UtahStartingRight");
	requiredStates.push_back("UtahStopToWalk");
	requiredStates.push_back("UtahWalkToStop");
	requiredStates.push_back("UtahIdleTurnLeft");
	requiredStates.push_back("UtahIdleTurnRight");
	requiredStates.push_back("UtahStep");

	int numMissing = 0;
	for (size_t x = 0; x < requiredStates.size(); x++)
	{
		PAStateData* state = lookUpPAState(requiredStates[x]);
		if (!state)
		{
			numMissing++;
			LOG("SteeringAgent::checkExamples() Failure: Could not find state '%s' needed for example-based locomotion.", requiredStates[x].c_str());
		}
	}

	if (numMissing > 0)
		return false;
	else
		return true;
}

void mcuCBHandle::setInteractive(bool val)
{
    _interactive = val;
}

bool mcuCBHandle::getInteractive()
{
    return _interactive;
}

void mcuCBHandle::setPhysicsEngine( bool start )
{
	if (start)
	{
		physicsTime = time;
		//updatePhysics = true;		
	}
	else
	{
		//updatePhysics = false;
	}

	if (physicsEngine)
	{
		physicsEngine->setEnable(start);
	}
}

std::vector<MeController*>& mcuCBHandle::getDefaultControllers()
{
	return _defaultControllers;
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

void mcuCBHandle::removePawn(std::string name)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(name);
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}
}

SbmPawn* mcuCBHandle::getPawn(std::string name)
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

bool mcuCBHandle::addCharacter(SbmCharacter* character)
{
	SbmCharacter* c = getCharacter(character->getName());
	if (!c)
	{
		character_map[character->getName()] = character;
		return true;
	}
	else
	{
		return false;
	}
}

void mcuCBHandle::removeCharacter(std::string name)
{
	std::map<std::string, SbmCharacter*>::iterator iter = character_map.find(name);
	if (iter != character_map.end())
	{
		character_map.erase(iter);
	}
}

SbmCharacter* mcuCBHandle::getCharacter(std::string name)
{
	std::map<std::string, SbmCharacter*>::iterator iter = character_map.find(name);
	if (iter == character_map.end())
		return NULL;
	else
		return (*iter).second;
}

int mcuCBHandle::getNumCharacters()
{
	return character_map.size();
}

SkMotion* mcuCBHandle::getMotion(std::string motionName)
{
	std::map<std::string, SkMotion*>::iterator iter = this->motion_map.find(motionName);
	if (iter == this->motion_map.end())
		return NULL;
	else
		return (*iter).second;
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

int mcuCBHandle::registerCharacter(SbmCharacter* character)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(character->getName());
	if (iter != pawn_map.end())
	{
		LOG( "Register character: pawn_map.insert(..) '%s' FAILED\n", character->getName().c_str() ); 
		return( CMD_FAILURE );
	}

	pawn_map.insert(std::pair<std::string, SbmPawn*>(character->getName(), character));
	

	std::map<std::string, SbmCharacter*>::iterator citer = character_map.find(character->getName());
	if (citer != character_map.end())
	{
		LOG( "Register character: character_map.insert(..) '%s' FAILED\n", character->getName().c_str() );
		pawn_map.erase(iter);
		return( CMD_FAILURE );
	}
	character_map.insert(std::pair<std::string, SbmCharacter*>(character->getName(), character));

	if (net_bone_updates)
		mcuCBHandle::singleton().bonebus.CreateCharacter( character->getName().c_str(), character->getClassType().c_str(), mcuCBHandle::singleton().net_face_bones );
	if ( mcuCBHandle::singleton().sbm_character_listener )
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterCreate( character->getName().c_str(), character->getClassType() );

	return 1;
}

int mcuCBHandle::unregisterCharacter(SbmCharacter* character)
{
	std::map<std::string, SbmPawn*>::iterator iter = pawn_map.find(character->getName());
	if (iter != pawn_map.end())
	{
		pawn_map.erase(iter);
	}

	std::map<std::string, SbmCharacter*>::iterator citer = character_map.find(character->getName());
	if (citer != character_map.end())
	{
		character_map.erase(citer);
	}

	//root_group_p->remove(character->scene_p); 

	return 1;
}

void mcuCBHandle::addNvbg(std::string id, Nvbg* nvbg)
{
	std::map<std::string, Nvbg*>::iterator iter = nvbgMap.find(id);
	if (iter != nvbgMap.end())
	{
		removeNvbg(id);
	}
	nvbgMap[id] = nvbg;
}

void mcuCBHandle::removeNvbg(std::string id)
{
	std::map<std::string, Nvbg*>::iterator iter = nvbgMap.find(id);
	if (iter != nvbgMap.end())
	{
		delete (*iter).second;
	}
}

Nvbg* mcuCBHandle::getNvbg(std::string id)
{
	std::map<std::string, Nvbg*>::iterator iter = nvbgMap.find(id);
	if (iter != nvbgMap.end())
	{
		return (*iter).second;
	}
	else
	{
		return NULL;
	}
}



/////////////////////////////////////////////////////////////
