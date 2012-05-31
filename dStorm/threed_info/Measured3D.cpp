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
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace threed_info {

Measured3D::Measured3D( 
    std::string filename, Direction direction,
    FluorophorePosition position,
    PixelSize pixel_size,
    quantity<camera::intensity> threshold,
    quantity<si::length> sigma_smoothing_sigma )
: filename( filename ), threshold(threshold),
  psf_data_has_been_read( false ), dir( direction ),
  sigma( quantity<si::nanolength>(sigma_smoothing_sigma) / pixel_size.z() / camera::pixel )
{
    image_x0 = value( position.cast< quantity< si::microlength > >() );
    
    typedef boost::units::divide_typeof_helper< si::microlength, camera::length > MuPerPx;

    for (int i = 0; i < pixel_size.rows(); ++i) {
        quantity<si::nanolength,float> in_nano(pixel_size[i] * camera::pixel);
        quantity<si::microlength> one_pixel_width(in_nano);
        this->pixel_size[i] = one_pixel_width.value();
    }

}

void Measured3D::read_calibration_image() {
    boost::lock_guard< boost::mutex > lock( psf_data_flag_mutex );
    if ( psf_data_has_been_read ) return;

    simparm::Object gui_node("Dummy", "Dummy");
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

    do {
        tiff::OpenFile::Image image = file.read_image( gui_node );
        Image<double,2> target_image = psf_data.slice( Direction_Z, slice_index * camera::pixel );

        assert( (image.sizes().head<2>() == target_image.sizes()).all() );
        std::copy( image.begin(), image.end(), target_image.begin() );

        double sum = std::accumulate( target_image.begin(), target_image.end(), 0.0 );

        std::transform( target_image.begin(), target_image.end(), target_image.begin(),
            boost::bind( std::divides<double>(), _1, sum ) );

        double accum = 0, total_mass = 0;
        for ( Image< double, 2 >::const_iterator i = target_image.begin(); i != target_image.end(); ++i ) {
            double delta_in_m = (i.position()[ dir ].value() * pixel_size[dir] - image_x0[dir]) * 1E-6;
            double weight = std::max(0.0, *i - threshold.value() / sum );
            accum += delta_in_m * delta_in_m * weight;
            total_mass += weight;
        }
        if ( total_mass < 1E-10 ) 
            throw std::runtime_error("Image " + boost::lexical_cast<std::string>( slice_index ) + " is below threshold" );
        float sigma = ( sqrt( accum / total_mass ) ) ;
        standard_deviations.push_back( sigma );

        ++slice_index;
    } while ( slice_index < psf_data.depth_in_pixels() && file.next_image( gui_node ) );

    psf_data_has_been_read = true;
}

ZRange Measured3D::z_range_() const {
    assert( pixel_size[2] > 0 );
    const_cast<Measured3D&>(*this).read_calibration_image();

    quantity<si::length> end_of_image = static_cast<float>(( (psf_data.depth_in_pixels()-1) * pixel_size[2]) *1E-6) *si::meter;
    ZRange rv;
    ZPosition low = ZPosition(image_x0.z() * 1E-6 * si::meter - end_of_image);
    ZPosition high = ZPosition(image_x0.z() * 1E-6 * si::meter);
    rv.insert( boost::icl::continuous_interval<ZPosition>( low, high ) );
    return rv;
}

Sigma Measured3D::get_sigma_( ZPosition z ) const {
    const_cast<Measured3D&>(*this).read_calibration_image();

    const int base = round(2*sigma); 
    double sigma_accum = 0, total_weight = 0;
    ZPosition z_calib = float(image_x0.z() * 1E-6) * si::meter - z;
    int center_pixel = round( z_calib.value() * 1E6 / pixel_size.z() );
    for ( int z = std::max( 0, center_pixel - base ), ez = std::min( psf_data.depth_in_pixels()-1, center_pixel + base); z <= ez; ++z )
    {
        double weight = exp( -0.5 * pow<2>( (z - center_pixel) / sigma ) );
        sigma_accum += standard_deviations[z] * weight;
        total_weight += weight;
    }
    return float(sigma_accum / total_weight) * si::meter;
}

SigmaDerivative Measured3D::get_sigma_deriv_( ZPosition z ) const {
     throw std::logic_error("Measured_psf can't calculate sigma_deriv!");
}

class Measured3DConfig : public simparm::Object, public Config {
    simparm::FileEntry z_calibration_file;
    simparm::Entry< Measured3D::FluorophorePosition > fluorophore_position;
    simparm::Entry< Measured3D::PixelSize > pixel_size;
    simparm::Entry< quantity<camera::intensity> > background;
    simparm::Entry< quantity<si::nanolength> > sigma_smoothing_sigma;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const {
        if ( z_calibration_file )
            return boost::shared_ptr<DepthInfo>(
                new Measured3D( 
                    z_calibration_file(), dir, 
                    fluorophore_position(),
                    pixel_size(),
                    background(),
                    quantity<si::length>( sigma_smoothing_sigma() ) ) );
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
        push_back( background );
        push_back( sigma_smoothing_sigma );
    }
  public:
    Measured3DConfig()
        : simparm::Object("Measured3D", "Interpolated 3D"),
          z_calibration_file("ZCalibration", "Z calibration file"),
          fluorophore_position("CalibFluorophorePosition", "Fluorophore position in calibration image"),
          pixel_size("CalibPixelSize", "Pixel size in calibration image", 
            Measured3D::PixelSize::Constant(100 * si::nanometre / camera::pixel) ) ,
          background("CalibImageBackground", "Calibration image background", 0 * camera::ad_count ),
          sigma_smoothing_sigma("SigmaSmoothingSigma", "Smoothing factor for width estimation", 30 * si::nanometre)
    {
        registerNamedEntries();
    }
    Measured3DConfig( const Measured3DConfig& o )
        : simparm::Object(o), z_calibration_file(o.z_calibration_file),
          fluorophore_position(o.fluorophore_position),
          pixel_size(o.pixel_size), background(o.background),
          sigma_smoothing_sigma(o.sigma_smoothing_sigma) { registerNamedEntries(); }
    Measured3DConfig* clone() const { return new Measured3DConfig(*this); }
};

std::auto_ptr< Config > make_measured_3d_config()
    { return std::auto_ptr< Config >( new Measured3DConfig() ); }


}
}
