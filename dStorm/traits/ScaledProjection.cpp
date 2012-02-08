#include "ScaledProjection.h"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include "debug.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

Projection::SamplePosition 
ScaledProjection::point_in_sample_space_
    ( const SubpixelImagePosition& pos ) const
{
    return boost::units::from_value< si::length >( to_sample * units::value( pos ) );
}

units::quantity<units::si::area> 
ScaledProjection::pixel_size_
    ( const ImagePosition& ) const
{
    return (to_sample * Eigen::Vector2f::Constant(1)).prod() * si::meter * si::meter;
}

std::vector< Projection::MappedPoint >
ScaledProjection::cut_region_of_interest_( 
    const SamplePosition& center,
    const SamplePosition& radius ) const
{
    std::vector< MappedPoint > rv;
    Bounds bb = get_region_of_interest_( center, radius );
    ImagePosition pos;
    typedef ImagePosition::Scalar Pixel;
    rv.reserve( (value( bb[1] - bb[0] ).array() + 1).prod() );
    for (Pixel x = bb[0].x(); x <= bb[1].x(); x += 1 * camera::pixel) {
        pos.x() = x;
        for (Pixel y = bb[0].y(); y <= bb[1].y(); y += 1 * camera::pixel) {
            pos.y() = y;
            rv.push_back( MappedPoint( pos, pixel_in_sample_space( pos ) ) );
        }
    }
    return rv;
}

ScaledProjection::Bounds
ScaledProjection::get_region_of_interest_( 
    const SamplePosition& center,
    const SamplePosition& radius ) const
{
    /* Determine bounds of region of interest */
    DEBUG("Cutting region around center " << center.transpose() << " with upper bound " << upper_bound.transpose()
          << " and range " << radius.transpose());
    Bounds r;
    r[0] = from_value< camera::length >( ceil(to_image * value(center - radius)).cast<int>() );
    r[1] = from_value< camera::length >( floor(to_image * value(center + radius)).cast<int>() );
    return r;
}

ScaledProjection::ScaledProjection( 
        quantity<camera::resolution> x, 
        quantity<camera::resolution> y )
: to_sample( 1.0 / x.value() , 1.0 / y.value() ),
  to_image( to_sample.inverse() )
{
}

ScaledProjection::ImagePosition
ScaledProjection::nearest_point_in_image_space_
    ( const SamplePosition& pos ) const
{
    return from_value< camera::length >( round(
        to_image * value(pos).cast<float>()).cast<int>() );
}

Projection::SubpixelImagePosition
ScaledProjection::point_in_image_space( SamplePosition pos ) const
{
    return from_value< camera::length >(
        to_image * value(pos).cast<float>());
}

}
}
