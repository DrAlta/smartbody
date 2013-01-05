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
#include <sb/SBScene.h>

#ifndef __native_client__
#include <sb/SBPythonClass.h>
#endif

#include <sb/SBBoneBusManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBCollisionManager.h>
#include <controllers/me_controller_tree_root.hpp>
#include <controllers/me_ct_channel_writer.hpp>
#include <controllers/me_ct_curve_writer.hpp>
#include "sbm_pawn.hpp"

#include "sbm_deformable_mesh.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#if defined(__ANDROID__) || defined(SBM_IPHONE) || defined(__native_client__)
#include <sbm/sbm_deformable_mesh.h>
#else
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#endif

#include <string.h>
#include <iostream>

#ifndef __native_client__

// added by AShapiro 6/30/11 - not sure why the USE_WSP variable this is not being picked up in mcontrol_util.h
#define USE_WSP 1
#if USE_WSP
#include "wsp.h"
#endif

#endif

#include "me_utilities.hpp"
#include "sr/sr_model.h"
#include "sr/sr_euler.h"
#include <sb/SBSkeleton.h>

using namespace std;

#if USE_WSP
using namespace WSP;
#endif

// Predeclare private functions defined below
inline bool parse_float_or_error( float& var, const char* str, const string& var_name );

/////////////////////////////////////////////////////////////
//  WSP Callbacks


/////////////////////////////////////////////////////////////
//  SbmPawn Constants
const char* SbmPawn::WORLD_OFFSET_JOINT_NAME = "world_offset";
SkChannelArray SbmPawn::WORLD_OFFSET_CHANNELS_P;


SbmPawn::SbmPawn() : SBObject(),
dMesh_p( NULL ),
dMeshInstance_p(NULL)
{
	SbmPawn::initData();

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::string validName = mcu.getValidName("object");
	setName(validName);	
	SbmPawn::initData();
}

// Constructor
SbmPawn::SbmPawn( const char * name ) : SmartBody::SBObject(),
scene_p( NULL ),
#ifdef __ANDROID__ // don't use the GPU version in android
dMesh_p( NULL) ,
dMeshInstance_p(NULL),
#else
dMesh_p( NULL ),
dMeshInstance_p(NULL),
#endif
ct_tree_p( MeControllerTreeRoot::create() ),
world_offset_writer_p( NULL ),
wo_cache_timestamp( -std::numeric_limits<float>::max() )
{
	SmartBody::SBObject::setName( name );
	//_skeleton->ref();
	ct_tree_p->ref();
	ct_tree_p->setPawn(this);

	_skeleton = new SmartBody::SBSkeleton();
	_skeleton->ref();

	SbmPawn::initData();
}

void SbmPawn::initData()
{
	bonebusCharacter = NULL;
	if (_skeleton)
		_skeleton->unref();
	_skeleton = new SmartBody::SBSkeleton();
	_skeleton->ref();
	ct_tree_p = MeControllerTreeRoot::create();
	world_offset_writer_p = new MeCtChannelWriter();
	std::string controllerName = this->getName();
	controllerName += "_worldOffsetWriter";
	world_offset_writer_p->setName( controllerName.c_str() );
	wo_cache_timestamp = -std::numeric_limits<float>::max(); 
	//skeleton_p->ref();
	ct_tree_p->ref();
	//colObj_p = NULL;
	//phyObj_p = NULL;
	steeringSpaceObj_p = NULL;
	steeringSpaceObjSize.x = 20.0f;
	steeringSpaceObjSize.y = 20.0f;
	steeringSpaceObjSize.z = 20.0f;
	// world_offset_writer_p, applies external inputs to the skeleton,
	//   and therefore needs to evaluate before other controllers
	world_offset_writer_p->ref();
	ct_tree_p->add_controller( world_offset_writer_p );
	
	collisionObjName = this->getName();
	collisionObjName += "_BV"; // bounding volume

	SmartBody::SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollisionManager();	
	SbmGeomObject* geomObj = colManager->createCollisionObject(collisionObjName,"null",SrVec());
	geomObj->attachToObj(this);

	wo_cache.x = 0;
	wo_cache.y = 0;
	wo_cache.z = 0;
	wo_cache.h = 0;
	wo_cache.p = 0;
	wo_cache.r = 0;
}

SkSkeleton* SbmPawn::getSkeleton() const
{
	return _skeleton;
}

void SbmPawn::setSkeleton(SkSkeleton* sk)
{
	if (!sk)
	{
		LOG("Cannot set a null skeleton on object %s", getName().c_str());
		return;
	}
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	if (_skeleton)
	{		
		ct_tree_p->remove_skeleton( _skeleton->getName() );
		_skeleton->unref();
	}
	_skeleton = sk;
	_skeleton->ref();	
	ct_tree_p->add_skeleton( _skeleton->getName(), _skeleton );	
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if ( scene->getCharacterListener() )
	{		
		scene->getCharacterListener()->OnCharacterChanged( getName() );
	}
	//scene_p->init(_skeleton);
	//int err = mcu.add_scene(scene_p);	
	if (dMesh_p)
		dMesh_p->skeleton = _skeleton;	
	if (dMeshInstance_p)
		dMeshInstance_p->setSkeleton(_skeleton);
		
	float height = _skeleton->getCurrentHeight();	
	setHeight(height);
	_skeleton->ref();
}

int SbmPawn::init( SkSkeleton* new_skeleton_p ) {
	if( _skeleton ) {
		ct_tree_p->remove_skeleton( _skeleton->getName() );
		_skeleton->unref();
	}
	_skeleton = new_skeleton_p;
	if( _skeleton ) {
		_skeleton->ref();
		if( init_skeleton()!=CMD_SUCCESS ) {
			return CMD_FAILURE; 
		}
		ct_tree_p->add_skeleton( _skeleton->getName(), _skeleton );
		SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if (scene->getCharacterListener())
		{
			scene->getCharacterListener()->OnCharacterChanged(getName());
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
	_skeleton->make_active_channels();

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

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if ( scene->getCharacterListener() )
	{
		scene->getCharacterListener()->OnCharacterUpdate( getName(), getClassType() );
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

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if ( scene->getCharacterListener() )
	{
		scene->getCharacterListener()->OnCharacterDelete( getName() );
	}

	if ( bonebusCharacter )
	{
		
		if (mcu.sendPawnUpdates)
			mcu._scene->getBoneBusManager()->getBoneBus().DeleteCharacter( bonebusCharacter );
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
	
	if (steeringSpaceObj_p)
	{
		if (mcuCBHandle::singleton()._scene->getSteerManager()->getEngineDriver()->isInitialized())
		{
			if (mcuCBHandle::singleton()._scene->getSteerManager()->getEngineDriver()->_engine)
			{
				mcuCBHandle::singleton()._scene->getSteerManager()->getEngineDriver()->_engine->removeObstacle(steeringSpaceObj_p);
				mcuCBHandle::singleton()._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->removeObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
			}
		}
		delete steeringSpaceObj_p;
	}

	if (dMesh_p)
	{
		delete dMesh_p;
	}
	if (dMeshInstance_p)
	{
		delete dMeshInstance_p;
	}

	SmartBody::SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollisionManager();
	colManager->removeCollisionObject(collisionObjName);
// 	if (_collisionObject)
// 		delete _collisionObject;
}



const SkJoint* SbmPawn::get_joint( const char* joint_name ) const {
	if (_skeleton)
		return _skeleton->search_joint( joint_name );
	else
		return NULL;
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

SrMat SbmPawn::get_world_offset()
{	
	float x,y,z,h,r,p;
	get_world_offset(x,y,z,h,p,r);
	gwiz::quat_t q = gwiz::euler_t(p,h,r);	
	SrQuat pawnQ = SrQuat((float)q.w(), (float)q.x(), (float)q.y(), (float)q.z());
	SrMat gmat;
	pawnQ.get_mat(gmat);
	gmat.e41() = x;
	gmat.e42() = y;
	gmat.e43() = z;
	return gmat;
}

SbmTransform& SbmPawn::getGlobalTransform()
{	
	globalTransform.gmat(get_world_offset());
	return globalTransform;
}

void SbmPawn::setGlobalTransform( SbmTransform& newGlobalTransform )
{
	SrMat gmat = globalTransform.gmat();
	setWorldOffset(gmat);
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




int SbmPawn::remove_remote_pawn_func( srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();

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

		mcu_p->_scene->removePawn(pawn_p->getName());

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


const std::string& SbmPawn::getGeomObjectName()
{
	return collisionObjName;
}

SbmGeomObject* SbmPawn::getGeomObject()
{	
	//return _collisionObject;
	SmartBody::SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollisionManager();
	SbmGeomObject* geomObj = colManager->getCollisionObject(collisionObjName);
	return geomObj;
} 

// void SbmPawn::setGeomObject(SbmGeomObject* object)
// {	
// 	if (_collisionObject)
// 		delete _collisionObject;
// 
// 	_collisionObject = object;
// 	_collisionObject->attachToObj(this);
// }


void SbmPawn::updateToColObject()
{
	SmartBody::SBPawn* sbpawn = dynamic_cast<SmartBody::SBPawn*>(this);
	SmartBody::SBPhysicsObj* phyObj = sbpawn->getPhysicsObject();
	if (phyObj)
	{
		SRT newWorldState; 
		newWorldState.gmat(get_world_offset());		
		phyObj->setGlobalTransform(newWorldState);		
		phyObj->updatePhySim();					
	}
}

void SbmPawn::updateToSteeringSpaceObject()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu._scene->getSteerManager()->getEngineDriver()->isInitialized())	return;
	if (!mcu._scene->getSteerManager()->getEngineDriver()->_engine)	return;
	if (steeringSpaceObj_p)
		initSteeringSpaceObject();
}

void SbmPawn::initSteeringSpaceObject()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!scene->getSteerManager()->getEngineDriver()->isInitialized())	return;
	if (!scene->getSteerManager()->getEngineDriver()->_engine)	return;

	float steerScale = 1.0f / scene->getScale();

	float x, y, z, h, p, r;
	this->get_world_offset(x, y, z, h, p, r);	
	float xmin = (x - steeringSpaceObjSize.x) / steerScale;
	float xmax = (x + steeringSpaceObjSize.x) / steerScale;
	float ymin = (y - steeringSpaceObjSize.y) / steerScale;
	float ymax = (y + steeringSpaceObjSize.y) / steerScale;
	float zmin = (z - steeringSpaceObjSize.z) / steerScale;
	float zmax = (z + steeringSpaceObjSize.z) / steerScale;

	//LOG("steeringSpaceObjSize = %f %f %f, scale = %f", steeringSpaceObjSize.x, steeringSpaceObjSize.y, steeringSpaceObjSize.z, steerScale);

	if (steeringSpaceObj_p)
	{
		const Util::AxisAlignedBox& box = steeringSpaceObj_p->getBounds();
		if (fabs(box.xmax - xmax) > .0001 ||
			fabs(box.xmin - xmin) > .0001 ||
			fabs(box.ymax - ymax) > .0001 ||
			fabs(box.ymin - ymin) > .0001 ||
			fabs(box.zmax - zmax) > .0001 ||
			fabs(box.zmin - zmin) > .0001)
		{
			mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->removeObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
			Util::AxisAlignedBox& mutableBox = const_cast<Util::AxisAlignedBox&>(box);
			mutableBox.xmax = xmax;
			mutableBox.xmin = xmin;
			mutableBox.ymax = ymax;
			mutableBox.ymin = ymin;
			mutableBox.zmax = zmax;
			mutableBox.zmin = zmin;
			mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->addObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());
		}
	}
	else
	{
		steeringSpaceObj_p = new SteerLib::BoxObstacle(xmin, xmax, ymin, ymax, zmin, zmax);
		mcu._scene->getSteerManager()->getEngineDriver()->_engine->addObstacle(steeringSpaceObj_p);
		mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->addObject(steeringSpaceObj_p, steeringSpaceObj_p->getBounds());	
	}
}

// void SbmPawn::setPhysicsSim( bool enable )
// {
// 	if (!phyObj_p)
// 		return;
// 
// 	phyObj_p->enablePhysicsSim(enable);	
// }
// 
// bool SbmPawn::hasPhysicsSim()
// {
// 	if (!phyObj_p)
// 		return false;
// 
// 	return phyObj_p->hasPhysicsSim();
// }
// 
// void SbmPawn::setCollision( bool enable )
// {
// 	if (!phyObj_p)
// 		return;
// 
// 	phyObj_p->enableCollisionSim(enable);
// }

///////////////////////////////////////////////////////////////////////////
//  Private sbm_pawn functions



std::string SbmPawn::getClassType()
{
	return _classType;
}

void SbmPawn::setClassType(std::string classType)
{
	_classType = classType;
}


void SbmPawn::notify(SBSubject* subject)
{

}


