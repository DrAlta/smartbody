/*
 *  sbm_pawn.cpp - part of SmartBody-lib
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
 *      Andrew n marshall, USC
 *      Ed Fast, USC
 *      Thomas Amundsen, USC
 */

#include <string.h>
#include <iostream>

#include "wsp.h"

#include "sbm_pawn.hpp"
#include "mcontrol_util.h"
#include "me_utilities.hpp"



using namespace std;
using namespace WSP;

// Predeclare private functions defined below
inline bool parse_float_or_error( float& var, const char* str, const string& var_name );

/////////////////////////////////////////////////////////////
//  WSP Callbacks

WSP::WSP_ERROR remote_pawn_position_update( std::string id, std::string attribute_name, wsp_vector& vector_3d, void* data, const std::string& data_provider ) {

	SbmPawn* pawn_p = NULL;
	
	mcuCBHandle *mcu_p = static_cast< mcuCBHandle* >( data );

	pawn_p = mcu_p->pawn_map.lookup( id );

	if ( pawn_p != NULL ) {

		pawn_p->set_world_offset( ( (float)vector_3d.y * -1 ), ( (float)vector_3d.z ), ( (float)vector_3d.x ), 0, 0, 0 );
	} else {

		cerr << "ERROR: SbmPawn::remote_pawn_position_update: SbmPawn '" << id << "' is NULL, cannot set_world_offset" << endl; 
		return not_found_error( "SbmPawn is NULL" );
	}

	return no_error();
}

WSP::WSP_ERROR remote_sbm_joint_update( std::string id, std::string attribute_name, wsp_vector& vector_3d, void* data ) {

	SbmPawn* pawn_p = NULL;
	
	mcuCBHandle *mcu_p = static_cast< mcuCBHandle* >( data );

	pawn_p = mcu_p->pawn_map.lookup( id );

	if ( pawn_p != NULL ) {

		SkSkeleton* skeleton_p = pawn_p->skeleton_p;

		if ( skeleton_p != NULL ) {
		
			SkJoint* joint_p = skeleton_p->search_joint( attribute_name.c_str() );

			if ( joint_p != NULL ) {

				SrQuat q = joint_p->quat()->value();

				q.set( static_cast<float>( vector_3d.q ), static_cast<float>( vector_3d.x ), static_cast<float>( vector_3d.y ), 
					static_cast<float>( vector_3d.z ) );
			} else {

				printf( "SkJoint(%s) is NULL\n", attribute_name.c_str() );

				return not_found_error( "SkJoint is NULL" );
			}
		} else {

			printf( "SkSkeleton(%s) is NULL\n", id.c_str() );

			return not_found_error( "SkSkeleton is NULL" );
		}
	} else {

		printf( "SbmPawn(%s) is NULL\n", id.c_str() );

		return not_found_error( "SbmPawn is NULL" );
	}

	return no_error();
}
void handle_wsp_error( std::string id, std::string attribute_name, int error, std::string reason, void* data ) {

	printf( "error getting id: %s attribute_name: %s. error_code: %d reason: %s\n", id.c_str(), attribute_name.c_str(), error, reason.c_str() );
}

/////////////////////////////////////////////////////////////
//  SbmPawn Constants
const char* SbmPawn::WORLD_OFFSET_JOINT_NAME = "world_offset";
SkChannelArray SbmPawn::WORLD_OFFSET_CHANNELS_P;


// Constructor
SbmPawn::SbmPawn( const char * name )
:	name( new char[strlen(name)+1] ),

	skeleton_p( NULL ),
	scene_p( new SkScene() ),
	pipeline_p( MeControllerPipeline::create() ),
	world_offset_writer_p( new MeCtRawWriter() ),
	wo_cache_timestamp( -std::numeric_limits<float>::max() )
{
	strcpy( this->name, name );
	//skeleton_p->ref();
	scene_p->ref();

	pipeline_p->ref();

	// world_offset_writer_p, applies external inputs to the skeleton,
	//   and therefore needs to evaluate before other controllers
	world_offset_writer_p->ref();
	pipeline_p->add_controller( world_offset_writer_p );
}

int SbmPawn::init( SkSkeleton* new_skeleton_p ) {
	if( skeleton_p ) {
		pipeline_p->remove_skeleton( skeleton_p->name() );
		skeleton_p->unref();
	}
	skeleton_p = new_skeleton_p;
	if( skeleton_p ) {
		skeleton_p->ref();
		if( init_skeleton()!=CMD_SUCCESS ) {
			return CMD_FAILURE; 
		}
		pipeline_p->add_skeleton( skeleton_p->name(), skeleton_p );
	}
	scene_p->init( skeleton_p );  // if skeleton_p == NULL, the scene is cleared

	// Name the controllers
	string ct_name( name );
	ct_name += "'s world_offset writer";
	world_offset_writer_p->name( ct_name.c_str() );

	return CMD_SUCCESS;
}

int SbmPawn::init_skeleton() {
	// Verifiy the joint name is not already in use.
	if( skeleton_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME ) ) {
		cerr << "ERROR: SbmPawn::init_skeleton_offset: Skeleton already contains joint \"" << SbmPawn::WORLD_OFFSET_JOINT_NAME << "\"." << endl; 
		return( CMD_FAILURE ); 
	}

	SkJoint* world_offset_joint = skeleton_p->insert_new_root_joint( SkJoint::TypeQuat );
	world_offset_joint->name( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ) );
	// Make sure the world_offset accepts new pos and quat values
	SkJointPos* world_offset_pos = world_offset_joint->pos();
	world_offset_pos->limits( SkVecLimits::X, false );
	world_offset_pos->limits( SkVecLimits::Y, false );
	world_offset_pos->limits( SkVecLimits::Z, false );
	world_offset_joint->quat()->activate();
	skeleton_p->compress();

	init_world_offset_channels();
	world_offset_writer_p->init( WORLD_OFFSET_CHANNELS_P, false );

	wo_cache.x = 0;
	wo_cache.y = 0;
	wo_cache.z = 0;
	wo_cache.h = 0;
	wo_cache.p = 0;
	wo_cache.r = 0;

	return( CMD_SUCCESS ); 
}

void SbmPawn::init_world_offset_channels() {
	if( WORLD_OFFSET_CHANNELS_P.size()==0 ) {
		SkJointName world_offset_joint_name( WORLD_OFFSET_JOINT_NAME );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::XPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::YPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::ZPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::Quat );
	}
}

bool SbmPawn::is_initialized() {
	return skeleton_p != NULL;
}

int SbmPawn::prune_controller_tree() {
	// Unimplemented...
	//  TODO: walk the controller pipeline for excessive world offset raw writers
	return CMD_SUCCESS;
}

void SbmPawn::remove_from_scene() {
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	
	if( scene_p != NULL )
		mcu.remove_scene( scene_p );
	mcu.pawn_map.remove( name );
}

//  Destructor
SbmPawn::~SbmPawn()	{
	if ( world_offset_writer_p )
		world_offset_writer_p->unref();

	pipeline_p->clear();  // Because controllers within reference back to pipeline as context

	scene_p->unref();
	if( skeleton_p )
		skeleton_p->unref();
	pipeline_p->unref();
    delete [] name;
}



const SkJoint* SbmPawn::get_joint( const char* joint_name ) const {
	return skeleton_p->search_joint( joint_name );
}

void SbmPawn::get_world_offset( float& x, float& y, float& z,
								 float& yaw, float& pitch, float& roll ) {
	if( mcuCBHandle::singleton().time != wo_cache_timestamp )
		wo_cache_update();

	x = wo_cache.x;
	y = wo_cache.y;
	z = wo_cache.z;
	yaw = wo_cache.h;
	pitch = wo_cache.p;
	roll = wo_cache.r;
	return;
}

void SbmPawn::set_world_offset( float x, float y, float z,
                                float yaw, float pitch, float roll )
{
	// Store values since they are not written to the joint immediately
	wo_cache.x = x;
	wo_cache.y = y;
	wo_cache.z = z;
	wo_cache.h = yaw;
	wo_cache.p = pitch;
	wo_cache.r = roll;
	wo_cache_timestamp = mcuCBHandle::singleton().time;

	quat_t q = euler_t(pitch,yaw,roll);
	float data[7] = { x, y, z, (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() };
	world_offset_writer_p->set_data( data );
	return;

	SkJoint* woj = skeleton_p->search_joint( WORLD_OFFSET_JOINT_NAME );
	SkJointPos* woj_pos = woj->pos();
	woj_pos->value( SkJointPos::X, x );
	woj_pos->value( SkJointPos::Y, y );
	woj_pos->value( SkJointPos::Z, z );

	switch( woj->rot_type() ) {
		case SkJoint::TypeEuler: {
				SkJointEuler* joint_euler = woj->euler();
				joint_euler->value( SkJointPos::X, pitch );
				joint_euler->value( SkJointPos::Y, yaw );
				joint_euler->value( SkJointPos::Z, roll );
			}
			break;
		case SkJoint::TypeQuat: {
				SkJointQuat* joint_quat = woj->quat();
				quat_t q = euler_t(pitch,yaw,roll);
				float q_data[4] = { (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() };
				joint_quat->value( q_data );
			}
			break;
		case SkJoint::TypeSwingTwist:
			cerr << "ERROR: SbmPawn::set_world_offset(..): Unsupported joint rotation type SwingTwist." << endl;
			break;
		default:
			cerr << "ERROR: SbmPawn::set_world_offset(..): Unknown joint rotation type: " << woj->rot_type() << endl;
			break;
	}
}

void SbmPawn::wo_cache_update() {
	const SkJoint* joint = get_world_offset_joint();
	if( joint==NULL ) {
		cerr << "ERROR: SbmPawn::wo_cache_update(..): \"" << name << "\" does not have a " << WORLD_OFFSET_JOINT_NAME << " joint." << endl;
		return;
	}
	const SkJointPos* pos = joint->const_pos();
	float x = pos->value( SkJointPos::X );
	float y = pos->value( SkJointPos::Y );
	float z = pos->value( SkJointPos::Z );

	SkJoint::RotType rot_type = joint->rot_type();
	if( rot_type != SkJoint::TypeQuat ) {
		cerr << "ERROR: SbmPawn::wo_cache_update(..): Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")"<<endl;
		return;
	}

	// const_cast because the SrQuat does validation (no const version of value())
	const SrQuat& quat = (const_cast<SkJoint*>(joint))->quat()->value();
	euler_t euler( quat_t( quat.w, quat.x, quat.y, quat.z ) );
	// Marcus's mappings
	float p = (float)euler.x();
	float h = (float)euler.y();
	float r = (float)euler.z();
}


int SbmPawn::pawn_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_name = args.read_token();
	if( pawn_name.length()==0 ) {
		cerr << "ERROR: Expected pawn name." << endl;
		return CMD_FAILURE;
	}

	string pawn_cmd  = args.read_token();
	if( pawn_cmd.length()==0 ) {
		cerr << "ERROR: Expected pawn command." << endl;
		return CMD_FAILURE;
	}

	bool all_pawns = false;
	SbmPawn* pawn_p = NULL;
	if( pawn_name=="*" ) {
		all_pawns = true;
	} else {
		pawn_p = mcu_p->pawn_map.lookup( pawn_name.c_str() );
	}

	if( pawn_cmd=="init" ) {
		// pawn <name> init [loc <x> <y> <z>] [geom <shape name>] [color <color hex>] [size <size>]

		if( pawn_name == "*" ) {  // TODO: better character name valiadtion
			cerr << "ERROR: Invalid SbmPawn name \"" << pawn_name << "\"." << endl;
			return( CMD_FAILURE );
		}
		if( pawn_p != NULL ) {
			cerr << "ERROR: Pawn \"" << pawn_name << "\" already exists." << endl;
			return CMD_FAILURE;
		}
		// Options
		double loc[3] = { 0, 0, 0 };

		bool has_geom = false;
		const char* geom_str = NULL;
		const char* size_str = NULL;
		const char* color_str = NULL;
		while( args.calc_num_tokens() > 0 ) {
			string option = args.read_token();
			// TODO: Make the following option case insensitive
			if( option == "loc" ) {
				args.read_double_vect( loc, 3 );
			} else if( option=="geom" ) {
				geom_str = args.read_token();
				has_geom = true;
			} else if( option=="size" ) {
				size_str = args.read_token();
				has_geom = true;
			} else if( option=="color" ) {
				color_str = args.read_token();
				has_geom = true;
			} else {
				cerr << "WARNING: Unrecognized pawn init option \"" << option << "\"." << endl;
			}
		}

		pawn_p = new SbmPawn( pawn_name.c_str() );

		SkSkeleton* skeleton = new SkSkeleton();
		skeleton->ref();
		string skel_name = pawn_name+"-skel";
		skeleton->name( skel_name.c_str() );
		// Init channels
		skeleton->make_active_channels();

		if( has_geom ) {
			cerr << "WARNING: SbmPawn geometry not implemented.  Ignoring options." << endl;
		}

		int err = pawn_p->init( skeleton );
		if( err != CMD_SUCCESS ) {
			cerr << "ERROR: Unable to initialize SbmPawn \"" << pawn_name << "\"." << endl;
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		err = mcu_p->pawn_map.insert( pawn_name.c_str(), pawn_p );
		if( err != CMD_SUCCESS )	{
			cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << endl; 
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		err = mcu_p->add_scene( pawn_p->scene_p );
		if( err != CMD_SUCCESS )	{
			cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << endl; 
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		return CMD_SUCCESS;
	} else if( pawn_cmd=="prune" ) {  // Prunes the controller trees of unused/overwritten controllers
		int result = CMD_SUCCESS;
		if( all_pawns ) {
			// Prune all pawns
			mcu_p->pawn_map.reset();
			while( pawn_p = mcu_p->pawn_map.next() ) {
				if( pawn_p->prune_controller_tree() != CMD_SUCCESS ) {
					cerr << "ERROR: Failed to prune pawn \""<<pawn_name<<"\"." << endl;
					result = CMD_FAILURE;
				}
			}
		} else if( pawn_p ) {
			result = pawn_p->prune_controller_tree();
			if( result != CMD_SUCCESS ) {
				cerr << "ERROR: Failed to prune pawn \""<<pawn_name<<"\"." << endl;
				result = CMD_FAILURE;
			}
		} else {
			cerr << "ERROR: Pawn \""<<pawn_name<<"\" not found." << endl;
			return CMD_FAILURE;
		}
		return result;
	} else if( pawn_cmd=="remove" ) {
		if( pawn_p != NULL ) {
			
			pawn_p->remove_from_scene();
			return CMD_SUCCESS;
		} else {
			cerr << "ERROR: Pawn \""<<pawn_name<<"\" not found." << endl;
			return CMD_FAILURE;
		}
	} else {
		return CMD_NOT_FOUND;
	}
}

int SbmPawn::remove_from_scene( const char* pawn_name ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if( strcmp( pawn_name, "*" )==0 ) {
		SbmPawn * pawn_p;
		mcu.character_map.reset();
		while( pawn_p = mcu.character_map.pull() ) {
			pawn_p->remove_from_scene();
			delete pawn_p;
		}
		return CMD_SUCCESS;
	} else {
		SbmPawn* pawn_p = mcu.character_map.lookup( pawn_name );

		if ( pawn_p ) {
			pawn_p->remove_from_scene();
			delete pawn_p;

			return CMD_SUCCESS;
		} else {
			printf( "ERROR: Unknown pawn \"%s\".\n", pawn_name );
			return CMD_FAILURE;
		}
	}
}

int SbmPawn::set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_id = args.read_token();
	if( pawn_id.length()==0 ) {
		cerr << "ERROR: SbmPawn::set_cmd_func(..): Missing pawn id." << endl;
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->pawn_map.lookup( pawn_id.c_str() );
	if( pawn==NULL ) {
		cerr << "ERROR: SbmPawn::set_cmd_func(..): Unknown pawn id \""<<pawn_id<<"\"." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmPawn::set_cmd_func(..): Missing attribute \"" << attribute << "\" to set." << endl;
		return CMD_FAILURE;
	}

	return set_attribute( pawn, attribute, args, mcu_p );
}

int SbmPawn::set_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: set pawn <character id> world_offset ...
		//  Sets the parameters of the world_offset joint
		return SbmPawn::set_world_offset_cmd( pawn, args );
	} else {
		cerr << "ERROR: SbmPawn::set_cmd_func(..): Unknown attribute \""<< attribute<<"\"." << endl;
		return CMD_FAILURE;
	}
}


int SbmPawn::set_world_offset_cmd( SbmPawn* pawn, srArgBuffer& args ) {
	float x, y, z, h, p, r;
	pawn->get_world_offset( x, y, z, h, p, r );

	bool has_error = false;
	string arg = args.read_token();
	if( arg.length() == 0 ) {
		cerr << "ERROR: SbmPawn::set_world_offset: Missing offset parameters." <<endl;
		return CMD_FAILURE;
	}

	while( arg.length() > 0 ) {
		// TODO: handle "+x", "-x", etc...
		if( arg=="x" ) {
			has_error |= !parse_float_or_error( x, args.read_token(), arg );
		} else if( arg=="y" ) {
			has_error |= !parse_float_or_error( y, args.read_token(), arg );
		} else if( arg=="z" ) {
			has_error |= !parse_float_or_error( z, args.read_token(), arg );
		} else if( arg=="z" ) {
			has_error |= !parse_float_or_error( z, args.read_token(), arg );
		} else if( arg=="p" || arg=="pitch" ) {
			has_error |= !parse_float_or_error( p, args.read_token(), "pitch" );
		} else if( arg=="r" || arg=="roll" ) {
			has_error |= !parse_float_or_error( r, args.read_token(), "roll" );
		} else if( arg=="h" || arg=="heading" || arg=="yaw" ) {
			has_error |= !parse_float_or_error( h, args.read_token(), "yaw" );
		} else if( arg=="xyz" || arg=="pos" || arg=="position" ) {
			has_error |= !parse_float_or_error( x, args.read_token(), "x" );
			has_error |= !parse_float_or_error( y, args.read_token(), "y" );
			has_error |= !parse_float_or_error( z, args.read_token(), "z" );
		} else if( arg=="hpr" ) {
			has_error |= !parse_float_or_error( x, args.read_token(), "heading" );
			has_error |= !parse_float_or_error( x, args.read_token(), arg );
			has_error |= !parse_float_or_error( x, args.read_token(), arg );
		} else {
			cerr << "ERROR: Unknown world_offset attribute \"" << arg << "\" ." << endl;
			has_error = true;
		}
		arg = args.read_token();
	}

	if( has_error )
		return CMD_FAILURE;

	pawn->set_world_offset( x, y, z, h, p, r );
	return CMD_SUCCESS;
}

int SbmPawn::print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_id = args.read_token();
	if( pawn_id.length()==0 ) {
		cerr << "ERROR: SbmPawn::print_cmd_func(..): Missing pawn id." << endl;
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->pawn_map.lookup( pawn_id.c_str() );
	if( pawn==NULL ) {
		cerr << "ERROR: SbmPawn::print_cmd_func(..): Unknown pawn \""<<pawn_id<<"\"." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmPawn::print_cmd_func(..): Missing attribute to print." << endl;
		return CMD_FAILURE;
	}

	return print_attribute( pawn, attribute, args, mcu_p );
}

int SbmPawn::print_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: print pawn <character id> world_offset
		//  Print out the current state of the world_offset joint
		cout << "pawn " << pawn->name << " world_offset:\t";
		const SkJoint* joint = pawn->get_world_offset_joint();
		if( joint==NULL ) {
			cout << "No world_offset joint." << endl;
		} else {
			print_joint( joint );
		}
		return CMD_SUCCESS;
	} else if( attribute=="joint" || attribute=="joints" ) {
		//  Command: print character <character id> [joint|joints] <joint name>*
		//  Print out the current state of the named joints
		string joint_name = args.read_token();
		if( joint_name.length()==0 ) {
			cerr << "ERROR: SbmPawn::print_attribute(..): Missing joint name of joint to print." << endl;
			return CMD_FAILURE;
		}

		do {
			cout << "pawn " << pawn->name << " joint "<<joint_name<<":\t";
			const SkJoint* joint = pawn->get_joint( joint_name.c_str() );
			if( joint==NULL ) {
				cout << "No joint \""<<joint_name<<"\"." << endl;
			} else {
				print_joint( joint );
			}

			joint_name = args.read_token();
		} while( joint_name.length()>0 );

		return CMD_SUCCESS;
	} else {
		cerr << "ERROR: SbmPawn::print_attribute(..): Unknown attribute \""<< attribute<<"\"." << endl;
		return CMD_FAILURE;
	}
}

int SbmPawn::create_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	
	std::string pawn_name = args.read_token();
	char* skel_file = args.read_token();

	if( pawn_name.length()==0 ) {
	
		std::cerr << "ERROR: Expected pawn name." << std::endl;
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->pawn_map.lookup( pawn_name.c_str() );

	if( pawn_p != NULL ) {

		std::cerr << "ERROR: Pawn \"" << pawn_name << "\" already exists." << std::endl;
		return CMD_FAILURE;
	}

	pawn_p = new SbmPawn( pawn_name.c_str() );

	SkSkeleton* skeleton = new SkSkeleton();
	skeleton->ref();
	std::string skel_name = pawn_name+"-skel";
	skeleton->name( skel_name.c_str() );
	// Init channels
	skeleton->make_active_channels();	

	int err = pawn_p->init( skeleton );

	if( err != CMD_SUCCESS ) {

		std::cerr << "ERROR: Unable to initialize SbmPawn \"" << pawn_name << "\"." << std::endl;
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->pawn_map.insert( pawn_name.c_str(), pawn_p );

	if( err != CMD_SUCCESS )	{

		std::cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << std::endl; 
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->add_scene( pawn_p->scene_p );

	if( err != CMD_SUCCESS )	{

		std::cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << std::endl; 
		delete pawn_p;
		skeleton->unref();
		return err;
	}
	

	mcu_p->theWSP->subscribe_vector_3d_interval( pawn_name, "coordinates", 10, handle_wsp_error, remote_pawn_position_update, mcu_p );

	return( CMD_SUCCESS );
}

int SbmPawn::create_remote_sbm_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	
	std::string pawn_name = args.read_token();

	if( pawn_name.length()==0 ) {
	
		std::cerr << "ERROR: Expected pawn name." << std::endl;
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->pawn_map.lookup( pawn_name.c_str() );

	if( pawn_p != NULL ) {

		std::cerr << "ERROR: Pawn \"" << pawn_name << "\" already exists." << std::endl;
		return CMD_FAILURE;
	}

	pawn_p = new SbmPawn( pawn_name.c_str() );

	SkSkeleton* skeleton = new SkSkeleton();
	skeleton->ref();
	std::string skel_name = pawn_name+"-skel";
	skeleton->name( skel_name.c_str() );
	// Init channels
	skeleton->make_active_channels();

	int err = pawn_p->init( skeleton );

	if( err != CMD_SUCCESS ) {

		std::cerr << "ERROR: Unable to initialize SbmPawn \"" << pawn_name << "\"." << std::endl;
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->pawn_map.insert( pawn_name.c_str(), pawn_p );

	if( err != CMD_SUCCESS )	{

		std::cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << std::endl; 
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->add_scene( pawn_p->scene_p );

	if( err != CMD_SUCCESS )	{

		std::cerr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED" << std::endl; 
		delete pawn_p;
		skeleton->unref();
		return err;
	}
	
	mcu_p->theWSP->subscribe_vector_3d_interval( pawn_name, "world_offset", 10, handle_wsp_error, remote_pawn_position_update, mcu_p );
//	mcu_p->theWSP->subscribe_vector_4d_interval( pawn_name, "skullbase", 1, handle_wsp_error, remote_sbm_joint_update, mcu_p );

	return( CMD_SUCCESS );
}

int SbmPawn::remove_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	std::string pawn_name = args.read_token();

	if( pawn_name.length()==0 ) {
	
		std::cerr << "ERROR: Expected pawn name." << std::endl;
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->pawn_map.lookup( pawn_name.c_str() );

	if( pawn_p != NULL ) {

		mcu_p->theWSP->unsubscribe( pawn_name, "coordinates", 1 );
		pawn_p->remove_from_scene();

		return CMD_SUCCESS;
	} else {

		cerr << "ERROR: Pawn \""<<pawn_name<<"\" not found." << endl;

		return CMD_FAILURE;
	}
}

///////////////////////////////////////////////////////////////////////////
//  Private sbm_pawn functions

// Print error on error..
bool parse_float_or_error( float& var, const char* str, const string& var_name ) {
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	cerr << "ERROR: Invalid value for " << var_name << ": " << str << endl;
	return false;
}
