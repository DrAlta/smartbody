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

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_SYSV_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_SYSV_HPP

#include <boost/numeric/bindings/traits/type_traits.hpp>
#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/lapack/lapack.h>
#include <boost/numeric/bindings/traits/detail/array.hpp>
#include "boost/numeric/bindings/traits/ublas_symmetric.hpp"
#include <boost/numeric/bindings/lapack/ilaenv.hpp>


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
    //
    /////////////////////////////////////////////////////////////////////

    namespace detail {

      inline
      integer_t sytrf_block (float, integer_t const ispec, char const ul, integer_t const n)
      {
        char ul2[2] = "x"; ul2[0] = ul; 
        return ilaenv (ispec, "SSYTRF", ul2, n);
      }
      inline
      integer_t sytrf_block (double, integer_t const ispec, char const ul, integer_t const n) {
        char ul2[2] = "x"; ul2[0] = ul;
        return ilaenv (ispec, "DSYTRF", ul2, n);
      }
      inline
      integer_t sytrf_block (traits::complex_f,
                       integer_t const ispec, char const ul, integer_t const n)
      {
        char ul2[2] = "x"; ul2[0] = ul;
        return ilaenv (ispec, "CSYTRF", ul2, n);
      }
      inline
      integer_t sytrf_block (traits::complex_d,
                       integer_t const ispec, char const ul, integer_t const n)
      {
        char ul2[2] = "x"; ul2[0] = ul;
        return ilaenv (ispec, "ZSYTRF", ul2, n);
      }
    }


    template <typename SymmA>
    integer_t sytrf_block (char const q, char const ul, SymmA const& a)
    {
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
#endif
      assert (q == 'O' || q == 'M');
      assert (ul == 'U' || ul == 'L');

      integer_t n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      integer_t ispec = (q == 'O' ? 1 : 2);
      return detail::sytrf_block (val_t(), ispec, ul, n);
    }

    template <typename SymmA>
    integer_t sytrf_block (char const q, SymmA const& a)
    {
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
#endif
      assert (q == 'O' || q == 'M');

      char ul = traits::matrix_uplo_tag (a);
      integer_t n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      integer_t ispec = (q == 'O' ? 1 : 2);
      return detail::sytrf_block (val_t(), ispec, ul, n);
    }

    template <typename SymmA>
    integer_t sytrf_work (char const q, char const ul, SymmA const& a)
    {
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
#endif
      assert (q == 'O' || q == 'M');
      assert (ul == 'U' || ul == 'L');

      integer_t n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      integer_t lw = -13;
      if (q == 'M')
        lw = 1;
      if (q == 'O')
        lw = n * detail::sytrf_block (val_t(), 1, ul, n);
      return lw;
    }

    template <typename SymmA>
    integer_t sytrf_work (char const q, SymmA const& a) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
#endif
      assert (q == 'O' || q == 'M');

      char ul = traits::matrix_uplo_tag (a);
      integer_t n = traits::matrix_size1 (a);
      assert (n == traits::matrix_size2 (a));

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      integer_t lw = -13;
      if (q == 'M')
        lw = 1;
      if (q == 'O')
        lw = n * detail::sytrf_block (val_t(), 1, ul, n);
      return lw;
    }


    template <typename SymmA>
    inline
    integer_t sysv_work (char const q, char const ul, SymmA const& a) {
      return sytrf_work (q, ul, a);
    }

    template <typename SymmA>
    inline
    integer_t sysv_work (char const q, SymmA const& a) { return sytrf_work (q, a); }


    /*
     * sysv() computes the solution to a system of linear equations
     * A * X = B, where A is an N-by-N symmetric matrix and X and B
     * are N-by-NRHS matrices.
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

    namespace detail
    {

      inline
      void sysv (char const uplo, integer_t const n, integer_t const nrhs,
                 float* a, integer_t const lda, integer_t* ipiv,
                 float* b, integer_t const ldb,
                 float* w, integer_t const lw, integer_t* info)
      {
        LAPACK_SSYSV (&uplo, &n, &nrhs, a, &lda, ipiv, b, &ldb, w, &lw, info);
      }

      inline
      void sysv (char const uplo, integer_t const n, integer_t const nrhs,
                 double* a, integer_t const lda, integer_t* ipiv,
                 double* b, integer_t const ldb,
                 double* w, integer_t const lw, integer_t* info)
      {
        LAPACK_DSYSV (&uplo, &n, &nrhs, a, &lda, ipiv, b, &ldb, w, &lw, info);
      }

      inline
      void sysv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_f* a, integer_t const lda, integer_t* ipiv,
                 traits::complex_f* b, integer_t const ldb,
                 traits::complex_f* w, integer_t const lw, integer_t* info)
      {
        LAPACK_CSYSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (a), &lda, ipiv,
                      traits::complex_ptr (b), &ldb,
                      traits::complex_ptr (w), &lw, info);
      }

      inline
      void sysv (char const uplo, integer_t const n, integer_t const nrhs,
                 traits::complex_d* a, integer_t const lda, integer_t* ipiv,
                 traits::complex_d* b, integer_t const ldb,
                 traits::complex_d* w, integer_t const lw, integer_t* info)
      {
        LAPACK_ZSYSV (&uplo, &n, &nrhs,
                      traits::complex_ptr (a), &lda, ipiv,
                      traits::complex_ptr (b), &ldb,
                      traits::complex_ptr (w), &lw, info);
      }

      template <typename SymmA, typename MatrB, typename IVec, typename Work>
      int sysv (char const ul, SymmA& a, IVec& i, MatrB& b, Work& w)
      {
        integer_t const n = traits::matrix_size1 (a);
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::matrix_size1 (b));
        assert (n == traits::vector_size (i));

        integer_t info;
        sysv (ul, n, traits::matrix_size2 (b),
              traits::matrix_storage (a),
              traits::leading_dimension (a),
              traits::vector_storage (i),
              traits::matrix_storage (b),
              traits::leading_dimension (b),
              traits::vector_storage (w),
              traits::vector_size (w),
              &info);
        return info;
      }

    }

    template <typename SymmA, typename MatrB, typename IVec, typename Work>
    inline
    int sysv (char const ul, SymmA& a, IVec& i, MatrB& b, Work& w)
    {
      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      assert (traits::vector_size (w) >= 1);
      return detail::sysv (ul, a, i, b, w);
    }

    template <typename SymmA, typename MatrB, typename IVec, typename Work>
    inline
    int sysv (SymmA& a, IVec& i, MatrB& b, Work& w) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      assert (traits::vector_size (w) >= 1);
      char uplo = traits::matrix_uplo_tag (a);
      return detail::sysv (uplo, a, i, b, w);
    }

    template <typename SymmA, typename MatrB>
    int sysv (char const ul, SymmA& a, MatrB& b)
    {
      // with 'internal' pivot and work vectors

      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      integer_t info = -101;
      traits::detail::array<integer_t> i (n);

      if (i.valid()) {
        info = -102;
        integer_t lw = sytrf_work ('O', ul, a);
        assert (lw >= 1); // paranoia ?
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
        typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
        typedef typename SymmA::value_type val_t;
#endif
        traits::detail::array<val_t> w (lw);
        if (w.valid())
          info =  detail::sysv (ul, a, i, b, w);
      }
      return info;
    }

    template <typename SymmA, typename MatrB>
    int sysv (SymmA& a, MatrB& b)
    {
      // with 'internal' pivot and work vectors

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const n = traits::matrix_size1 (a);
      char uplo = traits::matrix_uplo_tag (a);
      integer_t info = -101;
      traits::detail::array<integer_t> i (n);

      if (i.valid()) {
        info = -102;
        integer_t lw = sytrf_work ('O', a);
        assert (lw >= 1); // paranoia ?
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
        typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
        typedef typename SymmA::value_type val_t;
#endif
        traits::detail::array<val_t> w (lw);
        if (w.valid())
          info =  detail::sysv (uplo, a, i, b, w);
      }
      return info;
    }


    /*
     * sytrf() computes the factorization of a symmetric matrix A using
     * the  Bunch-Kaufman diagonal pivoting method. The form of the
     * factorization is
     *    A = U * D * U^T  or  A = L * D * L^T
     * where U (or L) is a product of permutation and unit upper (lower)
     * triangular matrices, and D is symmetric and block diagonal with
     * 1-by-1 and 2-by-2 diagonal blocks.
     */

    namespace detail
    {
      inline
      void sytrf (char const uplo, integer_t const n,
                  float* a, integer_t const lda, integer_t* ipiv,
                  float* w, integer_t const lw, integer_t* info)
      {
        LAPACK_SSYTRF (&uplo, &n, a, &lda, ipiv, w, &lw, info);
      }

      inline
      void sytrf (char const uplo, integer_t const n,
                  double* a, integer_t const lda, integer_t* ipiv,
                  double* w, integer_t const lw, integer_t* info)
      {
        LAPACK_DSYTRF (&uplo, &n, a, &lda, ipiv, w, &lw, info);
      }

      inline
      void sytrf (char const uplo, integer_t const n,
                  traits::complex_f* a, integer_t const lda, integer_t* ipiv,
                  traits::complex_f* w, integer_t const lw, integer_t* info)
      {
        LAPACK_CSYTRF (&uplo, &n,
                       traits::complex_ptr (a), &lda, ipiv,
                       traits::complex_ptr (w), &lw, info);
      }

      inline
      void sytrf (char const uplo, integer_t const n,
                  traits::complex_d* a, integer_t const lda, integer_t* ipiv,
                  traits::complex_d* w, integer_t const lw, integer_t* info)
      {
        LAPACK_ZSYTRF (&uplo, &n,
                       traits::complex_ptr (a), &lda, ipiv,
                       traits::complex_ptr (w), &lw, info);
      }

      template <typename SymmA, typename IVec, typename Work>
      int sytrf (char const ul, SymmA& a, IVec& i, Work& w) {

        integer_t const n = traits::matrix_size1 (a);
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::vector_size (i));

        integer_t info;
        sytrf (ul, n, traits::matrix_storage (a),
               traits::leading_dimension (a),
               traits::vector_storage (i),
               traits::vector_storage (w),
               traits::vector_size (w),
               &info);
        return info;
      }

    }

    template <typename SymmA, typename IVec, typename Work>
    inline
    int sytrf (char const ul, SymmA& a, IVec& i, Work& w)
    {
      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
#endif

      assert (traits::vector_size (w) >= 1);
      return detail::sytrf (ul, a, i, w);
    }

    template <typename SymmA, typename IVec, typename Work>
    inline
    int sytrf (SymmA& a, IVec& i, Work& w)
    {
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
#endif

      assert (traits::vector_size (w) >= 1);
      char uplo = traits::matrix_uplo_tag (a);
      return detail::sytrf (uplo, a, i, w);
    }

    template <typename SymmA, typename Ivec>
    int sytrf (char const ul, SymmA& a, Ivec& i)
    {
      // with 'internal' work vector

      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t info = -101;
      integer_t lw = sytrf_work ('O', ul, a);
      assert (lw >= 1); // paranoia ?
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      traits::detail::array<val_t> w (lw);
      if (w.valid())
        info =  detail::sytrf (ul, a, i, w);
      return info;
    }

    template <typename SymmA, typename Ivec>
    int sytrf (SymmA& a, Ivec& i)
    {
      // with 'internal' work vector

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
#endif

      char uplo = traits::matrix_uplo_tag (a);
      integer_t info = -101;
      integer_t lw = sytrf_work ('O', a);
      assert (lw >= 1); // paranoia ?
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<SymmA>::value_type val_t;
#else
      typedef typename SymmA::value_type val_t;
#endif
      traits::detail::array<val_t> w (lw);
      if (w.valid())
        info =  detail::sytrf (uplo, a, i, w);
      return info;
    }


    /*
     * sytrs() solves a system of linear equations A*X = B with
     * a symmetric matrix A using the factorization
     *    A = U * D * U^T   or  A = L * D * L^T
     * computed by sytrf().
     */

    namespace detail {

      inline
      void sytrs (char const uplo, integer_t const n, integer_t const nrhs,
                  float const* a, integer_t const lda, integer_t const* ipiv,
                  float* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_SSYTRS (&uplo, &n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void sytrs (char const uplo, integer_t const n, integer_t const nrhs,
                  double const* a, integer_t const lda, integer_t const* ipiv,
                  double* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_DSYTRS (&uplo, &n, &nrhs, a, &lda, ipiv, b, &ldb, info);
      }

      inline
      void sytrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_f const* a, integer_t const lda,
                  integer_t const* ipiv,
                  traits::complex_f* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_CSYTRS (&uplo, &n, &nrhs,
                      traits::complex_ptr (a), &lda, ipiv,
                      traits::complex_ptr (b), &ldb, info);
      }

      inline
      void sytrs (char const uplo, integer_t const n, integer_t const nrhs,
                  traits::complex_d const* a, integer_t const lda,
                  integer_t const* ipiv,
                  traits::complex_d* b, integer_t const ldb, integer_t* info)
      {
        LAPACK_ZSYTRS (&uplo, &n, &nrhs,
                       traits::complex_ptr (a), &lda, ipiv,
                       traits::complex_ptr (b), &ldb, info);
      }

      template <typename SymmA, typename MatrB, typename IVec>
      int sytrs (char const ul, SymmA const& a, IVec const& i, MatrB& b) {

        integer_t const n = traits::matrix_size1 (a);
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::matrix_size1 (b));
        assert (n == traits::vector_size (i));

        integer_t info;
        sytrs (ul, n, traits::matrix_size2 (b),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
               traits::matrix_storage (a),
#else
               traits::matrix_storage_const (a),
#endif
               traits::leading_dimension (a),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
               traits::vector_storage (i),
#else
               traits::vector_storage_const (i),
#endif
               traits::matrix_storage (b),
               traits::leading_dimension (b), &info);
        return info;
      }

    }

    template <typename SymmA, typename MatrB, typename IVec>
    inline
    int sytrs (char const ul, SymmA const& a, IVec const& i, MatrB& b) {

      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      return detail::sytrs (ul, a, i, b);
    }

    template <typename SymmA, typename MatrB, typename IVec>
    inline
    int sytrs (SymmA const& a, IVec const& i, MatrB& b) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrB>::matrix_structure,
        traits::general_t
      >::value));
#endif

      char uplo = traits::matrix_uplo_tag (a);
      return detail::sytrs (uplo, a, i, b);
    }



    namespace detail
    {
      inline
      void sytri (char const uplo, integer_t const n, float* a, integer_t const lda,
          integer_t const* ipiv, float* work, integer_t* info)
      {
        LAPACK_SSYTRI (&uplo, &n, a, &lda, ipiv, work, info);
      }

      inline
      void sytri (char const uplo, integer_t const n, double* a, integer_t const lda,
          integer_t const* ipiv, double* work, integer_t* info)
      {
        LAPACK_DSYTRI (&uplo, &n, a, &lda, ipiv, work, info);
      }

      inline
      void sytri (char const uplo, integer_t const n, traits::complex_f* a,
          integer_t const lda, integer_t const* ipiv, traits::complex_f* work, integer_t* info)
      {
        LAPACK_CSYTRI (&uplo, &n, traits::complex_ptr (a), &lda, ipiv,
            traits::complex_ptr (work), info);
      }

      inline
      void sytri (char const uplo, integer_t const n, traits::complex_d* a,
          integer_t const lda, integer_t const* ipiv, traits::complex_d* work, integer_t* info)
      {
        LAPACK_ZSYTRI (&uplo, &n, traits::complex_ptr (a), &lda, ipiv,
            traits::complex_ptr (work), info);
      }

      template <typename SymmA, typename IVec, typename Work>
      int sytri (char const ul, SymmA& a, IVec const& ipiv, Work& work)
      {
        assert (ul == 'U' || ul == 'L');

        integer_t const n = traits::matrix_size1 (a);
        assert (n == traits::matrix_size2 (a));
        assert (n == traits::vector_size (ipiv));
        assert (n == traits::vector_size (work));

        integer_t info;
        //const double* dummy = traits::matrix_storage (a);
        detail::sytri (ul, n, traits::matrix_storage (a),
            traits::leading_dimension (a),
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
            traits::vector_storage (ipiv),
#else
            traits::vector_storage_const (ipiv),
#endif
            traits::vector_storage (work),
            &info);
        return info;
      }

    } // namespace detail


    //Internal allocation of workspace, general matrix with up/low tag
    template <typename SymmA, typename IVec>
    int sytri (char const ul, SymmA& a, IVec const& ipiv)
    {
      assert (ul == 'U' || ul == 'L');

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
            typename traits::matrix_traits<SymmA>::matrix_structure,
            traits::general_t
            >::value));
#endif

      typedef typename SymmA::value_type value_type;
      std::ptrdiff_t n = traits::matrix_size1(a);
      traits::detail::array<value_type> work(std::max<std::ptrdiff_t>(1,n));

      return detail::sytri (ul, a, ipiv, work);
    }

    //Internal allocation of workspace, symmetric matrix

    /*Warning: the function will work only if SymmA is a
      symmetric_adaptor. With SymmA = symmetric_matrix a
      boost::STATIC_ASSERTION_FAILURE will be thrown at compile
      time, because symmetric_matrix has a symmetric_packed_t
      structure instead of symmetric_t. Use sptri() for
      symmetric packed matrices.
      */
    template <typename SymmA, typename IVec>
    int sytri (SymmA& a, IVec const& ipiv)
    {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<SymmA>::matrix_structure,
        traits::symmetric_t
      >::value));
#endif

      typedef typename SymmA::value_type value_type;
      std::ptrdiff_t n = traits::matrix_size1(a);
      traits::detail::array<value_type> work(std::max<std::ptrdiff_t>(1,n));

      char uplo = traits::matrix_uplo_tag (a);
      return detail::sytri (uplo, a, ipiv, work);
    }

  } // namespace lapack

}}} // namespace boost::numeric::bindings

#endif
