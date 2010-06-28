#include <fit++/FitFunction_impl.hh>
#include "impl.h"

namespace dStorm {
namespace gauss_2d_fitter {
namespace no_analysis {

template class CommonInfo<1, fitpp::Exponential2D::FreeForm>;
template class CommonInfo<1, fitpp::Exponential2D::FreeForm_NoCorrelation>;
template class CommonInfo<1, fitpp::Exponential2D::FixedForm>;

}
}
}
