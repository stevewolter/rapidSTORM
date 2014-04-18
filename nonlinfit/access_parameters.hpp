#ifndef NONLINFIT_ACCESS_PARAMETERS_H
#define NONLINFIT_ACCESS_PARAMETERS_H

#include "nonlinfit/Lambda.h"
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/contains.hpp>
#include <stdexcept>
#include <cassert>

namespace nonlinfit {

/** This class is instantiated by expressions for unit-safe parameter access.
 *  It uses the curiously recurring template pattern. It expects an access()
 *  method in the parameter class taking every supported parameter as its
 *  argument and returning a non-const reference to the parameter value.
 **/
template <typename CRTP>
struct access_parameters {
    /** Get a writeable reference to the parameter value. */
    template <typename Parameter>
    double& operator()( Parameter p )
        { return static_cast<CRTP&>(*this).access(p); }
    /** Get read-only access to the parameter value. */
    template <typename Parameter>
    double operator()( Parameter p ) const
        { return const_cast<CRTP&>( static_cast<const CRTP&>(*this) )( p ); }

    /** Get writeable access to a runtime-selected instance of a
     *  templated parameter. \note Is only implemented for two-dimensional
     *  parameters. */
    template <template <int Dim> class Parameter>
    double& get( int dim ) {
        if ( dim == 0 ) return (*this)( Parameter<0>() );
        else if ( dim == 1 ) return (*this)( Parameter<1>() );
        else assert( false );
    }
    /** Get read-only access to a runtime-selected instance of a
     *  templated parameter. \note Is only implemented for two-dimensional
     *  parameters. */
    template <template <int Dim> class Parameter>
    double get( int dim ) const {
        if ( dim == 0 ) return (*this)( Parameter<0>() );
        else if ( dim == 1 ) return (*this)( Parameter<1>() );
        else throw std::logic_error("Unknown dimension");
    }

    /** Get read-only access to a vector of templated parameters. 
     *  \note Is only implemented for two-dimensional vectors. */
    template <template <int Dim> class Parameter>
    Eigen::Vector2d get() const {
        Eigen::Vector2d rv;
        rv[0] = (*this)( Parameter<0>() );
        rv[1] = (*this)( Parameter<1>() );
        return rv;
    }
};

}

#endif
