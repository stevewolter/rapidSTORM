#ifndef NONLINFIT_BOUNDFUNCTION_H
#define NONLINFIT_BOUNDFUNCTION_H

#include <boost/utility.hpp>
#include <Eigen/Core>
#include "nonlinfit/VectorPosition.hpp"

namespace nonlinfit {

/** Function extension to store a Function and instances of its Lambda, its 
 *  Data and a VectorPosition together.
 *  The BoundFunction will take care of the appropriate set_data() call for the
 *  function.
 */
template <typename Function>
class BoundFunction
: public AbstractFunction<typename Function::Number>,
  private boost::noncopyable {
    typedef typename Function::Data Data;
    typedef typename Function::Lambda Lambda;
    Function f;
    Lambda m;
    Data d;

  public:
    typedef Function bound_function_type;
    typedef Lambda expression_type;
    typedef Data data_type;
    typedef typename Evaluation<typename Function::Number>::Vector Position;

    /** Construct function, expression and data by default constructors. */
    BoundFunction();
    /** Construct function, expression and data by copy constructors. */
    BoundFunction( const Function&, const Lambda&, const Data& );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    expression_type& get_expression() { return m; }
    const expression_type& get_expression() const { return m; }
    data_type& get_data() { return d; }
    const data_type& get_data() const { return d; }
    void get_position( Position & p ) const
        { f.get_position(p); }
    void set_position( const Position & p ) 
        { f.set_position(p); }
    int variable_count() const { return f.variable_count(); }
    bool evaluate(Evaluation<typename Function::Number> & p) {
        return f.evaluate(p);
    }
};

}

#endif
