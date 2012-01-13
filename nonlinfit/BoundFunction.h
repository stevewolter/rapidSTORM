#ifndef NONLINFIT_BOUNDFUNCTION_H
#define NONLINFIT_BOUNDFUNCTION_H

#include <boost/utility.hpp>
#include <Eigen/Core>
#include "VectorPosition.hpp"

namespace nonlinfit {

/** Function extension to store a Function and instances of its Lambda, its 
 *  Data and a VectorPosition together.
 *  The BoundFunction will take care of the appropriate set_data() call for the
 *  function.
 */
template <typename Function>
class BoundFunction : public Function, private boost::noncopyable {
    typedef typename Function::Data Data;
    typedef typename Function::Lambda Lambda;
    Lambda m;
    Data d;
    typedef VectorPosition<Lambda> Mover;
    Mover mover;

  public:
    typedef Function bound_function_type;
    typedef Lambda expression_type;
    typedef Data data_type;
    typedef typename Mover::Position Position;

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
        { mover.get_position(p); }
    void set_position( const Position & p ) 
        { mover.set_position(p); }
};

}

#endif
