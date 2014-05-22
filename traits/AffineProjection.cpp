#include "traits/Projection.h"
#include "traits/ProjectionFactory.h"
#include "traits/ProjectionConfig.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <boost/array.hpp>
#include "boost/units/systems/camera/resolution.hpp"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include "simparm/Object.h"
#include "simparm/FileEntry.h"
#include "image/MetaInfo.h"
#include <fstream>

namespace dStorm {
namespace traits {

static const std::string test_alignment_file = "DSTORM-INTERNAL:test-alignment-file";

using namespace boost::units;

class AffineProjection : public Projection {
    Eigen::Affine2f to_sample, to_image;
    Bounds size;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( const ROISpecification& ) const;
    Bounds get_region_of_interest_( const ROISpecification& ) const;
    ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const;
    SubpixelImagePosition point_in_image_space_
        ( const SamplePosition& pos ) const;

  public:
    AffineProjection( 
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y, 
        Eigen::Affine2f after_transform,
        ImagePosition size );
};

class AffineProjectionFactory
: public ProjectionFactory
{
    std::string micro_alignment_file;

    Projection* get_projection_( const image::MetaInfo<2>& mi ) const 
    {
        if ( micro_alignment_file == "" ) 
            throw std::runtime_error("An alignment matrix file must be given "
                                     "for affine alignment");
        Eigen::Matrix3f elements = Eigen::Matrix3f::Identity();
        if ( micro_alignment_file != test_alignment_file ) {
            std::ifstream is( micro_alignment_file.c_str(), std::ios::in );
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    is >> elements(r,c);
        } else {
            elements(0,0) = 1.2;
            elements(1,1) = 0.9;
            elements(0,1) = -0.2;
            elements(1,0) = 0.3;
            elements(0,2) = 5E-8;
            elements(1,2) = 2E-9;
        }
        return new AffineProjection(
            mi.resolution(0).in_dpm(), 
            mi.resolution(1).in_dpm(), 
            Eigen::Affine2f(elements),
            mi.size );
    }

  public:
    AffineProjectionFactory( std::string micro_alignment_file )
        : micro_alignment_file(micro_alignment_file) {}
};

class AffineProjectionConfig
: public ProjectionConfig
{
    simparm::FileEntry micro_alignment;
    ProjectionFactory* get_projection_factory_() const { 
        return new AffineProjectionFactory( micro_alignment() );
    }

    AffineProjectionConfig* clone_() const 
        { return new AffineProjectionConfig(*this); }

    void attach_ui( simparm::NodeHandle at ) {
        micro_alignment.attach_ui( attach_parent(at) );
    }

  public:
    AffineProjectionConfig() 
    : ProjectionConfig("AffineProjection"), micro_alignment("AlignmentFile", "") {}
};

std::auto_ptr<ProjectionConfig> make_affine_projection_config() {
    return std::auto_ptr<ProjectionConfig>( new AffineProjectionConfig() );
}

Projection::SamplePosition 
AffineProjection::point_in_sample_space_
    ( const SubpixelImagePosition& pos ) const
{
    return boost::units::from_value< si::length >( to_sample * units::value( pos ) );
}

std::vector< Projection::MappedPoint >
AffineProjection::cut_region_of_interest_( const ROISpecification& r ) const
{
    std::vector< MappedPoint > rv;
    Bounds bb = get_region_of_interest_( r );

    ImagePosition pos;
    typedef ImagePosition::Scalar Pixel;
    rv.reserve( bb.volume().value() );
    
    for ( Bounds::const_iterator i = bb.begin(); i != bb.end(); ++i) {
        SamplePosition sample = pixel_in_sample_space( *i );
        if ( r.contains( sample ) )
            rv.push_back( MappedPoint( *i, sample ) );
    }
    return rv;
}

AffineProjection::Bounds
AffineProjection::get_region_of_interest_( const ROISpecification& roi ) const
{
    /* Determine bounds of region of interest */
    Bounds::Position lower, upper;
    for (int i = 0; i < 2; ++i) {
        lower[i] = std::numeric_limits<int>::max() * camera::pixel;
        upper[i] = std::numeric_limits<int>::min() * camera::pixel;
    }
    SamplePosition sample_pos;
    int dir[2];
    for (dir[0] = -1; dir[0] <= 1; dir[0] += 2)
      for (dir[1] = -1; dir[1] <= 1; dir[1] += 2)
    {
        for (int d = 0; d < 2; ++d)
            sample_pos[d] = roi.center[d] + float(dir[d]) * roi.width[d];
        SubpixelImagePosition im = from_value< camera::length >(to_image * value(sample_pos));
        for (int d = 0; d < 2; ++d) {
            ImagePosition::Scalar p = ImagePosition::Scalar(im[d]);
            lower[d] = std::min( lower[d], p );
            upper[d] = std::max( upper[d], p );
        }
    }

    return Bounds( lower, upper ).intersection( size );
}

AffineProjection::AffineProjection( 
        quantity<camera::resolution> x, 
        quantity<camera::resolution> y, 
        Eigen::Affine2f after_transform,
        ImagePosition size )
: size( Bounds::ZeroOrigin(size) )
{
    to_sample = after_transform * Eigen::DiagonalMatrix<float,2>( 1.0 / x.value(), 1.0 / y.value() );
    to_image = to_sample.inverse();
}

AffineProjection::ImagePosition
AffineProjection::nearest_point_in_image_space_
    ( const SamplePosition& pos ) const
{
    return from_value< camera::length >( round(
        to_image * value(pos).cast<float>()).cast<int>() );
}

AffineProjection::SubpixelImagePosition
AffineProjection::point_in_image_space_
    ( const SamplePosition& pos ) const
{
    return from_value< camera::length >( 
        to_image * value(pos).cast<float>() );
}

boost::shared_ptr< const ProjectionFactory > test_affine_projection()
{
    return boost::shared_ptr< const ProjectionFactory >( new AffineProjectionFactory(test_alignment_file) );
}

}
}
