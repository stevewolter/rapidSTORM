#ifndef DSTORM_TRAITS_AFFINE_PROJECTION_H
#define DSTORM_TRAITS_AFFINE_PROJECTION_H

#include "Projection.h"
#include <Eigen/Geometry>
#include <boost/array.hpp>
#include <boost/units/systems/camera/resolution.hpp>

namespace dStorm {
namespace traits {

class AffineProjection : public Projection {
    Eigen::Affine2f to_sample, to_image;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    units::quantity<units::si::area> pixel_size_
        ( const ImagePosition& at ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( 
            const SamplePosition& center,
            const SamplePosition& width ) const;

    typedef boost::array< ImagePosition, 2 > Bounds;
    Bounds roi_bounding_box( 
        const SamplePosition& center, 
        const SamplePosition& width ) const;

  public:
    AffineProjection( 
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y, 
        Eigen::Affine2f after_transform );
};

}
}

#endif
