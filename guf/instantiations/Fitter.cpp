#include <boost/mpl/size.hpp>
#include <nonlinfit/levmar/Fitter.hpp>
#include <nonlinfit/AbstractFunction.h>
#include <nonlinfit/AbstractMoveable.h>
#include <nonlinfit/AbstractTerminator.h>
#include <nonlinfit/sum/AbstractFunction.h>
#include <boost/ref.hpp>

namespace nonlinfit {
namespace levmar {

template double
Fitter::fit< 
    AbstractFunction< Evaluation<double> >,
    AbstractMoveable< double >,
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0> >& >
( 
    AbstractFunction< Evaluation<double> >&,
    AbstractMoveable< double >&,
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0> >&
);

}
}
