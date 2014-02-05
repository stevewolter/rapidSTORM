#include "dStorm/traits/Projection.h"
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

bool Projection::ROISpecification::contains( const SamplePosition& p ) const
{
    return ( (p.array() >= (center - width).array()).all() && 
             (p.array() <= (center + width).array()).all() );
}

Projection::ROISpecification::ROISpecification( const SamplePosition& center_, 
                          const SamplePosition& width_ )
: center(center_), width(width_) {}

Projection::ROISpecification::ROISpecification( const engine::FitPosition& center_, 
                          const engine::FitPosition& width_ ) {
    for (int i = 0; i < 2; ++i) {
        center[i] = center_[i] * 1E-6f * si::meter;
        width[i] = width_[i] * 1E-6f * si::meter;
    }
}

}
}
