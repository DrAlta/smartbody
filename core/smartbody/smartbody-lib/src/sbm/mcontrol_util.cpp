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

#include "vhcl.h"
#include "mcontrol_util.h"

#include <stdlib.h>
#include <iostream>
#include <string>
#include <direct.h>

#include "sbm_audio.h"

#include "me_utilities.hpp"
#include "wsp.h"
#include "sr/sr_model.h"
#include "sbm/GPU/SbmShader.h"
#include "sbm_deformable_mesh.h"
#include "sbm/Physics/SbmPhysicsSimODE.h"

#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace WSP;

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
	internal_profiler_p( NULL ),
	external_profiler_p( NULL ),
	profiler_p( NULL ),
	net_bone_updates( true ),
	net_world_offset_updates( true ),
	net_face_bones( false ),
	net_host( NULL ),
	sbm_character_listener( NULL ),
	play_internal_audio( false ),
	skScale( 1.0 ),
	skmScale( 1.0 ),
	viewer_p( NULL ),
	bmlviewer_p( NULL ),
	panimationviewer_p( NULL ),
	channelbufferviewer_p( NULL ),
	camera_p( NULL ),
	root_group_p( new SrSnGroup() ),
	height_field_p( NULL ),
	logger_p( new joint_logger::EvaluationLogger() ),
	test_character_default( "" ),
	test_recipient_default( "ALL" ),
	queued_cmds( 0 ),
	use_locomotion( false ),
	use_param_animation( false ),
	updatePhysics( false ),
	locomotion_type( Basic ),
	viewer_factory ( new SrViewerFactory() ),
	bmlviewer_factory ( new GenericViewerFactory() ),
	panimationviewer_factory ( new GenericViewerFactory() ),
	channelbufferviewer_factory ( new GenericViewerFactory() ),
	commandviewer_factory ( new GenericViewerFactory() ),
	resource_manager(ResourceManager::getResourceManager()),
	snapshot_counter( 1 ),
	delay_behaviors(true),
	media_path("."),
	_interactive(true),
	steerEngine(NULL)
	//physicsEngine(NULL)
{

	
	root_group_p->ref();
	logger_p->ref();

	theWSP = WSP::create_manager();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );

	// initialize the default face motion mappings
	face_map["_default_"] = new FaceMotion();
	physicsEngine = new SbmPhysicsSimODE();
	physicsEngine->initSimulation();
}

/////////////////////////////////////////////////////////////

mcuCBHandle::~mcuCBHandle() {
	clear();
}

void mcuCBHandle::reset( void )	{
	clear();

	// reset initial variables to match the constructor.
	loop = true;
	time = 0.0;
	net_bone_updates = true;
	use_locomotion = false;
	root_group_p = new SrSnGroup();
	root_group_p->ref();
	logger_p = new joint_logger::EvaluationLogger();
	logger_p->ref();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );

	if ( net_host )
		bonebus.OpenConnection( net_host );
}

FILE* mcuCBHandle::open_sequence_file( const char *seq_name ) {

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

	for (std::map<std::string, FaceMotion*>::iterator iter = this->face_map.begin();
		iter != face_map.end();
		iter++)
	{
		FaceMotion* face = (*iter).second;
		VisemeMotionMap::iterator vis_it = face->viseme_map.begin();
		VisemeMotionMap::iterator vis_end = face->viseme_map.end();
		for( ; vis_it != vis_end; ++vis_it ) {
			if (vis_it->second)
				vis_it->second->unref();  // unref SkMotion
		}
		face->viseme_map.clear();
		face->au_motion_map.clear();

		
		if (face->face_neutral_p)
		{
			face->face_neutral_p->unref();
			face->face_neutral_p = NULL;
		}
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

	SbmPawn* pawn_p;
	character_map.reset();
	while( pawn_p = character_map.pull() )	{ 
		// characters are referenced by both maps
		character_map.remove( pawn_p->name );
		pawn_map.remove( pawn_p->name );
		delete pawn_p;
	}
	pawn_map.reset();
	while( pawn_p = pawn_map.pull() ) {
		delete pawn_p;
	}
	
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
	
	if( root_group_p )	{
		root_group_p->unref();
		root_group_p = NULL;
	}

	if( logger_p ) {
		logger_p->unref();
		logger_p = NULL;
	}
	if (steerEngine)
		delete steerEngine;

	//close_viewer();

	if ( net_host )
		bonebus.CloseConnection();

	theWSP->shutdown();

}

/////////////////////////////////////////////////////////////

int mcuCBHandle::open_viewer( int width, int height, int px, int py )	{	
	
	if( viewer_p == NULL )	{
		viewer_p = viewer_factory->create( px, py, width, height );
		viewer_p->label_viewer( "SBM Viewer" );
		camera_p = new SrCamera;
		viewer_p->set_camera( *camera_p );
		//((FltkViewer*)viewer_p)->set_mcu(this);
		viewer_p->show_viewer();
		if( root_group_p )	{
			viewer_p->root( root_group_p );
		}
		SbmShaderManager::singleton().setViewer(viewer_p);
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_viewer( void )	{

	if( viewer_p )	{
		viewer_factory->remove(viewer_p);
		viewer_p = NULL;
		SbmShaderManager::singleton().setViewer(NULL);
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
		panimationviewer_factory->destroy(bmlviewer_p);
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
		channelbufferviewer_factory->destroy(bmlviewer_p);
		channelbufferviewer_p = NULL;
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
	if (steerEngine)
	{
		if (!this->steerEngine->isDone())
		{
			if (this->steerEngine->getStartTime() == 0.0f)
				this->steerEngine->setStartTime(float(this->time));
	
			SbmCharacter* character;
			character_map.reset();
			while (character = character_map.next())
				character->steeringAgent->evaluate();

			bool running = this->steerEngine->_engine->update(false, true, float(this->time) - this->steerEngine->getStartTime());
			if (!running)
				this->steerEngine->setDone(true);
		}
	}

	if (physicsEngine && updatePhysics)
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

	SbmShaderManager& ssm = SbmShaderManager::singleton();
	bool hasOpenGL        = ssm.initOpenGL();
	bool hasShaderSupport = false;

	// init OpenGL extension
	if (hasOpenGL)
		hasShaderSupport = ssm.initGLExtension();
	// update the shader map
	if (hasShaderSupport)
		ssm.buildShaders();
	

	SbmPawn* pawn_p;
	SbmCharacter* char_p;
	pawn_map.reset();
	while( pawn_p = pawn_map.next() )	{

		pawn_p->reset_all_channels();
		pawn_p->ct_tree_p->evaluate( time );
		pawn_p->ct_tree_p->applyBufferToAllSkeletons();


		if (pawn_p->hasPhysicsSim() && updatePhysics)
		{
			pawn_p->updateFromColObject();
		}
		else
		{			
			pawn_p->updateToColObject();
			pawn_p->updateToSteeringSpaceObject();
		}

		char_p = character_map.lookup( pawn_p->name );
		if (!char_p)
		{
			NetworkSendSkeleton( pawn_p->bonebusCharacter, pawn_p->skeleton_p, &param_map );
		}
		if( char_p ) {

			char_p->forward_visemes( time );	
			char_p->scene_p->update();	
			char_p->updateJointPhyObjs();
			//char_p->dMesh_p->update();

			if ( net_bone_updates && char_p->skeleton_p && char_p->bonebusCharacter ) {
				NetworkSendSkeleton( char_p->bonebusCharacter, char_p->skeleton_p, &param_map );

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
				}
			}
		}  // end of char_p processing
	} // end of loop

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
		FILE* file_p = open_sequence_file( seq_name );
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
	FILE* first_file_p = open_sequence_file( first_seq_name.c_str() );
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

		FILE* file = open_sequence_file( next_seq.c_str() );
		if( file == NULL ) {
			if( error_prefix )
				LOG("%s Cannot find sequence \"%s\". Aborting seq-chain.", error_prefix, next_seq);
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
			strstr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify2 failed on message \"" << op << '  ' << message << "\"." << std::endl;
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
				LOG("%s Invalid controller name \"%s\".  Empty character name.", print_error_prefix, ctrl_name);
			return NULL;
		}

		SbmCharacter* char_p = character_map.lookup( char_name.c_str() );
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
	const SrArray<SkJoint *> & joints  = skeleton->joints();

	character->StartSendBoneRotations();

	for ( int i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];

		SrQuat q = j->quat()->value();

		character->AddBoneRotation( j->name(), q.w, q.x, q.y, q.z, time );

		//printf( "%s %f %f %f %f\n", (const char *)j->name(), q.w, q.x, q.y, q.z );
	}

	character->EndSendBoneRotations();


	character->StartSendBonePositions();

	for ( int i = 0; i < joints.size(); i++ )
	{
		SkJoint * j = joints[ i ];

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
		character->AddBonePosition( j->name(), posx, posy, posz, time );
	}

	character->EndSendBonePositions();

	// Passing General Parameters
	character->StartSendGeneralParameters();
	for (int i = 0; i < joints.size(); i++)
	{
		SkJoint* j = joints[ i ];
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
							if(_stricmp( j->name(), joint_name.str().c_str()) == 0)
								character->AddGeneralParameters(index, pos->second->size, j->pos()->value(0), m, time);
						}
					}
				}
			}
		}
	}
	character->EndSendGeneralParameters();
}

void mcuCBHandle::setMediaPath(std::string path)
{
	media_path = path;
	// update all the paths with the media path prefix
	seq_paths.setPathPrefix(media_path);
	me_paths.setPathPrefix(media_path);
	audio_paths.setPathPrefix(media_path);
}

std::string mcuCBHandle::getMediaPath()
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
		updatePhysics = true;
	}
	else
	{
		updatePhysics = false;
	}
}
/////////////////////////////////////////////////////////////
