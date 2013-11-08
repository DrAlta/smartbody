
#include "vhcl.h"

#include "sbm_deformable_mesh.h"

#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sr/sr_sn_group.h>
#include <sr/sr_random.h>
#include <sbm/gwiz_math.h>
#include <sbm/GPU/SbmTexture.h>
#include <boost/algorithm/string.hpp>

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

DeformableMesh::DeformableMesh() : SBAsset()
{
	meshName = "null";
	binding = false;
	initSkinnedVertexBuffer = false;
	initStaticVertexBuffer = false;

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


void DeformableMesh::setSkeleton(SkSkeleton* skel)
{
	//if (skeleton)
	//	skeleton->unref();
	skeleton = skel;
	skel->ref();
}

void DeformableMesh::update()
{
	if (!binding)	return;
	skeleton->update_global_matrices();
	int maxJoint = -1;
	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];
		std::map<std::string, std::vector<std::string> >::iterator iter = this->morphTargets.find(skinWeight->sourceMesh);
		int pos;
		int globalCounter = 0;
		if (iter != this->morphTargets.end() && iter->second.size() > 0)	pos = this->getMesh(iter->second[0]);
		else																pos = this->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			SrSnModel* dMeshStatic = dMeshStatic_p[pos];
			SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
			int numVertices = dMeshStatic->shape().V.size();
			for (int i = 0; i < numVertices; i++)
			{
				if (i >= (int) skinWeight->numInfJoints.size())
					continue;
				int numOfInfJoints = skinWeight->numInfJoints[i];
				//if (numOfInfJoints > maxJoint)
				//	maxJoint = numOfInfJoints;
				SrVec& skinLocalVec = dMeshStatic->shape().V[i];					
				SrVec finalVec;
				//printf("Vtx bind pose = \n");
				for (int j = 0; j < numOfInfJoints; j++)
				{
					const SkJoint* curJoint = skinWeight->infJoint[skinWeight->jointNameIndex[globalCounter]];
					if (curJoint == NULL) continue;
					const SrMat& gMat = curJoint->gmat();
					SrMat& invBMat = skinWeight->bindPoseMat[skinWeight->jointNameIndex[globalCounter]];	
					double jointWeight = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
					globalCounter ++;
					finalVec = finalVec + (float(jointWeight) * (skinLocalVec * skinWeight->bindShapeMat * invBMat  * gMat));						
				}
				dMeshDynamic->shape().V[i] = finalVec;
			}
			dMeshDynamic->changed(true);	
		}
		else
			continue;
	}

	//printf("Max Influence Joints = %d\n",maxJoint);
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

void DeformableMesh::set_visibility(int deformableMesh)
{
	if (deformableMesh != -1)
	{
		for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
			dMeshDynamic_p[i]->visible(deformableMesh? true:false );

		binding = deformableMesh? true:false;		
	}
}

bool DeformableMesh::buildSkinnedVertexBuffer()
{	
	if (initSkinnedVertexBuffer) return true;

	if (initStaticVertexBuffer && !isSkinnedMesh()) return true;

	bool buildSkinnedBuffer = isSkinnedMesh();

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

	std::vector<int> meshIndexList;
	std::vector<SkinWeight*> skinWeightList;
	boneJointIdxMap.clear();
	if (buildSkinnedBuffer)
	{
		for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
		{
			SkinWeight* skinWeight = skinWeights[skinCounter];		
			int pos;
			int globalCounter = 0;
			//pos = this->getMesh(skinWeight->sourceMesh);
			pos = this->getValidSkinMesh(skinWeight->sourceMesh);
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

	for (unsigned int i=0;i<meshIndexList.size();i++)
	{
		int pos = meshIndexList[i];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		dMeshStatic->shape().computeTangentBiNormal();
		SrArray<SrMaterial>& matList = dMeshDynamic->shape().M; 	
		std::map<std::string,std::string> mtlTexMap = dMeshDynamic->shape().mtlTextureNameMap;
		std::map<std::string,std::string> mtlNormalTexMap = dMeshDynamic->shape().mtlNormalTexNameMap;		
		std::map<std::string,std::string> mtlSpecularTexMap = dMeshDynamic->shape().mtlSpecularTexNameMap;		
		for (int j=0;j<matList.size();j++)
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
	SrModel::Face defaultIdx;
	defaultIdx.a = defaultIdx.b = defaultIdx.c = -1;
		
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
		
		int numTris = dMeshStatic->shape().F.size();
		for (int i=0; i < numTris ; i++)
		{
			SrModel& model = dMeshStatic->shape();
			if (dMeshStatic->shape().F.size() == 0)
				continue;
			SrModel::Face& faceIdx = dMeshStatic->shape().F[i];	
			SrModel::Face nIdx;
			nIdx.set(faceIdx.a,faceIdx.b,faceIdx.c);
			if (dMeshStatic->shape().Fn.size() != 0)
			{
				SrModel::Face& fnIdx = dMeshStatic->shape().Fn[i];	
				nIdx.set(fnIdx.a,fnIdx.b,fnIdx.c);
			}
			//SrModel::Face& nIdx = dMeshStatic->shape().Fn[i];
			SrModel::Face& tIdx = defaultIdx;
			if (model.Ft.size() > i)
				tIdx = model.Ft[i];

			vtxNormalIdxMap[faceIdx.a + iFaceIdxOffset].insert(IntPair(nIdx.a+iNormalIdxOffset,tIdx.a+iTextureIdxOffset));
			vtxNormalIdxMap[faceIdx.b + iFaceIdxOffset].insert(IntPair(nIdx.b+iNormalIdxOffset,tIdx.b+iTextureIdxOffset));
			vtxNormalIdxMap[faceIdx.c + iFaceIdxOffset].insert(IntPair(nIdx.c+iNormalIdxOffset,tIdx.c+iTextureIdxOffset));

			int nMatIdx = 0; // if no corresponding materials, push into the default gray material group
			if (i < dMeshStatic->shape().Fm.size())
				nMatIdx = dMeshStatic->shape().Fm[i] + iMaterialOffset;		
			meshSubsetMap[nMatIdx].push_back(iFace);			
			iFace++;
		}
		iFaceIdxOffset += nVtx;
		iNormalIdxOffset += nNormal;
		iMaterialOffset += dMeshDynamic->shape().M.size();
		iTextureIdxOffset += nTexture;
		//printf("iMaterial Offset = %d\n",iMaterialOffset);			
	}

	if (nTotalVtxs == 0 || nTotalTris ==0)
		return false;

	//printf("orig vtxs = %d\n",nTotalVtxs);
	for (unsigned int i=0;i<vtxNormalIdxMap.size();i++)
	{
		std::set<IntPair>& idxMap = vtxNormalIdxMap[i];
		if (idxMap.size() > 1)
		{
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
	//printf("new vtxs = %d\n",nTotalVtxs);

	// temporary storage 
	posBuf.resize(nTotalVtxs);
	normalBuf.resize(nTotalVtxs); 
	tangentBuf.resize(nTotalVtxs); 
	binormalBuf.resize(nTotalVtxs);
	texCoordBuf.resize(nTotalVtxs);
	skinColorBuf.resize(nTotalVtxs);
	triBuf.resize(nTotalTris);

	if (buildSkinnedBuffer)
	{
		for (int i=0;i<2;i++)
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
	std::vector<SrVec> boneColorMap;
	float step = 1.f/boneJointList.size();
	float floatBuf[4];
	SrRandom random;	
	for (unsigned int i=0;i<boneJointList.size();i++)
	{
		//SrColor boneColor = SrColor::interphue(random.getf());
		SrColor boneColor = SrColor::interphue(i*step);
		boneColor.get(floatBuf);
		if (i == 3)
			boneColorMap.push_back(SrVec(1.f,0.f,0.f));
		else
			boneColorMap.push_back(SrVec(floatBuf[0],floatBuf[1],floatBuf[2]));				
	}
	
	for (unsigned int c=0;c<meshIndexList.size();c++)
	{
		int pos = meshIndexList[c];
		int globalCounter = 0;
		SrSnModel* dMeshStatic = dMeshStatic_p[pos];
		SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
		dMeshDynamic->visible(false);
		int numVertices = dMeshStatic->shape().V.size();
		int numNormals = dMeshStatic->shape().N.size();
		int numTexCoords = dMeshStatic->shape().T.size();
		SrMat bindShapeMat;
		SkinWeight* skinWeight = NULL;
		if (buildSkinnedBuffer)
		{
			skinWeight = skinWeightList[c];
			bindShapeMat = skinWeight->bindShapeMat;
		}
		for (int i = 0; i < numVertices; i++)
		{
			
			if (buildSkinnedBuffer)
			{				
				if (i >= (int) skinWeight->numInfJoints.size())
					continue;				
			}
			SrVec& lv = dMeshStatic->shape().V[i];					
			posBuf[iVtx] = lv*bindShapeMat;
			SrVec& lt =	dMeshStatic->shape().Tangent[i];		
			SrVec& lb = dMeshStatic->shape().BiNormal[i];
			tangentBuf[iVtx] = lt*bindShapeMat;
			binormalBuf[iVtx] = lb*bindShapeMat;
				
			//normalBuffer(iVtx) = Vec3f(ln[0],ln[1],ln[2]);

			if (buildSkinnedBuffer && skinWeight)
			{
				int numOfInfJoints = skinWeight->numInfJoints[i];
				for (int k=0;k<2;k++)
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
				for (int j=0;j<numWeight;j++)
				{
					if (j >= weightList.size())
						continue;
					IntFloatPair& w = weightList[j];
					weightSum += w.second;
					if ( j < 4)
					{
						boneIDBuf[0][iVtx][j] = w.first;
						boneIDBuf_f[0][iVtx][j] = (float)w.first;
						boneWeightBuf[0][iVtx][j] = w.second;
					}
					else if (j < 8)
					{
						boneIDBuf[1][iVtx][j-4] = w.first;
						boneIDBuf_f[1][iVtx][j-4] = (float)w.first;
						boneWeightBuf[1][iVtx][j-4] = w.second;
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
					boneWeightBuf[1][iVtx][j] /= weightSum;
				}	

			}
			

			if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
			{
				std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
				// copy related vtx components 
				for (unsigned int k=0;k<idxMap.size();k++)
				{
					posBuf[idxMap[k]] = posBuf[iVtx];
					tangentBuf[idxMap[k]] = tangentBuf[iVtx];
					binormalBuf[idxMap[k]] = binormalBuf[iVtx];

					if (buildSkinnedBuffer)
					{
						boneIDBuf[0][idxMap[k]] = boneIDBuf[0][iVtx];
						boneIDBuf[1][idxMap[k]] = boneIDBuf[1][iVtx];
						boneIDBuf_f[0][idxMap[k]] = boneIDBuf_f[0][iVtx];
						boneIDBuf_f[1][idxMap[k]] = boneIDBuf_f[1][iVtx];
						boneWeightBuf[0][idxMap[k]] = boneWeightBuf[0][iVtx];
						boneWeightBuf[1][idxMap[k]] = boneWeightBuf[1][iVtx];
						skinColorBuf[idxMap[k]] = skinColorBuf[iVtx];
					}	
				}
			}
			iVtx++;
		}

		int numTris = dMeshStatic->shape().F.size();
		for (int i=0; i < numTris ; i++)
		{
			if (dMeshStatic->shape().F.size() <= i)
				continue;				
			SrModel::Face& faceIdx = dMeshStatic->shape().F[i];
			SrModel::Face normalIdx;// = dMeshStatic->shape().F[i];
			normalIdx.set(faceIdx.a,faceIdx.b,faceIdx.c);

			if (dMeshStatic->shape().Fn.size() > i)
			{
				//normalIdx = dMeshStatic->shape().Fn[i];							
				SrModel::Face& fnIdx = dMeshStatic->shape().Fn[i];	
				normalIdx.set(fnIdx.a,fnIdx.b,fnIdx.c);
			}
			SrModel::Face& texCoordIdx = defaultIdx;
			if (dMeshStatic->shape().Ft.size() > i)
				texCoordIdx = dMeshStatic->shape().Ft[i];
			int fIdx[3] = { faceIdx.a, faceIdx.b, faceIdx.c};
			int nIdx[3] = { normalIdx.a, normalIdx.b, normalIdx.c};
			int tIdx[3] = { texCoordIdx.a, texCoordIdx.b, texCoordIdx.c};
				
			SrVec faceNormal = dMeshStatic->shape().face_normal(i);
			for (int k=0;k<3;k++)
			{
				SrVec nvec;
				SrPnt2 tvec = SrPnt2(0,0);
				int nidx = nIdx[k];
				if (dMeshStatic->shape().N.size() > nidx)
					nvec = dMeshStatic->shape().N[nIdx[k]];
				else
					nvec = faceNormal;
				if (dMeshStatic->shape().T.size() > tIdx[k] && dMeshStatic->shape().T.size() > 0 && dMeshStatic->shape().Ft.size() > 0)
					tvec = dMeshStatic->shape().T[tIdx[k]];
				int newNIdx = nIdx[k] + iNormalIdxOffset;
				int newTIdx = tIdx[k] + iTextureIdxOffset;
				int vIdx = fIdx[k] + iFaceIdxOffset;
				if (ntNewVtxIdxMap.find(IntPair(newNIdx,newTIdx)) != ntNewVtxIdxMap.end())
					vIdx = ntNewVtxIdxMap[IntPair(newNIdx,newTIdx)];
				normalBuf[vIdx] = nvec;
				texCoordBuf[vIdx] = SrVec2(tvec.x, tvec.y);
				triBuf[iFace][k] = vIdx;
			}			
			iFace++;
		}
		iFaceIdxOffset += numVertices;
		iNormalIdxOffset += numNormals;
		iTextureIdxOffset += numTexCoords;		
	}
    
    int group = 0;
	std::vector<SbmSubMesh*> hairMeshList;
	std::map<int,std::vector<int> >::iterator vi;
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
		for (unsigned int k=0;k<faceIdxList.size();k++)
		{
			mesh->triBuf[k][0] = triBuf[faceIdxList[k]][0];
			mesh->triBuf[k][1] = triBuf[faceIdxList[k]][1];
			mesh->triBuf[k][2] = triBuf[faceIdxList[k]][2];
		}
#if TEST_HAIR_RENDER
		mesh->matName = allMatNameList[iMaterial];
		SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE,mesh->texName.c_str());
		std::string lowMatName = mesh->matName;
		boost::algorithm::to_lower(lowMatName);
		//if (lowMatName.find("hair") != std::string::npos || lowMatName.find("lash") != std::string::npos 
		//	|| lowMatName.find("shadow") != std::string::npos || lowMatName.find("shell") != std::string::npos)
		if (lowMatName.find("hair") != std::string::npos)
		{
			// is a hair mesh, based on a rough name searching
			mesh->isHair = true;
			hairMeshList.push_back(mesh);
		}
		else if (tex && tex->isTransparent())
		{
			mesh->isHair = true;
			hairMeshList.push_back(mesh);
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
	initStaticVertexBuffer = true;
	if (buildSkinnedBuffer)
		initSkinnedVertexBuffer = true;
	return true;
}

bool DeformableMesh::isSkinnedMesh()
{
	return skinWeights.size() > 0;
}

/************************************************************************/
/* Deformable Mesh Instance                                             */
/************************************************************************/

DeformableMeshInstance::DeformableMeshInstance()
{
	_mesh = NULL;
	_skeleton = NULL;
	_updateMesh = false;
	_isStaticMesh = false;
	_recomputeNormal = true;
	_meshScale = 1.f;
	_character = NULL;
}

DeformableMeshInstance::~DeformableMeshInstance()
{
	cleanUp();
}


void DeformableMeshInstance::setPawn(SmartBody::SBPawn* pawn)
{
	//if (_skeleton)
	//	_skeleton->unref();
	if (pawn)
		_skeleton = pawn->getSkeleton();
	if (_skeleton)
		_skeleton->ref();
	_character = dynamic_cast<SmartBody::SBCharacter*>(pawn);
	updateJointList();
}

void DeformableMeshInstance::cleanUp()
{
#if 0
	for (unsigned int i = 0; i < dynamicMesh.size(); i++)
	{

		//SmartBody::SBScene::getScene()->getRootGroup()->remove(dynamicMesh[i]);
		//dynamicMesh[i]->unref();		
		//delete dynamicMesh[i];
	}
	dynamicMesh.clear();
#endif
}

void DeformableMeshInstance::blendShapes()
{
	if (!_character)
	{
		return;
	}

	// find the base shape from static meshes
	std::map<std::string, std::vector<SrSnModel*> >::iterator mIter;
	for (mIter = _mesh->blendShapeMap.begin(); mIter != _mesh->blendShapeMap.end(); ++mIter)
	{
		SrSnModel* writeToBaseModel = NULL;
		SrSnModel* baseModel = NULL;
		for (size_t i = 0; i < _mesh->dMeshStatic_p.size(); ++i)
		{
			if (strcmp(_mesh->dMeshStatic_p[i]->shape().name, mIter->first.c_str()) == 0)
			{
				writeToBaseModel = _mesh->dMeshStatic_p[i];
				break;
			}
		}

		if (writeToBaseModel == NULL)
		{
			LOG("base model to write to cannot be found");
			continue;
		}
		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (strcmp(mIter->first.c_str(), (const char*)mIter->second[i]->shape().name) == 0)
			{
				baseModel = mIter->second[i];
				break;
			}
		}
		if (baseModel == NULL)
		{
			LOG("original base model cannot be found");
			continue;
		}

		SrArray<SrPnt>& neutralV = baseModel->shape().V;
		SrArray<SrPnt>& neutralN = baseModel->shape().N;
		SrArray<SrPnt> newV = neutralV;
		SrArray<SrPnt> newN = neutralN;
		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (strcmp(mIter->first.c_str(), (const char*)mIter->second[i]->shape().name) == 0)
				continue;	// don't do anything about base model

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
				std::string mappedCName = _character->getStringAttribute(ss.str());
				SmartBody::SBSkeleton* sbSkel = _character->getSkeleton();
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

			if (fabs(w) > gwiz::epsilon4())	// if it has weight
			{
				//LOG("blend in %s with weight %f", (const char*)mIter->second[i]->shape().name, w);
				SrArray<SrPnt>& visemeV = mIter->second[i]->shape().V;
				SrArray<SrPnt>& visemeN = mIter->second[i]->shape().N;
				if (visemeV.size() != neutralV.size())
				{
					LOG("number of vertices for %s is not same as neutral", mIter->first.c_str());
					continue;
				}
				if (visemeN.size() != neutralN.size())
				{
					LOG("number of normals for %s is not same as neutral", mIter->first.c_str());
					continue;
				}
				for (int v = 0; v < visemeV.size(); ++v)
				{
					SrPnt diff = visemeV[v] - neutralV[v];
					newV[v] = newV[v] + diff * w;
				}
				for (int n = 0; n < visemeN.size(); ++n)
				{
					SrPnt diff = visemeN[n] - neutralN[n];
					newN[n] = newN[n] + diff * w;
				}
			}
		}
		for (int n = 0; n < newN.size(); ++n)
		{
			newN[n].normalize();
		}

		writeToBaseModel->shape().V = newV;
		writeToBaseModel->shape().N = newN;
	}

	return;

#if 0
	for (size_t i = 0; i < dMeshBlend_p.size(); ++i)
	{
		delete dMeshBlend_p[i];
	}
	dMeshBlend_p.clear();

	for (size_t i = 0; i < dMeshStatic_p.size(); ++i)
	{
		delete dMeshStatic_p[i];
	}
	dMeshStatic_p.clear();

	if (visemeShapeMap.find("neutral") == visemeShapeMap.end())
	{
		LOG("neutral blend shape has not been defined!");
		return;
	}



	std::vector<SrSnModel*>& neutralModel = visemeShapeMap["neutral"];
	for (size_t i = 0; i < neutralModel.size(); ++i)
	{
		SrArray<SrPnt>& neutralV = neutralModel[i]->shape().V;
		SrArray<SrPnt>& neutralN = neutralModel[i]->shape().N;
		SrArray<SrPnt> newV = neutralV;
		SrArray<SrPnt> newN = neutralN;

		bool isBlending = false;
		std::map<std::string, float>::iterator iter;
		for (iter = visemeWeightMap.begin(); iter != visemeWeightMap.end(); ++iter)
		{
			for (size_t j = 0; j < visemeShapeMap[iter->first].size(); j++)
			{
				LOG("Shape %s", (const char*)visemeShapeMap[iter->first][j]->shape().name);
			}
			LOG("%s", (const char*) neutralModel[i]->shape().name);
			if (iter->second > 0.01f && iter->first != "neutral")
			{
				for (size_t j = 0; j < visemeShapeMap[iter->first].size(); j++)
				{
					if (visemeShapeMap[iter->first][j]->shape().name != neutralModel[i]->shape().name)
						continue;

					isBlending = true;
					SrArray<SrPnt>& visemeV = visemeShapeMap[iter->first][j]->shape().V;
					SrArray<SrPnt>& visemeN = visemeShapeMap[iter->first][j]->shape().N;
					if (visemeV.size() != neutralV.size())
					{
						LOG("number of vertices for %s is not same as neutral", iter->first.c_str());
						continue;
					}
					if (visemeN.size() != neutralN.size())
					{
						LOG("number of normals for %s is not same as neutral", iter->first.c_str());
						continue;
					}
					for (int v = 0; v < visemeV.size(); ++v)
					{
						SrPnt diff = visemeV[v] - neutralV[v];
						newV[v] = newV[v] + diff * iter->second;
					}
					for (int n = 0; n < visemeN.size(); ++n)
					{
						SrPnt diff = visemeN[n] - neutralN[n];
						newN[n] = newN[n] + diff * iter->second;
					}
				}
			}
		}
		for (int n = 0; n < newN.size(); ++n)
		{
			newN[n].normalize();
		}

		SrSnModel* blendedModel = new SrSnModel();
		//SrSnModel* blendedModel = dMeshStatic_p[i];
		blendedModel->shape(neutralModel[i]->shape());
		if (isBlending)
		{
			blendedModel->shape().V = newV;
			blendedModel->shape().N = newN;			
		}
		dMeshStatic_p.push_back(blendedModel);
	}
#endif
}


void DeformableMeshInstance::setDeformableMesh( DeformableMesh* mesh )
{
    //LOG("setDeformableMesh to be %s", mesh->meshName.c_str());
	_mesh = mesh;
	cleanUp(); // remove all previous dynamic mesh
	_mesh->buildSkinnedVertexBuffer(); // make sure the deformable mesh has vertex buffer
#if 0
	for (unsigned int i=0;i<_mesh->dMeshStatic_p.size();i++)
	{
		SrSnModel* srSnModel = _mesh->dMeshStatic_p[i];
		SrSnModel* srSnModelDynamic = new SrSnModel();
		srSnModelDynamic->shape(srSnModel->shape());
		srSnModelDynamic->changed(true);
		srSnModelDynamic->visible(false);
		srSnModelDynamic->shape().name = srSnModel->shape().name;	
		dynamicMesh.push_back(srSnModelDynamic);
		srSnModelDynamic->ref();
		//LOG("rootGroup add mesh %d, vtx = %d, face = %d",i, dynamicMesh[i]->shape().V.size(), dynamicMesh[i]->shape().F.size());
		//SmartBody::SBScene::getScene()->getRootGroup()->add(dynamicMesh[i]);
	}
#endif
	_deformPosBuf.resize(_mesh->posBuf.size()); // initialized deformation posBuffer
	for (unsigned int i=0;i<_deformPosBuf.size();i++)
		_deformPosBuf[i] = _mesh->posBuf[i];
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
		//for (unsigned int i = 0; i < dynamicMesh.size(); i++)
		//	dynamicMesh[i]->visible(deformableMesh? true:false );
		_updateMesh = deformableMesh? true:false;
		meshVisible = deformableMesh? true:false;
	}
}

void DeformableMeshInstance::update()
{
	blendShapes();

#define RECOMPUTE_NORMAL 0
	if (!_updateMesh)	return;
	if (!_skeleton || !_mesh) return;	
	if (isStaticMesh()) return; // not update the buffer if it's a static mesh
	_skeleton->update_global_matrices();
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
				//printf("Vtx bind pose = \n");

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
					SrVec transformVec = _meshScale*(skinLocalVec * skinWeight->bindShapeMat * invBMat);
					finalVec = finalVec + (float(jointWeight) * (transformVec  * gMat));		
					//finalVec = finalVec + (float(jointWeight) * (skinLocalVec * skinWeight->bindShapeMat * invBMat  * gMat));	
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

bool DeformableMeshInstance::getVisibility()
{
	return meshVisible;
}

void DeformableMeshInstance::setMeshScale( float scale )
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
	if (!_mesh) return;
	if (_mesh->blendShapeMap.size() == 0) return;

	DeformableMeshInstance::blendShapes();

	std::map<std::string, std::vector<SrSnModel*> >::iterator mIter;
	mIter = _mesh->blendShapeMap.begin();
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
	if (!writeToBaseModel) return;
	std::map<int,std::vector<int> >& vtxNewVtxIdxMap = _mesh->vtxNewVtxIdxMap;
	SrModel& baseModel = writeToBaseModel->shape();
	for (int i=0;i<baseModel.V.size();i++)
	{
		int iVtx = vtxBaseIdx+i;
		SrVec& basePos = baseModel.V[i];
		_deformPosBuf[iVtx] = basePos;
		if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
		{
			std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
			// copy related vtx components 
			for (unsigned int k=0;k<idxMap.size();k++)
			{
				int idx = idxMap[k];
				_deformPosBuf[idx] = basePos;
			}
		}			
	}	

}
