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
namespace threed_info { class DepthInfo; }
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
    boost::shared_ptr< const threed_info::DepthInfo > depth_info_;

  public:
    friend class dStorm::traits::PlaneConfig;

    Optics();
    ~Optics();

    boost::optional<camera_response> photon_response, background_stddev;
    boost::optional< units::quantity< units::camera::intensity, int > > dark_current;

    boost::optional<PSF>& psf_size( int ) { return psf; }
    const boost::optional<PSF> psf_size( int ) const { return psf; }
    const boost::shared_ptr< const ProjectionFactory > 
        projection_factory() const
        { return projection_factory_; }

    const boost::shared_ptr< const threed_info::DepthInfo > depth_info() const { return depth_info_; }
    void set_depth_info( boost::shared_ptr< const threed_info::DepthInfo > p ) { depth_info_ = p; }

    float transmission_coefficient( int fluorophore ) const;
    void set_fluorophore_transmission_coefficient( int fluorophore, float );
};

}
}

#endif
