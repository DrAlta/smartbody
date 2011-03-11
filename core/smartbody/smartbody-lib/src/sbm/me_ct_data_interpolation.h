#pragma once
#include <map>
#include "me_ct_motion_example.hpp"
#include "me_ct_ublas.hpp"

class DataInterpolator
{
protected:
	VecOfInterpExample interpExamples;	
	ExampleSet* exampleSet;
public:
	DataInterpolator() {}
	~DataInterpolator() {}
	virtual void init(ExampleSet* exSet);
	virtual bool buildInterpolator() = 0;
	virtual void predictInterpWeights(const VecOfDouble& para, VecOfInterpWeight& blendWeights) = 0;
	VecOfInterpExample* getInterpExamples() { return &interpExamples; }
};

// Motion interpolation based on radial-basis function (RBF). 
// A set of motions are first initialized and used for computing the RBF weights.
// At run-time, given an input parameter, corresponding interpolation weights are determined.
// An interpolated motion/pose is generated 
class RBFInterpolator : public DataInterpolator
{
protected:
	dMatrix rbfMatrix, linearMatrix; // prediction matrix for RBF regression
public:
	RBFInterpolator();
	~RBFInterpolator();

	virtual bool buildInterpolator();
	virtual void predictInterpWeights(const VecOfDouble& para, VecOfInterpWeight& blendWeights);

protected:
	double RBFValue(const VecOfDouble& p1, const VecOfDouble& p2);
};

// Motion interpolation based on K-nearest neighbor
class KNNInterpolator : public DataInterpolator
{   
protected:
	ANNpointArray dataPts; // data points	
	ANNkd_tree* kdTree;    // search structure	
	
	float minDist;	
	int resampleSize, numKNN; // parameters for resampling		
public: // temp set it to public for debug
	VecOfInterpExample resampleData;
	VecOfInterpExample finalExampleData;
	// a grid based hash map to accelerate KNN look up
	ParameterBoundingBox gridBox;
	std::map<int,VecOfInterpExample> exampleHash; 
public:
	KNNInterpolator(int numResample = 500, float sampleDist = 5.f);
	~KNNInterpolator();		

	virtual bool buildInterpolator();
	virtual void predictInterpWeights(const VecOfDouble& para, VecOfInterpWeight& blendWeights);	
protected:
	static void generateRandomWeight(int nK, vector<float>& outWeights);
	static void generateDistWeights(vector<float>& dists, vector<float>& outWeights);	
	bool addResample(InterpolationExample* ex);

	// both KNN methods return the indices of KNN samples ( in InterpWeight.first ), and store distances in the InterpWeight.second
	// further processing is needed to infer the blending weights from distances
	int  closestExampleInHash(const vector<double>& inPara, int nKNN, VecOfInterpWeight& outWeight);
	int  linearKNN(const VecOfInterpExample& sampleList, const vector<double>& inPara, 
		           int nKNN, VecOfInterpWeight& outWeight);
	int  kdTreeKNN(ANNkd_tree* kdTree, const vector<double>& inPara,
		           int nKNN, VecOfInterpWeight& outWeight);
};