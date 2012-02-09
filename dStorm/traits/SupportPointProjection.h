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
    class PointOrdering;

    typedef dStorm::Image< SamplePosition, 2 > Map;
    Eigen::Array2f higher_density;
    Map high_density_map, forward_map;
    Eigen::Affine2f approx_reverse;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    SamplePosition pixel_in_sample_space_
        ( const ImagePosition& pos ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( const ROISpecification& ) const;
    Bounds get_region_of_interest_( const ROISpecification& ) const;
    ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const;

    void compute_forward_map();
    void approximate_reverse_transformation();

  public:
    SupportPointProjection( 
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y, 
        units::quantity<units::camera::resolution> map_x, 
        units::quantity<units::camera::resolution> map_y,
        std::istream& file_map );

    std::vector< MappedPoint >
        cut_region_of_interest_naively( const ROISpecification& ) const;
};

}
}

#endif
