#include <boost/mpl/size.hpp>
#include <nonlinfit/levmar/Fitter.hpp>
#include <nonlinfit/AbstractFunction.h>
#include <nonlinfit/AbstractMoveable.h>
#include <nonlinfit/AbstractTerminator.h>
#include <nonlinfit/sum/AbstractFunction.h>
#include <boost/ref.hpp>

namespace nonlinfit {
namespace levmar {

#define INSTANTIATE_FITTER(width) \
template double \
Fitter::fit<  \
    AbstractFunction< Evaluation<double, Eigen::Dynamic, 9*width> >, \
    AbstractMoveable< Eigen::Matrix<double, Eigen::Dynamic, 1, 0, 9*width, 1> >, \
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0, 9*width, 1> >& > \
(  \
    AbstractFunction< Evaluation<double, Eigen::Dynamic, 9*width> >&, \
    AbstractMoveable< Eigen::Matrix<double, Eigen::Dynamic, 1, 0, 9*width, 1> >&, \
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0, 9*width, 1> >& \
);

INSTANTIATE_FITTER(4)
INSTANTIATE_FITTER(5)
INSTANTIATE_FITTER(6)
INSTANTIATE_FITTER(7)
INSTANTIATE_FITTER(9)
}
}
