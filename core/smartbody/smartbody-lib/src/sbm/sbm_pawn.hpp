/*
 *  sbm_pawn.hpp - part of SmartBody-lib
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

#ifndef SBM_PAWN_HPP
#define SBM_PAWN_HPP
#include "vhcl.h"

#include <sk/sk_scene.h>
#include <sk/sk_skeleton.h>
#include <sbm/Physics/SbmColObject.h>
#include <sbm/SteerSuiteEngineDriver.h>

#include <me/me_controller_tree_root.hpp>
#include <me/me_ct_channel_writer.hpp>
#include <me/me_ct_curve_writer.hpp>
#include <sbm/SBObject.h>
#include <SteerLib.h>

#include "bonebus.h"

#if USE_WSP
#include "wsp.h"
#endif

#include <map>

#include "sbm_constants.h"
#include "sbm_deformable_mesh.h"
#include "sbm/Physics/SbmPhysicsSim.h"
// Declare classes used (avoid circular references)
class mcuCBHandle;
class srArgBuffer;


#define SBM_PAWN_USE_WORLD_OFFSET_WRITER	(1)
#define SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK	(0)

class SbmPawn : public SmartBody::SBObject, public SbmTransformObjInterface {
public:
	//  Public Constants
	static const char* WORLD_OFFSET_JOINT_NAME;

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
	// Typedefs
	/**
	 *  Controller clean-up callback function prototypes.
	 *
	 *  Called when a controller is determined to no longer be in use
	 *  (by controller tree pruning, behavior interruption, etc.).
	 */
	typedef void (*controller_cleanup_callback_fp)( MeController*, SbmPawn*, mcuCBHandle* );
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

private:
	//  Private Constants
	static SkChannelArray WORLD_OFFSET_CHANNELS_P;

protected:
	//  Private Data
	struct {
		float x, y, z;  // translation (x,y,z)
		float h, p, r;  // rotation (heading, pitch, roll)
	} wo_cache;         // Caches values when setting world offset, because controller may not have been evaluated and skeleton value may not reflect the last call to set_world_offset (HACK-ish)
	double wo_cache_timestamp;

	MeCtChannelWriter*  world_offset_writer_p;
	float	_height;
	//SbmGeomObject* _collisionObject;
	std::string collisionObjName;

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
	// Map of pending controller clean-up callbacks
	std::multimap<MeController*,controller_cleanup_callback_fp> ct_cleanup_funcs;
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

public:  // TODO - properly encapsulate / privatize the following
	SkSkeleton*	_skeleton;;  // MAY BE NULL!!!
	SkScene*		scene_p;	 // Skeleton Scene and Rigid Mesh		
	DeformableMesh*	dMesh_p;	 // Deformable Mesh using smooth skinning	
	DeformableMeshInstance* dMeshInstance_p;
	SteerLib::ObstacleInterface* steeringSpaceObj_p;
	SrVec			steeringSpaceObjSize;
	std::string _classType;
	
	
	//		float scale = 0.5f;
//		SteerLib::BoxObstacle* box = new SteerLib::BoxObstacle(x / 100.0f - scale, x / 100.0f + scale, y / 100.0f - scale, y / 100.0f + scale, z / 100.0f - scale, z / 100.0f + scale);
//		mcuCBHandle::singleton().steerEngine->_engine->addObstacle(box);

	// Temporarily, until there is a unified multi-skeleton controller tree
	MeControllerTreeRoot	*ct_tree_p;
	bonebus::BoneBusCharacter * bonebusCharacter;
	SbmTransform                globalTransform;

public:	
	//  Public Methods
	SbmPawn();
	SbmPawn( const char* name );
	virtual ~SbmPawn();

	SkSkeleton* getSkeleton() const;
	void setSkeleton(SkSkeleton* sk);	
	virtual int init( SkSkeleton* skeleton_p );

	const std::string& getGeomObjectName();
	SbmGeomObject* getGeomObject(); // get geometry object associated with the pawn
	//void setGeomObject(SbmGeomObject* obj);
	SbmPhysicsObj* getPhysicsObject();

 	void updateToColObject();
	void updateToSteeringSpaceObject();
	void initSteeringSpaceObject();

	bool is_initialized();

	void setHeight( float height )	{ _height = height; }
	float getHeight( void ) 		{ return _height; }
	SrBox getBoundingBox( void ) 		{ if (_skeleton) return _skeleton->getBoundingBox(); return SrBox(); }

	virtual int prune_controller_tree();  // removes unused or overwritten controllers
	
	void reset_all_channels();

	const SkJoint* get_joint( const char* joint_name ) const;
	const SkJoint* get_world_offset_joint() const
	{	return get_joint( WORLD_OFFSET_JOINT_NAME ); }

	virtual SbmTransform& getGlobalTransform();
	virtual void setGlobalTransform(SbmTransform& newGlobalTransform);

	SrMat get_world_offset();
	void get_world_offset( float& x, float& y, float& z,
		                   float& yaw, float& pitch, float& roll );

	void setWorldOffset(const SrMat& newWorld);
	void set_world_offset( float x, float y, float z,
		                   float yaw, float pitch, float roll );

	virtual std::string getClassType();
	virtual void setClassType(std::string classType);

	virtual void notify(SBSubject* subject);

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
	virtual void register_controller_cleanup( MeController* ct, controller_cleanup_callback_fp func );
	virtual void exec_controller_cleanup( MeController* ct, mcuCBHandle* mcu_p );
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

protected:
	/*!
	 *  Initializes the static variable WORLD_OFFSET_CHANNELS_P.
	 *  TODO: When is this deleted?
	 */
	void init_world_offset_channels();

	/*!
	 *   Modify skeleton, if necessary.
	 *
	 *   SbmPawn inserts world_offset joint above the existing root.
	 */
	virtual int init_skeleton();	

	/*!
	 *   Modify skeleton, if necessary.
	 *
	 *   SbmPawn inserts world_offset joint above the existing root.
	 */
	virtual int setup();

	void wo_cache_update();

	void initData();

	int parse_pawn_command( std::string cmd, srArgBuffer& args, mcuCBHandle *mcu_p);



public:
	// static command functions


	/**
	 *  Creates a pawn that can be updated through the WSP library
	 */
	static int create_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Removes a remote pawn that was being manipulated by the WSP library
	 */
	static int remove_remote_pawn_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Handles commands beginning with "pawn ...".
	 */
	static int pawn_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Handles commands beginning with "set pawn <pawn id> ...".
	 */
	static int set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Sets the given attribute of the pawn..
	 */
	static int set_attribute( SbmPawn* pawn, std::string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Handles commands beginning with "set pawn <pawn id> world_offset ...".
	 */
	static int set_world_offset_cmd( SbmPawn* pawn, srArgBuffer& args );

	/**
	 *  Handles commands beginning with "print pawn <pawn id> ...".
	 */
	static int print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p );

	/**
	 *  Prints the given attribute of the pawn.
	 */
	static int print_attribute( SbmPawn* pawn, std::string& attribute, srArgBuffer& args, mcuCBHandle *mcu_p );

	MeCtChannelWriter* get_world_offset_writer_p()	{return world_offset_writer_p;}

	/**
	 *  WSP access functions.
	 */
#if USE_WSP
	static WSP::WSP_ERROR wsp_world_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_world_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
#endif
};

#endif // SBM_PAWN_HPP
