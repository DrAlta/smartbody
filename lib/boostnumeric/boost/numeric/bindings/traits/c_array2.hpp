/*
 * 
 * Copyright (c) 2002, 2003 Kresimir Fresl, Toon Knapen and Karl Meerbergen
 * Copyright (c) 2008 Markus Rickert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * KF acknowledges the support of the Faculty of Civil Engineering, 
 * University of Zagreb, Croatia.
 *
 */

#ifndef BOOST_NUMERIC_BINDINGS_TRAITS_C_ARRAY2_H
#define BOOST_NUMERIC_BINDINGS_TRAITS_C_ARRAY2_H

#include <boost/numeric/bindings/traits/config.hpp> 

#ifndef BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS 

#include <boost/numeric/bindings/traits/c_array.hpp>
#include <boost/numeric/bindings/traits/matrix_traits.hpp>

namespace boost { namespace numeric { namespace bindings { namespace traits {

  // built-in array as matrix (nx1)
  template <typename T, std::size_t N, typename V>
  struct matrix_detail_traits<T[N], V> 
  {
#ifndef BOOST_NUMERIC_BINDINGS_NO_SANITY_CHECK
    BOOST_STATIC_ASSERT( 
      (boost::is_same< 
         T[N], 
         typename boost::remove_const<V>::type 
       >::value) );
#endif

    typedef T identifier_type [N];
    typedef V matrix_type; 
    typedef general_t matrix_structure; 
    typedef column_major_t ordering_type; 

    typedef T value_type; 
    typedef typename default_vector_traits< V, T >::pointer pointer; 

    static pointer storage (matrix_type& v) {
      return vector_traits<matrix_type>::storage (v); 
    }
    static std::ptrdiff_t num_rows (matrix_type& v) { return N; }
    static std::ptrdiff_t num_columns (matrix_type&) { return 1; }
    static std::ptrdiff_t storage_size (matrix_type& v) { return N; }
//    static std::ptrdiff_t stride1 (matrix_type& v) { return vector_traits<V>::stride (v); }
//    static std::ptrdiff_t stride2 (matrix_type&) { return 1; }
    static std::ptrdiff_t leading_dimension (matrix_type& v) { return N; }
  }; 

}}}}

#else // BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS 

#error with your compiler plain C array cannot be used in bindings 

#endif // BOOST_NUMERIC_BINDINGS_POOR_MANS_TRAITS 

#endif // BOOST_NUMERIC_BINDINGS_TRAITS_C_ARRAY2_H
