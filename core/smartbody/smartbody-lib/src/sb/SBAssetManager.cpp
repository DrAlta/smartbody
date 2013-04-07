#include "SBAssetManager.h"
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>

#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <sbm/ParserBVH.h>
#include <sbm/ParserOpenCOLLADA.h>
#include <sbm/ParserOgre.h>
#include <sbm/ParserASFAMC.h>
#include <sbm/ParserFBX.h>
#include <sbm/lin_win.h>
#include <sbm/sr_path_list.h>
#include <sbm/sbm_constants.h>

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
	seq_paths = new srPathList();	
	me_paths = new srPathList();	
	audio_paths = new srPathList();
	mesh_paths = new srPathList();

	createDoubleAttribute("globalSkeletonScale", 1,true,"",30,false,false,false,"Multiplier when loading all skeletons. ");
	createDoubleAttribute("globalMotionScale", 1,true,"",30,false,false,false,"Multiplier when loading all motions.");
	
}

SBAssetManager::~SBAssetManager()
{
	for (std::map<std::string, SBMotion*>::iterator iter = _motions.begin();
		 iter != _motions.end();
		 iter++)
	{
		delete (*iter).second;
	}

	for (std::map<std::string, SBSkeleton*>::iterator iter = _skeletons.begin();
		iter != _skeletons.end();
		iter++)
	{
		delete (*iter).second;
	}

	delete seq_paths;
	delete me_paths;
	delete audio_paths;
	delete mesh_paths;

	_deformableMeshMap.clear();
}

double SBAssetManager::getGlobalMotionScale()
{
	return getDoubleAttribute("globalMotionScale");
}

void SBAssetManager::setGlobalMotionScale(double val)
{
	setDoubleAttribute("globalMotionScale", val);
}

double SBAssetManager::getGlobalSkeletonScale()
{
	return getDoubleAttribute("globalSkeletonScale");
}

void SBAssetManager::setGlobalSkeletonScale(double val)
{
	setDoubleAttribute("globalSkeletonScale", val);
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
	
	std::map<std::string, SBSkeleton*>::iterator iter = _skeletons.find(name);
	SBSkeleton* sbskel = NULL;
	if (iter != _skeletons.end())
		sbskel = iter->second;
	return sbskel;
}


std::vector<std::string> SBAssetManager::getAssetPaths(const std::string& type)
{
	std::vector<std::string> list;
	srPathList* path = NULL;
	if (type == "seq" || type == "script")
	{
		path = seq_paths;
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		path = me_paths;
	}
	else if (type == "audio")
	{
		path = audio_paths;
	}
	else if (type == "mesh")
	{
		path = mesh_paths;
	}
	else
	{
		LOG("Unknown path type: %s", type.c_str());
		return list;
	}
	
	path->reset();
	std::string nextPath = path->next_path(true);
	while (nextPath != "")
	{
		list.push_back(nextPath);
		nextPath = path->next_path(false);
	}
	return list;
}

std::vector<std::string> SBAssetManager::getLocalAssetPaths(const std::string& type)
{

	std::vector<std::string> list;
	srPathList* path = NULL;
	if (type == "seq" || type == "script")
	{
		path = seq_paths;
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		path = me_paths;
	}
	else if (type == "audio")
	{
		path = audio_paths;
	}
	else if (type == "mesh")
	{
		path = mesh_paths;
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

void SBAssetManager::addAssetPath(const std::string& type, const std::string& path)
{
	if (type == "seq" || type == "script")
	{
		seq_paths->insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		me_paths->insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		audio_paths->insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "mesh")
	{
		mesh_paths->insert(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
}

void SBAssetManager::removeAssetPath(const std::string& type, const std::string& path)
{
	if (type == "seq" || type == "script")
	{
		seq_paths->remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		me_paths->remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		audio_paths->remove(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
}

void SBAssetManager::removeAllAssetPaths(const std::string& type)
{
	if (type == "seq" || type == "script")
	{
		seq_paths->removeAll();
	}
	else if (type == "me" || type == "ME" || type == "motion")
	{
		me_paths->removeAll();
	}
	else if (type == "audio")
	{
		audio_paths->removeAll();
	}
	else if (type == "mesh")
	{
		mesh_paths->removeAll();
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
}

void SBAssetManager::loadAssets()
{
	me_paths->reset();

	std::string path = me_paths->next_path(false);
	while (path != "")
	{
		load_motions(path.c_str(), true);
		load_skeletons(path.c_str(), true);
		path = me_paths->next_path(false);
	}
}

void SBAssetManager::loadAsset(const std::string& assetPath)
{
	const std::string& mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	boost::filesystem::path p( mediaPath );
	boost::filesystem::path assetP( assetPath );

#if (BOOST_VERSION > 104400)
	boost::filesystem::path abs_p = boost::filesystem::absolute( assetP );	
	if( boost::filesystem::exists( abs_p ))
	{
		p = assetP;
	}
	else
	{
		p /= assetP;
	}
	boost::filesystem::path final = boost::filesystem::absolute( p );

#else
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

#endif

		std::string finalPath = p.string();

	// make sure the file exists and is readable
	std::ifstream file(finalPath.c_str());
	if (!file.good())
	{
		LOG("File %s cannot be read, asset will not be loaded.", finalPath.c_str());
		return;
	}

	 

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
			_skeletons.insert(std::pair<std::string, SBSkeleton*>(skeleton->getName(), skeleton));
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
			_skeletons.insert(std::pair<std::string, SBSkeleton*>(skel->getName(), skeleton));
		}
		else
		{
			delete skeleton;
		}
		return;
	}

	if( _stricmp( ext.c_str(), ".dae" ) == 0)
	{
		SmartBody::SBSkeleton*skeleton =  new SmartBody::SBSkeleton();					
		SkMotion motion;
		bool ok = ParserOpenCOLLADA::parse(*skeleton, motion, finalPath, 1.f, true, false);
		if (ok)
		{
			std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = _skeletons.find(fileName);
			if (motionIter != _skeletons.end()) {
				LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", fileName.c_str(), finalPath.c_str());
				delete skeleton;
				return;
			}
			skeleton->ref();
			skeleton->skfilename(finalPath.c_str());				
			skeleton->setName(fileName.c_str());
			_skeletons.insert(std::pair<std::string, SBSkeleton*>(skeleton->getName(), skeleton));
		}
		else
		{
			LOG("Problem loading skeleton from file '%s'.", finalPath.c_str());
			return;
		}
	}

	if( _stricmp( ext.c_str(), ".xml" ) == 0)
	{
		SmartBody::SBSkeleton*skeleton =  new SmartBody::SBSkeleton();					
		std::vector<SmartBody::SBMotion*> motions;
		bool ok = ParserOgre::parse(*skeleton,motions, finalPath, 1.f, true, false);		
		if (ok)
		{
			std::map<std::string, SmartBody::SBSkeleton*>::iterator motionIter = _skeletons.find(fileName);
			if (motionIter != _skeletons.end()) {
				LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", fileName.c_str(), finalPath.c_str());
				delete skeleton;
				return;
			}
			skeleton->ref();
			skeleton->skfilename(finalPath.c_str());				
			skeleton->setName(fileName.c_str());
			_skeletons.insert(std::pair<std::string, SBSkeleton*>(skeleton->getName(), skeleton));
		}
		else
		{
			LOG("Problem loading skeleton from file '%s'.", finalPath.c_str());
			return;
		}
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
	load_skeletons(assetPath.c_str(), true);
}

SBSkeleton* SBAssetManager::addSkeletonDefinition(const std::string& skelName )
{
	SBSkeleton* sbSkel = new SBSkeleton();	
	sbSkel->setName(skelName);
	sbSkel->skfilename(skelName.c_str());
	_skeletons.insert(std::pair<std::string, SBSkeleton*>(sbSkel->getName(), sbSkel));
	return sbSkel;
}

SBAPI void SBAssetManager::addSkeleton(SmartBody::SBSkeleton* skeleton)
{
	_skeletons[skeleton->getName()] = skeleton;
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
	return _skeletons.size();
}


std::vector<std::string> SBAssetManager::getSkeletonNames()
{
	std::vector<std::string> ret;

	for(std::map<std::string, SBSkeleton*>::iterator iter = _skeletons.begin();
		iter != _skeletons.end();
		iter++)
	{
		ret.push_back(std::string(iter->first));
	}

	return ret;	
}

int SBAssetManager::load_me_motions( const char* pathname, std::map<std::string, SBMotion*>& map, bool recurse_dirs, double scale )
{
	boost::filesystem::path motions_path(pathname);
	
	boost::filesystem::path finalPath;

#if (BOOST_VERSION > 104400)
	std::string rootDir = motions_path.root_directory().string();
#else
	std::string rootDir = motions_path.root_directory();
#endif
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
	if( !boost::filesystem::exists( pathname ) )
	{
#if (BOOST_VERSION > 104400)
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.string().c_str());
#else
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
#endif
		return CMD_FAILURE;
	}

	if( boost::filesystem::is_directory( pathname ) )
	{
		// ignore any '.' diretories
#if (BOOST_VERSION > 104400)
		std::string filebase = pathname.leaf().string();
#else
		std::string filebase = pathname.leaf();
#endif
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
				if( name && _stricmp( filebase.c_str(), name ) )
				{
#if (BOOST_VERSION > 104400)
					LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, pathname.string().c_str(), filebase.c_str());
#else
					LOG("WARNING: Motion name \"%s\" does not equal base of filename '%s'. Using '%s' in posture map.", name, pathname.native_file_string().c_str(), filebase.c_str());
#endif
					//motion->setName( filebase.c_str() );
				}
#if (BOOST_VERSION > 104400)
				motion->filename( pathname.string().c_str() );
				SBMotion* existingMotion = getMotion(filebase);
				if (existingMotion)
				{
					LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
#else
				motion->filename( pathname.native_file_string().c_str() );
				SBMotion* existingMotion = getMotion(filebase);
				if (existingMotion)
				{
					LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
#endif

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
	// SrInput requires a data stream that's null terminated (uses strlen() to figure out length).  So, allocate a new buffer and make sure to null-terminate it.
	// this is inefficient, but safer than assuming the user has given us valid data.
	char * dataCopy = new char [sizeBytes + 1];
	memcpy( dataCopy, data, sizeBytes );
	dataCopy[ sizeBytes ] = 0;
	SrInput input( dataCopy );
	input.filename(motionName);  // to prevent crashes in debug logs that blindy call filename()
	double scale = getDoubleAttribute("globalSkeletonScale");
	int ret = load_me_motion_individual( input, motionName, _motions, scale );
	delete [] dataCopy;
	return ret;
}

int SBAssetManager::load_skeletons( const char* pathname, bool recursive ) {
	return load_me_skeletons( pathname, _skeletons, recursive, SmartBody::SBScene::getScene()->getAssetManager()->getGlobalSkeletonScale() );
}

int SBAssetManager::load_skeleton( const void* data, int sizeBytes, const char* skeletonName )
{
	// SrInput requires a data stream that's null terminated (uses strlen() to figure out length).  So, allocate a new buffer and make sure to null-terminate it.
	// this is inefficient, but safer than assuming the user has given us valid data.
	char * dataCopy = new char [sizeBytes + 1];
	memcpy( dataCopy, data, sizeBytes );
	dataCopy[ sizeBytes ] = 0;
	SrInput input( dataCopy );
	input.filename(skeletonName);  // to prevent crashes in debug logs that blindy call filename()
	int ret = load_me_skeleton_individual( input, skeletonName, _skeletons, SmartBody::SBScene::getScene()->getAssetManager()->getGlobalSkeletonScale() );
	delete [] dataCopy;
	return ret;
}

SmartBody::SBSkeleton* SBAssetManager::load_skeleton( const char *skel_file, srPathList &path_list, double scale ) {
	
	
	std::map<std::string, SmartBody::SBSkeleton*>::iterator iter =_skeletons.find(std::string(skel_file));
	if (iter != _skeletons.end())
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
#if (BOOST_VERSION > 104400)
	boost::filesystem::path abs_p = boost::filesystem::absolute( p );
    if ( boost::filesystem::exists( abs_p ) )	{
#else
	boost::filesystem::path abs_p = boost::filesystem::complete( p );
    if ( boost::filesystem2::exists( abs_p ) )	{
#endif
//		sprintf( full_filename, "%s", abs_p.string().c_str() );
		
	}
	else	{
		LOG( "load_skeleton ERR: path '%s' does not exist\n", abs_p.string().c_str() );
	}
	// SUCCESS
	return skeleton_p;

}



int SBAssetManager::load_me_skeletons_impl( const boost::filesystem::path& pathname, std::map<std::string, SmartBody::SBSkeleton*>& map, bool recurse_dirs, double scale, const char* error_prefix )
{
		
	if( !exists( pathname ) ) {
#if (BOOST_VERSION > 104400)
		LOG("%s Skeleton path \"%s\" not found.", error_prefix,  pathname.string().c_str());
#else
		LOG("%s Skeleton path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
#endif
		return CMD_FAILURE;
	}

	if( is_directory( pathname ) ) {

		boost::filesystem::directory_iterator end;
		for( boost::filesystem::directory_iterator i( pathname ); i!=end; ++i ) {
			const boost::filesystem::path& cur = *i;

			if( boost::filesystem::is_directory( cur ) ) {
				if( recurse_dirs )
					load_me_skeletons_impl( cur, map, recurse_dirs, scale, "WARNING: " );
			} else {
				std::string ext = boost::filesystem::extension( cur );
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
				else if( DEBUG_LOAD_PATHS2 ) {
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
#if (BOOST_VERSION > 104400)

					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
#else
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
#endif
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
#if (BOOST_VERSION > 104400)
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
#else
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
#endif
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
#if (BOOST_VERSION > 104400)
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
#else
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
#endif
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
#if (BOOST_VERSION > 104400)
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
#else
					LOG("ERROR: Skeleton by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.native_file_string().c_str());
#endif
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
		//skeleton->setName(skeleton->skfilename());
		
	}
	return CMD_SUCCESS;
}




int SBAssetManager::load_me_skeleton_individual( SrInput & input, const std::string & skeletonName, std::map<std::string, SmartBody::SBSkeleton*>& map, double scale )
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

int SBAssetManager::load_me_skeletons( const char* pathname, std::map<std::string, SmartBody::SBSkeleton*>& map, bool recurse_dirs, double scale ) {
	boost::filesystem::path motions_path(pathname);
	
	boost::filesystem::path finalPath;
	// include the media path in the pathname if applicable
	
#if (BOOST_VERSION > 104400)
	std::string rootDir = motions_path.root_directory().string();
#else
	std::string rootDir = motions_path.root_directory();
#endif
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

FILE* SBAssetManager::open_sequence_file( const char *seq_name, std::string& fullPath ) {

	FILE* file_p = NULL;

	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];	
	// add the .seq extension if necessary
	std::string candidateSeqName = seq_name;
	if (candidateSeqName.find(".seq") == std::string::npos)
	{
		candidateSeqName.append(".seq");
	}
	sprintf( label, "%s", candidateSeqName.c_str());
	// current path containing .exe
	char CurrentPath[_MAX_PATH];
	_getcwd(CurrentPath, _MAX_PATH);

	seq_paths->reset();
	std::string filename = seq_paths->next_filename( buffer, candidateSeqName.c_str() );
	//filename = mcn_return_full_filename_func( CurrentPath, filename );
	//LOG("seq name = %s, filename = %s\n",seq_name,filename.c_str());
	
	while(filename.size() > 0)	{
		file_p = fopen( filename.c_str(), "r" );
		if( file_p != NULL ) {
	
			fullPath = filename;			
			break;
		}
		filename = seq_paths->next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension	
		sprintf( label, "%s.seq", seq_name );
		seq_paths->reset();
		filename = seq_paths->next_filename( buffer, candidateSeqName.c_str() );
		//filename = mcn_return_full_filename_func( CurrentPath, filename );
		while( filename.size() > 0 )	{
			if( ( file_p = fopen( filename.c_str(), "r" ) ) != NULL ) {
				
				fullPath = filename;
				break;
			}
			filename = seq_paths->next_filename( buffer, candidateSeqName.c_str() );
			//filename = mcn_return_full_filename_func( CurrentPath, filename );
		}
	}

	// return empty string if file not found
	return file_p;
}

const std::string SBAssetManager::findFileName(const std::string& type, const std::string& filename)
{
	srPathList* path = NULL;
	if (type == "script")
	{
		path = seq_paths;
	}
	else if (type == "motion")
	{
		path = me_paths;
	}
	else if (type == "mesh")
	{
		path = mesh_paths;
	}
	else if (type == "audio")
	{
		path = audio_paths;
	}
	else
	{
		LOG("findFileName(): type name needs to be 'script', 'motion', 'mesh' or 'audio'");
		return "";
	}

	char buffer[ MAX_FILENAME_LEN ];

	path->reset();
	std::string curFilename = path->next_filename( buffer, filename.c_str() );
	while (curFilename.size() > 0)
	{
		FILE* file = fopen(curFilename.c_str(), "r");
		if (file)
		{
			fclose(file);
			return curFilename;
		}
		else
		{
			curFilename = path->next_filename( buffer, filename.c_str() );
		}
	}

	return "";
}


void SBAssetManager::addDeformableMesh(const std::string& meshName, DeformableMesh* mesh)
{
	_deformableMeshMap[meshName] = mesh;
}

void SBAssetManager::removeDeformableMesh(const std::string& meshName)
{
	std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.find(meshName);
	if (iter != _deformableMeshMap.end())
	{
		_deformableMeshMap.erase(iter);
	}
}

DeformableMesh* SBAssetManager::getDeformableMesh(const std::string& meshName)
{
	std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.find(meshName);
	if (iter != _deformableMeshMap.end())
	{
		return (*iter).second;
	}

	return NULL;
}

void SBAssetManager::removeAllDeformableMeshes()
{
	_deformableMeshMap.clear();
}

}
