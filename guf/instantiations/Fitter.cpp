#include <boost/mpl/size.hpp>
#include <nonlinfit/levmar/Fitter.hpp>
#include <nonlinfit/AbstractFunction.h>
#include <nonlinfit/AbstractTerminator.h>
#include <nonlinfit/sum/AbstractFunction.h>
#include <boost/ref.hpp>

namespace nonlinfit {
namespace levmar {

template double
Fitter::fit< 
    AbstractFunction< double >,
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0> >& >
( 
    AbstractFunction< double >&,
    AbstractTerminator<Eigen::Matrix<double, Eigen::Dynamic, 1, 0> >&
);

}
}
