#ifndef NONLINFIT_STATESPACE_H
#define NONLINFIT_STATESPACE_H

#include "nonlinfit/Lambda.h"
#include "nonlinfit/Evaluation.h"
#include "nonlinfit/AbstractMoveable.h"
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/size.hpp>

namespace nonlinfit {

/** Vector view on the variables of a Lambda.
 *  This class provides access to the variables of a Lambda with a vector
 *  perspective, i.e. by assigning each variable a place in a unitless 
 *  vector. */
template <typename Lambda_>
class VectorPosition : public AbstractMoveable<double>
{
    typedef Lambda_ Expression;
  public:
    typedef typename Lambda_::Variables Variables;
    static const int VariableCount = boost::mpl::size<Variables>::type::value;
    typedef double Number;
    typedef typename Evaluation<Number>::Vector Position;
    
  private:
    Expression& expression;
    struct get_variable;
    struct set_variable;
  public:
    VectorPosition( Expression& e ) : expression(e) {}
    /** Store the variable values of the Lambda in the provided vector. */
    void get_position( Position& ) const;
    /** Change the variable values of the Lambda to the provided values. */
    void set_position( const Position& );
};

}

#endif
