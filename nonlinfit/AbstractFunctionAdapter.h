#ifndef NONLINFIT_ABSTRACT_FUNCTION_ADAPTER_H
#define NONLINFIT_ABSTRACT_FUNCTION_ADAPTER_H

#include "fwd.h"
#include "AbstractFunction.h"
#include <Eigen/Core>

namespace nonlinfit {

/** Type-erasure adapter for Function classes implementing AbstractFunction.
 *  
 *  This template takes a model of Function and uses its methods to implement
 *  the interface of AbstractFunction. */
template <typename Function>
class AbstractFunctionAdapter
: public AbstractFunction< typename Function::Derivatives >
{
    Function& fi;
  public:

    AbstractFunctionAdapter( Function& f ) : fi(f) {}
    bool evaluate( typename Function::Derivatives& p ) { return fi.evaluate(p); }
    int variable_count() const { return fi.variable_count(); }
    typedef AbstractFunction< typename Function::Derivatives > abstract_type;
    abstract_type& abstract() { return *this; }
};

template <typename Function>
struct AbstractedFunction 
: public Function,
  public AbstractFunctionAdapter<Function>
{
  public:
    AbstractedFunction() : AbstractFunctionAdapter<Function>( static_cast<Function&>(*this) ) {}
    AbstractedFunction( typename Function::Lambda& l ) 
        : Function(l), AbstractFunctionAdapter<Function>( static_cast<Function&>(*this) ) {}

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
    
#endif
