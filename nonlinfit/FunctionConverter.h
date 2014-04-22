#ifndef NONLINFIT_FUNCTIONCONVERTER_H
#define NONLINFIT_FUNCTIONCONVERTER_H

#include "nonlinfit/fwd.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace nonlinfit {

template <class ToType, class Function, 
    bool Trivial = boost::is_same< ToType, typename Function::Number >::value >
class FunctionConverter;

/** Derived Function instance to convert the numeric type of a function.
 *  A function of this class contains an object of the underlying Function
 *  type and exhibits the same behaviour, but converts the numeric types
 *  in positions and evaluations to the ToType class. If ToType is the same
 *  type as the underlying function's number type, the conversion is Trivial
 *  and a no-op. */
template <class ToType, class Function, bool Trivial>
class FunctionConverter
: public AbstractFunction<ToType>
{
    Function base;
  public:
    typedef typename Function::Lambda Lambda;
    typedef typename Function::Data Data;
    typedef Evaluation< ToType > Derivatives;
    typedef ToType Number;

    FunctionConverter( Lambda& a ) : base(a) {}

    void set_data( const Data& d ) { base.set_data(d); }
    int variable_count() const { return base.variable_count(); }
    bool evaluate( Derivatives& d ) {
        typename Function::Derivatives buffer(variable_count());
        if ( base.evaluate(buffer) ) {
            d.value = buffer.value;
            d.gradient = buffer.gradient.template cast<ToType>();
            d.hessian = buffer.hessian.template cast<ToType>();
            return true;
        } else {
            return false;
        }
    }
};

/** \cond */
template <class ToType, class Function>
struct FunctionConverter< ToType, Function, true >
: public Function
{
    FunctionConverter( typename Function::Lambda& a ) : Function(a) {}

};
/** \endcond */

}

#endif
