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

#include <sb/SBTypes.h>
#include <sk/sk_scene.h>
#include <sk/sk_skeleton.h>
#include <sb/SBColObject.h>
#include <sb/SBPhysicsSim.h>
#include <sb/SBObject.h>
#include <SteerLib.h>

#include "bonebus.h"

#if USE_WSP
#include "wsp.h"
#endif

#include <map>


// Declare classes used (avoid circular references)
class srArgBuffer;

class MeCtChannelWriter;
class MeControllerTreeRoot;

#if !defined(__FLASHPLAYER__)
class DeformableMesh;
class DeformableMeshInstance;
#else
#include <sbm/sbm_deformable_mesh.h>
#endif


#define SBM_PAWN_USE_WORLD_OFFSET_WRITER	(1)
#define SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK	(0)

class SbmPawn : public SmartBody::SBObject, public SBTransformObjInterface {
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
	//SBGeomObject* _collisionObject;
	std::string collisionObjName;

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
	// Map of pending controller clean-up callbacks
	std::multimap<MeController*,controller_cleanup_callback_fp> ct_cleanup_funcs;
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

public:  // TODO - properly encapsulate / privatize the following
	SkSkeleton*	_skeleton;  // MAY BE NULL!!!
	SkScene*		scene_p;	 // Skeleton Scene and Rigid Mesh		
	DeformableMeshInstance* dMeshInstance_p;
	SrSnGroup* blendMeshGroup;
	SteerLib::ObstacleInterface* steeringSpaceObj_p;
	SrVec			steeringSpaceObjSize;
	std::string _classType;
	
	
	//		float scale = 0.5f;
//		SteerLib::BoxObstacle* box = new SteerLib::BoxObstacle(x / 100.0f - scale, x / 100.0f + scale, y / 100.0f - scale, y / 100.0f + scale, z / 100.0f - scale, z / 100.0f + scale);
//		mcuCBHandle::singleton().steerEngine->_engine->addObstacle(box);

	// Temporarily, until there is a unified multi-skeleton controller tree
	MeControllerTreeRoot	*ct_tree_p;
	bonebus::BoneBusCharacter * bonebusCharacter;
	SBTransform                globalTransform;

public:	
	//  Public Methods
	SBAPI SbmPawn();
	SBAPI SbmPawn( const char* name );
	SBAPI virtual ~SbmPawn();

	SBAPI virtual void copy(SbmPawn* orignalPawn);

	SBAPI SkSkeleton* getSkeleton() const;
	SBAPI void setSkeleton(SkSkeleton* sk);	
	SBAPI virtual int init( SkSkeleton* skeleton_p );

	SBAPI const std::string& getGeomObjectName();
	SBAPI SBGeomObject* getGeomObject(); // get geometry object associated with the pawn
	//void setGeomObject(SBGeomObject* obj);

 	SBAPI void updateToColObject();
	SBAPI void updateToSteeringSpaceObject();
	SBAPI void initSteeringSpaceObject();

	SBAPI bool is_initialized();

	SBAPI void setHeight( float height )	{ _height = height; }
	SBAPI float getHeight( void ) 		{ return _height; }
	SBAPI SrBox getBoundingBox( void ) 		{ if (_skeleton) return _skeleton->getBoundingBox(); return SrBox(); }

	SBAPI virtual int prune_controller_tree();  // removes unused or overwritten controllers
	
	SBAPI void reset_all_channels();

	SBAPI const SkJoint* get_joint( const char* joint_name ) const;
	SBAPI const SkJoint* get_world_offset_joint() const
	{	return get_joint( WORLD_OFFSET_JOINT_NAME ); }

	SBAPI virtual SBTransform& getGlobalTransform();
	SBAPI virtual void setGlobalTransform(SBTransform& newGlobalTransform);

	SBAPI SrMat get_world_offset();
	SBAPI void get_world_offset( float& x, float& y, float& z,
		                   float& yaw, float& pitch, float& roll );

	SBAPI void setWorldOffset(const SrMat& newWorld);
	SBAPI void set_world_offset( float x, float y, float z,
		                   float yaw, float pitch, float roll );

	SBAPI virtual std::string getClassType();
	SBAPI virtual void setClassType(std::string classType);

	SBAPI virtual void notify(SBSubject* subject);

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

	void initData();

	


public:
	/**
	 *  Removes a remote pawn that was being manipulated by the WSP library
	 */
	static int remove_remote_pawn_func( srArgBuffer& args);



	MeCtChannelWriter* get_world_offset_writer_p()	{return world_offset_writer_p;}

	void wo_cache_update();
	

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
