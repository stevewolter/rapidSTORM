#include "traits/ScaledProjection.h"
#include "traits/ProjectionConfig.h"
#include "traits/ProjectionFactory.h"
#include <simparm/Object.h>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include "image/MetaInfo.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

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
    SubpixelImagePosition point_in_image_space_( const SamplePosition& ) const;

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
};

class ScaledProjectionFactory
: public ProjectionFactory
{
    Projection* get_projection_( const image::MetaInfo<2>& o ) const { 
        return new ScaledProjection(
            o.size,
            o.resolution(0).in_dpm(), 
            o.resolution(1).in_dpm() );
    }
};

boost::shared_ptr< const ProjectionFactory > test_scaled_projection()
{
    return boost::shared_ptr< const ProjectionFactory >(new ScaledProjectionFactory());
}

class ScaledProjectionConfig
: public ProjectionConfig
{
    ProjectionFactory* get_projection_factory_() const 
        { return new ScaledProjectionFactory(); }
    ProjectionConfig* clone_() const 
        { return new ScaledProjectionConfig(*this); }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }

  public:
    ScaledProjectionConfig() : ProjectionConfig("ScaledProjection") {}
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

    if ( r.guaranteed_row_width ) {
        quantity< camera::length, int > 
            one =1 * camera::pixel, width = bb.width(Direction_X),
            lower = bb.get_lower_edge( Direction_X ), upper = bb.get_upper_edge( Direction_X );
        if ( width < *r.guaranteed_row_width ) { 
            lower -= one; width += one; 
            if ( width < *r.guaranteed_row_width ) 
                { upper += one; width += one; }
        } else if ( width > *r.guaranteed_row_width ) {
            upper -= one; width -= one;
        }
        assert( width == *r.guaranteed_row_width );
        while ( upper >= size.x() )
            { upper -= one; lower -= one; }
        while ( lower < 0 * camera::pixel )
            { lower += one; upper += one; }
        if ( upper >= size.x() )
            throw std::runtime_error("Image is too small for desired ROI");

        bb.set_range( lower, upper, Direction_X );
    }

    ImagePosition pos;
    typedef ImagePosition::Scalar Pixel;
    rv.reserve( bb.volume().value() );
    SamplePosition sample;
    for ( Bounds::const_iterator i = bb.begin(); i != bb.end(); ++i ) {
        rv.push_back( MappedPoint( *i, pixel_in_sample_space(*i) ) );
    }
    return rv;
}

ScaledProjection::Bounds
ScaledProjection::get_region_of_interest_( const ROISpecification& r ) const
{
    /* Determine bounds of region of interest */
    Bounds native( 
        from_value< camera::length >( ceil(to_image * value(r.center - r.width)).cast<int>() ),
        from_value< camera::length >( floor(to_image * value(r.center + r.width)).cast<int>() ) );
    return native.intersection( Bounds::ZeroOrigin(size) );
}

ScaledProjection::ScaledProjection( 
        ImagePosition size,
        quantity<camera::resolution> x, 
        quantity<camera::resolution> y )
: size(size),
  to_sample( 1.0 / x.value() , 1.0 / y.value() ),
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
ScaledProjection::point_in_image_space_( const SamplePosition& pos ) const
{
    return from_value< camera::length >(
        to_image * value(pos).cast<float>());
}

}
}
