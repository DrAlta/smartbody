#include "SBAutoRigManager.h"
#ifdef WIN32
#include "pinocchioApi.h"
#endif
#include <vhcl.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sb/SBJoint.h>
#include <sb/SBAssetManager.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sbm/GPU/SbmDeformableMeshGPU.h>

#ifdef WIN32
bool SrModelToMesh( SrModel& model, Mesh& mesh );
bool AutoRigToSBSk( PinocchioOutput& out, Skeleton& sk, SmartBody::SBSkeleton& sbSk);
bool AutoRigToDeformableMesh(PinocchioOutput& out, SrModel& m, SmartBody::SBSkeleton& sbSk, DeformableMesh& deformMesh);
#endif

SBAutoRigManager* SBAutoRigManager::_singleton = NULL;
SBAutoRigManager::SBAutoRigManager()
{
	
}

SBAutoRigManager::~SBAutoRigManager()
{

}

bool SBAutoRigManager::buildAutoRigging( SrModel& inModel, std::string outSkName, std::string outDeformableMeshName )
{
#ifdef WIN32
	Mesh m;
	//Skeleton sk = HumanSkeleton(); // default human skeleton from Pinocchio. Should define our own custom skeleton to account for gaze and other behavior
	Skeleton sk = SmartBodySkeleton();
	inModel.computeNormals();
	bool isValidModel = SrModelToMesh(inModel,m);
	if (!isValidModel) return false; // no auto-rigging if the model is not valid
	PinocchioOutput out = autorig(sk,m);	
	if (out.embedding.size() == 0)
		return false; // no embedding

	for(int i = 0; i < (int)out.embedding.size(); ++i)
		out.embedding[i] = (out.embedding[i] - m.toAdd) / m.scale;
	SmartBody::SBSkeleton* sbSk = new SmartBody::SBSkeleton();
	//sbSk->setName("testAutoRig.sk");
	//sbSk->setFileName()
	bool isValidSkeleton = AutoRigToSBSk(out, sk, *sbSk);	
	//LOG("autoRig result, num joints = %d",out.embedding.size());
	//LOG("AutoRig Skeleton = %s", sbSk->saveToString().c_str());
	SbmDeformableMeshGPU* deformMesh = new SbmDeformableMeshGPU();
	deformMesh->meshName = outDeformableMeshName;	
	bool isValidDeformableMesh = AutoRigToDeformableMesh(out, inModel, *sbSk, *deformMesh);

	SmartBody::SBAssetManager* assetManager = SmartBody::SBScene::getScene()->getAssetManager();

	sbSk->ref();
	sbSk->skfilename(outSkName.c_str());				
	sbSk->setName(outSkName.c_str());
	deformMesh->skeletonName = outSkName;

	assetManager->addSkeleton(sbSk);
	assetManager->addDeformableMesh(outDeformableMeshName, deformMesh);
#endif
	return false;
}


#ifdef WIN32
bool AutoRigToDeformableMesh( PinocchioOutput& out, SrModel& m, SmartBody::SBSkeleton& sbSk, DeformableMesh& deformMesh )
{
	//deformMesh.dMeshStatic_p.push_back()
	// setup mesh
	std::string meshName = m.name;
	SrSnModel* srSnModelDynamic = new SrSnModel();
	SrSnModel* srSnModelStatic = new SrSnModel();
	srSnModelDynamic->shape(m);
	srSnModelStatic->shape(m);
	srSnModelDynamic->changed(true);
	srSnModelDynamic->visible(false);
	srSnModelStatic->shape().name = meshName.c_str();
	srSnModelDynamic->shape().name = meshName.c_str();	
	deformMesh.dMeshStatic_p.push_back(srSnModelStatic);
	deformMesh.dMeshDynamic_p.push_back(srSnModelDynamic);
	srSnModelDynamic->ref();
	srSnModelStatic->ref();

	// setup skin weights
	//int boneIdxMap[] = { 1, 2, 0, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 0, 15, 16  }; // hard coded for HumanSkeleton for now
	//int boneIdxMap[] = { 1, 2, 3, 4, 0, 5, 4, 7, 8, 9, 4, 11, 12, 13, 0, 15, 16, 0, 18, 19};
	//int boneIdxMap[] = {1, 2, 3}
	int prevJointIdx[] = { 1, 2, 3, 4, -1, 0, 5, 6,  4, 8, 9, 10, 11, 4, 13, 14, 15, 16, 0, 18, 19, 20, 0, 22, 23, 24};
	std::vector<int> boneIdxMap;
	for (int i=0;i< (int) out.embedding.size();i++)
	{
		int boneIdx = prevJointIdx[i];
		if (boneIdx < 0)
			continue;
		else
			boneIdxMap.push_back(boneIdx);
	}

	
	SkinWeight* sw = new SkinWeight();
	sw->sourceMesh = meshName;	
	for (unsigned int i=0;i< (size_t) m.V.size();i++)
	{
		Vector<double, -1> v = out.attachment->getWeights(i);
		double maxD = -1.0;
		int maxIdx = -1;
		std::map<int, float> weights;
		for(int j = 0; j < v.size(); ++j) {
			double d = floor(0.5 + v[j] * 10000.) / 10000.;
			if (d > 0.01) // remove small weights
			{
				int boneIdx = boneIdxMap[j];
				if (weights.find(boneIdx) == weights.end())
				{
					weights[boneIdx] = 0.f;
				}
				weights[boneIdx] += (float) d;
			}		
		}
		std::map<int,float>::iterator mi;
		sw->numInfJoints.push_back(weights.size());
		for ( mi  = weights.begin(); mi != weights.end(); mi++)
		{
			sw->weightIndex.push_back(sw->bindWeight.size());
			sw->bindWeight.push_back(mi->second);
			sw->jointNameIndex.push_back(mi->first);				
		}
	}
	sw->normalizeWeights();
	deformMesh.skinWeights.push_back(sw);

	SmartBody::SBSkeleton* sbOrigSk = &sbSk;
	for (int k=0;k<sbOrigSk->getNumJoints();k++)
	{
		// manually add all joint names
		SmartBody::SBJoint* joint = sbOrigSk->getJoint(k);
		sw->infJointName.push_back(joint->getName());
		sw->infJoint.push_back(joint);
		SrMat gmatZeroInv = joint->gmatZero().rigidInverse();						
		sw->bindPoseMat.push_back(gmatZeroInv);
	}
	
	return true;
}

bool AutoRigToSBSk( PinocchioOutput& out, Skeleton& sk, SmartBody::SBSkeleton& sbSk)
{
	//std::string jointNameMap[] = {"spine3","spine1", "base", "skullbase", "r_hip", "r_knee", "r_ankle", "r_forefoot", "l_hip", "l_knee", "l_ankle", "l_forefoot", "r_shoulder", "r_elbow", "r_wrist", "l_shoulder", "l_elbow", "l_wrist" };
	//int prevJointIdx[] = { 1, 2, -1, 0, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 0, 15, 16};

	// SmartBody skeleton
	std::string jointNameMap[] = {"spine4", "spine3", "spine2", "spine1", "base", "spine5", "skullbase", "head", "r_hip", "r_knee", "r_ankle", "r_forefoot", "r_toe", "l_hip", "l_knee", "l_ankle", "l_forefoot", "l_toe", "r_shoulder", "r_elbow", "r_wrist", "r_hand", "l_shoulder", "l_elbow", "l_wrist", "l_hand" };
	int prevJointIdx[] = { 1, 2, 3, 4, -1, 0, 5, 6,  4, 8, 9, 10, 11, 4, 13, 14, 15, 16, 0, 18, 19, 20, 0, 22, 23, 24};

	// build all joints
	for(int i = 0; i < (int)out.embedding.size(); ++i) // number of joints
	{	
		SmartBody::SBJoint* joint = dynamic_cast<SmartBody::SBJoint*>(sbSk.add_joint(SkJoint::TypeQuat, -1));
		joint->quat()->activate();
		joint->name(jointNameMap[i]);
		joint->extName(jointNameMap[i]);
		joint->extID(jointNameMap[i]);
		joint->extSID(jointNameMap[i]);

		sbSk.channels().add(joint->jointName(), SkChannel::XPos);
		sbSk.channels().add(joint->jointName(), SkChannel::YPos);
		sbSk.channels().add(joint->jointName(), SkChannel::ZPos);
		joint->pos()->limits(SkVecLimits::X, false);
		joint->pos()->limits(SkVecLimits::Y, false);
		joint->pos()->limits(SkVecLimits::Z, false);
		sbSk.channels().add(joint->jointName(), SkChannel::Quat);
		joint->quat()->activate();

		float pos[3];
		for (int j=0;j<3;j++)
		{
			pos[j] = (float) out.embedding[i][j];
			if (prevJointIdx[i] != -1)
			{
				pos[j] -= (float) out.embedding[prevJointIdx[i]][j];
			}
		}	
		joint->setOffset(SrVec(pos[0],pos[1],pos[2]));
	}
	// setup joint hierarchy
	for (unsigned int i=0;i< (size_t) sbSk.getNumJoints();i++)
	{
		SmartBody::SBJoint* joint = sbSk.getJoint(i);
		int parentIdx = prevJointIdx[i];
		if (parentIdx != -1)
		{
			//joint->setParent(sbSk.getJoint(parentIdx));
			sbSk.getJoint(parentIdx)->add_child(joint);
		}
		else // root joint
		{
			sbSk.root(joint);
		}
	}
	LOG("sbSk, num of joints = %d",sbSk.getNumJoints());
	sbSk.updateGlobalMatricesZero();

	return true;
}

bool SrModelToMesh( SrModel& model, Mesh& mesh )
{
	mesh.vertices.resize(model.V.size());
	for (int i=0;i<model.V.size();i++)
	{
		SrPnt& p = model.V[i];
		mesh.vertices[i].pos = Vector3(p.x,p.y,p.z);
	}

	mesh.edges.resize(model.F.size()*3);
	for (int i=0;i<model.F.size();i++)
	{
		SrModel::Face& f = model.F[i];
		mesh.edges[i*3].vertex = f.a;
		mesh.edges[i*3+1].vertex = f.b;
		mesh.edges[i*3+2].vertex = f.c;
	}

	mesh.fixDupFaces();
	mesh.computeTopology();
	if(!mesh.integrityCheck())
	{		
		//Debugging::out() << "Successfully read " << file << ": " << vertices.size() << " vertices, " << edges.size() << " edges" << endl;
		LOG("Error : SBAutoRigManager::SrModelToMesh Fail.");
		return false;
	}	
	mesh.normalizeBoundingBox();
	mesh.computeVertexNormals();
	return true;
}
#endif

