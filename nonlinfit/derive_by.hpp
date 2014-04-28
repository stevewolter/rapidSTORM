#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DERIVER_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DERIVER_H

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <nonlinfit/index_of.h>
#include <boost/bind/bind.hpp>
#include <Eigen/Core>

namespace nonlinfit {

/** Compute the Jacobian matrix.
 *
 *  \tparam _Variables The list of all variables to be supported by the functor.
 *                    Indices in the result vector will be computed relative to
 *                    the Variables list. */
template <typename Variables>
class Jacobian {
    struct jacobian_maker {
        typedef void result_type;
        template <typename Result, typename Evaluator, typename Parameter>
        void operator()( Result& r, Evaluator& cp, Parameter p ) { 
            static const int index = index_of<Variables,Parameter>::value;
            cp.derivative( r.col( index ), Parameter() );
        }
    };
  public:
    /** Compute the Jacobian matrix using the given Evaluator. All 
     *  elements will be computed eagerly and stored in the internal
     *  result matrix that can be accessed via jacobian(). */
    template <typename Evaluator, typename Result>
    static void compute( Evaluator& cp, Result& r ) {
        boost::mpl::for_each< Variables >( boost::bind(
            jacobian_maker(), boost::ref(r), boost::ref(cp), _1 ) );
    }
};

}

#endif
