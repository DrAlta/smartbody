#pragma once
#include "me_ct_reach.hpp"
#include "sr/sr_box.h"
#include <external/ANN/ANN.h>

class PoseExample
{
public:
	SrArray<SrQuat> jointQuat; // joint configuration
	SrArray<double> poseParameter; // parameter for searching KNN, use end effector position by default
public:
	PoseExample() {};
	~PoseExample() {};
	PoseExample& operator=(const PoseExample& rhs);
};


typedef std::vector<PoseExample> VecOfPoseExample;

class PoseExampleSet
{
protected:	
	VecOfPoseExample poseData;		
	ANNpointArray dataPts; // data points	
	ANNkd_tree* kdTree; // search structure
public:
	PoseExampleSet();
	~PoseExampleSet();	

	const VecOfPoseExample& PoseData() const { return poseData; }

	void buildKDTree();
	void kdTreeKNN(const SrArray<double>& parameter, SrArray<float>& dists, SrArray<PoseExample*>& KNN, int nK);
	int linearKNN(const SrArray<double>& parameter, SrArray<float>& dists, SrArray<PoseExample*>& KNN, int nK);	

	bool addPose(const PoseExample& poseEx, float fMinDist = -1.f);
	SrBox computeBBox();
	void clearData();
};

// MeCtDataDrivenReach improves the IK result by interpolating the example motions 
// Based on end-effector position ( or any other designated pose parameters ), a pose is predicted from examples
// The predicted pose is then used to guide the traditional IK ( Use as starting guess for CCD, or as the reference pose for other IK solver. )

class MeCtDataDrivenReach :
	public MeCtReach
{
protected:
	MotionDataSet motionData; // training motions to improve IK results
	PoseExampleSet examplePoseData; // extracted example poses from training motions			
	PoseExampleSet resampledPosedata;		
public:
	// a hack for debugging only
	bool useDataDriven;
	bool useIK;
public:
	MeCtDataDrivenReach(SkSkeleton* sk);
	~MeCtDataDrivenReach(void);
	const PoseExampleSet& ResampledPosedata() const { return resampledPosedata; }
	const PoseExampleSet& ExamplePoseData() const { return examplePoseData; }	

	void updateExamplesFromMotions(MotionDataSet& inMotionSet, bool rebuild = false, float minDist = 5.0);	
	void buildResamplePoseData(int nExamples, float fMinDist = 1.0);
	virtual bool controller_evaluate( double t, MeFrameData& frame );
private:
	void getPoseParameter(SrArray<double>& para, SkSkeleton* skeleton);
	void getPoseExampleFromSkeleton(PoseExample& pose);		
	
	static void blendPose(SrArray<SrQuat>& blendPoses, SrArray<float>& KNNweights, SrArray<PoseExample*>& KNNPoses);		
	static void computeWeightFromDists(SrArray<float>& dists, SrArray<float>& outWeights);
	static void generateRandomWeight(int nK, SrArray<float>& outWeights);
	static float Random(float r_min, float r_max);
	static SrVec randomPointInBox(SrBox& box);	
	static SrVec getWorldPos(SkJoint* joint);
};

template <class T> 
void Plus(const SrArray<T>& A, const SrArray<T>& B, SrArray<T>& Out, double ratio)
{
	assert(A.size() == B.size());
	Out.size(A.size());
	for (int i=0;i<A.size();i++)
	{
		Out[i] = A[i] + B[i]*ratio;		
	}
}

template <class T>
T Norm2(SrArray<T>& Out)
{
	T norm = 0.0;
	for (int i=0;i<Out.size();i++)
		norm += Out[i]*Out[i];
	return norm;
};

template <class T>
T Dist(const SrArray<T>& A, const SrArray<T>& B)
{
	SrArray<T> diff;
	Plus(A,B,diff,-1.0);
	return Norm2(diff);
}






