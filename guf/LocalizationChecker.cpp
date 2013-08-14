#include <dStorm/engine/InputTraits.h>
#include "debug.h"
#include "LocalizationChecker.h"
#include <dStorm/engine/JobInfo.h>
#include "Config.h"
#include "gaussian_psf/BaseExpression.h"
#include "gaussian_psf/Base3D.h"
#include "constant_background/model.hpp"
#include <dStorm/threed_info/DepthInfo.h>
#include "MultiKernelModel.h"
#include <dStorm/engine/FitJudger.h>
#include "dStorm/LengthUnit.h"

namespace dStorm {
namespace guf {

LocalizationChecker::LocalizationChecker( const Config& config, const dStorm::engine::JobInfo& info )
: info(info),
  theta_dist_sq( pow(ToLengthUnit(config.theta_dist()), 2.0) ),
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
            double intensity = (*j)( gaussian_psf::Amplitude() ) * (*j)( gaussian_psf::Prefactor() );
            double background = i->background_model()( constant_background::Amount() );
            DEBUG("Checking amplitude threshold of " << local_threshold << " against " << intensity);
            if ( info.get_judger( plane ).is_above_background( intensity, background ) )
                makes_it_in_one_plane = true;
            for ( MultiKernelModel::const_iterator k = j+1; k != i->end(); ++k ) {
                double dist_sq = (j->get<gaussian_psf::Mean>() - k->get<gaussian_psf::Mean>()).squaredNorm();
                if ( dist_sq < theta_dist_sq )
                    return false;
            }
        }
    }
    DEBUG("Amplitude threshold is met: " << makes_it_in_one_plane);
    return makes_it_in_one_plane;
}

template <int Dim>
bool LocalizationChecker::check_kernel_dimension( const gaussian_psf::BaseExpression& k, const guf::Spot& spot, int plane ) const
{
    /* TODO: Make this 3.0 configurable */
    bool close_to_original = 
        abs( k( gaussian_psf::Mean<Dim>() ) - spot[Dim] ) 
            < k.get_sigma()[Dim] * 3.0;
    DEBUG( "Result kernel is close to original in Dim " << Dim << ": " << close_to_original);
    return close_to_original;
}

bool LocalizationChecker::check_kernel( const gaussian_psf::BaseExpression& k, const guf::Spot& s, int plane ) const
{
    bool kernels_ok =
        check_kernel_dimension<0>(k,s, plane) &&
        check_kernel_dimension<1>(k,s, plane);
    const gaussian_psf::Base3D* threed = dynamic_cast<const gaussian_psf::Base3D*>( &k );
    bool z_ok = (!threed || 
        contains(allowed_z_positions, 
             samplepos::Scalar( (*threed)( gaussian_psf::MeanZ() ) * 1E-6 * si::meter ) ) );
    DEBUG("Z position " << (*threed)( gaussian_psf::MeanZ() ) << " OK: " << z_ok);
    return kernels_ok && z_ok;
}

}
}
