#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
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
#include <simparm/Entry_Impl.hh>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include <dStorm/Localization.h>
#include <dStorm/units/microlength.h>
#include "look_up_sigma_diff.h"
#include <Eigen/Dense>
#include <cmath>
#include "guf/GaussImage.h"
#include "tiff/OpenFile.h"
#include <dStorm/image/slice.h>

#include <algorithm>
#include <numeric>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace threed_info {

Measured3D::Measured3D( 
    std::string filename, Direction direction, simparm::Node& gui_node, 
    FluorophorePosition position,
    PixelSize pixel_size )
{
#if MOCK_MEASURED_3D_CONSTRUCTOR
    dir = direction;
    image_x0 << 0.9, 0.9, 1.5;
    pixel_size << 0.06, 0.06, 0.06;
    dStorm::Image<double,3>::Size size;
    size[0]= 30 * camera::pixel;
    size[1]= 30 * camera::pixel;
    size[2]= 50 * camera::pixel;
    psf_data = Image<double,3>( size );
    double correction_factor = 12.76615297;

    for (int x = 0; x < 30; ++x)
        for (int y = 0; y < 30; ++y)
            for (int z = 0; z < 50; ++z)
                psf_data(x,y,z) = correction_factor * dStorm::guf::psf_calib_image_test[x][y][z];
#else
    image_x0 = value( position.cast< quantity< si::microlength > >() );
    
    typedef boost::units::divide_typeof_helper< si::microlength, camera::length > MuPerPx;

    for (int i = 0; i < pixel_size.rows(); ++i) {
        quantity<si::nanolength,float> in_nano(pixel_size[i] * camera::pixel);
        quantity<si::microlength> one_pixel_width(in_nano);
        this->pixel_size[i] = one_pixel_width.value();
    }

    tiff::Config config;
    config.ignore_warnings = true;
    config.determine_length = true;

    tiff::OpenFile file( filename, config, gui_node );
    simparm::Entry<long> error_count("ErrorCount", "Error count");
    std::auto_ptr< input::Traits<engine::ImageStack> > traits = 
        file.getTraits( true, error_count );

    assert( traits->image_number().range().second.is_initialized() );
    assert( traits->plane_count() == 1 );

    dStorm::Image<double,3>::Size size;
    size.head<2>() = traits->image( 0 ).size;
    size.z() = quantity<camera::length,int>( *traits->image_number().range().second / camera::frame * camera::pixel );

    psf_data = Image<double,3>( size );
    int slice_index = 0;
    double max_sum = 0;

    do {
        tiff::OpenFile::Image image = file.read_image( gui_node );
        Image<double,2> target_image = psf_data.slice( Direction_Z, slice_index * camera::pixel );

        std::copy( image.begin(), image.end(), target_image.begin() );

        max_sum = std::max( max_sum, std::accumulate( target_image.begin(), target_image.end(), 0.0 ) );

        ++slice_index;
    } while ( file.next_image( gui_node ) );

    std::for_each( psf_data.begin(), psf_data.end(), 
        boost::bind( std::divides<double>(), _1, max_sum ) );

#endif
}

ZRange Measured3D::z_range_() const {
    ZRange rv;
    ZPosition low = 0 * si::meter;
    ZPosition high = static_cast<float>((psf_data.depth_in_pixels() * pixel_size[2]) *1E-6) *si::meter;
    rv.insert( boost::icl::continuous_interval<ZPosition>( low, high ) );
    return rv;
}

Sigma Measured3D::get_sigma_( ZPosition z ) const {
    double sum_quad = 0;
    double sum = 0;

    if (dir == Direction_X) {
        for (int dx = 0; dx < psf_data.width_in_pixels (); dx++)
            {
            sum += psf_data (dx, floor ( image_x0[1] ), z.value ());
            sum_quad += pow (psf_data (dx, floor ( image_x0[1] ), z.value ()), 2);
            return static_cast <
                float >(pixel_size[0] * 1E-6 *
                        sqrt ((sum_quad -
                            (sum * sum / psf_data.width_in_pixels ())) /
                            (psf_data.width_in_pixels () - 1))) * si::meter;

            }
    } else if (dir == Direction_Y) {
        for (int dy = 0; dy < psf_data.height_in_pixels (); dy++)
            {
            sum += psf_data (floor ( image_x0[0] ), dy, z.value ());
            sum_quad += pow (psf_data (floor ( image_x0[0] ), dy, z.value ()), 2);
            return static_cast <
                float >(pixel_size[0] * 1E-6 *
                        sqrt ((sum_quad -
                            (sum * sum / psf_data.height_in_pixels ())) /
                            (psf_data.height_in_pixels () - 1))) * si::meter;
            }
    } else
        throw std::logic_error ("This direction is not implented");
}

SigmaDerivative Measured3D::get_sigma_deriv_( ZPosition z ) const {
     throw std::logic_error("Measured_psf can't calculate sigma_deriv!");
}

class Measured3DConfig : public simparm::Object, public Config {
    simparm::FileEntry z_calibration_file;
    simparm::Entry< Measured3D::FluorophorePosition > fluorophore_position;
    simparm::Entry< Measured3D::PixelSize > pixel_size;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const {
        if ( z_calibration_file )
            return boost::shared_ptr<DepthInfo>(
                new Measured3D( 
                    z_calibration_file(), dir, 
                    const_cast<Measured3DConfig&>(*this),
                    fluorophore_position(),
                    pixel_size() ) );
        else
            return boost::shared_ptr<DepthInfo>();
    }
    void read_traits( const DepthInfo&, const DepthInfo& )
        { z_calibration_file = ""; }
    void set_context() {}
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { 
        push_back( z_calibration_file ); 
        push_back( fluorophore_position ); 
        push_back( pixel_size );
    }
  public:
    Measured3DConfig()
        : simparm::Object("Measured3D", "Interpolated 3D"),
          z_calibration_file("ZCalibration", "Z calibration file"),
          fluorophore_position("CalibFluorophorePosition", "Fluorophore position in calibration image"),
          pixel_size("CalibPixelSize", "Pixel size in calibration image", 
            Measured3D::PixelSize::Constant(100 * si::nanometre / camera::pixel) ) 
    {
        registerNamedEntries();
    }
    Measured3DConfig( const Measured3DConfig& o )
        : simparm::Object(o), z_calibration_file(o.z_calibration_file),
          fluorophore_position(o.fluorophore_position),
          pixel_size(o.pixel_size) { registerNamedEntries(); }
    Measured3DConfig* clone() const { return new Measured3DConfig(*this); }
};

std::auto_ptr< Config > make_measured_3d_config()
    { return std::auto_ptr< Config >( new Measured3DConfig() ); }


}
}
