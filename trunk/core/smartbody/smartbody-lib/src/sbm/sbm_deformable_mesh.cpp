
#include "vhcl.h"

#include "sbm_deformable_mesh.h"
#include "mcontrol_util.h"
#include <sbm/SBSkeleton.h>


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

DeformableMesh::DeformableMesh() 
{
	binding = false;
	skeleton = new SmartBody::SBSkeleton();
	skeleton->ref();
}

DeformableMesh::~DeformableMesh() 
{
	skeleton->unref();
	for (unsigned int i = 0; i < dMeshDynamic_p.size(); i++)
		dMeshDynamic_p[i]->unref();
	dMeshDynamic_p.clear();
	for (unsigned int i = 0; i < dMeshStatic_p.size(); i++)
		dMeshStatic_p[i]->unref();
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
}


void DeformableMesh::setSkeleton(SkSkeleton* skel)
{
	if (skeleton)
		skeleton->unref();
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
		size_t morphSize = 1;
		if (iter != this->morphTargets.end())	morphSize = iter->second.size();	
		for (size_t morphCounter = 0; morphCounter < morphSize; morphCounter++)
		{	
			int pos;
			int globalCounter = 0;
			if (iter != this->morphTargets.end())	pos = this->getMesh(iter->second[morphCounter]);
			else									pos = this->getMesh(skinWeight->sourceMesh);
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
					if (numOfInfJoints > maxJoint)
						maxJoint = numOfInfJoints;
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
