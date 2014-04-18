#ifndef NONLINFIT_FUNCTIORS_MAKE_BITSET_H
#define NONLINFIT_FUNCTIORS_MAKE_BITSET_H

#include <bitset>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/bind/bind.hpp>
#include "nonlinfit/index_of.h"

namespace nonlinfit {

namespace detail {

struct set_vector {
    typedef void result_type;
    template <typename BaseFunctor, typename IndexFunctor, typename Vector, typename Parameter>
    void operator()( BaseFunctor b, IndexFunctor i, Vector& v, Parameter p ) {
        v[ IndexFunctor::template apply<Parameter>::type::value ] = b(p);
    }
};

}

/** This function converts the boolean metafunction Functor to a bitset.
 *  \tparam Things Support of the metafunction. The elements of this sequence
 *                 correspond to the bits in the resulting bitset, with each
 *                 bit indicating the truth result of the metafunction's 
 *                 evaluation.
 **/
template <typename Things, typename Functor>
std::bitset< boost::mpl::size<Things>::type::value >
make_bitset( Things, Functor f ) {
    std::bitset< boost::mpl::size<Things>::type::value > rv;
    boost::mpl::for_each< Things >( 
        boost::bind( 
            detail::set_vector(), 
            f, 
            typename boost::mpl::lambda< index_of<Things,boost::mpl::_1> >::type(),
            boost::ref(rv),
            _1 ) );
    return rv;
}

}

#endif
