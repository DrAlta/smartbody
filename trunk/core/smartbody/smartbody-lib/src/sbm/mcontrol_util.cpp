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

#include "sbm_audio.h"

#include "me_utilities.hpp"
#include "wsp.h"


using namespace std;
using namespace WSP;




const bool LOG_ABORTED_SEQ = false;


/////////////////////////////////////////////////////////////
//  Singleton Instance
mcuCBHandle* mcuCBHandle::_singleton = NULL;


/////////////////////////////////////////////////////////////

mcuCBHandle::mcuCBHandle()
:	loop( true ),
	vhmsg_enabled( false ),
	lock_dt( false ),
	time( 0.0 ),
	perf( 10.0 ),
	net_bone_updates( true ),
	net_world_offset_updates( true ),
	net_face_bones( false ),
	net_host( NULL ),
	process_id( NULL ),
	play_internal_audio( false ),
	viewer_p( NULL ),
	camera_p( NULL ),
	root_group_p( new SrSnGroup() ),
	face_neutral_p( NULL ),
	logger_p( new joint_logger::EvaluationLogger() ),
	test_character_default( "" ),
	test_recipient_default( "ALL" ),
	queued_cmds( 0 )
{
	
	root_group_p->ref();
	logger_p->ref();

	theWSP = WSP::create_manager();

	// TODO: this needs to have a unique name so that multiple sbm
	// processes will be identified differently
	theWSP->init( "SMARTBODY" );
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

void mcuCBHandle::set_time( double real_time )	{

	if( lock_dt )	{
		time += 1.0/desired_max_fps;
	}
	else	{
		time = real_time;
	}
	perf.update( real_time, time );
}

FILE* mcuCBHandle::open_sequence_file( const char *seq_name ) {
	FILE* file_p = NULL;

	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];
	sprintf( label, "%s", seq_name );

	seq_paths.reset();
	char* filename = seq_paths.next_filename( buffer, label );
	while( filename != NULL )	{
		file_p = fopen( filename, "r" );
		if( file_p != NULL )
			break;
		filename = seq_paths.next_filename( buffer, label );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension
		sprintf( label, "%s.seq", seq_name );

		seq_paths.reset();
		filename = seq_paths.next_filename( buffer, label );
		while( filename )	{
			if( ( file_p = fopen( filename, "r" ) ) != NULL )
				break;
			filename = seq_paths.next_filename( buffer, label );
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
	VisemeMotionMap::iterator vis_it = viseme_map.begin();
	VisemeMotionMap::iterator vis_end = viseme_map.end();
	for( ; vis_it != vis_end; ++vis_it ) {
		vis_it->second->unref();  // unref SkMotion
	}
	viseme_map.clear();

	au_motion_map.clear();

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
	
	SkPosture* pose_p;
	pose_map.reset();
	while( pose_p = pose_map.pull() )	{
		pose_p->unref();
	}
	
	SkMotion* mot_p;
	motion_map.reset();
	while( mot_p = motion_map.pull() )	{
		mot_p->unref();
	}
	
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

	if( face_neutral_p ) {
		face_neutral_p->unref();
		face_neutral_p = NULL;
	}

	if( logger_p ) {
		logger_p->unref();
		logger_p = NULL;
	}

	close_viewer();

	if ( net_host )
		bonebus.CloseConnection();

	theWSP->shutdown();
}

/////////////////////////////////////////////////////////////

int mcuCBHandle::open_viewer( int width, int height, int px, int py )	{
	
	if( viewer_p == NULL )	{
		viewer_p = new SrViewer( px, py, width, height );
		viewer_p->label( "SBM Viewer" );
		camera_p = new SrCamera;
		viewer_p->set_camera( *camera_p );
		viewer_p->show();
		if( root_group_p )	{
			viewer_p->root( root_group_p );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

void mcuCBHandle::close_viewer( void )	{

	if( viewer_p )	{
		delete viewer_p;
		viewer_p = NULL;
	}
	if( camera_p )	{
		delete camera_p;
		camera_p = NULL;
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

void mcuCBHandle::update( void )	{

#if 0
	static int c = 3;
	if( c )	{
		test_map();
		c--;
	}
#endif

	srCmdSeq* seq_p;
	active_seq_map.reset();
	while( seq_p = active_seq_map.next() )	{
		char *cmd;
		while( cmd = seq_p->pop( (float)time ) )	{
			int err = execute( cmd );
			if( err != CMD_SUCCESS )	{
				printf( "mcuCBHandle::update ERR: execute FAILED: '%s'\n", cmd );
			}
			delete [] cmd;
		}
	}

	SbmPawn* pawn_p;
	SbmCharacter* char_p;
	pawn_map.reset();
	while( pawn_p = pawn_map.next() )	{

		//char_p->scheduler_p->evaluate( time );
		pawn_p->pipeline_p->evaluate( time );
		pawn_p->pipeline_p->applyBufferToAllSkeletons();

		char_p = character_map.lookup( pawn_p->name );
		if( char_p != NULL ) {
			//char_p->scheduler_p->apply();  // old controller API  See applyBufferToAllSkeletons() above
			char_p->scene_p->update();

			if ( net_bone_updates && char_p->skeleton_p && char_p->bonebusCharacter ) {
				NetworkSendSkeleton( char_p->bonebusCharacter, char_p->skeleton_p );

				// what a lot of hoop jumping...
				if ( net_world_offset_updates ) {
					const SkJoint * joint = char_p->get_world_offset_joint();

					const SkJointPos * pos = joint->const_pos();
					float x = pos->value( SkJointPos::X );
					float y = pos->value( SkJointPos::Y );
					float z = pos->value( SkJointPos::Z );

					SkJoint::RotType rot_type = joint->rot_type();
					if ( rot_type != SkJoint::TypeQuat ) {
						//cerr << "ERROR: Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")"<<endl;
					}

					// const_cast because the SrQuat does validation (no const version of value())
					const SrQuat & q = ((SkJoint *)joint)->quat()->value();

					// yet another wacky mapping between sbm coordinate system and unreal
					//NetworkSetPosition( char_p->net_handle, z, -x, y );
					//NetworkSetRotation( char_p->net_handle, (float)q.w, (float)q.z, -(float)q.x, (float)q.y );
					char_p->bonebusCharacter->SetPosition( z, -x, y, time );
					char_p->bonebusCharacter->SetRotation( (float)q.w, (float)q.z, -(float)q.x, (float)q.y, time );
				}
			}

			char_p->eye_blink_update( this->time );
		}  // end of char_p processing
	} // end of loop
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
				fprintf( stderr, "ERROR: mcuCBHandle::lookup_seq(..): '%s' PARSE FAILED\n", seq_name ); 

				delete seq_p;
				seq_p = NULL;
			}
		} else {
			fprintf( stderr, "ERROR: mcuCBHandle::lookup_seq(..): '%s' NOT FOUND\n", seq_name ); 
		}
	}
	
	return( seq_p );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq ) {
	ostringstream seq_id;
	seq_id << "execute_seq-" << (++queued_cmds);

	return execute_seq( seq, seq_id.str().c_str() );
}

int mcuCBHandle::execute_seq( srCmdSeq* seq, const char* seq_id ) {
	if ( active_seq_map.insert( seq_id, seq ) != CMD_SUCCESS ) {
		cerr << "ERROR: mcuCBHandle::execute_seq(..): Failed to insert srCmdSeq \"" << seq_id << "\" into active_seq_map." << endl;
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
			cerr << error_prefix << "Cannot find sequence \"" << first_seq_name << "\". Aborting seq-chain." << endl;
		return CMD_FAILURE;
	}

	srCmdSeq* seq_p = new srCmdSeq();
	int parse_result = seq_p->read_file( first_file_p );
	fclose( first_file_p );
	if( parse_result != CMD_SUCCESS ) {
		if( error_prefix )
			cerr << error_prefix << "Unable to parse sequence \"" << first_seq_name << "\"." << endl;

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
				cerr << error_prefix << "Cannot find sequence \"" << next_seq << "\". Aborting seq-chain." << endl;
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
				cerr << error_prefix << "Failed to insert seq-chain command at time "<<time<<endl;

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
		int err = ttu_notify2( op, message );
		if( err != TTU_SUCCESS )	{
			std::cerr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify2 failed on message \"" << op << '  ' << message << "\"." << std::endl;
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
		int err = ttu_notify1( message );
		if( err != TTU_SUCCESS )	{
			std::cerr << "ERROR: mcuCBHandle::vhmsg_send(..): ttu_notify1 failed on message \"" << message << "\"." << std::endl;
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
	return load_me_motions( pathname, motion_map, recursive );
}

int mcuCBHandle::load_poses( const char* pathname, bool recursive ) {
	return load_me_postures( pathname, pose_map, recursive );
}

//  Usage example: mcu_p->lookup_ctrl( ctrl_name, "ERROR: ctrl <controller name>: " );
MeController* mcuCBHandle::lookup_ctrl( const string& ctrl_name, const char* print_error_prefix  ) {
	MeController* ctrl_p;
	if( ctrl_name[0]=='~' ) {  // Referenced relative to a character
		string::size_type index = ctrl_name.find( "/" );
		if( index == string::npos ) {
			if( print_error_prefix )
				cerr << print_error_prefix<<"Invalid controller name \""<<ctrl_name<<"\".  Missing '/' after character name." << endl;
			return NULL;
		}
		const string char_name( ctrl_name, 1, index-1 );
		if( char_name.length() == 0 ) {
			if( print_error_prefix )
				cerr <<print_error_prefix<<"Invalid controller name \""<<ctrl_name<<"\".  Empty character name." << endl;
			return NULL;
		}

		SbmCharacter* char_p = character_map.lookup( char_name.c_str() );
		if( char_p == NULL ) {
			if( print_error_prefix )
				cerr <<print_error_prefix<<"Unknown character \""<<char_name<<"\" in controller reference \""<<ctrl_name<<"\"." << endl;
			return NULL;
		}

		++index; // character after slash
		if( index == ctrl_name.length() ) {  // slash was the last character
			if( print_error_prefix )
				cerr <<print_error_prefix<<"Invalid controller name \""<<ctrl_name<<"\".  Missing controller name after character." << endl;
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
		} else {
			// TODO: Character specific hash map?

			if( print_error_prefix )
				cerr <<print_error_prefix<<"Unknown controller \""<<ctrl_subname<<"\" relative to character \""<<char_name<<"\"." << endl;
			return NULL;
		}
	} else {
		ctrl_p = controller_map.lookup( ctrl_name.c_str() );
		if( ctrl_p == NULL ) {
			if( print_error_prefix )
				cerr <<print_error_prefix<<"Unknown controller \""<<ctrl_name<<"\"." << endl;
			return NULL;
		}
	}
	return ctrl_p;
}


void mcuCBHandle::NetworkSendSkeleton( BoneBusCharacter * character, SkSkeleton * skeleton )
{
	// Send the bone rotation for each joint in the skeleton (To be optimized soon)
   const SrArray<SkJoint*> & joints  = skeleton->joints();


   if ( character )
   {
      //NetworkStartSendBoneRotations( handle );
      character->StartSendBoneRotations();
   }

	int i;
	for ( i = 0; i < joints.size(); i++ )
	{
		SkJoint * j	= joints[ i ];

		//SrMat m;
		SrQuat q = j->quat()->value();
		//q.get_mat( m );

		//float posx, posy, posz;
		//posx = j->pos()->value( 0 );
		//posy = j->pos()->value( 1 );
		//posz = j->pos()->value( 2 );

		// for old doctor skeleton
		//SendMEBoneRotation( char_name,j->name(), q.w, -q.z, q.x, -q.y );


		// for new doctor skeleton
		if ( _stricmp( j->name(), "base" ) == 0 )
		{
         //NetworkAddBulkRotation( handle, j->name(), q.w, q.x, -q.y, q.z );
         character->AddBoneRotation( j->name(), q.w, q.x, -q.y, q.z, time );
			//SendMEBonePosition( char_name,j->name(), posx, -posy, posz );

			//printf( "%s %f %f %f\n", (const char *)j->name(),     posx, -posy, posz );
		}
		else
		{
         //NetworkAddBulkRotation( handle, j->name(), q.w, -q.x, q.y, -q.z );
         character->AddBoneRotation( j->name(), q.w, -q.x, q.y, -q.z, time );
		}
	}

   //NetworkEndSendBoneRotations( handle );
   character->EndSendBoneRotations();


   //NetworkStartSendBonePositions( handle );
   character->StartSendBonePositions();

	for ( i = 0; i < joints.size(); i++ )
	{
		SkJoint * j	= joints[ i ];

		float posx, posy, posz;
		posx = j->pos()->value( 0 );
		posy = j->pos()->value( 1 );
		posz = j->pos()->value( 2 );
		if( false ) {
			posx += j->offset().x;
			posy += j->offset().y;
			posz += j->offset().z;
		}

		if ( _stricmp( j->name(), "base" ) == 0 )
		{
         //NetworkAddBulkPosition( handle, j->name(), posx, -posy, posz );
         character->AddBonePosition( j->name(), posx, -posy, posz, time );
      }
        else
      {
		  
		  //these coordinates are meant to mimic the setpositionbyname coordinates you give to move the character
		  //so if you wanted to move a joint on the face in the x direction you'd do whatever you did to move the actor
		  //itself further in the x position.
		  //NetworkAddBulkPosition( handle, j->name(), -posz, -posy, posx );
		  //NetworkAddBulkPosition( handle, j->name(), posx, -posy, posz );
         character->AddBonePosition( j->name(), posx, -posy, posz, time );
	  }
   }

   //NetworkEndSendBonePositions( handle );
   character->EndSendBonePositions();
}


/////////////////////////////////////////////////////////////

/*

	path seq|me|bp <file-path>

*/

int mcu_filepath_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

    if( mcu_p )	{
		char *path_tok = args.read_token();
		char *path = args.read_token();
		
		if( strcmp( path_tok, "seq" ) == 0 )	{
			mcu_p->seq_paths.insert( path );
		}
		else
		if(
			( strcmp( path_tok, "me" ) == 0 )||
			( strcmp( path_tok, "ME" ) == 0 )
		)	{
			mcu_p->me_paths.insert( path );
		}
		else
		if(
			( strcmp( path_tok, "bp" ) == 0 )||
			( strcmp( path_tok, "BP" ) == 0 )
		)	{
			mcu_p->bp_paths.insert( path );
		}
		else	{
			printf( "mcu_filepath_func ERR: token '%s' NOT FOUND\n", path_tok );
			return( CMD_FAILURE );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

void flatten_inline_sequences( srCmdSeq *to_seq_p, srCmdSeq *fr_seq_p, mcuCBHandle *mcu_p )	{
	float t;
	char *cmd;
	
	fr_seq_p->reset();
	while( cmd = fr_seq_p->pull( & t ) )	{
		srCmdSeq *inline_seq_p = NULL;

		if( strncmp( cmd, "seq", 3 ) == 0 )	{
			srArgBuffer args( cmd );
			char *tok = args.read_token();
			if( strcmp( tok, "seq" ) == 0 )	{
				char *name = args.read_token();
				tok = args.read_token();
				if( strcmp( tok, "inline" ) == 0 )	{
					inline_seq_p = mcu_p->lookup_seq( name );
					if( inline_seq_p == NULL )	{
						printf( "flatten_inline_sequences ERR: inline seq '%s' NOT FOUND\n", name );
						return;
					}
				}
			}
		}
		
		float absolute_offset = fr_seq_p->offset() + t;
		if( inline_seq_p )	{
			delete [] cmd;
			// iterate hierarchy
			inline_seq_p->offset( absolute_offset );
			flatten_inline_sequences( to_seq_p, inline_seq_p, mcu_p );
		}
		else	{
			to_seq_p->insert_ref( absolute_offset, cmd );
		}
	}
	delete fr_seq_p;
}

/*

	seq <name> inline

*/

int begin_sequence( char* seq_name, mcuCBHandle *mcu_p )	{
	int err = CMD_FAILURE;
	
	srCmdSeq *seq_p = mcu_p->lookup_seq( seq_name );
	
	if( seq_p ) {
	
		// EXPAND INLINE SEQs HERE
#if 1
		srCmdSeq *cp_seq_p = new srCmdSeq;
		flatten_inline_sequences( cp_seq_p, seq_p, mcu_p );
		cp_seq_p->offset( (float)( mcu_p->time ) );
		err = mcu_p->active_seq_map.insert( seq_name, cp_seq_p );
#else	
		seq_p->offset( (float)( mcu_p->time ) );
		err = mcu_p->active_seq_map.insert( seq_name, seq_p );
#endif
		if( err != CMD_SUCCESS )	{
			printf( "begin_sequence ERR: insert active: '%s' FAILED\n", seq_name ); 
		}
	}

	return( err );
}

/*

	seq <name> at <time> <cmd...>
	seq <name> [begin|abort|print]
#	seq <name> write

*/

int mcu_sequence_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	int err;
	
	if( mcu_p )	{
		char *seq_name = args.read_token();
		char *seq_cmd = args.read_token();

		if( ( strcmp( seq_cmd, "begin" ) == 0 )||( strcmp( seq_cmd, EMPTY_STRING ) == 0 ) )	{
			return(
				begin_sequence( seq_name, mcu_p )
				);
		}
		else	{
		if( strcmp( seq_cmd, "at" ) == 0 )	{
		
			srCmdSeq *seq_p = mcu_p->pending_seq_map.lookup( seq_name );
			if( seq_p == NULL )	{
				seq_p = new srCmdSeq;
				err = mcu_p->pending_seq_map.insert( seq_name, seq_p );
				if( err == CMD_FAILURE )	{
					printf( "mcu_sequence_func ERR: insert pending '%s' FAILED\n", seq_name ); 
					return( err );
				}
			}
			
			float seq_time = args.read_float();
			char *seq_string = args.read_remainder_raw();
			return( seq_p->insert( seq_time, seq_string ) );
		}
		else
		if( strcmp( seq_cmd, "print" ) == 0 )	{
			
			srCmdSeq *seq_p = mcu_p->pending_seq_map.lookup( seq_name );
			if( seq_p == NULL )	{
				printf( "mcu_sequence_func ERR: print: '%s' NOT FOUND\n", seq_name ); 
				return( CMD_FAILURE );
			}
			seq_p->print( stdout );
		}
		else
		if( strcmp( seq_cmd, "abort" ) == 0 )	{
			int result = mcu_p->abort_seq( seq_name );
			if( result == CMD_NOT_FOUND )	{
				printf( "mcu_sequence_func ERR: abort: '%s' NOT FOUND\n", seq_name ); 
			}
			return( result );
		}
		else
		if( strcmp( seq_cmd, "delete" ) == 0 )	{
			int result = mcu_p->abort_seq( seq_name );
			if( result == CMD_NOT_FOUND )	{
				printf( "mcu_sequence_func ERR: delete: '%s' NOT FOUND\n", seq_name ); 
			}
			return( result );
		}
		else
			return( CMD_FAILURE );
		}
		
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

/*
	seq-chain <seqname>*
*/

int mcu_sequence_chain_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	vector<string> seq_names;
	const char* token = args.read_token();
	while( token[0] != '\0' ) {
		seq_names.push_back( token );

		token = args.read_token();
	}

	if( seq_names.empty() ) {
		cerr << "ERROR: seq-chain expected one or more .seq filenames." << endl;
		return CMD_FAILURE;
	}

	return mcu_p->execute_seq_chain( seq_names, "ERROR: seq-chian: " );
}


/////////////////////////////////////////////////////////////

/*
	Executes a command to set a configuration variable.
*/

int mcu_set_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->set_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		// TODO: Differentiate between not finding this var and subargs
		fprintf( stdout, "SBM ERR: Unknown Variable, Cannot set: '%s'\n> ", arg );  // Clarify this as a set command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/*
	Executes a command to print to the console some debug data.
	See insert_print_cmd and its call in main
*/

int mcu_print_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->print_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		// TODO: Differentiate between not finding this var and subargs
		fprintf( stdout, "SBM ERR: Print command NOT FOUND: '%s'\n> ", arg );  // Clarify this as a print command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/*
	Executes a test sub-command.
	See insert_test_cmd and its call in main
*/

int mcu_test_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
    char* arg = args.read_token();
    int result = mcu_p->test_cmd_map.execute( arg, args, mcu_p );
	if( result == CMD_NOT_FOUND ) {
		fprintf( stdout, "SBM ERR: Test command NOT FOUND: '%s'\n> ", arg );  // Clarify this as a test command error
		return CMD_SUCCESS; // Avoid multiple error messages
	} else {
		return result;
	}
}

/////////////////////////////////////////////////////////////

/*

	viewer open <width> <height> <px> <py> 
	viewer show|hide
	
*/

int mcu_viewer_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{

		char *view_cmd = args.read_token();
		if( strcmp( view_cmd, "open" ) == 0 )	{

			if( mcu_p->viewer_p == NULL )	{
				int argc = args.calc_num_tokens();
				if( argc >= 4 )	{

					int width = args.read_int();
					int height = args.read_int();
					int px = args.read_int();
					int py = args.read_int();
					int err = mcu_p->open_viewer( width, height, px, py );
					return( err );
				}
			}
		}
		else
		if( strcmp( view_cmd, "show" ) == 0 )	{
			if( mcu_p->viewer_p )	{
				mcu_p->viewer_p->show();
				return( CMD_SUCCESS );
			}
		}
		else
		if( strcmp( view_cmd, "hide" ) == 0 )	{
			if( mcu_p->viewer_p )	{
				mcu_p->viewer_p->hide();
				return( CMD_SUCCESS );
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*

	camera	eye <x y z>
	camera	center <x y z>
#	camera	up <x y z>
#	camera	fovy <degrees>
	camera	scale <factor>
	camera	default [<preset>]

*/

int mcu_camera_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		if( mcu_p->viewer_p )	{
			char *cam_cmd = args.read_token();
			if( strcmp( cam_cmd, "eye" ) == 0 )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				mcu_p->camera_p->eye.set( x, y, z );
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "center" ) == 0 )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				mcu_p->camera_p->center.set( x, y, z );
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "scale" ) == 0 )	{
				mcu_p->camera_p->scale = args.read_float();
				mcu_p->viewer_p->set_camera( *( mcu_p->camera_p ) );
			}
			else
			if( strcmp( cam_cmd, "default" ) == 0 )	{
				int preset = args.read_int();
				
				if( preset == 1 )	{
					mcu_p->viewer_p->view_all();
				}
			}
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*
	
	time maxfps|fps <desired-max-fps>
	time lockdt [0|1]
	time perf [0|1 [<interval>]]

*/

int mcu_time_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *time_cmd = args.read_token();
		if( 
			( strcmp( time_cmd, "maxfps" ) == 0 ) ||
			( strcmp( time_cmd, "fps" ) == 0 )
			)	{
			mcu_p->desired_max_fps = args.read_float();
		}
		else
		if( strcmp( time_cmd, "lockdt" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				int enable = args.read_int();
				mcu_p->lock_dt = enable ? true : false;
			}
			else	{
				mcu_p->lock_dt = !mcu_p->lock_dt;
			}
		}
		else
		if( strcmp( time_cmd, "perf" ) == 0 )	{
			int n = args.calc_num_tokens();
			if( n ) {
				int enable = args.read_int();
				mcu_p->perf.enable( enable ? true : false );
				if( n > 1 ) {
					float interval = args.read_float();
					mcu_p->perf.set_interval( interval );
				}
			}
			else	{
				mcu_p->perf.toggle();
			}
		}
		else	{
			return( CMD_NOT_FOUND );
		}
		return( CMD_SUCCESS );
	}
	return( CMD_FAILURE );
}

int mcu_character_init( 
	const char* char_name, 
	const char* skel_file, 
	const char* unreal_class, 
	mcuCBHandle *mcu_p
)	{
	int err;
	
	if( strcmp(char_name, "*" )==0 ) {  // TODO: better character name valiadtion
		printf( "init_character ERR: Invalid SbmCharacter name '%s'\n", char_name ); 
		return( CMD_FAILURE );
	}
	if( mcu_p->character_map.lookup( char_name ) )	{
		printf( "init_character ERR: SbmCharacter '%s' EXISTS\n", char_name ); 
		return( CMD_FAILURE );
	}

	SbmCharacter *char_p = new SbmCharacter(char_name);
	SkSkeleton* skeleton_p = load_skeleton( skel_file, mcu_p->me_paths );
	if( !skeleton_p ) {
		printf( "init_character ERR: Failed to load skeleton \"%s\"\n", skel_file ); 
		return CMD_FAILURE;
	}

	// Only initialize face_neutral if -facebone is enabled
	SkMotion* face_neutral_p = mcu_p->net_face_bones? mcu_p->face_neutral_p : NULL;
	err = char_p->init( skeleton_p, face_neutral_p, &mcu_p->au_motion_map, &mcu_p->viseme_map, unreal_class );
	if( err == CMD_SUCCESS ) {
		char_p->pipeline_p->set_evaluation_logger( mcu_p->logger_p );

		err = mcu_p->pawn_map.insert( char_name, char_p );
		if( err != CMD_SUCCESS )	{
			printf( "init_character ERR: SbmCharacter pawn_map.insert(..) '%s' FAILED\n", char_name ); 
			delete char_p;
			return( err );
		}

		err = mcu_p->character_map.insert( char_name, char_p );
		if( err != CMD_SUCCESS )	{
			printf( "init_character ERR: SbmCharacter character_map.insert(..) '%s' FAILED\n", char_name ); 
			mcu_p->pawn_map.remove( char_name );
			delete char_p;
			return( err );
		}

		err = mcu_p->add_scene( char_p->scene_p );
		if( err != CMD_SUCCESS )	{
			printf( "init_character ERR: add_scene '%s' FAILED\n", char_name ); 
			return( err );
		}


		// register wsp data
		// first register world_offset position/rotation
		string wsp_world_offset = vhcl::Format( "%s:world_offset", char_name );

		err = mcu_p->theWSP->register_vector_3d_source( wsp_world_offset, "position", SbmPawn::wsp_world_position_accessor, char_p );
		if( err != CMD_SUCCESS )	{
			printf( "WARNING: mcu_character_init \"%s\": Failed to register character position.\n", char_name ); 
		}

		err = mcu_p->theWSP->register_vector_4d_source( wsp_world_offset, "rotation", SbmPawn::wsp_world_rotation_accessor, char_p );
		if( err != CMD_SUCCESS )	{
			printf( "WARNING: mcu_character_init \"%s\": Failed to register character rotation.\n", char_name ); 
		}


		// now register all joints.  wsp data isn't sent out until a request for it is received
		const SrArray<SkJoint *> & joints  = char_p->skeleton_p->joints();

		int i;
		for ( i = 0; i < joints.size(); i++ )
		{
			SkJoint * j = joints[ i ];

			string wsp_joint_name = vhcl::Format( "%s:%s", char_name, (const char *)j->name() );

			err = mcu_p->theWSP->register_vector_3d_source( wsp_joint_name, "position", SbmPawn::wsp_position_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				printf( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" position.\n", char_name, wsp_joint_name ); 
			}

			err = mcu_p->theWSP->register_vector_4d_source( wsp_joint_name, "rotation", SbmPawn::wsp_rotation_accessor, char_p );
			if ( err != CMD_SUCCESS )
			{
				printf( "WARNING: mcu_character_init \"%s\": Failed to register joint \"%s\" rotation.\n", char_name, wsp_joint_name ); 
			}
		}
	}

	return( err );
}

int begin_controller( 
	const char *char_name, 
	const char *ctrl_name, 
	mcuCBHandle *mcu_p
)	{
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( ctrl_name );
		if( ctrl_p )	{
			// Use motion schedule by default
			MeCtScheduler2* sched_p = char_p->motion_sched_p;

			if( strcmp( ctrl_p->controller_type(), MeCtGaze::CONTROLLER_TYPE )==0 ) {
				sched_p = char_p->gaze_sched_p;
			}

			sched_p->schedule(
				ctrl_p, 
				mcu_p->time, 
				ctrl_p->indt(), 
				ctrl_p->outdt()
			);
			return( CMD_SUCCESS );
		}
		printf( "begin_controller ERR: ctrl '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	printf( "begin_controller ERR: char '%s' NOT FOUND\n", char_name );
	return( CMD_FAILURE );
}

int begin_controller( 
	const char *char_name, 
	const char *ctrl_name, 
	float ease_in, 
	float ease_out, 
	mcuCBHandle *mcu_p
)	{
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( ctrl_name );
		if( ctrl_p )	{
			//char_p->scheduler_p->schedule( 
			char_p->motion_sched_p->schedule( // Regardless of type, controllers created via ctrl commands are treated as motions
				ctrl_p, 
				mcu_p->time, 
				ease_in, 
				ease_out
			);
			return( CMD_SUCCESS );
		}
		printf( "begin_controller ERR: ctrl '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	printf( "begin_controller ERR: char '%s' NOT FOUND\n", char_name );
	return( CMD_FAILURE );
}

#if 0  // Version replaced by SbmCharacter::character_cmd_func // WHY ???
/*

	char <> init <skel-file>
	char <> ctrl <> begin [<ease-in> [<ease-out>]]
	char <> viseme <viseme_name> <weight>
	char <> bone <bone_name> <w> <x> <y> <z>
	char <> remove

*/

int mcu_character_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *char_name = args.read_token();
		char *char_cmd = args.read_token();
	
		if( strcmp( char_cmd, "init" ) == 0 )	{
			char *skel_file = args.read_token();
			char *unreal_class = args.read_token();
			return(	
				mcu_character_init( char_name, skel_file, unreal_class, mcu_p )
			);
		}
		if( strcmp( char_cmd, "ctrl" ) == 0 )	{
			return mcu_character_ctrl_cmd( char_name, args, mcu_p );
		}
		if ( strcmp( char_cmd, "viseme" ) == 0 )
		{
			char * viseme = args.read_token();
			float  weight = args.read_float();
			float  duration = args.read_float();	// added for ramps but not used yet!
		 
			SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
			if ( !actor ) {
				printf( "ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"%s\".\n", char_name );
				return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
			} else {
				actor->set_viseme( viseme, weight, duration );
				return CMD_SUCCESS;
			}
		}
		if ( strcmp( char_cmd, "bone" ) == 0 )
		{
			mcu_character_bone_cmd( char_name, args, mcu_p );
		}
		if ( strcmp( char_cmd, "remove" ) == 0 )
		{
			mcu_character_remove( char_name, mcu_p );

			return CMD_SUCCESS;
		}

		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}
#endif


/////////////////////////////////////////////////////////////

int mcu_character_ctrl_cmd(
	const char* char_name,
	srArgBuffer& args,
	mcuCBHandle *mcu_p 
) {
	const char *ctrl_name = args.read_token();
	const char *ctrl_cmd = args.read_token();
	
	if( strcmp( ctrl_cmd, "begin" ) == 0 )	{
		int n = args.calc_num_tokens();
		if( n )	{
			float ease_in = args.read_float();
			float ease_out = args.read_float();
			return(
				begin_controller( 
					char_name, 
					ctrl_name, 
					ease_in, 
					ease_out, 
					mcu_p
				)
			);
		}
		else	{
			return(
				begin_controller( 
					char_name, 
					ctrl_name, 
					mcu_p
				)
			);
		}
	}
	else
	if( strcmp( ctrl_cmd, "end" ) == 0 )	{
		
		//// TODO
		printf( "ERROR: \"char <char id> ctrl <ctrl id> end ...\" Unimplemented.\n" );
		return( CMD_FAILURE );
	}
	else
		return( CMD_FAILURE );
}


/////////////////////////////////////////////////////////////

// EDF - Hack!  This is currently used by Rapport for sending bone data straight through to Unreal.
//       It's only purpose is to be able to set a bone's rotation from an outside process (through VHMsg).

// "sbm char doctor bone base w x y z"
int mcu_character_bone_cmd(
	const char* char_name,
	srArgBuffer& args,
	mcuCBHandle *mcu_p 
) {
    char * bone = args.read_token();
    float  w    = args.read_float();
    float  x    = args.read_float();
    float  y    = args.read_float();
    float  z    = args.read_float();

    SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
    if ( !actor || !actor->skeleton_p )
    {
	    return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
    }
    else
    {
      //NetworkStartSendBoneRotations( actor->net_handle );
       actor->bonebusCharacter->StartSendBoneRotations();

		int i;
		for (	i = 0; i < actor->skeleton_p->joints().size();	i++ )
		{
			SkJoint * j	= actor->skeleton_p->joints()[ i ];

			if ( _stricmp( j->name(), bone ) == 0 )
			{
				//j->quat()->value().set( 1, 1, 0, 0 );
				//j->quat()->value()
				//j->quat()->value( SrQuat( w, x, y, z ) );

				//SrMat m;
				//SrQuat q = j->quat()->value();
				//q.get_mat( m );

				//float posx, posy, posz;
				//posx = j->pos()->value( 0 );
				//posy = j->pos()->value( 1 );
				//posz = j->pos()->value( 2 );

				// for old doctor skeleton
				//SendMEBoneRotation( j->skeleton()->name(),j->name(), q.w, -q.z, q.x, -q.y );

				// for new doctor skeleton
				if ( _stricmp( j->name(), "base" ) == 0 )
				{
					//SendMEBoneRotation( j->skeleton()->name(),j->name(), w, x, -y, z );
					//SendMEBonePosition( j->skeleton()->name(),j->name(), posx, -posy, posz );

					//printf( "%s %f %f %f\n", (const char *)j->name(),     posx, -posy, posz );

               //NetworkAddBulkRotation( actor->net_handle, j->name(), w, x, -y, z );
               actor->bonebusCharacter->AddBoneRotation( j->name(), w, x, -y, z, mcu_p->time );
				}
				else
				{
					//SendMEBoneRotation( j->skeleton()->name(),j->name(), w, -x, y, -z );
               //NetworkAddBulkRotation( actor->net_handle, j->name(), w, -x, y, -z );
               actor->bonebusCharacter->AddBoneRotation( j->name(), w, -x, y, -z, mcu_p->time );
				}
			}
      }

      //NetworkEndSendBoneRotations( actor->net_handle );
      actor->bonebusCharacter->EndSendBoneRotations();


      //NetworkStartSendBonePositions( actor->net_handle );
      actor->bonebusCharacter->StartSendBonePositions();

      for ( i = 0; i < actor->skeleton_p->joints().size(); i++ )
	   {
		   SkJoint * j	= actor->skeleton_p->joints()[ i ];

			if ( _stricmp( j->name(), bone ) == 0 )
			{
		      float posx, posy, posz;
		      posx = j->pos()->value( 0 );
		      posy = j->pos()->value( 1 );
		      posz = j->pos()->value( 2 );

		      if ( _stricmp( j->name(), "base" ) == 0 )
		      {
               //NetworkAddBulkPosition( actor->net_handle, j->name(), posx, -posy, posz );
               actor->bonebusCharacter->AddBonePosition( j->name(), posx, -posy, posz, mcu_p->time );
            }
            else
            {
            }
         }
      }

      //NetworkEndSendBonePositions( actor->net_handle );
      actor->bonebusCharacter->EndSendBonePositions();
	}

	return CMD_SUCCESS;
}


// "sbm char doctor bonep base x y z"
int mcu_character_bone_position_cmd(
   const char* char_name,
   srArgBuffer& args,
   mcuCBHandle *mcu_p 
) {
   char * bone = args.read_token();
   float  x    = args.read_float();
   float  y    = args.read_float();
   float  z    = args.read_float();
   int i;

   SbmCharacter * actor = mcu_p->character_map.lookup( char_name );
   if ( !actor || !actor->skeleton_p )
   {
      return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
   }
   else
   {
      actor->bonebusCharacter->StartSendBonePositions();

      for ( i = 0; i < actor->skeleton_p->joints().size(); i++ )
      {
         SkJoint * j	= actor->skeleton_p->joints()[ i ];

         if ( _stricmp( j->name(), bone ) == 0 )
         {
            float posx, posy, posz;

            /*
            if ( j->_ed == SrVec( 0, 0, 0 ) )
            {
               j->_ed = j->offset();
            }

            posx = j->_ed.x + x;
            posy = j->_ed.y + y;
            posz = j->_ed.z + z;
            */

            j->pos()->value( 0, x );
            j->pos()->value( 1, y );
            j->pos()->value( 2, z );

            //j->offset( SrVec( posx, posy, posz ) );

            posx = x;
            posy = y;
            posz = z;

            if ( _stricmp( j->name(), "base" ) == 0 )
            {
               actor->bonebusCharacter->AddBonePosition( j->name(), posx, -posy, posz, mcu_p->time );
            }
            else
            {
               //actor->bonebusCharacter->AddBonePosition( j->name(), -posz, -posy, posx );
               actor->bonebusCharacter->AddBonePosition( j->name(), posx, -posy, posz, mcu_p->time );
            }
         }
      }

      actor->bonebusCharacter->EndSendBonePositions();
   }

   return CMD_SUCCESS; 
}


/////////////////////////////////////////////////////////////

// Face pose mapping functions
const char* SET_FACE_AU_SYNTAX_HELP        = "set face au <unit-number> [left|right] <motion-name>";
const char* SET_FACE_VISEME_SYNTAX_HELP    = "set face viseme <viseme symbol> <motion-name>";
const char* PRINT_FACE_AU_SYNTAX1_HELP     = "print face au <unit number>";
const char* PRINT_FACE_AU_SYNTAX2_HELP     = "print face au *";
const char* PRINT_FACE_VISEME_SYNTAX1_HELP = "print face viseme <viseme name>";
const char* PRINT_FACE_VISEME_SYNTAX2_HELP = "print face viseme *";



int mcu_set_face_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string type = args.read_token();
	if( type.length() == 0 || type=="help" ) {
		// No arguments => help message
		cout << "Syntax:"<< endl
		     << "\t" << SET_FACE_AU_SYNTAX_HELP << endl
		     << "\t" << SET_FACE_VISEME_SYNTAX_HELP << endl;
		return CMD_SUCCESS;
	}

	if( type=="au" ) {
		return mcu_set_face_au_func( args, mcu_p );
	} else if( type=="viseme" ) {
		return mcu_set_face_viseme_func( args, mcu_p );
	} else if( type=="neutral" ) {
		const string motion_name = args.read_token();
		if( motion_name.length()==0 ) {
			cerr << "ERROR: Missing motion name." << endl;
			return CMD_FAILURE;
		}

		SkMotion* motion_p = mcu_p->motion_map.lookup( motion_name );
		if( motion_p ) {
			if( mcu_p->face_neutral_p )
				mcu_p->face_neutral_p->unref();
			mcu_p->face_neutral_p = motion_p;
			mcu_p->face_neutral_p->ref();
			return CMD_SUCCESS;
		} else {
			cerr << "ERROR: Unknown motion \"" << motion_name << "\"." << endl;
			return CMD_FAILURE;
		}
	} else {
		cerr << "ERROR: Unknown command \"set face "<<type<<"\"." << endl;
		return CMD_NOT_FOUND;
	}
}

int mcu_print_face_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string type = args.read_token();
	if( type.length() == 0 || type=="help" ) {
		// No arguments => help message
		cout << "Syntax:"<< endl
		     << "\t" << PRINT_FACE_AU_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX2_HELP << endl
		     << "\t" << PRINT_FACE_VISEME_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_VISEME_SYNTAX2_HELP << endl;
		return CMD_SUCCESS;
	}

	if( type=="au" ) {
		return mcu_print_face_au_func( args, mcu_p );
	} else if( type=="viseme" ) {
		return mcu_print_face_viseme_func( args, mcu_p );
	} else if( type=="neutral" ) {
		cout << "UNIMPLEMENTED" << endl;
		return CMD_FAILURE;
	} else {
		cerr << "ERROR: Unknown command \"print face "<<type<<"\"." << endl;
		return CMD_NOT_FOUND;
	}
}

/**
 *  Command processor for "set face au ...".
 *  Syntax:
 *		set face au <unit-number> [left|right] <motion-name>
 */
int mcu_set_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string unit_str = args.read_token();
	if( unit_str.length()==0 ) {
		// No arguments => help message
		cout << "Syntax: " << SET_FACE_AU_SYNTAX_HELP << endl;
		return CMD_SUCCESS;
	}

	istringstream unit_iss( unit_str );
	int unit;
	if( !( unit_iss >> unit )
		|| ( unit < 1 ) )
	{
		cerr << "ERROR: Invalid action unit number \"" << unit_str << "\"." << endl;
		return CMD_FAILURE;
	}

	enum { UNIFIED, LEFT, RIGHT } side;
	string token = args.read_token();
	string face_pose_name;
	if( token=="left" || token=="LEFT" ) {
		side = LEFT;
		face_pose_name = args.read_token();
	} else if( token=="right" || token=="RIGHT" ) {
		side = RIGHT;
		face_pose_name = args.read_token();
	} else {
		side = UNIFIED;
		face_pose_name = token;
	}

	if( face_pose_name.length()==0 ) {
		cerr << "ERROR: Missing viseme motion name." << endl;
		return CMD_FAILURE;
	}

	// Currently we use the first frame of SkMotion because
	// of limitations in our exports (can't export direct to .skp).
	// TODO: use .skp and/or convert arbitrary frame number/time to SkPosture
	SkMotion* motion = mcu_p->motion_map.lookup( face_pose_name );
	if( motion == NULL ) {
		cerr << "ERROR: Unknown facial pose \"" << face_pose_name << "\"." << endl;
		return CMD_FAILURE;
	}

	AUMotionPtr au;
	AUMotionMap& au_map = mcu_p->au_motion_map;
	AUMotionMap::iterator pos = au_map.find(unit);
	if( pos == au_map.end() ) {
		switch( side ) {
			case UNIFIED:
				au = new AUMotion( motion );
				break;
			case LEFT:
				au = new AUMotion( motion, NULL );
				break;
			case RIGHT:
				au = new AUMotion( NULL, motion );
				break;
			default:
				// Invalid code.  Throw assert?
				cerr << "ERROR: Invalid side \""<<side<<"\"" << endl;
				return CMD_FAILURE;
		}
		au_map.insert( make_pair( unit, au ) );
	} else {
		au = pos->second;  // value half of std::pair
		switch( side ) {
			case UNIFIED:
				if( au->left || au->right )
					cerr << "WARNING: Overwritting au #" << unit << endl;
				au->set( motion );
				break;
			case LEFT:
				if( au->left )
					cerr << "WARNING: Overwritting au #" << unit << " left" <<endl;
				au->set( motion, au->right );
				break;
			case RIGHT:
				if( au->right )
					cerr << "WARNING: Overwritting au #" << unit << " right" <<endl;
				au->set( au->left, motion );
				break;
			default:
				// Invalid code.  Throw assert?
				cerr << "ERROR: Invalid side \""<<side<<"\"" << endl;
				return CMD_FAILURE;
		}
	}

	return CMD_SUCCESS;
}

inline void print_au( const int unit, const AUMotionPtr au ) {
	if( au->is_bilateral() ) {
		cout << "Action Unit #" << unit << ": Left SkMotion ";
		if( au->left ) {
			cout << '\"' << au->left->name() << "\".";
		} else {
			cout << "is NULL.";
		}
		cout << endl;

		cout << "Action Unit #" << unit << ": Right SkMotion ";
		if( au->right ) {
			cout << '\"' << au->right->name() << "\".";
		} else {
			cout << "is NULL.";
		}
	} else {
		cout << "Action Unit #" << unit << ": SkMotion ";
		if( au->left ) {
			cout << '\"' << au->left->name() << "\".";
		} else {
			cout << "is NULL.";
		}
	}
	cout << endl;
}

/**
 *  Implements the "print face au ..." command.
 *
 *  Syntax:
 *     print face au <unit-number>
 *     print face au *
 */
int mcu_print_face_au_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	const int ALL_ACTION_UNITS = 0;  // Marker

	string unit_str = args.read_token();
	if( unit_str.length()==0 ) {
		// No arguments => help message
		cout << "Syntax:" << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX1_HELP << endl
		     << "\t" << PRINT_FACE_AU_SYNTAX2_HELP<< endl;
		return CMD_SUCCESS;
	}

	int unit;
	if( unit_str=="*" ) {
		unit = ALL_ACTION_UNITS;
	} else {
		istringstream unit_iss( unit_str );
		if( !( unit_iss >> unit )
			|| ( unit < 1 ) )
		{
			cerr << "ERROR: Invalid action unit number \"" << unit_str << "\"." << endl;
			return CMD_FAILURE;
		}
	}

	AUMotionPtr au;
	AUMotionMap& au_map = mcu_p->au_motion_map;
	AUMotionMap::iterator pos;
	AUMotionMap::iterator end = au_map.end();
	if( unit == ALL_ACTION_UNITS ) {
		for( pos = au_map.begin(); pos!=end; ++pos ) {
			unit = pos->first;
			au  = pos->second;

			print_au( unit, au );
		}
	} else {
		pos = au_map.find( unit );
		if( pos == end ) {
			cout << "Action Unit #" << unit << " is not set." << endl;
		} else {
			print_au( unit, pos->second );
		}
	}
	return CMD_SUCCESS;
}


/**
 *  Implements the "set face viseme ..." command.
 *
 *  Syntax: set face viseme <viseme symbol> <motion-name>
 */
int mcu_set_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		cout << "Syntax: " << SET_FACE_VISEME_SYNTAX_HELP << endl;
		return CMD_SUCCESS;
	}

	string motion_name = args.read_token();
	if( motion_name.length()==0 ) {
		cerr << "ERROR: Missing viseme motion name." << endl;
		return CMD_FAILURE;
	}

	// Currently we use the first frame of SkMotion because
	// of limitations in our exports (can't export direct to .skp).
	// TODO: use .skp and/or convert arbitrary frame number/time to SkPosture
	SkMotion* motion = mcu_p->motion_map.lookup( motion_name );
	if( motion == NULL ) {
		cerr << "ERROR: Unknown viseme pose \"" << motion_name << "\"." << endl;
		return CMD_FAILURE;
	}

	VisemeMotionMap& viseme_map = mcu_p->viseme_map;
	VisemeMotionMap::iterator pos = viseme_map.find( viseme );
	if( pos != viseme_map.end() ) {
		cerr << "WARNING: Overwriting viseme \""<<viseme<<"\" motion mapping." << endl;
	}
	viseme_map.insert( make_pair( viseme, motion ) );
	motion->ref();

	return CMD_SUCCESS;
}

void print_viseme( const string& viseme, const SkMotion* motion ) {
	cout << "Viseme \""<<viseme<<"\" pose: ";
	if( motion == NULL ) {
		cout << "NULL";
	} else {
		cout << '\"' << motion->name() << '\"';
	}
	cout << endl;
}

/**
 *  Implements the "print face viseme ..." command.
 *
 *  Syntax:
 *     print face viseme <viseme-name>
 *     print face viseme *
 */
int mcu_print_face_viseme_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string viseme = args.read_token();
	if( viseme.length() == 0 ) {
		// No arguments => help message
		cout << "Syntax:" << endl
		     << "\tprint face viseme <viseme name>" << endl
		     << "\tprint face viseme *" << endl;
		return CMD_SUCCESS;
	}

	VisemeMotionMap& viseme_map = mcu_p->viseme_map;
	VisemeMotionMap::iterator pos;
	VisemeMotionMap::iterator end = viseme_map.end();
	if( viseme=="*" ) {
		for( pos=viseme_map.begin(); pos!=end; ++pos ) {
			print_viseme( pos->first, pos->second );
		}
	} else {
		pos = viseme_map.find( viseme );
		if( pos == end ) {
			cout << "Viseme \""<<viseme<<"\" is unset." << endl;
		} else {
			print_viseme( viseme, pos->second );
		}
	}

	return CMD_SUCCESS;
}


/////////////////////////////////////////////////////////////

int init_pose_controller( 
	char *ctrl_name, 
	char *pose_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	SkPosture *pose_p = mcu_p->pose_map.lookup( pose_name );
	if( pose_p == NULL ) {
		printf( "init_pose_controller ERR: SkPosture '%s' NOT FOUND in pose map\n", pose_name ); 
		return( CMD_FAILURE );
	}

	MeCtPose* ctrl_p = new MeCtPose;
	err = mcu_p->pose_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_pose_controller ERR: MeCtPose '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_pose_controller ERR: MeCtPose '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init( *pose_p );
	return( CMD_SUCCESS );
}

int init_motion_controller( 
	char *ctrl_name, 
	char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	SkMotion *mot_p = mcu_p->motion_map.lookup( mot_name );
	if( mot_p == NULL ) {
		printf( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}

	MeCtMotion* ctrl_p = new MeCtMotion;
	err = mcu_p->motion_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_motion_controller ERR: MeCtMotion '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_motion_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p );
	return( CMD_SUCCESS );
}

int init_stepturn_controller( 
	char *ctrl_name, 
	char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	SkMotion *mot_p = mcu_p->motion_map.lookup( mot_name );
	if( mot_p == NULL ) {
		printf( "init_stepturn_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}

	MeCtStepTurn* ctrl_p = new MeCtStepTurn;
	err = mcu_p->stepturn_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_stepturn_controller ERR: MeCtStepTurn '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_stepturn_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p );
	return( CMD_SUCCESS );
}

int init_quickdraw_controller( 
	char *ctrl_name, 
	char *mot_name, 
	char *alt_mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	SkMotion *mot_p = mcu_p->motion_map.lookup( mot_name );
	if( mot_p == NULL ) {
		printf( "init_quickdraw_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	
	SkMotion *alt_mot_p = NULL;
	if( alt_mot_name )	{
		alt_mot_p = mcu_p->motion_map.lookup( alt_mot_name );
		if( alt_mot_p == NULL ) {
			printf( "init_quickdraw_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", alt_mot_name ); 
			return( CMD_FAILURE );
		}
	}

	MeCtQuickDraw* ctrl_p = new MeCtQuickDraw;
	err = mcu_p->quickdraw_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_quickdraw_controller ERR: MeCtQuickDraw '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_quickdraw_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
	ctrl_p->init( mot_p, alt_mot_p );
	return( CMD_SUCCESS );
}

int init_gaze_controller(
	char *ctrl_name, 
	char *key_fr,
	char *key_to,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	MeCtGaze *ctrl_p;
	ctrl_p = mcu_p->gaze_ctrl_map.lookup( ctrl_name );
	if( ctrl_p )	{
		printf( "init_gaze_controller ERR: MeCtGaze '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}

	ctrl_p = new MeCtGaze;
	err = mcu_p->gaze_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_gaze_controller ERR: MeCtGaze '%s' insertion FAILED\n", ctrl_name ); 
		delete ctrl_p;
		return( err );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_gaze_controller ERR: MeCtGaze '%s' EXISTS\n", ctrl_name ); 
		return( err );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init(
		MeCtGaze::key_index( strlen( key_fr ) ? key_fr : "back" ),  // WARN: does not handle NULL string
//		MeCtGaze::key_index( strlen( key_to ) ? key_to : "eyes" )
		MeCtGaze::key_index( strlen( key_to ) ? key_to : ( strlen( key_fr ) ? key_fr : "eyes" ) )
	);
	return( CMD_SUCCESS );
}

int init_simple_nod_controller(
	char *ctrl_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	MeCtSimpleNod* ctrl_p = new MeCtSimpleNod;
	err = mcu_p->snod_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_simple_nod_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( err );
	}
	ctrl_p->ref();
	
	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_simple_nod_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name ); 
		return( err );
	}
	ctrl_p->ref();

	ctrl_p->name( ctrl_name );
	ctrl_p->init();
	return( CMD_SUCCESS );
}

int init_lilt_controller(
	char *ctrl_name,
	char *char_name,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p= mcu_p->character_map.lookup( char_name );
	if ( !char_p ) {
		printf( "init_lilt_controller ERR: SbmCharacter '%s' NOT FOUND\n", char_name );
		return( CMD_FAILURE );
	}
	if ( !char_p->motion_sched_p ) {
		printf( "init_lilt_controller ERR: SbmCharacter '%s' UNINITIALIZED\n", char_name );
		return( CMD_FAILURE );
	}
	MeCtAnkleLilt* ctrl_p = new MeCtAnkleLilt;
	err = mcu_p->lilt_ctrl_map.insert( ctrl_name, ctrl_p );
	if ( err == CMD_FAILURE ) {
		printf( "init_lilt_controller ERR: MeCtAnkleLilt '%s' EXISTS\n", ctrl_name );
		delete ctrl_p;
		return( err );
	}
	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if (err == CMD_FAILURE){
		printf( "init_lilt_controller ERR: MeCtSimpleNod '%s' EXISTS\n", ctrl_name);
		return( err );
	}
	ctrl_p->ref();
	ctrl_p->name( ctrl_name );
	ctrl_p->init( char_p->skeleton_p );
	return( CMD_SUCCESS );
}

int init_eyelid_controller(
	char *ctrl_name,
//	char *mot_A_name,
//	char *mot_B_name,
//	char *mot_C_name,
//	char *mot_D_name,
//	char *mot_E_name,
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

#if 0
	SkMotion *mot_A_p = mcu_p->motion_map.lookup( mot_A_name );
	if( mot_A_p == NULL ) {
		printf( "init_eyelid_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_A_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_B_p = mcu_p->motion_map.lookup( mot_B_name );
	if( mot_B_p == NULL ) {
		printf( "init_eyelid_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_B_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_C_p = mcu_p->motion_map.lookup( mot_C_name );
	if( mot_C_p == NULL ) {
		printf( "init_eyelid_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_C_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_D_p = mcu_p->motion_map.lookup( mot_D_name );
	if( mot_D_p == NULL ) {
		printf( "init_eyelid_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_D_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_E_p = mcu_p->motion_map.lookup( mot_E_name );
	if( mot_E_p == NULL ) {
		printf( "init_eyelid_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_E_name ); 
		return( CMD_FAILURE );
	}
#endif

	MeCtEyeLid* ctrl_p = new MeCtEyeLid;
	err = mcu_p->eyelid_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_eyelid_controller ERR: MeCtEyeLid '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		printf( "init_eyelid_controller ERR: MeCtEyeLid '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();
	
	ctrl_p->name( ctrl_name );
//	ctrl_p->init( mot_A_p, mot_B_p, mot_C_p );
//	ctrl_p->init( mot_A_p, mot_B_p );
//	ctrl_p->init( mot_A_p, mot_B_p, mot_C_p, mot_D_p, mot_E_p );
	ctrl_p->init();
	return( CMD_SUCCESS );
}

int init_lifecycle_controller( char *ctrl_name, char *child_name, mcuCBHandle *mcu_p ) {

	MeController* ct = mcu_p->controller_map.lookup( child_name );
	if( ct==NULL ) {
		printf( "init_lifecycle_controller ERROR: MeController '%s' NOT FOUND\n", ctrl_name );
		return( CMD_FAILURE );
	}
	MeCtLifecycleTest* lifecycle_ct = new MeCtLifecycleTest();
	if( mcu_p->controller_map.insert( ctrl_name, lifecycle_ct ) != CMD_SUCCESS ) {
		printf( "init_lifecycle_controller ERROR: Failed to insert into controller_map\n" );
		return( CMD_FAILURE );
	}
	lifecycle_ct->ref();

	lifecycle_ct->name( ctrl_name );
	lifecycle_ct->init( ct );

	return CMD_SUCCESS;
}

int init_scheduler_controller( 
	char *ctrl_name, 
	char *char_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;
	
	SbmCharacter *char_p = mcu_p->character_map.lookup( char_name );
	if( char_p )	{
		MeCtScheduler2* sched_p = new MeCtScheduler2;
		err = mcu_p->sched_ctrl_map.insert( ctrl_name, sched_p );
		if( err == CMD_FAILURE )	{
			printf( "init_scheduler_controller ERR: MeCtScheduler '%s' EXISTS\n", ctrl_name ); 
			delete sched_p;
			return( err );
		}
		sched_p->ref();
	
		err = mcu_p->controller_map.insert( ctrl_name, sched_p );
		if( err == CMD_FAILURE )	{
			printf( "init_scheduler_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
			return( err );
		}
		sched_p->ref();
		
		sched_p->init();

		return( CMD_SUCCESS );
	}

	printf( "init_scheduler_controller ERR: SbmCharacter '%s' NOT FOUND\n", char_name ); 
	return( CMD_FAILURE );
}

int set_controller_timing(
	MeController* ctrl_p, 
	float indt, 
	float outdt, 
	float empht
)	{
	ctrl_p->inoutdt( indt, outdt );
	ctrl_p->emphasist( empht );
	return( CMD_SUCCESS );
}

//int set_controller_timing(
//	char *ctrl_name, 
//	float indt, 
//	float outdt, 
//	float empht, 
//	mcuCBHandle *mcu_p
//)	{
//
//	if( MeController* ctrl_p = mcu_p->controller_map.lookup( ctrl_name ) )	{
//		ctrl_p->inoutdt( indt, outdt );
//		ctrl_p->emphasist( empht );
//		return( CMD_SUCCESS );
//	}
//	printf( "set_controller_timing ERR: MeController '%s' NOT FOUND\n", ctrl_name );
//	return( CMD_FAILURE );
//}

int query_controller(
	MeController* ctrl_p
)	{
	printf( "MCU QUERY: MeController '%s':\n", ctrl_p->name() );
	printf( "  type... %s\n", ctrl_p->controller_type() );
	printf( "  indt... %.3f\n", ctrl_p->indt() );
	printf( "  outdt.. %.3f\n", ctrl_p->outdt() );
	float emph = ctrl_p->emphasist();
	if( emph < 0.0 )	{
		printf( "  emph... UNK\n");
	}
	else	{
		printf( "  emph... %.3f\n", emph );
	}
	double dur = ctrl_p->controller_duration();
	if( dur < 0.0 )	{
		printf( "  dur.... UNK\n" );
	}
	else	{
		printf( "  dur.... %.3f\n", dur );
	}
//	printf( "> " );
	return( CMD_SUCCESS );
}


//int query_controller(
//	char *ctrl_name, 
//	mcuCBHandle *mcu_p
//)	{
//
//	if( MeController* ctrl_p = mcu_p->controller_map.lookup( ctrl_name ) )	{
//		printf( "MCU QUERY: MeController '%s':\n", ctrl_name );
//		printf( "  type... %s\n", ctrl_p->controller_type() );
//		printf( "  indt... %.3f\n", ctrl_p->indt() );
//		printf( "  outdt.. %.3f\n", ctrl_p->outdt() );
//		float emph = ctrl_p->emphasist();
//		if( emph < 0.0 )	{
//			printf( "  emph... UNK\n");
//		}
//		else	{
//			printf( "  emph... %.3f\n", emph );
//		}
//		double dur = ctrl_p->controller_duration();
//		if( dur < 0.0 )	{
//			printf( "  dur.... UNK\n" );
//		}
//		else	{
//			printf( "  dur.... %.3f\n", dur );
//		}
////		printf( "> " );
//		return( CMD_SUCCESS );
//	}
//	printf( "query_controller ERR: MeController '%s' NOT FOUND\n", ctrl_name );
//	return( CMD_FAILURE );
//}

/*

	ctrl <> query
	ctrl <> timing <ease-in> <ease-out> [<emph>]
	ctrl <> record motion <full-file-prefix> <num-frames>
#	ctrl <> record pose <full-file-prefix>

#	ctrl <> interp <dur> <ctrl-a> <ctrl-b> linear|sine
#	ctrl <> interp <dur> <ctrl-a> <ctrl-b> curve <curve-name>
#	ctrl <> point <x y z>

	ctrl <> gaze [<key-fr> <key-to>]

	ctrl <> motion <motion-file>
	ctrl <> snod <char-name>

	ctrl <> quickdraw <quickdraw-motion-name> [<reholster-motion>]

	ctrl <> lifecycle <child-controller-name>

#	ctrl <> sched <char-name> [<skel-subset>]
	ctrl <> sched <char-name>

*/

int mcu_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	
	if( mcu_p )	{
		char *ctrl_name = args.read_token();
		char *ctrl_cmd = args.read_token();
	
		if( strcmp( ctrl_cmd, "pose" ) == 0 )	{
			char *pose_name = args.read_token();
			return(
				init_pose_controller( ctrl_name, pose_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "motion" ) == 0 )	{
			char *mot_name = args.read_token();
			return(
				init_motion_controller( ctrl_name, mot_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "stepturn" ) == 0 )	{
			char *mot_name = args.read_token();
			return(
				init_stepturn_controller( ctrl_name, mot_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "quickdraw" ) == 0 )	{
			char *mot_name = args.read_token();
			if( args.calc_num_tokens() > 0 )	{
				char *alt_mot_name = args.read_token();
				return(
					init_quickdraw_controller( ctrl_name, mot_name, alt_mot_name, mcu_p )
				);
			}
			return(
				init_quickdraw_controller( ctrl_name, mot_name, NULL, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "gaze" ) == 0 )	{
			char *key_fr = args.read_token();
			char *key_to = args.read_token();
			return(
				init_gaze_controller( ctrl_name, key_fr, key_to, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "snod" ) == 0 )	{
//			char *char_name = args.read_token();
			return(
				init_simple_nod_controller( ctrl_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "lilt" )== 0) {
			char *char_name= args.read_token();
			return(
				init_lilt_controller( ctrl_name, char_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "eyelid" )== 0) {
//			char *mot_A_name= args.read_token();
//			char *mot_B_name= args.read_token();
//			char *mot_C_name= args.read_token();
//			char *mot_D_name= args.read_token();
//			char *mot_E_name= args.read_token();
			return(
//				init_eyelid_controller( ctrl_name, mot_A_name, mot_B_name, mot_C_name, mot_D_name, mot_E_name, mcu_p )
//				init_eyelid_controller( ctrl_name, mot_A_name, mot_B_name, mot_C_name, mcu_p )
//				init_eyelid_controller( ctrl_name, mot_A_name, mot_B_name, mcu_p )
				init_eyelid_controller( ctrl_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "lifecycle" )== 0) {
			char *child_name= args.read_token();
			return(
				init_lifecycle_controller( ctrl_name, child_name, mcu_p )
			);
		}
		else
		if( strcmp( ctrl_cmd, "sched" ) == 0 )	{
			char *char_name = args.read_token();
			return(
				init_scheduler_controller( ctrl_name, char_name, mcu_p )
			);
		}
		else
		{
			// Non-initializing controllers need an actual instance
			MeController* ctrl_p = mcu_p->lookup_ctrl( string( ctrl_name ), "ERROR: ctrl <controller name>: " );
			if( ctrl_p==NULL ) {
				// should have printed error from above function
				return CMD_FAILURE;
			}

			if( strcmp( ctrl_cmd, "record" ) == 0 )	{
				char *record_type = args.read_token();
				char *full_prefix = args.read_token();
				if( strcmp( record_type, "motion" ) == 0 )	{
					int num_frames = args.read_int();
					ctrl_p->record_motion( full_prefix, num_frames );
					return( CMD_SUCCESS );
				}
				if( strcmp( record_type, "bvh" ) == 0 )	{
				
					if( mcu_p->lock_dt )	{
						int num_frames = args.read_int();
						ctrl_p->record_bvh( full_prefix, num_frames, 1.0/mcu_p->desired_max_fps );
						return( CMD_SUCCESS );
					}
					else	{
						printf( "mcu_controller_func ERR: BVH recording requires lockdt set\n" );
						return( CMD_FAILURE );
					}
				}
			}
			else
			if( strcmp( ctrl_cmd, "timing" ) == 0 )	{
				int n = args.calc_num_tokens();
				if( n )	{
					float indt = args.read_float();
					float outdt = args.read_float();
					float empht = -1.0;
					if( n > 2 )	{
						empht = args.read_float();
					}
					return(
						set_controller_timing( ctrl_p, indt, outdt, empht )
					);
				}
				return( CMD_FAILURE );
			}
			else
			if( strcmp( ctrl_cmd, "query" ) == 0 )	{
				return(
					query_controller( ctrl_p )
				);
			}
			else
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*
	motion <> speed <factor>
	motion <> dur <sec>
*/

int mcu_motion_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		char *warp_type = args.read_token(); // dur or speed
		float timing = args.read_float();
		MeCtMotion *mot_p= mcu_p->motion_ctrl_map.lookup( ctrl_name );
		if( mot_p ) {
			if( 
				( warp_type[ 0 ] == 'd' )||( warp_type[ 0 ] == 'D' )||
				( warp_type[ 0 ] == 't' )||( warp_type[ 0 ] == 'T' ) 
			)	{
				float out_dur = timing;
				timing = (float)( mot_p->phase_duration() ) / out_dur;
			}
			mot_p->warp_limits( timing, timing );
			mot_p->twarp( timing );
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/*
	stepturn <> dur|time|speed <sec|dps> local|world <heading-deg>
*/

int mcu_stepturn_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		char *time_type = args.read_token(); // dur or speed
		float timing = args.read_float();
		char *coord_type = args.read_token(); // local or world
		float deg = args.read_float();
		MeCtStepTurn *step_p= mcu_p->stepturn_ctrl_map.lookup( ctrl_name );
		if( step_p ) {
			if( 
				( time_type[ 0 ] == 'd' )||( time_type[ 0 ] == 'D' )||
				( time_type[ 0 ] == 't' )||( time_type[ 0 ] == 'T' ) 
			)	{
				step_p->set_duration( timing );
			}
			else	{
				step_p->set_speed( timing );
			}
			if( ( coord_type[ 0 ] == 'l' )||( coord_type[ 0 ] == 'L' ) )	{
				step_p->set_heading_local( deg );
			}
			else	{
				step_p->set_heading_world( deg );
			}
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

/*
X	quickdraw <> <dur-sec> point <x y z>
X	quickdraw <> <dur-sec> point <x y z> [<joint>]

	quickdraw <> target point <x y z>
	quickdraw <> aimoff <p h r>
	quickdraw <> dur <draw-dur>
#	quickdraw <> dur <draw-dur> [<withdraw-dur>]
	quickdraw <> smooth <basis>
	quickdraw <> track <tracking-dur>
	quickdraw <> persist | reholster
*/

int mcu_quickdraw_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		MeCtQuickDraw *qdraw_p= mcu_p->quickdraw_ctrl_map.lookup( ctrl_name );
		if( qdraw_p ) {
#if 0
			float dur = args.read_float();
			qdraw_p->set_motion_duration( dur );

			char *target_type = args.read_token(); // local or world
			if( ( target_type[ 0 ] == 'p' )||( target_type[ 0 ] == 'P' ) )	{
				float x = args.read_float();
				float y = args.read_float();
				float z = args.read_float();
				qdraw_p->set_target_point( x, y, z );
			}
#endif
			char *qdraw_cmd = args.read_token();
			if( strcmp( qdraw_cmd, "target" ) == 0 )	{
			
				char *target_type = args.read_token();
				if( strcmp( target_type, "point" ) == 0 )	{
					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					qdraw_p->set_target_point( x, y, z );
					return( CMD_SUCCESS );
				}
#if 0
				else
				if( strcmp( target_type, "joint" ) == 0 )	{
					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					char* joint_name = args.read_token();
					qdraw_p->set_target_joint( x, y, z, joint_name );
					return( CMD_SUCCESS );
				}
#endif
			}
			if( strcmp( qdraw_cmd, "aimoff" ) == 0 )	{
				float p = args.read_float();
				float h = args.read_float();
				float r = args.read_float();
				qdraw_p->set_aim_offset( p, h, r );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "dur" ) == 0 )	{
				float dur = args.read_float();
				if( args.calc_num_tokens() > 0 )	{
					float dur2 = args.read_float();
					qdraw_p->set_motion_duration( dur, dur2 );
				}
				else	{
					qdraw_p->set_motion_duration( dur );
				}
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "smooth" ) == 0 )	{
				float sm = args.read_float();
				qdraw_p->set_smooth( sm );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "track" ) == 0 )	{
				float dur = args.read_float();
				qdraw_p->set_track_duration( dur );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "persist" ) == 0 )	{
				qdraw_p->set_track_duration( -1.0 );
				return( CMD_SUCCESS );
			}
			if( strcmp( qdraw_cmd, "reholster" ) == 0 )	{
				qdraw_p->set_track_duration( 0.0 );
				return( CMD_SUCCESS );
			}
			printf( "mcu_quickdraw_controller_func ERR: command '%s' NOT RECOGNIZED\n", qdraw_cmd );
			return( CMD_NOT_FOUND );
		}
	}
	return( CMD_FAILURE );
}

/*
	gaze <> target point <x y z>
	gaze <> target euler <p h r>
	gaze <> offset euler <p h r>
	gaze <> smooth <basis>
	gaze <> speed <deg-per-sec>
	gaze <> bias <key> <p h r>
	gaze <> limit <key> <p h r>
	gaze <> limit <key> <p-up p-dn h r>
	gaze <> blend <key> <weight>
	gaze <> priority <key>
*/

int mcu_gaze_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p ) {
		char *ctrl_name = args.read_token();
		MeCtGaze *gaze_p= mcu_p->gaze_ctrl_map.lookup( ctrl_name );
		if( gaze_p )	{
			
			char *gaze_cmd = args.read_token();
			if( strcmp( gaze_cmd, "target" ) == 0 )	{

				char *target_type = args.read_token();
				if( strcmp( target_type, "point" ) == 0 )	{

					float x = args.read_float();
					float y = args.read_float();
					float z = args.read_float();
					gaze_p->set_target( x, y, z );
//					gaze_p->set_target_point( x, y, z );
					return( CMD_SUCCESS );
				}
				if( strcmp( target_type, "euler" ) == 0 )	{

					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_orient( p, h, r );
//					gaze_p->set_target_euler( p, h, r );
					return( CMD_SUCCESS );
				}
				printf( "mcu_gaze_controller_func ERR: target '%s' NOT RECOGNIZED\n", target_type );
				return( CMD_NOT_FOUND );
			}
			if( strcmp( gaze_cmd, "offset" ) == 0 )	{

				char *offset_type = args.read_token();
				if( strcmp( offset_type, "euler" ) == 0 )	{

					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_offset_euler( p, h, r );
					return( CMD_SUCCESS );
				}
				printf( "mcu_gaze_controller_func ERR: offset '%s' NOT RECOGNIZED\n", offset_type );
				return( CMD_NOT_FOUND );
			}
			if( strcmp( gaze_cmd, "smooth" ) == 0 )	{

				float sm = args.read_float();
				gaze_p->set_smooth( sm );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "speed" ) == 0 )	{

				float sp = args.read_float();
				gaze_p->set_speed( sp );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "bias" ) == 0 )	{

				char *key_label = args.read_token();
				float p = args.read_float();
				float h = args.read_float();
				float r = args.read_float();
				gaze_p->set_bias( MeCtGaze::key_index( key_label ), p, h, r );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "limit" ) == 0 )	{

				char *key_label = args.read_token();
				int n = args.calc_num_tokens();
				if( n > 3 ) {
					float p_up = args.read_float();
					float p_dn = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_limit( MeCtGaze::key_index( key_label ), p_up, p_dn, h, r );
				}
				else	{
					float p = args.read_float();
					float h = args.read_float();
					float r = args.read_float();
					gaze_p->set_limit( MeCtGaze::key_index( key_label ), p, h, r );
				}
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "blend" ) == 0 )	{

				char *key_label = args.read_token();
				float w = args.read_float();
				gaze_p->set_blend( MeCtGaze::key_index( key_label ), w );
				return( CMD_SUCCESS );
			}
			if( strcmp( gaze_cmd, "priority" ) == 0 )	{

				char *key_label = args.read_token();
				gaze_p->set_task_priority( MeCtGaze::key_index( key_label ) );
				return( CMD_SUCCESS );
			}
			printf( "mcu_gaze_controller_func ERR: command '%s' NOT RECOGNIZED\n", gaze_cmd );
			return( CMD_NOT_FOUND );
		}
		printf( "mcu_gaze_controller_func ERR: MeCtGaze '%s' NOT FOUND\n", ctrl_name );
	}
	return( CMD_FAILURE );
}

/*
	snod <> <dur-sec> <mag-deg> [<reps=1.0> [<affirm=1>]]
*/

int mcu_snod_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p )	{
		char *snod_ctrl_name = args.read_token();
		float dur = args.read_float();
		float mag = args.read_float();
		float rep = 1.0;
		int aff = TRUE;
		int n = args.calc_num_tokens();
		if( n )	{
			rep = args.read_float();
			if( n == 2 )	{
				aff = args.read_int();
			}
		}
		MeCtSimpleNod *snod_p = mcu_p->snod_ctrl_map.lookup( snod_ctrl_name );
		if( snod_p )	{
			snod_p->set_nod( dur, mag, rep, aff );
			return( CMD_SUCCESS );
		}
	}
	return( CMD_FAILURE );
}

int mcu_lilt_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p) {

	if( mcu_p ) {
		char *lilt_ctrl_name = args.read_token();
		float dur = args.read_float();
		float mag = args.read_float();
		MeCtAnkleLilt *lilt_p= mcu_p->lilt_ctrl_map.lookup( lilt_ctrl_name );
		if( lilt_p ) {
			lilt_p->set_lilt( dur,mag,0);
			return(CMD_SUCCESS);
		}

	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

int add_controller_to_scheduler( 
	char *sched_ctrl_name, 
	char *add_ctrl_name, 
	float T_at, 
	mcuCBHandle *mcu_p
)	{

	MeCtScheduler2 *sched_p = mcu_p->sched_ctrl_map.lookup( sched_ctrl_name );
	if( sched_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( add_ctrl_name );
		if( ctrl_p )	{
			sched_p->schedule( 
				ctrl_p, 
				T_at, 
				ctrl_p->indt(), 
				ctrl_p->outdt()
			);
			return( CMD_SUCCESS );
		}
	}

	return( CMD_FAILURE );
}

int add_controller_to_scheduler( 
	char *sched_ctrl_name, 
	char *add_ctrl_name, 
	float T_at, 
	float ease_in, 
	float ease_out, 
	mcuCBHandle *mcu_p
)	{
	
	MeCtScheduler2 *sched_p = mcu_p->sched_ctrl_map.lookup( sched_ctrl_name );
	if( sched_p )	{
		MeController *ctrl_p = mcu_p->controller_map.lookup( add_ctrl_name );
		if( ctrl_p )	{
			sched_p->schedule( 
				ctrl_p, 
				T_at, 
				ease_in, 
				ease_out
			);
			return( CMD_SUCCESS );
		}
	}

	return( CMD_FAILURE );
}

/*

	sched <> add <ctrl-name> <at> [<ease-in> [<ease-out>]] 

*/

int mcu_sched_controller_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{

	if( mcu_p )	{
		char *sched_ctrl_name = args.read_token();
		char *sched_ctrl_cmd = args.read_token();
		
		if( strcmp( sched_ctrl_cmd, "add" ) == 0 )	{
			char *add_ctrl_name = args.read_token();
			float T_at = args.read_float();
			int n = args.calc_num_tokens();
			if( n )	{
				float ease_in = args.read_float();
				float ease_out = args.read_float();
				return(
					add_controller_to_scheduler( 
						sched_ctrl_name, 
						add_ctrl_name, 
						T_at, 
						ease_in, 
						ease_out, 
						mcu_p
					)
				);
			}
			else	{
				return(
					add_controller_to_scheduler( 
						sched_ctrl_name, 
						add_ctrl_name, 
						T_at, 
						mcu_p
					)
				);
			}
		}
		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}

/////////////////////////////////////////////////////////////

/*

	load ME | content <me-file>
	load motion <file-path> [-R]
	load pose <file-path> [-R]
	
*/

int mcu_load_func( srArgBuffer& args, mcuCBHandle *mcu_p )	{
	if( mcu_p )	{
		char *load_cmd = args.read_token();
		
		if( strcmp( load_cmd, "motion" )==0 ||
		    strcmp( load_cmd, "motions" )==0 )
		{
			const char* token = args.read_token();

			bool recursive = false;
			if( strcmp( token, "-R" )==0 ) {
				recursive = true;
				token = args.read_token();
			}
			return mcu_p->load_motions( token, recursive );
		} else if( strcmp( load_cmd, "pose" )==0 ||
		           strcmp( load_cmd, "poses" )==0 )
		{
			const char* token = args.read_token();

			bool recursive = false;
			if( strcmp( token, "-R" )==0 ) {
				recursive = true;
				token = args.read_token();
			}
			return mcu_p->load_poses( token, recursive );
		}

		return( CMD_NOT_FOUND );
	}
	return( CMD_FAILURE );
}

/*

   net reset function -
   helpful in resyncing TCP socket connection to unreal if it breaks
	<EMF>
*/


int mcu_net_reset( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	bool ret;
	mcu_p->bonebus.CloseConnection();
	if (mcu_p->net_host)
		ret = mcu_p->bonebus.OpenConnection(mcu_p->net_host);
	
	if (!ret)
		return (CMD_SUCCESS);
	else
		return (CMD_FAILURE);
}
/*

   net boneupdates <0|1>
   net worldoffsetupdates <0|1>

*/

int mcu_net_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

   if( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "boneupdates" ) == 0 )
      {
         // turns on/off sending character bone information across the network
         // global setting that affects all characters and bones

         int enable = args.read_int();
         mcu_p->net_bone_updates = enable ? true : false;
         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "worldoffsetupdates" ) == 0 )
      {
         // turns on/off sending character world offset information across the network
         // global setting that affects all characters

         int enable = args.read_int();
         mcu_p->net_world_offset_updates = enable ? true : false;
         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*

   PlaySound <sound_file> <character_id>  // if sound_file starts with '<drive>:' - uses absolute path
                                          // if not - uses relative path, prepends absolute path of top-level saso directory to string
                                          // character_id can be used to position the sound where a character is in the world
*/

int mcu_play_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();


      if ( strlen( command ) > 0 )
      {
         bool absolutePath = false;

         if ( strlen( command ) > 1 )
         {
            if ( command[ 0 ] == '\\' ||
                 command[ 0 ] == '/' ||
                 command[ 1 ] == ':' )
            {
               absolutePath = true;
            }
         }

         string path = command;

         if ( !absolutePath )
         {
            char full[ _MAX_PATH ];
            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            {
               path = string( full ) + string( "\\" ) + path;
            }
         }

         char * charName = args.read_token();

         if ( mcu_p->play_internal_audio )
         {
            AUDIO_Play( path.c_str() );
         }
         else
         {
            mcu_p->bonebus.SendPlaySound( path.c_str(), charName );
         }

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*

   StopSound <sound_file>  // if sound_file starts with '<drive>:' - uses absolute path
                           // if not - uses relative path, prepends absolute path of top-level saso directory to string

*/

int mcu_stop_sound_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();

      if ( strlen( command ) > 0 )
      {
         bool absolutePath = false;

         if ( strlen( command ) > 1 )
         {
            if ( command[ 0 ] == '\\' ||
                 command[ 0 ] == '/' ||
                 command[ 1 ] == ':' )
            {
               absolutePath = true;
            }
         }

         string path = command;

         if ( !absolutePath )
         {
            char full[ _MAX_PATH ];
            if ( _fullpath( full, "..\\..\\..\\..\\..", _MAX_PATH ) != NULL )
            {
               path = string( full ) + string( "\\" ) + path;
            }
         }

         //SendStopSound( path.c_str() );
         mcu_p->bonebus.SendStopSound( path.c_str() );

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}


/*
   uscriptexec <uscript command>  - Passes command straight through to Unreal where it executes the given script command
                                  - This function existed in dimr, and is only supplied because other groups were using this command
                                  - and wish to keep using it, but don't want to have to run dimr (because of license issues).
*/

int mcu_uscriptexec_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      int num = args.calc_num_tokens();

      if ( num > 0 )
      {
         string command;

         command += args.read_token();

         int i = 0;
         while ( ++i < num )
         {
            command += " ";
            command += args.read_token();
         }

         //SendWinsockExecScript( command.c_str() );
         mcu_p->bonebus.ExecScript( command.c_str() );

         return CMD_SUCCESS;
      }
   }

   return CMD_FAILURE;
}


/*
   EDF - Temporary CommAPI hooks into the new CommAPI.  Being used by Ram's group to test out gaze/point detection.

   CommAPI create <unique_id> <object_name> <object_class>
      Creates an object in the world
      unique_id = a unique integer identifier of the client's choosing, to identify objects
      object_name = a string name for human identification
      object_class = a Unreal class to create

   CommAPI remove <unique_id>
      Removes an object previously created

   CommAPI setposition <unique_id> <x> <y> <z>
      Sets an object's position in the world.
      x,y,z = Unreal world coordinates to set the object to.

   CommAPI setpositionbyname <char_name> <x> <y> <z>
      Sets an object's position in the world.
      x,y,z = Unreal world coordinates to set the object to.

   CommAPI setrotation <unique_id> <h> <p> <r>
      Sets an object's rotation in the world.
      h,p,r = Orientation.

   CommAPI setrotationbyname <char_name> <h> <p> <r>
      Sets an object's rotation in the world.
      h,p,r = Orientation.

   CommAPI setcameraposition <x> <y> <z>
      Sets the camera's position in the world.
      x,y,z = Unreal world coordinates to set the object to.

   CommAPI setcamerarotation <r> <p> <h>
      Sets the camera's rotation in the world.
      r,p,h = Orientation in degrees.

*/

int mcu_commapi_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "create" ) == 0 )
      {
         printf( "CommAPI create ERR: command no longer supported\n" );
         return CMD_FAILURE;
      }
      else if ( _stricmp( command, "remove" ) == 0 )
      {
         int unique_id = args.read_int();

         BoneBusCharacter * character = mcu_p->bonebus.FindCharacter( unique_id );
         if ( character )
         {
            mcu_p->bonebus.DeleteCharacter( character );
         }

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setposition" ) == 0 )
      {
         int unique_id = args.read_int();
         float x = args.read_float();
         float y = args.read_float();
         float z = args.read_float();

         BoneBusCharacter * character = mcu_p->bonebus.FindCharacter( unique_id );
         if ( character )
         {
            character->SetPosition( x, y, z, mcu_p->time );
         }

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setpositionbyname" ) == 0 )
      {
         char * name = args.read_token();
         float x = args.read_float();
         float y = args.read_float();
         float z = args.read_float();

         BoneBusCharacter * character = mcu_p->bonebus.FindCharacterByName( name );
         if ( character )
         {
            character->SetPosition( x, y, z, mcu_p->time );
         }

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setrotation" ) == 0 )
      {
         int unique_id = args.read_int();
         float h = args.read_float();
         float p = args.read_float();
         float r = args.read_float();

         quat_t q = euler_t( h, p, r );

         BoneBusCharacter * character = mcu_p->bonebus.FindCharacter( unique_id );
         if ( character )
         {
            character->SetRotation( (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z(), mcu_p->time );
         }

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setrotationbyname" ) == 0 )
      {
         char * name = args.read_token();
         float h = args.read_float();
         float p = args.read_float();
         float r = args.read_float();

         quat_t q = euler_t( h, p, r );

         BoneBusCharacter * character = mcu_p->bonebus.FindCharacterByName( name );
         if ( character )
         {
            character->SetRotation( (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z(), mcu_p->time );
         }

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setcameraposition" ) == 0 )
      {
         float x = args.read_float();
         float y = args.read_float();
         float z = args.read_float();

         //NetworkSetCameraPosition( x, y, z );
         mcu_p->bonebus.SetCameraPosition( x, y, z );

         return CMD_SUCCESS;
      }
      else if ( _stricmp( command, "setcamerarotation" ) == 0 )
      {
         float h = args.read_float();
         float p = args.read_float();
         float r = args.read_float();

         quat_t q = euler_t( h, p, r );

         //NetworkSetCameraRotation( (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() );
         mcu_p->bonebus.SetCameraRotation( (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() );

         return CMD_SUCCESS;
      }

      return CMD_NOT_FOUND;
   }

   return CMD_FAILURE;
}



/*
   vrKillComponent sbm
   vrKillComponent all
      Kills the sbm process
*/

int mcu_vrKillComponent_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
   if ( mcu_p )
   {
      char * command = args.read_token();

      if ( _stricmp( command, "sbm" ) == 0 ||
           _stricmp( command, "all" ) == 0 )
      {
	      mcu_p->loop = false;
	      return CMD_SUCCESS;
      }
   }

	return CMD_SUCCESS;
}


/*
   vrAllCall
     In response to this message, send out vrComponent to indicate that this component is running
*/
int mcu_vrAllCall_func( srArgBuffer& args, mcuCBHandle *mcu_p )
{
    if ( mcu_p )
    {
	    // Anm - All of the following need to identify mcu_p->process_id
		//       but that breaks the following syntax
        mcu_p->vhmsg_send( "vrComponent sbm" );
 
        // EDF - For our reply, we're going to send one vrComponent 
        //       message for each agent loaded
        SbmCharacter * char_p;
        mcu_p->character_map.reset();
        while ( char_p = mcu_p->character_map.next() )
        {
            string message = "sbm ";
            message += char_p->name;
            mcu_p->vhmsg_send( "vrComponent", message.c_str() );
        }
    }

	return CMD_SUCCESS;
}



/////////////////////////////////////////////////////////////

/*
	Print contents of 
		srHashMap <SkPosture>		pose_map;
		srHashMap <SkMotion>		motion_map;
		srHashMap <MeCtPose>		pose_ctrl_map;
		srHashMap <MeCtMotion>		motion_ctrl_map;
		srHashMap <MeCtSimpleNod>	snod_ctrl_map;
		srHashMap <MeCtAnkleLilt>	lilt_ctrl_map;
		srHashMap <MeCtScheduler2>	sched_ctrl_map;
		srHashMap <MeController>	controller_map; 
*/

int mcu_divulge_content_func( srArgBuffer& args, mcuCBHandle* mcu_p ) {

	printf( "POSES:\n" );
	mcu_p->pose_map.reset();
	SkPosture * pose_p;
	while( pose_p = mcu_p->pose_map.next() )	{
		printf( "  '%s'\n", pose_p->name() );
	}
	
	printf( "MOTIONS:\n" );
	mcu_p->motion_map.reset();
	SkMotion * mot_p;
	while( mot_p = mcu_p->motion_map.next() )	{
		printf( "  '%s'\n", mot_p->name() );
	}
	
	printf( "POSE CTRL:\n" );
	mcu_p->pose_ctrl_map.reset();
	MeCtPose * pose_ctrl_p;
	while( pose_ctrl_p = mcu_p->pose_ctrl_map.next() )	{
		printf( "  '%s' : '%s'\n", pose_ctrl_p->name(), pose_ctrl_p->posture_name() );
	}
	
	printf( "MOTION CTRL:\n" );
	mcu_p->motion_ctrl_map.reset();
	MeCtMotion * mot_ctrl_p;
	while( mot_ctrl_p = mcu_p->motion_ctrl_map.next() )	{
		printf( "  '%s' : '%s'\n", mot_ctrl_p->name(), mot_ctrl_p->motion()->name() );
	}
	
	printf( "SIMPLE-NOD:\n" );
	mcu_p->snod_ctrl_map.reset();
	MeCtSimpleNod * snod_p;
	while( snod_p = mcu_p->snod_ctrl_map.next() )	{
		printf( "  '%s'\n", snod_p->name() );
	}
	
	printf( "ANKLE-LILT:\n" );
	mcu_p->lilt_ctrl_map.reset();
	MeCtAnkleLilt * lilt_p;
	while( lilt_p = mcu_p->lilt_ctrl_map.next() )	{
		printf( "  '%s'\n", lilt_p->name() );
	}
	
	printf( "SCHEDULE:\n" );
	mcu_p->sched_ctrl_map.reset();
	MeCtScheduler2 * sched_p;

	while( sched_p = mcu_p->sched_ctrl_map.next() )	{
		printf( "  '%s'\n", sched_p->name() );
	}
	
	printf( "ALL CONTROLLERS:\n" );
	mcu_p->controller_map.reset();
	MeController * ctrl_p;
	while( ctrl_p = mcu_p->controller_map.next() )	{
		printf( "  '%s'\n", ctrl_p->name() );
	}
	
	return (CMD_SUCCESS);
}

/////////////////////////////////////////////////////////////

int mcu_wsp_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	mcu_p->theWSP->process_command( args.read_remainder_raw() );

	return( CMD_SUCCESS );
}

/////////////////////////////////////////////////////////////
