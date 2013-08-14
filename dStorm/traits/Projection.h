#ifndef DSTORM_TRAITS_PROJECTION_H
#define DSTORM_TRAITS_PROJECTION_H

#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/optional/optional.hpp>
#include <vector>
#include <dStorm/image/Box.h>
#include <dStorm/engine/FitPosition.h>

namespace dStorm {
namespace traits {

namespace units = boost::units;

struct Projection {
    typedef Eigen::Matrix< units::quantity<units::camera::length,float>, 2, 1, Eigen::DontAlign > SubpixelImagePosition;
    typedef Eigen::Matrix< units::quantity<units::camera::length,int>, 2, 1, Eigen::DontAlign > ImagePosition;
    typedef Eigen::Matrix< units::quantity<units::si::length,float>, 2, 1, Eigen::DontAlign > SamplePosition;

    struct MappedPoint {
        ImagePosition image_position;
        SamplePosition sample_position;
        MappedPoint( const ImagePosition& i, const SamplePosition& s )
            : image_position(i), sample_position(s) {}
    };

    struct ROISpecification {
        SamplePosition center;
        SamplePosition width;
        boost::optional< units::quantity<units::camera::length,int> > 
            guaranteed_row_width;

        ROISpecification( const SamplePosition& center_, 
                          const SamplePosition& width_ );
        // Positions given in micrometers.
        ROISpecification( const engine::FitPosition& center_, 
                          const engine::FitPosition& width_ );
        bool contains( const SamplePosition& ) const;
    };
    typedef std::vector< MappedPoint > ROI;
    typedef image::Box<2> Bounds;

    virtual ~Projection() {}
  private:
    virtual SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const = 0;
    virtual units::quantity<units::si::area> pixel_size_
        ( const ImagePosition& at ) const;
    virtual std::vector< MappedPoint >
        cut_region_of_interest_( const ROISpecification& ) const = 0;
    virtual Bounds
        get_region_of_interest_( const ROISpecification& ) const = 0;
    virtual ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const = 0;
    virtual SubpixelImagePosition point_in_image_space_
        ( const SamplePosition& pos ) const = 0;
    virtual SamplePosition pixel_in_sample_space_
        ( const ImagePosition& pos ) const 
        { return point_in_sample_space_(pos.cast< SubpixelImagePosition::Scalar >()); }
    virtual bool supports_guaranteed_row_width_() const 
        { return false; }

  public:
    SubpixelImagePosition point_in_image_space
        ( const SamplePosition& pos ) const
        { return point_in_image_space_(pos); }
    ImagePosition nearest_point_in_image_space
        ( const SamplePosition& pos ) const
        { return nearest_point_in_image_space_(pos); }
    SamplePosition point_in_sample_space
        ( const SubpixelImagePosition& pos ) const 
        { return point_in_sample_space_(pos); }
    SamplePosition pixel_in_sample_space
        ( const ImagePosition& pos ) const 
        { return pixel_in_sample_space_(pos); }
    units::quantity<units::si::area> pixel_size
        ( const ImagePosition& at ) const 
        { return pixel_size_(at); }

    ROI cut_region_of_interest( const ROISpecification& r ) const
        { return cut_region_of_interest_( r ); }

    Bounds get_region_of_interest( const ROISpecification& r ) const
        { return get_region_of_interest_( r ); }

    bool supports_guaranteed_row_width() const 
        { return supports_guaranteed_row_width_(); }
};

}
}

#endif
