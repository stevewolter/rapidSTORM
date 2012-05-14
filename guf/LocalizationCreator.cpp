#include "debug.h"
#include <Eigen/StdVector>
#include <dStorm/engine/InputTraits.h>
#include "fit_window/Stack.h"
#include "fit_window/Plane.h"
#include "LocalizationCreator.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/Localization.h>
#include "Config.h"
#include "constant_background/model.hpp"
#include <boost/units/Eigen/Array>
#include <boost/variant/get.hpp>
#include "fit_window/Optics.h"
#include "MultiKernelModel.h"

namespace dStorm {
namespace guf {

LocalizationCreator::LocalizationCreator( const Config& config, const dStorm::engine::JobInfo& info )
: fluorophore(info.fluorophore), output_sigmas( config.output_sigmas() ), laempi_fit( config.laempi_fit() )
{
}

void LocalizationCreator::operator()( Localization& loc, const MultiKernelModelStack& pos, double chi_sq, const fit_window::Stack& data ) const
{
    DEBUG("Creating localization for fluorophore " << fluorophore << " from parameters " << parameters.transpose() );

    assert( ! pos.empty() );

    const int plane_count = pos.size();
    if ( pos.size() == 1 ) {
        write_parameters( loc, pos[0], chi_sq, data[0] );
    } else {
        /* TODO: Can weight_by_uncertainty be computed statically? */
        bool weight_by_uncertainty = true;
        std::vector<Localization> by_plane;
        for ( int plane = 0; plane < plane_count; ++plane ) {
            by_plane.push_back( Localization() );
            write_parameters( by_plane.back(), pos[plane], chi_sq, data[plane] );
            if ( ! data[plane].optics.can_compute_localization_precision() )
                weight_by_uncertainty = false;
        }
        join_localizations( loc, by_plane, weight_by_uncertainty );
    }
}

void LocalizationCreator::join_localizations( Localization& result, const std::vector<Localization>& by_plane, bool weight_by_uncertainty ) const
{
    result = by_plane[0];

    for (int d = 0; d < 2; ++d) {
        quantity< power_typeof_helper< si::length, static_rational<-1> >::type > accum
            = 0 / si::meter;
        quantity< power_typeof_helper< si::area, static_rational<-1> >::type > inv_variance,
            total_inv_variance = 0 / si::meter / si::meter;
        for (int p = 0; p < int( by_plane.size() ); ++p ) {
            if ( weight_by_uncertainty )
                inv_variance = pow<-2>( by_plane[p].position.uncertainty()[d] );
            else
                inv_variance = 1.0 / si::meter / si::meter;
            accum += by_plane[p].position()[d] * inv_variance;
            total_inv_variance += inv_variance;
        }
        if ( laempi_fit )
            result.position()[d] = accum / total_inv_variance;
        if ( weight_by_uncertainty )
            result.position.uncertainty()[d] = sqrt(1.0 / total_inv_variance);
        // TODO: This should probably be the total, pre-prefactor amplitude, and the others with PF
        result.amplitude() = by_plane[0].amplitude();
    }
    result.children = by_plane;
}

void LocalizationCreator::compute_uncertainty( Localization& rv, const MultiKernelModel& m, const fit_window::Plane& p )  const
{
    assert( m.kernel_count() == 1 );
    using namespace boost::units;
    /* Mortenson formula */
    /* Number of photons */
//    double N = m[0]( gaussian_psf::Amplitude() ) * m[0]( gaussian_psf::Prefactor() );
     double N = m[0].intensity();
    double B = m.background_model()( constant_background::Amount() );
    double background_variance =
        ( p.optics.background_is_poisson_distributed() )
        ? B
        : p.optics.background_noise_variance();
    /* Compute/get \sigma */
    for (int i = 0; i < 2; ++i) {
        quantity<si::area> psf_variance
            = rv.fit_covariance_matrix()(i,i) + p.pixel_size / 12.0;
        double background_term
            = psf_variance * 8.0 * M_PI * background_variance / (N * p.pixel_size);
        rv.position.uncertainty()[i]
            = sqrt( (psf_variance / N) * ( 16.0 / 9.0 + background_term ) );
    }
}

void LocalizationCreator::write_parameters( Localization& rv, const MultiKernelModel& m, double chi_sq, const fit_window::Plane& data ) const
{
    assert( m.kernel_count() == 1 );

    const guf::SingleKernelModel& only_kernel = m[0];
    Localization::Position::Type pos;
    assert( pos.rows() == 3 && pos.cols() == 1 );
    pos.x() = only_kernel.get_fluorophore_position(0);
    pos.y() = only_kernel.get_fluorophore_position(1);
    if ( only_kernel.has_z_position() )
        pos[2] = only_kernel.get_fluorophore_position(2);
    Localization::Amplitude::Type amp( only_kernel.get_amplitude() * data.optics.photon_response() );
    rv = Localization(pos, amp );

    rv.local_background() =
            quantity< camera::intensity, float >
                ( m.background_model()( constant_background::Amount() ) * data.optics.photon_response() );
    rv.fit_residues = chi_sq;
    if ( output_sigmas || data.optics.can_compute_localization_precision() )
        for (int i = 0; i < 2; ++i) {
            quantity<si::length,float> sigma(only_kernel.get_sigma()[i]);
            rv.fit_covariance_matrix()(i,i) = sigma*sigma;
        }

    if ( data.optics.can_compute_localization_precision() )
        compute_uncertainty( rv, m, data );

    rv.fluorophore = fluorophore;
}

}
}
