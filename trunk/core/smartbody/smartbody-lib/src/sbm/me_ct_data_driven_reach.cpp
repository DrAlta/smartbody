#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "me_ct_data_driven_reach.hpp"

using namespace boost;

PoseExample& PoseExample::operator=( const PoseExample& rhs )
{
	jointQuat = rhs.jointQuat;
	poseParameter = rhs.poseParameter;
	return *this;
}

/************************************************************************/
/* Pose Example Set with KD-tree based KNN search                       */
/************************************************************************/

#define FREE_DATA(data) if (data) delete data; data=NULL;
#define FREE_DATA_ARRAY(data) if (data) delete [] data; data=NULL;


PoseExampleSet::PoseExampleSet()
{
	kdTree = NULL;
	dataPts = NULL;

}

PoseExampleSet::~PoseExampleSet()
{
	FREE_DATA(kdTree);
	//FREE_DATA_ARRAY(dataPts);
	if (dataPts)
	{
		annDeallocPts(dataPts);
	}
}

bool PoseExampleSet::addPose( const PoseExample& poseEx, float fMinDist )
{
	SrArray<float> dists;
	SrArray<PoseExample*> KNN;
	if (fMinDist <= 0.f) // just add the pose
	{
		poseData.push_back(poseEx);	
		return true;
	}
	else
	{
		int nK = linearKNN(poseEx.poseParameter,dists,KNN,1); // find closest pose data in parameter space
		if (dists[0] > fMinDist)
		{
			poseData.push_back(poseEx);
			return true;
		}
	}
	return false;
}


int PoseExampleSet::linearKNN( const SrArray<double>& parameter, SrArray<float>& dists, SrArray<PoseExample*>& KNN, int nK )
{
	KNN.size(nK+1);
	dists.size(nK+1);
	dists.setall(1e10);
	int nCurK = 0;
	for (int i=0;i<poseData.size();i++)
	{
		PoseExample& curEx = poseData[i];
		double dist = sqrt(Dist(curEx.poseParameter,parameter));
		int k;
		for (k=nCurK;k>0;k--)
		{
			if (dists[k-1] > dist)
			{
				dists[k] = dists[k-1]; // shift large value out
				KNN[k] = KNN[k-1];
			}
			else 
				break;
		}		
		dists[k] = dist;
		KNN[k]  = &poseData[i];
		if (nCurK < nK)
			nCurK++;
	}
	dists.pop();
	KNN.pop();
	return nCurK; 	
}


void PoseExampleSet::kdTreeKNN( const SrArray<double>& parameter, SrArray<float>& dists, SrArray<PoseExample*>& KNN, int nK )
{	
	dists.size(nK);
	KNN.size(nK);		
	ANNidxArray nnIdx;
	ANNdistArray nnDists;
	nnIdx = new ANNidx[nK];
	nnDists = new ANNdist[nK];

	const double* paraData = (const double*)(parameter);
	kdTree->annkSearch((double*)paraData,nK,nnIdx,nnDists);

	float weightSum = 0.f;
	for (int i=0;i<nK;i++)
	{
		int index = nnIdx[i];				
		KNN[i] = &poseData[index];
		dists[i] = nnDists[i];
	}
	//computeWeightFromDists(distList,KNNweights);

	// clean up memory
	delete [] nnIdx;
	delete [] dists;
}

void PoseExampleSet::buildKDTree()
{
	FREE_DATA(kdTree);
	if (dataPts)
	{
		annDeallocPts(dataPts);
	}
	
	// build KD-tree for KNN search
	int nPts = poseData.size(); // actual number of data points
	int dim = 3;	
	dataPts = annAllocPts(nPts, dim); // allocate data points	
	for (int i=0;i<nPts;i++)
	{
		memcpy(dataPts[i],(const double*)(poseData[i].poseParameter),sizeof(ANNcoord)*dim);
	}	
	kdTree = new ANNkd_tree( // build search structure
		dataPts, // the data points
		nPts, // number of points
		dim); // dimension of space	 
}

void PoseExampleSet::clearData()
{
	FREE_DATA(kdTree);
	if (dataPts)
	{
		annDeallocPts(dataPts);
	}
	poseData.clear();
}

SrBox PoseExampleSet::computeBBox()
{
	SrBox BBox;
	for (int i=0;i<poseData.size();i++)
	{
		PoseExample& ex = poseData[i];
		SrVec pt = SrVec(ex.poseParameter[0],ex.poseParameter[1],ex.poseParameter[2]);
		BBox.extend(pt);
	}
	return BBox;
}

/************************************************************************/
/* Exampled-Based IK Solver                                             */
/************************************************************************/

MeCtDataDrivenReach::MeCtDataDrivenReach(SkSkeleton* sk) : MeCtReach(sk)
{
	useDataDriven = true;
	useIK = true;
}

MeCtDataDrivenReach::~MeCtDataDrivenReach(void)
{
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

	_skeleton->update_global_matrices();
	SrVec targetPos = get_reach_target();
// 	SrVec curPos    = getWorldPos(limb.getChainEndEffector());
// 
// 	const float reachSpeed = 0.1f;
// 	SrVec targetDir = (targetPos - curPos); targetDir.normalize();
	SrVec nextTarget= targetPos;//curPos + targetDir*reachSpeed;

	float fRatio = 1.0f;
	// save a copy of initial joint quat angles before any modification
	SrArray<SrQuat> cur_quat_list = limb.joint_quat;
	
	if (useDataDriven)
	{
		
		const SrMat& rootMat = _skeleton->root()->gmat();
		//SrMat rootMat;
		//rootMat.translation(getRootJoint()->gmat().get(12),getRootJoint()->gmat().get(13),getRootJoint()->gmat().get(14));
		SrMat rootInv = rootMat.inverse();
		SrVec targetPos = nextTarget*rootInv;//get_reach_target()*rootInv; // get target position to find KNN
		// find KNN and corresponding distance to determine the blending weights
		SrArray<double> targetParameter;
		targetParameter.size(3);

		int nK = 6; // nearby examples should be (parameter dim + 1)
		SrArray<float> KNNWeight;
		SrArray<float> KNNDist;
		SrArray<PoseExample*> KNNPoses;

		for (int i=0;i<3;i++)
			targetParameter[i] = targetPos[i];

		//findKNNPoseExamples(targetParameter,KNNWeight,KNNPoses,nK);
		/*
		resampledPosedata.kdTreeKNN(targetParameter,KNNDist,KNNPoses,nK);
		computeWeightFromDists(KNNDist,KNNWeight);
		SrArray<SrQuat> blendQuat;
		blendPose(blendQuat,KNNWeight,KNNPoses);

		// damped blending		
		for (int i=0;i<blendQuat.size();i++)
		{
			MeCtIKScenarioJointInfo* info = &(limb.ik.joint_info_list.get(i));
			float damping_angle = (float)RAD(info->angular_speed_limit*dt);
			limb.joint_quat[i] = MeCtReachIK::dampQuat(limb.joint_quat[i],blendQuat[i],damping_angle);
// 			SrQuat diff = limb.joint_quat[i].inverse()*blendQuat[i];	
// 			diff.normalize();
// 			float angle = diff.angle() > damping_angle ? damping_angle : diff.angle();
// 			diff.set(diff.axis(),angle);
// 			limb.joint_quat[i] = limb.joint_quat[i]*diff;
// 			limb.joint_quat[i].normalize();					   
		}		
		*/
	}

	// solve CCD IK
	if (useIK)
	{
		MeCtIKScenario* ik_scenario = &limb.ik;

		ik_scenario->ik_offset = nextTarget; // set the target
		ik_scenario->ik_quat_orientation = SrQuat(0,0,0,1.0); // set to default rotation for testing
		ik_scenario->joint_quat_list = limb.joint_quat;	

		limb.skeleton->update_global_matrices();
		SkJoint* chain_root = limb.getChainRoot();
		ik_scenario->gmat = chain_root->parent()->gmat();

		ik.setDt(dt);
		ik.update(ik_scenario);
		//limb.joint_quat = ik_scenario->joint_quat_list;
		for (int i=0;i<ik_scenario->joint_quat_list.size();i++)
		{
			MeCtIKScenarioJointInfo* info = &(limb.ik.joint_info_list.get(i));
			float damping_angle = (float)RAD(info->angular_speed_limit*dt);
			limb.joint_quat[i] = MeCtReachIK::dampQuat(cur_quat_list[i],ik_scenario->joint_quat_list[i],damping_angle);
		}
	}
	// write results from limb to buffer
	limb.updateQuat(frame,false);
	return true;
}

void MeCtDataDrivenReach::updateExamplesFromMotions( MotionDataSet& inMotionSet, bool rebuild /*= false*/, float minDist /*= 5.0*/ )
{
	if (rebuild) 
	{		
		motionData.clear();
		examplePoseData.clearData();
	}

	BOOST_FOREACH(SkMotion* motion, inMotionSet)
	{
		if (motionData.find(motion) != motionData.end())
			continue; // we do not process example motions that are already used for this controller instance
		
		motion->connect(_skeleton);
		for (int j=0;j<motion->frames();j++)
		{
			PoseExample poseEx;			
			limb.updateMotionFrameToJointChain(motion,j);
			_skeleton->update_global_matrices();				
			getPoseExampleFromSkeleton(poseEx);	
			examplePoseData.addPose(poseEx,minDist);
			//resamplePosedata.addPose(po)
		}
		motion->disconnect();		
		motionData.insert(motion);
	}	
	examplePoseData.buildKDTree();
}

void MeCtDataDrivenReach::buildResamplePoseData( int nExamples, float fMinDist /*= 1.0*/ )
{
	// infer the bounding box from resample pose data
	SrBox BBox = examplePoseData.computeBBox();

	int nCount = 0;
	int nSamples = nExamples;
	int nKNN = 6;
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

		// find nearby samples for blending
		SrArray<PoseExample*> KNN;
		SrArray<float> dists, weights;			
		examplePoseData.linearKNN(dummy.poseParameter,dists,KNN,nKNN);		
		generateRandomWeight(nKNN,weights);
		blendPose(blendQuats,weights,KNN);

		limb.updateQuatToJointChain(blendQuats);
		_skeleton->update_global_matrices();		
		getPoseParameter(dummy.poseParameter,_skeleton);		
		dummy.jointQuat = blendQuats;

		// naive adding samples
		resampledPosedata.addPose(dummy,fMinDist);
		nCount++;
	}	

	resampledPosedata.buildKDTree();	
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


SrVec MeCtDataDrivenReach::getWorldPos( SkJoint* joint )
{
	const SrMat& endMat = joint->gmat();
	return SrVec(endMat.get(12), endMat.get(13), endMat.get(14));
}


void MeCtDataDrivenReach::getPoseParameter( SrArray<double>& para, SkSkeleton* skeleton )
{	
	const SrMat& rootMat = _skeleton->root()->gmat();//limb.getChainRoot()->gmat();//_skeleton->root()->gmat();
	SrMat rootInv = rootMat.inverse();
	//rootInv.translation(rootMat.get(12),rootMat.get(13),rootMat.get(14));
	//rootInv.invert();
	SrVec endGlobal = getWorldPos(limb.getChainEndEffector());
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