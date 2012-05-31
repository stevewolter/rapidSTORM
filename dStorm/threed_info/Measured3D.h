#ifndef DSTORM_THREED_INFO_MEASURED_3D_H
#define DSTORM_THREED_INFO_MEASURED_3D_H

#include <vector>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/Direction.h>
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/units/nanoresolution.h>

#include <Eigen/Core>

#include <dStorm/Localization_decl.h>
#include "types.h"
#include "DepthInfo.h"
#include <simparm/Node.hh>

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

class Measured3D : public DepthInfo {
public:
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 3, 1, Eigen::DontAlign > FluorophorePosition;
    typedef  Eigen::Matrix< quantity< nanometer_pixel_size, float >, 3, 1, Eigen::DontAlign > PixelSize;

    Measured3D( std::string file, Direction dir, FluorophorePosition, PixelSize, quantity<camera::intensity>, quantity<si::length> sigma_smoothing_sigma );

    const Eigen::Vector3d& get_pixel_size_in_mum_per_px() const
        { return pixel_size; }
    const Eigen::Vector3d& get_calibration_image_offset_in_mu() const
        { return image_x0; }
    const Image<double,3>& get_calibration_image() const
        { const_cast< Measured3D& >(*this).read_calibration_image(); return psf_data; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    std::string config_name_() const { return "Measured3D"; }
    Sigma get_sigma_( ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( ZPosition z ) const;
    ZRange z_range_() const;
    std::ostream& print_( std::ostream& o ) const
        { return o << "measured 3D information"; }
    bool provides_3d_info_() const { return true; }

    void read_calibration_image();

    std::string filename;
    quantity<camera::intensity> threshold;
    bool psf_data_has_been_read;
    boost::mutex psf_data_flag_mutex;

    Eigen::Vector3d pixel_size, image_x0;
    Image<double,3> psf_data;
    Direction dir;
    std::vector< double > standard_deviations;
    double sigma;
};

}
}

#endif
