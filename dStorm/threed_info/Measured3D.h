#ifndef DSTORM_THREED_INFO_SPLINE_H
#define DSTORM_THREED_INFO_SPLINE_H

#include <vector>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/Direction.h>
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>

#include <Eigen/Core>

#include <dStorm/Localization_decl.h>
#include "types.h"
#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

class Measured3D : public DepthInfo {
public:
    Measured3D( std::string file, Direction dir );

private:
    std::string config_name_() const { return "Measured3D"; }
    Sigma get_sigma_( ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( ZPosition z ) const;
    ZRange z_range_() const;
    ZPosition equifocal_plane_() const { return equifocal_plane__; }
    std::ostream& print_( std::ostream& o ) const
        { return o << "measured 3D information"; }
    bool provides_3d_info_() const { return true; }
    int image_stack_count;
    Eigen::Vector3d pixel_size;
    Image<double,3> psf_data;
    Direction dir;
    ZPosition equifocal_plane__; //?? not used
    double mu_x, mu_y;
    /*center of calibration image*/

};

}
}

#endif
