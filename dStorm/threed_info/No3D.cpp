#include "No3D.h"
#include <boost/units/io.hpp>
#include <stdexcept>

namespace dStorm {
namespace threed_info {

SigmaDerivative No3D::get_sigma_deriv_( ZPosition ) const
{ 
    throw std::logic_error("Attempted to get dSigma/dZ for no-3D model"); 
}

std::ostream& No3D::print_( std::ostream& o ) const
{
    return o << "no 3D information with PSF width " << sigma * 2.35f;
}

}
}
