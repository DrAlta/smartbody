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

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <sk/sk_posture.h>

#include "sbm_constants.h"
#include "gwiz_math.h"
#include "ParserBVH.h"
#include "ParserOpenCOLLADA.h"
#include "ParserASFAMC.h"
#include "ParserFBX.h"
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>

using namespace std;
using namespace boost::filesystem;


#define DEBUG_LOAD_PATHS (0)


const char* MOTION_EXT  = ".skm";
const char* POSTURE_EXT = ".skp";

////////////////////////////////////////////////////////////////////////////////////////////

SkSkeleton* load_skeleton( const char *skel_file, srPathList &path_list, SBResourceManager* manager, double scale ) {
	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkSkeleton*>::iterator iter = mcu.skeleton_map.find(std::string(skel_file));
	if (iter != mcu.skeleton_map.end())
	{
		//SkSkeleton* ret = new SkSkeleton(iter->second);
		SkSkeleton* existingSkel = iter->second;
		SmartBody::SBSkeleton* existingSBSkel = dynamic_cast<SmartBody::SBSkeleton*>(existingSkel);
		SkSkeleton* ret = new SmartBody::SBSkeleton(existingSBSkel);
		ret->ref();
		return ret;
	}
	
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

	SkSkeleton* skeleton_p = new SmartBody::SBSkeleton();
	skeleton_p->ref();
#if 0
	if( !skeleton_p->load( input, path ) )	{ 
#else
	if (filename.find(".bvh") == (filename.size() - 4) || 
		filename.find(".BVH") == (filename.size() - 4))
	{
		fclose(fp);
		std::ifstream filestream(filename.c_str());
		SkMotion motion;
		ParserBVH::parse(*skeleton_p, motion, skel_file, filestream, float(scale));
		skeleton_p->skfilename(filename.c_str());
	}
	else if (filename.find(".asf") == (filename.size() - 4) || 
			 filename.find(".ASF") == (filename.size() - 4))
	{
		fclose(fp);
		std::ifstream filestream(filename.c_str());
		std::ifstream datastream("");
		SkMotion motion;
		ParserASFAMC::parse(*skeleton_p, motion, filestream, datastream, float(scale));
		skeleton_p->skfilename(filename.c_str());
	}
	else if (filename.find(".dae") == (filename.size() - 4) || 
			 filename.find(".DAE") == (filename.size() - 4))
	{
		fclose(fp);
		SkMotion motion;
		ParserOpenCOLLADA::parse(*skeleton_p, motion, filename, float(scale), true, false);
		skeleton_p->skfilename(filename.c_str());
		skeleton_p->name(skel_file);
	}
#if ENABLE_FBX_PARSER
	else if (filename.find(".fbx") == (filename.size() - 4) || 
			 filename.find(".FBX") == (filename.size() - 4))
	{
		fclose(fp);
		SkMotion motion;
		//LOG("FBX parse load skeleton: %s", filename.c_str());
		ParserFBX::parse(*skeleton_p, motion, filename, float(scale));
		skeleton_p->skfilename(filename.c_str());
		skeleton_p->name(skel_file);
	}
#endif
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
//	char CurrentPath[_MAX_PATH];
//	_getcwd(CurrentPath, _MAX_PATH);
//	char *full_filename = new char[_MAX_PATH];
	SkeletonResource * skelRes = new SkeletonResource();
	skelRes->setType("sk");

//	full_filename = mcn_return_full_filename_func( CurrentPath, filename.c_str() );
//	char *full_filename = new char[_MAX_PATH]; // REALLY??
	
	boost::filesystem::path p( filename );
	boost::filesystem::path abs_p = boost::filesystem::complete( p );
    if ( boost::filesystem2::exists( abs_p ) )	{
//		sprintf( full_filename, "%s", abs_p.string().c_str() );
		skelRes->setSkeletonFile( abs_p.string() );
		manager->addResource(skelRes);
	}
	else	{
		LOG( "load_skeleton ERR: path '%s' does not exist\n", abs_p.string().c_str() );
	}
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
			LOG("DEBUG: validate_path(..): Invalid name \"%s\". Tried native and no_check.", path_str.c_str() );
		return false;
	}

	// TODO - warn about non portable names?

	return true;
}

int load_me_motions_impl( const path& pathname, std::map<std::string, SkMotion*>& map, bool recurse_dirs, SBResourceManager* manager, double scale, const char* error_prefix ) {
	if( !exists( pathname ) ) {
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
		return CMD_FAILURE;
	}

	if( is_directory( pathname ) ) {
		// ignore any '.' diretories
		std::string filebase = pathname.leaf();
		if (filebase.find(".") == 0)
		{
			// ignore hidden directories
			return CMD_SUCCESS;
		}
		LOG("Attempting to load motions from path '%s'...", pathname.string().c_str());	
	

		directory_iterator end;
		for( directory_iterator i( pathname ); i!=end; ++i ) {
			const path& cur = *i;

			if( is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_motions_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
			} else {
				string ext = extension( cur );
#if ENABLE_FBX_PARSER
				if( _stricmp( ext.c_str(), MOTION_EXT ) == 0 || 
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".amc" ) == 0 ||
					_stricmp( ext.c_str(), ".fbx" ) == 0)
#else
				if( _stricmp( ext.c_str(), MOTION_EXT ) == 0 || 
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".amc" ) == 0)
#endif
				{
					load_me_motions_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS ) {
					LOG("DEBUG: load_me_motion_impl(): Skipping \"%s\".  Extension \"%s\" does not match MOTION_EXT.", cur.string().c_str(), ext.c_str() );
				}
			}
		}
	} else {

		std::string ext = extension( pathname );
		SkMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = false;

		if (ext == ".skm" || ext == ".SKM")
		{
			SrInput in( pathname.string().c_str(), "rt" );
			SrString fullin_string;
			in.getall( fullin_string );
			SrInput fullin( (const char *)fullin_string );
			fullin.filename( pathname.string().c_str() ); // copy filename for error message
			
			parseSuccessful = motion->load( fullin, scale );

		}
		else if (ext == ".bvh" || ext == ".BVH")
		{
			std::ifstream filestream( pathname.string().c_str() );
			
			SkSkeleton skeleton;
			parseSuccessful = ParserBVH::parse(skeleton, *motion, pathname.string(), filestream, float(scale));

		}
		else if (ext == ".dae" || ext == ".DAE" || ext == ".xml" || ext == ".XML")
		{			
			SkSkeleton skeleton;
			parseSuccessful = ParserOpenCOLLADA::parse(skeleton, *motion, pathname.string(), float(scale), true, true);			
		}
		else if (ext == ".amc" || ext == ".AMC")
		{
			// at the same directory, looking for one asf file
			std::string asf = "";
			directory_iterator end;
			std::string filebase = basename(pathname);
			std::string fileext = extension(pathname);
			int dirSize = pathname.string().size() - filebase.size() - fileext.size() - 1;
			std::string directory = pathname.string().substr(0, dirSize);
			for( directory_iterator i( directory ); i!=end; ++i ) 
			{
				const path& cur = *i;
				if (!is_directory(cur)) 
				{
					std::string ext = extension(cur);
					if (_stricmp(ext.c_str(), ".asf") == 0)
					{
						asf = cur.string().c_str();
						break;
					}
				}
			}
			std::ifstream metafilestream(asf.c_str());
			std::ifstream filestream(pathname.string().c_str());
			SkSkeleton skeleton;
			parseSuccessful = ParserASFAMC::parse(skeleton, *motion, metafilestream, filestream, float(scale));
			motion->setName(filebase.c_str());
		}
#if ENABLE_FBX_PARSER
		else if (ext == ".fbx" || ext == ".FBX")
		{
			SkSkeleton skeleton;
			LOG("FBX motion parse: %s", pathname.string().c_str());
			parseSuccessful = ParserFBX::parse(skeleton, *motion, pathname.string(), float(scale));	
		}
#endif
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
			const char* name = motion->getName().c_str();
			if( name && _stricmp( filebase.c_str(), name ) ) {
				LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, pathname.native_file_string().c_str(), filebase.c_str());
				motion->setName( filebase.c_str() );
			}
			motion->filename( pathname.native_file_string().c_str() );

			std::map<std::string, SkMotion*>::iterator motionIter = map.find(filebase);
			if (motionIter != map.end()) {
				LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
				delete motion;
				return CMD_FAILURE;
			}
			map.insert(std::pair<std::string, SkMotion*>(filebase, motion));
			
		} else {
			// SkMotion::load() already prints an error...
			//strstr << error_prefix << "Failed to load motion \"" << pathname.string() << "\"." << endl;
			delete motion;
			return CMD_FAILURE;
		}
		
	}
	return CMD_SUCCESS;
}

int load_me_skeletons_impl( const path& pathname, std::map<std::string, SkSkeleton*>& map, bool recurse_dirs, SBResourceManager* manager, double scale, const char* error_prefix ) {
		
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
					load_me_skeletons_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
			} else {
				string ext = extension( cur );
#if ENABLE_FBX_PARSER
				if( _stricmp( ext.c_str(), ".sk" ) == 0 ||
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".BVH" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".DAE" ) == 0 ||
					_stricmp( ext.c_str(), ".asf" ) == 0 ||
					_stricmp( ext.c_str(), ".ASF" ) == 0 ||
					_stricmp( ext.c_str(), ".fbx" ) == 0 ||
					_stricmp( ext.c_str(), ".FBX" ) == 0)
#else
				if( _stricmp( ext.c_str(), ".sk" ) == 0 ||
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".BVH" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".DAE" ) == 0 ||
					_stricmp( ext.c_str(), ".asf" ) == 0 ||
					_stricmp( ext.c_str(), ".ASF" ) == 0)
#endif
				{
					load_me_skeletons_impl( cur, map, recurse_dirs, manager, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS ) {
					LOG("DEBUG: load_me_skeleton_impl(): Skipping \"%s\".  Extension \"%s\" does not match .sk.", cur.string().c_str(), ext.c_str() );
				}
			}
		}
	} else {

		std::string ext = extension( pathname );
		std::string filebase = boost::filesystem::basename(pathname);

		SkSkeleton* skeleton = NULL;
		if (ext == ".sk")
		{			
			skeleton = new SmartBody::SBSkeleton();
			skeleton->ref();
			
			FILE* fp = fopen( pathname.string().c_str(), "rt" );
			if (fp)
			{
				SrInput input(fp);
				input.filename(pathname.string().c_str());
				if( !skeleton->load( input, scale ) )
				{ 
					LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
					input.close();
					delete skeleton;
					return CMD_FAILURE;
				}
				else
				{
					std::string fullName = filebase + ext;
					skeleton->name(fullName.c_str());
					map.insert(std::pair<std::string, SkSkeleton*>(filebase + ext, skeleton));
				}
			}
		}
		else if (ext == ".bvh" || ext == ".BVH")
		{		
			std::ifstream filestream(pathname.string().c_str());
			skeleton = new SmartBody::SBSkeleton();
			skeleton->ref();
			skeleton->skfilename(filebase.c_str());
			SkMotion motion;
			bool ok = ParserBVH::parse(*skeleton, motion, filebase, filestream, float(scale));
			if (ok)
			{
				std::map<std::string, SkSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SkSkeleton*>(filebase + ext, skeleton));
			}
			else
			{
				LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
				return CMD_FAILURE;
			}
		}
		else if (ext == ".asf" || ext == ".ASF")
		{		
			std::ifstream filestream(pathname.string().c_str());
			std::ifstream datastream("");
			skeleton = new SmartBody::SBSkeleton();
			skeleton->skfilename(filebase.c_str());
			SkMotion motion;
			bool ok = ParserASFAMC::parse(*skeleton, motion, filestream, datastream, float(scale));
			if (ok)
			{
				std::map<std::string, SkSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SkSkeleton*>(filebase + ext, skeleton));
			}
			else
			{
				LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
				return CMD_FAILURE;
			}
		}
		else if (ext == ".dae" || ext == ".DAE")
		{			
			skeleton =  new SmartBody::SBSkeleton();
			skeleton->skfilename(filebase.c_str());
			skeleton->name(filebase.c_str());
			SkMotion motion;
			bool ok = ParserOpenCOLLADA::parse(*skeleton, motion, pathname.string(), float(scale), true, false);
			if (ok)
			{
				std::map<std::string, SkSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SkSkeleton*>(filebase + ext, skeleton));
			}
			else
			{
				LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
				return CMD_FAILURE;
			}
		}
#if ENABLE_FBX_PARSER
		else if (ext == ".fbx" || ext == ".FBX")
		{
			skeleton = new SkSkeleton();
			skeleton->skfilename(filebase.c_str());
			skeleton->name(filebase.c_str());
			SkMotion motion;
			//LOG("FBX skeleton skeleton load: %s", pathname.string().c_str());
			bool ok = ParserFBX::parse(*skeleton, motion, pathname.string(), float(scale));
			if (ok)
			{
				std::map<std::string, SkSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SkSkeleton*>(filebase + ext, skeleton));
			}
			else
			{
				LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
				return CMD_FAILURE;
			}
		}
#endif

		skeleton->skfilename(pathname.string().c_str());		
		SBResourceManager* manager = SBResourceManager::getResourceManager();
		SkeletonResource* skelRes = new SkeletonResource();
		skelRes->setType("skm");
		skelRes->setSkeletonFile(pathname.string());
		manager->addResource(skelRes);
	}
	return CMD_SUCCESS;
}

int load_me_postures_impl( const path& pathname, std::map<std::string, SkPosture*>& map, bool recurse_dirs, SBResourceManager* manager, double scale, const char* error_prefix ) {
	
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
			delete posture;
			return CMD_FAILURE;
		} else {

//			char CurrentPath[_MAX_PATH];
//			_getcwd(CurrentPath, _MAX_PATH);
			std::string filename;
			MotionResource * motionRes = new MotionResource();
			motionRes->setType("skp");
			
//			filename = mcn_return_full_filename_func( CurrentPath, pathname.string().c_str() );

//			boost::filesystem::path p( pathname.string() );
			boost::filesystem::path abs_p = boost::filesystem::complete( pathname );
            if ( boost::filesystem2::exists( abs_p ) )	{
				filename = abs_p.string();
			}
			
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
		}
	}
	return CMD_SUCCESS;
}

int load_me_motions( const char* pathname, std::map<std::string, SkMotion*>& map, bool recurse_dirs, SBResourceManager* manager, double scale ) {
	path motions_path(pathname);
	
	path finalPath;
	// include the media path in the pathname if applicable
	std::string rootDir = motions_path.root_directory();
	if (rootDir.size() == 0)
	{	
		std::string mediaPath = mcuCBHandle::singleton().getMediaPath();
		finalPath = operator/(mediaPath, motions_path);
	}
	else
	{
		finalPath = pathname;
	}

	if (1) {
		return load_me_motions_impl( finalPath, map, recurse_dirs, manager, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid motion path \"%s\".", finalPath.string().c_str());
		return CMD_FAILURE;
	}
}

int load_me_skeletons( const char* pathname, std::map<std::string, SkSkeleton*>& map, bool recurse_dirs, SBResourceManager* manager, double scale ) {
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
		return load_me_skeletons_impl( finalPath, map, recurse_dirs, manager, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid skeleton path \"%s\".", finalPath.string().c_str() );
		return CMD_FAILURE;
	}
}

int load_me_postures( const char* pathname, std::map<std::string, SkPosture*>& map, bool recurse_dirs, SBResourceManager* manager, double scale ) {
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
	std::stringstream strstr;

	strstr << "position:\t" << pos->value(SkVecLimits::X) << ", "
	                        << pos->value(SkVecLimits::Y) << ", "
	                        << pos->value(SkVecLimits::Z) << endl;

	switch( joint->rot_type() ) {
		case SkJoint::TypeEuler: {
			const SkJointEuler* euler = joint->const_euler();
			strstr << "rotation euler:\t" << euler->value(SkVecLimits::X) << ", "
			                              << euler->value(SkVecLimits::Y) << ", "
			                              << euler->value(SkVecLimits::Z) << endl;
			break;
		}
		case SkJoint::TypeQuat: {
			// const_cast because the SrQuat does validation (no const version of value())
			const SrQuat& quat = (const_cast<SkJoint*>(joint))->quat()->value();
			strstr << "rotation quat:\t" << quat.w << ", "
			                             << quat.x << ", "
			                             << quat.y << ", "
			                             << quat.z;
			// Marcus's mappings
			gwiz::euler_t euler( gwiz::quat_t( quat.w, quat.x, quat.y, quat.z ) );
			strstr << "(as hpr: " << euler.y() << ", "
			                      << euler.x() << ", "
			                      << euler.z() << ")" << endl;
			break;
		}
		case SkJoint::TypeSwingTwist: {
			// const_cast because the SwingTwist does validation (no const version of swingx(), etc.)
			SkJointSwingTwist* st = (const_cast<SkJoint*>(joint))->st();
			strstr << "rotation swing-twist (xyt):\t" << st->swingx() << ", "
			                                         << st->swingy() << ", "
			                                         << st->twist() << endl;

			break;
		}
	}
	LOG(strstr.str().c_str());
}
