//
//  Copyright (C) Toon Knapen 2003
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_NUMERIC_BINDINGS_BLAS_BLAS1_OVERLOADS_HPP
#define BOOST_NUMERIC_BINDINGS_BLAS_BLAS1_OVERLOADS_HPP

#include <boost/numeric/bindings/blas/blas.h>
#include <boost/numeric/bindings/traits/type.hpp>
#include <boost/numeric/bindings/traits/type_traits.hpp>

namespace boost { namespace numeric { namespace bindings { namespace blas { namespace detail {

  using namespace boost::numeric::bindings::traits ;

  // x *= alpha
  inline void scal(const integer_t& n, const float&     alpha, float*     x, const integer_t& incx) { BLAS_SSCAL( &n,              &alpha,                x  , &incx ) ; }
  inline void scal(const integer_t& n, const double&    alpha, double*    x, const integer_t& incx) { BLAS_DSCAL( &n,              &alpha,                x  , &incx ) ; }
  inline void scal(const integer_t& n, const complex_f& alpha, complex_f* x, const integer_t& incx) { BLAS_CSCAL( &n, complex_ptr( &alpha ), complex_ptr( x ), &incx ) ; }
  inline void scal(const integer_t& n, const complex_d& alpha, complex_d* x, const integer_t& incx) { BLAS_ZSCAL( &n, complex_ptr( &alpha ), complex_ptr( x ), &incx ) ; }

  // y += alpha * x
  inline void axpy(const integer_t& n, const float    & alpha, const float    * x, const integer_t& incx, float    * y, const integer_t& incy) { BLAS_SAXPY( &n,            &alpha  ,            x  , &incx,            y  , &incy ) ; }
  inline void axpy(const integer_t& n, const double   & alpha, const double   * x, const integer_t& incx, double   * y, const integer_t& incy) { BLAS_DAXPY( &n,            &alpha  ,            x  , &incx,            y  , &incy ) ; }
  inline void axpy(const integer_t& n, const complex_f& alpha, const complex_f* x, const integer_t& incx, complex_f* y, const integer_t& incy) { BLAS_CAXPY( &n, complex_ptr( &alpha ), complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }
  inline void axpy(const integer_t& n, const complex_d& alpha, const complex_d* x, const integer_t& incx, complex_d* y, const integer_t& incy) { BLAS_ZAXPY( &n, complex_ptr( &alpha ), complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }

  // x^T . y
  inline float  dot(const integer_t& n, const float * x, const integer_t& incx, const float * y, const integer_t& incy) { return (float)BLAS_SDOT( &n, x, &incx, y, &incy ) ; }
  inline double dot(const integer_t& n, const double* x, const integer_t& incx, const double* y, const integer_t& incy) { return BLAS_DDOT( &n, x, &incx, y, &incy ) ; }

  // x^T . y
#ifndef BIND_FORTRAN_F2C_RETURN_CONVENTIONS
  inline void dotu(complex_f& ret, const integer_t& n, const complex_f* x, const integer_t& incx, const complex_f* y, const integer_t& incy) { ret = complex_ret( BLAS_CDOTU( &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ) ; }
#else
  inline void dotu(complex_f& ret, const integer_t& n, const complex_f* x, const integer_t& incx, const complex_f* y, const integer_t& incy) { BLAS_CDOTU( complex_ptr( &ret ), &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }
#endif
#ifndef BIND_FORTRAN_F2C_RETURN_CONVENTIONS
  inline void dotu(complex_d& ret, const integer_t& n, const complex_d* x, const integer_t& incx, const complex_d* y, const integer_t& incy) { ret = complex_ret( BLAS_ZDOTU( &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ) ; }
#else
  inline void dotu(complex_d& ret, const integer_t& n, const complex_d* x, const integer_t& incx, const complex_d* y, const integer_t& incy) { BLAS_ZDOTU( complex_ptr( &ret ), &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }
#endif

  // x^H . y
#ifndef BIND_FORTRAN_F2C_RETURN_CONVENTIONS
  inline void dotc(complex_f& ret, const integer_t& n, const complex_f* x, const integer_t& incx, const complex_f* y, const integer_t& incy) { ret = complex_ret( BLAS_CDOTC( &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ) ; }
#else
  inline void dotc(complex_f& ret, const integer_t& n, const complex_f* x, const integer_t& incx, const complex_f* y, const integer_t& incy) { BLAS_CDOTC( complex_ptr( &ret ), &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }
#endif
#ifndef BIND_FORTRAN_F2C_RETURN_CONVENTIONS
  inline void dotc(complex_d& ret, const integer_t& n, const complex_d* x, const integer_t& incx, const complex_d* y, const integer_t& incy) { ret = complex_ret( BLAS_ZDOTC( &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ) ; }
#else
  inline void dotc(complex_d& ret, const integer_t& n, const complex_d* x, const integer_t& incx, const complex_d* y, const integer_t& incy) { BLAS_ZDOTC( complex_ptr( &ret ), &n, complex_ptr( x ), &incx, complex_ptr( y ), &incy ) ; }
#endif

  // euclidean norm
  inline float  nrm2(const integer_t& n, const float*   x, const integer_t& incx) { return (float)BLAS_SNRM2( &n, x, &incx ) ; }
  inline double nrm2(const integer_t& n, const double*  x, const integer_t& incx) { return BLAS_DNRM2( &n, x, &incx ) ; }
  inline float  nrm2(const integer_t& n, const complex_f*   x, const integer_t& incx) { return (float)BLAS_SCNRM2( &n, complex_ptr(x), &incx ) ; }
  inline double nrm2(const integer_t& n, const complex_d*  x, const integer_t& incx) { return BLAS_DZNRM2( &n, complex_ptr(x), &incx ) ; }

  // 1-norm
  inline float  asum(const integer_t& n, const float*   x, const integer_t& incx) { return (float)BLAS_SASUM( &n, x, &incx ) ; }
  inline double asum(const integer_t& n, const double*  x, const integer_t& incx) { return BLAS_DASUM( &n, x, &incx ) ; }
  inline float  asum(const integer_t& n, const complex_f*   x, const integer_t& incx) { return (float)BLAS_SCASUM( &n, complex_ptr(x), &incx ) ; }
  inline double asum(const integer_t& n, const complex_d*  x, const integer_t& incx) { return BLAS_DZASUM( &n, complex_ptr(x), &incx ) ; }

  // copy
  inline void copy(const integer_t& n, const float*     x, const integer_t& incx, float*     y, const integer_t& incy) { BLAS_SCOPY( &n, x, &incx, y, &incy ) ; }
  inline void copy(const integer_t& n, const double*    x, const integer_t& incx, double*    y, const integer_t& incy) { BLAS_DCOPY( &n, x, &incx, y, &incy ) ; }
  inline void copy(const integer_t& n, const complex_f* x, const integer_t& incx, complex_f* y, const integer_t& incy) { BLAS_CCOPY( &n, complex_ptr(x), &incx, complex_ptr(y), &incy ) ; }
  inline void copy(const integer_t& n, const complex_d* x, const integer_t& incx, complex_d* y, const integer_t& incy) { BLAS_ZCOPY( &n, complex_ptr(x), &incx, complex_ptr(y), &incy ) ; }
}}}}}

#endif // BOOST_NUMERIC_BINDINGS_BLAS_BLAS1_OVERLOADS_HPP
