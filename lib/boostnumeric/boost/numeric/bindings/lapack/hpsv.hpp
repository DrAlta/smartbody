/*
 *
 * Copyright (c) Toon Knapen & Kresimir Fresl 2003
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * KF acknowledges the support of the Faculty of Civil Engineering,
 * University of Zagreb, Croatia.
 *
 */

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_HPSV_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_HPSV_HPP

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
    // system of linear equations A * X = B
    // with A Hermitian indefinite matrix stored in packed format
    //
    /////////////////////////////////////////////////////////////////////

    /*
     * hpsv() computes the solution to a system of linear equations
     * A * X = B, where A is an N-by-N Hermitian matrix in packed
     * storage and X and B are N-by-NRHS matrices.
     *
     * The diagonal pivoting method is used to factor A as
     *   A = U * D * U^H,  if UPLO = 'U',
     *   A = L * D * L^H,  if UPLO = 'L',
     * where  U (or L) is a product of permutation and unit upper
     * (lower) triangular matrices, and D is Hermitian and block
     * diagonal with 1-by-1 and 2-by-2 diagonal blocks. The factored
     * form of A is then used to solve the system of equations A * X = B.
     */

    namespace detail {

      inline
      void hpsv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_f* ap, integer_t* ipiv,
                 traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CHPSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (ap), ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      inline
      void hpsv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_d* ap, integer_t* ipiv,
                 traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZHPSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (ap), ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      template <typename HermA, typename MatrB, typename IVec>
      int hpsv (HermA& a, IVec& i, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
        BOOST_STATIC_ASSERT((boost::is_same<
          typename traits::matrix_traits<HermA>::matrix_structure,
          traits::hermitian_packed_t
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
        hpsv (uplo, n, traits::matrix_size2 (b),
              traits::matrix_storage (a),
              traits::vector_storage (i),
              traits::matrix_storage (b),
              traits::leading_dimension (b),
              &info);
        return info;
      }

    }

    template <typename HermA, typename MatrB, typename IVec>
    inline
    int hpsv (HermA& a, IVec& i, MatrB& b) {
      assert (traits::matrix_size1 (a) == traits::vector_size (i));
      return detail::hpsv (a, i, b);
    }

    template <typename HermA, typename MatrB>
    int hpsv (HermA& a, MatrB& b) {
      // with 'internal' pivot vector

      integer_t info = -101;
      traits::detail::array<integer_t> i (traits::matrix_size1 (a));

      if (i.valid())
        info = detail::hpsv (a, i, b);
      return info;
    }


    /*
     * hptrf() computes the factorization of a Hermitian matrix A
     * in packed storage using the  Bunch-Kaufman diagonal pivoting
     * method. The form of the factorization is
     *    A = U * D * U^H  or  A = L * D * L^H
     * where U (or L) is a product of permutation and unit upper (lower)
     * triangular matrices, and D is Hermitian and block diagonal with
     * 1-by-1 and 2-by-2 diagonal blocks.
     */

    namespace detail {

      inline
      void hptrf (char const uplo, integer_t const n,
                  traits::complex_f* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_CHPTRF (&uplo, &n, traits::complex_ptr (ap), ipiv, info);
      }

      inline
      void hptrf (char const uplo, integer_t const n,
                  traits::complex_d* ap, integer_t* ipiv, integer_t* info)
      {
        LAPACK_ZHPTRF (&uplo, &n, traits::complex_ptr (ap), ipiv, info);
      }

      template <typename HermA, typename IVec, typename Work>
      inline
      int hptrf (char const ul, HermA& a, IVec& i, Work& w, integer_t const lw) {
        assert( 0 ) ;
        return 0 ;
      }

    }

    template <typename HermA, typename IVec>
    int hptrf (HermA& a, IVec& i) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<HermA>::matrix_structure,
        traits::hermitian_packed_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::vector_size (i));

      char uplo = traits::matrix_uplo_tag (a);
      integer_t info;
      detail::hptrf (uplo, n, traits::matrix_storage (a),
                     traits::vector_storage (i), &info);
      return info;
    }


    /*
     * hptrs() solves a system of linear equations A*X = B with
     * a Hermitian matrix A in packed storage using the factorization
     *    A = U * D * U^H   or  A = L * D * L^H
     * computed by hptrf().
     */

    namespace detail {

      inline
      void hptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_f const* ap, integer_t const* ipiv,
                  traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CHPTRS (&uplo, &n, &nrhs,
                       traits::complex_ptr (ap), ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

      inline
      void hptrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_d const* ap, integer_t const* ipiv,
                  traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZHPTRS (&uplo, &n, &nrhs,
                       traits::complex_ptr (ap), ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

    }

    template <typename HermA, typename MatrB, typename IVec>
    int hptrs (HermA const& a, IVec const& i, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<HermA>::matrix_structure,
        traits::hermitian_packed_t
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
      detail::hptrs (uplo, n, traits::matrix_size2 (b),
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


    // TO DO: hptri

  }

}}}

#endif
