#ifndef DSTORM_TRAITS_AFFINE_PROJECTION_H
#define DSTORM_TRAITS_AFFINE_PROJECTION_H

#include "Projection.h"
#include <Eigen/Geometry>
#include <dStorm/Image.h>
#include <boost/array.hpp>
#include <boost/units/systems/camera/resolution.hpp>

namespace dStorm {
namespace traits {

class SupportPointProjection : public Projection {
    Eigen::DiagonalMatrix<float,2> to_prewarp;
    dStorm::Image< SamplePosition, 2 > prewarp_to_warp;
    Eigen::Affine2f approx_to_image;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    units::quantity<units::si::area> pixel_size_
        ( const ImagePosition& at ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( 
            const SamplePosition& center,
            const SamplePosition& width ) const;
    Bounds get_region_of_interest_( 
        const SamplePosition& center, 
        const SamplePosition& width ) const;
    ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const;

  public:
    SupportPointProjection( 
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y, 
        units::quantity<units::camera::resolution> map_x, 
        units::quantity<units::camera::resolution> map_y,
        std::istream& file_map );
};

}
}

#endif
