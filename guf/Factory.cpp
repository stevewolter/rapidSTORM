#include <Eigen/StdVector>
#include "guf/Factory.h"
#include "guf/Fitter.h"
#include "guf/Config.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/output/Traits.h>
#include <boost/variant/get.hpp>
#include <dStorm/threed_info/DepthInfo.h>

namespace dStorm {
namespace guf {

namespace spf = engine::spot_fitter;

std::auto_ptr<engine::spot_fitter::Implementation>
Factory::make( const engine::JobInfo& info )
{
    check_configuration( info );
    return std::auto_ptr<engine::spot_fitter::Implementation>(
        new Fitter( info, config) );
}

void Factory::set_traits( output::Traits& traits, const engine::JobInfo& info )
{
    bool have_z_information = false;
    threed_info::ZRange z_range;
    for (int i = 0; i < info.traits.plane_count(); ++i) {
        for ( Direction dir = Direction_First; dir != Direction_2D; ++dir )
            if ( info.traits.optics(i).depth_info(dir).get() ) {
                z_range += info.traits.optics(i).depth_info(dir)->z_range();
                have_z_information = have_z_information || info.traits.optics(i).depth_info(dir)->provides_3d_info();
            }
    }
    if ( ! is_empty( z_range ) ) {
        traits.position_z().range().first = float(lower( z_range ) * 1E-6) * si::meter;
        traits.position_z().range().second = float(upper( z_range ) * 1E-6) * si::meter;
    }

    traits.psf_width_x().is_given = config.output_sigmas();
    traits.psf_width_y().is_given = config.output_sigmas();
    traits.position_x().is_given = true;
    traits.position_y().is_given = true;
    traits.position_z().is_given = have_z_information;
    traits.amplitude().is_given = true;
    traits.fit_residues().is_given = true;
    traits.local_background().is_given = (info.traits.plane_count() <= 1);
    traits.fluorophore().is_given = true;
    traits.two_kernel_improvement().is_given= config.two_kernel_fitting();

    config.laempi_fit.set_visibility( info.traits.plane_count() > 1 );
    config.disjoint_amplitudes.set_visibility( info.traits.plane_count() > 1 );

    bool all_uncertainties_given = can_compute_uncertainty( info.traits.plane(0) );
    traits.source_traits.clear();
    if ( info.traits.plane_count() > 1 ) {
        boost::shared_ptr< input::Traits<Localization> > p( new input::Traits<Localization>() );
        p->local_background().is_given = true;
        p->amplitude().is_given = config.disjoint_amplitudes();
        p->position_x().is_given = config.laempi_fit();
        p->position_y().is_given = config.laempi_fit();
        p->repetitions = info.traits.plane_count();
        for (int i = 0; i < info.traits.plane_count(); ++i ) {
            bool have_uncertainty = can_compute_uncertainty( info.traits.plane(i) );
            all_uncertainties_given = all_uncertainties_given && have_uncertainty;
        }
        p->position_uncertainty_x().is_given = all_uncertainties_given;
        p->position_uncertainty_y().is_given = all_uncertainties_given;
        p->position_uncertainty_z().is_given = false;
        traits.source_traits.push_back( p );
    }

    traits.position_uncertainty_x().is_given = all_uncertainties_given;
    traits.position_uncertainty_y().is_given = all_uncertainties_given;
    traits.position_uncertainty_z().is_given = false;
    my_traits = traits;
}

bool Factory::can_compute_uncertainty( const engine::InputPlane& t ) const
{
    return t.optics.photon_response.is_initialized() && 
           t.optics.dark_current.is_initialized();
}

void Factory::set_requirements( input::Traits<engine::ImageStack>& ) {}

void Factory::attach_ui( simparm::NodeHandle to ) {
    config.attach_ui( to );

    listening[0] = config.free_sigmas.value.notify_on_value_change( traits_changed );
    listening[1] = config.output_sigmas.value.notify_on_value_change( traits_changed );
    listening[2] = config.laempi_fit.value.notify_on_value_change( traits_changed );
    listening[3] = config.disjoint_amplitudes.value.notify_on_value_change( traits_changed );
}

void Factory::register_trait_changing_nodes( simparm::BaseAttribute::Listener l )
{
    traits_changed.connect(l);
}

void Factory::check_configuration( 
    const engine::JobInfo& info
) {
    bool spectral_unmixing = true;
    for (int i = 0; i < info.traits.plane_count(); ++i)
        spectral_unmixing = spectral_unmixing && std::abs( info.traits.optics(i)
            .transmission_coefficient(info.fluorophore) ) > 0.01;
    if ( config.laempi_fit() && ! spectral_unmixing )
        throw std::runtime_error("You asked for a Lämpi fit, but some "
            "transmission coefficients are zero. This will not work.");
    if ( config.disjoint_amplitudes() && ! spectral_unmixing )
        throw std::runtime_error("You asked for a disjoint amplitude fit, but "
            "some transmission coefficients are zero. This will not work.");

    if ( config.free_sigmas() && config.two_kernel_fitting() ) {
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
    if ( config.laempi_fit() && config.two_kernel_fitting() ) {
        /* Note: If for some dirty reason this needs to be implemented, be sure to
        * also fix the bitfield construction in NaiveFitter::NaiveFitter */
        throw std::runtime_error("Enabling " + config.laempi_fit.getDesc() + " and " +
            config.two_kernel_fitting.getDesc() + " at the same time doesn't work, sorry");
    }
    if ( config.disjoint_amplitudes() && config.two_kernel_fitting() ) {
        /* Note: If for some dirty reason this needs to be implemented, be sure to
        * also fix the bitfield construction in NaiveFitter::NaiveFitter */
        throw std::runtime_error("Enabling " + config.disjoint_amplitudes.getDesc() + " and " +
            config.two_kernel_fitting.getDesc() + " at the same time doesn't work, sorry");
    }

    bool can_do_mle = true;
    for(int i = 0; i < info.traits.plane_count(); ++i) {
        can_do_mle = can_do_mle
            && info.traits.optics(i).dark_current.is_initialized() 
            && info.traits.optics(i).photon_response.is_initialized() ;
    }
    if ( ! can_do_mle && config.mle_fitting() ) {
        throw std::runtime_error("Enabling " + config.mle_fitting.getDesc() + " requires "
            "that you have set the dark intensity and the photon response for all "
            "layers");
    }
}

}
}
