#ifndef DSTORM_TRAITS_BASE_APPLY_H
#define DSTORM_TRAITS_BASE_APPLY_H

#include "base.h"

namespace dStorm {
namespace traits {

template <int Rows, int Cols>
template <class Functor>
typename MatrixOrScalar<Rows, Cols>::template value<typename Functor::result_type>::type
MatrixOrScalar<Rows, Cols>::apply(
    Functor& f, 
    typename value<typename Functor::argument_type>::type& t 
) {
    typename boost::remove_const< typename value<typename Functor::result_type>::type >::type rv;
    for (int r = 0; r < t.rows(); ++r)
        for (int c = 0; c < t.cols(); ++c)
            rv(r,c) = f(t(r,c));
    return rv;
}

template <int Rows, int Cols>
template <class Functor>
typename MatrixOrScalar<Rows, Cols>::template value<typename Functor::result_type>::type
MatrixOrScalar<Rows, Cols>::apply(
    Functor& f, 
    typename value<typename Functor::first_argument_type>::type& t1,
    typename value<typename Functor::second_argument_type>::type& t2
) {
    typename boost::remove_const< typename value<typename Functor::result_type>::type >::type rv;
    for (int r = 0; r < t1.rows(); ++r)
        for (int c = 0; c < t1.cols(); ++c)
            rv(r,c) = f(t1(r,c), t2(r,c));
    return rv;
}

}
}

#endif
