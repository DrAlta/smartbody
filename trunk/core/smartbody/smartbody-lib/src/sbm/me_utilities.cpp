/*
 *  me_utilities.cpp - part of SmartBody-lib
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
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 */

#include "vhcl.h"

#include "me_utilities.hpp"

#include <string>
#include <iostream>
#include <stdio.h>
#include <direct.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <SK/sk_posture.h>

#include "sbm_constants.h"
#include "gwiz_math.h"
#include "ParserBVH.h"

using namespace std;
using namespace boost::filesystem;


#define DEBUG_LOAD_PATHS (0)


const char* MOTION_EXT  = ".skm";
const char* POSTURE_EXT = ".skp";

////////////////////////////////////////////////////////////////////////////////////////////

SkSkeleton* load_skeleton( const char *skel_file, srPathList &path_list, ResourceManager* manager, double scale ) {
	FILE *fp = NULL;
	char buffer[ MAX_FILENAME_LEN ];
	std::string filename;
	//char *path = NULL;
	path_list.reset();
	int done = FALSE;
	while( !done )	{
		//filename = path_list.next_filename( buffer, skel_file, & path );
		filename = path_list.next_filename( buffer, skel_file);
		if( filename.size() > 0 )	{
			if( fp = fopen( filename.c_str(), "rt" ) )	{
				done = TRUE;
			}
		}
		else	{
			done = TRUE;
		}
	}
	if( fp == NULL )	{
		LOG("ERROR: load_skeleton(..): Skeleton file \"%s\" not found.", skel_file);
		return NULL;
	}
	SrInput input( fp );
	if( !input.valid() ) {
		LOG("ERROR: load_skeleton(..): Unable to access skeleton file \"%s\".", skel_file);
		return NULL;
	}

	SkSkeleton* skeleton_p = new SkSkeleton();
#if 0
	if( !skeleton_p->load( input, path ) )	{ 
#else

	if (filename.find(".bvh") == (filename.size() - 4) || 
		filename.find(".BVH") == (filename.size() - 4))
	{
		fclose(fp);
		std::ifstream filestream(filename.c_str());
		SkMotion* motion = new SkMotion();
		ParserBVH::parse(*skeleton_p, *motion, skel_file, filestream);
		delete motion;
		skeleton_p->skfilename(filename.c_str());
	}
	else
	{
		//now the "geopath" can be still sent in the load() method as before,
		//but for extracting the path from a file name, the filename should be
		//associated with the input, as done here:
		input.filename(filename.c_str());
		if( !skeleton_p->load( input, scale ) )	{ 
	#endif
			LOG("ERROR: load_skeleton(..): Unable to load skeleton file \"%s\".", skel_file);
			return NULL;
		}
		skeleton_p->skfilename(filename.c_str());
	}
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);
	char *full_filename = new char[_MAX_PATH];
	MotionResource * motionRes = new MotionResource();
	motionRes->setType("sk");
	full_filename = mcn_return_full_filename_func( CurrentPath, filename.c_str() );
	motionRes->setMotionFile( full_filename );
	manager->addResource(motionRes);
	
	// SUCCESS
	return skeleton_p;

}

////////////////////////////////////////////////////////////////////////////////////////////

bool validate_path( path& result, const char* pathname ) {
	// Validate pathname before creating path to prevent exceptions
	string path_str( pathname );
	if( native( path_str ) ) {
		if( DEBUG_LOAD_PATHS )
			LOG("DEBUG: validate_path(..): Native path \"%s\".", path_str.c_str());
		result = path( path_str, native );
	} else if( no_check( path_str ) ) {
		if( DEBUG_LOAD_PATHS )
			LOG("DEBUG: validate_path(..): Valid no_check path  \"%s\".", path_str.c_str());
		result = path( path_str, no_check );
	} else {
		if( DEBUG_LOAD_PATHS )
			LOG("DEBUG: validate_path(..): Invalid name \"%s\". Tried native and no_check.", path_str);
		return false;
	}

	// TODO - warn about non portable names?

	return true;
}

int load_me_motions_impl( const path& pathname, std::map<std::string, SkMotion*>& map, bool recurse_dirs, ResourceManager* manager, double scale, const char* error_prefix ) {
		
	if( !exists( pathname ) ) {
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
		return CMD_FAILURE;
	}

	if( is_directory( pathname ) ) {

		directory_iterator end;
		for( directory_iterator i( pathname ); i!=end; ++i ) {
			const path& cur = *i;

			if( is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_motions_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
			} else {
				string ext = extension( cur );
				if( _stricmp( ext.c_str(), MOTION_EXT ) == 0 || _stricmp( ext.c_str(), ".bvh" ) == 0) {
					load_me_motions_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS ) {
					LOG("DEBUG: load_me_motion_impl(): Skipping \"%s\".  Extension \"%s\" does not match MOTION_EXT.", cur.string(), ext);
				}
			}
		}
	} else {

		std::string ext = extension( pathname );
		SkMotion* motion = new SkMotion();
		motion->ref();
		bool parseSuccessful = false;

		if (ext == ".skm")
		{
			SrInput in( pathname.string().c_str(), "rt" );
			SrString fullin_string;
			in.getall( fullin_string );
			SrInput fullin( (const char *)fullin_string );
			fullin.filename( pathname.string().c_str() ); // copy filename for error message
			
			parseSuccessful = motion->load( fullin, scale );
		}
		else if (ext == ".bvh")
		{
			std::ifstream filestream( pathname.string().c_str() );
			
			SkSkeleton skeleton;
			parseSuccessful = ParserBVH::parse(skeleton, *motion, pathname.string(), filestream);

			int numChannels = motion->channels().size();
			for (int c = 0; c < numChannels; c++)
			{
				SkChannel& chan = motion->channels().get(c);
				int y = 0;
			}
		}

		if (parseSuccessful)
		{
			// register the motion
			//motion->registerAnimation();

			char CurrentPath[_MAX_PATH];
			_getcwd(CurrentPath, _MAX_PATH);
			std::string filename;
			MotionResource * motionRes = new MotionResource();
			motionRes->setType("skm");
			filename = pathname.filename().c_str();
			
			//filename = mcn_return_full_filename_func( CurrentPath, finalPath.string().c_str() );
			motionRes->setMotionFile( pathname.string() );
			manager->addResource(motionRes);

			string filebase = basename( pathname );
			const char* name = motion->name();
			if( name && _stricmp( filebase.c_str(), name ) ) {
				LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, pathname.native_file_string().c_str(), filebase.c_str());
				motion->name( filebase.c_str() );
			}
			motion->filename( pathname.native_file_string().c_str() );

			std::map<std::string, SkMotion*>::iterator motionIter = map.find(filebase);
			if (motionIter != map.end()) {
				LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
				motion->unref();
				return CMD_FAILURE;
			}
			map.insert(std::pair<std::string, SkMotion*>(filebase, motion));
			
		} else {
			// SkMotion::load() already prints an error...
			//strstr << error_prefix << "Failed to load motion \"" << pathname.string() << "\"." << endl;
			motion->unref();
			return CMD_FAILURE;
		}
		
	}
	return CMD_SUCCESS;
}

int load_me_postures_impl( const path& pathname, std::map<std::string, SkPosture*>& map, bool recurse_dirs, ResourceManager* manager, double scale, const char* error_prefix ) {
	
	if( !exists( pathname ) ) {
		LOG("%s Posture path \"%s\" not found.", error_prefix, pathname.native_file_string().c_str());
		return CMD_FAILURE;
	}

	if( is_directory( pathname ) ) {
		
		directory_iterator end;
		for( directory_iterator i( pathname ); i!=end; ++i ) {
			const path& cur = *i;

			if( is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_postures_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
			} else {
				string ext = extension( cur );
				if( _stricmp( ext.c_str(), POSTURE_EXT )==0 ) {
					load_me_postures_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
				} else if( DEBUG_LOAD_PATHS ) {
					LOG("DEBUG: load_me_posture_impl(): Skipping \"%s\". Extension \"%s\" does not match POSTURE_EXT.",  cur.string().c_str(), ext.c_str());
				}
			}
		}
	} else {
		SkPosture* posture = new SkPosture();
		
		SrInput in( pathname.string().c_str(), "rt" );
		in >> (*posture);
		if( in.had_error() ) {
			std::stringstream strstr;
			strstr << error_prefix << "Failed to load posture \"" << pathname.string() << "\".";
			LOG(strstr.str().c_str());
			posture->unref();
			return CMD_FAILURE;
		} else {

			char CurrentPath[_MAX_PATH];
			_getcwd(CurrentPath, _MAX_PATH);
			std::string filename;
			MotionResource * motionRes = new MotionResource();
			motionRes->setType("skp");
			filename = mcn_return_full_filename_func( CurrentPath, pathname.string().c_str() );
			motionRes->setMotionFile( filename );
			manager->addResource(motionRes);

			string filebase = basename( pathname );
			const char* name = posture->name();
			if( name && _stricmp( filebase.c_str(), name ) ) {
				std::stringstream strstr;
				strstr << "WARNING: Posture name \"" << name << "\" does not equal base of filename \"" << pathname.native_file_string() << "\".  Using \"" << filebase << "\" in posture map." << endl;
				LOG(strstr.str().c_str());
				posture->name( filebase.c_str() );
			}
			posture->filename( pathname.native_file_string().c_str() );

			std::map<std::string, SkPosture*>::iterator postureIter = map.find(filebase);
			if (postureIter != map.end()) {
				std::stringstream strstr;
				strstr << "ERROR: Posture by name of \"" << filebase << "\" already exists.  Ignoring file \"" << pathname.native_file_string() << "\"." << endl;
				LOG(strstr.str().c_str());
				return CMD_FAILURE;
			}
			map.insert(std::pair<std::string, SkPosture*>(filebase, posture));
			posture->ref();
		}
	}
	return CMD_SUCCESS;
}

int load_me_motions( const char* pathname, std::map<std::string, SkMotion*>& map, bool recurse_dirs, ResourceManager* manager, double scale ) {
	path motions_path(pathname);
	
	path finalPath;
	// include the media path in the pathname if applicable
	std::string rootDir = motions_path.root_directory();
	if (rootDir.size() == 0)
	{		
		finalPath = operator/(mcuCBHandle::singleton().getMediaPath(), motions_path);
	}
	else
	{
		finalPath = pathname;
	}

	if (1) {
		return load_me_motions_impl( finalPath, map, recurse_dirs, manager, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid motion path \"%s\".", finalPath);
		return CMD_FAILURE;
	}
}

int load_me_postures( const char* pathname, std::map<std::string, SkPosture*>& map, bool recurse_dirs, ResourceManager* manager, double scale ) {
	path posture_path(pathname);

	path finalPath;
	// include the media path in the pathname if applicable
	std::string rootDir = posture_path.root_directory();
	if (rootDir.size() == 0)
	{		
		finalPath = operator/(mcuCBHandle::singleton().getMediaPath(), posture_path);
	}
	else
	{
		finalPath = pathname;
	}

	if (1) {
		return load_me_postures_impl( finalPath, map, recurse_dirs, manager, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid posture path \"%s\".", pathname);
		return CMD_FAILURE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void print_joint( const SkJoint* joint ) {
	const SkJointPos* pos = joint->const_pos();

	cout << "position:\t" << pos->value(SkVecLimits::X) << ", "
	                        << pos->value(SkVecLimits::Y) << ", "
	                        << pos->value(SkVecLimits::Z);

	switch( joint->rot_type() ) {
		case SkJoint::TypeEuler: {
			const SkJointEuler* euler = joint->const_euler();
			cout << "\trotation euler:\t" << euler->value(SkVecLimits::X) << ", "
			                              << euler->value(SkVecLimits::Y) << ", "
			                              << euler->value(SkVecLimits::Z) << endl;
			break;
		}
		case SkJoint::TypeQuat: {
			// const_cast because the SrQuat does validation (no const version of value())
			const SrQuat& quat = (const_cast<SkJoint*>(joint))->quat()->value();
			cout << "\trotation quat:\t" << quat.w << ", "
			                             << quat.x << ", "
			                             << quat.y << ", "
			                             << quat.z;
			// Marcus's mappings
			euler_t euler( quat_t( quat.w, quat.x, quat.y, quat.z ) );
			cout << "\t(as hpr: " << euler.y() << ", "
			                      << euler.x() << ", "
			                      << euler.z() << ")" << endl;
			break;
		}
		case SkJoint::TypeSwingTwist: {
			// const_cast because the SwingTwist does validation (no const version of swingx(), etc.)
			SkJointSwingTwist* st = (const_cast<SkJoint*>(joint))->st();
			cout << "\troation swing-twist (xyt):\t" << st->swingx() << ", "
			                                         << st->swingy() << ", "
			                                         << st->twist() << endl;
			break;
		}
	}
}
