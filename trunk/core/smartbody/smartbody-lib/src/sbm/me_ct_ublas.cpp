//#include <boost/numeric/bindings/atlas/cblas.hpp>
#include "me_ct_ublas.hpp"
#include <boost/numeric/bindings/blas/blas.hpp>
#include <boost/numeric/bindings/lapack/lapack.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/ublas_vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/banded.hpp> 
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <time.h>

#pragma comment(lib,"blas.lib")
#pragma comment(lib,"libf2c.lib")
#pragma comment(lib,"lapack.lib")

namespace lapack = boost::numeric::bindings::lapack;
namespace blas   = boost::numeric::bindings::blas;
namespace ublas  = boost::numeric::ublas;

void MeCtUBLAS::matrixMatMult(const dMatrix& mat1, const dMatrix& mat2, dMatrix& mat3)
{
	if (mat3.size1() != mat1.size1() || mat3.size2() != mat2.size2())
		mat3.resize(mat1.size1(),mat2.size2());

	blas::gemm(mat1,mat2,mat3);	
}

void MeCtUBLAS::matrixVecMult(const dMatrix& mat1, const dVector& vin, dVector& vout)
{
	if (vout.size() != mat1.size1())
		vout.resize(mat1.size1());
	if (vin.size() != mat1.size2())
		return;

	blas::gemv('N',1.0,mat1,vin,0.0,vout);	
}

bool MeCtUBLAS::inverseMatrix( const dMatrix& mat, dMatrix& inv )
{
	using namespace boost::numeric::ublas;
	dMatrix A(mat);
	inv = identity_matrix<double>(mat.size1());
	lapack::gesv(A,inv);	
	return true;
}

bool MeCtUBLAS::linearLeastSquare( const dMatrix& A, const dMatrix& B, dMatrix& sol )
{
	dMatrix AtA, AtB, invAtA;
	MeCtUBLAS::matrixMatMult(ublas::trans(A),A,AtA);
	MeCtUBLAS::matrixMatMult(ublas::trans(A),B,AtB);
	MeCtUBLAS::inverseMatrix(AtA,invAtA);
	MeCtUBLAS::matrixMatMult(invAtA,AtB,sol);
	return true;
}

/************************************************************************/
/* MeCtMath Routines                                                    */
/************************************************************************/

float MeCtMath::Random( float r_min, float r_max )
{
	static bool initRand = false;
	if (!initRand)
	{
		srand(int(time(NULL)));
		initRand = true;
	}
	float frand = (float)rand()/(float)RAND_MAX; 
	frand = r_min + frand*(r_max-r_min);
	return frand;
}