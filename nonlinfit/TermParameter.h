#ifndef NONLINFIT_TERMPARAMETER_H
#define NONLINFIT_TERMPARAMETER_H

#include <iostream>

namespace nonlinfit {

/** A TermParameter identifies one parameter in a function set.
 *  It is a parameter tag for use for aggregate Expression objects
 *  such as nonlinfit::sum::Expression, which use it to distinguish
 *  between same-typed parameters in multiple contributing
 *  expressions.
 *
 *  \tparam Func Index of the contributing expression for this parameter
 *  \tparam BaseParameter Tag of this parameter in the contributing 
 *                        expression.
 */
template <typename Func, typename BaseParameter>
struct TermParameter {
    typedef Func Function;
    typedef BaseParameter base;
    typedef typename BaseParameter::Unit Unit;
};

template <typename Func, typename BaseParameter>
inline std::ostream& operator<<(std::ostream& o, TermParameter<Func,BaseParameter>)  { 
    return (o << "summand " << Func::value << ", " << BaseParameter()); 
}

}

#endif
