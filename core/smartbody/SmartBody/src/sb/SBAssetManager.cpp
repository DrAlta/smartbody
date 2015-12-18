#include "SBAssetManager.h"
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <sb/SBScene.h>
#include <sb/SBSceneListener.h>
#include <sb/SBSkeleton.h>
#include <sb/SBNavigationMesh.h>
#include <sb/SBAssetHandlerSkm.h>
#include <sb/SBAssetHandlerSk.h>
#include <sb/SBAssetHandlerCOLLADA.h>
#include <sb/SBAssetHandlerAsf.h>
#include <sb/SBAssetHandlerOgre.h>
#include <sb/SBAssetHandlerObj.h>
#include <sb/SBAssetHandlerSkmb.h>
#include <sb/SBAssetHandlerBvh.h>
#include <sb/SBAssetHandlerAmc.h>
#include <sb/SBAssetHandlerPly.h>
#include <sb/SBAssetHandlerSBMeshBinary.h>
#ifndef SB_NO_ASSIMP
#include <sb/SBAssetHandlerAssimp.h>
#endif
#ifdef USE_FBX_PARSER
#include <sb/SBAssetHandlerFbx.h>
#endif
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <sbm/ParserBVH.h>
#include <sbm/ParserOpenCOLLADA.h>
#include <sbm/ParserCOLLADAFast.h>
#include <sbm/ParserOgre.h>
#include <sbm/ParserASFAMC.h>
#include <sbm/ParserBVH.h>
#include <sbm/lin_win.h>
#include <sbm/sr_path_list.h>
#include <sbm/sbm_constants.h>
#include <sbm/GPU/SbmTexture.h>
#include <external/soil/SOIL.h>

#include <boost/filesystem/path.hpp>

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

	addAssetHandler(new SBAssetHandlerSkm());
	addAssetHandler(new SBAssetHandlerSk());	
	addAssetHandler(new SBAssetHandlerCOLLADA());	
	addAssetHandler(new SBAssetHandlerAsf());	
	addAssetHandler(new SBAssetHandlerAmc());	
	addAssetHandler(new SBAssetHandlerOgre());	
	addAssetHandler(new SBAssetHandlerObj());	
	addAssetHandler(new SBAssetHandlerPly());	
	addAssetHandler(new SBAssetHandlerSkmb());	
	addAssetHandler(new SBAssetHandlerBvh());	
	addAssetHandler(new SBAssetHandlerSBMeshBinary());
#ifndef SB_NO_ASSIMP
	addAssetHandler(new SBAssetHandlerAssimp());	
#endif
#ifdef USE_FBX_PARSER
	addAssetHandler(new SBAssetHandlerFbx());
#endif
	uniqueSkeletonId = 0;

	_motionCounter = 0;
	_skeletonCounter = 0;
	_meshCounter = 0;
}

SBAssetManager::~SBAssetManager()
{
	removeAllAssets();
	delete seq_paths;
	delete me_paths;
	delete audio_paths;
	delete mesh_paths;	

	std::vector<SBAssetHandler*>& handlerList = getAssetHandlers();
	for (unsigned int i=0;i<handlerList.size();i++)
		delete handlerList[i];
	handlerList.clear();
	_assetHandlerMap.clear();
}


SBAPI void SBAssetManager::removeAllAssets()
{
	for (std::map<std::string, SBMotion*>::iterator iter = _motions.begin();
		iter != _motions.end();
		iter++)
	{
		delete (*iter).second;
	}
	_motions.clear();

	for (std::map<std::string, SBSkeleton*>::iterator iter = _skeletons.begin();
		iter != _skeletons.end();
		iter++)
	{
		delete (*iter).second;
	}
	_skeletons.clear();

	for (std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.begin();
		iter != _deformableMeshMap.end();
		iter++)
	{
		delete (*iter).second;
	}
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
	if (skeletonDefinition == "")
	{
		SBSkeleton* skeleton = new SBSkeleton();
		std::stringstream strstr;
		strstr << "skeleton" << uniqueSkeletonId;
		uniqueSkeletonId++;
		skeleton->setName(strstr.str());
		skeleton->skfilename(strstr.str().c_str());
		_skeletons[strstr.str()] = skeleton;

		return skeleton;

	}

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
		nextPath = path->next_path(true);
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

std::string SBAssetManager::findAsset(const std::string& type, const std::string& assetName)
{
	// load the assets from the mesh directories
	std::vector<std::string> meshPaths = this->getAssetPaths(type);
	for (size_t m = 0; m < meshPaths.size(); m++)
	{
		std::string location = this->findAssetFromLocation(meshPaths[m], assetName);
		if (location != "")
			return location;
	}
	return "";
}

std::string SBAssetManager::findAssetFromLocation(const std::string& filepath, const std::string& assetName)
{
	boost::filesystem::path path(filepath);

	const std::string& mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	if (mediaPath.size() > 0)
	{
		// if the path already contains the media path, ignore it
		if (filepath.find(mediaPath) == 0)
		{
			 // do nothing
		}
		else
		{
			if (boost::filesystem::exists(path))
			{
				// do nothing
			}
			else
			{
				boost::filesystem::path finalPath(mediaPath);
				finalPath /= path;
				if (boost::filesystem::exists(finalPath))
					path = finalPath;
				else
				{
					LOG("Could not load assets from %s, does not exist", finalPath.string().c_str());
					return "";
				}
			}
		}

	}

	if (boost::filesystem::is_regular_file(path))
	{
		std::string base = boost::filesystem::basename(path);
		std::string ext = boost::filesystem::basename(path);
		if (assetName == base + ext)
			return filepath;
	}

	std::vector<boost::filesystem::path> dirs;
	dirs.push_back(path);
	while (dirs.size() > 0)
	{
		boost::filesystem::path curPath = dirs[0];
		dirs.erase(dirs.begin());
		for (boost::filesystem::directory_iterator it(curPath), eit; it != eit; ++it)
		{
			if (boost::filesystem::is_directory(it->path()))
			{
				// ignore directories that start with a '.'
				std::string basename = boost::filesystem::basename(it->path());
				std::string extension = boost::filesystem::extension(it->path());
				if (basename.size() == 0 && extension.find(".") == 0)
					continue;
				dirs.push_back(it->path());
			}
			else if (boost::filesystem::is_regular_file(it->path()))
			{
				std::string base = boost::filesystem::basename(it->path());
				std::string ext = boost::filesystem::extension(it->path());
				if (assetName == base + ext)
					return filepath;
			} 
			/*
			else if (boost::filesystem::is_symlink(p))
			{
				loadAsset(p.string());
			}
			*/
		}
	}
	for (size_t d = 0; d < dirs.size(); d++)
	{
		std::string location = findAssetFromLocation(dirs[d].string(), assetName);
		if (location != "")
			return location;
	}
	return "";
}

void SBAssetManager::loadAssets()
{
	me_paths->reset();

	std::string path = me_paths->next_path(true);
	while (path != "")
	{
		loadAssetsFromPath(path);
//		load_motions(path.c_str(), true);
//		load_skeletons(path.c_str(), true);
		path = me_paths->next_path(true);
	}
}

std::vector<SBAsset*> SBAssetManager::loadAsset(const std::string& assetPath)
{
	//LOG("Loading asset [%s]", assetPath.c_str());
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	const std::string& mediaPath = scene->getMediaPath();
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


	std::vector<SBAsset*> allAssets;
	
	//LOG("Asset loading from final path = %s", finalPath.c_str());

	// make sure the file exists and is readable
	std::ifstream file(finalPath.c_str());
	if (!file.good())
	{
		LOG("File %s cannot be read, asset will not be loaded.", finalPath.c_str());
		return allAssets;
	}

	if (boost::filesystem::is_directory(p))
	{
		LOG("%s is a directory, cannot load asset.", finalPath.c_str());
		return allAssets;
	}

	 

	std::string ext = boost::filesystem::extension( finalPath );
	std::string baseName = boost::filesystem::basename( finalPath );
	std::string fileName = baseName+ext;

	std::string extNoDot = "";
	if (ext.size() > 0)
		extNoDot = ext.substr(1);

	// get all the asset handlers for that extension
	const std::vector<SBAssetHandler*>& assetHandlers = this->getAssetHandlers(extNoDot);


	for (size_t h = 0; h < assetHandlers.size(); h++)
	{
		SBAssetHandler* assetHandler = assetHandlers[h];
		std::vector<SBAsset*> assets = assetHandler->getAssets(finalPath);
		for (size_t a = 0; a < assets.size(); a++)
		{
			assets[a]->setFullFilePath(finalPath);
			allAssets.push_back(assets[a]);
		}
	}

	// place the assets in their proper place
	for (size_t x = 0; x < allAssets.size(); x++)
	{
		SBAsset* asset = allAssets[x];
		SmartBody::SBMotion* motion = dynamic_cast<SmartBody::SBMotion*>(asset);
		if (motion)
		{
			SBMotion* existingMotion = this->getMotion(motion->getName());
			if (existingMotion)
			{
				std::string name = this->getAssetNameVariation(existingMotion);
				LOG("Motion named %s already exist, changing name to %s", motion->getName().c_str(), name.c_str());
				motion->setName(name);
			}
			this->addMotion(motion);
			addAssetHistory("MOTION " + motion->getName());
			continue;
		}
		SmartBody::SBSkeleton* skeleton = dynamic_cast<SmartBody::SBSkeleton*>(asset);
		if (skeleton)
		{
			SBSkeleton* existingSkeleton = this->getSkeleton(skeleton->getName());
			if (existingSkeleton)
			{
				std::string name = this->getAssetNameVariation(existingSkeleton);
				LOG("Skeleton named %s already exist, changing name to %s", skeleton->getName().c_str(), name.c_str());
				skeleton->setName(name);
			}
			this->addSkeleton(skeleton);
			skeleton->ref();
			addAssetHistory("SKELETON " + skeleton->getName());
			continue;
		}
		DeformableMesh* mesh = dynamic_cast<DeformableMesh*>(asset);
		if (mesh)
		{
			DeformableMesh* existingMesh = this->getDeformableMesh(mesh->getName());
			if (existingMesh)
			{
				std::string name = this->getAssetNameVariation(existingMesh);
				LOG("Mesh named %s already exist, changing name to %s", existingMesh->getName().c_str(), name.c_str());
				existingMesh->setName(name);
			}
			this->addMesh(mesh);
			addAssetHistory("MESH " + mesh->getName());
			continue;
		}
		LOG("Unknown asset type for file %s", assetPath.c_str());


	}

	return allAssets;
}

void SBAssetManager::loadAssetsFromPath(const std::string& assetPath)
{
	LOG("Loading [%s]", assetPath.c_str());

	boost::filesystem::path path(assetPath);

	const std::string& mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	if (mediaPath.size() > 0)
	{
		// if the path already contains the media path, ignore it
		if (assetPath.find(mediaPath) == 0)
		{
			 // do nothing
		}
		else
		{
			if (boost::filesystem::exists(path))
			{
				// do nothing
			}
			else
			{
				boost::filesystem::path finalPath(mediaPath);
				finalPath /= path;
				if (boost::filesystem::exists(finalPath))
					path = finalPath;
				else
				{
					LOG("Could not load assets from %s, does not exist", finalPath.string().c_str());
					return;
				}
			}
		}

	}
	std::vector<boost::filesystem::path> dirs;

	if (boost::filesystem::is_directory(path))
	{
		dirs.push_back(path);
		while (dirs.size() > 0)
		{
			boost::filesystem::path curPath = dirs[0];
			dirs.erase(dirs.begin());
			for (boost::filesystem::directory_iterator it(curPath), eit; it != eit; ++it)
			{
				if (boost::filesystem::is_directory(it->path()))
				{
					// ignore directories that start with a '.'
					std::string basename = boost::filesystem::basename(it->path());
					std::string extension = boost::filesystem::extension(it->path());
					if (basename.size() == 0 && extension.find(".") == 0)
						continue;
					dirs.push_back(it->path());
				}
				else if (boost::filesystem::is_regular_file(it->path()))
				{
					loadAsset(it->path().string());
				} 
				/*
				else if (boost::filesystem::is_symlink(p))
				{
					loadAsset(p.string());
				}
				*/
			}
		}
		for (size_t d = 0; d < dirs.size(); d++)
		{
			loadAssetsFromPath(dirs[d].string());
		}
	}
	else
	{
		loadAsset(assetPath);
	}
	
	
/*
	load_motions(assetPath.c_str(), true);
	load_skeletons(assetPath.c_str(), true);
*/
}

SBSkeleton* SBAssetManager::addSkeletonDefinition(const std::string& skelName )
{
	SBSkeleton* existingSkeleton = this->getSkeleton(skelName);
	if (existingSkeleton)
	{
		LOG("Skeleton named %s already exists, new skeleton will not be created.", skelName.c_str());
		return NULL;
	}
	SBSkeleton* sbSkel = new SBSkeleton();	
	sbSkel->setName(skelName);
	sbSkel->skfilename(skelName.c_str());
	_skeletons.insert(std::pair<std::string, SBSkeleton*>(sbSkel->getName(), sbSkel));

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectCreate(sbSkel);
	}

	return sbSkel;
}

void SBAssetManager::removeSkeletonDefinition(const std::string& skelName )
{
	std::map<std::string, SBSkeleton*>::iterator iter = _skeletons.find(skelName);
	if (iter == _skeletons.end())
	{
		LOG("Skeleton named %s does not exist.", skelName.c_str());
		return;
	}

	SBSkeleton* existingSkeleton = (*iter).second;

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectDelete(existingSkeleton);
	}

	_skeletons.erase(iter);
	delete existingSkeleton;
}

SBAPI void SBAssetManager::addSkeleton(SmartBody::SBSkeleton* skeleton)
{
	_skeletons[skeleton->getName()] = skeleton;
}

SBMotion* SBAssetManager::createMotion(const std::string& motionName)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(motionName);
	if (iter != _motions.end())
	{
		LOG("Motion named %s already exists, new motion will not be created.", motionName.c_str());
		return NULL;
	}

	SBMotion* motion = new SBMotion();
	motion->setName(motionName);
	_motions[motionName] = motion;

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectCreate(motion);
	}

	return motion;
}

SBAPI bool SBAssetManager::addMotion(SmartBody::SBMotion* motion)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(motion->getName());
	if (iter != _motions.end())
	{
		LOG("Motion named %s already exists, new motion will not be added.", motion->getName().c_str());
		return false;
	}
	_motions[motion->getName()] = motion;
	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectCreate(motion);
	}
	return true;
}

SBMotion* SBAssetManager::addMotionDefinition(const std::string& name, double duration, int numFrames)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(name);
	if (iter != _motions.end())
	{
		LOG("Motion named %s already exists, new motion will not be added.", name.c_str());
		return NULL;
	}

	SBMotion* sbMotion = new SBMotion();
	if (numFrames <= 2 && duration > 0)
	{
		sbMotion->insert_frame(0,0.f);
		sbMotion->insert_frame(1,(float)duration);
	}	
	else if (numFrames > 2 && duration > 0)// motion frame > 2
	{
		float deltaT = (float)duration/(numFrames-1);
		for (int i=0;i<numFrames;i++)
		{
			sbMotion->insert_frame(i,deltaT*i);			
		}
	}
	sbMotion->setName(name);	
	this->addMotion(sbMotion);

	return sbMotion;
}


SBAPI void SBAssetManager::removeMotion(SmartBody::SBMotion* motion)
{
	std::map<std::string, SBMotion*>::iterator iter = _motions.find(motion->getName());
	if (iter != _motions.end())
	{
		_motions.erase(iter);
	}

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectDelete(motion);
	}
	delete motion;
}

SBAPI void SBAssetManager::addMesh(DeformableMesh* mesh)
{
	std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.find(mesh->getName());
	if (iter != _deformableMeshMap.end())
	{
		LOG("Mesh named %s already exists, new mesh will not be added.", mesh->getName().c_str());
		return;
	}
	_deformableMeshMap.insert(std::pair<std::string, DeformableMesh*>(mesh->getName(), mesh));

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectCreate(mesh);
	}
}

SBAPI void SBAssetManager::removeMesh(DeformableMesh* mesh)
{
	std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.find(mesh->getName());
	if (iter != _deformableMeshMap.end())
	{
		_deformableMeshMap.erase(iter);
	}

	std::vector<SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
	for (size_t l = 0; l < listeners.size(); l++)
	{
		listeners[l]->OnObjectDelete(mesh);
	}
	delete mesh;
}

int SBAssetManager::getNumMeshes()
{
	return _deformableMeshMap.size();
}


std::vector<std::string> SBAssetManager::getMeshNames()
{
	std::vector<std::string> ret;

	for(std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.begin();
		iter != _deformableMeshMap.end();
		iter++)
	{
		DeformableMesh* mesh = (*iter).second;
		ret.push_back(mesh->getName());
	}

	return ret;
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


std::vector<std::string> SBAssetManager::getDeformableMeshNames()
{
	std::vector<std::string> ret;

	for(std::map<std::string, DeformableMesh*>::iterator iter = _deformableMeshMap.begin();
		iter != _deformableMeshMap.end();
		iter++)
	{
		ret.push_back(std::string(iter->first));
	}

	return ret;	
}

int SBAssetManager::load_me_motions( const char* pathname, bool recurse_dirs, double scale )
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
#if (BOOST_VERSION > 104400)
		std::string finalPathStr = finalPath.string();
#else
		std::string finalPathStr = finalPath.native_file_string();  
#endif
		return load_me_motions_impl( finalPathStr, recurse_dirs, scale, "ERROR: " );
	} else {
		LOG("ERROR: Invalid motion path \"%s\".", finalPath.string().c_str());
		return CMD_FAILURE;
	}
}


int SBAssetManager::load_me_motions_impl( const std::string& pathStr, bool recurse_dirs, double scale, const char* error_prefix )
{
	boost::filesystem::path pathname(pathStr);

	if( !boost::filesystem::exists( pathname ) )
	{
#if (BOOST_VERSION > 104400)
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.string().c_str());
#else
		LOG("%s Motion path \"%s\" not found.", error_prefix,  pathname.native_file_string().c_str());
#endif
		return CMD_FAILURE;
	}

	if( boost::filesystem::is_directory( pathname ) ) // path indicates a directory
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
				{
#if (BOOST_VERSION > 104400)
					std::string curStr = cur.string();
#else
					std::string curStr = cur.native_file_string();  
#endif
					load_me_motions_impl( curStr, recurse_dirs, scale, "WARNING: " );
				}
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
#if (BOOST_VERSION > 104400)
					std::string curStr = cur.string();
#else
					std::string curStr = cur.native_file_string();  
#endif
					load_me_motions_impl( curStr, false, scale, "WARNING: " );
				} 
				else if( DEBUG_LOAD_PATHS2 ) {
					LOG("DEBUG: load_me_motion_impl(): Skipping \"%s\".  Extension \"%s\" does not match MOTION_EXT.", cur.string().c_str(), ext.c_str() );
				}
			}
		}
	} 
	else // path indicates a file
	{

		std::vector<SBMotion*> motions;
		std::string ext = boost::filesystem::extension( pathname );
		SmartBody::SBMotion* motion = new SmartBody::SBMotion();
		bool parseSuccessful = false;

		std::string convertedPath = pathname.string();
#ifdef WIN32
		boost::replace_all(convertedPath, "\\", "/");
#endif
		if (ext == ".skm" || ext == ".SKM")
		{
			
			SrInput in( convertedPath.c_str(), "rt" );
			SrString fullin_string;
			in.getall( fullin_string );
			SrInput fullin( (const char *)fullin_string );
			fullin.filename( convertedPath.c_str() ); // copy filename for error message
			
			parseSuccessful = motion->load( fullin, scale );
			if (parseSuccessful)
				motions.push_back(motion);
		}
		else if (ext == ".bvh" || ext == ".BVH")
		{
			std::ifstream filestream( convertedPath.c_str() );
			
			SkSkeleton skeleton;
			parseSuccessful = ParserBVH::parse(skeleton, *motion, convertedPath, filestream, float(scale));
			if (parseSuccessful)
				motions.push_back(motion);
		}
		else if (ext == ".dae" || ext == ".DAE")
		{			
			SBSkeleton skeleton;
			if (SmartBody::SBScene::getScene()->getBoolAttribute("useFastCOLLADAParsing"))
				parseSuccessful = ParserCOLLADAFast::parse(skeleton, *motion, convertedPath, float(scale), true, true);		
			else
				parseSuccessful = ParserOpenCOLLADA::parse(skeleton, *motion, convertedPath, float(scale), true, true);		
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
			parseSuccessful = ParserOgre::parse(skeleton, motions, convertedPath, float(scale), true, true);			
		}
		else if (ext == ".amc" || ext == ".AMC")
		{
			// at the same directory, looking for one asf file
			std::string asf = "";
			boost::filesystem::directory_iterator end;
			std::string filebase = boost::filesystem::basename(pathname);
			std::string fileext = boost::filesystem::extension(pathname);
			int dirSize = convertedPath.size() - filebase.size() - fileext.size() - 1;
			std::string directory = convertedPath.substr(0, dirSize);
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
			std::ifstream filestream(convertedPath.c_str());
			SBSkeleton skeleton;
			parseSuccessful = ParserASFAMC::parseAsf(skeleton,metafilestream, float(scale));
			parseSuccessful = ParserASFAMC::parseAmc(*motion, &skeleton, filestream, float(scale));
			motion->setName(filebase.c_str());
			if (parseSuccessful)
				motions.push_back(motion);
		}
#if ENABLE_FBX_PARSER
		else if (ext == ".fbx" || ext == ".FBX")
		{
			SkSkeleton skeleton;
			LOG("FBX motion parse: %s", convertedPath.c_str());
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

#if (BOOST_VERSION > 104400)
				filename = pathname.filename().string();		
#ifdef WIN32
		boost::replace_all(filename, "\\", "/");
#endif
#else
				filename = pathname.filename().c_str();
#endif

				//filename = mcn_return_full_filename_func( CurrentPath, finalPath.string().c_str() );
				//LOG("motion filename = %s, pathname string = %s, pathname nativestring = %s",filename.c_str(), pathname.string().c_str(), pathname.native_file_string().c_str());
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
				motion->filename( convertedPath.c_str() );
				SBMotion* existingMotion = getMotion(filebase);
				if (existingMotion)
				{
	//				LOG("ERROR: Motion by name of \"%s\" already exists. Ignoring file '%s'.", filebase.c_str(), pathname.string().c_str());
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
	//LOG("loadMotionIndividual, filename = %s, filebase = %s",filename.c_str(), filebase.c_str());
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
	return load_me_motions( pathname, recursive, scale );
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
			if( (fp = fopen( filename.c_str(), "rt" )) )	{
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
		ParserASFAMC::parseAsf(*skeleton_p, filestream, float(scale));
		skeleton_p->skfilename(filename.c_str());
	}
	else if (filename.find(".dae") == (filename.size() - 4) || 
			 filename.find(".DAE") == (filename.size() - 4))
	{
		fclose(fp);
		SmartBody::SBMotion motion;
		if (SmartBody::SBScene::getScene()->getBoolAttribute("useFastCOLLADAParsing"))
			ParserCOLLADAFast::parse(*skeleton_p, motion, filename, float(scale), true, false);
		else
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



int SBAssetManager::load_me_skeletons_impl( const std::string& pathStr, std::map<std::string, SmartBody::SBSkeleton*>& map, bool recurse_dirs, double scale, const char* error_prefix )
{
	boost::filesystem::path pathname(pathStr);	
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
				{
#if (BOOST_VERSION > 104400)
					std::string curStr = cur.string();
#else
					std::string curStr = cur.native_file_string();  
#endif
					load_me_skeletons_impl( curStr, map, recurse_dirs, scale, "WARNING: " );
				}
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
#if (BOOST_VERSION > 104400)
					std::string curStr = cur.string();
#else
					std::string curStr = cur.native_file_string();  
#endif
					load_me_skeletons_impl( curStr, map, recurse_dirs, scale, "WARNING: " );
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
			bool ok = ParserASFAMC::parseAsf(*skeleton, filestream, float(scale));
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
			bool ok = false;
			if (SmartBody::SBScene::getScene()->getBoolAttribute("useFastCOLLADAParsing"))
				ok = ParserCOLLADAFast::parse(*skeleton, motion, pathname.string(), float(scale), true, false);
			else
				ok = ParserOpenCOLLADA::parse(*skeleton, motion, pathname.string(), float(scale), true, false);
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
		skeleton->skfilename(skeletonName.c_str());
		std::string filebasename = boost::filesystem::basename(skeleton->skfilename());
		std::string fileextension = boost::filesystem::extension(skeleton->skfilename());
		skeleton->setName(filebasename+fileextension);	
		SmartBody::SBSkeleton* sbskel = dynamic_cast<SmartBody::SBSkeleton*>(skeleton);
		sbskel->setFileName(skeletonName);
		map.insert(std::pair<std::string, SmartBody::SBSkeleton*>(skeleton->getName(), skeleton));
	}
	//LOG("skeleton %s, filename = %s, skfilename = %s",skeleton->getName().c_str(),skeleton->getFileName().c_str(), skeleton->skfilename().c_str());

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
#if (BOOST_VERSION > 104400)
		std::string finalPathStr = finalPath.string();
#else
		std::string finalPathStr = finalPath.native_file_string();  
#endif
		return load_me_skeletons_impl( finalPathStr, map, recurse_dirs, scale, "ERROR: " );
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
		delete (*iter).second;		
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

void SBAssetManager::addAssetHandler(SBAssetHandler* handler)
{
	if (!handler)
		return;

	// make sure the handler doesn't exist already
	std::vector<SBAssetHandler*>::iterator iter = std::find(_assetHandlers.begin(), _assetHandlers.end(), handler);
	if (iter != _assetHandlers.end())
		return;

	// add the asset types to the map
	_assetHandlers.push_back(handler);
	std::vector<std::string> assetTypes = handler->getAssetTypes();
	for (std::vector<std::string>::iterator iter = assetTypes.begin();
		 iter != assetTypes.end();
		 iter++)
	{
		std::map<std::string, std::vector<SBAssetHandler*> >::iterator mapIter = _assetHandlerMap.find((*iter));
		if (mapIter == _assetHandlerMap.end())
		{
			std::vector<SBAssetHandler*> h;
			h.push_back(handler);
			_assetHandlerMap[(*iter)] = h;
		}
		else
		{
			_assetHandlerMap[(*iter)].push_back(handler);
		}
	}
}

void SBAssetManager::removeAssetHandler(SBAssetHandler* handler)
{
	if (!handler)
		return;

	// make sure the handler exists
	std::vector<SBAssetHandler*>::iterator iter = std::find(_assetHandlers.begin(), _assetHandlers.end(), handler);
	if (iter == _assetHandlers.end())
		return;

	// remove the handler from the handler list
	_assetHandlers.erase(iter);

	// remove the handler from the map
	// add the asset types to the map
	std::vector<std::string> assetTypes = handler->getAssetTypes();
	for (std::vector<std::string>::iterator iter = assetTypes.begin();
		 iter != assetTypes.end();
		 iter++)
	{
		std::map<std::string, std::vector<SBAssetHandler*> >::iterator mapIter = _assetHandlerMap.find((*iter));
		std::vector<SBAssetHandler*>::iterator assetIter = std::find((*mapIter).second.begin(), (*mapIter).second.end(), handler);
	
		(*mapIter).second.erase(assetIter);
	}

}

std::vector<SBAssetHandler*>& SBAssetManager::getAssetHandlers()
{
	return _assetHandlers;
}

std::vector<SBAssetHandler*> SBAssetManager::getAssetHandlers(const std::string& assetTypes)
{
	// search for file extensions in lower case only
	std::string lowerCaseAssetTypes = vhcl::ToLower(assetTypes);
	std::map<std::string, std::vector<SBAssetHandler*> >::iterator mapIter = _assetHandlerMap.find(lowerCaseAssetTypes);
	if (mapIter == _assetHandlerMap.end())
		return std::vector<SBAssetHandler*>();
	else
		return (*mapIter).second;
}

SBAPI std::string SBAssetManager::getAssetNameVariation(SBAsset* asset)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	int* counter = NULL;
	SBMotion* motion = dynamic_cast<SBMotion*>(asset);
	if (motion)
	{
		while (true)
		{
			_motionCounter++;
			std::stringstream strstr;
			strstr << motion->getName() << "xxx" << _motionCounter;
			if (!scene->getMotion(strstr.str()))
			{
				return strstr.str();
			}
		}
	}
	SBSkeleton* skeleton = dynamic_cast<SBSkeleton*>(asset);
	if (skeleton)
	{
		while (true)
		{
			_skeletonCounter++;
			std::stringstream strstr;
			strstr << skeleton->getName() << _skeletonCounter;
			if (!scene->getSkeleton(strstr.str()))
			{
				return strstr.str();
		
			}
		}
	}
	DeformableMesh* mesh = dynamic_cast<DeformableMesh*>(asset);
	if (mesh)
	{
		while (true)
		{
			_meshCounter++;
			std::stringstream strstr;
			strstr << mesh->getName() << _meshCounter;
			if (!scene->getAssetManager()->getDeformableMesh(strstr.str()))
			{
				return strstr.str();
		
			}
		}
	}

	return  "xxxxxxx";

	
}

void SBAssetManager::addAssetHistory(const std::string& str)
{
	_assetHistory.push_back(str);
}

std::vector<std::string>& SBAssetManager::getAssetHistory()
{
	return _assetHistory;
}

void SBAssetManager::clearAssetHistory()
{
	_assetHistory.clear();
}

bool SBAssetManager::createMeshFromBlendMasks(const std::string& neutralShapeFile, const std::string& neutralTextureFile, const std::string& expressiveShapeFile, const std::string& expressiveTextureFile, const std::string& maskTextureFile, const std::string& outputMeshFile, const std::string& outputTextureFile)
{
	bool ok = true;

	SbmTextureManager& texManager = SbmTextureManager::singleton();

	// load the neutral shape
	DeformableMesh* neutralMesh = NULL;
	std::vector<SBAsset*> assets = this->loadAsset(neutralShapeFile);
	for (size_t a = 0; a < assets.size(); a++)
	{
		DeformableMesh* mesh = dynamic_cast<DeformableMesh*>(assets[a]);
		if (mesh)
		{
			neutralMesh = mesh;
		}
	}
	if (!neutralMesh)
	{
		LOG("Could not find neutral mesh in file %s.", neutralShapeFile.c_str());
		ok = false;
	}

	// make sure that a diffuse texture exists
	SbmTexture* neutralTexture = NULL;

	if (neutralMesh)
	{
		for (unsigned int i=0; i < neutralMesh->subMeshList.size(); i++)
		{
			SbmSubMesh* subMesh = neutralMesh->subMeshList[i];
			neutralTexture = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, subMesh->texName.c_str());
			if (neutralTexture)
			{
				break;
			}
		}
		if (!neutralTexture)
		{
			if (neutralMesh->dMeshStatic_p.size() > 0)
			{
				SrModel& model = neutralMesh->dMeshStatic_p[0]->shape();
				for (std::map<std::string,std::string>::iterator iter = model.mtlTextureNameMap.begin(); 
					 iter != model.mtlTextureNameMap.end();
					 iter++)
				{
					std::string textureCandidate = (*iter).second;
					neutralTexture = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, textureCandidate.c_str());
					if (neutralTexture)
						break;
				}
			}
		}
	}
	if (!neutralTexture)
	{
		LOG("Could not find neutral diffuse texture in file %s.", neutralShapeFile.c_str());
		ok = false;
	}

	// load the expressive shape
	DeformableMesh* expressiveMesh = NULL;
	assets = this->loadAsset(expressiveShapeFile);
	for (size_t a = 0; a < assets.size(); a++)
	{
		DeformableMesh* mesh = dynamic_cast<DeformableMesh*>(assets[a]);
		if (mesh)
		{
			expressiveMesh = mesh;
		}
	}
	if (!expressiveMesh)
	{
		LOG("Could not find expressive mesh in file %s.", expressiveShapeFile.c_str());
		ok = false;
	}

	// make sure that an expressive diffuse texture exists
	SbmTexture* expressiveTexture = NULL;

	if (expressiveMesh)
	{
		for (unsigned int i=0; i < expressiveMesh->subMeshList.size(); i++)
		{
			SbmSubMesh* subMesh = expressiveMesh->subMeshList[i];
			expressiveTexture = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, subMesh->texName.c_str());
			if (expressiveTexture)
			{
				break;
			}
		}
		if (!expressiveTexture)
		{
			if (expressiveMesh->dMeshStatic_p.size() > 0)
			{
				SrModel& model = expressiveMesh->dMeshStatic_p[0]->shape();
				for (std::map<std::string,std::string>::iterator iter = model.mtlTextureNameMap.begin(); 
					 iter != model.mtlTextureNameMap.end();
					 iter++)
				{
					std::string textureCandidate = (*iter).second;
					expressiveTexture = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, textureCandidate.c_str());
					if (expressiveTexture)
						break;
				}
			}
		}
	}
	if (!expressiveTexture)
	{
		LOG("Could not find expressive diffuse texture in file %s.", expressiveShapeFile.c_str());
		ok = false;
	}

	// load the masked texture
	texManager.loadTexture(SbmTextureManager::TEXTURE_DIFFUSE, maskTextureFile.c_str(), maskTextureFile.c_str());
	SbmTexture* maskedTexture = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, maskTextureFile.c_str());
	if (!maskedTexture)
	{
		LOG("Could not load masked texture file %s.", maskTextureFile.c_str());
		ok = false;
	}

	// make sure the neutral and the expression have the same number of vertices
	
	int numEMeshes = 0;
	if (expressiveMesh)
		numEMeshes = expressiveMesh->dMeshStatic_p.size();
	int numNMeshes = 0;
	if (neutralMesh)
		numNMeshes = neutralMesh->dMeshStatic_p.size();
	if (numEMeshes != numNMeshes)
	{
		LOG("Neutral mesh %s has different number of meshes as expresive mesh %s (%d vs %d).", neutralShapeFile.c_str(), expressiveShapeFile.c_str(), numNMeshes, numEMeshes);
		ok = false;
	}
	
	int numEVertices = 0;
	int numNVertices = 0;
	if (expressiveMesh)
	{
		for (size_t i = 0; i < expressiveMesh->dMeshStatic_p.size(); i++)
		{
			numEVertices += expressiveMesh->dMeshStatic_p[i]->shape().V.size();
		}
	}
	if (neutralMesh)
	{
		for (size_t i = 0; i < neutralMesh->dMeshStatic_p.size(); i++)
		{
			numNVertices += neutralMesh->dMeshStatic_p[i]->shape().V.size();
		}
	}
	if (numEVertices != numNVertices)
	{
		LOG("Neutral mesh %s has different number of vertices as expresive mesh %s (%d vs %d).", neutralShapeFile.c_str(), expressiveShapeFile.c_str(), numNMeshes, numEMeshes);
		ok = false;
	}

	// make sure the neutral, expression and mask textures are the same size
	int eHeight = 0;
	int eWidth = 0;
	if (expressiveTexture)
	{
		eHeight = expressiveTexture->getHeight();
		eWidth = expressiveTexture->getWidth();
	}

	int nHeight = 0;
	int nWidth = 0;
	if (neutralTexture)
	{
		nHeight = neutralTexture->getHeight();
		nWidth = neutralTexture->getWidth();
	}

	int mHeight = 0;
	int mWidth = 0;
	if (maskedTexture)
	{
		mHeight = maskedTexture->getHeight();
		mWidth = maskedTexture->getWidth();
	}


	if (eHeight != nHeight || 
		eHeight != mHeight ||
		nHeight != mHeight)
	{
		LOG("Texture heights don't match (%s: %d, %s: %d, %s: %d", neutralTextureFile.c_str(), nHeight, expressiveTextureFile.c_str(), eHeight, maskTextureFile.c_str(), mHeight);
		ok = false;
	}
	if (eWidth != nWidth || 
		eWidth != mWidth ||
		nWidth != mWidth)
	{
		LOG("Texture heights don't match (%s: %d, %s: %d, %s: %d", neutralTextureFile.c_str(), nWidth, expressiveTextureFile.c_str(), eWidth, maskTextureFile.c_str(), mWidth);
		ok = false;
	}

	if (!ok)
		return false;

	// create a template for the final mesh
	// load the expressive shape
	DeformableMesh* maskedMesh = NULL;
	assets = this->loadAsset(expressiveShapeFile);
	for (size_t a = 0; a < assets.size(); a++)
	{
		DeformableMesh* mesh = dynamic_cast<DeformableMesh*>(assets[a]);
		if (mesh)
		{
			maskedMesh = mesh;
		}
	}

	
	int numMaskChannels = maskedTexture->getNumChannels();

	unsigned char* maskedBuffer = maskedTexture->getBuffer();
	for (size_t m = 0; m < maskedMesh->dMeshStatic_p.size(); m++)
	{
		SrModel& neutralModel = neutralMesh->dMeshStatic_p[m]->shape();
		SrModel& expressiveModel = expressiveMesh->dMeshStatic_p[m]->shape();
		SrModel& maskedModel = maskedMesh->dMeshStatic_p[m]->shape();


		std::set<int> vertexUsed;
		for (int f = 0; f < neutralModel.F.size(); f++)
		{
			SrModel::Face& face = neutralModel.F[f];
			SrModel::Face& faceTextureIndex = neutralModel.Ft[f];

			int curVertex = -1;
			int curTextureIndex = -1;
			for (int v = 0; v < 3; v++)
			{
				curVertex = face[v];
				std::set<int>::iterator iter = vertexUsed.find(curVertex);
				if (iter == vertexUsed.end())
				{
					vertexUsed.insert(curVertex);
				}
				else
				{
					continue;
				}
				curTextureIndex = faceTextureIndex[v];
			
				// get the texture coordinates for the vertex
				SrPnt2& uv = neutralModel.T[curTextureIndex];
				// normalize uv to size of texture
				float cf = ((float) mWidth) * uv.x;
				float rf = ((float) mHeight) * uv.y;
				int rpos = (int) rf;
				int cpos = (int) cf;
			
				// find the greyscale color of the mask texture
				int position = rpos * mWidth * numMaskChannels + (cpos * numMaskChannels);
				SrColor maskColor;
				maskColor.r = maskedBuffer[position];
				maskColor.g = maskedBuffer[position + 1];
				maskColor.b = maskedBuffer[position + 2];
				if (numMaskChannels == 4)
					maskColor.a = maskedBuffer[position + 3];
				else
					maskColor.a = 255;

				// assuming masks are in greyscale, use only red channel to determine 'whiteness'
				float mcolor[4];
				maskColor.get(mcolor);
				float maskGreyAmount = mcolor[0];

				SrPnt& pointNeutral = neutralModel.V[curVertex];
				SrPnt& pointExpressive = expressiveModel.V[curVertex];
				SrPnt& pointMasked = maskedModel.V[curVertex];
				for (int n = 0; n < 3; n++)
					pointMasked[n] = maskGreyAmount * pointExpressive[n] + (1.0f - maskGreyAmount) * pointNeutral[n];
	
			}
		}

	}

	boost::filesystem::path p(outputMeshFile);
	std::string fileName = boost::filesystem::basename( p );
	std::string extension =  boost::filesystem::extension( p );
	std::stringstream strstr; 
	strstr << fileName << ".mtl";
	
	// save the final mesh
	for (size_t m = 0; m < maskedMesh->dMeshStatic_p.size(); m++)
	{
		maskedMesh->dMeshStatic_p[m]->shape().export_obj(outputMeshFile.c_str(), strstr.str().c_str(), outputTextureFile.c_str());
	}
	
	// for each texture:
	// get the color of the mask
	// interpolate the vertex according to the mask color (black = neutral, white = expression, gray = interpolated)
	
	unsigned char* neutralBuffer = neutralTexture->getBuffer();
	unsigned char* expressiveBuffer = expressiveTexture->getBuffer();


	int numExpressiveChannels = expressiveTexture->getNumChannels();
	int numNeutralChannels = neutralTexture->getNumChannels();

	int maxSize = mWidth * mHeight * 4 * sizeof(unsigned char);
	unsigned char* outData = new unsigned char[maxSize];
	SbmTexture* outputExpressiveTexture = new SbmTexture("maskedTexture");
	outputExpressiveTexture->setTextureSize(mWidth, mHeight, 4);
	for (int h = 0; h < mHeight; h++)
	{
		for (int w = 0; w < mWidth; w++)
		{
			int position = h * mWidth * 4 + w * 4;
			int flippedPosition = (mHeight - h - 1) * mWidth * 4 + w * 4;

			int position3 = h * mWidth * 3 + w * 3;
			int flippedPosition3 = (mHeight - h - 1) * mWidth * 3 + w * 3;

			SrColor maskColor;
			if (numMaskChannels == 4)
			{
				maskColor.r = maskedBuffer[position];
				maskColor.g = maskedBuffer[position + 1];
				maskColor.b = maskedBuffer[position + 2];
				maskColor.a = maskedBuffer[position + 3];
			}
			else if (numMaskChannels == 3)
			{
				maskColor.r = maskedBuffer[position3];
				maskColor.g = maskedBuffer[position3 + 1];
				maskColor.b = maskedBuffer[position3 + 2];
				maskColor.a = 1.0f;
			}

			
			// assuming masks are in greyscale, use only red channel to determine 'whiteness'
			float mcolor[4];
			maskColor.get(mcolor);
			float maskGreyAmount = mcolor[0];
			
			bool stop = false;
			if (maskGreyAmount > 0.0f)
				stop = true;


			SrColor expressiveColor;
			if (numExpressiveChannels == 4)
			{
				expressiveColor.r = expressiveBuffer[position];
				expressiveColor.g = expressiveBuffer[position + 1];
				expressiveColor.b = expressiveBuffer[position + 2];
				expressiveColor.a = expressiveBuffer[position + 3];
			}
			else if (numExpressiveChannels == 3)
			{
				expressiveColor.r = expressiveBuffer[position3];
				expressiveColor.g = expressiveBuffer[position3 + 1];
				expressiveColor.b = expressiveBuffer[position3 + 2];
				expressiveColor.a = 1.0f;
			}
			float ecolor[4];
			expressiveColor.get(ecolor);

			SrColor neutralColor;
			if (numNeutralChannels == 4)
			{
				neutralColor.r = neutralBuffer[position];
				neutralColor.g = neutralBuffer[position + 1];
				neutralColor.b = neutralBuffer[position + 2];
				neutralColor.a = neutralBuffer[position + 3];
			}
			else if (numNeutralChannels == 3)
			{
				neutralColor.r = neutralBuffer[position3];
				neutralColor.g = neutralBuffer[position3 + 1];
				neutralColor.b = neutralBuffer[position3 + 2];
				neutralColor.a = 1.0f;
			}
			float ncolor[4];
			neutralColor.get(ncolor);
			
			// white = use expressive mask, black = use neutral mask, grey = interpolate values
			float finalRf = ecolor[0] * maskGreyAmount + (1.0f - maskGreyAmount) * ncolor[0];
			float finalGf = ecolor[1] * maskGreyAmount + (1.0f - maskGreyAmount) * ncolor[1];
			float finalBf = ecolor[2] * maskGreyAmount + (1.0f - maskGreyAmount) * ncolor[2];
			float finalAf = ecolor[3] * maskGreyAmount + (1.0f - maskGreyAmount) * ncolor[3];
			
			SrColor finalColor(finalRf, finalGf, finalBf, finalAf);

			// make sure to flip the data so that it gets written correctly
			outData[flippedPosition3] = finalColor.r;
			outData[flippedPosition3 + 1] = finalColor.g;
			outData[flippedPosition3 + 2] = finalColor.b;
			outData[flippedPosition3 + 3] = finalColor.a;
		}
	}
	outputExpressiveTexture->setBuffer(outData, maxSize);
	// save the texture here....
	int ret = SOIL_save_image(outputTextureFile.c_str(), SOIL_SAVE_TYPE_BMP, mWidth, mHeight, 3, outData);

	if (ret == 0)
	{
		LOG("Could not save masked texture image to file %s.", outputTextureFile.c_str());
	}
	else
	{
		LOG("Successfully saved masked texture image to file %s.", outputTextureFile.c_str());
	}

	return true;

}



}

