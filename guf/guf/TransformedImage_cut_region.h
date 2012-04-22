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
        get_region_of_interest( traits::Projection::ROISpecification(center, max_distance) );
    Bounds r;
    r.col(0) = bounds.lower_corner();
    r.col(1) = bounds.upper_corner();
    return r;
}

}
}

#endif
