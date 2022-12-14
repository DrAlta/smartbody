/*
 *
 * Copyright (c) Kresimir Fresl & Toon Knapen 2003
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * KF acknowledges the support of the Faculty of Civil Engineering,
 * University of Zagreb, Croatia.
 *
 */

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_SPSV_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_SPSV_HPP

#include <boost/numeric/bindings/traits/type_traits.hpp>
#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/lapack/lapack.h>
#include <boost/numeric/bindings/traits/detail/array.hpp>

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
#  include <boost/static_assert.hpp>
#  include <boost/type_traits/is_same.hpp>
#endif

#include <cassert>

namespace boost { namespace numeric { namespace bindings {

  namespace lapack {

    /////////////////////////////////////////////////////////////////////
    //
    // system of linear equations A * X = B with A symmetric matrix
    // stored in packed format 
    //
    /////////////////////////////////////////////////////////////////////

    /*
     * spsv() computes the solution to a system of linear equations
     * A * X = B, where A is an N-by-N symmetric matrix stored in packed
     * format and X and B are N-by-NRHS matrices.
     *
     * The diagonal pivoting method is used to factor A as
     *   A = U * D * U^T,  if UPLO = 'U',
     *   A = L * D * L^T,  if UPLO = 'L',
     * where  U (or L) is a product of permutation and unit upper
     * (lower) triangular matrices, and D is symmetric and block
     * diagonal with 1-by-1 and 2-by-2 diagonal blocks.
     * The factored form of A is then used to solve the system
     * of equations A * X = B.
     */

    namespace detail {

      inline
      void spsv (char const uplo, integer_t const n, integer_t const nrhs,
                 float* ap, integer_t* ipiv,
                 float* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_SSPSV (&uplo, &n, &nrhs, ap, ipiv, b, &ldb, info);
      }

      inline
      void spsv (char const uplo, integer_t const n, integer_t const nrhs,
                 double* ap, integer_t* ipiv,
                 double* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_DSPSV (&uplo, &n, &nrhs, ap, ipiv, b, &ldb, info);
      }

      inline
      void spsv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_f* ap, integer_t* ipiv,
                 traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CSPSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (ap), ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      inline
      void spsv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_d* ap, integer_t* ipiv,
                 traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZSPSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (ap), ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      template <typename SymmA, typename MatrB, typename IVec>
      int spsv (SymmA& a, IVec& i, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
        BOOST_STATIC_ASSERT((boost::is_same<
          typename traits::matrix_traits<SymmA>::matrix_structure,
          traits::symmetric_packed_t
        >::value));
        BOOST_STATIC_ASSERT((boost::is_same<
          typename traits::matrix_traits<MatrB>::matrix_structure,
          traits::general_t
        >::value));
#endif

        integer_t const n = traits::matrix_size1 (a);
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::matrix_size1 (b));

        char uplo = traits::matrix_uplo_tag (a);
        integer_t info;
        spsv (uplo, n, traits::matrix_size2 (b),
              traits::matrix_storage (a),
              traits::vector_storage (i),
              traits::matrix_storage (b),
              traits::leading_dimension (b),
              &info);
        return info;
      }

    }

    template <typename SymmA, typename MatrB, typename IVec>
    inline
    int spsv (SymmA& a, IVec& i, MatrB& b) {
      assert (traits::matrix_size1 (a) == traits::vector_size (i));
      return detail::spsv (a, i, b);
    }

    template <typename SymmA, typename MatrB>
    int spsv (SymmA& a, MatrB& b) {
      // with 'internal' pivot vector

      integer_t info = -101;
      traits::detail::array<integer_t> i (traits::matrix_size1 (a));

      if (i.valid())
        info = detail::spsv (a, i, b);
      return info;
    }


    /*
     * sptrf() computes the factorization of a symmetric matrix A
     * in packed storage using the  Bunch-Kaufman diagonal pivoting
     * method. The form of the factorization is
     *    A = U * D * U^T  or  A = L * D * L^T
     * where U (or L) is a product of permutation and unit upper (lower)
     * triangular matrices, and D is symmetric and block diagonal with
     * 1-by-1 and 2-by-2 diagonal blocks.
     */

    namespace detail {

      inline
      void sptrf (char const uplo, integer_t const n,
                  float* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_SSPTRF (&uplo, &n, ap, ipiv, info);
      }

      inline
      void sptrf (char const uplo, integer_t const n,
                  double* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_DSPTRF (&uplo, &n, ap, ipiv, info);
      }

      inline
      void sptrf (char const uplo, integer_t const n,
                  traits::complex_f* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_CSPTRF (&uplo, &n, traits::complex_ptr (ap), ipiv, info);
      }

      inline
      void sptrf (char const uplo, integer_t const n,
                  traits::complex_d* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_ZSPTRF (&uplo, &n, traits::complex_ptr (ap), ipiv, info);
      }

    }

    template <typename SymmA, typename IVec>
    int sptrf (SymmA& a, IVec& i) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_packed_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::vector_size (i));

      char uplo = traits::matrix_uplo_tag (a);
      integer_t info;
      detail::sptrf (uplo, n, traits::matrix_storage (a),
                     traits::vector_storage (i), &info);
      return info;
    }


    /*
     * sptrs() solves a system of linear equations A*X = B with
     * a symmetric matrix A in packed storage using the factorization
     *    A = U * D * U^T   or  A = L * D * L^T
     * computed by sptrf().
     */

    namespace detail {

      inline
      void sptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  float const* a, integer_t const* ipiv,
                  float* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_SSPTRS (&uplo, &n, &nrhs, a, ipiv, b, &ldb, info);
      }

      inline
      void sptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  double const* a, integer_t const* ipiv,
                  double* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_DSPTRS (&uplo, &n, &nrhs, a, ipiv, b, &ldb, info);
      }

      inline
      void sptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_f const* a, integer_t const* ipiv,
                  traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CSPTRS (&uplo, &n, &nrhs,
                      traits::complex_ptr (a), ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      inline
      void sptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_d const* a, integer_t const* ipiv,
                  traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZSPTRS (&uplo, &n, &nrhs,
                       traits::complex_ptr (a), ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

    }

    template <typename SymmA, typename MatrB, typename IVec>
    int sptrs (SymmA const& a, IVec const& i, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_packed_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::matrix_size1 (b));
      assert (n == traits::vector_size (i));

      char uplo = traits::matrix_uplo_tag (a);
      integer_t info; 
      detail::sptrs (uplo, n, traits::matrix_size2 (b),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
                     traits::matrix_storage (a),
                     traits::vector_storage (i),
#else
                     traits::matrix_storage_const (a),
                     traits::vector_storage_const (i),
#endif 
                     traits::matrix_storage (b),
                     traits::leading_dimension (b),
                     &info);
      return info;
    }


    namespace detail {
      inline
      void sptri (char const uplo, integer_t const n,
          float* ap, integer_t* ipiv, float* work, integer_t* info)
      {
        LAPACK_SSPTRI (&uplo, &n, ap, ipiv, work, info);
      }

      inline
      void sptri (char const uplo, integer_t const n,
          double* ap, integer_t* ipiv, double* work, integer_t* info)
      {
        LAPACK_DSPTRI (&uplo, &n, ap, ipiv, work, info);
      }

      inline
      void sptri (char const uplo, integer_t const n,
          traits::complex_f* ap, integer_t* ipiv, traits::complex_f* work, integer_t* info)
      {
        LAPACK_CSPTRI (&uplo, &n, traits::complex_ptr (ap),
            ipiv, traits::complex_ptr (work), info);
      }

      inline
      void sptri (char const uplo, integer_t const n,
          traits::complex_d* ap, integer_t* ipiv, traits::complex_d* work, integer_t* info)
      {
        LAPACK_ZSPTRI (&uplo, &n, traits::complex_ptr (ap),
            ipiv, traits::complex_ptr (work), info);
      }
    } // namespace detail

    template <typename SymmA, typename IVec>
    int sptri (SymmA& a, IVec& ipiv)
    {
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_packed_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::vector_size (ipiv));

      char uplo = traits::matrix_uplo_tag (a);
      integer_t info;

      typedef typename SymmA::value_type value_type;
      traits::detail::array<value_type> work(traits::matrix_size1(a));

      detail::sptri (uplo, n, traits::matrix_storage (a),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
          traits::vector_storage (ipiv),
#else
          traits::vector_storage_const (ipiv),
#endif
          traits::vector_storage (work),
          &info);
      return info;
    }

  } // namespace lapack

}}} // namespace boost::numeric::bindings




#endif
