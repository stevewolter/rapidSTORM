#include "ScaledProjection.h"
#include "ProjectionConfig.h"
#include "ProjectionFactory.h"
#include <simparm/Object.hh>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <dStorm/image/MetaInfo.h>
#include "debug.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

class ScaledProjectionFactory
: public ProjectionFactory
{
    Projection* get_projection_( const image::MetaInfo<2>& o ) const { 
        return new ScaledProjection(
            o.resolution(0).in_dpm(), 
            o.resolution(1).in_dpm() );
    }
};

class ScaledProjectionConfig
: public ProjectionConfig
{
    simparm::Object node;
    simparm::Node& getNode_() { return node; }
    ProjectionFactory* get_projection_factory_() const 
        { return new ScaledProjectionFactory(); }
    ProjectionConfig* clone_() const 
        { return new ScaledProjectionConfig(*this); }

  public:
    ScaledProjectionConfig() : node("ScaledProjection", "No alignment") {}
};

std::auto_ptr<ProjectionConfig> make_scaling_projection_config() {
    return std::auto_ptr<ProjectionConfig>( new ScaledProjectionConfig() );
}

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
ScaledProjection::cut_region_of_interest_( const ROISpecification& r ) const
{
    std::vector< MappedPoint > rv;
    Bounds bb = get_region_of_interest_( r );
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
ScaledProjection::get_region_of_interest_( const ROISpecification& r ) const
{
    /* Determine bounds of region of interest */
    DEBUG("Cutting region around center " << center.transpose() << " with upper bound " << upper_bound.transpose()
          << " and range " << radius.transpose());
    Bounds rv;
    rv[0] = from_value< camera::length >( ceil(to_image * value(r.center - r.width)).cast<int>() );
    rv[1] = from_value< camera::length >( floor(to_image * value(r.center + r.width)).cast<int>() );
    return rv;
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
