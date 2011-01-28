#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include "me_ct_data_driven_reach.hpp"

bool MeCtDataDrivenReach::useDataDriven = true;
bool MeCtDataDrivenReach::useIK = true;



PoseExample& PoseExample::operator=( const PoseExample& rhs )
{
	jointQuat = rhs.jointQuat;
	poseParameter = rhs.poseParameter;
	return *this;
}


MeCtDataDrivenReach::MeCtDataDrivenReach(SkSkeleton* sk) : MeCtReach(sk)
{
}

MeCtDataDrivenReach::~MeCtDataDrivenReach(void)
{
}

void MeCtDataDrivenReach::addMotion( SkMotion* motion )
{
	if (!motion)
		return;
	motionData.push(motion);
}


void MeCtDataDrivenReach::blendPose( SrArray<SrQuat>& blendPoses, SrArray<float>& KNNweights, SrArray<PoseExample*>& KNNPoses )
{
	blendPoses = KNNPoses[0]->jointQuat;
	float weightSum = KNNweights[0];
	float w1,w2;	
	for (int i=1;i<KNNweights.size();i++)
	{
		w1 = weightSum;
		w2 = KNNweights[i];
		weightSum += w2;
		PoseExample* pose = KNNPoses[i];
		for (int k=0;k<blendPoses.size();k++)
		{
			blendPoses[k] = slerp( blendPoses[k], pose->jointQuat[k], w2/weightSum );
		}
	}
}

int MeCtDataDrivenReach::linearKNN(VecOfPoseExample& curList, PoseExample& newData, int nK,  SrArray<float>& distList, SrArray<PoseExample*>& KNN )
{
	KNN.size(nK+1);
	distList.size(nK+1);
	distList.setall(1e10);
	int nCurK = 0;
	for (int i=0;i<curList.size();i++)
	{
		PoseExample& curEx = curList[i];
		double dist = sqrt(Dist(curEx.poseParameter,newData.poseParameter));
		int k;
		for (k=nCurK;k>0;k--)
		{
			if (distList[k-1] > dist)
			{
				distList[k] = distList[k-1]; // shift large value out
			}
			else 
				break;
		}		
		distList[k] = dist;
		KNN[k]  = &curEx;
		if (nCurK < nK)
			nCurK++;
	}
	distList.pop();
	KNN.pop();
	return nCurK; 	
}

void MeCtDataDrivenReach::generateRandomWeight( int nK, SrArray<float>& outWeights )
{
	float delta = 0.1;
	outWeights.size(nK);
	float weightSum = 0.0;
	for (int i=0;i<nK-1;i++)
	{
		float w = Random(std::max(-delta, -delta-weightSum),std::min(1.f+delta,1.f+delta-weightSum));
		outWeights[i] = w;
		weightSum += w;
	}	
	outWeights[nK-1] = 1.f - weightSum;
}

void MeCtDataDrivenReach::computeWeightFromDists( SrArray<float>& dists, SrArray<float>& outWeights )
{
	int nK = dists.size();
	outWeights.size(nK);

	float weightSum = 0.f;
	for (int i=0;i<nK;i++)
	{
		float weight = 1.0/dists[i] - 1.0/dists[nK-1];
		weightSum += weight;
		outWeights[i] = weight;
	}

	std::stringstream strstr;
	strstr << "weights = ";		
	for (int i=0;i<nK;i++)
	{
		outWeights[i] /= weightSum;
		strstr << outWeights[i] << " ,";				
	}
	strstr << std::endl;
	//LOG(strstr.str().c_str());
}

void MeCtDataDrivenReach::findKNNPoseExamples(SrArray<double>& parameter, SrArray<float>& KNNweights, SrArray<PoseExample*>& KNNPoses, int nK )
{
	SrArray<float> distList;
	distList.size(nK);
	KNNweights.size(nK);
	KNNPoses.size(nK);		
	ANNidxArray nnIdx;
	ANNdistArray dists;
	nnIdx = new ANNidx[nK];
	dists = new ANNdist[nK];

	
	const double* paraData = (const double*)(parameter);
	kdTree->annkSearch((double*)paraData,nK,nnIdx,dists);

	float weightSum = 0.f;
	for (int i=0;i<nK;i++)
	{
		int index = nnIdx[i];		
		distList[i] = dists[i];
		KNNPoses[i] = &examplePoseData[index];
	}
	computeWeightFromDists(distList,KNNweights);
}

bool MeCtDataDrivenReach::controller_evaluate( double t, MeFrameData& frame )
{
	float dt = 0.001f;
	if (prev_time == -1.0) // first start
	{
		dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		limb.updateQuat(frame,true);
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}
	prev_time = (float)t;	

	
	if (useDataDriven)
	{
		_skeleton->update_global_matrices();
		const SrMat& rootMat = _skeleton->root()->gmat();
		//SrMat rootMat;
		//rootMat.translation(getRootJoint()->gmat().get(12),getRootJoint()->gmat().get(13),getRootJoint()->gmat().get(14));
		SrMat rootInv = rootMat.inverse();
		SrVec targetPos = get_reach_target()*rootInv; // get target position to find KNN
		// find KNN and corresponding distance to determine the blending weights
		SrArray<double> targetParameter;
		targetParameter.size(3);

		int nK = 1; // nearby examples should be (parameter dim + 1)
		SrArray<float> KNNWeight;
		SrArray<PoseExample*> KNNPoses;

		for (int i=0;i<3;i++)
			targetParameter[i] = targetPos[i];

		findKNNPoseExamples(targetParameter,KNNWeight,KNNPoses,nK);
		blendPose(limb.joint_quat,KNNWeight,KNNPoses);
	}
	// solve CCD IK

	if (useIK)
	{
		MeCtIKScenario* ik_scenario = &limb.ik;

		ik_scenario->ik_offset = get_reach_target(); // set the target
		ik_scenario->ik_quat_orientation = SrQuat(0,0,0,1.0); // set to default rotation for testing
		ik_scenario->joint_quat_list = limb.joint_quat;	

		limb.skeleton->update_global_matrices();
		SkJoint* chain_root = limb.getChainRoot();
		ik_scenario->gmat = chain_root->parent()->gmat();

		ik.setDt(dt);
		ik.update(ik_scenario);
		limb.joint_quat = ik_scenario->joint_quat_list;
	}
	

	// write results from limb to buffer
	limb.updateQuat(frame,false);
	return true;
}

void MeCtDataDrivenReach::buildResamplePoseData( float fMinDist /*= 1.0*/ )
{
	// build a more densely sampled, but more evenly spaced example space
	/*			
	for (int i=0;i<examplePoseData.size();i++)
	{
		SrArray<PoseExample*> KNN;
		SrArray<float> dists;	
		int nK = linearKNN(resamplePosedata,examplePoseData[i],1,dists,KNN);
		if (dists[0] > fMinDist)
		{
			resamplePosedata.push_back(examplePoseData[i]);
		}			
	}

	
	// infer the bounding box from resample pose data
	SrBox BBox;
	for (int i=0;i<resamplePosedata.size();i++)
	{
		PoseExample& ex = resamplePosedata[i];
		SrVec pt = SrVec(ex.poseParameter[0],ex.poseParameter[1],ex.poseParameter[2]);
		BBox.extend(pt);
	}

	int nCount = 0;
	int nSamples = 300;
	int nKNN = 4;
	PoseExample dummy;
	dummy.poseParameter.size(3);
	// sampling new poses inside the bounding box by interpolating current pose data randomly
	srand(time(NULL));
	SrArray<SrQuat> blendQuats;
	while (nCount < nSamples)
	{
		SrVec pt = randomPointInBox(BBox);
		for (int i=0;i<3;i++)
			dummy.poseParameter[i] = pt[i];

		// find 
		SrArray<PoseExample*> KNN;
		SrArray<float> dists, weights;	
		linearKNN(resamplePosedata,dummy,nKNN,dists,KNN);		
		//computeWeightFromDists(dists,weights);		
		generateRandomWeight(nKNN,weights);
		blendPose(blendQuats,weights,KNN);
		limb.updateQuatToJointChain(blendQuats);
	}	
	examplePoseData = resamplePosedata;
	*/
	// build KD-tree for KNN search
	int nPts = examplePoseData.size(); // actual number of data points
	int dim = 3;	
	dataPts = annAllocPts(nPts, dim); // allocate data points	
	for (int i=0;i<nPts;i++)
	{
		memcpy(dataPts[i],(const double*)(examplePoseData[i].poseParameter),sizeof(ANNcoord)*dim);
	}	

	kdTree = new ANNkd_tree( // build search structure
		dataPts, // the data points
		nPts, // number of points
		dim); // dimension of space	 
}

void MeCtDataDrivenReach::buildPoseExamplesFromMotions()
{
	// ensure we have a skeleton
	assert(_skeleton);
	int nExamples = 0;
	// get total number of frames 
	for (int i=0;i<motionData.size();i++)
		nExamples += motionData[i]->frames();
	
	//examplePoseData.capacity(nExamples);
	//examplePoseData.size(nExamples);

	int poseIndex = 0;
	for (int i=0;i<motionData.size();i++)
	{
		SkMotion* motion = motionData[i];
		motion->connect(_skeleton);
		for (int j=0;j<motion->frames();j++)
		{
			PoseExample poseEx;
			//motion->apply_frame(j);
			limb.updateMotionFrameToJointChain(motion,j);
			_skeleton->update_global_matrices();
			examplePoseData.push_back(PoseExample());			
			getPoseExampleFromSkeleton(examplePoseData[poseIndex]);			
			poseIndex++;
		}
		motion->disconnect();
	}		
}

void MeCtDataDrivenReach::getPoseExampleFromSkeleton( PoseExample& pose )
{
	// get current skeleton configuration and save it as pose
	SrArray<SkJoint*>& limbJoints = limb.joint_chain;	
	pose.jointQuat.leave_data();
	pose.jointQuat.capacity(limbJoints.size());
	pose.jointQuat.size(limbJoints.size());
	pose.poseParameter.leave_data();
	pose.poseParameter.size(3);
	// use end effector's local position as parameter		
	getPoseParameter(pose.poseParameter,_skeleton);
	// get joint quat 
	for (int i=0;i<limbJoints.size();i++)
	{
		pose.jointQuat[i] = limbJoints[i]->quat()->value();
	}		
}



void MeCtDataDrivenReach::getPoseParameter( SrArray<double>& para, SkSkeleton* skeleton )
{
	const SrMat& endMat = limb.getChainEndEffector()->gmat();
	const SrMat& rootMat = _skeleton->root()->gmat();//limb.getChainRoot()->gmat();//_skeleton->root()->gmat();
	SrMat rootInv = rootMat.inverse();
	//rootInv.translation(rootMat.get(12),rootMat.get(13),rootMat.get(14));
	//rootInv.invert();
	SrVec endGlobal = SrVec(endMat.get(12), endMat.get(13), endMat.get(14));
	SrVec endLocal = endGlobal*rootInv;
	int nSize = 3;
	if (para.size() != nSize)
		para.size(nSize);

	for (int i=0;i<nSize;i++)
		para[i] = endLocal[i];
}

SrVec MeCtDataDrivenReach::randomPointInBox( SrBox& box )
{
	float fx,fy,fz;
	// generate 
	fx = Random(0.f,1.f);
	fy = Random(0.f,1.f);
	fz = Random(0.f,1.f);
	SrVec offset = (box.b - box.a);
	SrVec pt = box.a + SrVec(offset.x*fx,offset.y*fy,offset.z*fz);
	
	return pt;
}

float MeCtDataDrivenReach::Random( float r_min, float r_max )
{
	float frand = (float)rand()/(float)RAND_MAX; 
	frand = r_min + frand*(r_max-r_min);
	return frand;
}