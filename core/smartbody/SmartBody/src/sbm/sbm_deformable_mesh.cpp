/*************************************************************
Copyright (C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/

#include "vhcl.h"
#include <sb/SBTypes.h>

#if !defined(__FLASHPLAYER__) && !defined(ANDROID_BUILD) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
#include "external/glew/glew.h"
#include "sbm/GPU/SbmDeformableMeshGPU.h"
#endif

#include "sbm/GPU/SbmBlendFace.h"
#include "sbm_deformable_mesh.h"

#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sr/sr_sn_group.h>
#include <sr/sr_random.h>
#include <sbm/gwiz_math.h>
#include <sbm/GPU/SbmTexture.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <boost/algorithm/string.hpp>
#include <protocols/sbmesh.pb.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/lexical_cast.hpp>

#include "external/glm/glm/glm.hpp"
#include "external/glm/glm/gtc/type_ptr.hpp"
#include "external/glm/glm/gtc/matrix_transform.hpp"

#include <boost/filesystem.hpp>
#include <algorithm>
#include <sbm/ParserCOLLADAFast.h>

#define TEST_HAIR_RENDER 1

typedef std::pair<int,int> IntPair;
typedef std::pair<int,float> IntFloatPair;
static bool intFloatComp(const IntFloatPair& p1, const IntFloatPair& p2)
{
	return (p1.second > p2.second);
}

SkinWeight::SkinWeight()
{
}

SkinWeight::~SkinWeight()
{
	for (unsigned int i = 0; i < infJoint.size(); i++)
	{
		SkJoint* j = infJoint[i];
		if (j)
			j = NULL;
	}
	infJoint.clear();
}

void SkinWeight::normalizeWeights()
{
	int idx = 0;
	for (unsigned int i=0;i<numInfJoints.size();i++)
	{
		int nJoint = numInfJoints[i];
		float wTotal = 0.f;
		for (int k=0;k<nJoint;k++)
		{
			int widx = weightIndex[idx+k];
			wTotal += bindWeight[widx];			
		}

		for (int k=0;k<nJoint;k++)
		{
			int widx = weightIndex[idx+k];
			bindWeight[widx] /= wTotal;			
		}

		idx+= nJoint;
	}
}

void SkinWeight::copyWeights(SkinWeight* copy, const std::string& morphName)
{
	infJointName.clear();
	for (size_t jn = 0; jn < copy->infJointName.size(); jn++)
	{
		infJointName.push_back(copy->infJointName[jn]);
	}

	infJoint.clear();
	for (size_t j = 0; j < copy->infJoint.size(); j++)
	{
		infJoint.push_back(copy->infJoint[j]);
	}

	bindWeight.clear();
	for (size_t bw = 0; bw < copy->bindWeight.size(); bw++)
	{
		bindWeight.push_back(copy->bindWeight[bw]);
	}

	bindPoseMat.clear();
	for (size_t bp = 0; bp < copy->bindPoseMat.size(); bp++)
	{
		bindPoseMat.push_back(copy->bindPoseMat[bp]);
	}

	bindShapeMat = copy->bindShapeMat;
	sourceMesh = morphName; // set the name to the input parameter, instead of the source name (i.e. copy->sourceMesh)

	numInfJoints.clear();
	for (size_t n = 0; n < copy->numInfJoints.size(); n++)
	{
		numInfJoints.push_back(copy->numInfJoints[n]);
	}

	weightIndex.clear();
	for (size_t wi = 0; wi < copy->weightIndex.size(); wi++)
	{
		weightIndex.push_back(copy->weightIndex[wi]);
	}

	jointNameIndex.clear();
	for (size_t jni = 0; jni < copy->jointNameIndex.size(); jni++)
	{
		jointNameIndex.push_back(copy->jointNameIndex[jni]);
	}
}

SBAPI void SkinWeight::initWeights(std::string srcMesh, std::vector<SrVec4i>& boneID, std::vector<SrVec4>& boneWeights, std::vector<std::string>& boneJointNameList, std::vector<SrMat>& bindPoseMatList )
{
	sourceMesh = srcMesh;
	bindShapeMat = SrMat::id;

	infJointName.clear();
	infJoint.clear();
	bindPoseMat.clear();
	for (unsigned int i=0;i<boneJointNameList.size();i++)
	{
		bindPoseMat.push_back(bindPoseMatList[i]);
		infJointName.push_back(boneJointNameList[i]);
	}

	numInfJoints.clear();
	weightIndex.clear();
	bindWeight.clear();
	jointNameIndex.clear();
	
	int icount = 0;
	for (unsigned int i=0;i<boneID.size();i++)
	{
		numInfJoints.push_back(4);
		for (int k=0;k<4;k++)
		{
			weightIndex.push_back(icount++);
			jointNameIndex.push_back(boneID[i][k]);
			bindWeight.push_back(boneWeights[i][k]);
		}
	}
}

SBAPI void SkinWeight::addWeight( SkinWeight* weight )
{
	std::map<std::string, int> newJointMap;
	// initialize with original map
	for (unsigned int i=0;i<infJointName.size();i++)
	{
		newJointMap[infJointName[i]] = i;
	}
	// add new joints from input weight
	int indexCount = newJointMap.size();
	for (unsigned int i=0;i<weight->infJointName.size();i++)
	{
		std::string jointName = weight->infJointName[i];
		if (newJointMap.find(jointName) == newJointMap.end()) // add new joint
		{
			newJointMap[jointName] = indexCount;
			// update new joint name and bind pose matrix
			infJointName.push_back(jointName);
			bindPoseMat.push_back(weight->bindPoseMat[i]);
			indexCount++;
		}
	}
	// append bind weights
	int weightIdxOffset = bindWeight.size();
	bindWeight.insert(bindWeight.end(), weight->bindWeight.begin(), weight->bindWeight.end());
	numInfJoints.insert(numInfJoints.end(), weight->numInfJoints.begin(), weight->numInfJoints.end());
	for (unsigned int i=0;i<weight->weightIndex.size();i++)
		weightIndex.push_back(weight->weightIndex[i] + weightIdxOffset);
	for (unsigned int i=0;i<weight->jointNameIndex.size();i++)
	{
		int newJointNameIdx = newJointMap[weight->infJointName[weight->jointNameIndex[i]]];
		jointNameIndex.push_back(newJointNameIdx);
	}
}

SBAPI void SkinWeight::mergeRedundantWeight( std::vector<int>& vtxIdxMap )
{
	if (vtxIdxMap.size() != numInfJoints.size())
	{
		LOG("Warning!! mergeRedundantWeight() : vtxIdxMap.size() != numInfJoint.size()");
		return;
	}
	
	std::vector<unsigned int>	newNumInfJoints;	// number of influenced joints for very vertex
	std::vector<unsigned int>	newWeightIdx;	// looking up the weight according to this index
	std::vector<unsigned int>	newJointNameIdx;	// looking up the joint name according to this index

	int icount = 0;	
	for (unsigned int i=0;i<vtxIdxMap.size();i++)
	{
		int numJoint = numInfJoints[i];
		unsigned int newIdx = vtxIdxMap[i];
		if (newIdx >= newNumInfJoints.size())
		{
			newNumInfJoints.push_back(numJoint);
			for (int k=0;k<numJoint;k++)
			{
				newWeightIdx.push_back(weightIndex[icount+k]);
				newJointNameIdx.push_back(jointNameIndex[icount+k]);
			}
		}
		icount += numJoint;
	}

	// remove unuse binding weights
	/* 
	std::vector<bool> useBindWeight(bindWeight.size(), false);
	for (unsigned int i=0;i<newWeightIdx.size();i++)
		useBindWeight[newWeightIdx[i]] = true;

	std::vector<int> weightIdxMap(bindWeight.size(),-1);
	std::vector<float> newBindWeight;
	int idx = 0;
	for (unsigned int i=0;i<useBindWeight.size();i++)
	{
		if (useBindWeight[i])
		{
			newBindWeight.push_back(bindWeight[i]);
			weightIdxMap[i] = idx++;
		}
	}
	for (unsigned int i=0;i<newWeightIdx.size();i++)
	{
		newWeightIdx[i] = weightIdxMap[newWeightIdx[i]];
	}
	
	bindWeight = newBindWeight;
	*/

	numInfJoints = newNumInfJoints;
	weightIndex = newWeightIdx;
	jointNameIndex = newJointNameIdx;
}

SBAPI void SkinWeight::buildSkinWeightBuf()
{
	int numSkinVtxs = numInfJoints.size();
	boneIDs.resize(numSkinVtxs);
	boneWeights.resize(numSkinVtxs);
	int globalCounter = 0;
	for (unsigned int i = 0; i < numInfJoints.size() ; i++)
	{
		int numOfInfJoints = numInfJoints[i];
		
		boneIDs[i] = SrVec4i(0,0,0,0);
		boneWeights[i] = SrVec4(0,0,0,0);
	
		std::vector<IntFloatPair> weightList;
		if (infJointName.size() > 0)
		{
			for (int j = 0; j < numOfInfJoints; j++)
			{
				const std::string& curJointName = infJointName[jointNameIndex[globalCounter]];					
				float jointWeight = bindWeight[weightIndex[globalCounter]];
				int    jointIndex = jointNameIndex[globalCounter];
				weightList.push_back(IntFloatPair(jointIndex,jointWeight));							
				globalCounter ++;									
			}
		}
		std::sort(weightList.begin(),weightList.end(),intFloatComp); // sort for minimum weight
		int numWeight = numOfInfJoints > 4 ? 4 : numOfInfJoints;
		float weightSum = 0.f;
		SrVec skinColor;
		for (int j=0;j<numWeight;j++)
		{
			if (j >= (int)weightList.size())
				continue;
			IntFloatPair& w = weightList[j];

			if ( j < 4)
			{
				boneIDs[i][j] = w.first;
				boneWeights[i][j] = w.second;
				weightSum += w.second;
			}
		}
		if (weightSum != 0)
		{
			for (int j=0;j<4;j++)
			{
				boneWeights[i][j] /= weightSum;
			}
		}
	}
}


DeformableMesh::DeformableMesh() : SBAsset()
{	
	initSkinnedVertexBuffer = false;
	initStaticVertexBuffer = false;
	hasVertexColor = false;	
	hasTexCoord = false;
	skeleton = new SmartBody::SBSkeleton();
	skeleton->ref();
}

DeformableMesh::~DeformableMesh() 
{
	skeleton->unref();

	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->unref();
		//delete dMeshDynamic_p[i];
	}
	dMeshDynamic_p.clear();
	for (unsigned int i = 0; i < dMeshStatic_p.size(); i++)
	{
		dMeshStatic_p[i]->unref();
		//delete dMeshStatic_p[i];
	}
	dMeshStatic_p.clear();
	for (unsigned int i = 0; i < skinWeights.size(); i++)
	{
		SkinWeight* sw = skinWeights[i];
		if (sw)
		{
			delete sw;
			sw = NULL;
		}
	}	
	skinWeights.clear();

	for (size_t i = 0; i < subMeshList.size(); ++i)
	{
		if (subMeshList[i])
			delete subMeshList[i];
	}
	subMeshList.clear();
}


SBAPI void DeformableMesh::initDeformMesh( std::vector<SrModel*>& meshModelVec )
{
	dMeshStatic_p.clear();
	dMeshDynamic_p.clear();
	for (unsigned int i = 0; i < meshModelVec.size(); i++)
	{
		if (meshModelVec[i]->Fn.size() == 0)
		{
			meshModelVec[i]->computeNormals();
		}

		SrSnModel* srSnModelStatic = new SrSnModel();
		srSnModelStatic->shape(*meshModelVec[i]);
		srSnModelStatic->shape().name = meshModelVec[i]->name;

		SrSnModel* srSnModelDynamic = new SrSnModel();
		srSnModelDynamic->shape(*meshModelVec[i]);
		srSnModelDynamic->changed(true);
		srSnModelDynamic->visible(false);
		srSnModelDynamic->shape().name = meshModelVec[i]->name;

		dMeshStatic_p.push_back(srSnModelStatic);
		srSnModelStatic->ref();
		dMeshDynamic_p.push_back(srSnModelDynamic);
		srSnModelDynamic->ref();		
	}
	
}


SkinWeight* DeformableMesh::getSkinWeight(const std::string& skinSourceName)
{
	for (unsigned int i = 0; i < skinWeights.size(); i++)
	{
		if (skinSourceName == skinWeights[i]->sourceMesh)
			return skinWeights[i];
	}
	return NULL;
}

int DeformableMesh::getValidSkinMesh(const std::string& meshName)
{
	std::string sourceMeshName = meshName;
	std::map<std::string, std::vector<std::string> >::iterator iter = morphTargets.find(sourceMeshName);
	if (iter != morphTargets.end())
	{
		int morphSize = iter->second.size();	
		for (int morphCounter = 0; morphCounter < morphSize; morphCounter++)
		{	
			int pos;
			int globalCounter = 0;
			pos = getMesh(iter->second[morphCounter]);
			if (pos != -1)
			{
				return pos;
			}
		}	
	}
	else
		return getMesh(sourceMeshName);
	
	return -1;
}

int	DeformableMesh::getMesh(const std::string& meshName)
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		if (dMeshDynamic_p[i]->shape().name == meshName.c_str())
			return i;
	}
	return -1;
}


SBAPI void DeformableMesh::updateVertexBuffer()
{
	int iVtx = 0;
	for (unsigned int c=0;c<meshIndexList.size();c++)
	{
		unsigned int pos = meshIndexList[c];
		int globalCounter = 0;
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
		dMeshDynamic->visible(false);
		int numVertices = dMeshStatic->shape().V.size();
		SrMat bindShapeMat;
		SkinWeight* skinWeight = NULL;
		
		if ( pos >= 0 && skinWeights.size() > pos)
		{
			skinWeight = skinWeights[pos];
			bindShapeMat = skinWeight->bindShapeMat;
		}
		for (int i = 0; i < numVertices; i++)
		{
			SrVec& lv = dMeshStatic->shape().V[i];					
			posBuf[iVtx] = lv*bindShapeMat;
		
			if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
			{
				std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
				// copy related vtx components 
				for (unsigned int k=0;k<idxMap.size();k++)
				{
					posBuf[idxMap[k]] = posBuf[iVtx];
				}
			}
			iVtx++;
		}
	}

}


bool DeformableMesh::buildBlendShapes()
{
	if (blendShapeMap.size() == 0) return false;
	blendShapeNewVtxIdxMap.clear();
	std::string neutralName = blendShapeMap.begin()->first;
	int meshIdx = getMesh(neutralName);
	SrModel& neutralMesh = getStaticModel(meshIdx);
	// build Kd-tree from posBuf
	unsigned int numKNN = 10;
	MeshPointCloud posCloud;
	posCloud.pts = posBuf;
	std::vector<float> knnPtDists(numKNN);
	std::vector<int>   knnPtIdxs(numKNN);	
	MeshKDTree* meshKDTree = new MeshKDTree(3, posCloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
	meshKDTree->buildIndex();
	// for each vertex in neutral blendshape model, search for closest vertex in Kd-tree
	// if distance is zero, put vertex index in the blendShapeNewVtxIdxMap 
	for (unsigned int i=0;i<neutralMesh.V.size(); i++)
	{
		SrVec inPt = neutralMesh.V[i];
		meshKDTree->knnSearch((float*)&inPt, numKNN, (size_t*)&knnPtIdxs[0], &knnPtDists[0]);
		for (unsigned int k=0;k<numKNN;k++)
		{
			if (knnPtDists[k] < 1e-30) // almost identical vertex
			{
				if (blendShapeNewVtxIdxMap.find(i) == blendShapeNewVtxIdxMap.end())
				{
					blendShapeNewVtxIdxMap[i] = std::vector<int>();
				}
				blendShapeNewVtxIdxMap[i].push_back(knnPtIdxs[k]);
			}
		}
	}

	return true;
}


void DeformableMesh::rebuildVertexBuffer( bool rebuild )
{
	if (rebuild)
	{
		initSkinnedVertexBuffer = false;
		buildSkinnedVertexBuffer();
	}
}


bool DeformableMesh::buildSkinnedVertexBuffer()
{	
	if (initSkinnedVertexBuffer) return true;

	if (initStaticVertexBuffer && !isSkinnedMesh()) return true;

	bool buildSkinnedBuffer = isSkinnedMesh();

	LOG("Build Skinned Buffer");
	int nTotalVtxs=0, nTotalTris = 0, nTotalBones = 0;	
	std::vector<std::set<IntPair> > vtxNormalIdxMap;
	std::vector<std::set<int> > vtxMaterialIdxMap;
	std::map<IntPair,int> ntNewVtxIdxMap;	
	
	std::map<int,std::vector<int> > meshSubsetMap;	

	SrColor colorArray[6] = { SrColor::blue, SrColor::red, SrColor::green, SrColor::magenta, SrColor::gray, SrColor::yellow};
	// setup deformable mesh color	
	int nMaterial = 1;	
	std::vector<SrMaterial> allMatList;
	//std::vector<SrColor> allMatList;
	std::vector<std::string> allTexNameList;
	std::vector<std::string> allMatNameList;
	std::vector<std::string> allNormalTexNameList;
	std::vector<std::string> allSpecularTexNameList;	
	std::map<std::string, std::string> allMaterialMeshMap;
	SrMaterial defaultMaterial;
	defaultMaterial.diffuse = SrColor(0.6f,0.6f,0.6f);//SrColor::gray;	
	defaultMaterial.specular = SrColor(101,101,101);//SrColor::gray;
	defaultMaterial.shininess = 29;

	allMatList.push_back(defaultMaterial);
	allMatNameList.push_back("defaultMaterial");
	allTexNameList.push_back("");
	allNormalTexNameList.push_back("");
	allSpecularTexNameList.push_back("");
	meshSubsetMap[0] = std::vector<int>(); // default material group : gray color

	//std::vector<int> meshIndexList;
	std::vector<SkinWeight*> skinWeightList;
	meshIndexList.clear();
	boneJointIdxMap.clear();
	bindPoseMatList.clear();
	LOG("dynamic mesh size = %d, skin weight size = %d",dMeshDynamic_p.size(), skinWeights.size());
	if (buildSkinnedBuffer)
	{
		for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
		{
			SkinWeight* skinWeight = skinWeights[skinCounter];		
			int pos;
			int globalCounter = 0;
			//pos = this->getMesh(skinWeight->sourceMesh);
			pos = this->getValidSkinMesh(skinWeight->sourceMesh);
			//LOG("skinWeight->sourceMesh = %s, pos = %d", skinWeight->sourceMesh.c_str(), pos);
			if (pos != -1)
			{
				meshIndexList.push_back(pos);
				skinWeightList.push_back(skinWeight);
				for (size_t j = 0; j < skinWeight->infJointName.size(); j++)
				{
					std::string& jointName = skinWeight->infJointName[j];
					SkJoint* curJoint = skeleton->search_joint(jointName.c_str());					
					skinWeight->infJoint.push_back(curJoint); // NOTE: If joints are added/removed during runtime, this list will contain stale data
				}

				for (unsigned int k=0;k<skinWeight->infJointName.size();k++)
				{
					std::string& jointName = skinWeight->infJointName[k];
					SkJoint* curJoint = skinWeight->infJoint[k];
					if (boneJointIdxMap.find(jointName) == boneJointIdxMap.end()) // new joint
					{
						boneJointIdxMap[jointName] = nTotalBones++;		
						boneJointList.push_back(curJoint);
						boneJointNameList.push_back(jointName);
						//bindPoseMatList.push_back(skinWeight->bindShapeMat*skinWeight->bindPoseMat[k]);
						bindPoseMatList.push_back(skinWeight->bindPoseMat[k]);
					}
				}
			}	
		}
	}
	else
	{
		for (size_t i=0;i<dMeshDynamic_p.size();i++)
			meshIndexList.push_back(i);
	}

	//LOG("meshIndexList.size() = %d \n", meshIndexList.size());
	for (unsigned int i=0;i<meshIndexList.size();i++)
	{
		int pos = meshIndexList[i];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		dMeshStatic->shape().computeTangentBiNormal();
		std::string meshName = (const char*)dMeshStatic->shape().name;
		std::vector<SrMaterial>& matList = dMeshDynamic->shape().M; 	
		std::map<std::string,std::string> mtlTexMap = dMeshDynamic->shape().mtlTextureNameMap;
		std::map<std::string,std::string> mtlNormalTexMap = dMeshDynamic->shape().mtlNormalTexNameMap;		
		std::map<std::string,std::string> mtlSpecularTexMap = dMeshDynamic->shape().mtlSpecularTexNameMap;		
		//LOG("meshIndexList %d, matList.size() = %d \n", i, matList.size());
		for (size_t j=0;j<matList.size();j++)
		{			
			SrMaterial& mat = matList[j];	
			std::string mtlName = dMeshDynamic->shape().mtlnames[j];
			if (mtlTexMap.find(mtlName) != mtlTexMap.end())
			{
				allTexNameList.push_back(mtlTexMap[mtlName]);					
			}
			else
			{
				allTexNameList.push_back("");
			}	

			if (mtlNormalTexMap.find(mtlName) != mtlNormalTexMap.end())
			{
				allNormalTexNameList.push_back(mtlNormalTexMap[mtlName]);
			}
			else
			{
				allNormalTexNameList.push_back("");
			}

			if (mtlSpecularTexMap.find(mtlName) != mtlSpecularTexMap.end())
			{
				allSpecularTexNameList.push_back(mtlSpecularTexMap[mtlName]);
			}
			else
			{
				allSpecularTexNameList.push_back("");
			}
			allMaterialMeshMap[mtlName] = meshName;
			allMatList.push_back(mat);
			allMatNameList.push_back(mtlName);
			//colorArray[j%6].get(fcolor);				
			meshSubsetMap[nMaterial] = std::vector<int>(); 
			nMaterial++;
		}
	}				
		

	//printf("num of mesh subset =  %d\n",meshSubsetMap.size());

	int iMaterialOffset = 1;	
	int iFaceIdxOffset = 0, iNormalIdxOffset = 0, iTextureIdxOffset = 0;
	int iFace = 0;
	SrVec3i defaultIdx = SrVec3i(-1,-1,-1);
	for (unsigned int i=0;i<meshIndexList.size();i++)
	{
		int pos = meshIndexList[i];
		
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];			
		int nVtx = dMeshStatic->shape().V.size();	
		int nFace = dMeshStatic->shape().F.size();
		int nNormal = dMeshStatic->shape().N.size();	
		int nTexture = dMeshStatic->shape().T.size();
		for (int i=0;i<nVtx;i++)
		{
			vtxNormalIdxMap.push_back(std::set<IntPair>());				
		}

		nTotalVtxs += nVtx;				
		nTotalTris += nFace;
		
		unsigned int numTris = dMeshStatic->shape().F.size();
		for (unsigned int i=0; i < numTris ; i++)
		{
			SrModel& model = dMeshStatic->shape();
			if (dMeshStatic->shape().F.size() == 0)
				continue;
			SrVec3i& faceIdx = dMeshStatic->shape().F[i];	
			SrVec3i nIdx;
			nIdx = faceIdx; 
			if (dMeshStatic->shape().Fn.size() != 0)
			{
				SrVec3i& fnIdx = dMeshStatic->shape().Fn[i];	
				nIdx = fnIdx;
			}
			//SrModel::Face& nIdx = dMeshStatic->shape().Fn[i];
			SrVec3i& tIdx = defaultIdx;
			if (model.Ft.size() > i)
				tIdx = model.Ft[i]; 

			if (tIdx[0] >= 0 || 
				tIdx[1] >= 0 || 
				tIdx[2] >= 0)
			{
				vtxNormalIdxMap[faceIdx[0] + iFaceIdxOffset].insert(IntPair(nIdx[0]+iNormalIdxOffset,tIdx[0]+iTextureIdxOffset));
				vtxNormalIdxMap[faceIdx[1] + iFaceIdxOffset].insert(IntPair(nIdx[1]+iNormalIdxOffset,tIdx[1]+iTextureIdxOffset));
				vtxNormalIdxMap[faceIdx[2] + iFaceIdxOffset].insert(IntPair(nIdx[2]+iNormalIdxOffset,tIdx[2]+iTextureIdxOffset));
			}

			int nMatIdx = 0; // if no corresponding materials, push into the default gray material group
			if (i < dMeshStatic->shape().Fm.size())
			{
				nMatIdx = dMeshStatic->shape().Fm[i] + iMaterialOffset;		
				std::map<int,std::vector<int> >::iterator iter = meshSubsetMap.find(nMatIdx);
				if (iter == meshSubsetMap.end())
				{
					LOG("Bad material index %d", nMatIdx);
					nMatIdx = 0;
				}
			}
			meshSubsetMap[nMatIdx].push_back(iFace);			
			iFace++;
		}
		iFaceIdxOffset += nVtx;
		iNormalIdxOffset += nNormal;
		iMaterialOffset += dMeshDynamic->shape().M.size();
		iTextureIdxOffset += nTexture;
		//printf("iMaterial Offset = %d\n",iMaterialOffset);			
	}

	LOG("nTotalVtxs = %d, nTotalTris = %d", nTotalVtxs, nTotalTris);

	if (nTotalVtxs == 0 || nTotalTris ==0)
		return false;

	vtxNewVtxIdxMap.clear();
	//printf("orig vtxs = %d\n",nTotalVtxs);
	for (unsigned int i=0;i<vtxNormalIdxMap.size();i++)
	{
		std::set<IntPair>& idxMap = vtxNormalIdxMap[i];
		if (idxMap.size() > 1)
		{
			//LOG("vtx %d, idxMap size = %d", i, idxMap.size());
			vtxNewVtxIdxMap[i] = std::vector<int>();
			std::set<IntPair>::iterator si = idxMap.begin();
			si++;
			for ( ;
				si != idxMap.end();
				si++)
			{
				vtxNewVtxIdxMap[i].push_back(nTotalVtxs);
				ntNewVtxIdxMap[*si] = nTotalVtxs;
				nTotalVtxs++;
			}
		}
	}
	printf("new vtxs = %d\n",nTotalVtxs);

	// temporary storage 
	posBuf.resize(nTotalVtxs);
	normalBuf.resize(nTotalVtxs); 
	tangentBuf.resize(nTotalVtxs); 
	binormalBuf.resize(nTotalVtxs);
	texCoordBuf.resize(nTotalVtxs);
	skinColorBuf.resize(nTotalVtxs);
	meshColorBuf.resize(nTotalVtxs);
	boneCountBuf.resize(nTotalVtxs);
	triBuf.resize(nTotalTris);	

#if USE_SKIN_WEIGHT_SIZE_8
	int skinWeightIter = 2;
#else
	int skinWeightIter = 1; 
#endif

	if (buildSkinnedBuffer)
	{
		
		for (int i=0;i<skinWeightIter;i++)
		{
			boneIDBuf[i].resize(nTotalVtxs);
			boneIDBuf_f[i].resize(nTotalVtxs);
			boneWeightBuf[i].resize(nTotalVtxs);
		}
	}
	

	int iVtx = 0;
	iFace = 0;
	iFaceIdxOffset = 0;
	iNormalIdxOffset = 0;
	iTextureIdxOffset = 0;		
	float step = 1.f/boneJointList.size();
	float floatBuf[4];
	SrRandom random;	

	boneColorMap.clear();
	for (unsigned int i=0;i<boneJointList.size();i++)
	{
		//SrColor boneColor = SrColor::interphue(random.getf());
		SrColor boneColor = SrColor::interphue(i*step);
		boneColor.get(floatBuf);
// 		if (i == 3)
// 			boneColorMap.push_back(SrVec(1.f,0.f,0.f));
// 		else
		boneColorMap.push_back(SrVec(floatBuf[0],floatBuf[1],floatBuf[2]));				
	}	
	
	for (unsigned int c=0;c<meshIndexList.size();c++)
	{
		int pos = meshIndexList[c];
		int globalCounter = 0;
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
		dMeshDynamic->visible(false);
		unsigned int numVertices = dMeshStatic->shape().V.size();
		unsigned int numNormals = dMeshStatic->shape().N.size();
		unsigned int numTexCoords = dMeshStatic->shape().T.size();
		SrMat bindShapeMat;
		SkinWeight* skinWeight = NULL;
		if (buildSkinnedBuffer)
		{
			skinWeight = skinWeightList[c];
			bindShapeMat = skinWeight->bindShapeMat;
		}
		for (unsigned int i = 0; i < numVertices; i++)
		{
			
			if (buildSkinnedBuffer)
			{				
				if (i >= (int) skinWeight->numInfJoints.size())
					continue;				
			}
			SrVec& lv = dMeshStatic->shape().V[i];	
			//if (i % 1000 == 0)
			//	LOG("mesh vtx %d = %s", i, lv.toString().c_str());
			posBuf[iVtx] = lv*bindShapeMat;
			SrVec meshColor = SrVec(1.f,1.f,1.f);
			if (i < dMeshStatic->shape().Vc.size())
			{
				meshColor = dMeshStatic->shape().Vc[i];
				hasVertexColor = true;
			}			
			if (dMeshStatic->shape().T.size() > 0)
			{
				hasTexCoord = true;
			}

			meshColorBuf[iVtx] = meshColor;

			SrVec& lt =	dMeshStatic->shape().Tangent[i];		
			SrVec& lb = dMeshStatic->shape().BiNormal[i];
			tangentBuf[iVtx] = lt*bindShapeMat;
			binormalBuf[iVtx] = lb*bindShapeMat;
				
			//normalBuffer(iVtx) = Vec3f(ln[0],ln[1],ln[2]);

			if (buildSkinnedBuffer && skinWeight)
			{
				int numOfInfJoints = skinWeight->numInfJoints[i];
				for (int k=0;k<skinWeightIter;k++)
				{
					boneIDBuf[k][iVtx] = SrVec4i(0,0,0,0);
					boneIDBuf_f[k][iVtx] = SrVec4(0,0,0,0);
					boneWeightBuf[k][iVtx] = SrVec4(0,0,0,0);
				}
				std::vector<IntFloatPair> weightList;
				if (skinWeight->infJointName.size() > 0)
				{
					for (int j = 0; j < numOfInfJoints; j++)
					{
						const std::string& curJointName = skinWeight->infJointName[skinWeight->jointNameIndex[globalCounter]];					
						float jointWeight = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
						int    jointIndex  = boneJointIdxMap[curJointName];
						// 					 					if (numOfInfJoints > 8)
						// 					 						printf("w = %d : %f\n",jointIndex,jointWeight);
						weightList.push_back(IntFloatPair(jointIndex,jointWeight));							
						globalCounter ++;									
					}
				}
				std::sort(weightList.begin(),weightList.end(),intFloatComp); // sort for minimum weight
				int numWeight = numOfInfJoints > 8 ? 8 : numOfInfJoints;
				float weightSum = 0.f;
				SrVec skinColor;
				boneCountBuf[iVtx] = numWeight;
				for (int j=0;j<numWeight;j++)
				{
					if (j >= (int)weightList.size())
						continue;
					IntFloatPair& w = weightList[j];
					
					if ( j < 4)
					{
						boneIDBuf[0][iVtx][j] = w.first;
						boneIDBuf_f[0][iVtx][j] = (float)w.first;
						boneWeightBuf[0][iVtx][j] = w.second;
						weightSum += w.second;
					}
					else if (j < 8)
					{
#if USE_SKIN_WEIGHT_SIZE_8
						boneIDBuf[1][iVtx][j-4] = w.first;
						boneIDBuf_f[1][iVtx][j-4] = (float)w.first;
						boneWeightBuf[1][iVtx][j-4] = w.second;
						weightSum += w.second;
#endif
					}	

					if (w.first >= 0 && w.first < (int) boneJointList.size())
					{
						skinColor += boneColorMap[w.first]*w.second;
					}
				}
				skinColorBuf[iVtx] = skinColor;

				for (int j=0;j<4;j++)
				{
					boneWeightBuf[0][iVtx][j] /= weightSum;
#if USE_SKIN_WEIGHT_SIZE_8
					boneWeightBuf[1][iVtx][j] /= weightSum;
#endif
				}	

			}
			

			if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
			{
				std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
				// copy related vtx components 
				for (unsigned int k=0;k<idxMap.size();k++)
				{
					posBuf[idxMap[k]] = posBuf[iVtx];
					meshColorBuf[idxMap[k]] = meshColorBuf[iVtx];
					tangentBuf[idxMap[k]] = tangentBuf[iVtx];
					binormalBuf[idxMap[k]] = binormalBuf[iVtx];

					if (buildSkinnedBuffer)
					{
						boneIDBuf[0][idxMap[k]] = boneIDBuf[0][iVtx];						
						boneIDBuf_f[0][idxMap[k]] = boneIDBuf_f[0][iVtx];
						boneWeightBuf[0][idxMap[k]] = boneWeightBuf[0][iVtx];
#if USE_SKIN_WEIGHT_SIZE_8
						boneIDBuf[1][idxMap[k]] = boneIDBuf[1][iVtx];
						boneIDBuf_f[1][idxMap[k]] = boneIDBuf_f[1][iVtx];	
						boneWeightBuf[1][idxMap[k]] = boneWeightBuf[1][iVtx];						
#endif						
						skinColorBuf[idxMap[k]] = skinColorBuf[iVtx];
						boneCountBuf[idxMap[k]] = boneCountBuf[iVtx];
					}	
				}
			}
			iVtx++;
		}

		size_t numTris = dMeshStatic->shape().F.size();
		for (size_t i=0; i < numTris ; i++)
		{
			if (dMeshStatic->shape().F.size() <= i)
				continue;				
			SrVec3i& faceIdx = dMeshStatic->shape().F[i];
			if (faceIdx[0] < 0 ||
				faceIdx[1] < 0 ||
				faceIdx[2] < 0)
				continue;
			SrVec3i normalIdx = faceIdx;
			
			if (dMeshStatic->shape().Fn.size() > i)
			{
				normalIdx = dMeshStatic->shape().Fn[i];							
			}
			SrVec3i& texCoordIdx = defaultIdx;
			if (dMeshStatic->shape().Ft.size() > i)
				texCoordIdx = dMeshStatic->shape().Ft[i];
			SrVec3i fIdx = faceIdx; //{ faceIdx.a, faceIdx.b, faceIdx.c};
			SrVec3i nIdx = normalIdx; //{ normalIdx.a, normalIdx.b, normalIdx.c};
			SrVec3i tIdx = texCoordIdx; //{ texCoordIdx.a, texCoordIdx.b, texCoordIdx.c};
				
			SrVec faceNormal = dMeshStatic->shape().face_normal(i);
			for (int k=0;k<3;k++)
			{
				SrVec nvec;
				SrPnt2 tvec	= SrPnt2(0,0);
				unsigned int nidx	= nIdx[k];

				if (dMeshStatic->shape().N.size() > nidx &&
					nidx > 0)
					nvec = dMeshStatic->shape().N[nIdx[k]];
				else
					nvec = faceNormal;

				if (dMeshStatic->shape().T.size() > tIdx[k] && 
					dMeshStatic->shape().T.size() > 0 && 
					dMeshStatic->shape().Ft.size() > 0)
					tvec = dMeshStatic->shape().T[tIdx[k]];

				int newNIdx = nIdx[k] + iNormalIdxOffset;
				int newTIdx	= tIdx[k] + iTextureIdxOffset;
				int vIdx	= fIdx[k] + iFaceIdxOffset;

				if (ntNewVtxIdxMap.find(IntPair(newNIdx,newTIdx)) != ntNewVtxIdxMap.end())
					vIdx = ntNewVtxIdxMap[IntPair(newNIdx,newTIdx)];

				normalBuf[vIdx]		= nvec;
				texCoordBuf[vIdx]	= SrVec2(tvec.x, tvec.y);
				triBuf[iFace][k]	= vIdx;
			}			
			iFace++;
		}
		iFaceIdxOffset += numVertices;
		iNormalIdxOffset += numNormals;
		iTextureIdxOffset += numTexCoords;		
	}
    
	int group = 0;
	std::vector<SbmSubMesh*> hairMeshList;
	std::vector<SbmSubMesh*> alphaMeshList;
	std::map<int,std::vector<int> >::iterator vi;
	subMeshList.clear();
	LOG("subMeshList.size() = %d \n", meshSubsetMap.size());
	for (vi  = meshSubsetMap.begin();
		vi != meshSubsetMap.end();
		vi++)
	{
		int iMaterial = vi->first;

		std::vector<int>& faceIdxList = vi->second;	
		if (faceIdxList.size() == 0)
			continue;		
		SbmSubMesh* mesh = new SbmSubMesh();
		mesh->isHair = false;
		mesh->material = allMatList[iMaterial];		
		mesh->texName  = allTexNameList[iMaterial];
		mesh->normalMapName = allNormalTexNameList[iMaterial];		
		mesh->specularMapName = allSpecularTexNameList[iMaterial];
		mesh->numTri = faceIdxList.size();
		mesh->triBuf.resize(faceIdxList.size());	
		mesh->faceIdxList = faceIdxList;
		for (unsigned int k=0;k<faceIdxList.size();k++)
		{
			mesh->triBuf[k][0] = triBuf[faceIdxList[k]][0];
			mesh->triBuf[k][1] = triBuf[faceIdxList[k]][1];
			mesh->triBuf[k][2] = triBuf[faceIdxList[k]][2];
		}

		mesh->matName = allMatNameList[iMaterial];
		mesh->modelName = allMaterialMeshMap[mesh->matName];
		SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, mesh->texName.c_str());
		//if (lowMatName.find("hair") != std::string::npos || lowMatName.find("lash") != std::string::npos 
		//	|| lowMatName.find("shadow") != std::string::npos || lowMatName.find("shell") != std::string::npos)
#if TEST_HAIR_RENDER		
		std::string lowMatName = mesh->matName;
		boost::algorithm::to_lower(lowMatName);		
		if (mesh->material.useAlphaBlend)
		{
			LOG("alpha mesh = %s",mesh->matName.c_str());
			alphaMeshList.push_back(mesh);
		}
		else
		{
			subMeshList.push_back(mesh);
		}
#else
		subMeshList.push_back(mesh);
#endif			
	}		
	subMeshList.insert(subMeshList.end(),hairMeshList.begin(),hairMeshList.end());
	subMeshList.insert(subMeshList.end(),alphaMeshList.begin(),alphaMeshList.end());

	buildBlendShapes();

	initStaticVertexBuffer = true;
	if (buildSkinnedBuffer)
		initSkinnedVertexBuffer = true;
	return true;
}

bool DeformableMesh::isSkinnedMesh()
{
	return skinWeights.size() > 0;
}


void DeformableMesh::saveToStaticMeshBinary(SmartBodyBinary::StaticMesh* outputStaticMesh)
{
	// 1	StaticMesh	
	outputStaticMesh->set_staticmeshname(getName());
	// 2
	std::vector<SrModel*> modelsToSave;
	// prepare models
	for (size_t i = 0; i < dMeshStatic_p.size(); ++i)
	{
		modelsToSave.push_back(&(dMeshStatic_p[i]->shape()));
	}
	// prepare morph targets
	for (std::map<std::string, std::vector<SrSnModel*> >::iterator iter = blendShapeMap.begin();
		iter != blendShapeMap.end();
		iter++)
	{
		std::vector<SrSnModel*>& targets = (*iter).second;
		for (size_t t = 1; t < targets.size(); t++) // ignore first target since it is a base mesh
		{
			SrModel& curModel = targets[t]->shape();
			modelsToSave.push_back(&curModel);
		}
	}

	for (size_t s = 0; s < modelsToSave.size(); s++)
	{
		SmartBodyBinary::MeshModel* newMeshModel = outputStaticMesh->add_meshmodels();
		// 1	MeshModel
		SrModel* curModel = modelsToSave[s];
		newMeshModel->set_meshname((const char*)curModel->name);
		// 2
		for (unsigned int m = 0; m < curModel->M.size(); ++m)
		{
			SmartBodyBinary::Material* newMaterial = newMeshModel->add_materials();
			// 1 Material
			newMaterial->add_ambient(curModel->M[m].ambient.r);
			newMaterial->add_ambient(curModel->M[m].ambient.g);
			newMaterial->add_ambient(curModel->M[m].ambient.b);
			newMaterial->add_ambient(curModel->M[m].ambient.a);
			// 2
			newMaterial->add_diffuse(curModel->M[m].diffuse.r);
			newMaterial->add_diffuse(curModel->M[m].diffuse.g);
			newMaterial->add_diffuse(curModel->M[m].diffuse.b);
			newMaterial->add_diffuse(curModel->M[m].diffuse.a);
			// 3
			newMaterial->add_specular(curModel->M[m].specular.r);
			newMaterial->add_specular(curModel->M[m].specular.g);
			newMaterial->add_specular(curModel->M[m].specular.b);
			newMaterial->add_specular(curModel->M[m].specular.a);
			// 4
			newMaterial->add_emission(curModel->M[m].emission.r);
			newMaterial->add_emission(curModel->M[m].emission.g);
			newMaterial->add_emission(curModel->M[m].emission.b);
			newMaterial->add_emission(curModel->M[m].emission.a);
			// 5
			newMaterial->set_shininess(curModel->M[m].shininess);
			// 6
			newMaterial->set_transparency(curModel->M[m].transparency);
			// 7
			newMaterial->set_usealphablend(curModel->M[m].useAlphaBlend);

		}
		// 3
		for (unsigned int v = 0; v < curModel->V.size(); ++v)
		{
			newMeshModel->add_vertexcoordinates(curModel->V[v].x);
			newMeshModel->add_vertexcoordinates(curModel->V[v].y);
			newMeshModel->add_vertexcoordinates(curModel->V[v].z);
		}
		// 4
		for (unsigned int n = 0; n < curModel->N.size(); ++n)
		{
			newMeshModel->add_normals(curModel->N[n].x);
			newMeshModel->add_normals(curModel->N[n].y);
			newMeshModel->add_normals(curModel->N[n].z);
		}
		// 5
		for (unsigned int n = 0; n < curModel->Tangent.size(); ++n)
		{
			newMeshModel->add_tangents(curModel->Tangent[n].x);
			newMeshModel->add_tangents(curModel->Tangent[n].y);
			newMeshModel->add_tangents(curModel->Tangent[n].z);
		}
		// 5
		for (unsigned int n = 0; n < curModel->BiNormal.size(); ++n)
		{
			newMeshModel->add_binormals(curModel->BiNormal[n].x);
			newMeshModel->add_binormals(curModel->BiNormal[n].y);
			newMeshModel->add_binormals(curModel->BiNormal[n].z);
		}
		//7
		for (unsigned int t = 0; t < curModel->T.size(); ++t)
		{
			newMeshModel->add_texturecoordinates(curModel->T[t].x);
			newMeshModel->add_texturecoordinates(curModel->T[t].y);
		}
		//8
		for (unsigned int t = 0; t < curModel->F.size(); ++t)
		{
			newMeshModel->add_trianglefaceindices(curModel->F[t][0]);
			newMeshModel->add_trianglefaceindices(curModel->F[t][1]);
			newMeshModel->add_trianglefaceindices(curModel->F[t][2]);
		}
		// 9
		for (unsigned int t = 0; t < curModel->Fm.size(); ++t)
		{
			newMeshModel->add_materialindices(curModel->Fm[t]);
		}
		// 10
		for (unsigned int t = 0; t < curModel->Fn.size(); ++t)
		{
			newMeshModel->add_normalindices(curModel->Fn[t][0]);
			newMeshModel->add_normalindices(curModel->Fn[t][1]);
			newMeshModel->add_normalindices(curModel->Fn[t][2]);
		}
		// 11
		for (unsigned int t = 0; t < curModel->Ft.size(); ++t)
		{
			newMeshModel->add_texturecoordinatesindices(curModel->Ft[t][0]);
			newMeshModel->add_texturecoordinatesindices(curModel->Ft[t][1]);
			newMeshModel->add_texturecoordinatesindices(curModel->Ft[t][2]);
		}
		// 12
		newMeshModel->set_culling(curModel->culling);
		// 13
		for (unsigned int m = 0; m < curModel->mtlnames.size(); ++m)
		{
			newMeshModel->add_materialnames( curModel->mtlnames[m].c_str());
		}
		// 14
		std::map<std::string,std::string>::iterator iter;
		for (iter = curModel->mtlTextureNameMap.begin(); iter != curModel->mtlTextureNameMap.end(); ++iter)
		{
			SmartBodyBinary::StringToStringMap* m2DiffuseMapping = newMeshModel->add_materialtodiffusetexturemapping();
			m2DiffuseMapping->set_from(iter->first);
			m2DiffuseMapping->set_to(iter->second);
		}
		// 15
		for (iter = curModel->mtlNormalTexNameMap.begin(); iter != curModel->mtlNormalTexNameMap.end(); ++iter)
		{
			SmartBodyBinary::StringToStringMap* m2NormalMapping = newMeshModel->add_materialtonormaltexturemapping();
			m2NormalMapping->set_from(iter->first);
			m2NormalMapping->set_to(iter->second);
		}
		// 16
		for (iter = curModel->mtlSpecularTexNameMap.begin(); iter != curModel->mtlSpecularTexNameMap.end(); ++iter)
		{
			SmartBodyBinary::StringToStringMap* m2SpecularMapping = newMeshModel->add_materialtospeculartexturemapping();
			m2SpecularMapping->set_from(iter->first);
			m2SpecularMapping->set_to(iter->second);
		}
		// 17
		std::map<std::string,std::vector<int> >::iterator iter1;
		for (iter1 = curModel->mtlFaceIndices.begin(); iter1 != curModel->mtlFaceIndices.end(); ++iter1)
		{
			SmartBodyBinary::StringToIntVectorMap* m2FaceIndicesMapping = newMeshModel->add_materialtofaceindices();
			m2FaceIndicesMapping->set_from(iter1->first);
			for (size_t f = 0; f < iter1->second.size(); ++f)
				m2FaceIndicesMapping->add_to(iter1->second[f]);
		}
		// 18
		for (iter = curModel->mtlTransparentTexNameMap.begin(); iter != curModel->mtlTransparentTexNameMap.end(); ++iter)
		{
			SmartBodyBinary::StringToStringMap* m2TransparentMapping = newMeshModel->add_materialtotransparenttexturemapping();
			m2TransparentMapping->set_from(iter->first);
			m2TransparentMapping->set_to(iter->second);
		}
		// 19
		for (iter = curModel->mtlGlossyTexNameMap.begin(); iter != curModel->mtlGlossyTexNameMap.end(); ++iter)
		{
			SmartBodyBinary::StringToStringMap* m2GlossyMapping = newMeshModel->add_materialtoglossytexturemapping();
			m2GlossyMapping->set_from(iter->first);
			m2GlossyMapping->set_to(iter->second);
		}
	}
	
}

void DeformableMesh::readFromStaticMeshBinary(SmartBodyBinary::StaticMesh* mesh, std::vector<SrModel*>& models, std::string file)
{
	SmartBodyBinary::StaticMesh staticMesh = *mesh;

	// 1 StaticMesh
	this->setName(staticMesh.staticmeshname());
	for (int numMeshModels = 0; numMeshModels < staticMesh.meshmodels_size(); ++numMeshModels)
	{
		const SmartBodyBinary::MeshModel& meshModel = staticMesh.meshmodels(numMeshModels);
		SrModel* newModel = new SrModel();
		// 1 MeshModel
		newModel->name.set(meshModel.meshname().c_str());
		// 2
		for (int numMaterials = 0; numMaterials < meshModel.materials_size(); ++numMaterials)
		{
			const SmartBodyBinary::Material& material = meshModel.materials(numMaterials);
			SrMaterial newMat;
			// 1 Material
			newMat.ambient.r = material.ambient(0);
			newMat.ambient.g = material.ambient(1);
			newMat.ambient.b = material.ambient(2);
			newMat.ambient.a = material.ambient(3);
			// 2
			newMat.diffuse.r = material.diffuse(0);
			newMat.diffuse.g = material.diffuse(1);
			newMat.diffuse.b = material.diffuse(2);
			newMat.diffuse.a = material.diffuse(3);
			if (newMat.diffuse.a < 1.0)
			{
				LOG("mesh %d, mat %d, useAlphaBlending",numMeshModels, numMaterials);
				newMat.useAlphaBlend = true;
			}
			// 3
			newMat.specular.r = material.specular(0);
			newMat.specular.g = material.specular(1);
			newMat.specular.b = material.specular(2);
			newMat.specular.a = material.specular(3);
			// 4
			newMat.emission.r = material.emission(0);
			newMat.emission.g = material.emission(1);
			newMat.emission.b = material.emission(2);
			newMat.emission.a = material.emission(3);
			// 5
			newMat.shininess = material.shininess();

			// 6 transparency
			newMat.transparency = material.transparency();

			// 7 useAlphaBlend
			newMat.useAlphaBlend = material.usealphablend();

			newModel->M.push_back(newMat);
		}
		// 3
		for (int x = 0; x < meshModel.vertexcoordinates_size() / 3; ++x)
		{
			SrPnt newPoint(meshModel.vertexcoordinates(x * 3 + 0), meshModel.vertexcoordinates(x * 3 + 1), meshModel.vertexcoordinates(x * 3 + 2));
			newModel->V.push_back(newPoint);
		}
		// 4
		for (int x = 0; x < meshModel.normals_size() / 3; ++x)
		{
			SrPnt newPoint(meshModel.normals(x * 3 + 0), meshModel.normals(x * 3 + 1), meshModel.normals(x * 3 + 2));
			newModel->N.push_back(newPoint);
		}
		// 5
		for (int x = 0; x < meshModel.tangents_size() / 3; ++x)
		{
			SrPnt newPoint(meshModel.tangents(x * 3 + 0), meshModel.tangents(x * 3 + 1), meshModel.tangents(x * 3 + 2));
			newModel->Tangent.push_back(newPoint);
		}
		// 6
		for (int x = 0; x < meshModel.binormals_size() / 3; ++x)
		{
			SrPnt newPoint(meshModel.binormals(x * 3 + 0), meshModel.binormals(x * 3 + 1), meshModel.binormals(x * 3 + 2));
			newModel->BiNormal.push_back(newPoint);
		}
		// 7
		for (int x = 0; x < meshModel.texturecoordinates_size() / 2; ++x)
		{
			SrPnt2 newPoint(meshModel.texturecoordinates(x * 2 + 0), meshModel.texturecoordinates(x * 2 + 1));
			newModel->T.push_back(newPoint);
		}
		// 8
		for (int x = 0; x < meshModel.trianglefaceindices_size() / 3; ++x)
		{
			SrVec3i newFace;
			newFace[0] = meshModel.trianglefaceindices(x * 3 + 0);
			newFace[1] = meshModel.trianglefaceindices(x * 3 + 1);
			newFace[2] = meshModel.trianglefaceindices(x * 3 + 2);
			newModel->F.push_back(newFace);
		}
		// 9
		for (int x = 0; x < meshModel.materialindices_size(); ++x)
		{
			newModel->Fm.push_back(meshModel.materialindices(x));
		}
		// 10
		for (int x = 0; x < meshModel.normalindices_size() / 3; ++x)
		{
			SrVec3i newFace;
			newFace[0] = meshModel.normalindices(x * 3 + 0);
			newFace[1] = meshModel.normalindices(x * 3 + 1);
			newFace[2] = meshModel.normalindices(x * 3 + 2);
			newModel->Fn.push_back(newFace);
		}
		// 11
		for (int x = 0; x < meshModel.texturecoordinatesindices_size() / 3; ++x)
		{
			SrVec3i newFace;
			newFace[0] = meshModel.texturecoordinatesindices(x * 3 + 0);
			newFace[1] = meshModel.texturecoordinatesindices(x * 3 + 1);
			newFace[2] = meshModel.texturecoordinatesindices(x * 3 + 2);
			newModel->Ft.push_back(newFace);
		}
		// 12
		newModel->culling = meshModel.culling();
		// 13
		for (int x = 0; x < meshModel.materialnames_size(); ++x)
		{
			newModel->mtlnames.push_back(meshModel.materialnames(x).c_str());
		}
		// 14
		for (int x = 0; x < meshModel.materialtodiffusetexturemapping_size(); ++x)
		{
			const SmartBodyBinary::StringToStringMap& m2d = meshModel.materialtodiffusetexturemapping(x);
			newModel->mtlTextureNameMap.insert(std::make_pair(m2d.from(), m2d.to()));
		}
		// 15
		for (int x = 0; x < meshModel.materialtonormaltexturemapping_size(); ++x)
		{
			const SmartBodyBinary::StringToStringMap& m2d = meshModel.materialtonormaltexturemapping(x);
			newModel->mtlNormalTexNameMap.insert(std::make_pair(m2d.from(), m2d.to()));
		}
		// 16
		for (int x = 0; x < meshModel.materialtospeculartexturemapping_size(); ++x)
		{
			const SmartBodyBinary::StringToStringMap& m2d = meshModel.materialtospeculartexturemapping(x);
			newModel->mtlSpecularTexNameMap.insert(std::make_pair(m2d.from(), m2d.to()));
		}
		// 17
		for (int x = 0; x < meshModel.materialtofaceindices_size(); ++x)
		{
			const SmartBodyBinary::StringToIntVectorMap& m2d = meshModel.materialtofaceindices(x);
			std::vector<int> indices;
			for (int z = 0; z < m2d.to_size(); ++z)
				indices.push_back(m2d.to(z));
			newModel->mtlFaceIndices.insert(std::make_pair(m2d.from(), indices));
		}
		// 18
		for (int x = 0; x < meshModel.materialtotransparenttexturemapping_size(); ++x)
		{
			const SmartBodyBinary::StringToStringMap& m2d = meshModel.materialtotransparenttexturemapping(x);
			newModel->mtlTransparentTexNameMap.insert(std::make_pair(m2d.from(), m2d.to()));
		}
		// 19
		for (int x = 0; x < meshModel.materialtoglossytexturemapping_size(); ++x)
		{
			const SmartBodyBinary::StringToStringMap& m2d = meshModel.materialtoglossytexturemapping(x);
			newModel->mtlGlossyTexNameMap.insert(std::make_pair(m2d.from(), m2d.to()));
		}

		// bake transparent and glossy maps into diffuse and specular maps
		SrString path = file.c_str();
		SrString filename;
		path.extract_file_name(filename);
		SrStringArray paths;
		paths.push(path);

		SbmTextureManager& texManager = SbmTextureManager::singleton();

		for (size_t i = 0; i < newModel->M.size(); i++)
		{
			std::string matName = newModel->mtlnames[i];			
			if (newModel->mtlTextureNameMap.find(matName) != newModel->mtlTextureNameMap.end())
			{
				std::string prefixedName = newModel->mtlTextureNameMap[matName];
				int index = prefixedName.find_first_of("|");
				std::string fileName = prefixedName;
				if (index >= 0)
					fileName = prefixedName.substr(index + 1);
				ParserCOLLADAFast::load_texture(SbmTextureManager::TEXTURE_DIFFUSE, prefixedName.c_str(), fileName.c_str(), paths);

				if (newModel->mtlTransparentTexNameMap.find(matName) != newModel->mtlTransparentTexNameMap.end())
				{
					SbmTexture* diffuseTex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, prefixedName.c_str());
					SbmTexture* transTex = new SbmTexture("Dummy");
					std::string transpTexFile = (*newModel->mtlTransparentTexNameMap.find(matName)).second;
					std::string finalTextureName = ParserCOLLADAFast::getFinalTextureFileName(transpTexFile, paths);
					transTex->loadImage(finalTextureName.c_str());
					if (diffuseTex && transTex)
					{
						diffuseTex->bakeAlphaIntoTexture(transTex);
					}
				}
				newModel->mtlTextureNameMap[matName] = prefixedName;
			}

			if (newModel->mtlSpecularTexNameMap.find(matName) != newModel->mtlSpecularTexNameMap.end())
			{
				std::string prefixedName = newModel->mtlSpecularTexNameMap[matName];
				int index = prefixedName.find_first_of("|");
				std::string fileName = prefixedName;
				if (index >= 0)
					fileName = prefixedName.substr(index + 1);
				ParserCOLLADAFast::load_texture(SbmTextureManager::TEXTURE_SPECULARMAP, prefixedName.c_str(), fileName.c_str(), paths);

				if (newModel->mtlGlossyTexNameMap.find(matName) != newModel->mtlGlossyTexNameMap.end())
				{
					SbmTexture* specularTex = texManager.findTexture(SbmTextureManager::TEXTURE_SPECULARMAP, prefixedName.c_str());
					SbmTexture* glossyTex = new SbmTexture("Dummy");
					std::string glossyTexFile = (*newModel->mtlGlossyTexNameMap.find(matName)).second;
					std::string finalTextureName = ParserCOLLADAFast::getFinalTextureFileName(glossyTexFile, paths);
					glossyTex->loadImage(finalTextureName.c_str());
					if (specularTex && glossyTex)
					{
						specularTex->bakeAlphaIntoTexture(glossyTex);
					}
				}

				newModel->mtlSpecularTexNameMap[matName] = prefixedName;
			}
		}
		
		if (newModel->Fn.size() == 0)
		{
			newModel->computeNormals();
		}

		models.push_back(newModel);
	}
}


SBAPI void DeformableMesh::addTransform( const SrMat& transform )
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().addTransform(transform);
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().addTransform(transform);
	}
}


void DeformableMesh::translate(SrVec trans)
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().translate(trans);
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().translate(trans);
	}
}

void DeformableMesh::rotate(SrVec rot)
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().rotate(rot);
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().rotate(rot);
	}
}

void DeformableMesh::scale(float factor)
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().scale(factor);
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().scale(factor);
	}
}

void DeformableMesh::centralize()
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().centralize();
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().centralize();
	}
}

void DeformableMesh::computeNormals()
{
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshDynamic_p[i]->shape().computeNormals();
	}
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
	{
		dMeshStatic_p[i]->shape().computeNormals();
	}
}


bool DeformableMesh::saveToSmb(std::string inputFileName)
{
	SmartBodyBinary::StaticMesh* outputStaticMesh = new SmartBodyBinary::StaticMesh();
	saveToStaticMeshBinary(outputStaticMesh);
	// set properly the binary static mesh to be output file name (fileName + extension)
	boost::filesystem::path p(inputFileName);
	std::string fileName = boost::filesystem::basename(p);
	std::string extension =  boost::filesystem::extension(p);
	if (extension != ".smb")
	{
		extension = ".smb";
		inputFileName = fileName + extension;
	}
	outputStaticMesh->set_staticmeshname(fileName + extension);
	
	// TODO: copy the textures out if needed

	std::fstream file(inputFileName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outputStaticMesh->SerializeToOstream(&file)) 
	{
		LOG("Fail to write to binary file %s", inputFileName.c_str());
		return false;
	}
	google::protobuf::ShutdownProtobufLibrary();

	return true;
}

bool DeformableMesh::saveToDmb(std::string inputFileName)
{
	if (!isSkinnedMesh())
	{
		LOG("mesh %s is a static mesh, skip saving to %s", this->getName().c_str(), inputFileName.c_str());
		return false;
	}
	boost::filesystem::path p(inputFileName);
	std::string fileName = boost::filesystem::basename(p);
	std::string extension =  boost::filesystem::extension(p);
	if (extension != ".dmb")
	{
		extension = ".dmb";
		inputFileName = fileName + extension;
	}

	SmartBodyBinary::DeformableMesh* outputDeformableMesh = new SmartBodyBinary::DeformableMesh();
	// 1 DeformableMesh
	outputDeformableMesh->set_deformablemeshname(fileName + extension);
	// 2
	SmartBodyBinary::StaticMesh* outputStaticMesh = outputDeformableMesh->mutable_staticmesh(); 
	saveToStaticMeshBinary(outputStaticMesh);
	outputStaticMesh->set_staticmeshname(fileName + extension);

	// save skin weights
	for (size_t i = 0; i < skinWeights.size(); ++i)
	{
		SmartBodyBinary::SkinWeight* newSkinWeight = outputDeformableMesh->add_skinweights();
		// 1 SkinWeight
		newSkinWeight->set_sourcemeshname(skinWeights[i]->sourceMesh);
		// 2
		for (size_t x = 0; x < skinWeights[i]->infJointName.size(); ++x)
		{
			newSkinWeight->add_influencejointnames(skinWeights[i]->infJointName[x]);
		}
		// 3
		for (size_t x = 0; x < skinWeights[i]->bindWeight.size(); ++x)
		{
			newSkinWeight->add_bindweights(skinWeights[i]->bindWeight[x]);
		}
		// 4
		for (size_t x = 0; x < skinWeights[i]->bindPoseMat.size(); ++x)
		{
			const SrMat& mat = skinWeights[i]->bindPoseMat[x];
			for (int m = 0; m < 16; ++m)
				newSkinWeight->add_bindposematrice(mat.get(m));
		}
		// 5
		for (int m = 0; m < 16; ++m)
			newSkinWeight->add_bindshapematrix(skinWeights[i]->bindShapeMat.get(m));
		// 6
		for (size_t x = 0; x < skinWeights[i]->numInfJoints.size(); ++x)
			newSkinWeight->add_numberofinfluencejoints(skinWeights[i]->numInfJoints[x]);
		// 7
		for (size_t x = 0; x < skinWeights[i]->weightIndex.size(); ++x)
			newSkinWeight->add_weightindices(skinWeights[i]->weightIndex[x]);
		// 8
		for (size_t x = 0; x < skinWeights[i]->jointNameIndex.size(); ++x)
			newSkinWeight->add_jointnameindices(skinWeights[i]->jointNameIndex[x]);
	}
	
	// save morph targets
	std::map<std::string, std::vector<std::string> >::iterator iter;
	for (iter = this->morphTargets.begin(); iter != this->morphTargets.end(); ++iter)
	{
		SmartBodyBinary::StringToStringVectorMap* morphMap = outputDeformableMesh->add_morphtargets();
		morphMap->set_from(iter->first);
		for (size_t x = 0; x < iter->second.size(); ++x)
		{
			morphMap->add_to(iter->second[x]);
		}
	}
	

	std::fstream file(inputFileName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outputDeformableMesh->SerializeToOstream(&file)) 
	{
		LOG("Fail to write to binary file %s", inputFileName.c_str());
		return false;
	}
	google::protobuf::ShutdownProtobufLibrary();
	return true;
}

bool DeformableMesh::readFromSmb(std::string inputFileName)
{
	SmartBodyBinary::StaticMesh staticMesh;

	std::fstream input(inputFileName.c_str(), std::ios::in | std::ios::binary);
	google::protobuf::io::IstreamInputStream* raw_input = new google::protobuf::io::IstreamInputStream(&input);
	google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(raw_input);
	coded_input->SetTotalBytesLimit(256000000, 256000000);
	
	if (!staticMesh.ParseFromCodedStream(coded_input))
	{
		LOG("Failed to parse binary static mesh from file %s", inputFileName.c_str());
		return false;
	}

	std::vector<SrModel*> models;
	readFromStaticMeshBinary(&staticMesh, models, inputFileName);

	for (size_t m = 0; m > models.size(); m++)
	{
		SrSnModel* srSnModelStatic = new SrSnModel();
		srSnModelStatic->shape(*models[m]);
		srSnModelStatic->shape().name = models[m]->name;
		srSnModelStatic->ref();
		
		dMeshStatic_p.push_back(srSnModelStatic);
		
		SrSnModel* srSnModelDynamic = new SrSnModel();
		srSnModelDynamic->shape(*(models[m]));
		srSnModelDynamic->changed(true);
		srSnModelDynamic->visible(false);
		srSnModelDynamic->shape().name = models[m]->name;
		srSnModelDynamic->ref();
		
		dMeshDynamic_p.push_back(srSnModelDynamic);
		//delete models[m];
	}

	// explicitly load all the textures
	boost::filesystem::path p(inputFileName);
	std::string filePath = p.parent_path().string();
	loadAllFoundTextures(filePath);
	
	return true;
}

bool DeformableMesh::readFromDmb(std::string inputFileName)
{
	SmartBodyBinary::DeformableMesh deformableMesh;

	std::fstream input(inputFileName.c_str(), std::ios::in | std::ios::binary);
	google::protobuf::io::IstreamInputStream* raw_input = new google::protobuf::io::IstreamInputStream(&input);
	google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(raw_input);
	coded_input->SetTotalBytesLimit(256000000, 256000000);

	if (!deformableMesh.ParseFromCodedStream(coded_input))
	{
		LOG("Failed to parse binary deformable mesh from file %s", inputFileName.c_str());
		return false;
	}

	SmartBodyBinary::StaticMesh staticMesh = deformableMesh.staticmesh();

	std::vector<SrModel*> models;
	readFromStaticMeshBinary(&staticMesh, models, inputFileName);

	// don't write into static/dynamic models until we'll sure which models
	// are base shapes and which are morphs. Morphs fo in the blendShapeMap

	// make the models easier to access
	std::map<std::string, SrSnModel*> modelMap;
	std::map<std::string, bool> modelsUsed;
	for (size_t m = 0; m < models.size(); m++)
	{
		SrSnModel* srSnModelStatic = new SrSnModel();
		srSnModelStatic->shape(*models[m]);
		srSnModelStatic->shape().name = models[m]->name;
		modelMap.insert(std::pair<std::string, SrSnModel*>(std::string(models[m]->name), srSnModelStatic));
		modelsUsed.insert(std::pair<std::string, bool>(std::string(models[m]->name), false));
	}
	// keep track of the models used for morphs; non-morph models are assumed to be part of the basic mesh

	// load morph targets
	for (int i = 0; i < deformableMesh.morphtargets_size(); ++i)
	{
		const SmartBodyBinary::StringToStringVectorMap& morphMap = deformableMesh.morphtargets(i);
		std::vector<std::string> morphs;
		
		for (int x = 0; x < morphMap.to_size(); ++x)
		{
			morphs.push_back(morphMap.to(x));
		}
		// morphTargets contains a morphgroup->morphname relationship
		morphTargets.insert(std::make_pair(morphMap.from(), morphs));

		// blendshapemap contains the morphgroup->model relationship
		// add the accompanying models
		std::vector<SrSnModel*> morphModels;
		for (size_t n = 0; n < morphs.size(); n++)
		{
			std::map<std::string, SrSnModel*>::iterator iter = modelMap.find(morphs[n]);
			if (iter != modelMap.end())
			{
				SrSnModel* model = (*iter).second;
				SrSnModel* baseModelCopy = new SrSnModel();
				baseModelCopy->shape(model->shape());
				baseModelCopy->ref();
				morphModels.push_back(baseModelCopy);
				if (n > 0) // mark non-base models as morph targets so that they are not added as static/dynamic meshes
					modelsUsed[morphs[n]] = true;
			}
		}
		if (morphs.size() > 0)
			blendShapeMap.insert(std::make_pair(morphs[0], morphModels));
	}

	for (std::vector<SrModel*>::iterator iter = models.begin();
		iter != models.end();
		iter++)
	{
		SrModel* model = (*iter);
		std::string modelName((const char*) model->name);

		std::map<std::string, bool>::iterator iter2 = modelsUsed.find(modelName);
		if ((*iter2).second == false)
		{
			std::map<std::string, SrSnModel*>::iterator iter3 = modelMap.find(modelName);
			SrSnModel* srsnmodel = (*iter3).second;
			dMeshStatic_p.push_back(srsnmodel);
			srsnmodel->ref();

			SrSnModel* srSnModelDynamic = new SrSnModel();
			srSnModelDynamic->shape(srsnmodel->shape());
			srSnModelDynamic->changed(true);
			srSnModelDynamic->visible(false);
			srSnModelDynamic->shape().name = srsnmodel->shape().name;
			
			dMeshDynamic_p.push_back(srSnModelDynamic);
			srSnModelDynamic->ref();
			//delete models[m];
		}
	}

	// load skin weights
	for (int i = 0; i < deformableMesh.skinweights_size(); ++i)
	{
		SkinWeight* newSkinWeights = new SkinWeight();
		// 1 SkinWeight
		newSkinWeights->sourceMesh = deformableMesh.skinweights(i).sourcemeshname();
		// 2
		for (int x = 0; x < deformableMesh.skinweights(i).influencejointnames_size(); ++x)
		{
			newSkinWeights->infJointName.push_back(deformableMesh.skinweights(i).influencejointnames(x));
		}
		// 3
		for (int x = 0; x < deformableMesh.skinweights(i).bindweights_size(); ++x)
		{
			newSkinWeights->bindWeight.push_back(deformableMesh.skinweights(i).bindweights(x));
		}
		// 4
		for (int x = 0; x < deformableMesh.skinweights(i).bindposematrice_size() / 16; ++x)
		{
			SrMat mat;
			for (int m = 0; m < 16; ++m)
			{
				mat.set(m, deformableMesh.skinweights(i).bindposematrice(x * 16 + m));
			}
			newSkinWeights->bindPoseMat.push_back(mat);
		}
		// 5
		for (int m = 0; m < deformableMesh.skinweights(i).bindshapematrix_size(); ++m)
		{
			newSkinWeights->bindShapeMat.set(m, deformableMesh.skinweights(i).bindshapematrix(m));
		}
		// 6
		for (int x = 0; x < deformableMesh.skinweights(i).numberofinfluencejoints_size(); ++x)
		{
			newSkinWeights->numInfJoints.push_back(deformableMesh.skinweights(i).numberofinfluencejoints(x));
		}
		// 7
		for (int x = 0; x < deformableMesh.skinweights(i).weightindices_size(); ++x)
		{
			newSkinWeights->weightIndex.push_back(deformableMesh.skinweights(i).weightindices(x));
		}
		// 8
		for (int x = 0; x < deformableMesh.skinweights(i).jointnameindices_size(); ++x)
		{
			newSkinWeights->jointNameIndex.push_back(deformableMesh.skinweights(i).jointnameindices(x));
		}

		this->skinWeights.push_back(newSkinWeights);
	}

	
	
	// explicitly load all the textures
	boost::filesystem::path p(inputFileName);
	std::string filePath = p.parent_path().string();
	loadAllFoundTextures(filePath);
	
	return true;
}

void DeformableMesh::loadAllFoundTextures(std::string textureDirectory)
{
	// load texture specifically into SmartBody after parsing the texture file names
	std::map<std::string,std::string>::iterator iter;
	for (size_t i = 0; i < dMeshStatic_p.size(); ++i)
	{
		for (iter = dMeshStatic_p[i]->shape().mtlTextureNameMap.begin(); iter != dMeshStatic_p[i]->shape().mtlTextureNameMap.end(); ++iter)
		{
			SbmTextureManager& texManager = SbmTextureManager::singleton();
			SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, iter->second.c_str());
			if (!tex)
			{
				// separate the texture prefix and |
				std::string textureLabel = iter->second;				
				int labelIndex = textureLabel.find_first_of("|");
				std::string textureFile = textureLabel.substr(labelIndex + 1);
				int prefixIndex = textureFile.find_first_of("file:///");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(7);
				prefixIndex = textureFile.find_first_of("file://");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(6);
				std::string textureName = textureDirectory + "/" + textureFile;
				if (!boost::filesystem::exists(boost::filesystem::path(textureName)))
					LOG("Texture %s doesn't exist under same path of mesh %s", textureName.c_str(), getName().c_str());
				texManager.loadTexture(SbmTextureManager::TEXTURE_DIFFUSE, iter->second.c_str(), textureName.c_str());
			}
		}
		for (iter = dMeshStatic_p[i]->shape().mtlNormalTexNameMap.begin(); iter != dMeshStatic_p[i]->shape().mtlNormalTexNameMap.end(); ++iter)
		{
			SbmTextureManager& texManager = SbmTextureManager::singleton();
			SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_NORMALMAP, iter->second.c_str());
			if (!tex)
			{
				// separate the texture prefix and |
				std::string textureLabel = iter->second;
				int labelIndex = textureLabel.find_first_of("|");
				std::string textureFile = textureLabel.substr(labelIndex + 1);
				int prefixIndex = textureFile.find_first_of("file:///");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(7);
				prefixIndex = textureFile.find_first_of("file://");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(6);
				std::string textureName = textureDirectory + "/" + textureFile;
				if (!boost::filesystem::exists(boost::filesystem::path(textureName)))
					LOG("Texture %s doesn't exist under same path of mesh %s", textureName.c_str(), getName().c_str());
				texManager.loadTexture(SbmTextureManager::TEXTURE_NORMALMAP, iter->second.c_str(), textureName.c_str());
			}
		}
		for (iter = dMeshStatic_p[i]->shape().mtlSpecularTexNameMap.begin(); iter != dMeshStatic_p[i]->shape().mtlSpecularTexNameMap.end(); ++iter)
		{
			SbmTextureManager& texManager = SbmTextureManager::singleton();
			SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_SPECULARMAP, iter->second.c_str());
			if (!tex)
			{
				// separate the texture prefix and |
				std::string textureLabel = iter->second;
				int labelIndex = textureLabel.find_first_of("|");
				std::string textureFile = textureLabel.substr(labelIndex + 1);
				int prefixIndex = textureFile.find_first_of("file:///");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(7);
				prefixIndex = textureFile.find_first_of("file://");
				if (prefixIndex == 0)
					textureFile = textureFile.substr(6);
				std::string textureName = textureDirectory + "/" + textureFile;
				if (!boost::filesystem::exists(boost::filesystem::path(textureName)))
					LOG("Texture %s doesn't exist under same path of mesh %s", textureName.c_str(), getName().c_str());
				texManager.loadTexture(SbmTextureManager::TEXTURE_SPECULARMAP, iter->second.c_str(), textureName.c_str());
			}
		}
	}
}

SrVec DeformableMesh::computeCenterOfMass()
{
	SrVec CoM = SrVec(0,0,0);
	for (unsigned int i=0;i<posBuf.size();i++)
	{
		CoM += posBuf[i];
	}
	CoM /= posBuf.size();
	return CoM;
}

SBAPI SrBox DeformableMesh::computeBoundingBox()
{
	SrBox bbox;
	for (unsigned int i=0;i<posBuf.size();i++)
	{
		bbox.extend(posBuf[i]);
	}
	return bbox;
}

/************************************************************************/
/* Deformable Mesh Instance                                             */
/************************************************************************/

DeformableMeshInstance::DeformableMeshInstance()
{
	_mesh				= NULL;
	_skeleton			= NULL;
	_updateMesh			= false;
	_isStaticMesh		= false;
	_recomputeNormal	= true;
	_meshScale			= SrVec(1.f, 1.f, 1.f);
	_character			= NULL;
	meshVisibleType		= 1;

	//	Auxiliar FBO and Texture for offline rendering to blend textures
	_tempTex			= 0;
	_tempFBO			= 0;

	_tempTexPairs		= NULL;
	_tempFBOPairs		= NULL;

	_tempTexWithMask	= NULL;
	_tempFBOTexWithMask = NULL;
	_pawn = _character = NULL;
}

DeformableMeshInstance::~DeformableMeshInstance()
{
	if(_tempFBO > 0)
	{
		glDeleteBuffers(1, &_tempFBO);
	}

	if(_tempTex > 0)
	{
		LOG("Deleting _tempTex #%d", _tempTex);
		glDeleteTextures(1, &_tempTex);
	}

	if(_tempFBOPairs != NULL)
	{
		glDeleteBuffers(_mesh->blendShapeMap.size(), _tempFBOPairs);
	}
	delete _tempFBOPairs;

	if(_tempTexPairs != NULL)
	{
		glDeleteTextures(_mesh->blendShapeMap.size(), _tempTexPairs);
		std::cerr << "Deleting _tempTexPairs\n";
	}
	delete _tempTexPairs;
}


void DeformableMeshInstance::setPawn(SmartBody::SBPawn* pawn)
{
	//if (_skeleton)
	//	_skeleton->unref();
	if (pawn)
		_skeleton = pawn->getSkeleton();
	if (_skeleton)
		_skeleton->ref();
	_pawn = pawn;
	_character = dynamic_cast<SmartBody::SBCharacter*>(pawn);
	updateJointList();
}



void DeformableMeshInstance::GPUblendShapes(glm::mat4x4 translation, glm::mat4x4 rotation)
{
#if WIN_BUILD
	DeformableMesh * _mesh		= this->getDeformableMesh();
	
	bool showMasks = false;

	SrSnModel* writeToBaseModel = NULL;
	SrSnModel* baseModel		= NULL;

	int tex_h = 1024;
	int tex_w = 1024;

	// find the base shape from static meshes
	std::map<std::string, std::vector<SrSnModel*> >::iterator mIter;

	//	Initializes vector of wieghts, of size (#shapes) 
	if (_mesh->blendShapeMap.begin() == _mesh->blendShapeMap.end())
		return;
	std::vector<float> weights(_mesh->blendShapeMap.begin()->second.size(), 0);

	//	Initializes vector of wieghts, of size (#shapes) each shape got a texture
	std::vector<GLuint> texIDs(_mesh->blendShapeMap.begin()->second.size(), 0);

	std::vector<std::string> texture_names(_mesh->blendShapeMap.begin()->second.size());
	std::vector<std::string> textureFileNames(_mesh->blendShapeMap.begin()->second.size());

	for (mIter = _mesh->blendShapeMap.begin(); mIter != _mesh->blendShapeMap.end(); ++mIter)
	{
		bool foundBaseModel = false;

		for (size_t i = 0; i < _mesh->dMeshStatic_p.size(); ++i)
		{
			if (strcmp(_mesh->dMeshStatic_p[i]->shape().name, mIter->first.c_str()) == 0)
			{
				//	If base shape, copies pointer to _mesh->dMeshStatic (here is where the result resulting vertices position are stored)
				writeToBaseModel = _mesh->dMeshStatic_p[i];
				break;
			}
		}

		if (writeToBaseModel == NULL)
		{
			//LOG("base model to write to cannot be found");
			continue;
		}
		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (strcmp(mIter->first.c_str(), (const char*)mIter->second[i]->shape().name) == 0)
			{
				baseModel		= mIter->second[i];
				foundBaseModel	= true;
				break;
			}
		}
		if (baseModel == NULL)
		{
			LOG("original base model cannot be found");
			continue;
		}

		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (!mIter->second[i])
				continue;

			float w = 0.0f;
			float wLimit = 1.0f;
			// get weight
			std::stringstream ss;
			ss << "blendShape.channelName." << (const char*)mIter->second[i]->shape().name;
			std::stringstream ss1;
			ss1 << "blendShape.channelWeightLimit." << (const char*)mIter->second[i]->shape().name;
					
			if (_character->hasAttribute(ss1.str()))
			{
				wLimit = (float)_character->getDoubleAttribute(ss1.str());
			}

			if (_character->hasAttribute(ss.str()))
			{
				const std::string& mappedCName	= _character->getStringAttribute(ss.str());
				SmartBody::SBSkeleton* sbSkel	= _character->getSkeleton();
				if (sbSkel && mappedCName != "")
				{
					SmartBody::SBJoint* joint = sbSkel->getJointByName(mappedCName);
					if (joint)
					{
						SrVec pos = joint->getPosition();
						w = pos.x;
						//LOG("shape %s(%s) with weight %f", (const char*)mIter->second[i]->shape().name, mappedCName.c_str(), w);
						// clamp
						//if (w > wLimit)
						//	w = wLimit;
						
						// multiplier
						w = w * wLimit;
					}
				}
			}
			else
				continue;

			// Stores weights of each face
			weights[i]		= w;
		}

		// Starts computing blended textures
		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (!mIter->second[i])
				continue;

			//	Gets the map of (material name, texture) for the current mesh
			std::vector<std::string> materials;
			std::map<std::string, std::string> textures_map = mIter->second[i]->shape().mtlTextureNameMap;
			for(std::map<std::string,std::string>::iterator it = textures_map.begin(); it != textures_map.end(); ++it) {
				materials.push_back(it->first);
			}

			//	In a face there will be just one texture, material name will be always the first
			std::string matName = "";
			if (materials.size() > 0)
				matName = materials[0];

			// If base model
			if (strcmp(mIter->first.c_str(), (const char*)mIter->second[i]->shape().name) == 0)
			{
				std::string fileName = (std::string)mIter->second[i]->shape().mtlTextureNameMap[matName];
				SbmTexture* tex		= SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, fileName.c_str());
				if (tex)
				{
					texIDs[i]		= tex->getID();
					texture_names[i]= fileName;
					textureFileNames[i] = tex->getFileName();
					//std::cerr << "Retriving texture " << matName << "\ttexIDs[" << i << "]: " << texIDs[i] << "\n";

					tex_h			= tex->getHeight();
					tex_w			= tex->getWidth();
				} 
				else
				{
					texIDs[i] = 0;
				}
				continue;	// don't do anything about base model
			}

			// Rest of the models
			std::string fileName = (std::string)mIter->second[i]->shape().mtlTextureNameMap[matName];
			SbmTexture* tex		= SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, fileName.c_str());
			if (tex)
			{
				texIDs[i]		= tex->getID();
				
				texture_names[i]= fileName;
				textureFileNames[i] = tex->getFileName();
				//std::cout << "Retriving texture " << matName << "\ttexIDs[" << i << "]: " << texIDs[i] << "\n";
			}
			else
			{
				texIDs[i] =  0;
			}
		}
	}

	if(tex_w > 2048)
		tex_w = 2048;

	if(tex_h > 2048)
		tex_h = 2048;

	if(_tempFBO == 0) 
	{
		glGenFramebuffersEXT(1, &_tempFBO);
	}


	// Aux textures used when calling BlendAllAppearancesPairwise to store temporary results for texture blending pair wise
	if(_tempTexPairs == NULL) 
	{
		_tempTexPairs = new GLuint[weights.size()];
		glGenTextures(weights.size(), _tempTexPairs);
		for(unsigned int i=0; i<weights.size(); i++) {
			glBindTexture(GL_TEXTURE_2D, _tempTexPairs[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(__ANDROID__) || defined(SB_IPHONE) || defined(EMSCRIPTEN)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_FLOAT, NULL);
#endif
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if(_tempFBOTexWithMask == NULL) 
	{
		_tempFBOTexWithMask = new GLuint[weights.size()];
		glGenFramebuffers(weights.size(), _tempFBOTexWithMask);
	}

	// If images with masks in the alpha channel have not been created
	if(_tempTexWithMask == NULL)
	{
		_tempTexWithMask = new GLuint[weights.size()];
		glGenTextures(weights.size(), _tempTexWithMask);
		for(unsigned int i=0; i<weights.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, _tempTexWithMask[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if !defined(ANDROID_BUILD) && !defined(EMSCRIPTEN)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
#endif
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Adds masking images to the alpha channel of the textures
		SbmBlendTextures::ReadMasks(_tempFBOTexWithMask, _tempTexWithMask, weights, texIDs, texture_names, textureFileNames, SbmBlendTextures::getShader("ReadMasks"), tex_w, tex_h); 
	}



	if (texIDs.size() > 0 && texIDs[0] != 0)
	{

		//SbmShaderProgram::printOglError("texIDs.size() > 0 ");

		// New attempt to blend textures with masks (also renders a face). It uses the _tempTexWithMask, which are the texture maps with the masking encoded in its ALPHA channel.
		// The _tempTexWithMask texture were created above in the SbmBlendTextures::ReadMasks call
		//SbmBlendTextures::BlendGeometryWithMasks( _tempFBOTexWithMask, weights, _tempTexWithMask, texture_names, this,  SbmBlendTextures::getShader("BlendGeometryWithMasks"), translation, rotation);
		
		SbmBlendTextures::BlendGeometryWithMasksFeedback( _tempFBOTexWithMask, weights, _tempTexWithMask, texture_names, this,  SbmBlendTextures::getShader("BlendGeometryWithMasksFeedback"), translation, rotation);
		//SbmBlendTextures::BlendTextureWithMasks(_tempFBO, _tempTexPairs[0], weights, _tempTexWithMask, texture_names, this,  SbmBlendTextures::getShader("BlendAllTexturesWithMask"),tex_w, tex_h);
		SbmBlendTextures::RenderGeometryWithMasks( _tempFBOTexWithMask, weights, _tempTexWithMask, texture_names, this,  SbmBlendTextures::getShader("RenderGeometryWithMasks"), translation, rotation);

		// Blends geometry and texture in the same GLSL (also renders a face) (this does NOT use masking)
		//SbmBlendTextures::BlendGeometry( _tempFBOPairs, weights, texIDs, texture_names, this,  SbmBlendTextures::getShader("BlendGeometry"));

		// Computes blended texture pairwise, and saves it into _tempTexPairs[0], which is going to be used later as a texture (in the normal blendshape pipeline)
		//SbmBlendTextures::BlendAllAppearancesPairwise( _tempFBOPairs, _tempTexPairs, weights, texIDs, texture_names, SbmBlendTextures::getShader("Blend_All_Textures_Pairwise"), tex_w, tex_h);
	}
#endif
}

void DeformableMeshInstance::blendShapes()
{
	
	//SbmShaderProgram::printOglError("DeformableMeshInstance::blendShapes() #0 ");

	if (!_character)
	{
		return;
	}


	SrSnModel* writeToBaseModel = NULL;
	SrSnModel* baseModel = NULL;

	// find the base shape from static meshes
	for (std::map<std::string, std::vector<SrSnModel*> >::iterator mIter = _mesh->blendShapeMap.begin(); 
		 mIter != _mesh->blendShapeMap.end(); 
		 mIter++)
	{
		bool foundBaseModel = false;
		std::string baseModelName = (*mIter).first;
		std::vector<SrSnModel*>& targets = (*mIter).second;

		for (size_t i = 0; i < _mesh->dMeshStatic_p.size(); ++i)
		{
			if (strcmp(_mesh->dMeshStatic_p[i]->shape().name, baseModelName.c_str()) == 0)
			{
				//	If base shape, copies pointer to _mesh->dMeshStatic (here is where the result resulting vertices position are stored)
				writeToBaseModel = _mesh->dMeshStatic_p[i];
				break;
			}
		}

		if (writeToBaseModel == NULL)
		{
			//LOG("base model to write to cannot be found");
			continue;
		}
		for (size_t i = 0; i < targets.size(); ++i)
		{
			if (strcmp(mIter->first.c_str(), (const char*) targets[i]->shape().name) == 0)
			{
				baseModel		= targets[i];
				foundBaseModel	= true;
				break;
			}
		}
		if (baseModel == NULL)
		{
			LOG("original base model cannot be found");
			continue;
		}

		//SrArray<SrPnt>& neutralV = baseModel->shape().V;
		std::vector<SrVec>& neutralV = baseModel->shape().V;
		std::vector<SrPnt>& neutralN = baseModel->shape().N;
		std::vector<SrVec> newV = neutralV;
		std::vector<SrPnt> newN = neutralN;

		if (foundBaseModel && 
			_character->getBoolAttribute("useOptimizedBlendShapes"))
		{
			std::map<std::string, std::vector<BlendShapeData> >::iterator optIter = _mesh->optimizedBlendShapeData.find(baseModelName);
			if (optIter == _mesh->optimizedBlendShapeData.end())
			{
				_mesh->optimizedBlendShapeData.insert(std::pair<std::string, std::vector<BlendShapeData> >(baseModelName, std::vector<BlendShapeData>()));
				optIter = _mesh->optimizedBlendShapeData.find(baseModelName);
			}

			std::vector<BlendShapeData>& optimizedShapeData = (*optIter).second;
			if (optimizedShapeData.size() != targets.size())
			{
				LOG("Optimizing blend shapes. Only have %d/%d shapes.", optimizedShapeData.size(), targets.size());
				optimizedShapeData.clear();
				// optimize the blend shape maps as needed
				for (size_t i = 0; i < targets.size(); ++i)
				{
					optimizedShapeData.push_back(BlendShapeData());
					if (i == 0)
					{
						continue;
					}
					if (!targets[i])
					{
						continue;
					}
					BlendShapeData& blendData = optimizedShapeData[i];
					//SrArray<SrPnt>& visemeV = mIter->second[i]->shape().V;
					std::vector<SrVec>& visemeV = targets[i]->shape().V;
					std::vector<SrPnt>& visemeN = targets[i]->shape().N;


					SrVec vVec;
					SrVec nVec;
					for (unsigned int v = 0; v < visemeV.size(); ++v)
					{
						vVec = visemeV[v] - neutralV[v];
						if (fabs(vVec[0]) >  gwiz::epsilon4() ||
							fabs(vVec[1]) >  gwiz::epsilon4() ||
							fabs(vVec[2]) >  gwiz::epsilon4())
						{
							std::pair<int, SrVec> temp;
							temp.first = v;
							temp.second = vVec;
							blendData.diffV.push_back(temp);
						}
					}
					for (unsigned int n = 0; n < visemeN.size(); ++n)
					{
						nVec = visemeN[n] - neutralN[n];
						if (fabs(nVec[0]) >  gwiz::epsilon4() ||
							fabs(nVec[1]) >  gwiz::epsilon4() ||
							fabs(nVec[2]) >  gwiz::epsilon4())	
						{
							blendData.diffN.push_back(std::pair<int, SrVec>(n, nVec));
						}
					}
					LOG("Optimized blend %s has %d/%d vertices, %d/%d normals.", (const char*) targets[i]->shape().name, blendData.diffV.size(), visemeV.size(), blendData.diffN.size(), visemeN.size());
				}
			}
		}


		//	Initializes vector of wieghts, of size (#shapes) 
		std::vector<float> weights(targets.size(), 0);


		//	Initializes vector of wieghts, of size (#shapes) each shape got a texture
		std::vector<GLuint> texIDs(targets.size(), 0);

		std::vector<std::string> texture_names(targets.size());

		int tex_h = 1024;
		int tex_w = 1024;

		for (size_t i = 0; i < targets.size(); ++i)
		{
			if (!targets[i])
				continue;

			float w = 0.0f;
			float wLimit = 1.0f;
			// get weight
			std::stringstream ss;
			ss << "blendShape.channelName." << (const char*) targets[i]->shape().name;
			std::stringstream ss1;
			ss1 << "blendShape.channelWeightLimit." << (const char*) targets[i]->shape().name;
					
			if (_character->hasAttribute(ss1.str()))
			{
				wLimit = (float)_character->getDoubleAttribute(ss1.str());
			}

			if (_character->hasAttribute(ss.str()))
			{
				const std::string& mappedCName	= _character->getStringAttribute(ss.str());
				SmartBody::SBSkeleton* sbSkel	= _character->getSkeleton();
				if (sbSkel && mappedCName != "")
				{
					SmartBody::SBJoint* joint = sbSkel->getJointByName(mappedCName);
					if (joint)
					{
						SrVec pos = joint->getPosition();
						w = pos.x;
						//LOG("shape %s(%s) with weight %f", (const char*)mIter->second[i]->shape().name, mappedCName.c_str(), w);
						// clamp
						//if (w > wLimit)
						//	w = wLimit;
						
						// multiplier
						w = w * wLimit;
					}
				}
			}
			else
				continue;

			// Stores weights of each face
			weights[i]		= w;


			//std::cerr << "weights[" << i << "]: " << w << "\n";

			if (fabs(w) > gwiz::epsilon4())	// if it has weight
			{
				//LOG("blend in %s with weight %f", (const char*)mIter->second[i]->shape().name, w);
				//SrArray<SrPnt>& visemeV = mIter->second[i]->shape().V;
				std::vector<SrVec>& visemeV = targets[i]->shape().V;
				std::vector<SrPnt>& visemeN = targets[i]->shape().N;
				if (visemeV.size() != neutralV.size())
				{
					LOG("number of vertices for %s (%d) is not same as neutral (%d)", baseModelName.c_str(), visemeV.size(), neutralV.size());
					continue;
				}
				if (visemeN.size() != neutralN.size())
				{
					LOG("number of normals for %s (%d) is not same as neutral (%d)", baseModelName.c_str(), visemeN.size(), neutralN.size());
					continue;
				}

				if (_character->getBoolAttribute("useOptimizedBlendShapes"))
				{
					// loop through a shorter list of different vertices and normals
					std::map<std::string, std::vector<BlendShapeData> >::iterator optIter = _mesh->optimizedBlendShapeData.find(baseModelName);
					std::vector<BlendShapeData>& optimizedShapeData = (*optIter).second;
					BlendShapeData& blendData = optimizedShapeData[i];
					int vSize = optimizedShapeData[i].diffV.size();
					for (int v = 0; v < vSize; ++v)
					{
						int index	= blendData.diffV[v].first;
						SrVec& diff = blendData.diffV[v].second;
						newV[index] = newV[index] + diff * w;
					}
					int nSize = optimizedShapeData[i].diffN.size();
					for (int n = 0; n < nSize; ++n)
					{
						int index	= blendData.diffN[n].first;
						SrVec& diff = blendData.diffN[n].second;
						newN[index] = newN[index] + diff * w;
					}
				}
				else
				{
					// loop through all vertices and normals
					for (unsigned int v = 0; v < visemeV.size(); ++v)
					{
						SrPnt diff = visemeV[v] - neutralV[v];
						if (fabs(diff[0]) >  gwiz::epsilon4() ||
							fabs(diff[1]) >  gwiz::epsilon4() ||
							fabs(diff[2]) >  gwiz::epsilon4())	
							newV[v] = newV[v] + diff * w;
					}
					for (unsigned int n = 0; n < visemeN.size(); ++n)
					{
						SrPnt diff = visemeN[n] - neutralN[n];
						if (fabs(diff[0]) >  gwiz::epsilon4() ||
							fabs(diff[1]) >  gwiz::epsilon4() ||
							fabs(diff[2]) >  gwiz::epsilon4())	
							newN[n] = newN[n] + diff * w;
					}
				}
			}
		}
		for (unsigned int n = 0; n < newN.size(); ++n)
		{
			newN[n].normalize();
		}


		// END OF SECOND ATTEMPT
		writeToBaseModel->shape().V = newV;
		writeToBaseModel->shape().N = newN;
		writeToBaseModel->changed(true);

		if (_character->getStringAttribute("texturesType") != "dynamic")
			continue;


		

		// Starts computing blended textures
		for (size_t i = 0; i < targets.size(); ++i)
		{
			if (!mIter->second[i])
				continue;

			//	Gets the map of (material name, texture) for the current mesh
			std::vector<std::string> materials = targets[i]->shape().mtlnames;
#if 0
			std::map<std::string, std::string> textures_map = mIter->second[i]->shape().mtlTextureNameMap;
			for(std::map<std::string,std::string>::iterator it = textures_map.begin(); it != textures_map.end(); ++it) {
				materials.push_back(it->first);
			}
#endif
			//	In a face there will be just one texture, material name will be always the first
			std::string matName = "";
			if (materials.size() > 0)
				matName = materials[0];

			// If base model
			if (strcmp(mIter->first.c_str(), (const char*) targets[i]->shape().name) == 0)
			{
				std::string fileName = (std::string) targets[i]->shape().mtlTextureNameMap[matName];
				SbmTexture* tex		= SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, fileName.c_str());
				if (tex)
				{
					texIDs[i]		= tex->getID();
					texture_names[i]= fileName;
					//std::cerr << "Retriving texture " << matName << "\ttexIDs[" << i << "]: " << texIDs[i] << "\n";

					tex_h			= tex->getHeight();
					tex_w			= tex->getWidth();
				} 
				else
				{
					texIDs[i] = 0;
				}
				continue;	// don't do anything about base model
			}

			// Rest of the models
			std::string fileName = (std::string) targets[i]->shape().mtlTextureNameMap[matName];
			SbmTexture* tex		= SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE, fileName.c_str());
			if (tex)
			{
				texIDs[i]		= tex->getID();
				
				texture_names[i]= fileName;
				//std::cout << "Retriving texture " << matName << "\ttexIDs[" << i << "]: " << texIDs[i] << "\n";
			}
			else
			{
				texIDs[i] =  0;
			}
		}

	

		//	Here I try to blend the faces two at a time. This way I avoid hardcoded constant vector size.
#if !defined(SB_IPHONE) 

		SbmTextureManager& texManager = SbmTextureManager::singleton();
		if(_tempFBOPairs == NULL) 
		{
			_tempFBOPairs = new GLuint[weights.size()];
			//glGenFramebuffers(weights.size(), _tempFBOPairs);
			std::string FBOName = "TempFBO";
			for (unsigned int i=0;i<weights.size();i++)
			{
				std::string tempName = FBOName + "_weight" + boost::lexical_cast<std::string>(i);
				texManager.createFBO(tempName.c_str());
		}
		}
		std::string FBOName = "TempFBO";
		for (unsigned int i=0;i<weights.size();i++)
		{
			std::string tempName = FBOName + "_weight" + boost::lexical_cast<std::string>(i);
			_tempFBOPairs[i] = texManager.findFBO(tempName.c_str());
		}

//		SbmShaderProgram::printOglError("HERE #4 ");

		if(tex_w > 2048)
			tex_w = 2048;

		if(tex_h > 2048)
			tex_h = 2048;

		
		// Aux textures used when calling BlendAllAppearancesPairwise to store temporary results for texture blending pair wise
		if(_tempTexPairs == NULL) 
		{
#define USE_TEXTURE_MANAGER 1
#if USE_TEXTURE_MANAGER
			_tempTexPairs = new GLuint[weights.size()];
			std::string meshName = this->getDeformableMesh()->getName();
			
			for (unsigned int i=0;i<weights.size();i++)
			{
				std::string tempTexName = meshName + "_weight" + boost::lexical_cast<std::string>(i);
				texManager.createWhiteTexture(tempTexName.c_str(), tex_w, tex_h);
			}
#else
			glGenTextures(weights.size(), _tempTexPairs);
			for(int i=0; i<weights.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, _tempTexPairs[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(__ANDROID__) || defined(SB_IPHONE) || defined(EMSCRIPTEN)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_FLOAT, NULL);
#endif
				glBindTexture(GL_TEXTURE_2D, 0);
			}
#endif
		}
		for (unsigned int i=0;i<weights.size();i++)
		{
			std::string meshName = this->getDeformableMesh()->getName();
			std::string tempTexName = meshName + "_weight" + boost::lexical_cast<std::string>(i);
			SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE, tempTexName.c_str());
			if (tex)
			{
				_tempTexPairs[i] = tex->getID();
			}
			else
			{
				LOG("Error : BlendShape Texture, '%s' does not exist.", tempTexName.c_str());
			}
		}		

		if (texIDs.size() > 0 && texIDs[0] != 0)
		{		
#if !defined(EMSCRIPTEN)
			// Computes blended texture pairwise, and saves it into _tempTexPairs[0], which is going to be used later as a texture (in the normal blendshape pipeline)
			SbmBlendTextures::BlendAllAppearancesPairwise( _tempFBOPairs, _tempTexPairs, weights, texIDs, texture_names, SbmBlendTextures::getShader("Blend_All_Textures_Pairwise"), tex_w, tex_h);
#endif
		}
#endif

	}

	return;
}


void DeformableMeshInstance::setDeformableMesh( DeformableMesh* mesh )
{
	//LOG("setDeformableMesh to be %s", mesh->meshName.c_str());
	_mesh = mesh;
	_mesh->buildSkinnedVertexBuffer(); // make sure the deformable mesh has vertex buffer
	_deformPosBuf.resize(_mesh->posBuf.size()); // initialized deformation posBuffer
	_restPosBuf.resize(_mesh->posBuf.size());
	_deformNormalBuf.resize(_mesh->normalBuf.size());
	_deformTangentBuf.resize(_mesh->tangentBuf.size());
	for (unsigned int i=0;i<_deformPosBuf.size();i++)
	{
		_restPosBuf[i] = _mesh->posBuf[i];
		_deformPosBuf[i] = _mesh->posBuf[i];
	}
	updateJointList();
}

void DeformableMeshInstance::updateJointList()
{
	if (!_skeleton || !_mesh) return;
	std::vector<SkinWeight*>& skinWeights = _mesh->skinWeights;
	_boneJointList.clear();
	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];
		SkJointList jlist;
		for (unsigned int k=0;k<skinWeight->infJointName.size();k++)
		{
			std::string& jname = skinWeight->infJointName[k];
			SkJoint* joint = _skeleton->search_joint(jname.c_str());
			jlist.push_back(joint);				
		}
		_boneJointList.push_back(jlist);
	}
}

void DeformableMeshInstance::setVisibility(int deformableMesh)
{
	if (deformableMesh != -1)
	{		
		_updateMesh = deformableMesh? true:false;
		meshVisibleType = deformableMesh;
	}
}

void DeformableMeshInstance::updateTransformBuffer()
{
	if (!_mesh) return;
	unsigned int boneSize = 120;
	size_t jointMapSize = _mesh->boneJointIdxMap.size();
	int transformSize = (boneSize > jointMapSize) ? boneSize : jointMapSize;
	if (transformBuffer.size() != transformSize)
		transformBuffer.resize(transformSize);
	std::map<std::string,int>& boneIdxMap = _mesh->boneJointIdxMap;
	std::map<std::string,int>::iterator mi;	
	for ( mi  = boneIdxMap.begin();
		mi != boneIdxMap.end();
		mi++)	
	{
		int idx = mi->second;
		SkJoint* joint = _skeleton->search_joint(mi->first.c_str());//boneJointList[i];		
		if (!joint)
			continue;
		SrMat bindPoseMat = _mesh->bindPoseMatList[idx];
		bindPoseMat.set_translation(bindPoseMat.get_translation()*_meshScale[0]);
		transformBuffer[idx] = bindPoseMat*joint->gmat();			
	}
}


void DeformableMeshInstance::updateSkin( const std::vector<SrVec>& restPos, std::vector<SrVec>& deformPos )
{
	_skeleton->update_global_matrices();
	updateTransformBuffer();

	for (unsigned int i=0;i<restPos.size();i++)
	{
		SrVec vPos = restPos[i]*_meshScale[0];
		SrVec vSkinPos = SrVec(0,0,0);	
		for (int k=0;k<_mesh->boneCountBuf[i];k++)		
		{	

#if USE_SKIN_WEIGHT_SIZE_8
			int a = (k<4) ? 0 : 1;
			int b = k%4;		
#else
			if (k >= 4)
				break;
			int a = 0;
			int b = k;			
#endif
			vSkinPos += (vPos*transformBuffer[_mesh->boneIDBuf[a][i][b]])*_mesh->boneWeightBuf[a][i][b];
		}
		deformPos[i] = vSkinPos;
	}
}


SBAPI void DeformableMeshInstance::updateFast()
{
	if (!_updateMesh)	return;
	if (!_skeleton || !_mesh) return;	
	if (isStaticMesh()) return; // not update the buffer if it's a static mesh
	//LOG("Update Skin...");
	//updateSkin(_mesh->posBuf,  _deformPosBuf);
	updateSkin(_restPosBuf, _deformPosBuf);

	SrVec tempPos = _deformPosBuf[150];
	//LOG("deformPos = %f %f %f",tempPos[0],tempPos[1],tempPos[2]);
}


void DeformableMeshInstance::update()
{
	//blendShapes();
	//LOG("Update deformable mesh");
#define RECOMPUTE_NORMAL 0
	if (!_updateMesh)
	{
		//LOG("!_updateMesh...");
		return;
	}
	if (!_skeleton || !_mesh)
	{
		//LOG("No skeleton or no mesh...");
		return;
	}
	if (isStaticMesh())
	{
		//LOG("is static mesh...");
		return; // not update the buffer if it's a static mesh
	}
	_skeleton->update_global_matrices();
#if defined(EMSCRIPTEN)
	updateTransformBuffer();
	return;
#endif
	//LOG("Update DeformableMeshInstance");
	updateFast();
	return;

	int maxJoint = -1;
	std::vector<SkinWeight*>& skinWeights = _mesh->skinWeights;
	if (skinWeights.size() != _boneJointList.size()) updateJointList();
	std::map<int,std::vector<int> >& vtxNewVtxIdxMap = _mesh->vtxNewVtxIdxMap;
	int iVtx = 0;

#ifdef RECOMPUTE_NORMAL
	int iNormalVtx = 0;
#endif

	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];
		SkJointList& jointList = _boneJointList[skinCounter];
		std::map<std::string, std::vector<std::string> >::iterator iter = _mesh->morphTargets.find(skinWeight->sourceMesh);
		int pos;
		int globalCounter = 0;
		if (iter != _mesh->morphTargets.end() && iter->second.size() > 0)	pos = _mesh->getMesh(iter->second[0]);
		else																pos = _mesh->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			//LOG("update skinning, skin counter = %d", skinCounter);
			SrSnModel* dMeshStatic = _mesh->dMeshStatic_p[pos];
			//SrSnModel* dMeshDynamic = dynamicMesh[pos];
			int numVertices = dMeshStatic->shape().V.size();
			for (int i = 0; i < numVertices; i++)
			{
				if (i >= (int) skinWeight->numInfJoints.size())
					continue;
				int numOfInfJoints = skinWeight->numInfJoints[i];
				if (numOfInfJoints > maxJoint)
					maxJoint = numOfInfJoints;
				SrVec& skinLocalVec = dMeshStatic->shape().V[i];
				SrVec finalVec;		

				if (numOfInfJoints >= 5 )
				{
					int a = 0; // dummy for break point
				}


				for (int j = 0; j < numOfInfJoints; j++)
				{
					//std::string jointName = skinWeight->infJointName[skinWeight->jointNameIndex[globalCounter]];	
					int jointIndex = skinWeight->jointNameIndex[globalCounter];
					SkJoint* curJoint = jointList[jointIndex];//skinWeight->infJoint[skinWeight->jointNameIndex[globalCounter]];
					if (curJoint == NULL) continue;
						
					const SrMat& gMat = curJoint->gmat();
					SrMat& invBMat = skinWeight->bindPoseMat[skinWeight->jointNameIndex[globalCounter]];	
					double jointWeight = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
					globalCounter ++;
					SrVec transformVec = _meshScale[0]*(skinLocalVec * skinWeight->bindShapeMat * invBMat);
					SrVec finalTransformVec = (transformVec  * gMat);
					finalVec = finalVec + (float(jointWeight) * finalTransformVec);							
				}
				
				_deformPosBuf[iVtx] = finalVec;
				if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
				{
					std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
					// copy related vtx components 
					for (unsigned int k=0;k<idxMap.size();k++)
					{
						_deformPosBuf[idxMap[k]] = finalVec;
					}
				}					
				iVtx++;
#if RECOMPUTE_NORMAL
				if (_recomputeNormal)
					dMeshDynamic->shape().V[i] = finalVec;
#endif
			}
#if RECOMPUTE_NORMAL
			if (_recomputeNormal)
			{
				dMeshDynamic->shape().computeNormals();
				for (int i = 0; i < numVertices; i++)
				{
					SrVec finalN = dMeshDynamic->shape().N[i];
					_mesh->normalBuf[iNormalVtx] = finalN;
					if (vtxNewVtxIdxMap.find(iNormalVtx) != vtxNewVtxIdxMap.end())
					{
						std::vector<int>& idxMap = vtxNewVtxIdxMap[iNormalVtx];
						// copy related vtx components 
						for (unsigned int k=0;k<idxMap.size();k++)
						{
							_mesh->normalBuf[idxMap[k]] = finalN;
						}
					}
					iNormalVtx++;
				}
					
			}
#endif
			//dMeshDynamic->changed(true);	
		}
		else
			continue;
	}
	_recomputeNormal = false;
}

int DeformableMeshInstance::getVisibility()
{
	return meshVisibleType;
}

void DeformableMeshInstance::setMeshScale( SrVec scale )
{
	_meshScale = scale;
}

SmartBody::SBSkeleton* DeformableMeshInstance::getSkeleton()
{
	SmartBody::SBSkeleton* skel = dynamic_cast<SmartBody::SBSkeleton*>(_skeleton);
	return skel;
}

void DeformableMeshInstance::setToStaticMesh( bool isStatic )
{
	_isStaticMesh = isStatic;
}

SBAPI bool DeformableMeshInstance::isStaticMesh()
{
	if (!_mesh) return true;
	return (_isStaticMesh || !_mesh->isSkinnedMesh());
}

SBAPI void DeformableMeshInstance::blendShapeStaticMesh()
{
	//return;
	//LOG("Running blendShapeStaticMesh");

	//SbmShaderProgram::printOglError("DeformableMeshInstance::blendShapeStaticMesh() #0 ");

	if (!_mesh) 
		return;
	//LOG("Mesh blendshape size = %d", _mesh->blendShapeMap.size());
	if (_mesh->blendShapeMap.size() == 0)
		return;

	DeformableMeshInstance::blendShapes();
	bool mergeBoundary = false;

	if (_character)
		mergeBoundary = _character->getBoolAttribute("blendShape.mergeBoundary");

	for (std::map<std::string, std::vector<SrSnModel*> >::iterator mIter = _mesh->blendShapeMap.begin();
		mIter != _mesh->blendShapeMap.end();
		mIter++)
	{
		SrSnModel* writeToBaseModel = NULL;
		int vtxBaseIdx = 0;

		for (size_t i = 0; i < _mesh->dMeshStatic_p.size(); ++i)
		{
			if (strcmp(_mesh->dMeshStatic_p[i]->shape().name, mIter->first.c_str()) == 0)
			{
				writeToBaseModel = _mesh->dMeshStatic_p[i];
				break;
			}
			else
			{
				// skip vertices for this sub mesh
				vtxBaseIdx += _mesh->dMeshStatic_p[i]->shape().V.size();
			}
		}

		if (!writeToBaseModel)
			continue;

		std::map<int, std::vector<int> >& vtxNewVtxIdxMap = _mesh->vtxNewVtxIdxMap;
		std::map<int, std::vector<int> >& vtxBlendShapeVtxIdxMap = _mesh->blendShapeNewVtxIdxMap;
		SrModel& baseModel = writeToBaseModel->shape();

		std::vector<SrVec>& newPosBuf = (_mesh->isSkinnedMesh()) ? _restPosBuf : _deformPosBuf;
		for (unsigned int i = 0; i < baseModel.V.size(); i++)
		{
			int iVtx = vtxBaseIdx + i;
			SrVec& basePos = baseModel.V[i];
			newPosBuf[iVtx] = basePos;

			if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
			{
				std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
				// copy related vtx components 
				for (unsigned int k = 0; k < idxMap.size(); k++)
				{
					int idx = idxMap[k];
					newPosBuf[idx] = basePos;	// Here copies blended vertices position
				}
			}			
			if (mergeBoundary && vtxBlendShapeVtxIdxMap.find(i) != vtxBlendShapeVtxIdxMap.end())
			{
			 	std::vector<int>& idxMap = vtxBlendShapeVtxIdxMap[i];
			 	// copy related vtx components 
			 	for (unsigned int k=0;k<idxMap.size();k++)
			 	{
			 		int idx				= idxMap[k];
			 		newPosBuf[idx]	= basePos;	// Here copies blended vertices position
			 	}
			}
		}
	}
	
	//SbmShaderProgram::printOglError("DeformableMeshInstance::blendShapeStaticMesh() #FINAL");
}

int DeformableMesh::getNumMeshes()
{
	return dMeshStatic_p.size();
}

const std::string DeformableMesh::getMeshName(int index)
{
	if ((int) dMeshStatic_p.size() > index &&
		index >= 0)
		return (const char*) dMeshStatic_p[index]->shape().name;
	else
		return "";
}

SrModel& DeformableMesh::getStaticModel(int index)
{
	if ((int) dMeshStatic_p.size() > index &&
		index >= 0)
		return dMeshStatic_p[index]->shape();
	else
		return _emptyModel;
}

void DeformableMesh::copySkinWeights(DeformableMesh* fromMesh, const std::string& morphName)
{
	LOG("Start copy skin weights");
	// clear any existing skin weights
	for (size_t w = 0; w < this->skinWeights.size(); w++)
	{
		SkinWeight* weight = this->skinWeights[w];
		delete weight;
	}
	this->skinWeights.clear();


	for (size_t w = 0; w < fromMesh->skinWeights.size(); w++)
	{
		LOG("copy weight %d...", w);
		SkinWeight* weight = new SkinWeight();
		weight->copyWeights(fromMesh->skinWeights[w], morphName);
		this->skinWeights.push_back(weight);
	}
	LOG("Finish copy skin weights");
}
