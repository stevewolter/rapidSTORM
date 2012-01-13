#ifndef NONLINFIT_IMAGE_CUT_REGION_H
#define NONLINFIT_IMAGE_CUT_REGION_H

#include "TransformedImage.h"
#include "debug.h"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>

namespace dStorm {
namespace guf {

using namespace boost::units;

template <typename LengthUnit>
TransformedImage<LengthUnit>
    ::TransformedImage( const Spot& max_distance, const dStorm::traits::Optics<2>& optics )
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
    Bounds r;
    r.col(0).fill( std::numeric_limits<int>::max() * camera::pixel);
    r.col(1).fill( std::numeric_limits<int>::min() * camera::pixel);
    Optics::SamplePosition sample_pos;
    sample_pos[2] = quantity<si::length>(*optics.z_position);
    for (int xo = -1; xo <= 1; xo += 2)
      for (int yo = -1; yo <= 1; yo += 2)
    {
        sample_pos.x() = Spot::Scalar(center.x()) + float(xo) * max_distance.x();
        sample_pos.y() = Spot::Scalar(center.y()) + float(yo) * max_distance.y();
        ImageSize im_sp = this->optics.nearest_point_in_image_space(sample_pos);
        Optics::SamplePosition rev = this->optics.point_in_sample_space( im_sp );

        /* Adjust the borders to avoid rounding to pixels outside of ROI */
        if ( xo < 0 && rev.x() < sample_pos.x() ) im_sp.x() += 1 * camera::pixel;
        if ( yo < 0 && rev.y() < sample_pos.y() ) im_sp.y() += 1 * camera::pixel;
        if ( xo > 0 && rev.x() > sample_pos.x() ) im_sp.x() -= 1 * camera::pixel;
        if ( yo > 0 && rev.y() > sample_pos.y() ) im_sp.y() -= 1 * camera::pixel;

        DEBUG("Corner " << xo << " " << yo << " is " << im_sp.transpose());
        r.col(0) = r.col(0).min( im_sp.array() );
        r.col(1) = r.col(1).max( im_sp.array() );
    }

    r = r.max( Bounds::Constant(0 * camera::pixel) );
    r.col(1) = r.col(1).min( upper_bound.array() );
    r.col(0) = r.col(0).min( upper_bound.array() );
    
    DEBUG("Got box " << r.col(0).transpose() << " to " << r.col(1).transpose());
    return r;
}

}
}

#endif
