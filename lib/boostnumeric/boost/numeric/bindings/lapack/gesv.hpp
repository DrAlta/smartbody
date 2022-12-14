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

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_GESV_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_GESV_HPP

#include <boost/numeric/bindings/traits/type_traits.hpp>
#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/lapack/lapack.h>
#include <boost/numeric/bindings/traits/detail/array.hpp>
#include <boost/numeric/bindings/lapack/workspace.hpp>
#include <boost/numeric/bindings/lapack/ilaenv.hpp>


#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
#  include <boost/static_assert.hpp>
#  include <boost/type_traits/is_same.hpp>
#endif

#include <cassert>


namespace boost { namespace numeric { namespace bindings {

  namespace lapack {

    ///////////////////////////////////////////////////////////////////
    //
    // general system of linear equations A * X = B
    //
    ///////////////////////////////////////////////////////////////////

    /*
     * gesv() computes the solution to a system of linear equations 
     * A * X = B, where A is an N-by-N matrix and X and B are N-by-NRHS 
     * matrices.
     *
     * The LU decomposition with partial pivoting and row interchanges
     * is used to factor A as A = P * L * U, where P is a permutation
     * matrix, L is unit lower triangular, and U is upper triangular.
     * The factored form of A is then used to solve the system of
     * equations A * X = B.
     */

    namespace detail {

      inline
      void gesv (integer_t const n, integer_t const nrhs,
                 float* a, integer_t const lda, integer_t* ipiv,
                 float* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_SGESV (&n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void gesv (integer_t const n, integer_t const nrhs,
                 double* a, integer_t const lda, integer_t* ipiv,
                 double* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_DGESV (&n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void gesv (integer_t const n, integer_t const nrhs,
                 traits::complex_f* a, integer_t const lda, integer_t* ipiv,
                 traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CGESV (&n, &nrhs,
                      traits::complex_ptr (a), &lda, ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      inline
      void gesv (integer_t const n, integer_t const nrhs,
                 traits::complex_d* a, integer_t const lda, integer_t* ipiv,
                 traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZGESV (&n, &nrhs,
                      traits::complex_ptr (a), &lda, ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

    } //namespace detail

    template <typename MatrA, typename MatrB, typename IVec>
    int gesv (MatrA& a, IVec& ipiv, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::matrix_size1 (b));
      assert (n == traits::vector_size (ipiv));

      integer_t info;
      detail::gesv (n, traits::matrix_size2 (b),
                    traits::matrix_storage (a),
                    traits::leading_dimension (a),
                    traits::vector_storage (ipiv),
                    traits::matrix_storage (b),
                    traits::leading_dimension (b),
                    &info);
      return info;
    }

    template <typename MatrA, typename MatrB>
    int gesv (MatrA& a, MatrB& b) {
      // with 'internal' pivot vector

      // gesv() errors:
      //   if (info == 0), successful
      //   if (info < 0), the -info argument had an illegal value
      //   -- we will use -101 if allocation fails
      //   if (info > 0), U(i-1,i-1) is exactly zero
      integer_t info = -101;
      traits::detail::array<integer_t> ipiv (traits::matrix_size1 (a));
      if (ipiv.valid())
        info = gesv (a, ipiv, b);
      return info;
    }


    /*
     * getrf() computes an LU factorization of a general M-by-N matrix A
     * using partial pivoting with row interchanges. The factorization
     * has the form A = P * L * U, where P is a permutation matrix,
     * L is lower triangular with unit diagonal elements (lower
     * trapezoidal if M > N), and U is upper triangular (upper
     * trapezoidal if M < N).
     */

    namespace detail {

      inline
      void getrf (integer_t const n, integer_t const m,
                  float* a, integer_t const lda, integer_t* ipiv, integer_t* info)
      {
        LAPACK_SGETRF (&n, &m, a, &lda, ipiv, info);
      }

      inline
      void getrf (integer_t const n, integer_t const m,
                  double* a, integer_t const lda, integer_t* ipiv, integer_t* info)
      {
        LAPACK_DGETRF (&n, &m, a, &lda, ipiv, info);
      }

      inline
      void getrf (integer_t const n, integer_t const m,
                  traits::complex_f* a, integer_t const
                  lda, integer_t* ipiv, integer_t* info)
      {
        LAPACK_CGETRF (&n, &m, traits::complex_ptr (a), &lda, ipiv, info);
      }

      inline
      void getrf (integer_t const n, integer_t const m,
                  traits::complex_d* a, integer_t const lda,
                  integer_t* ipiv, integer_t* info)
      {
        LAPACK_ZGETRF (&n, &m, traits::complex_ptr (a), &lda, ipiv, info);
      }

    } //namespace detail

    template <typename MatrA, typename IVec>
    int getrf (MatrA& a, IVec& ipiv) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      integer_t const m = traits::matrix_size2 (a);
      assert (traits::vector_size (ipiv) == (m < n ? m : n));

      integer_t info;
      detail::getrf (n, m,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (ipiv),
                     &info);
      return info;
    }


    /*
     * getrs() solves a system of linear equations A * X = B
     * or A^T * X = B with a general N-by-N matrix A using
     * the LU factorization computed by getrf().
     */

    namespace detail {

      inline
      void getrs (char const trans, integer_t const n, integer_t const nrhs,
                  float const* a, integer_t const lda, integer_t const* ipiv,
                  float* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_SGETRS (&trans, &n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void getrs (char const trans, integer_t const n, integer_t const nrhs,
                  double const* a, integer_t const lda, integer_t const* ipiv,
                  double* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_DGETRS (&trans, &n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void getrs (char const trans, integer_t const n, integer_t const nrhs,
                  traits::complex_f const* a, integer_t const lda,
                  integer_t const* ipiv,
                  traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CGETRS (&trans, &n, &nrhs,
                       traits::complex_ptr (a), &lda, ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

      inline
      void getrs (char const trans, integer_t const n, integer_t const nrhs,
                  traits::complex_d const* a, integer_t const lda,
                  integer_t const* ipiv,
                  traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZGETRS (&trans, &n, &nrhs,
                       traits::complex_ptr (a), &lda, ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

    } // namespace detail

    template <typename MatrA, typename MatrB, typename IVec>
    int getrs (char const trans, MatrA const& a, IVec const& ipiv, MatrB& b)
    {
      assert (trans == 'N' || trans == 'T' || trans == 'C');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));
      assert (n == traits::matrix_size1 (b));
      assert (n == traits::vector_size (ipiv));

      integer_t info;
      detail::getrs (trans, n, traits::matrix_size2 (b),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
                     traits::matrix_storage (a),
#else
                     traits::matrix_storage_const (a),
#endif
                     traits::leading_dimension (a),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
                     traits::vector_storage (ipiv),
#else
                     traits::vector_storage_const (ipiv),
#endif
                     traits::matrix_storage (b),
                     traits::leading_dimension (b),
                     &info);
      return info;
    }

    template <typename MatrA, typename MatrB, typename IVec>
    inline
    int getrs (MatrA const& a, IVec const& ipiv, MatrB& b) {
      char const no_transpose = 'N';
      return getrs (no_transpose, a, ipiv, b);
    }

    /*
     * getri() computes the inverse of a matrix using
     * the LU factorization computed by getrf().
     */

    namespace detail {

      inline
      void getri (integer_t const n, float* a, integer_t const lda, integer_t const* ipiv,
                  float* work, integer_t const lwork, integer_t* info)
      {
        LAPACK_SGETRI (&n, a, &lda, ipiv, work, &lwork, info);
      }

      inline
      void getri (integer_t const n, double* a, integer_t const lda, integer_t const* ipiv,
                  double* work, integer_t const lwork, integer_t* info)
      {
        LAPACK_DGETRI (&n, a, &lda, ipiv, work, &lwork, info);
      }

      inline
      void getri (integer_t const n, traits::complex_f* a, integer_t const lda,
          integer_t const* ipiv, traits::complex_f* work, integer_t const lwork,
          integer_t* info)
      {
        LAPACK_CGETRI (&n, traits::complex_ptr (a), &lda, ipiv,
            traits::complex_ptr (work), &lwork, info);
      }

      inline
      void getri (integer_t const n, traits::complex_d* a, integer_t const lda,
          integer_t const* ipiv, traits::complex_d* work, integer_t const lwork,
          integer_t* info)
      {
        LAPACK_ZGETRI (&n, traits::complex_ptr (a), &lda, ipiv,
            traits::complex_ptr (work), &lwork, info);
      }



      template <typename MatrA, typename IVec, typename Work>
      int getri (MatrA& a, IVec const& ipiv, Work& work)
      {
        #ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
        BOOST_STATIC_ASSERT((boost::is_same<
              typename traits::matrix_traits<MatrA>::matrix_structure,
              traits::general_t
              >::value));
        #endif

        integer_t const n = traits::matrix_size1 (a);
        assert (n > 0);
        assert (n <= traits::leading_dimension (a));
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::vector_size (ipiv));
        assert (n <= traits::vector_size (work)); //Minimal workspace size

        integer_t info;
        //double* dummy = traits::matrix_storage (a);
        detail::getri (n, traits::matrix_storage (a),
            traits::leading_dimension (a),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
            traits::vector_storage (ipiv),
#else
            traits::vector_storage_const (ipiv),
#endif
            traits::vector_storage (work),
            traits::vector_size (work),
            &info);
        return info;
      }


      inline
      integer_t getri_block(float)
      {
        return lapack::ilaenv(1, "sgetri", "");
      }

      inline
      integer_t getri_block(double)
      {
        return lapack::ilaenv(1, "dgetri", "");
      }

      inline
      integer_t getri_block(traits::complex_f)
      {
        return lapack::ilaenv(1, "cgetri", "");
      }

      inline
      integer_t getri_block(traits::complex_d)
      {
        return lapack::ilaenv(1, "zgetri", "");
      }

    } // namespace detail


    template <typename MatrA, typename IVec>
    int getri(MatrA& a, IVec& ipiv, minimal_workspace)
    {
      typedef typename MatrA::value_type value_type;

      std::ptrdiff_t n = traits::matrix_size1(a);
      traits::detail::array<value_type> work(std::max<std::ptrdiff_t>(1,n));

      return detail::getri(a, ipiv, work);

    }


    // optimal workspace allocation
    template <typename MatrA, typename IVec>
    int getri(MatrA& a, IVec& ipiv, optimal_workspace)
    {
      typedef typename MatrA::value_type value_type;

      std::ptrdiff_t n = traits::matrix_size1(a);
      std::ptrdiff_t nb = detail::getri_block(value_type());
      traits::detail::array<value_type> work(std::max<std::ptrdiff_t>(1,n*nb));

      return detail::getri(a, ipiv, work);
    }


    template <typename MatrA, typename IVec>
    inline
    int getri(MatrA& a, IVec& ipiv)
    {
      return getri(a, ipiv, optimal_workspace());
    }


    template <typename MatrA, typename IVec, typename Work>
    inline
    int getri(MatrA& a, IVec& ipiv, Work& work)
    {
      return detail::getri(a, ipiv, work);
    }

  } // namespace lapack

}}} // namespace boost::numeric::bindings




#endif
