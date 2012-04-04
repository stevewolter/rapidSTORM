#include <Eigen/StdVector>
#include "Factory.h"
#include "Fitter.h"
#include "Config.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/output/Traits.h>
#include <boost/variant/get.hpp>
#include <dStorm/threed_info/equifocal_plane.h>

namespace dStorm {
namespace guf {

namespace spf = engine::spot_fitter;

Factory::Factory()
: spf::Factory( static_cast<Config&>(*this) ) {}
Factory::Factory(const Factory& c)
: simparm::Structure<Config>(c),
  spf::Factory( static_cast<Config&>(*this) ) 
{
}

Factory::~Factory() {}

bool Factory::can_do_3d( const input::Traits<engine::ImageStack>& t ) const
{
    bool do_3d = boost::get< traits::No3D >( t.optics(0).depth_info().get_ptr() ) == NULL;
    return do_3d;
}

std::auto_ptr<engine::spot_fitter::Implementation>
Factory::make( const engine::JobInfo& info )
{
    check_configuration( info );
    return std::auto_ptr<engine::spot_fitter::Implementation>(
        new Fitter( info, *this) );
}

void Factory::set_traits( output::Traits& traits, const engine::JobInfo& info )
{

    traits.covariance_matrix().is_given.diagonal().fill( output_sigmas() );
    traits.position().is_given.head<2>().fill( true );
    traits.position().is_given[2] = can_do_3d( info.traits );
    traits.amplitude().is_given= true;
    traits.fit_residues().is_given= true;
    traits.local_background().is_given = (info.traits.plane_count() < 1);
    traits.fluorophore().is_given = true;
    traits.two_kernel_improvement().is_given= two_kernel_fitting();

    z_range.viewable = traits.position().is_given[2];
    laempi_fit.viewable = info.traits.plane_count() > 1;
    disjoint_amplitudes.viewable = info.traits.plane_count() > 1;

    if ( info.traits.optics(0).z_position ) {
        quantity<si::length> equifocal_first = 
                equifocal_plane( info.traits.optics(0) );
        traits.position().range().z().first = samplepos::Scalar(equifocal_first-quantity<si::length>(z_range()));
        traits.position().range().z().second = samplepos::Scalar(equifocal_first+quantity<si::length>(z_range()));
        for (int i = 1; i < info.traits.plane_count(); ++i)
        {
            quantity<si::length> equifocal = 
                equifocal_plane( info.traits.optics(i) );
            samplepos::Scalar low = samplepos::Scalar(equifocal -samplepos::Scalar( z_range())),
                            high = samplepos::Scalar(equifocal +samplepos::Scalar(z_range()));
            traits.position().range().z().first = std::min( low, *traits.position().range().z().first );
            traits.position().range().z().second = std::max( high, *traits.position().range().z().second );
        }
    }

    bool all_uncertainties_given = can_compute_uncertainty( info.traits.plane(0) );
    traits.source_traits.clear();
    if ( info.traits.plane_count() > 1 ) {
        boost::shared_ptr< input::Traits<Localization> > p( new input::Traits<Localization>() );
        p->local_background().is_given = true;
        p->amplitude().is_given = disjoint_amplitudes();
        p->position().is_given.head<2>().fill( laempi_fit() );
        p->repetitions = info.traits.plane_count();
        for (int i = 0; i < info.traits.plane_count(); ++i ) {
            bool have_uncertainty = can_compute_uncertainty( info.traits.plane(i) );
            all_uncertainties_given = all_uncertainties_given && have_uncertainty;
        }
        p->position().uncertainty_is_given.head<2>().fill( all_uncertainties_given );
        traits.source_traits.push_back( p );
    }

    traits.position().uncertainty_is_given.head<2>().fill( all_uncertainties_given );
    my_traits = traits;
}

bool Factory::can_compute_uncertainty( const engine::InputPlane& t ) const
{
    return t.optics.photon_response.is_initialized() && 
           t.optics.background_stddev.is_initialized();
}

void Factory::set_requirements( input::Traits<engine::ImageStack>& ) {}

void Factory::register_trait_changing_nodes( simparm::Listener& l )
{
    l.receive_changes_from( z_range.value );
    l.receive_changes_from( free_sigmas.value );
    l.receive_changes_from( output_sigmas.value );
    l.receive_changes_from( laempi_fit.value );
    l.receive_changes_from( disjoint_amplitudes.value );
}

void Factory::check_configuration( 
    const engine::JobInfo& info
) {
    bool spectral_unmixing = true;
    for (int i = 0; i < info.traits.plane_count(); ++i)
        spectral_unmixing = spectral_unmixing && std::abs( info.traits.optics(i)
            .transmission_coefficient(info.fluorophore) ) > 0.01;
    if ( laempi_fit() && ! spectral_unmixing )
        throw std::runtime_error("You asked for a Lämpi fit, but some "
            "transmission coefficients are zero. This will not work.");
    if ( disjoint_amplitudes() && ! spectral_unmixing )
        throw std::runtime_error("You asked for a disjoint amplitude fit, but "
            "some transmission coefficients are zero. This will not work.");

    if ( free_sigmas() && two_kernel_fitting() ) {
        throw std::runtime_error(
            "You have requested both free sigma fitting and two kernel fitting. "
            "This is madness. This is not Sparta. Please disable one of these "
            "options.");
    }
    if ( info.traits.plane_count() > Config::maximum_plane_count )
        throw std::runtime_error(
            "You have requested more input layers than hell has circles. "
            "Sorry, this is not implemented. Please send a bug report, "
            "a photo of your setup and of the poor lad who had to calibrate it.");
    if ( laempi_fit() && two_kernel_fitting() ) {
        /* Note: If for some dirty reason this needs to be implemented, be sure to
        * also fix the bitfield construction in NaiveFitter::NaiveFitter */
        throw std::runtime_error("Enabling " + laempi_fit.getDesc() + " and " +
            two_kernel_fitting.getDesc() + " at the same time doesn't work, sorry");
    }
    if ( disjoint_amplitudes() && two_kernel_fitting() ) {
        /* Note: If for some dirty reason this needs to be implemented, be sure to
        * also fix the bitfield construction in NaiveFitter::NaiveFitter */
        throw std::runtime_error("Enabling " + disjoint_amplitudes.getDesc() + " and " +
            two_kernel_fitting.getDesc() + " at the same time doesn't work, sorry");
    }

    bool can_do_mle = true;
    for(int i = 0; i < info.traits.plane_count(); ++i) {
        can_do_mle = can_do_mle
            && info.traits.optics(i).dark_current.is_initialized() 
            && info.traits.optics(i).photon_response.is_initialized() ;
    }
    if ( ! can_do_mle && mle_fitting() ) {
        throw std::runtime_error("Enabling " + mle_fitting.getDesc() + " requires "
            "that you have set the dark intensity and the photon response for all "
            "layers");
    }
}

}
}
