#include <map>
#include <boost/foreach.hpp>
#include "controllers/me_ct_inverse_interpolation.h"


#define FREE_DATA(data) if (data) delete data; data=NULL;
/************************************************************************/
/* Inverse Interpolator                                             */
/************************************************************************/

InverseInterpolation::InverseInterpolation()
{
	numKNN  = 13;
}

InverseInterpolation::~InverseInterpolation()
{

}

bool InverseInterpolation::buildInterpolator()
{
	// we will use k-nearest neighbor as a starting point for optimization
	// so a KD-tree is still built for all examples.
	VecOfInterpExample& exampleData = exampleSet->getExamples();
	// build KD-tree for KNN search
	int nPts = exampleData.size(); // actual number of data points
	int dim = exampleSet->parameterDim();
	dataPts = annAllocPts(nPts, dim); // allocate data points	
	for (int i=0;i<nPts;i++)
	{
		memcpy(dataPts[i],(const double*)(&exampleData[i]->parameter[0]),sizeof(ANNcoord)*dim);
	}	
	kdTree = new ANNkd_tree( // build search structure
		dataPts, // the data points
		nPts, // number of points
		dim); // dimension of space	 

	prevWeight.clear();
	prevWeight.push_back(InterpWeight(0,1.f));

	return true;
}


void InverseInterpolation::predictInterpWeights( const dVector& para, VecOfInterpWeight& blendWeights )
{
	vector<int> KNNIdx;
	vector<float> KNNDists, KNNWeights;
	VecOfInterpWeight tempWeight;

	// calculate the initial weights
	kdTreeKNN(kdTree,para,numKNN,tempWeight);
	pairToVec(tempWeight,KNNIdx,KNNDists);
	generateDistWeights(KNNDists,KNNWeights);
	vecToPair(KNNIdx,KNNWeights,tempWeight);	
	mapDistWeightToBlendWeight(exampleSet->getExamples(),tempWeight,blendWeights);	

	//printf("rough blend weight : ");
// 	for (unsigned int i=0;i<blendWeights.size();i++)
// 	{
// 		printf("%d : %.4f ", blendWeights[i].first, blendWeights[i].second);
// 	}
	//printf("\n");
	float prevError = evaluateErrorFunction(para,prevWeight);
	float initError = evaluateErrorFunction(para,blendWeights);
	//printf("prev error = %f, init error = %f\n",prevError,initError);
	if (prevError < initError)
		blendWeights = prevWeight;
	int nIteration = optimizeBlendWeight(para,blendWeights);
	float finalError = evaluateErrorFunction(para,blendWeights);
	//printf("final blend weight : ");
// 	for (unsigned int i=0;i<blendWeights.size();i++)
// 	{
// 		printf("%d : %.2f ", blendWeights[i].first, blendWeights[i].second);
// 	}
// 	printf("\n");
 	printf("init error = %f, final error = %f, num iterations = %d\n",initError, finalError,nIteration);
	prevWeight = blendWeights;

}

int InverseInterpolation::optimizeBlendWeight( const dVector& para, VecOfInterpWeight& blendWeight )
{
	int counter = 0;
	int no_hit_counter = 0;
	int max_iter = 100; // maximum iterations before termination
	int add_iter = 20;  // additional iterations on steepest weight descent
	int total_iter = 0; // total iterations, iter + add_iter

	float step = 0.01f; // step for weight variation
	bool dir; // direction for weight variation, true:+  false:-

	float fmin_inner = 0.0f; // fmin_inner = min(f_p, f_n)
	float fmin = evaluateErrorFunction(para,blendWeight);// fmin = min(fmin_inner), init with ObjEval()
	float f_p = 0.0f; // f(x+step)
	float f_n = 0.0f; // f(x-step)
	int steepest_idx = 0;
	bool steepest_dir = false; // true:+  false:-
	
	VecOfInterpWeight tempWeight = blendWeight;
	while(counter<max_iter)
	{
		for(unsigned int i=0; i<blendWeight.size(); i++)
		{
			if(blendWeight[i].second + step < 1.0f) // limit weight within bound
			{
				tempWeight = blendWeight;
				tempWeight[i].second += step;
				//NormalizeWeightArray();
				normalizeBlendWeight(tempWeight);
				//f_p = ObjEval(type);
				f_p = evaluateErrorFunction(para,tempWeight);
			}
			else f_p = fmin;

			if(blendWeight[i].second - step > 0.0f)
			{
				tempWeight = blendWeight;
				tempWeight[i].second -= step;				
				normalizeBlendWeight(tempWeight);			
				f_n = evaluateErrorFunction(para,tempWeight);				
			}
			else f_n = fmin;

			if(f_p < f_n)
			{
				fmin_inner = f_p;
				dir = true;
			}
			else
			{
				fmin_inner = f_n;
				dir = false;
			}

			if(fmin_inner < fmin)
			{
				steepest_idx = i;
				steepest_dir = dir;
				fmin = fmin_inner;				
				if(dir) blendWeight[i].second += step;
				else blendWeight[i].second -= step;
				//NormalizeWeightArray(&temp_w[0], size);
				normalizeBlendWeight(blendWeight);
				no_hit_counter = 0;
				total_iter ++;
			}
			else no_hit_counter++;
			if(no_hit_counter > 10) goto terminate_iter;
		}
		counter++;

		// additional iterations on steepest weight
		for(int j =1; j<add_iter; j++)
		{
			if(steepest_dir && (blendWeight[steepest_idx].second+j*step < 1.0f))
			{				
				tempWeight = blendWeight;
				tempWeight[steepest_idx].second += step*j;				
			}
			else if(!steepest_dir && (blendWeight[steepest_idx].second-j*step > 0.0f))
			{
				tempWeight = blendWeight;
				tempWeight[steepest_idx].second -= step*j;						
			}
			else break;

			normalizeBlendWeight(tempWeight);
			fmin_inner = evaluateErrorFunction(para,tempWeight);

			if(fmin_inner < fmin)
			{
				fmin = fmin_inner;
				blendWeight = tempWeight;
				total_iter ++;
			}
			else
				break;
		}
	}
terminate_iter:
	//
	return total_iter ;
}

float InverseInterpolation::evaluateErrorFunction( const dVector& targetPara, VecOfInterpWeight& blendWeight )
{
	static InterpolationExample* ex = NULL;
	if (!ex) ex = exampleSet->createPseudoExample();
	dVector exPara;
	ex->weight = blendWeight;
	ex->getExampleParameter(exPara);	
	//exPara = exPara - targetPara;
	double error = 0.f;
	//printf("target para : ");
	for (unsigned int i=0;i<exPara.size();i++)
	{
		//printf("%f ", targetPara(i));
		double diff = exPara(i) - targetPara(i);
		error += diff*diff;
	}
	//printf("\n");
	return (float)sqrtf((float)error);
	//return (float)ublas::norm_2(exPara);
}