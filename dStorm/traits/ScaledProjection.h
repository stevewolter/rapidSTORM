#ifndef DSTORM_TRAITS_SCALED_PROJECTION_H
#define DSTORM_TRAITS_SCALED_PROJECTION_H

#include "Projection.h"
#include <Eigen/Geometry>
#include <boost/array.hpp>
#include <boost/units/systems/camera/resolution.hpp>

namespace dStorm {
namespace traits {

class ScaledProjection : public Projection {
    ImagePosition size;
    Eigen::DiagonalMatrix<float,2> to_sample, to_image;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    units::quantity<units::si::area> pixel_size_
        ( const ImagePosition& at ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( const ROISpecification& ) const;
    Bounds get_region_of_interest_( const ROISpecification& ) const;
    ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const;
    bool supports_guaranteed_row_width_() const { return true; }

  public:
    ScaledProjection( 
        ImagePosition size,
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y );

    units::quantity<units::camera::length,float>
        length_in_image_space( int d, units::quantity<units::si::length,float> l ) const
        { return to_image.diagonal()[d] * l.value() * units::camera::pixel; }
    units::quantity<units::si::length,float>
        length_in_sample_space( int d, units::quantity<units::camera::length,float> l ) const
        { return to_sample.diagonal()[d] * l.value() * units::si::meter; }
    SubpixelImagePosition point_in_image_space( SamplePosition ) const;
};

}
}

#endif
