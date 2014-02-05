#ifndef NONLINFIT_IMAGE_CENTROID_IMPL_H
#define NONLINFIT_IMAGE_CENTROID_IMPL_H

#include "fit_window/Centroid.h"

namespace dStorm {
namespace fit_window {

Centroid& Centroid::operator+=( const Centroid& o ) 
{
    if ( ! min.is_initialized() ) min = o.min;
    else if ( o.min.is_initialized() ) min = min->max( *o.min );
    if ( ! max.is_initialized() ) max = o.max;
    else if ( o.max.is_initialized() ) max = max->min( *o.max );
    for (int i = 0; i < 2; ++i) {
        weighted_sum[i] += o.weighted_sum[i];
        total_weight[i] += o.total_weight[i];
        assert( weighted_sum[i].x() <= 1E40 );
    }
    return *this;
}

Centroid::Spot Centroid::current_position() const
{
    Spot positive_center, negative_center;
    for (int i = 0; i < 2; ++i) {
        positive_center[i] = weighted_sum[0][i] / total_weight[0],
        negative_center[i] = weighted_sum[1][i] / total_weight[1];
    }
    /* If there were neither positive nor negative pixels, just return the centers
     * of mass. */
    assert( total_weight[0] >= 1E-20 || total_weight[1] >= 1E-20 );
    if ( total_weight[0] < std::numeric_limits<double>::epsilon() )
        return negative_center;
    else if ( total_weight[1] < std::numeric_limits<double>::epsilon() )
        return positive_center;

    Spot naive_sum = (weighted_sum[0] - weighted_sum[1]);
    Spot naive_center_of_mass;
    for (int i = 0; i < 2; ++i)
        naive_center_of_mass[i] = naive_sum[i] / (total_weight[0] - total_weight[1]);
    Spot center_vector = positive_center - negative_center;
    Spot naive_center_distance = positive_center - naive_center_of_mass;

    Spot centroid;
    /* Avoid the singularity occuring when negative mass is approx. equal to positive mass.
        * This is actually the normal case in least-squares fitting. */
    if ( std::abs( total_weight[0] - total_weight[1] ) < 1E-5 ||
         center_vector.matrix().squaredNorm() < naive_center_distance.matrix().squaredNorm() )
    {
        for (int i = 0; i < 2; ++i)
            centroid[i] = 2.0 * positive_center[i] - negative_center[i];
    } else {
        centroid = naive_center_of_mass;
    }

    if ( max.is_initialized() && min.is_initialized() ) {
        Eigen::Array2d center = (*max + *min) / 2.0;
        Eigen::Array2d centroid_offset = centroid - center;
        float scale_factor = 1.0;
        for (int i = 0; i < 2; ++i) {
            if ( centroid[i] < (*min)[i] ) 
                scale_factor = std::min<float>( scale_factor,
                    ((*min)[i] - center[i]) / centroid_offset[i] );
            if ( centroid[i] > (*max)[i] ) 
                scale_factor = std::min<float>( scale_factor,
                    ((*max)[i] - center[i]) / centroid_offset[i] );
            assert( scale_factor >= 0.0 );
        }
        Eigen::Array2d scaled_centroid = center + centroid_offset * (scale_factor * 0.99);
        assert( (scaled_centroid >= (*min).array()).all() );
        assert( (scaled_centroid <= (*max).array()).all() );
        return scaled_centroid;
    } else {
        return centroid;
    }
}

}
}

#endif
