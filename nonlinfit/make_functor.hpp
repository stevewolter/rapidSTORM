#ifndef NONLINFIT_BOOLEAN_METAFUNCTION_H
#define NONLINFIT_BOOLEAN_METAFUNCTION_H

namespace nonlinfit {

/** Converts a Boost.MPL lambda into a functor object. The resulting
 *  functor object will return the same results as the Lambda metafunction
 *  class, but is usable as a function object (such as std::less). */
template <class Lambda, class Result>
struct make_functor
{
    typedef Result result_type;
    template <typename Type>
    Result operator()( Type ) { return Lambda::template apply<Type>::type::value; }
};

}

#endif
