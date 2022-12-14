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

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_GESVD_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_GESVD_HPP

#include <boost/numeric/bindings/traits/type_traits.hpp>
#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/lapack/lapack.h>
#include <boost/numeric/bindings/traits/detail/array.hpp>
#include <boost/numeric/bindings/traits/detail/utils.hpp>

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
#  include <boost/static_assert.hpp>
#  include <boost/type_traits/is_same.hpp>
#endif

#include <cassert>

namespace boost { namespace numeric { namespace bindings {

  namespace lapack {

    ///////////////////////////////////////////////////////////////////
    //
    // singular value decomposition 
    //
    ///////////////////////////////////////////////////////////////////

    /*
     * (simple driver)
     * gesvd() computes the singular value decomposition (SVD) of
     * M-by-N matrix A, optionally computing the left and/or right
     * singular vectors. The SVD is written
     *
     *     A = U * S * V^T    or    A = U * S * V^H
     *
     * where S is an M-by-N matrix which is zero except for its min(m,n)
     * diagonal elements, U is an M-by-M orthogonal/unitary matrix, and V
     * is an N-by-N orthogonal/unitary matrix. The diagonal elements of S
     * are the singular values of A; they are real and non-negative, and
     * are returned in descending  order. The first min(m,n) columns of
     * U and V are the left and right singular vectors of A. (Note that
     * the routine returns V^T or V^H, not V.
     */

    namespace detail {

      inline
      void gesvd (char const jobu, char const jobvt,
                  integer_t const m, integer_t const n, float* a, integer_t const lda,
                  float* s, float* u, integer_t const ldu,
                  float* vt, integer_t const ldvt,
                  float* work, integer_t const lwork, float* /* dummy */,
                  integer_t* info)
      {
        LAPACK_SGESVD (&jobu, &jobvt, &m, &n, a, &lda,
                       s, u, &ldu, vt, &ldvt, work, &lwork, info);
      }

      inline
      void gesvd (char const jobu, char const jobvt,
                  integer_t const m, integer_t const n, double* a, integer_t const lda,
                  double* s, double* u, integer_t const ldu,
                  double* vt, integer_t const ldvt,
                  double* work, integer_t const lwork, double* /* dummy */,
                  integer_t* info)
      {
        LAPACK_DGESVD (&jobu, &jobvt, &m, &n, a, &lda,
                       s, u, &ldu, vt, &ldvt, work, &lwork, info);
      }

      inline
      void gesvd (char const jobu, char const jobvt,
                  integer_t const m, integer_t const n,
                  traits::complex_f* a, integer_t const lda,
                  float* s, traits::complex_f* u, integer_t const ldu,
                  traits::complex_f* vt, integer_t const ldvt,
                  traits::complex_f* work, integer_t const lwork,
                  float* rwork, integer_t* info)
      {
        LAPACK_CGESVD (&jobu, &jobvt, &m, &n,
                       traits::complex_ptr (a), &lda, s,
                       traits::complex_ptr (u), &ldu,
                       traits::complex_ptr (vt), &ldvt,
                       traits::complex_ptr (work), &lwork, rwork, info);
      }

      inline
      void gesvd (char const jobu, char const jobvt,
                  integer_t const m, integer_t const n,
                  traits::complex_d* a, integer_t const lda,
                  double* s, traits::complex_d* u, integer_t const ldu,
                  traits::complex_d* vt, integer_t const ldvt,
                  traits::complex_d* work, integer_t const lwork,
                  double* rwork, integer_t* info)
      {
        LAPACK_ZGESVD (&jobu, &jobvt, &m, &n,
                       traits::complex_ptr (a), &lda, s,
                       traits::complex_ptr (u), &ldu,
                       traits::complex_ptr (vt), &ldvt,
                       traits::complex_ptr (work), &lwork, rwork, info);
      }

      inline
      integer_t gesvd_min_work (float, integer_t m, integer_t n) {
        integer_t minmn = m < n ? m : n;
        integer_t maxmn = m < n ? n : m;
        integer_t m3x = 3 * minmn + maxmn;
        integer_t m5 = 5 * minmn;
        return m3x < m5 ? m5 : m3x;
      }
      inline
      integer_t gesvd_min_work (double, integer_t m, integer_t n) {
        integer_t minmn = m < n ? m : n;
        integer_t maxmn = m < n ? n : m;
        integer_t m3x = 3 * minmn + maxmn;
        integer_t m5 = 5 * minmn;
        return m3x < m5 ? m5 : m3x;
      }
      inline
      integer_t gesvd_min_work (traits::complex_f, integer_t m, integer_t n) {
        integer_t minmn = m < n ? m : n;
        integer_t maxmn = m < n ? n : m;
        return 2 * minmn + maxmn;
      }
      inline
      integer_t gesvd_min_work (traits::complex_d, integer_t m, integer_t n) {
        integer_t minmn = m < n ? m : n;
        integer_t maxmn = m < n ? n : m;
        return 2 * minmn + maxmn;
      }

      inline
      integer_t gesvd_rwork (float, integer_t, integer_t) { return 1; }
      inline
      integer_t gesvd_rwork (double, integer_t, integer_t) { return 1; }
      inline
      integer_t gesvd_rwork (traits::complex_f, integer_t m, integer_t n) {
        return 5 * (m < n ? m : n);
      }
      inline
      integer_t gesvd_rwork (traits::complex_d, integer_t m, integer_t n) {
        return 5 * (m < n ? m : n);
      }

    } // detail


    template <typename MatrA>
    integer_t gesvd_work (char const q,
                    char const jobu, char const jobvt, MatrA const& a)
    {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
#endif

#ifdef BOOST_NUMERIC_BINDINGS_LAPACK_2
      assert (q == 'M');
#else
      assert (q == 'M' || q == 'O');
#endif
      assert (jobu == 'N' || jobu == 'O' || jobu == 'A' || jobu == 'S');
      assert (jobvt == 'N' || jobvt == 'O' || jobvt == 'A' || jobvt == 'S');
      assert (!(jobu == 'O' && jobvt == 'O'));

      integer_t const m = traits::matrix_size1 (a);
      integer_t const n = traits::matrix_size2 (a);
      integer_t lw = -13;

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else
      typedef typename MatrA::value_type val_t;
#endif

      if (q == 'M')
        lw = detail::gesvd_min_work (val_t(), m, n);

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_2
      MatrA& a2 = const_cast<MatrA&> (a);
      if (q == 'O') {
        // traits::detail::array<val_t> w (0);
        val_t w;
        integer_t info;
        detail::gesvd (jobu, jobvt, m, n,
                       traits::matrix_storage (a2),
                       traits::leading_dimension (a2),
                       0, // traits::vector_storage (s),
                       0, // traits::matrix_storage (u),
                       m, // traits::leading_dimension (u),
                       0, // traits::matrix_storage (vt),
                       n, // traits::leading_dimension (vt),
                       &w, // traits::vector_storage (w),
                       -1, // traits::vector_size (w),
                       0, // traits::vector_storage (rw),
                       &info);
        assert (info == 0);
        lw = traits::detail::to_int (w);  // (w[0]);
      }
#endif

      return lw;
    }


    template <typename MatrA>
    integer_t gesvd_rwork (MatrA const& a) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
#endif

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else 
      typedef typename MatrA::value_type val_t;
#endif

      return detail::gesvd_rwork (val_t(),
                                  traits::matrix_size1 (a),
                                  traits::matrix_size2 (a));
    }


    template <typename MatrA, typename VecS,
              typename MatrU, typename MatrV, typename VecW>
    int gesvd (char const jobu, char const jobvt,
               MatrA& a, VecS& s, MatrU& u, MatrV& vt, VecW& w)
    {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrU>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrV>::matrix_structure,
        traits::general_t
      >::value));

      BOOST_STATIC_ASSERT(
        (boost::is_same<
          typename traits::matrix_traits<MatrA>::value_type, float
        >::value
        ||
        boost::is_same<
          typename traits::matrix_traits<MatrA>::value_type, double
        >::value));
#endif

      integer_t const m = traits::matrix_size1 (a);
      integer_t const n = traits::matrix_size2 (a);
#ifndef NDEBUG /* this variable is only used in assertions below */
      integer_t const minmn = m < n ? m : n;
#endif

      assert (minmn == traits::vector_size (s));
      assert (!(jobu == 'O' && jobvt == 'O'));
      assert ((jobu == 'N')
              || (jobu == 'O')
              || (jobu == 'A' && m == traits::matrix_size2 (u))
              || (jobu == 'S' && minmn == traits::matrix_size2 (u)));
      assert ((jobu == 'N' && traits::leading_dimension (u) >= 1)
              || (jobu == 'O' && traits::leading_dimension (u) >= 1)
              || (jobu == 'A' && traits::leading_dimension (u) >= m)
              || (jobu == 'S' && traits::leading_dimension (u) >= m));
      assert (n == traits::matrix_size2 (vt));
      assert ((jobvt == 'N' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'O' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'A' && traits::leading_dimension (vt) >= n)
              || (jobvt == 'S' && traits::leading_dimension (vt) >= minmn));
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else
      typedef typename MatrA::value_type val_t;
#endif 
      assert (traits::vector_size(w) >= detail::gesvd_min_work(val_t(),m,n));

      integer_t info;
      detail::gesvd (jobu, jobvt, m, n,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (s),
                     traits::matrix_storage (u),
                     traits::leading_dimension (u),
                     traits::matrix_storage (vt),
                     traits::leading_dimension (vt),
                     traits::vector_storage (w),
                     traits::vector_size (w),
                     0, // dummy argument
                     &info);
      return info;
    }


    template <typename MatrA, typename VecS,
              typename MatrU, typename MatrV, typename VecW, typename VecRW>
    int gesvd (char const jobu, char const jobvt,
               MatrA& a, VecS& s, MatrU& u, MatrV& vt, VecW& w, VecRW& rw)
    {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrU>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrV>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const m = traits::matrix_size1 (a);
      integer_t const n = traits::matrix_size2 (a);
#ifndef NDEBUG /* this variable is only used in assertions below */
      integer_t const minmn = m < n ? m : n;
#endif

      assert (minmn == traits::vector_size (s));
      assert (!(jobu == 'O' && jobvt == 'O'));
      assert ((jobu == 'N')
              || (jobu == 'O')
              || (jobu == 'A' && m == traits::matrix_size2 (u))
              || (jobu == 'S' && minmn == traits::matrix_size2 (u)));
      assert ((jobu == 'N' && traits::leading_dimension (u) >= 1)
              || (jobu == 'O' && traits::leading_dimension (u) >= 1)
              || (jobu == 'A' && traits::leading_dimension (u) >= m)
              || (jobu == 'S' && traits::leading_dimension (u) >= m));
      assert (n == traits::matrix_size2 (vt));
      assert ((jobvt == 'N' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'O' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'A' && traits::leading_dimension (vt) >= n)
              || (jobvt == 'S' && traits::leading_dimension (vt) >= minmn));
#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else
      typedef typename MatrA::value_type val_t;
#endif
      assert (traits::vector_size(w) >= detail::gesvd_min_work(val_t(),m,n));
      assert (traits::vector_size(rw) >= detail::gesvd_rwork(val_t(),m,n));

      integer_t info;
      detail::gesvd (jobu, jobvt, m, n,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (s),
                     traits::matrix_storage (u),
                     traits::leading_dimension (u),
                     traits::matrix_storage (vt),
                     traits::leading_dimension (vt),
                     traits::vector_storage (w),
                     traits::vector_size (w),
                     traits::vector_storage (rw),
                     &info);
      return info;
    }


    template <typename MatrA, typename VecS, typename MatrU, typename MatrV>
    int gesvd (char const opt, char const jobu, char const jobvt,
               MatrA& a, VecS& s, MatrU& u, MatrV& vt)
    {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrU>::matrix_structure,
        traits::general_t
      >::value));
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrV>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const m = traits::matrix_size1 (a);
      integer_t const n = traits::matrix_size2 (a);
#ifndef NDEBUG /* this variable is only used in assertions below */
      integer_t const minmn = m < n ? m : n;
#endif

      assert (minmn == traits::vector_size (s));
      assert (!(jobu == 'O' && jobvt == 'O'));
      assert ((jobu == 'N')
              || (jobu == 'O')
              || (jobu == 'A' && m == traits::matrix_size2 (u))
              || (jobu == 'S' && minmn == traits::matrix_size2 (u)));
      assert ((jobu == 'N' && traits::leading_dimension (u) >= 1)
              || (jobu == 'O' && traits::leading_dimension (u) >= 1)
              || (jobu == 'A' && traits::leading_dimension (u) >= m)
              || (jobu == 'S' && traits::leading_dimension (u) >= m));
      assert ((jobvt == 'N' || traits::matrix_size2(vt) == n)) ;
      assert ((jobvt == 'N' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'O' && traits::leading_dimension (vt) >= 1)
              || (jobvt == 'A' && traits::leading_dimension (vt) >= n)
              || (jobvt == 'S' && traits::leading_dimension (vt) >= minmn));

#ifdef BOOST_NUMERIC_BINDINGS_LAPACK_2
      assert (opt == 'M');
#else
      assert (opt == 'M' || opt == 'O');
#endif

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else
      typedef typename MatrA::value_type val_t;
#endif
      typedef typename traits::type_traits<val_t>::real_type real_t;

      integer_t const lw = gesvd_work (opt, jobu, jobvt, a);
      traits::detail::array<val_t> w (lw);
      if (!w.valid()) return -101;

      integer_t const lrw = gesvd_rwork (a);
      traits::detail::array<real_t> rw (lrw);
      if (!rw.valid()) return -102;

      integer_t info;
      detail::gesvd (jobu, jobvt, m, n,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (s),
                     traits::matrix_storage (u),
                     traits::leading_dimension (u),
                     traits::matrix_storage (vt),
                     traits::leading_dimension (vt),
                     traits::vector_storage (w),
                     traits::vector_size (w),
                     traits::vector_storage (rw),
                     &info);
      return info;
    }


#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_2

    template <typename MatrA, typename VecS, typename MatrU, typename MatrV>
    inline
    int gesvd (char const jobu, char const jobvt,
               MatrA& a, VecS& s, MatrU& u, MatrV& vt)
    {
      return gesvd ('O', jobu, jobvt, a, s, u, vt);
    }

    template <typename MatrA, typename VecS, typename MatrU, typename MatrV>
    inline
    int gesvd (MatrA& a, VecS& s, MatrU& u, MatrV& vt) {
      return gesvd ('O', 'S', 'S', a, s, u, vt);
    }

    template <typename MatrA, typename VecS> 
    int gesvd (MatrA& a, VecS& s) {

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK 
      BOOST_STATIC_ASSERT((boost::is_same<
        typename traits::matrix_traits<MatrA>::matrix_structure,
        traits::general_t
      >::value));
#endif

      integer_t const m = traits::matrix_size1 (a);
      integer_t const n = traits::matrix_size2 (a);
#ifndef NDEBUG /* this variable is only used in assertions below */
      integer_t const minmn = m < n ? m : n;
#endif

      assert (minmn == traits::vector_size (s));

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS
      typedef typename traits::matrix_traits<MatrA>::value_type val_t;
#else
      typedef typename MatrA::value_type val_t;
#endif
      typedef typename traits::type_traits<val_t>::real_type real_t;

      integer_t const lw = gesvd_work ('O', 'N', 'N', a);
      traits::detail::array<val_t> w (lw);
      if (!w.valid()) return -101;

      integer_t const lrw = gesvd_rwork (a);
      traits::detail::array<real_t> rw (lrw);
      if (!rw.valid()) return -102;

      integer_t info;
      detail::gesvd ('N', 'N', m, n,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (s),
                     0, // traits::matrix_storage (u),
                     1, // traits::leading_dimension (u),
                     0, // traits::matrix_storage (vt),
                     1, // traits::leading_dimension (vt),
                     traits::vector_storage (w),
                     traits::vector_size (w),
                     traits::vector_storage (rw),
                     &info);
      return info;
    }

#endif

  } // namespace lapack

}}}

#endif
