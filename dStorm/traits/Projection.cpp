#include "Projection.h"
#include <boost/units/Eigen/Array>

namespace dStorm {
namespace traits {

namespace si = boost::units::si;
namespace camera = boost::units::camera;

units::quantity<units::si::area> 
Projection::pixel_size_
    ( const ImagePosition& at ) const
{
    SamplePosition 
        upper_left = pixel_in_sample_space( ImagePosition(at.array()-1*camera::pixel) ),
        lower_right = pixel_in_sample_space( ImagePosition(at.array()+1*camera::pixel) );
    return (value(lower_right - upper_left).array() / 2).prod() * si::meter * si::meter;
}

}
}
