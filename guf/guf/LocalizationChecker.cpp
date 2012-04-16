#include "debug.h"
#include "LocalizationChecker.h"
#include <dStorm/engine/JobInfo.h>
#include "Config.h"
#include "guf/psf/BaseExpression.h"
#include "guf/psf/Base3D.h"
#include <dStorm/engine/InputTraits.h>
#include <dStorm/threed_info/DepthInfo.h>

namespace dStorm {
namespace guf {

using namespace boost::units;

LocalizationChecker::LocalizationChecker( const Config& config, const dStorm::engine::JobInfo& info )
: info(info),
  theta_dist( config.theta_dist() ),
  allowed_z_positions()
{
    for (int i = 0; i < info.traits.plane_count(); ++i) {
        allowed_z_positions += info.traits.optics(i).depth_info()->z_range();
    }
}

bool LocalizationChecker::operator()( const FitPosition& result, const guf::Spot& spot ) const
{
    bool makes_it_in_one_plane = false;
    for (FitPosition::const_iterator i = result.begin(); i != result.end(); ++i) {
        for ( FittedPlane::const_iterator j = i->begin(); j != i->end(); ++j ) {
            int plane = i - result.begin();
            if ( ! check_kernel(*j, spot, plane) ) return false;
            DEBUG("Checking amplitude threshold");
            quantity<camera::intensity> photon = 
                info.traits.optics(plane)
                    .photon_response.get_value_or( 1 * camera::ad_count );
            double local_threshold = info.amplitude_threshold / photon;
            if ( local_threshold < (*j)( PSF::Amplitude() ) * (*j)( PSF::Prefactor() ) )
                makes_it_in_one_plane = true;
            for ( FittedPlane::const_iterator k = j+1; k != i->end(); ++k ) {
                quantity<si::length> x_dist( (*j)( PSF::Mean<0>() ) - (*k)( PSF::Mean<0>() ) );
                quantity<si::length> y_dist( (*j)( PSF::Mean<1>() ) - (*k)( PSF::Mean<1>() ) );
                if ( pow<2>(x_dist) + pow<2>(y_dist) < pow<2>(theta_dist) )
                    return false;
            }
        }
    }
    DEBUG("Amplitude threshold is met: " << makes_it_in_one_plane);
    return makes_it_in_one_plane;
}

template <int Dim>
bool LocalizationChecker::check_kernel_dimension( const PSF::BaseExpression& k, const guf::Spot& spot, int plane ) const
{
    /* TODO: Make this 3.0 configurable */
    bool close_to_original = 
        abs( quantity<si::length>(k( PSF::Mean<Dim>() ) ) - spot[Dim] ) 
            < (*info.traits.optics(plane).psf_size( info.fluorophore ))[Dim] * 3.0f;
    DEBUG( "Result kernel is close to original in Dim " << Dim << ": " << close_to_original);
    return close_to_original;
}

bool LocalizationChecker::check_kernel( const PSF::BaseExpression& k, const guf::Spot& s, int plane ) const
{
    bool kernels_ok =
        check_kernel_dimension<0>(k,s, plane) &&
        check_kernel_dimension<1>(k,s, plane);
    const PSF::Base3D* threed = dynamic_cast<const PSF::Base3D*>( &k );
    bool z_ok = (!threed || 
        contains(allowed_z_positions, 
             samplepos::Scalar( (*threed)( PSF::MeanZ() ) ) ) );
    DEBUG("Z position " << (*threed)( PSF::MeanZ() ) << " OK: " << z_ok);
    return kernels_ok && z_ok;
}

}
}
