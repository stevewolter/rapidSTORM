#ifndef DSTORM_TRAITS_OPTICS_H
#define DSTORM_TRAITS_OPTICS_H

#include <memory>
#include <vector>

#include <boost/optional.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/units/Eigen/Core>
#include <boost/units/quantity.hpp>
#include "boost/units/systems/camera/length.hpp"
#include "boost/units/systems/camera/resolution.hpp"
#include <boost/units/systems/si/length.hpp>

#include <Eigen/Core>

#include "Direction.h"
#include "types/samplepos.h"

#include "units/camera_response.h"

namespace dStorm {
namespace threed_info { class DepthInfo; }
namespace traits {

class ProjectionFactory;

namespace units = boost::units;

struct PlaneConfig;

class Optics
{
    std::vector<float> tmc;
    boost::shared_ptr< const ProjectionFactory > projection_factory_;
    boost::shared_ptr< const threed_info::DepthInfo > depth_info_[Direction_2D];

  public:
    friend class dStorm::traits::PlaneConfig;

    Optics();
    ~Optics();

    boost::optional<camera_response> photon_response;
    boost::optional< units::quantity< units::camera::intensity, int > > dark_current;

    const boost::shared_ptr< const ProjectionFactory > 
        projection_factory() const
        { return projection_factory_; }
    void set_projection_factory( boost::shared_ptr< const ProjectionFactory > p )
        { projection_factory_ = p; }

    const boost::shared_ptr< const threed_info::DepthInfo > depth_info( Direction dir ) const { return depth_info_[dir]; }
    void set_depth_info( Direction dir, boost::shared_ptr< const threed_info::DepthInfo > p ) { depth_info_[dir] = p; }

    float transmission_coefficient( int fluorophore ) const;
    void set_fluorophore_transmission_coefficient( int fluorophore, float );
};

}
}

#endif
