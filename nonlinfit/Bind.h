#ifndef NONLINFIT_FUNCTION_H
#define NONLINFIT_FUNCTION_H

#include "fwd.h"
#include "Lambda.h"

#include <boost/mpl/size.hpp>
#include <boost/mpl/copy_if.hpp>

namespace nonlinfit {

/** Lambda modifier that binds some parameters.
 *
 *  This class models a Lambda with some parameters being bound as
 *  constants, i.e. are removed from the variable list. The Bind 
 *  class retains all access methods from the base class, thus allowing
 *  to change the constants via the access operators.
 *
 *  \tparam Lambda     A valid lambda that acts whose variables will be bound.
 *  \tparam Assignment A Boost.MPL metafunction class that returns 
 *                     boost::mpl::true_ for variables and boost::mpl::false_
 *                     for constants.
 *  */
template <typename Lambda_, typename Assignment_>
struct Bind
: public Lambda_
{
    /** A MPL sequence of the variables. */
    typedef typename boost::mpl::copy_if< 
        typename Lambda_::Variables, 
        Assignment_ >::type
        Variables;

    Bind() {}
    Bind( const Lambda_& lambda ) : Lambda_(lambda) {}
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

/** Specialization of the general get_evaluator metafunction for binds.
 *  The evaluator for a Bind function is identical to the evaluator for
 *  the base Lambda. */
template <typename Lambda, typename Assignment, typename Tag>
struct get_evaluator< Bind<Lambda,Assignment>, Tag >
    : public get_evaluator< Lambda, Tag > {};

}

#endif
