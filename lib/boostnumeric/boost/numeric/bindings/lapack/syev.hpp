/*
 *
 * Copyright (c) Toon Knapen, Karl Meerbergen & Kresimir Fresl 2003
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * KF acknowledges the support of the Faculty of Civil Engineering,
 * University of Zagreb, Croatia.
 *
 */

#ifndef BOOST_NUMERIC_BINDINGS_LAPACK_SYEV_HPP
#define BOOST_NUMERIC_BINDINGS_LAPACK_SYEV_HPP

#include <boost/numeric/bindings/traits/traits.hpp>
#include <boost/numeric/bindings/lapack/lapack.h>
#include <boost/numeric/bindings/lapack/workspace.hpp>
#include <boost/numeric/bindings/traits/detail/array.hpp>

#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
#  include <boost/static_assert.hpp>
#  include <boost/type_traits.hpp>
#endif

#include <cassert>


namespace boost { namespace numeric { namespace bindings {

  namespace lapack {

    ///////////////////////////////////////////////////////////////////
    //
    // Eigendecomposition of a real symmetric matrix A = Q * D * Q'
    //
    ///////////////////////////////////////////////////////////////////

    /*
     * syev() computes the eigendecomposition of a N x N matrix
     * A = Q * D * Q',  where Q is a N x N orthogonal matrix and
     * D is a diagonal matrix. The diagonal elements D(i,i) is an
     * eigenvalue of A and Q(:,i) is a corresponding eigenvector.
     *
     * On return of syev, A is overwritten by Q and w contains the main
     * diagonal of D.
     *
     * int syev (char jobz, char uplo, A& a, W& w, minimal_workspace ) ;
     *    jobz : 'V' : compute eigenvectors
     *           'N' : do not compute eigenvectors
     *    uplo : 'U' : only the upper triangular part of A is used on input.
     *           'L' : only the lower triangular part of A is used on input.
     */

    namespace detail {

      inline
      void syev (char const jobz, char const uplo, integer_t const n,
                 float* a, integer_t const lda,
                 float* w, float* work, integer_t const lwork, integer_t& info)
      {
        LAPACK_SSYEV (&jobz, &uplo, &n, a, &lda, w, work, &lwork, &info);
      }

      inline
      void syev (char const jobz, char const uplo, integer_t const n,
                 double* a, integer_t const lda,
                 double* w, double* work, integer_t const lwork, integer_t& info)
      {
        LAPACK_DSYEV (&jobz, &uplo, &n, a, &lda, w, work, &lwork, &info);
      }


      template <typename A, typename W, typename Work>
      int syev (char jobz, char uplo, A& a, W& w, Work& work) {

/*#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
        BOOST_STATIC_ASSERT((boost::is_same<
          typename traits::matrix_traits<A>::matrix_structure,
          traits::general_t
        >::value));
#endif*/

        integer_t const n = traits::matrix_size1 (a);
        assert ( n>0 );
        assert (traits::matrix_size2 (a)==n);
        assert (traits::leading_dimension (a)>=n);
        assert (traits::vector_size (w)==n);
        assert (3*n-1 <= traits::vector_size (work));
        assert ( uplo=='U' || uplo=='L' );
        assert ( jobz=='N' || jobz=='V' );

        integer_t info;
        detail::syev (jobz, uplo, n,
                     traits::matrix_storage (a),
                     traits::leading_dimension (a),
                     traits::vector_storage (w),
                     traits::vector_storage (work),
                     traits::vector_size (work),
                     info);
        return info;
      }
    }  // namespace detail


    // Function that allocates work arrays
    template <typename A, typename W>
    int syev (char jobz, char uplo, A& a, W& w, optimal_workspace ) {
       typedef typename A::value_type value_type ;

       std::ptrdiff_t const n = traits::matrix_size1 (a);

       traits::detail::array<value_type> work( std::max<std::ptrdiff_t>(1,34*n) );
       return detail::syev(jobz, uplo, a, w, work);
    } // syev()


    // Function that allocates work arrays
    template <typename A, typename W>
    int syev (char jobz, char uplo, A& a, W& w, minimal_workspace ) {
       typedef typename A::value_type value_type ;

       std::ptrdiff_t const n = traits::matrix_size1 (a);

       traits::detail::array<value_type> work( std::max<std::ptrdiff_t>(1,3*n-1) );
       return detail::syev(jobz, uplo, a, w, work);
    } // syev()


    // Function that allocates work arrays
    template <typename A, typename W, typename Work>
    int syev (char jobz, char uplo, A& a, W& w, detail::workspace1<Work> workspace ) {
       typedef typename traits::matrix_traits<A>::value_type value_type ;

       return detail::syev(jobz, uplo, a, w, workspace.select(value_type()));
    } // syev()

    // Function without workarray as argument
    template <typename A, typename W>
    inline
    int syev (char jobz, char uplo, A& a, W& w) {
       return syev(jobz, uplo, a, w, optimal_workspace());
    } // syev()

    //
    // With UPLO integrated in matrix type (this is not possible
    // since a contains the eigenvectors afterwards and thus A cannot be symmetric)
    //
    template <typename A, typename W>
    int syev (char jobz, A& a, W& w, optimal_workspace ) {
       typedef typename A::value_type value_type ;

       std::ptrdiff_t const n = traits::matrix_size1 (a);
       char uplo = traits::matrix_uplo_tag( a ) ;
/*#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
       typedef typename traits::matrix_traits<A>::matrix_structure matrix_structure ;
       BOOST_STATIC_ASSERT( (boost::mpl::or_< boost::is_same< matrix_structure, traits::symmetric_t >
                                            , boost::is_same< matrix_structure, traits::hermitian_t >
                                            >::value)
                          ) ;
#endif*/

       traits::detail::array<value_type> work( std::max<std::ptrdiff_t>(1,34*n) );
       return detail::syev(jobz, uplo, a, w, work);
    } // syev()


    // Function that allocates work arrays
    template <typename A, typename W>
    int syev (char jobz, A& a, W& w, minimal_workspace ) {
       typedef typename A::value_type value_type ;

       std::ptrdiff_t const n = traits::matrix_size1 (a);
       char uplo = traits::matrix_uplo_tag( a ) ;
/*#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
       typedef typename traits::matrix_traits<A>::matrix_structure matrix_structure ;
       BOOST_STATIC_ASSERT( (boost::mpl::or_< boost::is_same< matrix_structure, traits::symmetric_t >
                                            , boost::is_same< matrix_structure, traits::hermitian_t >
                                            >::value)
                          ) ;
#endif*/
       traits::detail::array<value_type> work( std::max<std::ptrdiff_t>(1,3*n-1) );
       return detail::syev(jobz, uplo, a, w, work);
    } // syev()


    // Function that allocates work arrays
    template <typename A, typename W, typename Work>
    int syev (char jobz, A& a, W& w, detail::workspace1<Work> workspace ) {
       typedef typename A::value_type value_type ;
       char uplo = traits::matrix_uplo_tag( a ) ;
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
       typedef typename traits::matrix_traits<A>::matrix_structure matrix_structure ;
       BOOST_STATIC_ASSERT( (boost::mpl::or_< boost::is_same< matrix_structure, traits::symmetric_t >
                                            , boost::is_same< matrix_structure, traits::hermitian_t >
                                            >::value)
                          ) ;
#endif
       return detail::syev(jobz, uplo, a, w, workspace.select(value_type()));
    } // syev()

    // Function without workarray as argument
    template <typename A, typename W>
    inline
    int syev (char jobz, A& a, W& w) {
       char uplo = traits::matrix_uplo_tag( a ) ;
#ifndef BOOST_NUMERIC_BINDINGS_NO_STRUCTURE_CHECK
       typedef typename traits::matrix_traits<A>::matrix_structure matrix_structure ;
       BOOST_STATIC_ASSERT( (boost::mpl::or_< boost::is_same< matrix_structure, traits::symmetric_t >
                                            , boost::is_same< matrix_structure, traits::hermitian_t >
                                            >::value)
                          ) ;
#endif
       return syev(jobz, uplo, a, w, optimal_workspace());
    } // syev()

  }



}}}

#endif
