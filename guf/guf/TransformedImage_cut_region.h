#ifndef NONLINFIT_IMAGE_CUT_REGION_H
#define NONLINFIT_IMAGE_CUT_REGION_H

#include "TransformedImage.h"
#include "debug.h"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace guf {

using namespace boost::units;

template <typename LengthUnit>
TransformedImage<LengthUnit>
    ::TransformedImage( const Spot& max_distance, const engine::InputPlane& optics )
: optics(optics), max_distance(max_distance)
{
    DEBUG("Max distance is set to " << max_distance.transpose());
}

template <typename LengthUnit>
typename TransformedImage<LengthUnit>::Bounds
TransformedImage<LengthUnit>::cut_region( 
    const Spot& center, const ImageSize& upper_bound ) const
{
    DEBUG("Cutting region around center " << center.transpose() << " with upper bound " << upper_bound.transpose()
          << " and range " << max_distance.transpose());

    traits::Projection::Bounds bounds = optics.projection().
        get_region_of_interest( traits::Projection::ROISpecification(center.head<2>(), max_distance.head<2>()) );
    Bounds r;
    for (int i = 0; i < 2; ++i) 
        r.col(i) = bounds[i].array().min( upper_bound.array() ).
            max( traits::Projection::ImagePosition::Constant(0 * camera::pixel).array() );
    return r;
}

}
}

#endif
