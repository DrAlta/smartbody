#pragma once
#include "controllers/me_ct_data_interpolation.h"

class InverseInterpolation : public KNNBaseInterpolator
{
protected:
	// still need to build a KNN for existing examples
	ANNpointArray dataPts; // data points	
	ANNkd_tree* kdTree;    // search structure	
	VecOfInterpWeight prevWeight; // stored the previous weight	
public:
	InverseInterpolation();
	~InverseInterpolation();
public:
	virtual bool buildInterpolator();
	virtual void predictInterpWeights(const dVector& para, VecOfInterpWeight& blendWeights);
	virtual void drawInterpolator() {} // debugging information	
protected:
	int optimizeBlendWeight(const dVector& para, VecOfInterpWeight& blendWeight);
	float evaluateErrorFunction(const dVector& targetPara, VecOfInterpWeight& blendWeight);
};