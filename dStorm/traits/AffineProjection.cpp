#include "AffineProjection.h"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include "debug.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

Projection::SamplePosition 
AffineProjection::point_in_sample_space_
    ( const SubpixelImagePosition& pos ) const
{
    return boost::units::from_value< si::length >( to_sample * units::value( pos ) );
}

units::quantity<units::si::area> 
AffineProjection::pixel_size_
    ( const ImagePosition& at ) const
{
    SamplePosition 
        upper_left = point_in_sample_space( ImagePosition(at.array()-1*camera::pixel) ),
        lower_right = point_in_sample_space( ImagePosition(at.array()+1*camera::pixel) );
    return (value(lower_right - upper_left).array() / 2).prod() * si::meter * si::meter;
}

std::vector< Projection::MappedPoint >
AffineProjection::cut_region_of_interest_( 
    const SamplePosition& center,
    const SamplePosition& radius ) const
{
    std::vector< MappedPoint > rv;
    Bounds bb = roi_bounding_box( center, radius );
    ImagePosition pos;
    typedef ImagePosition::Scalar Pixel;
    rv.reserve( ((bb[1].x() - bb[0].x()).value() + 1) * ((bb[1].y() - bb[0].y()).value() + 1) );
    for (Pixel x = bb[0].x(); x <= bb[1].x(); x += 1 * camera::pixel) {
        pos.x() = x;
        for (Pixel y = bb[0].y(); y <= bb[1].y(); y += 1 * camera::pixel) {
            pos.y() = y;
            SamplePosition sample = point_in_sample_space( pos );
            if ( (sample.array() >= (center - radius).array()).all() && 
                 (sample.array() <= (center + radius).array()).all() )
                rv.push_back( MappedPoint( pos, sample ) );
        }
    }
    return rv;
}

AffineProjection::Bounds
AffineProjection::roi_bounding_box( 
    const SamplePosition& center,
    const SamplePosition& radius ) const
{
    /* Determine bounds of region of interest */
    DEBUG("Cutting region around center " << center.transpose() << " with upper bound " << upper_bound.transpose()
          << " and range " << radius.transpose());
    Bounds r;
    for (int i = 0; i < 2; ++i) {
        r[0][i] = std::numeric_limits<int>::max() * camera::pixel;
        r[1][i] = std::numeric_limits<int>::min() * camera::pixel;
    }
    SamplePosition sample_pos;
    int dir[2];
    for (dir[0] = -1; dir[0] <= 1; dir[0] += 2)
      for (dir[1] = -1; dir[1] <= 1; dir[1] += 2)
    {
        for (int d = 0; d < 2; ++d)
            sample_pos[d] = center[d] + float(dir[d]) * radius[d];
        SubpixelImagePosition im = from_value< camera::length >(to_image * value(sample_pos));
        for (int d = 0; d < 2; ++d)
            if ( dir[d] < 0 ) r[0][d] = std::min( ImagePosition::Scalar(ceil(im[d])), r[0][d] );
            else              r[1][d] = std::max( ImagePosition::Scalar(floor(im[d])), r[1][d] );
    }

    DEBUG("Got box " << r.col(0).transpose() << " to " << r.col(1).transpose());
    return r;
}

AffineProjection::AffineProjection( 
        quantity<camera::resolution> x, 
        quantity<camera::resolution> y, 
        Eigen::Affine2f after_transform )
{
    to_sample = after_transform * Eigen::DiagonalMatrix<float,2>( 1.0 / x.value(), 1.0 / y.value() );
    to_image = to_sample.inverse();
}

}
}
