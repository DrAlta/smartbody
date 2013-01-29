#include "SBAssetManager.h"
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sbm/mcontrol_util.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <sbm/ParserBVH.h>
#include <sbm/ParserOpenCOLLADA.h>
#include <sbm/ParserOgre.h>
#include <sbm/ParserASFAMC.h>
#include <sbm/ParserFBX.h>
#include <sbm/lin_win.h>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif
#endif

#define DEBUG_LOAD_PATHS2 (0)

namespace SmartBody {

SBAssetManager::SBAssetManager()
{
	createDoubleAttribute("globalSkeletonScale", 1,true,"",30,false,false,false,"Multiplier when loading all skeletons. ");
	createDoubleAttribute("globalMotionScale", 1,true,"",30,false,false,false,"Multiplier when loading all motions.");
	
}

SBAssetManager::~SBAssetManager()
{
}


SBSkeleton* SBAssetManager::createSkeleton(const std::string& skeletonDefinition)
{
	SBSkeleton* skeleton = NULL;
	SBSkeleton* templateSkeleton = this->getSkeleton(skeletonDefinition);
	if (templateSkeleton)
	{
		skeleton = new SBSkeleton(templateSkeleton);
	}
	else
	{
		skeleton = new SBSkeleton(skeletonDefinition);
	}

	return skeleton;
	
}


SBSkeleton* SBAssetManager::getSkeleton(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SBSkeleton*>::iterator iter = mcu.skeleton_map.find(name);
	SkSkeleton* skskel = NULL;
	if (iter != mcuCBHandle::singleton().skeleton_map.end())
		skskel = iter->second;
	SBSkeleton* sbskel = dynamic_cast<SBSkeleton*>(skskel);
	return sbskel;
}


std::vector<std::string> SBAssetManager::getAssetPaths(const std::string& type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> list;
	srPathList* path = NULL;
	if (type == "seq" || type == "script")
	{
		path = &mcu.seq_paths;
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		path = &mcu.me_paths;
	}
	else if (type == "audio")
	{
		path = &mcu.audio_paths;
	}
	else if (type == "mesh")
	{
		path = &mcu.mesh_paths;
	}
	else
	{
		LOG("Unknown path type: %s", type.c_str());
		return list;
	}
	
	path->reset();
	std::string nextPath = path->next_path(false);
	while (nextPath != "")
	{
		list.push_back(nextPath);
		nextPath = path->next_path(false);
	}
	return list;
}

std::vector<std::string> SBAssetManager::getLocalAssetPaths(const std::string& type)
{

	std::string mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	boost::filesystem::path mpath(mediaPath);
	std::string completeMediaPath =  boost::filesystem::complete(mpath).string();
	size_t mediaPathLength = completeMediaPath.size();

	std::vector<std::string> paths = getAssetPaths(type);

	std::vector<std::string> localPaths;

	// remove the media path
	for (std::vector<std::string>::iterator iter = paths.begin();
		 iter != paths.end();
		 iter++)
	{
		boost::filesystem::path path((*iter));
		std::string completePath = boost::filesystem::complete( path ).string();
		size_t loc = completePath.find(completeMediaPath);
		if (loc == 0)
		{
			localPaths.push_back(completePath.substr(mediaPathLength + 1));
		}
		else
		{
			localPaths.push_back((*iter));
		}
	}

	return localPaths;

}

void SBAssetManager::addAssetPath(const std::string& type, const std::string& path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	
	if (type == "seq" || type == "script")
	{
		mcu.seq_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		mcu.me_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		mcu.audio_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "mesh")
	{
		mcu.mesh_paths.insert(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
}

void SBAssetManager::removeAssetPath(const std::string& type, const std::string& path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	bool ret = false;
	if (type == "seq" || type == "script")
	{
		ret = mcu.seq_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		ret = mcu.me_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		ret = mcu.audio_paths.remove(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}

	if (ret)
	{
		// remove the resource from the resource manager
	}
}

void SBAssetManager::removeAllAssetPaths(const std::string& type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	bool ret = false;
	if (type == "seq" || type == "script")
	{
		 mcu.seq_paths.removeAll();
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		mcu.me_paths.removeAll();
	}
	else if (type == "audio")
	{
		mcu.audio_paths.removeAll();
	}
	else if (type == "mesh")
	{
		mcu.mesh_paths.removeAll();
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}

	if (ret)
	{
		// remove the resource from the resource manager
	}
}

void SBAssetManager::loadAssets()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.me_paths.reset();

	std::string path = mcu.me_paths.next_path(false);
	while (path != "")
	{
		load_motions(path.c_str(), true);
		mcu.load_skeletons(path.c_str(), true);
		path = mcu.me_paths.next_path(false);
	}
}

void SBAssetManager::loadAsset(const std::string& assetPath)
{
	const std::string& mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	boost::filesystem::path p( mediaPath );
	boost::filesystem::path assetP( assetPath );

	boost::filesystem::path abs_p = boost::filesystem::complete( assetP );	

	if( boost::filesystem2::exists( abs_p ))
	{
		p = assetP;
	}
	else
	{
		p /= assetP;
	}
	boost::filesystem::path final = boost::filesystem::complete( p );
	std::string finalPath = p.string();

	// make sure the file exists and is readable
	std::ifstream file(finalPath.c_str());
	if (!file.good())
	{
		LOG("File %s cannot be read, asset will not be loaded.", finalPath.c_str());
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	std::string ext = boost::filesystem::extension( finalPath );
	std::string baseName = boost::filesystem::basename( finalPath );
	std::string fileName = baseName+ext;
	// determine the type of asset: skeleton, motion, mesh, texture, ...
	if( _stricmp( ext.c_str(), ".skm" ) == 0)
	{
		FILE* myfile = fopen(finalPath.c_str(), "rt");
		SrInput in( myfile );
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = motion->load( in, 1.0 );
		if (parseSuccessful)
			_motions.insert(std::pair<std::string, SBMotion*>(motion->getName(), motion));
		else
			delete motion;
		return;
	}

	if( _stricmp( ext.c_str(), ".bvh" ) == 0)
	{
		SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = ParserBVH::parse(*skeleton, *motion, finalPath, file, 1.0);
		if (parseSuccessful)
		{
			_motions.insert(std::pair<std::string, SBMotion*>(motion->getName(), motion));
			mcu.skeleton_map.insert(std::pair<std::string, SBSkeleton*>(skeleton->getName(), skeleton));
		}
		else
		{
			delete motion;
			delete skeleton;
		}
			
		return;
	}

	if( _stricmp( ext.c_str(), ".sk" ) == 0)
	{
		FILE* myfile = fopen(finalPath.c_str(), "rt");
		SrInput input(myfile);
		SmartBody::SBSkeleton* skeleton = new SmartBody::SBSkeleton();
		SkSkeleton* skel = skeleton;
		if( skel->loadSk( input, 1.0) )
		{
			skeleton->ref();
			skeleton->setFileName(fileName);
			skeleton->setName(skeleton->getFileName());
			mcu.skeleton_map.insert(std::pair<std::string, SBSkeleton*>(skel->getName(), skeleton));
		}
		else
		{
			delete skeleton;
		}
		return;
	}

	if( _stricmp( ext.c_str(), ".dae" ) == 0)
	{

	}

	if( _stricmp( ext.c_str(), ".xml" ) == 0)
	{

	}

	if( _stricmp( ext.c_str(), ".asf" ) == 0)
	{

	}
	if( _stricmp( ext.c_str(), ".amc" ) == 0)
	{

	}
}

void SBAssetManager::loadAssetsFromPath(const std::string& assetPath)
{
	load_motions(assetPath.c_str(), true);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.load_skeletons(assetPath.c_str(), true);
}

SBSkeleton* SBAssetManager::addSkeletonDefinition(const std::string& skelName )
{
	SBSkeleton* sbSkel = new SBSkeleton();	
	sbSkel->setName(skelName);
	sbSkel->skfilename(skelName.c_str());
	mcuCBHandle::singleton().skeleton_map.insert(std::pair<std::string, SBSkeleton*>(sbSkel->getName(), sbSkel));
	return sbSkel;
}

SBMotion* SBAssetManager::addMotionDefinition(const std::string& motionName, double duration )
{
	SBMotion* sbMotion = new SBMotion();
	if (duration > 0)
	{
		sbMotion->insert_frame(0,0.f);
		sbMotion->insert_frame(1,(float)duration);
	}	
	sbMotion->setName(motionName);	
	//mcuCBHandle::singleton().motion_map.insert(std::pair<std::string, SkMotion*>(motionName, sbMotion));
	_motions[motionName] = sbMotion;
	return sbMotion;
}

SBAPI void SBAssetManager::addMotion(SmartBody::SBMotion* motion)
{
	_motions[motion->getName()] = motion;
}

SBAPI void SBAssetManager::removeMotion(SmartBody::SBMotion* motion)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(motion->getName());
	if (iter != _motions.end())
	{
		_motions.erase(iter);
	}
	delete motion;
}


void SBAssetManager::addMotions(const std::string& path, bool recursive)
{
	load_motions(path.c_str(), recursive);
}

SBMotion* SBAssetManager::getMotion(const std::string& name)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(name);
	if (iter != _motions.end())
		return (*iter).second;
	else
		return NULL;
}

int SBAssetManager::getNumMotions()
{
	return _motions.size();
}

std::vector<std::string> SBAssetManager::getMotionNames()
{
	std::vector<std::string> ret;

	for(std::map<std::string, SBMotion*>::iterator iter = _motions.begin();
		iter != _motions.end();
		iter++)
	{
		SBMotion* motion = (*iter).second;
		ret.push_back(motion->getName());
	}

	return ret;
}

int SBAssetManager::getNumSkeletons()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.getSkeletonMap().size();
}


std::vector<std::string> SBAssetManager::getSkeletonNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::vector<std::string> ret;

	for(std::map<std::string, SBSkeleton*>::iterator iter = mcu.getSkeletonMap().begin();
		iter != mcu.getSkeletonMap().end();
		iter++)
	{
		SBSkeleton* skeleton = (*iter).second;
		ret.push_back(std::string(iter->first));
	}

	return ret;	
}

int SBAssetManager::load_me_motions( const char* pathname, std::map<std::string, SBMotion*>& map, bool recurse_dirs, double scale )
{
	boost::filesystem::path motions_path(pathname);
	
	boost::filesystem::path finalPath;

	std::string rootDir = motions_path.root_directory();
	if (rootDir.size() == 0)
	{	
		std::string mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
		finalPath = operator/(mediaPath, motions_path);
	}
	else
	{
		finalPath = pathname;
	}

	if (1) {
		return load_me_motions_impl( finalPath, map, recurse_dirs, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid motion path \"%s\".", finalPath.string().c_str());
		return CMD_FAILURE;
	}
}


int SBAssetManager::load_me_motions_impl( const boost::filesystem::path& pathname, std::map<std::string, SmartBody::SBMotion*>& map, bool recurse_dirs, double scale, const char* error_prefix )
{
	if( !boost::filesystem::exists( pathname ) ) {
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
		return CMD_FAILURE;
	}

	if( boost::filesystem::is_directory( pathname ) ) {
		// ignore any '.' diretories
		std::string filebase = pathname.leaf();
		if (filebase.find(".") == 0 && filebase.size() > 1)
		{
			// ignore hidden directories
			return CMD_SUCCESS;
		}
		LOG("Attempting to load motions from path '%s'...", pathname.string().c_str());	
	

		boost::filesystem::directory_iterator end;
		for( boost::filesystem::directory_iterator i( pathname ); i!=end; ++i ) {
			const boost::filesystem::path& cur = *i;

			if( boost::filesystem::is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_motions_impl( cur, map, recurse_dirs, scale, "WARNING: " );
			} else {
				std::string ext = boost::filesystem::extension( cur );
#if ENABLE_FBX_PARSER
				if( _stricmp( ext.c_str(), ".skm" ) == 0 || 
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".amc" ) == 0 ||
					_stricmp( ext.c_str(), ".xml" ) == 0 ||
					_stricmp( ext.c_str(), ".fbx" ) == 0)
#else
				if( _stricmp( ext.c_str(), ".skm" ) == 0 || 
					_stricmp( ext.c_str(), ".bvh" ) == 0 ||
					_stricmp( ext.c_str(), ".dae" ) == 0 ||
					_stricmp( ext.c_str(), ".xml" ) == 0 ||
					_stricmp( ext.c_str(), ".amc" ) == 0)
#endif
				{
					load_me_motions_impl( cur, map, recurse_dirs, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS2 ) {
					LOG("DEBUG: load_me_motion_impl(): Skipping \"%s\".  Extension \"%s\" does not match MOTION_EXT.", cur.string().c_str(), ext.c_str() );
				}
			}
		}
	} else {

		std::vector<SBMotion*> motions;
		std::string ext = boost::filesystem::extension( pathname );
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = false;

		if (ext == ".skm" || ext == ".SKM")
		{
			SrInput in( pathname.string().c_str(), "rt" );
			SrString fullin_string;
			in.getall( fullin_string );
			SrInput fullin( (const char *)fullin_string );
			fullin.filename( pathname.string().c_str() ); // copy filename for error message
			
			parseSuccessful = motion->load( fullin, scale );
			if (parseSuccessful)
				motions.push_back(motion);
		}
		else if (ext == ".bvh" || ext == ".BVH")
		{
			std::ifstream filestream( pathname.string().c_str() );
			
			SkSkeleton skeleton;
			parseSuccessful = ParserBVH::parse(skeleton, *motion, pathname.string(), filestream, float(scale));
			if (parseSuccessful)
				motions.push_back(motion);
		}
		else if (ext == ".dae" || ext == ".DAE")
		{			
			SBSkeleton skeleton;
			parseSuccessful = ParserOpenCOLLADA::parse(skeleton, *motion, pathname.string(), float(scale), true, true);		
			// now there's adjust for the channels by default
			//animationPostProcessByChannels(skeleton, motion, channelsForAdjusting);
			SmartBody::SBMotion* sbMotion = dynamic_cast<SmartBody::SBMotion*>(motion);
			int pretrimFrames = SmartBody::SBScene::getScene()->getIntAttribute("colladaTrimFrames");
			if (pretrimFrames > 0 && sbMotion)
			{
				sbMotion->trim(pretrimFrames,0);				
			}
			if (parseSuccessful)
				motions.push_back(motion);

		}
		else if (ext == ".xml" || ext == ".XML")
		{			
			SBSkeleton skeleton;
			parseSuccessful = ParserOgre::parse(skeleton, motions, pathname.string(), float(scale), true, true);			
		}
		else if (ext == ".amc" || ext == ".AMC")
		{
			// at the same directory, looking for one asf file
			std::string asf = "";
			boost::filesystem::directory_iterator end;
			std::string filebase = boost::filesystem::basename(pathname);
			std::string fileext = boost::filesystem::extension(pathname);
			int dirSize = pathname.string().size() - filebase.size() - fileext.size() - 1;
			std::string directory = pathname.string().substr(0, dirSize);
			for( boost::filesystem::directory_iterator i( directory ); i!=end; ++i ) 
			{
				const boost::filesystem::path& cur = *i;
				if (!boost::filesystem::is_directory(cur)) 
				{
					std::string ext = boost::filesystem::extension(cur);
					if (_stricmp(ext.c_str(), ".asf") == 0)
					{
						asf = cur.string().c_str();
						break;
					}
				}
			}
			std::ifstream metafilestream(asf.c_str());
			std::ifstream filestream(pathname.string().c_str());
			SBSkeleton skeleton;
			parseSuccessful = ParserASFAMC::parse(skeleton, *motion, metafilestream, filestream, float(scale));
			motion->setName(filebase.c_str());
			if (parseSuccessful)
				motions.push_back(motion);
		}
#if ENABLE_FBX_PARSER
		else if (ext == ".fbx" || ext == ".FBX")
		{
			SkSkeleton skeleton;
			LOG("FBX motion parse: %s", pathname.string().c_str());
			parseSuccessful = ParserFBX::parse(skeleton, *motion, pathname.string(), float(scale));	
			if (parseSuccessful)
				motions.push_back(motion);
		}
#endif
		if (parseSuccessful)
		{
			// register the motion
			//motion->registerAnimation();

			for (std::vector<SBMotion*>::iterator iter = motions.begin();
				 iter != motions.end();
				 iter++)
			{
				SBMotion* motion = (*iter);
				char CurrentPath[_MAX_PATH];
#ifdef WIN32
				_getcwd(CurrentPath, _MAX_PATH);
#else
				getcwd(CurrentPath, _MAX_PATH);
#endif
				std::string filename;
			
				filename = pathname.filename().c_str();
			
				//filename = mcn_return_full_filename_func( CurrentPath, finalPath.string().c_str() );

				std::string filebase = boost::filesystem::basename( pathname );
				const char* name = motion->getName().c_str();
				if( name && _stricmp( filebase.c_str(), name ) ) {
					LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, pathname.native_file_string().c_str(), filebase.c_str());
					//motion->setName( filebase.c_str() );
				}
				motion->filename( pathname.native_file_string().c_str() );

				SBMotion* existingMotion = getMotion(filebase);
				if (existingMotion)
				{
					LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
					delete motion;
					return CMD_FAILURE;
				}
				addMotion(motion);
			}
			
		} else {
			// SkMotion::load() already prints an error...
			//strstr << error_prefix << "Failed to load motion \"" << pathname.string() << "\"." << endl;
			delete motion;
			return CMD_FAILURE;
		}
		
	}
	return CMD_SUCCESS;
}

int SBAssetManager::load_me_motion_individual( SrInput & input, const std::string & motionName, std::map<std::string, SBMotion*>& map, double scale )
{
	SBMotion* motion = new SmartBody::SBMotion();

	bool parseSuccessful = motion->load( input, scale );

	std::string filename = motionName;

	std::string filebase = boost::filesystem::basename( motionName );
	const char* name = motion->getName().c_str();
	if( name && _stricmp( filebase.c_str(), name ) )
	{
		LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, motionName.c_str(), filebase.c_str());
		motion->setName( filebase.c_str() );
	}

	motion->filename( motionName.c_str() );

	std::map<std::string, SBMotion*>::iterator motionIter = map.find(filebase);
	if (motionIter != map.end()) 
	{
		LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), motionName.c_str());
		delete motion;
		return CMD_FAILURE;
	}

	map.insert(std::pair<std::string, SBMotion*>(filebase, motion));

	return CMD_SUCCESS;
}

int SBAssetManager::load_motions( const char* pathname, bool recursive )
{
	double scale = getDoubleAttribute("globalMotionScale");
	return load_me_motions( pathname, _motions, recursive, scale );
}

int SBAssetManager::load_motion( const void* data, int sizeBytes, const char* motionName )
{
	double scale = getDoubleAttribute("globalSkeletonScale");
	SrInput input( (char *)data, sizeBytes );
	return load_me_motion_individual( input, motionName, _motions, scale );
}
}
