#include "sbm_deformable_mesh.h"
#include "mcontrol_util.h"

DeformableMesh::DeformableMesh() 
{
	binding = false;
	skeleton = new SkSkeleton();
}

DeformableMesh::~DeformableMesh() 
{
	delete skeleton;
}


void DeformableMesh::update()
{
	if (!binding)	return;
	skeleton->update_global_matrices();
	for (unsigned int counter = 0; counter < skinWeights.size(); counter++)
	{
		int globalCounter = 0;
		SkinWeight* skinWeight = skinWeights[counter];
		int pos = this->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			SrSnModel* dMeshStatic = dMeshStatic_p[pos];
			SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
			int numVertices = dMeshStatic->shape().V.size();
			for (int i = 0; i < numVertices; i++)
			{
				int numOfInfJoints = skinWeight->numInfJoints[i];
				SrVec& skinLocalVec = dMeshStatic->shape().V[i];
				SrVec finalVec;
				for (int j = 0; j < numOfInfJoints; j++)
				{
					const SkJoint* curJoint = skinWeight->infJoint[skinWeight->jointNameIndex[globalCounter]];
					const SrMat& gMat = curJoint->gmat();
					SrMat& invBMat = skinWeight->bindPoseMat[skinWeight->jointNameIndex[globalCounter]];	
					double jointWeight = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
					globalCounter ++;
					finalVec = finalVec + (float(jointWeight) * (skinLocalVec * invBMat * gMat));
				}
				dMeshDynamic->shape().V[i] = finalVec;
			}
			dMeshDynamic->changed(true);	
		}
		else
			continue;
	}
}

SkinWeight* DeformableMesh::getSkinWeight(std::string skinSourceName)
{
	for (unsigned int i = 0; i < skinWeights.size(); i++)
	{
		if (skinSourceName == skinWeights[i]->sourceMesh)
			return skinWeights[i];
	}
	return NULL;
}

int	DeformableMesh::getMesh(std::string meshName)
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