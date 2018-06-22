#ifndef NONLINFIT_ABSTRACTFUNCTION_H
#define NONLINFIT_ABSTRACTFUNCTION_H

#include "nonlinfit/fwd.h"
#include <Eigen/Core>
#include <boost/static_assert.hpp>
#include "nonlinfit/Evaluation.h"

namespace nonlinfit {

/** \brief Polymorphic abstraction of a Function instantiation.
 *
 *  This class provides a lambda-agnostic view on a Function object,
 *  defining its interface purely in terms of the anonymous variables.
 *
 *  \tparam Vars gives the number of variables of this function, 
 *          or Eigen::Dynamic if this number is runtime-dynamic
 *  \tparam MaxVarCount gives an upper bound on the number of variables
 *          if Vars is set to Eigen::Dynamic
 **/
template <typename Number>
class AbstractFunction {
  public:
    /** \copydoc nonlinfit::Evaluation */
    typedef Evaluation<Number> Derivatives;

    virtual ~AbstractFunction() {}
    /** The runtime number of parameters. Equal to Vars for non-dynamic 
     *  functions and always bounded by MaxVarCount. */
    virtual int variable_count() const = 0;
    /** Compute the value of the function at the position set by
     *  the last call to set_position(). This function can be called before
     *  set_position() and will return the result at an initial state.
     *
     *  \param[out] p Buffer to be initialized to function value.
     *  \return True iff the current variable configuration is within the
     *               function's support. */
    virtual bool evaluate( Derivatives& p ) = 0;

};

}

#endif
