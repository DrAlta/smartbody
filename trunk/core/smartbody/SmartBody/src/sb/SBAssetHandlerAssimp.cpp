#include "SBAssetHandlerAssimp.h"
#include <vhcl.h>
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <sb/SBMotion.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include "sbm/ParserCOLLADAFast.h"
#include "sbm/ParserOpenCOLLADA.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <external/assimp-3.1.1/include/assimp/Importer.hpp> 
#include <external/assimp-3.1.1/include/assimp/scene.h>
#include <external/assimp-3.1.1/include/assimp/postprocess.h> 

namespace SmartBody {

SBAssetHandlerAssimp::SBAssetHandlerAssimp()
{
	assetTypes.push_back("fbx");
}

SBAssetHandlerAssimp::~SBAssetHandlerAssimp()
{
}

std::vector<SBAsset*> SBAssetHandlerAssimp::getAssets(const std::string& path)
{
	std::vector<SBAsset*> assets;

	std::string convertedPath = checkPath(path);
	if (convertedPath == "")
		return assets;

	////////////////////////////////////////////
	
	Assimp::Importer importer;
	  const aiScene* scene = importer.ReadFile( convertedPath, 
        aiProcess_CalcTangentSpace       | 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);
  
  // If the import failed, report it
  if( !scene)
  {
    LOG(importer.GetErrorString());
    return assets;
  }
  else
  {
	  if (scene->HasAnimations())
	  {
		  LOG("HAS ANIMATIONS");
	  }
	  if (scene->HasCameras())
	  {
		  LOG("HAS CAMERAS");
	  }
	  if (scene->HasLights())
	  {
		  LOG("HAS LIGHTS");
	  }
	  if (scene->HasMaterials())
	  {
		  LOG("HAS MATERIALS");
	  }
	  if (scene->HasMeshes())
	  {
		  LOG("HAS MESHES");
	  }
	  if (scene->HasTextures())
	  {
		  LOG("HAS TEXTURES");
	  }

	  for (int m = 0; m < scene->mNumMeshes; m++)
	  {
		  LOG("FOUND MESH WITH %d VERTICES, %d FACES", scene->mMeshes[m]->mNumVertices, scene->mMeshes[m]->mNumFaces);
#if !defined (__ANDROID__) && !defined(SB_IPHONE) &&  !defined(__FLASHPLAYER__) && !defined(__native_client__)
			SbmDeformableMeshGPU* mesh = new SbmDeformableMeshGPU();
#else
			DeformableMesh* mesh = new DeformableMesh();
#endif
			
		  // extract vertices
		  for (int v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
		  {
			  scene->mMeshes[m]->mVertices[v].x;
			  scene->mMeshes[m]->mVertices[v].y;
			  scene->mMeshes[m]->mVertices[v].z;
		  }
		  // extract faces


		  if (scene->mMeshes[m]->HasBones())
		  {
			  //for (int b = 0; b < scene->mMeshes[m]->mBones[b]->
		  }

		  
		  // extract skeleton
		  if (scene->mMeshes[m]->HasBones())
		  {
			  //for (int b = 0; b < scene->mMeshes[m]->mBones[b]->
		  }

		  // extract animations



		  // extract textures

		  // extract materials
	  }

  }

	return assets;
}

};
