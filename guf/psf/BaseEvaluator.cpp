#include "debug.h"
#include "BaseEvaluator.h"
#include "expressions.h"
#include "Polynomial3D.h"
#include "No3D.h"
#include <nonlinfit/plane/GenericData.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Number>
bool BaseParameters<Number>::prepare_iteration( const Data& data )
{ 
    if ( ! expr->form_parameters_are_sane() || 
            ! expr->mean_within_range(data.min, data.max) ||
            expr->sigma_is_negligible( data.pixel_size ) )
    {
        DEBUG("Start parameters are invalid");
        return false;
    }

    spatial_mean = expr->spatial_mean.template cast<Number>();
    amplitude = expr->amplitude;
    transmission = expr->transmission;
    boost::optional< Eigen::Array<Number,2,1> > new_sigma = compute_sigma_();
    if ( new_sigma )
        sigma = *new_sigma;
    else {
        DEBUG("Unable to compute sigma");
        return false;
    }
    assert( (sigma == sigma).all() );
    DEBUG("Sigma is " << sigma.transpose());
    sigmaI = sigma.array().inverse();
    prefactor = data.pixel_size.value() * amplitude * transmission 
            / (2 * M_PI * sigma.x() * sigma.y());
    compute_prefactors();
    return true;
}

template <typename Number>
boost::optional< Eigen::Array<Number,2,1> >
Parameters<Number,No3D>::compute_sigma_() {
        return Eigen::Array<Number,2,1>( expr->best_sigma.cast<Number>() );
}

template <typename Number>
boost::optional< Eigen::Array<Number,2,1> > Parameters<Number,Polynomial3D>::compute_sigma_() {
    relative_z = expr->zposition.array().cast<Number>() - Eigen::Array<Number,2,1>::Constant( expr->axial_mean );
    threed_factor = Eigen::Array<Number,2,1>::Constant(1);
    for (int term = 1; term <= Polynomial3D::Order; ++term)
        threed_factor += (relative_z / expr->delta_sigma.col(term).cast<Number>()).pow(term);
    DEBUG("Computed threed factor of " << threed_factor.transpose() << " from " << expr->delta_sigma.transpose() << " and " << relative_z.transpose());
    if ( (threed_factor < 0).any() )
        return boost::optional< Eigen::Array<Number,2,1> >();
    else
        return Eigen::Array<Number,2,1>(expr->best_sigma.array().cast< Number >() * threed_factor.sqrt());
}

template <typename Number>
void Parameters<Number,Polynomial3D>::compute_prefactors_() {
    z_deriv_prefactor.fill(0);
    Eigen::Array<Number,2,1> p = 0.5 * threed_factor.inverse();
    for (int term = 1; term <= Polynomial3D::Order; ++term) {
        delta_z_deriv_prefactor.col(term) =
            - p * term * relative_z.pow(term) / expr->delta_sigma.col(term).cast<Number>().pow(term+1);
        z_deriv_prefactor += - delta_z_deriv_prefactor.col(term)
            * expr->delta_sigma.col(term).cast<Number>() / relative_z;
    }
    this->sigma_deriv = expr->best_sigma.cast<Number>().inverse();
}

template <typename Number>
void Parameters<Number,No3D>::compute_prefactors_() {
    this->sigma_deriv = expr->best_sigma.cast<Number>().inverse();
}

template class BaseParameters< double >;
template class BaseParameters< float >;
template class Parameters< double, PSF::No3D >;
template class Parameters< double, PSF::Polynomial3D >;
template class Parameters< float, PSF::No3D >;
template class Parameters< float, PSF::Polynomial3D >;

}
}
}
