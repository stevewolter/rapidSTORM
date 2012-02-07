#ifndef DSTORM_TRAITS_PROJECTION_H
#define DSTORM_TRAITS_PROJECTION_H

#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/area.hpp>
#include <vector>

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

    virtual ~Projection() {}
  private:
    virtual SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const = 0;
    virtual units::quantity<units::si::area> pixel_size_
        ( const ImagePosition& at ) const = 0;
    virtual std::vector< MappedPoint >
        cut_region_of_interest_( 
            const SamplePosition& center,
            const SamplePosition& width ) const = 0;

  public:
    SamplePosition point_in_sample_space
        ( const SubpixelImagePosition& pos ) const 
        { return point_in_sample_space_(pos); }
    SamplePosition point_in_sample_space
        ( const ImagePosition& pos ) const 
        { return point_in_sample_space_(pos.cast< SubpixelImagePosition::Scalar >()); }
    units::quantity<units::si::area> pixel_size
        ( const ImagePosition& at ) const 
        { return pixel_size_(at); }

    typedef std::vector< MappedPoint > ROI;
    ROI cut_region_of_interest( 
            const SamplePosition& center,
            const SamplePosition& width ) const
        { return cut_region_of_interest_( center, width ); }

};

}
}

#endif
