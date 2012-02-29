#include "Projection.h"
#include "ProjectionFactory.h"
#include "ProjectionConfig.h"
#include <Eigen/Geometry>
#include <boost/array.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/image/MetaInfo.h>
#include "debug.h"
#include <fstream>

namespace dStorm {
namespace traits {

using namespace boost::units;

class AffineProjection : public Projection {
    Eigen::Affine2f to_sample, to_image;

    SamplePosition point_in_sample_space_
        ( const SubpixelImagePosition& pos ) const;
    std::vector< MappedPoint >
        cut_region_of_interest_( const ROISpecification& ) const;
    Bounds get_region_of_interest_( const ROISpecification& ) const;
    ImagePosition nearest_point_in_image_space_
        ( const SamplePosition& pos ) const;

  public:
    AffineProjection( 
        units::quantity<units::camera::resolution> x, 
        units::quantity<units::camera::resolution> y, 
        Eigen::Affine2f after_transform );
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
        DEBUG("Micro alignment is given as " << micro_alignment());
        std::ifstream is( micro_alignment_file.c_str(), std::ios::in );
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                is >> elements(r,c);
        return new AffineProjection(
            mi.resolution(0).in_dpm(), 
            mi.resolution(1).in_dpm(), 
            Eigen::Affine2f(elements) );
    }

  public:
    AffineProjectionFactory( std::string micro_alignment_file )
        : micro_alignment_file(micro_alignment_file) {}
};

class AffineProjectionConfig
: public ProjectionConfig
{
    simparm::Object node;
    simparm::FileEntry micro_alignment;
    simparm::Node& getNode_() { return node; }
    ProjectionFactory* get_projection_factory_() const { 
        return new AffineProjectionFactory( micro_alignment() );
    }

    AffineProjectionConfig* clone_() const 
        { return new AffineProjectionConfig(*this); }

  public:
    AffineProjectionConfig() 
    : node("AffineProjection", "Linear alignment"),
      micro_alignment("AlignmentFile", "Plane Alignment file") 
      { node.push_back( micro_alignment ); }
    AffineProjectionConfig( const AffineProjectionConfig& o )
    : node(o.node), micro_alignment(o.micro_alignment) 
    {
        node.push_back( micro_alignment );
    }
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
    rv.reserve( ((bb[1].x() - bb[0].x()).value() + 1) * ((bb[1].y() - bb[0].y()).value() + 1) );
    for (Pixel x = bb[0].x(); x <= bb[1].x(); x += 1 * camera::pixel) {
        pos.x() = x;
        for (Pixel y = bb[0].y(); y <= bb[1].y(); y += 1 * camera::pixel) {
            pos.y() = y;
            SamplePosition sample = pixel_in_sample_space( pos );
            if ( r.contains( sample ) )
                rv.push_back( MappedPoint( pos, sample ) );
        }
    }
    return rv;
}

AffineProjection::Bounds
AffineProjection::get_region_of_interest_( const ROISpecification& roi ) const
{
    /* Determine bounds of region of interest */
    DEBUG("Cutting region around center " << center.transpose() << " with upper bound " << upper_bound.transpose()
          << " and range " << radius.transpose());
    Bounds r;
    for (int i = 0; i < 2; ++i) {
        r[0][i] = std::numeric_limits<int>::max() * camera::pixel;
        r[1][i] = std::numeric_limits<int>::min() * camera::pixel;
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
            r[0][d] = std::min( r[0][d], p );
            r[1][d] = std::max( r[1][d], p );
        }
    }

    DEBUG("Got box " << r.col(0).transpose() << " to " << r.col(1).transpose());
    return r;
}

AffineProjection::AffineProjection( 
        quantity<camera::resolution> x, 
        quantity<camera::resolution> y, 
        Eigen::Affine2f after_transform )
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

}
}
