#ifndef GUF_EVALUATOR_FACTORY_H
#define GUF_EVALUATOR_FACTORY_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <memory>
#include <boost/utility.hpp>
#include "PlaneFunction.h"
#include <nonlinfit/VectorPosition.h>
#include <nonlinfit/AbstractMoveable.h>

namespace dStorm {
namespace guf {

/** Generator for an evaluating function for each tag in the instantiation 
 *  schedule. 
 *  This class is responsible for generating and storing one instance of 
 *  nonlinfit::plane::Distance per tag in the instantiation schedule. It also stores an instance of the function's
 *  base expression shared between all functions, which can be accessed by 
 *  get_expression(). */
template <class Lambda>
class FunctionRepository
: public boost::noncopyable
{
  public:
    typedef typename nonlinfit::get_abstract_moveable_from_lambda< Lambda, double >::type Mover;
  private:
    class instantiate;

    /** The expression is dynamically allocated to avoid Eigen alignment trouble. */
    std::auto_ptr<Lambda> expression;
    boost::ptr_vector< PlaneFunction<Lambda> > store;
    std::auto_ptr<Mover> mover;

  public:
    FunctionRepository();
    ~FunctionRepository();
    typedef typename PlaneFunction<Lambda>::abstraction
        result_type;
    /** Return an abstract function with the expression set to the result of
     *  get_expression() and the data to the supplied data. If \c mle is true,
     *  the function will be an instance of 
     *  nonlinfit::plane::InversePoissonLikelihood, and of 
     *  nonlinfit::plane::SquaredDeviations otherwise.
     **/
    result_type* operator()( const DataPlane&, bool mle );

    /** Return a reference to the expression shared by all functions in the
     *  repository. */
    Lambda& get_expression() { return *expression; }
    Mover& get_moveable() { return *mover; }
};

}
}

#endif
