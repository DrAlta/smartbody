#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "me_ct_motion_example.hpp"
#include "me_ct_motion_parameter.h"
#include "me_ct_ublas.hpp"

using namespace boost;

BodyMotionFrame& BodyMotionFrame::operator=( const BodyMotionFrame& rhs )
{
	jointQuat = rhs.jointQuat;
	rootPos   = rhs.rootPos;
	return *this;
}

void BodyMotionInterface::getMotionParameter( VecOfDouble& outPara )
{
	motionParameterFunc->getMotionParameter(this,outPara);
}


/************************************************************************/
/* BodyMotion                                                           */
/************************************************************************/
BodyMotion::BodyMotion()
{
	motion = NULL;
	timeWarp = NULL;
}

BodyMotion::~BodyMotion()
{
	if (timeWarp)
		delete timeWarp;
	timeWarp = NULL;
}

double BodyMotion::getRefDeltaTime( float u, float dt )
{
	return dt/timeWarp->timeSlope(u);	
}

double BodyMotion::motionPercent( float time )
{
	double rt = timeWarp->timeWarp(time);
	return timeWarp->invTimeWarp(rt)/timeWarp->refTimeLength();
}

double BodyMotion::getMotionFrame( float time, SkSkeleton* skel, const vector<SkJoint*>& affectedJoints, BodyMotionFrame& outMotionFrame )
{
	// Because the SkMotion stored its joint quats in an indirect way, it is not straightforward to grab corresponding quats we need.
	// This is a hack to apply the motion on a skeleton, and then get the quat values directly.

	motion->connect(skel);	
	double rt = timeWarp->timeWarp(time);
	motion->apply((float)rt);	
	motion->disconnect();

	if (outMotionFrame.jointQuat.size() != affectedJoints.size())
		outMotionFrame.jointQuat.resize(affectedJoints.size());

	for (size_t i=0;i<affectedJoints.size();i++)
		outMotionFrame.jointQuat[i] = affectedJoints[i]->quat()->value();
	
	outMotionFrame.rootPos.set(skel->root()->child(0)->pos()->value());
	return timeWarp->invTimeWarp(rt);
}

double BodyMotion::motionDuration(DurationType durType)
{
	double motionTime = 0.0;
	if (durType == DURATION_REF)
		motionTime = timeWarp->refTimeLength();
	else if (durType == DURATION_ACTUAL)
		motionTime = timeWarp->actualTimeLength();		

	return motionTime;
}

/************************************************************************/
/* Motion Example                                                       */
/************************************************************************/

void MotionExample::getExampleParameter( VecOfDouble& outPara )
{
	this->getMotionParameter(outPara);
}



/************************************************************************/
/* ResampleMotion                                                       */
/************************************************************************/
ResampleMotion::ResampleMotion( VecOfBodyMotionPtr* motionRef )
{
	motionDataRef = motionRef;
}


double ResampleMotion::getMotionFrame( float time, SkSkeleton* skel, const vector<SkJoint*>& affectedJoints, BodyMotionFrame& outMotionFrame )
{
	// use blended weights to get the motion frame
	return MotionExampleSet::blendMotionFunc(time,skel,affectedJoints,*motionDataRef,weight,outMotionFrame);	
}

double ResampleMotion::motionDuration(DurationType durType)
{
	double motionTime = 0.0;
	VecOfBodyMotionPtr& motions = *motionDataRef;
	for (size_t i=0;i<weight.size();i++)
	{
		int idx = weight[i].first;
		float w = weight[i].second;
		BodyMotionInterface* motion = motions[idx];
		motionTime += motion->motionDuration(durType)*w;
	}
	return motionTime;
}

void ResampleMotion::getExampleParameter( VecOfDouble& outPara )
{
	this->getMotionParameter(outPara);
}

double ResampleMotion::motionPercent( float time )
{
	double timePercent = 0.0;
	for (size_t i=0;i<weight.size();i++)
	{		
		float w = weight[i].second;		
		int idx = weight[i].first;
		BodyMotionInterface* motion = (*motionDataRef)[idx];
		timePercent += motion->motionPercent(time)*w;		
	}		
	return timePercent;
}

double ResampleMotion::getRefDeltaTime( float u, float dt )
{
	double du = 0.0;
	for (size_t i=0;i<weight.size();i++)
	{		
		float w = weight[i].second;		
		int idx = weight[i].first;
		BodyMotionInterface* motion = (*motionDataRef)[idx];
		du += motion->getRefDeltaTime(u,dt)*w;	
	}		
	return du;
	
}

/************************************************************************/
/* Grid Box                                                             */
/************************************************************************/
ParameterBoundingBox::ParameterBoundingBox( int nD )
{
	initBBox(nD);	
}

ParameterBoundingBox::ParameterBoundingBox()
{
	isInit = false;
}

ParameterBoundingBox::ParameterBoundingBox( ParameterBoundingBox& bBox )
{
	numDim = bBox.numDim;
	minParameter = bBox.minParameter;
	maxParameter = bBox.maxParameter;
	isInit = true;
}


void ParameterBoundingBox::initBBox( int nD )
{
	numDim = nD;
	minParameter.resize(numDim);
	maxParameter.resize(numDim);
	for (int i=0;i<nD;i++)
	{
		// initialize to invalid box
		minParameter[i] = 1e30;
		maxParameter[i] = -1e30;
	}

	isInit = true;	
}

void ParameterBoundingBox::extendBBox( const VecOfDouble& inPara )
{
	// check for dimension
	if (!isInit)
		initBBox(inPara.size());

	if (inPara.size() != numDim)
		return;

	for (int i=0;i<numDim;i++)
	{
		SR_UPDMIN ( minParameter[i], inPara[i] ); 
		SR_UPDMAX ( maxParameter[i], inPara[i] );	
	}	
}

void ParameterBoundingBox::randomPointInBox( VecOfDouble& outPara )
{
	outPara.resize(numDim);
	for (int i=0;i<numDim;i++)
	{
		double coord = minParameter[i] + (maxParameter[i] - minParameter[i])*MeCtMath::Random(0.f,1.f);
		outPara[i] = coord;
	}
}

int ParameterBoundingBox::gridHashing( const VecOfDouble& inPara, double cellSize, VecOfInt& adjHash )
{
	int nHash = 0;

	VecOfDouble localPara(numDim);
	VecOfDouble gridSize(numDim);
	VecOfInt    intPara(numDim);

	for (int i=0;i<numDim;i++)
	{
		localPara[i] = inPara[i] - minParameter[i];
		gridSize[i] = floor((maxParameter[i]-minParameter[i])/cellSize) + 1;
		intPara[i] = static_cast<int>(localPara[i]/cellSize);
		nHash *= (int)gridSize[i];
		nHash += intPara[i];		
	}

	int nAdjCells = static_cast<int>(pow(3.0,numDim));
	adjHash.resize(nAdjCells);
	adjHash.assign(nAdjCells,0);
	for (int k=0;k<nAdjCells;k++)
	{		
		int powNum = static_cast<int>(pow(3.0,numDim-1));
		int idx = k;
		for (int l=0;l<numDim;l++)
		{
			int tempPara = intPara[l];
			int bit = idx/powNum -1;

			adjHash[k] *= (int)gridSize[l];			
			tempPara += bit;
			idx = idx%powNum;
			powNum /= 3;				
			adjHash[k] += tempPara;
		}		
	}	
	return nHash;	
}

void ParameterBoundingBox::scaleBBox( double scaleRatio )
{
	double extendRatio = scaleRatio-1.0;
	for (int i=0;i<numDim;i++)
	{
		double length = maxParameter[i] - minParameter[i];
		minParameter[i] -= length*0.5*extendRatio;
		maxParameter[i] += length*0.5*extendRatio;
	}
}

/************************************************************************/
/* MotionExampleSet                                                     */
/************************************************************************/
void ExampleSet::addExample( InterpolationExample* ex )
{
	// initialize the parameter size
	if (paraSize == 0)
		paraSize = ex->parameter.size();

	interpExamples.push_back(ex);
	parameterBBox.extendBBox(ex->parameter);
}

bool MotionExampleSet::addMotionExample(MotionExample* ex )
{
	motionData.push_back(ex);
	motionExamples.push_back(ex);
	addExample(ex);
	return true;
}

double MotionExampleSet::blendMotion( float time, const VecOfInterpWeight& blendWeight, BodyMotionFrame& outMotionFrame )
{
	return MotionExampleSet::blendMotionFunc(time,skeletonRef,affectedJoints,motionData,blendWeight,outMotionFrame);
}

MotionExampleSet::~MotionExampleSet()
{
	for (size_t i=0;i<motionExamples.size();i++)
	{
		// feng : need to figure out why we can not delete the base pointer "BodyMotionInterface".
		MotionExample* ex = motionExamples[i];
		delete ex;
	}
	motionExamples.clear();
}


InterpolationExample* MotionExampleSet::createPseudoExample()
{
	ResampleMotion* ex = new ResampleMotion(&motionData);
	ex->motionParameterFunc = motionParameterFunc;
	return ex;
}

void MotionExampleSet::initMotionExampleSet(MotionParameter* parameterFunc)
{
	motionParameterFunc = parameterFunc;
	skeletonRef = parameterFunc->skeletonRef;
	affectedJoints = parameterFunc->affectedJoints;
}

double MotionExampleSet::blendMotionFunc( float time, SkSkeleton* skel, const vector<SkJoint*>& joints, const VecOfBodyMotionPtr& motions, 
									    const VecOfInterpWeight& blendWeight, BodyMotionFrame& outMotionFrame )
{
	int idx = blendWeight[0].first;
	double newTime = 0.0;
	float weightSum = blendWeight[0].second;
	BodyMotionInterface* ex = motions[idx];

	newTime = ex->getMotionFrame(time,skel,joints,outMotionFrame);			
	BodyMotionFrame tempFrame;
	float w1,w2;	
	for (size_t i=1;i<blendWeight.size();i++)
	{
		w1 = weightSum;
		w2 = blendWeight[i].second;
		weightSum += w2;
		idx = blendWeight[i].first;
		if (weightSum == 0.f)
			continue;
				
		float weight = w2/weightSum;
		float oneMinusWeight = 1.f - weight;

		newTime = motions[idx]->getMotionFrame(time,skel,joints,tempFrame)*weight + newTime*oneMinusWeight;

		for (size_t k=0;k<outMotionFrame.jointQuat.size();k++)
		{
			outMotionFrame.jointQuat[k] = slerp( outMotionFrame.jointQuat[k], tempFrame.jointQuat[k], weight );
			outMotionFrame.jointQuat[k].normalize();
		}		
		outMotionFrame.rootPos = outMotionFrame.rootPos*oneMinusWeight + tempFrame.rootPos*weight;
	}	
	return newTime;
}

