#pragma once
#include "me_ct_reach.hpp"
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

// MeCtDataDrivenReach improves the IK result by interpolating the example motions 
// Based on end-effector position ( or any other designated pose parameters ), a pose is predicted from examples
// The predicted pose is then used to guide the traditional IK ( Use as starting guess for CCD, or as the reference pose for other IK solver. )

class MeCtDataDrivenReach :
	public MeCtReach
{
protected:
	SrArray<SkMotion*> motionData; // training motions to improve IK results
	VecOfPoseExample examplePoseData; // extracted example poses from training motions		
	VecOfPoseExample resamplePosedata;
	// for KNN search
	ANNkd_tree* kdTree; // search structure
	ANNpointArray dataPts; // data points		
public:
	// a hack for debugging only
	static bool useDataDriven;
	static bool useIK;
public:
	MeCtDataDrivenReach(SkSkeleton* sk);
	~MeCtDataDrivenReach(void);

	void addMotion(SkMotion* motion);
	void buildPoseExamplesFromMotions(); 
	void buildResamplePoseData(float fMinDist = 1.0);

	const VecOfPoseExample& getExamplePoseData() const { return examplePoseData; }
	const VecOfPoseExample& getResamplePoseData() const { return resamplePosedata; }
	virtual bool controller_evaluate( double t, MeFrameData& frame );
private:
	void getPoseExampleFromSkeleton(PoseExample& pose);
	void findKNNPoseExamples(SrArray<double>& parameter, SrArray<float>& KNNweights, SrArray<PoseExample*>& KNNPoses, int nK);
	void blendPose(SrArray<SrQuat>& blendPoses, SrArray<float>& KNNweights, SrArray<PoseExample*>& KNNPoses);	

	// brute force method to look up nearby examples
	int  linearKNN(VecOfPoseExample& curList, PoseExample& newData, int nK, SrArray<float>& dists, SrArray<PoseExample*>& KNN);

	SrVec randomPointInBox(SrBox& box);
	void computeWeightFromDists(SrArray<float>& dists, SrArray<float>& outWeights);
	void generateRandomWeight(int nK, SrArray<float>& outWeights);
	float Random(float r_min, float r_max);
	void getPoseParameter(SrArray<double>& para, SkSkeleton* skeleton);
};

template <class T> 
void Plus(SrArray<T>& A, SrArray<T>& B, SrArray<T>& Out, double ratio)
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
T Dist(SrArray<T>& A, SrArray<T>& B)
{
	SrArray<T> diff;
	Plus(A,B,diff,-1.0);
	return Norm2(diff);
}






