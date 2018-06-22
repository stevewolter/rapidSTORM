#ifndef NONLINFIT_FUNCTIONCONVERTER_H
#define NONLINFIT_FUNCTIONCONVERTER_H

#include "nonlinfit/fwd.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace nonlinfit {

template <class ToType, class FromType, 
    bool Trivial = boost::is_same< ToType, FromType >::value >
class FunctionConverter;

/** Derived Function instance to convert the numeric type of a function.
 *  A function of this class contains an object of the underlying Function
 *  type and exhibits the same behaviour, but converts the numeric types
 *  in positions and evaluations to the ToType class. If ToType is the same
 *  type as the underlying function's number type, the conversion is Trivial
 *  and a no-op. */
template <class ToType, class FromType, bool Trivial>
class FunctionConverter
: public AbstractFunction<ToType>
{
    AbstractFunction<FromType>& base;
    Evaluation<FromType> buffer;
  public:
    FunctionConverter( AbstractFunction<FromType>& a ) : base(a), buffer(base.variable_count()) {}

    int variable_count() const { return base.variable_count(); }
    bool evaluate( Evaluation<ToType>& d ) {
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

template <class ToType, class FromType>
class FunctionConverter<ToType, FromType, true>
: public AbstractFunction<ToType>
{
    AbstractFunction<FromType>& base;
  public:
    FunctionConverter( AbstractFunction<FromType>& a ) : base(a) {}
    int variable_count() const { return base.variable_count(); }
    bool evaluate( Evaluation<ToType>& d ) { return base.evaluate(d); }
};

}

#endif
