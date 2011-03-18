#include <map>
#include <boost/foreach.hpp>
#include "me_ct_data_interpolation.h"


#define FREE_DATA(data) if (data) delete data; data=NULL;

void DataInterpolator::init( ExampleSet* exSet )
{
	interpExamples = exSet->getExamples();
	exampleSet     = exSet;
}

/************************************************************************/
/* RBF Interpolator                                                     */
/************************************************************************/

RBFInterpolator::RBFInterpolator()
{

}

RBFInterpolator::~RBFInterpolator()
{

}

bool RBFInterpolator::buildInterpolator()
{	
	if (interpExamples.empty())
		return false;

	InterpolationExample* ex = interpExamples.front();
	int nD = ex->parameter.size();
	int nK = interpExamples.size();

	// build linear approximation
	linearMatrix.resize(nK,nD+1); // nD+1 for the constant term
	dMatrix A,B, Sol;
	B = ublas::identity_matrix<double>(nK,nK);
	A.resize(nK,nD+1);
	for (int i=0;i<nK;i++)
	{
		InterpolationExample* ex = interpExamples[i];
		for (int d=0;d<nD;d++)
		{
			A(i,d) = ex->parameter[d];
		}
		A(i,nD) = 1.0;
	}
	MeCtUBLAS::linearLeastSquare(A,B,Sol);
	linearMatrix = ublas::trans(Sol);

	dMatrix matLin;
	MeCtUBLAS::matrixMatMult(A,Sol,matLin);
	dMatrix matResidue = ublas::identity_matrix<double>(nK,nK);
	matResidue -= matLin;	


	dMatrix ARbf;
	ARbf.resize(nK,nK);
	for (int i=0;i<nK;i++)
	{
		for (int j=0;j<nK;j++)
		{
			ARbf(i,j) = RBFValue(interpExamples[i]->parameter,interpExamples[j]->parameter);
		}
	}
	dMatrix ARbfInv;
	MeCtUBLAS::inverseMatrix(ARbf,ARbfInv);
	MeCtUBLAS::matrixMatMult(ARbfInv,matResidue,rbfMatrix);
	rbfMatrix = ublas::trans(rbfMatrix);
	//MeCtUBLAS::linearLeastSquare(ARbf,matResidue,rbfMatrix);
	return true;
}

double RBFInterpolator::RBFValue(const VecOfDouble& p1,const VecOfDouble& p2 )
{
	VecOfDouble diff;
	Plus(p1,p2,diff,-1.0);
	double r = sqrtf((float)Norm2(diff));
	//return exp(-(r*r)*0.0005);
	if (r==0.0)
		return r;
	else
		return r*r*log(r);
}

void RBFInterpolator::predictInterpWeights( const VecOfDouble& para, VecOfInterpWeight& blendWeights )
{
	dVector rbfIn, linearIn;
	InterpolationExample* ex = interpExamples.front();
	int nK = interpExamples.size();	
	int nD = ex->parameter.size();
	rbfIn.resize(nK);
	linearIn.resize(nD+1);
	for (int i=0;i<nD;i++)
		linearIn(i) = para[i];
	linearIn(nD) = 1.0;

	for (int i=0;i<nK;i++)
	{
		rbfIn(i) = RBFValue(para,interpExamples[i]->parameter);
	}

	dVector linearWeight, rbfWeight;
	MeCtUBLAS::matrixVecMult(linearMatrix,linearIn,linearWeight);
	MeCtUBLAS::matrixVecMult(rbfMatrix,rbfIn,rbfWeight);
	blendWeights.resize(nK);	

	float weightSum = 0.0;
	for (int i=0;i<nK;i++)
	{
		InterpWeight w;		
		float weight = (float)(linearWeight(i) + rbfWeight(i));
		// 		if (weight > 1.0) weight = 1.0;
		if (weight < 0.0) weight = 0.0;
		w.first = i;
		w.second = weight;
		blendWeights[i] = w;		
		weightSum += weight;
	}

	for (int i=0;i<nK;i++)
		blendWeights[i].second /= weightSum;
}

/************************************************************************/
/* KNN Interpolator                                                     */
/************************************************************************/
KNNInterpolator::KNNInterpolator( int numResample /*= 500*/, float sampleDist /*= 5.f*/ )
{
	minDist = sampleDist;
	resampleSize = numResample;
	numKNN  = 8;
	kdTree = NULL;
	dataPts = NULL;
}

KNNInterpolator::~KNNInterpolator()
{

}

bool KNNInterpolator::buildInterpolator()
{	
	// add "real" example data 
// 	for (int i=0;i<interpExamples.size();i++)
// 	{
// 		resampleData.push_back(interpExamples[i]);			
// 	}

	// build resample
	int nCount = 0;
	ParameterBoundingBox& BBox = exampleSet->getParameterBBox();
	gridBox = ParameterBoundingBox(BBox);
	gridBox.scaleBBox(1.5);
	InterpolationExample* ex = NULL;
	while (nCount < resampleSize)
	{		
		ex = exampleSet->createPseudoExample();
		BBox.randomPointInBox(ex->parameter);

 		// find nearby samples for blending
 		vector<InterpolationExample*> KNN;
 		vector<float> weights;
		linearKNN(exampleSet->getExamples(),ex->parameter,numKNN,ex->weight); 		
 		generateRandomWeight(ex->weight.size(),weights);
		// copy the weights to VecOfInterpWeight
		for (size_t i=0;i<ex->weight.size();i++)
			ex->weight[i].second = weights[i];

		ex->getExampleParameter(ex->parameter);
		bool exampleAdded = addResample(ex);		
		if (!exampleAdded) // delete the example if it's not necessary
			delete ex;

		nCount++;
	}	

	// put together example data & resample data
	finalExampleData = resampleData;
	for (size_t i=0;i<interpExamples.size();i++)	
		finalExampleData.push_back(interpExamples[i]);

	// build Kd-Tree
	FREE_DATA(kdTree);
	if (dataPts)
	{
		annDeallocPts(dataPts);
	}

	// build KD-tree for KNN search
	int nPts = finalExampleData.size(); // actual number of data points
	int dim = exampleSet->parameterDim();
	dataPts = annAllocPts(nPts, dim); // allocate data points	
	for (int i=0;i<nPts;i++)
	{
		memcpy(dataPts[i],(const double*)(&finalExampleData[i]->parameter[0]),sizeof(ANNcoord)*dim);
	}	
	kdTree = new ANNkd_tree( // build search structure
		dataPts, // the data points
		nPts, // number of points
		dim); // dimension of space	 

	

	return true;
}

void KNNInterpolator::predictInterpWeights( const VecOfDouble& para, VecOfInterpWeight& blendWeights )
{
	vector<int> KNNIdx;
	vector<float> KNNDists, KNNWeights;
	VecOfInterpWeight tempWeight;
	
	kdTreeKNN(kdTree,para,numKNN,tempWeight);
	pairToVec(tempWeight,KNNIdx,KNNDists);
	generateDistWeights(KNNDists,KNNWeights);
	vecToPair(KNNIdx,KNNWeights,tempWeight);	

	// map pseudo examples to final weight	
	std::map<int,float> finalWeight;
	
	float weightSum = 0.f;
	for (size_t i=0;i<tempWeight.size();i++)
	{
		InterpWeight& tempW = tempWeight[i];
		InterpolationExample* ex = finalExampleData[tempW.first];
		VecOfInterpWeight& realW = ex->weight;
		for (size_t k=0;k<realW.size();k++)
		{
			int index = realW[k].first;
			float w   = realW[k].second*tempW.second;
			if (finalWeight.find(index) == finalWeight.end())
				finalWeight[index] = 0.f;
			finalWeight[index] += w;
			weightSum += w;
		}
	}	

	blendWeights.clear();
	std::map<int,float>::iterator mi;
	//BOOST_FOREACH(const map_type::value_type&& w, finalWeight)
	for ( mi  = finalWeight.begin(); 
		  mi != finalWeight.end();
		  mi++)
	{
		if (mi->second == 0.f)
			continue;

		InterpWeight w = *mi;
		w.second /= weightSum;
		blendWeights.push_back(w);
	}
}

bool KNNInterpolator::addResample( InterpolationExample* ex )
{	
	VecOfInterpWeight outDist;
	if (minDist <= 0.f) // just add the pose
	{
		resampleData.push_back(ex);	
		return true;
	}
	else
	{
		//int nK = linearKNN(resampleData,ex->parameter,1,outDist); // find closest pose data in parameter space
		int iHash = closestExampleInHash(ex->parameter,1,outDist);
		if (outDist.size() == 0 || outDist[0].second > minDist)
		{
			resampleData.push_back(ex);			
 			if (exampleHash.find(iHash) == exampleHash.end())
 				exampleHash[iHash] = VecOfInterpExample(); 
 			exampleHash[iHash].push_back(ex);
			return true;
		}
	}
	return false;
}

void KNNInterpolator::generateRandomWeight( int nK, vector<float>& outWeights )
{
	float delta = 0.02f;
	outWeights.resize(nK);
	float weightSum = 0.0;
	for (int i=0;i<nK-1;i++)
	{
		float w = MeCtMath::Random(std::max(-delta, -delta-weightSum),std::min(1.f+delta,1.f+delta-weightSum));
		outWeights[i] = w;
		weightSum += w;
	}	
	outWeights[nK-1] = 1.f - weightSum;
}

void KNNInterpolator::generateDistWeights( vector<float>& dists, vector<float>& outWeights )
{
	int nK = dists.size();
	outWeights.resize(nK);

	float weightSum = 0.f;
	for (int i=0;i<nK;i++)
	{
		float weight = 1.0f/dists[i] - 1.0f/dists[nK-1];
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
}

int KNNInterpolator::closestExampleInHash(const vector<double>& inPara, int nKNN, VecOfInterpWeight& outWeight)
{
	ParameterBoundingBox& bbox = gridBox;//exampleSet->getParameterBBox();
	VecOfInt adjHash;
	VecOfInterpExample exList;
	int iHash = bbox.gridHashing(inPara,minDist,adjHash);	
	//adjHash.push_back(iHash);
	bool bCorrectHash = false;
 	for (size_t i=0;i<adjHash.size();i++)
 	{
 		int idx = adjHash[i];
		if (iHash == idx)
			bCorrectHash = true;
 		if (exampleHash.find(idx) != exampleHash.end())
 			exList.insert(exList.end(),exampleHash[idx].begin(),exampleHash[idx].end());
 	}
	if (!bCorrectHash)
		printf("Incorrect hash number !\n");
	if (exList.size() >= (size_t)nKNN)
		linearKNN(exList,inPara,nKNN,outWeight);

	return iHash;
}


int KNNInterpolator::kdTreeKNN( ANNkd_tree* kdTree, const vector<double>& inPara, int nKNN, VecOfInterpWeight& outWeight )
{	
	outWeight.resize(nKNN);
	
	ANNidxArray nnIdx;
	ANNdistArray nnDists;
	nnIdx = new ANNidx[nKNN];
	nnDists = new ANNdist[nKNN];

	const double* paraData = (const double*)(&inPara[0]);
	kdTree->annkSearch((double*)paraData,nKNN,nnIdx,nnDists);

	float weightSum = 0.f;
	for (int i=0;i<nKNN;i++)
	{
		int index = nnIdx[i];				
		outWeight[i].first  = index;
		outWeight[i].second = float(nnDists[i]);
	}
	// clean up memory
	delete [] nnIdx;
	delete [] nnDists;
	
	return nKNN;
}

int KNNInterpolator::linearKNN( const VecOfInterpExample& sampleList, const vector<double>& inPara, int nK, 
								 VecOfInterpWeight& outWeight )
{	
	outWeight.resize(nK+1);	
	outWeight.assign(outWeight.size(),InterpWeight(-1,1e10));
	int nCurK = 0;
	for (size_t i=0;i<sampleList.size();i++)
	{
		InterpolationExample* curEx = sampleList[i];
		double dist = sqrt(Dist(curEx->parameter,inPara));
		int k;
		for (k=nCurK;k>0;k--)
		{
			if (outWeight[k-1].second > dist)
			{				
				outWeight[k] = outWeight[k-1];
			}
			else 
				break;
		}				
		outWeight[k].first  = i;
		outWeight[k].second = (float)dist;
		if (nCurK < nK)
			nCurK++;
	}
	outWeight.pop_back();	
	return nCurK; 	

}
