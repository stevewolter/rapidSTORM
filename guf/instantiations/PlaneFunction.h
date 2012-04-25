#include <nonlinfit/sum/Evaluator.hpp>
#include "gaussian_psf/parameters.h"
#include "guf/PlaneFunction.hpp"

namespace dStorm {
namespace guf {

template std::auto_ptr< PlaneFunction<InstantiatedFunction> >
    PlaneFunction<InstantiatedFunction>::create<InstantiatedTag>
    ( InstantiatedFunction&, InstantiatedTag );

}
}
