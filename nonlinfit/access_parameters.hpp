#ifndef NONLINFIT_ACCESS_PARAMETERS_H
#define NONLINFIT_ACCESS_PARAMETERS_H

#include "Lambda.h"
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/contains.hpp>
#include <stdexcept>
#include <cassert>
#include <boost/units/quantity.hpp>

namespace nonlinfit {

/** This class realizes write access to a parameter's value. It is used
 *  by access_parameters to add or remove Boost.Units tags as needed. */
template <typename Parameter>
class ParameterReference {
    double &_value;
    typedef boost::units::quantity< typename Parameter::Unit > value_type;
  public:
    ParameterReference( double& v ) : _value(v) {}
    
    /** Get the parameter value as a boost::units::quantity. */
    operator const value_type () const { return value_type::from_value(_value);}
    /** Get the parameter value as a boost::units::quantity. */
    const value_type operator*() const { return value_type::from_value(_value);}
    /** Set the parameter value from a boost::units::quantity. */
    template <typename Type>
    ParameterReference& operator=( const Type v ) 
        { _value = value_type(v).value(); return *this; }
    /** Set the parameter value from a unitless scalar. The user of this 
     *  method must make sure that the units are correct. */
    void set_value( const double v ) { _value = v; }
    /** Get the parameter value as a unitless scalar.
     *  This method is provided for compatibility to the boost::units::quantity
     *  interface. */
    double value() const { return _value; }
};

/** This class is instantiated by expressions for unit-safe parameter access.
 *  It uses the curiously recurring template pattern. It expects an access()
 *  method in the parameter class taking every supported parameter as its
 *  argument and returning a non-const reference to the parameter value.
 **/
template <typename CRTP>
struct access_parameters {
    /** Get a writeable reference to the parameter value. */
    template <typename Parameter>
    ParameterReference<Parameter> operator()( Parameter p ) 
        { return ParameterReference<Parameter>( 
            static_cast<CRTP&>(*this).access(p) ); }
    /** Get read-only access to the parameter value. */
    template <typename Parameter>
    boost::units::quantity<typename Parameter::Unit> 
    operator()( Parameter p ) const
        { return const_cast<CRTP&>( static_cast<const CRTP&>(*this) )( p ); }

    /** Get writeable access to a runtime-selected instance of a
     *  templated parameter. \note Is only implemented for two-dimensional
     *  parameters. */
    template <template <int Dim> class Parameter>
    ParameterReference< Parameter<0> > get( int dim ) {
        if ( dim == 0 ) return (*this)( Parameter<0>() );
        else if ( dim == 1 ) return (*this)( Parameter<1>() );
        else assert( false );
    }
    /** Get read-only access to a runtime-selected instance of a
     *  templated parameter. \note Is only implemented for two-dimensional
     *  parameters. */
    template <template <int Dim> class Parameter>
    boost::units::quantity<typename Parameter<0>::Unit> 
    get( int dim ) const {
        if ( dim == 0 ) return (*this)( Parameter<0>() );
        else if ( dim == 1 ) return (*this)( Parameter<1>() );
        else throw std::logic_error("Unknown dimension");
    }

    /** Get read-only access to a vector of templated parameters. 
     *  \note Is only implemented for two-dimensional vectors. */
    template <template <int Dim> class Parameter>
    Eigen::Matrix< boost::units::quantity<typename Parameter<0>::Unit>, 2, 1 > 
    get() const {
        Eigen::Matrix< boost::units::quantity<typename Parameter<0>::Unit>, 2, 1 > rv;
        rv[0] = (*this)( Parameter<0>() );
        rv[1] = (*this)( Parameter<1>() );
        return rv;
    }
};

}

#endif
