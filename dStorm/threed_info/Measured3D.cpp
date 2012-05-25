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

namespace dStorm {
namespace threed_info {

Measured3D::Measured3D( std::string filename, Direction direction )
{
}

ZRange Measured3D::z_range_() const { 
    ZRange rv;
    rv.insert( boost::icl::continuous_interval<ZPosition>( 0 * si::meter, 1E-5 * si::meter ) ); 
    return rv;
}

Sigma Measured3D::get_sigma_( ZPosition z ) const {
}

SigmaDerivative
Measured3D::get_sigma_deriv_( ZPosition z ) const {
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
