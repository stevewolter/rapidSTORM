#ifndef NONLINFIT_STATESPACE_H
#define NONLINFIT_STATESPACE_H

#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/size.hpp>

#include "nonlinfit/AbstractMoveable.h"
#include "nonlinfit/Evaluation.h"
#include "nonlinfit/Lambda.h"

namespace nonlinfit {

/** Vector view on the variables of a Lambda.
 *  This class provides access to the variables of a Lambda with a vector
 *  perspective, i.e. by assigning each variable a place in a unitless 
 *  vector. */
template <typename Lambda_>
class VectorPosition : public AbstractMoveable<double>
{
  protected:
    typedef typename Lambda_::Variables Variables;
    static const int VariableCount = boost::mpl::size<Variables>::type::value;
    
  private:
    Lambda_& expression;
    struct get_variable;
    struct set_variable;
  public:
    VectorPosition( Lambda_& e ) : expression(e) {}

    int variable_count() const OVERRIDE { return VariableCount; }

    /** Store the variable values of the Lambda in the provided vector. */
    void get_position( Position& ) const;
    /** Change the variable values of the Lambda to the provided values. */
    void set_position( const Position& );
};

}

#endif
