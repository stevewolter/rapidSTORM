#include <nonlinfit/sum/Evaluator.hpp>
#include "guf/psf/parameters.h"
#include "guf/guf/PlaneFunction.hpp"

namespace dStorm {
namespace guf {

template std::auto_ptr< PlaneFunction<InstantiatedFunction> >
    PlaneFunction<InstantiatedFunction>::create<InstantiatedTag>
    ( InstantiatedFunction&, InstantiatedTag );

}
}
