#include "pixelatedBessel.h"
#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf_bessel.h>
#include <Eigen/Core>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <complex>
#include <gsl/gsl_errno.h>
#include <dStorm/traits/Projection.h>
#include <dStorm/threed_info/DepthInfo.h>
#include <dStorm/threed_info/Lens3D.h>

namespace input_simulation {

struct BesselFunction::IntegrationInfo
{
    Subpixel orig_position;
    Subpixel eval_position;
    const BesselFunction* function;
    const int integration_size;
    gsl_integration_workspace* const x;
    gsl_integration_workspace* const y;
    gsl_integration_workspace* const theta;
    gsl_integration_workspace* const lambda;
    double theta_max;

    quantity< power_typeof_helper< si::length, static_rational<-1> >::type > 
        wavenumber;
    double bessel_factor, z_offset;
    quantity<si::length> delta_z;

    IntegrationInfo() 
        : integration_size(10000),
          x( gsl_integration_workspace_alloc(integration_size) ),
          y( gsl_integration_workspace_alloc(integration_size) ),
          theta( gsl_integration_workspace_alloc(integration_size) ),
          lambda( gsl_integration_workspace_alloc(integration_size) ) {}
    ~IntegrationInfo() {
        gsl_integration_workspace_free( x );
        gsl_integration_workspace_free( y );
        gsl_integration_workspace_free( theta );
        gsl_integration_workspace_free( lambda );
    }
};

BesselFunction::BesselFunction( 
        const dStorm::engine::InputPlane& transformation_into_sample_space,
        const dStorm::samplepos& fluorophore_position_in_sample_space,
        double num_apert, double opt_density, quantity<si::length> wavelength,
        quantity<si::area> pixel_size )
        : trafo( transformation_into_sample_space ),
          fluorophore( fluorophore_position_in_sample_space ),
          na(num_apert), n( opt_density ), theta_max( asin( na / n ) ),
          lambda( wavelength ), 
          pixel_size( pixel_size ),
          int_info( new IntegrationInfo() )
{
}

/* PSF computation done with Dusch et al, Journal of Microscopy 2007, Vol. 228 pg. 132, Eq. 4.
 * Added a 2 PI hack because it was necessary to yield sensible widths. */

template <bool imag_part>
double BesselFunction::theta_callback(double theta, void *params) 
{
    IntegrationInfo *i = static_cast<IntegrationInfo*>(params);

    double cos_theta = cos(theta);
    std::complex<double> rv = std::complex<double>( 
        sqrt(cos_theta) * sin(theta) * i->wavenumber.value() * gsl_sf_bessel_J0( sin(theta) * i->bessel_factor ), 0.0 )
        * exp( std::complex<double>(0, cos_theta * i->z_offset ) );

    if ( imag_part )
        return imag(rv);
    else
        return real(rv);
}

double BesselFunction::compute_point( const Subpixel& position, IntegrationInfo& info ) const
{
    dStorm::samplepos sample;
    sample.head<2>() =
        trafo.projection().point_in_sample_space(
            dStorm::traits::Projection::SubpixelImagePosition(
                position.head<2>()));
    sample -= fluorophore;
    quantity<si::length> distance = sqrt( pow<2>(sample[0]) + pow<2>(sample[1]) );
    info.bessel_factor = double( distance * info.wavenumber);

    gsl_function func;
    func.params = &info;

    double relerr = 1E-3;
    double abserr, real_part, imag_part, value;
    func.function = &BesselFunction::theta_callback<false>;
    int status;
    do {
        status = gsl_integration_qags( &func, 0, theta_max, 0, relerr,
                            info.integration_size, info.theta, &real_part, &abserr );
        relerr *= 1.1;
    } while ( status );

    /* Optimization: If detection plane and fluorophore plane are identical,
     * just integrate the real part because the exp(z) term is always unit 1.
     * Actually, gsf_bessel_J1 might suffice here, but I'm too lazy to do the math. */
    if ( std::abs( info.z_offset ) <= 1E-3 ) {
        imag_part = 0;
    } else {
        func.function = &BesselFunction::theta_callback<true>;
        relerr = 1E-3;
        do {
            status = gsl_integration_qags( &func, 0, theta_max, 0, relerr, 
                                info.integration_size, info.theta, &imag_part, &abserr );
            relerr *= 1.1;
        } while ( status );
    }

    value = norm( std::complex<double>(real_part, imag_part) );
    return (value * pixel_size).value();
}

double BesselFunction::point_callback(double x, void *params) 
{
    IntegrationInfo *i = static_cast<IntegrationInfo*>(params);
    i->eval_position.x() = x * camera::pixel;
    return i->function->compute_point( i->eval_position, *i );
}

double BesselFunction::line_integration_callback(double y, void *params) 
{
    IntegrationInfo *i = static_cast<IntegrationInfo*>(params);
    i->eval_position.y() = y * camera::pixel;
    double xc = i->orig_position.x().value();

    double val = 0, abserr;

    gsl_function func;
    func.function = &BesselFunction::point_callback;
    func.params = i;

    int status = gsl_integration_qags( &func, xc-0.5, xc+0.5, 0, 1E-3,
                         i->integration_size, i->x, &val, &abserr );
    assert( ! status );

    return val;
}

double BesselFunction::wavelength_callback(double l, void *params) 
{
    IntegrationInfo *i = static_cast<IntegrationInfo*>(params);
    quantity<si::length> lambda = l * si::meter;
    i->wavenumber = (i->function->n * 2 * M_PI / lambda);
    i->z_offset = -1.0 * (i->delta_z * i->wavenumber);

    double yc = i->orig_position.y().value(), val, abserr;

    gsl_function func;
    func.function = &BesselFunction::line_integration_callback;
    func.params = i;

    int status = gsl_integration_qags( &func, yc-0.5, yc+0.5, 0, 1E-3,
        i->integration_size, i->y, &val, &abserr );
    assert( ! status );

    return val;
}

double BesselFunction::integrate( const Subpixel& pixel_center) const 
{
    gsl_error_handler_t * old_handler=gsl_set_error_handler_off();
    double val = -1, abserr;
    dStorm::threed_info::ZPosition plane_z
        = dynamic_cast<const dStorm::threed_info::Lens3D&>
                (*trafo.optics.depth_info(dStorm::Direction_X))
            .z_position();
    int_info->delta_z = plane_z * 1E-6f * si::meter - fluorophore.z();

    int_info->orig_position = pixel_center;
    int_info->function = this;

    gsl_function func;
    func.function = &BesselFunction::wavelength_callback;
    func.params = int_info.get();

    if ( false ) {
        int status = gsl_integration_qags( &func, lambda.value()-15E-9, lambda.value()+15E-9, 0, 1E-3,
            int_info->integration_size, int_info->lambda, &val, &abserr );

        assert( ! status );
        gsl_set_error_handler(old_handler);
        return val / 30E-9;
    } else {
        double val = wavelength_callback( lambda.value(), func.params );
        return val;
    }
}

}
