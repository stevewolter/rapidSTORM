#include "debug.h"
#include "BaseEvaluator.h"
#include "expressions.h"
#include <nonlinfit/plane/GenericData.h>

namespace dStorm {
namespace gaussian_psf {

template <typename Number>
bool BaseParameters<Number>::prepare_iteration( const Data& data )
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
    boost::optional< Eigen::Array<Number,2,1> > new_sigma = compute_sigma_();
    if ( new_sigma ) {
        sigma = *new_sigma;
        double covariance_ellipse_area = 4 * M_PI * sigma[0] * sigma[1],
               pixel_size_in_sqmum = quantity< BaseExpression::PixelSizeUnit >(data.pixel_size).value();
        if ( pixel_size_in_sqmum > covariance_ellipse_area )
            return false;
    } else {
        DEBUG("Unable to compute sigma");
        return false;
    }
    assert( (sigma == sigma).all() && (sigma > 0).all() );
    DEBUG("Sigma is " << sigma.transpose());
    sigmaI = sigma.array().inverse();
    prefactor = data.pixel_size.value() * amplitude * transmission 
            / (2 * M_PI * sigma.x() * sigma.y());
    compute_prefactors();
    return true;
}

template class BaseParameters< double >;
template class BaseParameters< float >;

}
}
