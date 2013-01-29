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

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <sk/sk_posture.h>
#include <sb/SBMotion.h>

#include "sbm_constants.h"
#include "sbm/gwiz_math.h"
#include "ParserBVH.h"
#include "ParserOpenCOLLADA.h"
#include "ParserOgre.h"
#include "ParserASFAMC.h"
#include "ParserFBX.h"
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>
#include <sbm/lin_win.h>

using namespace std;
using namespace boost::filesystem;


#define DEBUG_LOAD_PATHS1 (0)


////////////////////////////////////////////////////////////////////////////////////////////

SmartBody::SBSkeleton* load_skeleton( const char *skel_file, srPathList &path_list, double scale ) {
	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SmartBody::SBSkeleton*>::iterator iter = mcu.skeleton_map.find(std::string(skel_file));
	if (iter != mcu.skeleton_map.end())
	{
		//SkSkeleton* ret = new SkSkeleton(iter->second);
		SmartBody::SBSkeleton* existingSkel = iter->second;
		SmartBody::SBSkeleton* ret = new SmartBody::SBSkeleton(existingSkel);
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

	SmartBody::SBSkeleton* skeleton_p = new SmartBody::SBSkeleton();
	skeleton_p->ref();
#if 0
	if( !skeleton_p->load( input, path ) )	{ 
#else
	if (filename.find(".bvh") == (filename.size() - 4) || 
		filename.find(".BVH") == (filename.size() - 4))
	{
		fclose(fp);
		std::ifstream filestream(filename.c_str());
		SmartBody::SBMotion motion;
		ParserBVH::parse(*skeleton_p, motion, skel_file, filestream, float(scale));
		skeleton_p->skfilename(filename.c_str());
	}
	else if (filename.find(".asf") == (filename.size() - 4) || 
			 filename.find(".ASF") == (filename.size() - 4))
	{
		fclose(fp);
		std::ifstream filestream(filename.c_str());
		std::ifstream datastream("");
		SmartBody::SBMotion motion;
		ParserASFAMC::parse(*skeleton_p, motion, filestream, datastream, float(scale));
		skeleton_p->skfilename(filename.c_str());
	}
	else if (filename.find(".dae") == (filename.size() - 4) || 
			 filename.find(".DAE") == (filename.size() - 4))
	{
		fclose(fp);
		SmartBody::SBMotion motion;
		ParserOpenCOLLADA::parse(*skeleton_p, motion, filename, float(scale), true, false);
		skeleton_p->skfilename(filename.c_str());
		skeleton_p->setName(skel_file);
	}
	else if (filename.find(".skeleton.xml") == (filename.size() - 13) || 
			 filename.find(".SKELETON.XML") == (filename.size() - 13))
	{
		fclose(fp);
		SmartBody::SBMotion motion;
		std::vector<SmartBody::SBMotion*> motions;
		motions.push_back(&motion);
		ParserOgre::parse(*skeleton_p, motions, filename, float(scale), true, false);
		skeleton_p->skfilename(filename.c_str());
		skeleton_p->setName(skel_file);
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
		if( !skeleton_p->loadSk( input, scale ) )	{ 
#endif
			LOG("ERROR: load_skeleton(..): Unable to load skeleton file \"%s\".", skel_file);
			return NULL;
		}
		skeleton_p->skfilename(filename.c_str());
	}
//	char CurrentPath[_MAX_PATH];
//	_getcwd(CurrentPath, _MAX_PATH);
//	char *full_filename = new char[_MAX_PATH];

//	full_filename = mcn_return_full_filename_func( CurrentPath, filename.c_str() );
//	char *full_filename = new char[_MAX_PATH]; // REALLY??
	
	boost::filesystem::path p( filename );
	boost::filesystem::path abs_p = boost::filesystem::complete( p );
    if ( boost::filesystem2::exists( abs_p ) )	{
//		sprintf( full_filename, "%s", abs_p.string().c_str() );
		
	}
	else	{
		LOG( "load_skeleton ERR: path '%s' does not exist\n", abs_p.string().c_str() );
	}
	// SUCCESS
	return skeleton_p;

}



int load_me_skeletons_impl( const path& pathname, std::map<std::string, SmartBody::SBSkeleton*>& map, bool recurse_dirs, double scale, const char* error_prefix ) {
		
	if( !exists( pathname ) ) {
		LOG("%s Skeleton path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
		return CMD_FAILURE;
	}

	if( is_directory( pathname ) ) {

		directory_iterator end;
		for( directory_iterator i( pathname ); i!=end; ++i ) {
			const path& cur = *i;

			if( is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_skeletons_impl( cur, map, recurse_dirs, scale, "WARNING: " );
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
					_stricmp( ext.c_str(), ".xml" ) == 0 ||
					_stricmp( ext.c_str(), ".FBX" ) == 0)
#else
				if( _stricmp( ext.c_str(), ".sk" ) == 0 ||
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".BVH" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".DAE" ) == 0 ||
					_stricmp( ext.c_str(), ".asf" ) == 0 ||
					_stricmp( ext.c_str(), ".ASF" ) == 0 ||
					_stricmp( ext.c_str(), ".xml" ) == 0)
#endif
				{
					load_me_skeletons_impl( cur, map, recurse_dirs, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS1 ) {
					LOG("DEBUG: load_me_skeleton_impl(): Skipping \"%s\".  Extension \"%s\" does not match .sk.", cur.string().c_str(), ext.c_str() );
				}
			}
		}
	} else {

		std::string ext = extension( pathname );
		std::string filebase = boost::filesystem::basename(pathname);
		std::string fullName = filebase + ext;
		SmartBody::SBSkeleton* skeleton = NULL;
		if (ext == ".sk")
		{			
			skeleton = new SmartBody::SBSkeleton();
			skeleton->ref();
			
			FILE* fp = fopen( pathname.string().c_str(), "rt" );
			if (fp)
			{
				SrInput input(fp);
				input.filename(pathname.string().c_str());
				if( !skeleton->loadSk( input, scale ) )
				{ 
					LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
					input.close();
					delete skeleton;
					return CMD_FAILURE;
				}
				else
				{
					std::string fullName = filebase + ext;
					skeleton->setName(fullName);
					SmartBody::SBSkeleton* sbskel = dynamic_cast<SmartBody::SBSkeleton*>(skeleton);
					sbskel->setFileName(pathname.string());
					map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(fullName, skeleton));
				}
			}
		}
		else if (ext == ".bvh" || ext == ".BVH")
		{		
			std::ifstream filestream(pathname.string().c_str());
			skeleton = new SmartBody::SBSkeleton();
			skeleton->ref();
			skeleton->skfilename(fullName.c_str());
			skeleton->setName(fullName);
			SkMotion motion;
			bool ok = ParserBVH::parse(*skeleton, motion, filebase, filestream, float(scale));
			if (ok)
			{
				std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(filebase + ext, skeleton));
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
			skeleton->skfilename(fullName.c_str());
			skeleton->setName(fullName.c_str());
			SkMotion motion;
			bool ok = ParserASFAMC::parse(*skeleton, motion, filestream, datastream, float(scale));
			if (ok)
			{
				std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(filebase + ext, skeleton));
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
			skeleton->skfilename(fullName.c_str());				
			skeleton->setName(fullName.c_str());
			SkMotion motion;
			bool ok = ParserOpenCOLLADA::parse(*skeleton, motion, pathname.string(), float(scale), true, false);
			if (ok)
			{
				std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(filebase + ext, skeleton));
			}
			else
			{
				LOG("Problem loading skeleton from file '%s'.", pathname.string().c_str());
				return CMD_FAILURE;
			}
		}
		else if (ext == ".xml" || ext == ".XML")
		{			
			skeleton =  new SmartBody::SBSkeleton();
			skeleton->skfilename(fullName.c_str());			
			skeleton->setName(fullName);
			std::vector<SmartBody::SBMotion*> motions;
			bool ok = ParserOgre::parse(*skeleton, motions, pathname.string(), float(scale), true, false);
			if (ok)
			{
				std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = map.find(filebase);
				if (motionIter != map.end()) {
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete skeleton;
					return CMD_FAILURE;
				}
				map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(filebase + ext, skeleton));
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

		skeleton->ref();		
		skeleton->setName(skeleton->skfilename());
		
	}
	return CMD_SUCCESS;
}




int load_me_skeleton_individual( SrInput & input, const std::string & skeletonName, std::map<std::string, SmartBody::SBSkeleton*>& map, double scale )
{
	SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();
	skeleton->ref();

	//path skeletonPath(skeletonName);
	//string ext = extension(skeletonPath);
	//ext = vhcl::ToLower(ext);
	//if (ext == ".sk")

	if( !skeleton->loadSk( input, scale ) )
	{ 
		LOG("Problem loading skeleton from file ''.");
		input.close();
		delete skeleton;
		return CMD_FAILURE;
	}
	else
	{
		skeleton->setName(skeletonName);
		SmartBody::SBSkeleton* sbskel = dynamic_cast<SmartBody::SBSkeleton*>(skeleton);
		sbskel->setFileName(skeletonName);
		map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(skeletonName, skeleton));
	}


	skeleton->skfilename(skeletonName.c_str());
	std::string filebasename = boost::filesystem::basename(skeleton->skfilename());
	std::string fileextension = boost::filesystem::extension(skeleton->skfilename());
	skeleton->setName(filebasename+fileextension);	


	return CMD_SUCCESS;
}

int load_me_skeletons( const char* pathname, std::map<std::string, SmartBody::SBSkeleton*>& map, bool recurse_dirs, double scale ) {
	path motions_path(pathname);
	
	path finalPath;
	// include the media path in the pathname if applicable
	
	std::string rootDir = motions_path.root_directory();
	if (rootDir.size() == 0)
	{		
		finalPath = operator/(SmartBody::SBScene::getScene()->getMediaPath(), motions_path);
	}
	else
	{
		finalPath = pathname;
	}

	if (1) {
		return load_me_skeletons_impl( finalPath, map, recurse_dirs, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid skeleton path \"%s\".", finalPath.string().c_str() );
		return CMD_FAILURE;
	}
}



