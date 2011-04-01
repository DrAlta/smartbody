#include <map>
#include <boost/foreach.hpp>
#include "me_ct_barycentric_interpolation.h"
#include <external/tetgen/tetgen.h>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#define FREE_DATA(data) if (data) delete data; data=NULL;

/************************************************************************/
/* Simplex class                                                        */
/************************************************************************/

Simplex& Simplex::operator=( const Simplex& rhs )
{
	numDim = rhs.numDim;
	vertexIndices = rhs.vertexIndices;
	return *this;	
}

/************************************************************************/
/* Barycentric Interpolator                                             */
/************************************************************************/
BarycentricInterpolator::BarycentricInterpolator()
{
	dataDim = DATA_3D; // only use 3D by default
}

BarycentricInterpolator::~BarycentricInterpolator()
{

}

bool BarycentricInterpolator::pointInsideSimplex( dVector& pt, Simplex& tet )
{	
	int numDim  = tet.numDim;
	int numElem = numDim + 1;
	assert(numElem == tet.vertexIndices.size());
	double prevDet = 0.0;
	dMatrix mat(numElem,numElem);
	dVector v(numElem);
	v[numDim] = 1.0;
	for (int i=0;i<numElem;i++)
	{
		int vtxIdx = tet.vertexIndices[i];
		InterpolationExample* ex = interpExamples[vtxIdx];		
		subrange(v,0,numDim) = ex->parameter;
		row(mat,i) = v;
	}
	prevDet = MeCtUBLAS::matrixDeterminant(mat);

	if (prevDet == 0.0)
		return false; // degenerate case

	dVector tempRow, ptRow;
	ptRow = v;
	subrange(ptRow,0,numDim) = pt;
	for (int i=0;i<numElem;i++)
	{
		double curDet;
		tempRow = row(mat,i);
		row(mat,i) = ptRow;
		curDet = MeCtUBLAS::matrixDeterminant(mat);
		if (MeCtMath::sgn(curDet) != MeCtMath::sgn(prevDet))
			return false;

		prevDet = curDet;
		row(mat,i) = tempRow;
	}
	return true;
}

bool BarycentricInterpolator::buildInterpolator()
{
	tetgenio ptIn, tetOut;
	// initialize input points
	ptIn.numberofpoints = interpExamples.size();
	ptIn.pointlist = new REAL[interpExamples.size()*3];
	for (unsigned int i=0;i<interpExamples.size();i++)
	{
		InterpolationExample* pt = interpExamples[i];
		assert(pt->parameter.size() >= (unsigned int)dataDim);		
		SrVec posIn = SrVec();
		// since tetgen always assume the input to be a 3D point sets, we handle the case when dataDim < 3
		for (int k=0;k<dataDim;k++)
			posIn[k] = (float)pt->parameter[k];
		for (int k=0;k<3;k++)
			ptIn.pointlist[i*3+k] = posIn[k];		
	}
	tetrahedralize("V",&ptIn,&tetOut);

	for (int i=0;i<tetOut.numberoftetrahedra;i++)
	{
		Simplex tet;
		tet.numDim = dataDim;
		for (int k=0;k<tetOut.numberofcorners;k++)
			tet.vertexIndices.push_back(tetOut.tetrahedronlist[i*tetOut.numberofcorners+k]);
		simplexList.push_back(tet);
	}

	//ptIn.deinitialize();	
	//tetOut.deinitialize();
	return true;	
}

void BarycentricInterpolator::predictInterpWeights( const dVector& para, VecOfInterpWeight& blendWeights )
{

}