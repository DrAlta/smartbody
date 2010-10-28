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

#include <SK/sk_scene.h>
#include <SK/sk_skeleton.h>

#include <ME/me_controller_tree_root.hpp>
#include <ME/me_ct_raw_writer.hpp>
#include "wsp.h"

#include <map>

#include "sbm_constants.h"
#include "sbm_deformable_mesh.h"

// Declare classes used (avoid circular references)
class mcuCBHandle;
class srArgBuffer;


#define SBM_PAWN_USE_WORLD_OFFSET_WRITER	(1)
#define SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK	(0)

class SbmPawn {
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

	MeCtRawWriter*  world_offset_writer_p;

#if SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK
	// Map of pending controller clean-up callbacks
	std::multimap<MeController*,controller_cleanup_callback_fp> ct_cleanup_funcs;
#endif // SBM_PAWN_USE_CONTROLLER_CLEANUP_CALLBACK

public:  // TODO - properly encapsulate / privatize the following
    char*           name;
	SkSkeleton*		skeleton_p;  // MAY BE NULL!!!
	SkScene*		scene_p;	 // Skeleton Scene and Rigid Mesh		
	DeformableMesh*	dMesh_p;	 // Deformable Mesh using smooth skinning

	// Temporarily, until there is a unified multi-skeleton controller tree
	MeControllerTreeRoot	*ct_tree_p;

public:
	
	//  Public Methods
	SbmPawn( const char* name );
	virtual ~SbmPawn();

	virtual int init( SkSkeleton* skeleton_p );

	bool is_initialized();

	virtual int prune_controller_tree();  // removes unused or overwritten controllers
	virtual void remove_from_scene();

	void reset_all_channels();

	const SkJoint* get_joint( const char* joint_name ) const;
	const SkJoint* get_world_offset_joint() const
	{	return get_joint( WORLD_OFFSET_JOINT_NAME ); }

	void get_world_offset( float& x, float& y, float& z,
		                   float& yaw, float& pitch, float& roll );
	void set_world_offset( float x, float y, float z,
		                   float yaw, float pitch, float roll );

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


	void wo_cache_update();


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
	 *  Removes a pawn from the scene by name.
	 *  Using "*" as a pawn name will remove all pawns.
	 */
	static int remove_from_scene( const char* pawn_name );

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

	/**
	 *  WSP access functions.
	 */
	static WSP::WSP_ERROR wsp_world_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_world_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_position_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
	static WSP::WSP_ERROR wsp_rotation_accessor( const std::string id, const std::string attribute_name, wsp_vector & value, void * data );
};

#endif // SBM_PAWN_HPP
