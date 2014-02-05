#ifndef DSTORM_TRAITS_SCALED_PROJECTION_H
#define DSTORM_TRAITS_SCALED_PROJECTION_H

#include "traits/Projection.h"
#include <Eigen/Geometry>
#include <boost/array.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace traits {

class ProjectionFactory;
boost::shared_ptr< const ProjectionFactory > test_scaled_projection();

}
}

#endif
