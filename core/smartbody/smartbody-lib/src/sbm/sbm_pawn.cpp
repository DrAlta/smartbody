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

#include "vhcl.h"

#include "sbm_pawn.hpp"

#include <string.h>
#include <iostream>

#include "wsp.h"

#include "mcontrol_util.h"
#include "me_utilities.hpp"
#include "sr/sr_model.h"



using namespace std;
using namespace WSP;

// Predeclare private functions defined below
inline bool parse_float_or_error( float& var, const char* str, const string& var_name );

/////////////////////////////////////////////////////////////
//  WSP Callbacks

WSP::WSP_ERROR remote_pawn_position_update( std::string id, std::string attribute_name, wsp_vector & vector_3d, void * data, const std::string & data_provider )
{
	mcuCBHandle * mcu_p = static_cast< mcuCBHandle * >( data );

	SbmPawn * pawn_p = mcu_p->pawn_map.lookup( id );
	if ( pawn_p != NULL )
	{
		float x, y, z, h, p, r;
		pawn_p->get_world_offset( x, y, z, h, p, r );

		pawn_p->set_world_offset( (float)vector_3d.x, (float)vector_3d.y, (float)vector_3d.z, h, p, r );
	}
	else
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::remote_pawn_position_update: SbmPawn '" << id << "' is NULL, cannot set_world_offset";
		LOG(strstr.str().c_str());
		return not_found_error( "SbmPawn is NULL" );
	}

	return no_error();
}

WSP::WSP_ERROR remote_pawn_rotation_update( std::string id, std::string attribute_name, wsp_vector & vector_4d, void * data, const std::string & data_provider )
{
	mcuCBHandle * mcu_p = static_cast< mcuCBHandle * >( data );

	SbmPawn * pawn_p = mcu_p->pawn_map.lookup( id );

	if ( pawn_p != NULL )
	{
		float x, y, z, h, p, r;
		pawn_p->get_world_offset( x, y, z, h, p, r );

		euler_t e = quat_t( vector_4d.q, vector_4d.x, vector_4d.y, vector_4d.z );
		pawn_p->set_world_offset( x, y, z, (float)e.h(), (float)e.p(), (float)e.r() );
	}
	else
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::remote_pawn_rotation_update: SbmPawn '" << id << "' is NULL, cannot set_world_offset";
		LOG(strstr.str().c_str());
		return not_found_error( "SbmPawn is NULL" );
	}

	return no_error();
}

void handle_wsp_error( std::string id, std::string attribute_name, int error, std::string reason, void* data ) {

	LOG( "error getting id: %s attribute_name: %s. error_code: %d reason: %s\n", id.c_str(), attribute_name.c_str(), error, reason.c_str() );
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
	dMesh_p( new DeformableMesh() ),
	ct_tree_p( MeControllerTreeRoot::create() ),
	world_offset_writer_p( new MeCtRawWriter() ),
	wo_cache_timestamp( -std::numeric_limits<float>::max() )
{
	strcpy( this->name, name );
	//skeleton_p->ref();
	scene_p->ref();

	ct_tree_p->ref();

	// world_offset_writer_p, applies external inputs to the skeleton,
	//   and therefore needs to evaluate before other controllers
	world_offset_writer_p->ref();
	ct_tree_p->add_controller( world_offset_writer_p );
}

int SbmPawn::init( SkSkeleton* new_skeleton_p ) {
	if( skeleton_p ) {
		ct_tree_p->remove_skeleton( skeleton_p->name() );
		skeleton_p->unref();
	}
	skeleton_p = new_skeleton_p;
	if( skeleton_p ) {
		skeleton_p->ref();
		if( init_skeleton()!=CMD_SUCCESS ) {
			return CMD_FAILURE; 
		}
		ct_tree_p->add_skeleton( skeleton_p->name(), skeleton_p );
	}
	scene_p->init( skeleton_p );  // if skeleton_p == NULL, the scene is cleared
	dMesh_p->skeleton = new_skeleton_p;

	// Name the controllers
	string ct_name( name );
	ct_name += "'s world_offset writer";
	world_offset_writer_p->name( ct_name.c_str() );

	return CMD_SUCCESS;
}

int SbmPawn::init_skeleton() {
	// Verifiy the joint name is not already in use.
	if( skeleton_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME ) ) {
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::init_skeleton_offset: Skeleton already contains joint \"" << SbmPawn::WORLD_OFFSET_JOINT_NAME << "\".";
		LOG(strstr.str().c_str());
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

void SbmPawn::reset_all_channels()
{
	SkChannelArray& channels = skeleton_p->channels();
	MeFrameData& frameData = ct_tree_p->getLastFrame();
	SrBuffer<float> sr_fbuff = frameData.buffer();
	int n = channels.size();
	for (int c = 0; c < n; c++)
	{
		SkChannel& chan = channels[c];
		int buffIndex = ct_tree_p->toBufferIndex(c);
		if( buffIndex > -1 )	
		{
			// Assume only have Quat or X/Y/Z
			if (chan.type == SkChannel::Quat)
			{
				sr_fbuff[ buffIndex + 0 ] = 1.0f;
				sr_fbuff[ buffIndex + 1 ] = 0.0f;
				sr_fbuff[ buffIndex + 2 ] = 0.0f;
				sr_fbuff[ buffIndex + 3 ] = 0.0f;			
			}
			else
			{
				sr_fbuff[ buffIndex ] = 0.0f;
			}
		}
	}
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
	//  TODO: walk the controller tree for excessive world offset raw writers
	return CMD_SUCCESS;
}

void SbmPawn::remove_from_scene() {
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	
	if( scene_p != NULL )
		mcu.remove_scene( scene_p );
	mcu.pawn_map.remove( name );
}

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
void SbmPawn::register_controller_cleanup( MeController* ct, controller_cleanup_callback_fp func ) {
	ct_cleanup_funcs.insert( make_pair( ct, func ) );
}

void SbmPawn::exec_controller_cleanup( MeController* ct, mcuCBHandle* mcu_p ) {
	typedef std::multimap<MeController*,controller_cleanup_callback_fp>::iterator fp_iterator;
	
	fp_iterator lower = ct_cleanup_funcs.lower_bound( ct );
	fp_iterator upper = ct_cleanup_funcs.upper_bound( ct );

	for( fp_iterator it = lower; it != upper; ++it ) {
		controller_cleanup_callback_fp func = it->second;
		if( func != NULL )
			func( ct, this, mcu_p );
	}
	if( lower != upper ) {
		ct_cleanup_funcs.erase( ct );
	}
}
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

//  Destructor
SbmPawn::~SbmPawn()	{
	if ( world_offset_writer_p )
		world_offset_writer_p->unref();

	ct_tree_p->clear();  // Because controllers within reference back to tree root context

	scene_p->unref();
	if( skeleton_p )
		skeleton_p->unref();
	ct_tree_p->unref();
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

	
	std::stringstream strstr;
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
			strstr << "ERROR: SbmPawn::set_world_offset(..): Unsupported joint rotation type SwingTwist.";
			LOG(strstr.str().c_str());
			break;
		default:
			strstr << "ERROR: SbmPawn::set_world_offset(..): Unknown joint rotation type: " << woj->rot_type();
			LOG(strstr.str().c_str());
			break;
	}
}

void SbmPawn::wo_cache_update() {
	const SkJoint* joint = get_world_offset_joint();
	if( joint==NULL )
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::wo_cache_update(..): \"" << name << "\" does not have a " << WORLD_OFFSET_JOINT_NAME << " joint.";
		LOG(strstr.str().c_str());
		return;
	}
	const SkJointPos* pos = joint->const_pos();
	float x = pos->value( SkJointPos::X );
	float y = pos->value( SkJointPos::Y );
	float z = pos->value( SkJointPos::Z );
	this->wo_cache.x = x;
	this->wo_cache.y = y;
	this->wo_cache.z = z;

	SkJoint::RotType rot_type = joint->rot_type();
	if( rot_type != SkJoint::TypeQuat ) {
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::wo_cache_update(..): Unsupported world_offset rotation type: " << rot_type << " (Expected TypeQuat, "<<SkJoint::TypeQuat<<")";
		LOG(strstr.str().c_str());
		return;
	}

	// const_cast because the SrQuat does validation (no const version of value())
	const SrQuat& quat = (const_cast<SkJoint*>(joint))->quat()->value();
	euler_t euler( quat_t( quat.w, quat.x, quat.y, quat.z ) );
	// Marcus's mappings
	float p = (float)euler.x();
	float h = (float)euler.y();
	float r = (float)euler.z();
	this->wo_cache.p = p;
	this->wo_cache.h = h;
	this->wo_cache.r = r;
}


int SbmPawn::pawn_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_name = args.read_token();
	if( pawn_name.length()==0 ) {
		
		std::stringstream strstr;
		strstr << "ERROR: Expected pawn name." << endl;
		LOG(strstr.str().c_str());
		return CMD_FAILURE;
	}

	string pawn_cmd  = args.read_token();
	if( pawn_cmd.length()==0 ) {
		std::stringstream strstr;
		strstr << "ERROR: Expected pawn command." << endl;
		LOG(strstr.str().c_str());
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
			std::stringstream strstr;
			strstr << "ERROR: Invalid SbmPawn name \"" << pawn_name << "\"." << endl;
			LOG(strstr.str().c_str());
			return( CMD_FAILURE );
		}
		if( pawn_p != NULL ) {
			std::stringstream strstr;
			strstr << "ERROR: Pawn \"" << pawn_name << "\" already exists." << endl;
			LOG(strstr.str().c_str());
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
				std::stringstream strstr;
				strstr << "WARNING: Unrecognized pawn init option \"" << option << "\"." << endl;
				LOG(strstr.str().c_str());
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
			LOG("WARNING: SbmPawn geometry not implemented.  Ignoring options.");
		}

		int err = pawn_p->init( skeleton );
		if( err != CMD_SUCCESS ) {
			std::stringstream strstr;		
			strstr << "ERROR: Unable to initialize SbmPawn \"" << pawn_name << "\".";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		err = mcu_p->pawn_map.insert( pawn_name.c_str(), pawn_p );
		if( err != CMD_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		err = mcu_p->add_scene( pawn_p->scene_p );
		if( err != CMD_SUCCESS )	{
			std::stringstream strstr;
			strstr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}


		// [BMLR] Send notification to the renderer that a pawn was created.
		// NOTE: This is sent both for characters AND pawns
		mcu_p->bonebus.SendCreatePawn( pawn_name.c_str(), loc[ 0 ], loc[ 1 ], loc[ 2 ] );


		return CMD_SUCCESS;
	} else if( pawn_cmd=="prune" ) {  // Prunes the controller trees of unused/overwritten controllers
		int result = CMD_SUCCESS;
		if( all_pawns ) {
			// Prune all pawns
			mcu_p->pawn_map.reset();
			while( pawn_p = mcu_p->pawn_map.next() ) {
				if( pawn_p->prune_controller_tree() != CMD_SUCCESS ) {
					std::stringstream strstr;
					strstr << "ERROR: Failed to prune pawn \""<<pawn_name<<"\".";
					LOG(strstr.str().c_str());
					result = CMD_FAILURE;
				}
			}
		} else if( pawn_p ) {
			result = pawn_p->prune_controller_tree();
			if( result != CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "ERROR: Failed to prune pawn \""<<pawn_name<<"\".";
				LOG(strstr.str().c_str());
				result = CMD_FAILURE;
			}
		} else {
			std::stringstream strstr;
			strstr << "ERROR: Pawn \""<<pawn_name<<"\" not found.";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
		return result;
	} else if( pawn_cmd=="remove" ) {
		if( pawn_p != NULL ) {
			
			pawn_p->remove_from_scene();
			return CMD_SUCCESS;
		} else {
			std::stringstream strstr;
			strstr << "ERROR: Pawn \""<<pawn_name<<"\" not found.";
			LOG(strstr.str().c_str());
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
			LOG( "ERROR: Unknown pawn \"%s\".\n", pawn_name );
			return CMD_FAILURE;
		}
	}
}

int SbmPawn::set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_id = args.read_token();
	if( pawn_id.length()==0 ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Missing pawn id.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->pawn_map.lookup( pawn_id.c_str() );
	if( pawn==NULL ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Unknown pawn id \"%s\".", pawn_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Missing attribute \"%s\" to set.", attribute.c_str());
		return CMD_FAILURE;
	}

	return set_attribute( pawn, attribute, args, mcu_p );
}

int SbmPawn::set_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: set pawn <character id> world_offset ...
		//  Sets the parameters of the world_offset joint
		return SbmPawn::set_world_offset_cmd( pawn, args );
	} 
	else if (attribute == "mass")
	{
		if (args.calc_num_tokens() == 0)
		{
			SkSkeleton* skeleton = pawn->skeleton_p;
			SrArray<SkJoint*>& joints = skeleton->get_joint_array();
			for (int j = 0; j < joints.size(); j++)
			{
				LOG("%s : %f", joints[j]->name().get_string(), joints[j]->mass());
				
			}
			return CMD_SUCCESS;
		}
		std::string jointName = args.read_token();
		if (jointName.length() == 0)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): Need joint name. Use: set char mass <joint> <amount>");
			return CMD_FAILURE;
		}
		const SkJoint* joint = pawn->get_joint(jointName.c_str());
		if (!joint)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): No joint found with name '%s'.", jointName.c_str());
			return CMD_FAILURE;
		}
		float mass = args.read_float();
		if (mass < 0)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): Mass must be > 0.");
			return CMD_FAILURE;
		}
		// is there a function that returns an SkJoint* and not a const SkJoint*?
		// That would make this next line of code unnecessary.
		SkJoint* editableJoint = const_cast<SkJoint*>(joint);
		editableJoint->mass(mass);
		//LOG("Set joint '%s' on character '%s' to mass '%f'.", jointName.c_str(), pawn->name, mass);
		return CMD_SUCCESS;
	} 
	else 
	{
		LOG("ERROR: SbmPawn::set_cmd_func(..): Unknown attribute \"%s\".", attribute);
		return CMD_FAILURE;
	}
}


int SbmPawn::set_world_offset_cmd( SbmPawn* pawn, srArgBuffer& args ) {
	float x, y, z, h, p, r;
	pawn->get_world_offset( x, y, z, h, p, r );

	bool has_error = false;
	string arg = args.read_token();
	if( arg.length() == 0 ) {
		LOG("ERROR: SbmPawn::set_world_offset: Missing offset parameters.");
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
			has_error |= !parse_float_or_error( h, args.read_token(), "heading" );
			has_error |= !parse_float_or_error( p, args.read_token(), arg );
			has_error |= !parse_float_or_error( r, args.read_token(), arg );
		} else {
			LOG("ERROR: Unknown world_offset attribute \"%s\".", arg.c_str());
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
		LOG("ERROR: SbmPawn::print_cmd_func(..): Missing pawn id.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->pawn_map.lookup( pawn_id.c_str() );
	if( pawn==NULL ) {
		LOG("ERROR: SbmPawn::print_cmd_func(..): Unknown pawn \"%s\".", pawn_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmPawn::print_cmd_func(..): Missing attribute to print.");
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
			LOG("ERROR: SbmPawn::print_attribute(..): Missing joint name of joint to print.");
			return CMD_FAILURE;
		}

		do {
			cout << "pawn " << pawn->name << " joint "<<joint_name<<":\t";
			const SkJoint* joint = pawn->get_joint( joint_name.c_str() );
			if( joint==NULL ) {
				LOG("No joint \"%s\".", joint_name);
			} else {
				print_joint( joint );
			}

			joint_name = args.read_token();
		} while( joint_name.length()>0 );

		return CMD_SUCCESS;
	} else {
		LOG("ERROR: SbmPawn::print_attribute(..): Unknown attribute \"%s\".", attribute);
		return CMD_FAILURE;
	}
}

int SbmPawn::create_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	
	std::string pawn_and_attribute = args.read_token();
	int interval = args.read_int();

	if( pawn_and_attribute.length()==0 ) {
		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->pawn_map.lookup( pawn_and_attribute.c_str() );

	if( pawn_p != NULL ) {
		LOG("ERROR: Pawn \"%s\" already exists.", pawn_and_attribute);
		return CMD_FAILURE;
	}

	pawn_p = new SbmPawn( pawn_and_attribute.c_str() );

	SkSkeleton* skeleton = new SkSkeleton();
	skeleton->ref();
	std::string skel_name = pawn_and_attribute+"-skel";
	skeleton->name( skel_name.c_str() );
	// Init channels
	skeleton->make_active_channels();	

	int err = pawn_p->init( skeleton );

	if( err != CMD_SUCCESS ) {
		LOG("ERROR: Unable to initialize SbmPawn \"%s\".", pawn_and_attribute);
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->pawn_map.insert( pawn_and_attribute.c_str(), pawn_p );

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute);
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->add_scene( pawn_p->scene_p );

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute);
		delete pawn_p;
		skeleton->unref();
		return err;
	}


	mcu_p->theWSP->subscribe_vector_3d_interval( pawn_and_attribute, "position", interval, handle_wsp_error, remote_pawn_position_update, mcu_p );
	mcu_p->theWSP->subscribe_vector_4d_interval( pawn_and_attribute, "rotation", interval, handle_wsp_error, remote_pawn_rotation_update, mcu_p );

	return( CMD_SUCCESS );
}

int SbmPawn::remove_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	std::string pawn_name = args.read_token();

	if( pawn_name.length()==0 ) {
	
		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->pawn_map.lookup( pawn_name.c_str() );

	if( pawn_p != NULL ) {

		mcu_p->theWSP->unsubscribe( pawn_name, "position", 1 );
		pawn_p->remove_from_scene();

		return CMD_SUCCESS;
	} else {
		LOG("ERROR: Pawn \"%s\" not found.", pawn_name);
		return CMD_FAILURE;
	}
}

WSP_ERROR SbmPawn::wsp_world_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data )
{
	SbmPawn * pawn_p = (SbmPawn *)data;

	const SkJoint * wo_joint = pawn_p->get_world_offset_joint();
	if ( wo_joint != NULL )
	{
		value.x = wo_joint->const_pos()->value(0);
		value.y = wo_joint->const_pos()->value(1);
		value.z = wo_joint->const_pos()->value(2);
		value.num_dimensions = 3;

		return WSP::no_error();
	}
	else
	{
		return WSP::not_found_error( "no world_offset joint" );
	}
}

WSP_ERROR SbmPawn::wsp_world_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data )
{
	SbmPawn * pawn_p = (SbmPawn *)data;

	const SkJoint * wo_joint = pawn_p->get_world_offset_joint();
	if ( wo_joint != NULL )
	{
		value.x = ((SkJoint *)wo_joint)->quat()->value().x;
		value.y = ((SkJoint *)wo_joint)->quat()->value().y;
		value.z = ((SkJoint *)wo_joint)->quat()->value().z;
		value.q = ((SkJoint *)wo_joint)->quat()->value().w;
		value.num_dimensions = 4;

		return WSP::no_error();
	}
	else
	{
		return WSP::not_found_error( "no world_offset joint" );
	}
}

WSP_ERROR SbmPawn::wsp_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data )
{
	SbmPawn * pawn_p = (SbmPawn *)data;

	vector< string > tokens;
	vhcl::Tokenize( id, tokens, ":" );
	string & char_name = tokens[ 0 ];
	string & joint_name = tokens[ 1 ];

	SkJoint * joint = pawn_p->skeleton_p->search_joint( joint_name.c_str() );
	if ( joint != NULL )
	{
		joint->update_gmat();
		const SrMat & sr_m = joint->gmat();

		matrix_t m;
		for ( int i=0; i<4; i++ )
		{
			for ( int j=0; j<4; j++ )
			{
				m.set( i, j, sr_m.get( i, j ) );
			}
		}

		vector_t pos = m.translation( GWIZ_M_TR );

		value.x = pos.x();
		value.y = pos.y();
		value.z = pos.z();
		value.num_dimensions = 3;

		return WSP::no_error();
	}
	else
	{
		return WSP::not_found_error( "no joint" );
	}
}

WSP_ERROR SbmPawn::wsp_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data )
{
	SbmPawn * pawn_p = (SbmPawn *)data;

	vector< string > tokens;
	vhcl::Tokenize( id, tokens, ":" );
	string & char_name = tokens[ 0 ];
	string & joint_name = tokens[ 1 ];

	SkJoint * joint = pawn_p->skeleton_p->search_joint( joint_name.c_str() );
	if ( joint != NULL )
	{
		joint->update_gmat();
		const SrMat & sr_m = joint->gmat();

		matrix_t m;
		for( int i=0; i<4; i++ )
		{
			for( int j=0; j<4; j++ )
			{
				m.set( i, j, sr_m.get( i, j ) );
			}
		}

		quat_t quat = m.quat( GWIZ_M_TR );

		value.x = quat.x();
		value.y = quat.y();
		value.z = quat.z();
		value.q = quat.w();
		value.num_dimensions = 4;

		return WSP::no_error();
	}
	else
	{
		return WSP::not_found_error( "no joint" );
	}
}


///////////////////////////////////////////////////////////////////////////
//  Private sbm_pawn functions

// Print error on error..
bool parse_float_or_error( float& var, const char* str, const string& var_name ) {
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	LOG("ERROR: Invalid value for %s: %s", var_name.c_str(), str);
	return false;
}
