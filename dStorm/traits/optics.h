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
#include <vector>
#include <boost/units/Eigen/Core>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "../units/camera_response.h"
#include <dStorm/types/samplepos.h>

namespace dStorm {
namespace traits {

class ProjectionFactory;

namespace units = boost::units;

struct PlaneConfig;

struct Optics
{
    typedef units::quantity< units::camera::resolution, float > Resolution;
    typedef Eigen::Array< boost::units::quantity< boost::units::si::length, float >, 2,1, Eigen::DontAlign > PSF;

  private:
    std::vector<float> tmc;
    boost::optional< PSF > psf;
    boost::shared_ptr< const ProjectionFactory > projection_factory_;

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
    boost::shared_ptr< const ProjectionFactory > projection_factory() const
        { return projection_factory_; }

    float transmission_coefficient( int fluorophore ) const;

    void set_fluorophore_transmission_coefficient( int fluorophore, float );
};

}
}

#endif
