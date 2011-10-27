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
#include <sbm/mcontrol_util.h>
#include "sbm_pawn.hpp"

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#if defined(__ANDROID__) || defined(SBM_IPHONE)
#include <sbm/sbm_deformable_mesh.h>
#else
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#endif

#include <string.h>
#include <iostream>

// added by AShapiro 6/30/11 - not sure why the USE_WSP variable this is not being picked up in mcontrol_util.h
#define USE_WSP 1
#if USE_WSP
#include "wsp.h"
#endif

#include "mcontrol_util.h"
#include "me_utilities.hpp"
#include "sr/sr_model.h"
#include "sr/sr_euler.h"
#include <sbm/SBSkeleton.h>

using namespace std;

#if USE_WSP
using namespace WSP;
#endif

// Predeclare private functions defined below
inline bool parse_float_or_error( float& var, const char* str, const string& var_name );

/////////////////////////////////////////////////////////////
//  WSP Callbacks

#if USE_WSP
WSP::WSP_ERROR remote_pawn_position_update( std::string id, std::string attribute_name, wsp_vector & vector_3d, void * data, const std::string & data_provider )
{
	mcuCBHandle * mcu_p = static_cast< mcuCBHandle * >( data );

	SbmPawn * pawn_p = mcu_p->getPawn( id );
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

	SbmPawn * pawn_p = mcu_p->getPawn( id );

	if ( pawn_p != NULL )
	{
		float x, y, z, h, p, r;
		pawn_p->get_world_offset( x, y, z, h, p, r );

		gwiz::euler_t e = gwiz::quat_t( vector_4d.q, vector_4d.x, vector_4d.y, vector_4d.z );
		pawn_p->set_world_offset( x, y, z, (float)e.h(), (float)e.p(), (float)e.r() );
	}
	else
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::remote_pawn_rotation_update: SbmPawn '" << id << "' is NULL, cannotsbm set_world_offset";
		LOG(strstr.str().c_str());
		return not_found_error( "SbmPawn is NULL" );
	}

	return no_error();
}

void handle_wsp_error( std::string id, std::string attribute_name, int error, std::string reason, void* data ) {

	LOG( "error getting id: %s attribute_name: %s. error_code: %d reason: %s\n", id.c_str(), attribute_name.c_str(), error, reason.c_str() );
}
#endif

/////////////////////////////////////////////////////////////
//  SbmPawn Constants
const char* SbmPawn::WORLD_OFFSET_JOINT_NAME = "world_offset";
SkChannelArray SbmPawn::WORLD_OFFSET_CHANNELS_P;


SbmPawn::SbmPawn() : DObject()
{
	SbmPawn::initData();

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::string validName = mcu.getValidName("object");
	setName(validName);	
}

// Constructor
SbmPawn::SbmPawn( const char * name ) : DObject(),
scene_p( NULL ),
#ifdef __ANDROID__ // don't use the GPU version in android
dMesh_p( NULL) ),
#else
dMesh_p( NULL ),
#endif
ct_tree_p( MeControllerTreeRoot::create() ),
world_offset_writer_p( NULL ),
wo_cache_timestamp( -std::numeric_limits<float>::max() )
{
	setName( name );
	//_skeleton->ref();
	ct_tree_p->ref();
	ct_tree_p->setPawn(this);

	_skeleton = new SmartBody::SBSkeleton();
	_skeleton->ref();

	SbmPawn::initData();

	this->createBoolAttribute("physics", false, true, "Basic", 300, false, false, "is the pawn physics enabled");
}

void SbmPawn::initData()
{
	bonebusCharacter = NULL;
	_skeleton = new SmartBody::SBSkeleton();
	_skeleton->ref();
	ct_tree_p = MeControllerTreeRoot::create();
	world_offset_writer_p = new MeCtChannelWriter();
	std::string controllerName = this->getName();
	controllerName += "'s world offset writer";
	world_offset_writer_p->setName( controllerName.c_str() );
	wo_cache_timestamp = -std::numeric_limits<float>::max(); 
	//skeleton_p->ref();
	ct_tree_p->ref();
	colObj_p = NULL;
	phyObj_p = NULL;
	steeringSpaceObj_p = NULL;
	steeringSpaceObjSize.x = 20.0f;
	steeringSpaceObjSize.y = 20.0f;
	steeringSpaceObjSize.z = 20.0f;
	// world_offset_writer_p, applies external inputs to the skeleton,
	//   and therefore needs to evaluate before other controllers
	world_offset_writer_p->ref();
	ct_tree_p->add_controller( world_offset_writer_p );

}

SkSkeleton* SbmPawn::getSkeleton() const
{
	return _skeleton;
}

void SbmPawn::setSkeleton(SkSkeleton* sk)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	if (_skeleton)
	{
		ct_tree_p->remove_skeleton( _skeleton->name() );
		_skeleton->unref();
	}
	_skeleton = sk;
	_skeleton->ref();
	ct_tree_p->add_skeleton( _skeleton->name(), _skeleton );

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterChanged( getName() );
	}
	//scene_p->init(_skeleton);
	//int err = mcu.add_scene(scene_p);
	dMesh_p->skeleton = _skeleton;
}

int SbmPawn::init( SkSkeleton* new_skeleton_p ) {
	if( _skeleton ) {
		ct_tree_p->remove_skeleton( _skeleton->name() );
		_skeleton->unref();
	}
	_skeleton = new_skeleton_p;
	if( _skeleton ) {
		_skeleton->ref();
		if( init_skeleton()!=CMD_SUCCESS ) {
			return CMD_FAILURE; 
		}
		ct_tree_p->add_skeleton( _skeleton->name(), _skeleton );
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if (mcu.sbm_character_listener)
		{
			mcu.sbm_character_listener->OnCharacterChanged(getName());
		}
	}

	// 	if (colObj_p)
	// 	{
	// 		initPhysicsObj();
	// 	}

	// Name the controllers
	string ct_name( getName() );
	ct_name += "'s world_offset writer";
	world_offset_writer_p->setName( ct_name.c_str() );

	return CMD_SUCCESS;
}

int SbmPawn::setup() {
	// Verifiy the joint name is not already in use.
	if( _skeleton->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME ) ) {
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::init_skeleton_offset: Skeleton already contains joint \"" << SbmPawn::WORLD_OFFSET_JOINT_NAME << "\".";
		LOG(strstr.str().c_str());
		return( CMD_FAILURE ); 
	}

	SkJoint* world_offset_joint = _skeleton->insert_new_root_joint( SkJoint::TypeQuat );
	world_offset_joint->name( SbmPawn::WORLD_OFFSET_JOINT_NAME );
	world_offset_joint->extName( SbmPawn::WORLD_OFFSET_JOINT_NAME );
	// Make sure the world_offset accepts new pos and quat values
	SkJointPos* world_offset_pos = world_offset_joint->pos();
	world_offset_pos->limits( SkVecLimits::X, false );
	world_offset_pos->limits( SkVecLimits::Y, false );
	world_offset_pos->limits( SkVecLimits::Z, false );
	world_offset_joint->quat()->activate();
	_skeleton->compress();

	if( WORLD_OFFSET_CHANNELS_P.size()==0 ) {
		std::string world_offset_joint_name( WORLD_OFFSET_JOINT_NAME );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::XPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::YPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::ZPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::Quat );
	}
	world_offset_writer_p->init( this, WORLD_OFFSET_CHANNELS_P, true );

	wo_cache.x = 0;
	wo_cache.y = 0;
	wo_cache.z = 0;
	wo_cache.h = 0;
	wo_cache.p = 0;
	wo_cache.r = 0;

	if ( mcuCBHandle::singleton().sbm_character_listener )
	{
		mcuCBHandle::singleton().sbm_character_listener->OnCharacterUpdate( getName(), getClassType() );
	}
	return( CMD_SUCCESS ); 
}

int SbmPawn::init_skeleton() {
	// Verifiy the joint name is not already in use.
	if( _skeleton->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME ) ) {
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::init_skeleton_offset: Skeleton already contains joint \"" << SbmPawn::WORLD_OFFSET_JOINT_NAME << "\".";
		LOG(strstr.str().c_str());
		return( CMD_FAILURE ); 
	}

	SkJoint* world_offset_joint = _skeleton->insert_new_root_joint( SkJoint::TypeQuat );
	world_offset_joint->name( SbmPawn::WORLD_OFFSET_JOINT_NAME );
	world_offset_joint->extName( SbmPawn::WORLD_OFFSET_JOINT_NAME );
	// Make sure the world_offset accepts new pos and quat values
	SkJointPos* world_offset_pos = world_offset_joint->pos();
	world_offset_pos->limits( SkVecLimits::X, false );
	world_offset_pos->limits( SkVecLimits::Y, false );
	world_offset_pos->limits( SkVecLimits::Z, false );
	world_offset_joint->quat()->activate();
	_skeleton->compress();

	init_world_offset_channels();
	world_offset_writer_p->init( this, WORLD_OFFSET_CHANNELS_P, true );

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
	if (!_skeleton)
		return;

	SkChannelArray& channels = _skeleton->channels();
	MeFrameData& frameData = ct_tree_p->getLastFrame();
	SrBuffer<float>& sr_fbuff = frameData.buffer();
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
		std::string world_offset_joint_name = WORLD_OFFSET_JOINT_NAME;
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::XPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::YPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::ZPos );
		WORLD_OFFSET_CHANNELS_P.add( world_offset_joint_name, SkChannel::Quat );
	}
}


bool SbmPawn::is_initialized() {
	return _skeleton != NULL;
}

int SbmPawn::prune_controller_tree() {
	// Unimplemented...
	//  TODO: walk the controller tree for excessive world offset raw writers
	return CMD_SUCCESS;
}

void SbmPawn::remove_from_scene() {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if ( scene_p  )
		mcu.remove_scene( scene_p );
	if ( dMesh_p)
	{
		for (size_t i = 0; i < dMesh_p->dMeshDynamic_p.size(); i++)
		{
			mcu.root_group_p->remove( dMesh_p->dMeshDynamic_p[i] );
		}
	}
	mcu.removePawn( getName() );
	// remove the connected steering object for steering space
	if (steeringSpaceObj_p)
	{
		if (mcu.steerEngine.isInitialized())
		{
			if (mcu.steerEngine._engine)
			{
				mcu.steerEngine._engine->removeObstacle(steeringSpaceObj_p);
				mcu.steerEngine._engine->getSpatialDatabase()->removeObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
			}
		}
		delete steeringSpaceObj_p;
		steeringSpaceObj_p = NULL;
	}
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

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if ( mcu.sbm_character_listener )
	{
		mcu.sbm_character_listener->OnCharacterDelete( getName() );
	}

	if ( bonebusCharacter )
	{
		
		if (mcu.sendPawnUpdates)
			mcu.bonebus.DeleteCharacter( bonebusCharacter );
		bonebusCharacter = NULL;
	}

	if ( world_offset_writer_p )
		world_offset_writer_p->unref();

	ct_tree_p->clear();  // Because controllers within reference back to tree root context

	if (scene_p)
		scene_p->unref();
	if( _skeleton )
		_skeleton->unref();
	ct_tree_p->unref();

	if (colObj_p)
		delete colObj_p;
	if (phyObj_p)
		delete phyObj_p;
	if (steeringSpaceObj_p)
	{
		if (mcuCBHandle::singleton().steerEngine.isInitialized())
		{
			if (mcuCBHandle::singleton().steerEngine._engine)
			{
				mcuCBHandle::singleton().steerEngine._engine->removeObstacle(steeringSpaceObj_p);
				mcuCBHandle::singleton().steerEngine._engine->getSpatialDatabase()->removeObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
			}
		}
		delete steeringSpaceObj_p;
	}

	if (dMesh_p)
	{
		delete dMesh_p;
	}
}



const SkJoint* SbmPawn::get_joint( const char* joint_name ) const {
	return _skeleton->search_joint( joint_name );
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

void SbmPawn::setWorldOffset( const SrMat& newWorld )
{	
	SrQuat quat = SrQuat(newWorld);
	gwiz::quat_t q = gwiz::quat_t(quat.w,quat.x,quat.y,quat.z);
	gwiz::euler_t e = gwiz::euler_t(q);	
	SrVec tran = newWorld.get_translation();
	set_world_offset(tran[0],tran[1],tran[2],(float)e.h(),(float)e.p(),(float)e.r());		
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

	gwiz::quat_t q = gwiz::euler_t(pitch,yaw,roll);
	float data[7] = { x, y, z, (float)q.w(), (float)q.x(), (float)q.y(), (float)q.z() };
	world_offset_writer_p->set_data( data );
	return;

	SkJoint* woj = _skeleton->search_joint( WORLD_OFFSET_JOINT_NAME );
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
			gwiz::quat_t q = gwiz::euler_t(pitch,yaw,roll);
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
		//std::stringstream strstr;
		//strstr << "ERROR: SbmPawn::wo_cache_update(..): \"" << getName() << "\" does not have a " << WORLD_OFFSET_JOINT_NAME << " joint.";
		//LOG(strstr.str().c_str());
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
	gwiz::euler_t euler( gwiz::quat_t( quat.w, quat.x, quat.y, quat.z ) );
	// Marcus's mappings
	float p = (float)euler.x();
	float h = (float)euler.y();
	float r = (float)euler.z();
	this->wo_cache.p = p;
	this->wo_cache.h = h;
	this->wo_cache.r = r;
}

int SbmPawn::parse_pawn_command( std::string cmd, srArgBuffer& args, mcuCBHandle *mcu_p)
{
	if (cmd == "remove")
	{	
		remove_from_scene();
		return CMD_SUCCESS;
	}
	else if (cmd == "prune")
	{
		int result = prune_controller_tree();
		if( result != CMD_SUCCESS )
		{
			LOG("ERROR: Failed to prune pawn \"%s\"", getName().c_str());
			return CMD_FAILURE;
		}
		else 
		{
			return CMD_SUCCESS;
		}
	}
	else if (cmd == "setshape")
	{
		std::string geom_str = "box", color_str = "red", file_str = "", type_str = "";
		bool setRec = false;
		bool has_geom = false;
		SrVec size = SrVec(1.f,1.f,1.f);		
		while( args.calc_num_tokens() > 0 )
		{
			string option = args.read_token();
			// TODO: Make the following option case insensitive
			if( option=="geom" ) {
				geom_str = args.read_token();
				has_geom = true;
			} else if( option=="size" ) {
				//size_str = args.read_token();
				float uniformSize = args.read_float();
				for (int i=0;i<3;i++)
					size[i] = uniformSize;//args.read_float();
				has_geom = true;

			} else if (option=="file" ) {
				file_str = args.read_token();
				has_geom = true;			
			} else if( option=="color" ) {
				color_str = args.read_token();
				has_geom = true;
			} else if( option=="type" ) {
				type_str = args.read_token();
				has_geom = true;
			} else if( option=="rec" ) {
				setRec = true;
				size[0] = steeringSpaceObjSize.x = args.read_float();
				size[1] = steeringSpaceObjSize.y = args.read_float();
				size[2] = steeringSpaceObjSize.z = args.read_float();

				has_geom = true;
			} else {
				std::stringstream strstr;
				strstr << "WARNING: Unrecognized pawn setshape option \"" << option << "\"." << endl;
				LOG(strstr.str().c_str());
			}
		}	

		if (has_geom)
		{				
			initGeomObj(geom_str.c_str(),size,color_str.c_str(),file_str.c_str());
			// init steering space
			if (!setRec)
				steeringSpaceObjSize = size;//SrVec(size, size, size);
			if (type_str == "steering")
				initSteeringSpaceObject();
			return CMD_SUCCESS;
		}
		else
		{
			LOG("Pawn %s, fail to setshape. Incorrect parameters.", getName().c_str());
			return CMD_FAILURE;
		}
	}
	else if (cmd == "physics")
	{
		string option = args.read_token();

		bool turnOn = false;
		if (option == "on" || option == "ON")
			turnOn = true;			
		else if (option == "off" || option == "OFF")
			turnOn = false;			
		else
			return CMD_FAILURE;

		setPhysicsSim(turnOn);
		return CMD_SUCCESS;
	}
	else if (cmd == "collision")
	{	
		string option = args.read_token();
		bool turnOn = false;
		if (option == "on" || option == "ON")
			turnOn = true;			
		else if (option == "off" || option == "OFF")
			turnOn = false;			
		else
			return CMD_FAILURE;

		setCollision(turnOn);			
		return CMD_SUCCESS;
	}
	else
	{
		return CMD_FAILURE;
	}
}


int SbmPawn::pawn_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string pawn_name = args.read_token();
	if( pawn_name.length()==0 )
	{
		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	string pawn_cmd = args.read_token();
	if( pawn_cmd.length()==0 )
	{
		LOG("ERROR: Expected pawn command.");
		return CMD_FAILURE;
	}

	if (pawn_cmd == "init")
	{
		// pawn <name> init [loc <x> <y> <z>] [geom <shape name>] [color <color hex>] [size <size>]
		SbmPawn* pawn_p = mcu_p->getPawn(pawn_name);
		if( pawn_p != NULL ) {
			LOG("ERROR: Pawn \"%s\" already exists.", pawn_name.c_str());
			return CMD_FAILURE;
		}
		// Options
		float loc[3] = { 0, 0, 0 };

		bool has_geom = false;
		std::string geom_str = "box";
		std::string file_str = "";
		std::string size_str = "";
		std::string color_str = "red";
		std::string type_str = "";
		SrVec size = SrVec(1.f,1.f,1.f);
		bool setRec = false;
		SrVec rec;
		std::string defaultColor = "red";
		while( args.calc_num_tokens() > 0 ) {
			string option = args.read_token();
			// TODO: Make the following option case insensitive
			if( option == "loc" ) {
				args.read_float_vect( loc, 3 );
			} else if( option=="geom" ) {
				geom_str = args.read_token();
				has_geom = true;
			} else if (option == "file")
			{
				file_str = args.read_token();
				has_geom = true;	
			} else if( option=="type" ) {
				type_str = args.read_token();
				has_geom = true;
			} else if( option=="size" ) {
				size_str = args.read_token();
				has_geom = true;
			} else if( option=="color" ) {
				color_str = args.read_token();
				has_geom = true;
			} else if( option=="rec" ) {
				setRec = true;
				size.x = rec.x = args.read_float();
				size.y = rec.y = args.read_float();
				size.z = rec.z = args.read_float();
				has_geom = true;
			} else {
				std::stringstream strstr;
				strstr << "WARNING: Unrecognized pawn init option \"" << option << "\"." << endl;
				LOG(strstr.str().c_str());
			}
		}		

		pawn_p = new SmartBody::SBPawn( pawn_name.c_str() );
		pawn_p->setClassType("pawn");
		SkSkeleton* skeleton = new SmartBody::SBSkeleton();
		skeleton->ref();
		string skel_name = pawn_name+"-skel";
		skeleton->name( skel_name.c_str() );
		// Init channels
		skeleton->make_active_channels();
		if (mcu_p->sbm_character_listener)
		{
			mcu_p->sbm_character_listener->OnCharacterChanged(pawn_name);
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

		// setting up geometry and physics 
		if( has_geom && !geom_str.empty() ) {
			//LOG("WARNING: SbmPawn geometry not implemented.  Ignoring options.");			
			if (!size_str.empty())
			{
				float uniformSize = (float)atof(size_str.c_str());
				for (int i=0;i<3;i++)
					size[i] = uniformSize;
			}			
			pawn_p->initGeomObj(geom_str.c_str(),size,color_str.c_str(),file_str.c_str());
		}
		if (pawn_p->colObj_p)
		{
			if (geom_str == "box")
			{
				pawn_p->steeringSpaceObjSize = rec;
				if (!setRec)
				{
					float size = (float)atof(size_str.c_str());
					pawn_p->steeringSpaceObjSize = SrVec(size, size, size);
				}
				if (type_str == "steering")
					pawn_p->initSteeringSpaceObject();
			}
		}
		// 		else // default null geom object
		// 		{
		// 			SbmGeomObject* colObj = new SbmGeomNullObject();
		// 			pawn_p->colObj_p = colObj;
		// 		}

		bool ok = mcu_p->addPawn( pawn_p );
		if( !ok )	{
			std::stringstream strstr;
			strstr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		if (pawn_p->colObj_p)
		{
			pawn_p->setWorldOffset(pawn_p->colObj_p->getWorldState().gmat());
		}
		// [BMLR] Send notification to the renderer that a pawn was created.
		// NOTE: This is sent both for characters AND pawns
		mcu_p->bonebus.SendCreatePawn( pawn_name.c_str(), loc[ 0 ], loc[ 1 ], loc[ 2 ] );
		float x,y,z,h,p,r;
		pawn_p->get_world_offset(x,y,z,h,p,r);
		//printf("h = %f, p = %f, r = %f\n",h,p,r);	
		pawn_p->set_world_offset(loc[0],loc[1],loc[2],h,p,r);	
		pawn_p->wo_cache_update();

		if (mcu_p->sendPawnUpdates)
			pawn_p->bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( pawn_name.c_str(), pawn_p->getClassType().c_str(), false );

		if ( mcuCBHandle::singleton().sbm_character_listener )
		{
			mcuCBHandle::singleton().sbm_character_listener->OnCharacterCreate( pawn_name, pawn_p->getClassType().c_str() );
		}

		return CMD_SUCCESS;
	}

	bool all_pawns = false;
	SbmPawn* pawn_p = NULL;
	if( pawn_name== "*" )
	{
		std::vector<std::string> pawns;
		for (std::map<std::string, SbmPawn*>::iterator iter = mcu_p->getPawnMap().begin();
			iter != mcu_p->getPawnMap().end();
			iter++)
		{
			pawns.push_back((*iter).second->getName());
		}
		for (std::vector<std::string>::iterator citer = pawns.begin();
			citer != pawns.end();
			citer++)
		{
			srArgBuffer copy_args( args.peek_string() );
			pawn_p = mcu_p->getPawn( *citer );
			int err = pawn_p->parse_pawn_command( pawn_cmd, copy_args, mcu_p);
			if( err != CMD_SUCCESS )
				return( err );
		}
		return CMD_SUCCESS;
	} 
	else
	{
		pawn_p = mcu_p->getPawn( pawn_name.c_str() );
		if( pawn_p ) 
		{
			int ret = pawn_p->parse_pawn_command( pawn_cmd, args, mcu_p );
			return( ret );
		}
		else
		{
			LOG("No pawn named '%s' exists.", pawn_name.c_str());
			return CMD_FAILURE;
		}
	}
}

int SbmPawn::remove_from_scene( const char* pawn_name ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if( strcmp( pawn_name, "*" )==0 ) {
		for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
			iter != mcu.getPawnMap().end();
			iter++)
		{
			SbmPawn* pawn = (*iter).second;
			pawn->remove_from_scene();
			delete pawn;
		}
		return CMD_SUCCESS;
	} else {
		SbmPawn* pawn_p = mcu.getPawn( pawn_name );

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

	SbmPawn* pawn = mcu_p->getPawn( pawn_id );
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
			SkSkeleton* skeleton = pawn->_skeleton;
			std::vector<SkJoint*>& joints = skeleton->get_joint_array();
			for (size_t j = 0; j < joints.size(); j++)
			{
				LOG("%s : %f", joints[j]->name().c_str(), joints[j]->mass());

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
		LOG("ERROR: SbmPawn::set_cmd_func(..): Unknown attribute \"%s\".", attribute.c_str() );
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

	SbmPawn* pawn = mcu_p->getPawn( pawn_id );
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

int SbmPawn::print_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p )
{
	std::stringstream strstr;
	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: print pawn <character id> world_offset
		//  Print out the current state of the world_offset joint
		strstr << "pawn " << pawn->getName() << " world_offset:\t";
		const SkJoint* joint = pawn->get_world_offset_joint();
		LOG(strstr.str().c_str());
		if( joint==NULL ) {
			LOG("No world_offset joint.");
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
			strstr.clear();
			strstr << "pawn " << pawn->getName() << " joint "<<joint_name<<":\t";
			const SkJoint* joint = pawn->get_joint( joint_name.c_str() );
			LOG(strstr.str().c_str());
			if( joint==NULL ) {
				LOG("No joint \"%s\".", joint_name.c_str() );
			} else {
				print_joint( joint );
			}

			joint_name = args.read_token();
		} while( joint_name.length()>0 );

		return CMD_SUCCESS;
	} else {
		LOG("ERROR: SbmPawn::print_attribute(..): Unknown attribute \"%s\".", attribute.c_str() );
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

	pawn_p = mcu_p->getPawn( pawn_and_attribute );

	if( pawn_p != NULL ) {
		LOG("ERROR: Pawn \"%s\" already exists.", pawn_and_attribute.c_str() );
		return CMD_FAILURE;
	}

	pawn_p = new SmartBody::SBPawn( pawn_and_attribute.c_str() );

	SkSkeleton* skeleton = new SmartBody::SBSkeleton();
	skeleton->ref();
	std::string skel_name = pawn_and_attribute+"-skel";
	skeleton->name( skel_name.c_str() );
	// Init channels
	skeleton->make_active_channels();	

	if (mcu_p->sbm_character_listener)
	{
		mcu_p->sbm_character_listener->OnCharacterChanged(pawn_and_attribute);
	}

	int err = pawn_p->init( skeleton );

	if( err != CMD_SUCCESS ) {
		LOG("ERROR: Unable to initialize SbmPawn \"%s\".", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->addPawn( pawn_p );

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}


#if USE_WSP
	mcu_p->theWSP->subscribe_vector_3d_interval( pawn_and_attribute, "position", interval, handle_wsp_error, remote_pawn_position_update, mcu_p );
	mcu_p->theWSP->subscribe_vector_4d_interval( pawn_and_attribute, "rotation", interval, handle_wsp_error, remote_pawn_rotation_update, mcu_p );
#endif

	return( CMD_SUCCESS );
}

int SbmPawn::remove_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {

	std::string pawn_name = args.read_token();

	if( pawn_name.length()==0 ) {

		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->getPawn( pawn_name );

	if( pawn_p != NULL ) {

#if USE_WSP
		mcu_p->theWSP->unsubscribe( pawn_name, "position", 1 );
#endif

		pawn_p->remove_from_scene();

		return CMD_SUCCESS;
	} else {
		LOG("ERROR: Pawn \"%s\" not found.", pawn_name.c_str() );
		return CMD_FAILURE;
	}
}

#if USE_WSP
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

	SkJoint * joint = pawn_p->_skeleton->search_joint( joint_name.c_str() );
	if ( joint != NULL )
	{
		joint->update_gmat();
		const SrMat & sr_m = joint->gmat();

		gwiz::matrix_t m;
		for ( int i=0; i<4; i++ )
		{
			for ( int j=0; j<4; j++ )
			{
				m.set( i, j, sr_m.get( i, j ) );
			}
		}

		gwiz::vector_t pos = m.translation( gwiz::COMP_M_TR );

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

	SkJoint * joint = pawn_p->_skeleton->search_joint( joint_name.c_str() );
	if ( joint != NULL )
	{
		joint->update_gmat();
		const SrMat & sr_m = joint->gmat();

		gwiz::matrix_t m;
		for( int i=0; i<4; i++ )
		{
			for( int j=0; j<4; j++ )
			{
				m.set( i, j, sr_m.get( i, j ) );
			}
		}

		gwiz::quat_t quat = m.quat( gwiz::COMP_M_TR );

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
#endif

bool SbmPawn::initGeomObj( const char* geomType, SrVec size, const char* color, const char* meshName  )
{
	SbmGeomObject* colObj = NULL;
	if (strcmp(geomType,"sphere") == 0)
	{
		colObj = new SbmGeomSphere(size[0]);		
	}
	else if (strcmp(geomType,"box") == 0)
	{
		colObj = new SbmGeomBox(SrVec(size[0],size[1],size[2]));		
	}
	else if (strcmp(geomType,"capsule") == 0)
	{
		SrVec pos[2];
		float capLen = size[0]*1.5f;
		pos[0] = SrVec(0,-capLen*0.5f,0);
		pos[1] = SrVec(0,capLen*0.5f,0);
		//pos[0] = SrVec(0,capLen,0);
		//pos[1] = SrVec(0,capLen*2.f,0);		
		//SbmColObject* colObj = new SbmColCapsule(size*1.5f,size*0.5f);
		colObj = new SbmGeomCapsule(pos[0],pos[1],size[0]*0.5f);		
	}	
	else if (strcmp(geomType,"mesh") == 0)
	{
		SrModel* model = NULL;
		bool hasObjModel = false;
		if (meshName)
		{
			model = new SrModel();
			hasObjModel = model->import_obj(meshName);
		}
		if (model && hasObjModel)
		{
			colObj = new SbmGeomTriMesh(model);
		}	
	}
	else if (strcmp(geomType,"null") == 0)
	{
		if (phyObj_p) // remove physics
		{
			removePhysicsObj();
		}

		if (colObj_p) // remove geometry
		{
			delete colObj_p;
			colObj_p = NULL;
		}
		return true;
	}
	else
	{
		LOG("Can not create pawn geometry. Undefined geom type : %s\n",geomType);
		return false;
	}

	if (phyObj_p) // remove physics
	{
		removePhysicsObj();
	}

	if (colObj_p) // remove geometry
	{
		delete colObj_p;
		colObj_p = NULL;
	}
	if (colObj)
		colObj->color = color;
	colObj_p = colObj;
	updateToColObject();
	initPhysicsObj();
	return true;
}

void SbmPawn::initPhysicsObj()
{	
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	if (!phySim)
		return;
	//printf("init physics obj\n");
	phyObj_p = phySim->createPhyObj();
	phyObj_p->initGeometry(colObj_p,1.f);	
	phySim->addPhysicsObj(phyObj_p);
}

void SbmPawn::removePhysicsObj()
{
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	if (!phySim || !phyObj_p)
		return;
	phySim->removePhysicsObj(phyObj_p);
	delete phyObj_p;
	phyObj_p = NULL;
}

void SbmPawn::updateFromColObject()
{
	if (colObj_p)
	{
		//setWorldOffset(colObj_p->getWorldState().gmat());
		setWorldOffset(colObj_p->getWorldState().gmat());
	}
}

void SbmPawn::updateToColObject()
{
	if (colObj_p)
	{
		SRT newWorldState; 
		newWorldState.gmat(get_world_offset_joint()->gmat());
		//colObj_p->getWorldState().gmat();
		colObj_p->setWorldState(newWorldState);
		//colObj_p->updateGlobalTransform(get_world_offset_joint()->gmat());
		if (phyObj_p)
		{
			phyObj_p->updateSimObj();			
		}
	}
}

void SbmPawn::updateToSteeringSpaceObject()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.steerEngine.isInitialized())	return;
	if (!mcu.steerEngine._engine)	return;
	if (steeringSpaceObj_p)
		initSteeringSpaceObject();
}

void SbmPawn::initSteeringSpaceObject()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.steerEngine.isInitialized())	return;
	if (!mcu.steerEngine._engine)	return;

	float x, y, z, h, p, r;
	this->get_world_offset(x, y, z, h, p, r);	
	float xmin = (x - steeringSpaceObjSize.x) / 100.0f;
	float xmax = (x + steeringSpaceObjSize.x) / 100.0f;
	float ymin = (y - steeringSpaceObjSize.y) / 100.0f;
	float ymax = (y + steeringSpaceObjSize.y) / 100.0f;
	float zmin = (z - steeringSpaceObjSize.z) / 100.0f;
	float zmax = (z + steeringSpaceObjSize.z) / 100.0f;

	if (steeringSpaceObj_p)
	{
		const Util::AxisAlignedBox& box = steeringSpaceObj_p->getBounds();
		if (fabs(box.xmax - xmax) < .0001 ||
			fabs(box.xmin - xmin) < .0001 ||
			fabs(box.ymax - ymax) < .0001 ||
			fabs(box.ymin - ymin) < .0001 ||
			fabs(box.zmax - zmax) < .0001 ||
			fabs(box.zmin - zmin) < .0001)
		{
			mcu.steerEngine._engine->getSpatialDatabase()->removeObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
			Util::AxisAlignedBox& mutableBox = const_cast<Util::AxisAlignedBox&>(box);
			mutableBox.xmax = xmax;
			mutableBox.xmin = xmin;
			mutableBox.ymax = ymax;
			mutableBox.ymin = ymin;
			mutableBox.zmax = zmax;
			mutableBox.zmin = zmin;
			mcu.steerEngine._engine->getSpatialDatabase()->addObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
		}
	}
	else
	{
		steeringSpaceObj_p = new SteerLib::BoxObstacle(xmin, xmax, ymin, ymax, zmin, zmax);
		mcu.steerEngine._engine->addObstacle(steeringSpaceObj_p);
		mcu.steerEngine._engine->getSpatialDatabase()->addObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());	
	}
}

void SbmPawn::clearSteeringGoals()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.steerEngine.isInitialized())	return;
	if (!mcu.steerEngine._engine)	return;

	SbmCharacter* character = dynamic_cast<SbmCharacter*> (this);
	if (!character)	return;
	if (!character->steeringAgent)	return;
	character->steeringAgent->getAgent()->clearGoals();
}

void SbmPawn::setPhysicsSim( bool enable )
{
	if (!phyObj_p)
		return;

	phyObj_p->setPhysicsSim(enable);	
}

bool SbmPawn::hasPhysicsSim()
{
	if (!phyObj_p)
		return false;

	return phyObj_p->hasPhysicsSim();
}

void SbmPawn::setCollision( bool enable )
{
	if (!phyObj_p)
		return;

	phyObj_p->setCollisionSim(enable);
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

std::string SbmPawn::getClassType()
{
	return _classType;
}

void SbmPawn::setClassType(std::string classType)
{
	_classType = classType;
}


void SbmPawn::notify(DSubject* subject)
{
	DAttribute* attribute = dynamic_cast<DAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "physics")
		{
			BoolAttribute* physicsAttr = dynamic_cast<BoolAttribute*>(attribute);
			setPhysicsSim(physicsAttr->getValue());
		}
	}
}

