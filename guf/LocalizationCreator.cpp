#include <boost/math/constants/constants.hpp>

#include "debug.h"
#include <Eigen/StdVector>
#include "engine/InputTraits.h"
#include "fit_window/Plane.h"
#include "guf/LocalizationCreator.h"
#include "engine/JobInfo.h"
#include "Localization.h"
#include "guf/Config.h"
#include "gaussian_psf/BaseExpression.h"
#include "gaussian_psf/Base3D.h"
#include "constant_background/model.hpp"
#include <boost/variant/get.hpp>
#include "fit_window/Optics.h"
#include "guf/MultiKernelModel.h"

namespace dStorm {
namespace guf {

LocalizationCreator::LocalizationCreator( const Config& config, const dStorm::engine::JobInfo& info )
: fluorophore(info.fluorophore), output_sigmas( config.output_sigmas() ), laempi_fit( config.laempi_fit() )
{
}

void LocalizationCreator::operator()( Localization& loc, const MultiKernelModelStack& pos, double chi_sq, const fit_window::PlaneStack& data ) const
{
    assert( ! pos.empty() );
    write_parameters( loc, pos[0], chi_sq, data[0] );
}

void LocalizationCreator::compute_uncertainty( Localization& rv, const MultiKernelModel& m, const fit_window::Plane& p )  const
{
    assert( m.kernel_count() == 1 );
    /* Mortenson formula */
    /* Number of photons */
    double N = m[0]( gaussian_psf::Amplitude() ) * m[0]( gaussian_psf::Prefactor() );
    double B = m.background_model()( constant_background::Amount() );
    double background_variance = 
        ( p.optics->background_is_poisson_distributed() )
        ? B
        : p.optics->background_noise_variance();
    /* Compute/get \sigma */
    for (int i = 0; i < 2; ++i) {
        double psf_variance 
            = quantity<si::area>(pow<2>( rv.psf_width(i) / 2.35 )).value() * 1E12 + p.pixel_size / 12.0;
        double background_term
            = psf_variance * 8.0 * boost::math::constants::pi<double>() * background_variance / (N * p.pixel_size);
        rv.set_position_uncertainty(i,
            samplepos::Scalar(sqrt( (psf_variance / N) * ( 16.0 / 9.0 + background_term ) ) * 1E-6 * si::meter));
    }
}

void LocalizationCreator::write_parameters( Localization& rv, const MultiKernelModel& m, double chi_sq, const fit_window::Plane& data ) const
{
    assert( m.kernel_count() == 1 );

    const gaussian_psf::BaseExpression& only_kernel = m[0];

    samplepos pos;
    assert( pos.rows() == 3 && pos.cols() == 1 );
    pos.x() = only_kernel( gaussian_psf::Mean<0>() ) * 1E-6 * si::meter;
    pos.y() = only_kernel( gaussian_psf::Mean<1>() ) * 1E-6 * si::meter;
    const gaussian_psf::Base3D* threed = dynamic_cast<const gaussian_psf::Base3D*>( &only_kernel );
    if ( threed )
        pos[2] = (*threed)( gaussian_psf::MeanZ() ) * 1E-6 * si::meter;
    localization::Amplitude::ValueType amp( only_kernel( gaussian_psf::Amplitude() ) * data.optics->photon_response() );
    rv = Localization(pos, amp );

    rv.local_background() = 
            quantity< camera::intensity, float >
                ( m.background_model()( constant_background::Amount() ) * data.optics->photon_response() );
    rv.fit_residues = chi_sq;
    if ( output_sigmas || data.optics->can_compute_localization_precision() ) {
        rv.psf_width_x() = quantity<si::length,float>(only_kernel.get_sigma()[0] * 1E-6 * si::meter) * 2.35f;
        rv.psf_width_y() = quantity<si::length,float>(only_kernel.get_sigma()[1] * 1E-6 * si::meter) * 2.35f;
    }

    if ( data.optics->can_compute_localization_precision() )
        compute_uncertainty( rv, m, data );

    rv.fluorophore = fluorophore;
}

}
}
