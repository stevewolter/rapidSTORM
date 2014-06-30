#include "engine/InputTraits.h"
#include "debug.h"
#include "guf/LocalizationChecker.h"
#include "engine/JobInfo.h"
#include "guf/Config.h"
#include "gaussian_psf/BaseExpression.h"
#include "gaussian_psf/Base3D.h"
#include "constant_background/model.hpp"
#include "threed_info/DepthInfo.h"
#include "guf/MultiKernelModel.h"
#include "engine/FitJudger.h"
#include "LengthUnit.h"

namespace dStorm {
namespace guf {

LocalizationChecker::LocalizationChecker( const Config& config, const dStorm::engine::JobInfo& info )
: info(info),
  spot_distance_threshold(
          pow(ToLengthUnit(config.maximum_distance_from_spot()), 2.0)),
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

bool LocalizationChecker::check_kernel( const gaussian_psf::BaseExpression& k, const guf::Spot& s, int plane ) const
{
    Eigen::Array2d final_distance;
    final_distance.x() = k( gaussian_psf::Mean<0>() ) - s.x();
    final_distance.y() = k( gaussian_psf::Mean<1>() ) - s.y();
    if (final_distance.abs().matrix().squaredNorm() > spot_distance_threshold) {
        return false;
    }

    const gaussian_psf::Base3D* threed = dynamic_cast<const gaussian_psf::Base3D*>( &k );
    return !threed || contains(allowed_z_positions,
                               (*threed)(gaussian_psf::MeanZ()));
}

}
}
