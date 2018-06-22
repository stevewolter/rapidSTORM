#include "nonlinfit/sum/Evaluator.hpp"
#include "gaussian_psf/parameters.h"
#include "guf/PlaneFunction.hpp"

namespace dStorm {
namespace guf {

template std::auto_ptr< nonlinfit::AbstractFunction<double> >
    PlaneFunction::create<InstantiatedFunction, InstantiatedTag>
    ( InstantiatedFunction&, const fit_window::Plane& data, bool );

}
}
