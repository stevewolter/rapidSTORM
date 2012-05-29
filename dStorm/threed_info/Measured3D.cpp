#include "dejagnu.h"
#include "Measured3D.h"
#include "Config.h"
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <stdexcept>
#include <fstream>
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include <dStorm/Localization.h>
#include "look_up_sigma_diff.h"
#include <Eigen/Dense>
#include <cmath>
#include "guf/GaussImage.h"

namespace dStorm {
namespace threed_info {

Measured3D::Measured3D( std::string filename, Direction direction )

{
    dir = direction;
    Eigen::Vector3d image_x0;
    image_x0 << 0.9, 0.9, 1.5;
    pixel_size << 0.06, 0.06, 0.06;
    dStorm::Image<double,3>::Size size;
    size[0]= 30 * camera::pixel;
    size[1]= 30 * camera::pixel;
    size[2]= 50 * camera::pixel;
    Image <double,3> psf_data= Image<double,3>( size );
    double correction_factor = 12.76615297;

    for (int x = 0; x < 30; ++x)
        for (int y = 0; y < 30; ++y)
            for (int z = 0; z < 50; ++z)
                psf_data(x,y,z) = correction_factor * dStorm::guf::psf_calib_image_test[x][y][z];

image_stack_count = psf_data.depth_in_pixels();
//    dStorm::tiff::OpenFile()(filename, config, node);

//load image  Image <double,3> psf_data
}

ZRange Measured3D::z_range_() const {
    ZRange rv;
    ZPosition low = 0 * si::meter;
    ZPosition high = static_cast<float>((image_stack_count * pixel_size[2]) *1E-6) *si::meter;
    rv.insert( boost::icl::continuous_interval<ZPosition>( low, high ) );
    return rv;
}

Sigma Measured3D::get_sigma_( ZPosition z ) const {
double sum_quad = 0;
double sum = 0;

if (dir ==0)
   for (int dx =0; dx < psf_data.width_in_pixels(); dx++)
{
   sum +=  psf_data(dx ,floor(mu_y) ,z.value());
   sum_quad+=  pow(psf_data(dx ,floor(mu_y) ,z.value()), 2);
   return static_cast<float>(pixel_size[0]*1E-6 * sqrt( (sum_quad - (sum* sum / psf_data.width_in_pixels())) /(psf_data.width_in_pixels()-1)))*si::meter;

}
else
   for (int dy =0; dy < psf_data.height_in_pixels(); dy++)
{
   sum +=  psf_data(floor(mu_x), dy , z.value());
   sum_quad+=  pow(psf_data(floor(mu_x), dy , z.value()), 2);
   return static_cast<float>(pixel_size[0]*1E-6*sqrt( (sum_quad - (sum* sum / psf_data.height_in_pixels())) /(psf_data.height_in_pixels()-1))) *si::meter;
}


}

SigmaDerivative Measured3D::get_sigma_deriv_( ZPosition z ) const {
     throw std::logic_error("Measured_psf can't calculate sigma_deriv!");
}

class Measured3DConfig : public simparm::Object, public Config {
    simparm::FileEntry z_calibration_file;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const {
        if ( z_calibration_file )
            return boost::shared_ptr<DepthInfo>(new Measured3D( z_calibration_file(), dir ) );
        else
            return boost::shared_ptr<DepthInfo>();
    }
    void read_traits( const DepthInfo&, const DepthInfo& )
        { z_calibration_file = ""; }
    void set_context() {}
    simparm::Node& getNode() { return *this; }
  public:
    Measured3DConfig()
        : simparm::Object("Measured3D", "Interpolated 3D"),
          z_calibration_file("ZCalibration", "Z calibration file") {}
    Measured3DConfig* clone() const { return Measured3DConfig(*this); }
};

std::auto_ptr< Config > make_measured_3d_config()
    { return std::auto_ptr< Config >( new Measured3DConfig() ); }


}
}
