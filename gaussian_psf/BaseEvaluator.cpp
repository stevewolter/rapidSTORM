#include "debug.h"
#include "BaseEvaluator.h"
#include "expressions.h"
#include <nonlinfit/plane/GenericData.h>

namespace dStorm {
namespace gaussian_psf {

template <typename Number>
bool BaseParameters<Number>::prepare_iteration( const nonlinfit::plane::GenericData& data )
{ 
    if ( ! expr->form_parameters_are_sane() || 
         ! expr->mean_within_range(data.min, data.max) )
    {
        DEBUG("Start parameters are invalid");
        return false;
    }

    spatial_mean = expr->spatial_mean.template cast<Number>();
    amplitude = expr->amplitude;
    transmission = expr->transmission;
    sigma = compute_sigma_();
    double covariance_ellipse_area = 4 * M_PI * sigma[0] * sigma[1],
            pixel_size_in_sqmum = data.pixel_size;
    if ( (sigma < 0).any() || pixel_size_in_sqmum > covariance_ellipse_area ) {
        return false;
    }

    sigmaI = sigma.array().inverse();
    prefactor = data.pixel_size * amplitude * transmission 
            / (2 * M_PI * sigma.x() * sigma.y());
    compute_prefactors();
    return true;
}

template class BaseParameters< double >;
template class BaseParameters< float >;

}
}
