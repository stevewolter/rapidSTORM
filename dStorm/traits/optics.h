#ifndef DSTORM_TRAITS_OPTICS_H
#define DSTORM_TRAITS_OPTICS_H

#include <memory>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/variant/variant.hpp>
#include <Eigen/Core>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include "image_resolution.h"
#include <vector>
#include <boost/units/Eigen/Core>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "../units/camera_response.h"

namespace dStorm {
namespace traits {

class Projection;

namespace units = boost::units;

struct PlaneConfig;

template <int Dimensions>
struct Optics;

struct No3D {};

struct Zhuang3D {
    typedef units::power_typeof_helper< 
            units::si::length,
            units::static_rational<-1> >::type Unit; 
    typedef Eigen::Matrix< units::quantity< Unit >, 2, 1, Eigen::DontAlign > 
        Widening;
    Widening widening;
};

template <>
struct Optics<2>
{
    typedef units::quantity< units::camera::resolution, float > Resolution;
    typedef Eigen::Matrix< units::quantity<units::si::length,float>, 3, 1, Eigen::DontAlign >
        SamplePosition;
    typedef Eigen::Matrix< units::quantity<units::camera::length,int>, 2, 1, Eigen::DontAlign > ImagePosition;
    typedef Eigen::Matrix< units::quantity<units::camera::length,float>, 2, 1, Eigen::DontAlign > SubpixelImagePosition;
    typedef Eigen::Matrix< int, 2, 1, Eigen::DontAlign > UnitlessImagePosition;
    typedef Eigen::Array< boost::units::quantity< boost::units::si::length, float >, 2,1, Eigen::DontAlign > PSF;

    typedef Eigen::Matrix< float, 3, Eigen::Dynamic > PointSet;

    enum TransformationClass { Scaled, ScaledTranslation, Affine, Projective };

    typedef boost::array< boost::optional<ImageResolution>,2> Resolutions;

  private:
    struct Pimpl;
    std::auto_ptr< Pimpl > pimpl;
    boost::array< boost::optional<ImageResolution> ,2> resolutions;
    boost::optional< PSF > psf;
    boost::shared_ptr< const Projection > projection_;

  public:
    friend class dStorm::traits::PlaneConfig;

    Optics();
    Optics( const Optics& );
    Optics& operator=( const Optics& );
    ~Optics();

    boost::optional< units::quantity< units::si::length > > z_position, offsets[2];
    boost::optional<camera_response> photon_response, background_stddev;
    boost::optional< units::quantity< units::camera::intensity, int > > dark_current;

    boost::optional<PSF>& psf_size( int ) { return psf; }
    boost::optional<PSF> psf_size( int ) const { return psf; }
    boost::shared_ptr< const Projection > projection() const
        { return projection_; }

    SamplePosition point_in_sample_space( const ImagePosition& pos ) const;
    SamplePosition point_in_sample_space( const SubpixelImagePosition& pos ) const;
    SamplePosition vector_in_sample_space( const ImagePosition& pos ) const;
    SamplePosition vector_in_sample_space( const SubpixelImagePosition& pos ) const;
    units::quantity<units::si::length,float>
        point_offset_in_sample_space( int dimension, units::quantity<units::camera::length,float> ) const;
    units::quantity<units::si::length,float>
        length_in_sample_space( int dimension, units::quantity<units::camera::length,float> ) const;

    void points_in_sample_space( PointSet& pos ) const;
    void points_in_image_space( PointSet& pos ) const;

    ImagePosition nearest_point_in_image_space( const SamplePosition& pos ) const;
    SubpixelImagePosition point_in_image_space( const SamplePosition& pos ) const;
    SubpixelImagePosition vector_in_image_space( const SamplePosition& pos ) const;
    units::quantity<units::camera::length,float>
        point_offset_in_image_space( int dimension, units::quantity<units::si::length,float> ) const;
    units::quantity<units::camera::length,float>
        length_in_image_space( int dimension, units::quantity<units::si::length,float> ) const;

    inline boost::optional<ImageResolution> resolution(int r) const 
        { assert( transformation_class() <= Scaled); return resolutions[r]; }
    inline bool resolution_given_in_dpm(int r) const { 
        return resolutions[r].is_initialized() && resolutions[r]->is_in_dpm() && transformation_class() <= Scaled; }
    inline const Resolutions& image_resolutions() const { return resolutions; }
    TransformationClass transformation_class() const;

    float transmission_coefficient( int fluorophore ) const;

    void set_resolution( const boost::array< ImageResolution, 2 >& f );
    void set_resolution( const Resolutions& f );
    void set_fluorophore_transmission_coefficient( int fluorophore, float );
    void apply_transformation( const Eigen::Matrix3f& );

    Optics<2>& plane( int i ) { assert(i == 0); return *this; }
    const Optics<2>& plane( int i ) const { assert(i == 0); return *this; }
    int plane_count() const { return 1; }
};

template <>
struct Optics<3>
{
    Optics() { planes.push_back( Optics<2>() ); }
    typedef std::vector< Optics<2> > Planes;
    Planes planes;
    typedef boost::variant< Zhuang3D, No3D > DepthInfo;
    boost::optional< DepthInfo > depth_info;

    Optics<2>& plane( int i ) { return planes.at(i); }
    const Optics<2>& plane( int i ) const { return planes.at(i); }
    int plane_count() const { return planes.size(); }

    Optics<2>::SamplePosition 
        size_in_sample_space( const Optics<2>::SubpixelImagePosition& projection_size ) const;
};

}
}

#endif
