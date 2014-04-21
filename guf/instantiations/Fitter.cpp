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
Fitter::fit<double>(
    AbstractFunction<double>&,
    AbstractMoveable<double> >&,
    AbstractTerminator<double> >&
);

}
}
