#include "debug.h"
#include "LocalizationChecker.h"
#include <dStorm/engine/JobInfo.h>
#include "Config.h"
#include <dStorm/engine/InputTraits.h>
#include <dStorm/threed_info/DepthInfo.h>
#include "MultiKernelModel.h"

namespace dStorm {
namespace guf {

using namespace boost::units;

LocalizationChecker::LocalizationChecker( const Config& config, const dStorm::engine::JobInfo& info )
: info(info),
  theta_dist( config.theta_dist() ),
  allowed_z_positions()
{
    for (int i = 0; i < info.traits.plane_count(); ++i) {
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir)
            allowed_z_positions += info.traits.optics(i).depth_info(dir)->z_range();
    }
}

bool LocalizationChecker::operator()( const MultiKernelModelStack& result, const guf::Spot& spot ) const
{
    bool makes_it_in_one_plane = false;
    for (MultiKernelModelStack::const_iterator i = result.begin(); i != result.end(); ++i) {
        for ( MultiKernelModel::const_iterator j = i->begin(); j != i->end(); ++j ) {
            int plane = i - result.begin();
            if ( ! check_kernel(*j, spot, plane) ) return false;
            quantity<camera::intensity> photon =
                info.traits.optics(plane)
                    .photon_response.get_value_or( 1 * camera::ad_count );
            double local_threshold = info.amplitude_threshold / photon;
            double intensity = (*j).intensity();
            DEBUG("Checking amplitude threshold " << local_threshold << " against value " << intensity);
            if ( local_threshold < intensity )
                makes_it_in_one_plane = true;
            for ( MultiKernelModel::const_iterator k = j+1; k != i->end(); ++k ) {
                quantity<si::length> x_dist( (*j).get_fluorophore_position(0) - (*k).get_fluorophore_position(0));
                quantity<si::length> y_dist( (*j).get_fluorophore_position(1) - (*k).get_fluorophore_position(1) );
                if ( pow<2>(x_dist) + pow<2>(y_dist) < pow<2>(theta_dist) )
                    return false;
            }
        }
    }
    DEBUG("Amplitude threshold is met: " << makes_it_in_one_plane);
    return makes_it_in_one_plane;
}

bool LocalizationChecker::check_kernel_dimension( const guf::SingleKernelModel& k, const guf::Spot& spot, int plane, int Dim ) const
{
    bool close_to_original =
        abs( k.get_fluorophore_position(Dim)- spot[Dim] )
            < quantity<si::length>(k.get_sigma()[Dim] * 3.0);

    DEBUG( "Result kernel " <<  k.get_fluorophore_position(Dim) << " is close to original " 
           << spot[Dim] << " in Dim " << Dim << " with respect to std.dev. "
           << quantity<si::length>(k.get_sigma()[Dim]) << ": " << close_to_original);
    return close_to_original;
}

bool LocalizationChecker::check_kernel( const guf::SingleKernelModel& k, const guf::Spot& s, int plane ) const
{
    bool kernels_ok =
        check_kernel_dimension(k,s, plane, 0) &&
        check_kernel_dimension(k,s, plane, 1);
    bool has_z_position = k.has_z_position();
    bool z_ok = ( ! has_z_position ||
        contains(allowed_z_positions, samplepos::Scalar(k.get_fluorophore_position(2) ) ));
    DEBUG( "Z is OK: " << z_ok << " " << has_z_position );
    return kernels_ok && z_ok;
}

}
}
