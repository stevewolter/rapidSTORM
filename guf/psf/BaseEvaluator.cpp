#include "debug.h"
#include "BaseEvaluator.h"
#include "expressions.h"
#include "Zhuang.h"
#include "No3D.h"
#include <nonlinfit/plane/GenericData.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Number, typename Expression>
bool BaseEvaluator<Number,Expression>::prepare_iteration( const Data& data )
{ 
    if ( ! expr->form_parameters_are_sane() || 
            ! expr->mean_within_range(data.min, data.max) ||
            expr->sigma_is_negligible( data.pixel_size ) )
        return false;

    spatial_mean = expr->spatial_mean.template cast<Number>();
    amplitude = expr->amplitude;
    wavelength = expr->wavelength;
    transmission = expr->transmission;
    sigma = get_sigma();
    DEBUG("Sigma is " << sigma.transpose());
    sigmaI = sigma.array().inverse();
    prefactor = data.pixel_size.value() * amplitude * transmission 
            / (2 * M_PI * sigma.x() * sigma.y());
    compute_prefactors();
    return true;
}

template <>
Eigen::Matrix<double,2,1> BaseEvaluator<double,No3D>::get_sigma() const {
        return (expr->best_sigma * expr->wavelength).cast<double>();
}
template <>
Eigen::Matrix<float,2,1> BaseEvaluator<float,No3D>::get_sigma() const {
        return (expr->best_sigma * expr->wavelength).cast<float>();
}

template <typename Number, typename Model>
Eigen::Matrix<Number,2,1> BaseEvaluator<Number,Model>::get_sigma() const {
    BOOST_STATIC_ASSERT (( boost::is_same<Model,Zhuang>::value ));
    Eigen::Vector2d relative_z = 
        (Eigen::Vector2d::Constant( expr->zposition - expr->axial_mean ) - expr->z_offset);
    return ((relative_z.array().square() * expr->delta_sigma.array() + expr->best_sigma.array()) * expr->wavelength).template cast<Number>();
}

template <typename Number, typename Model>
void BaseEvaluator<Number,Model>::compute_prefactors() {
    BOOST_STATIC_ASSERT (( boost::is_same<Model,Zhuang>::value ));
    relative_z = 
        (Eigen::Vector2d::Constant( expr->zposition - expr->axial_mean ) - expr->z_offset)
            .template cast<Number>();
    z_deriv_prefactor = - 2 * (relative_z.array() * 
        (expr->delta_sigma.template cast<Number>().array() * sigmaI.array())) * wavelength;
    delta_z_deriv_prefactor = (relative_z.array().square() 
        * sigmaI.array()) * wavelength;
}

template <>
void BaseEvaluator<float,No3D>::compute_prefactors() {
#ifndef NDEBUG
        relative_z.fill( std::numeric_limits<float>::quiet_NaN() );
        z_deriv_prefactor = relative_z;
        delta_z_deriv_prefactor = relative_z;
#endif
}
template <>
void BaseEvaluator<double,No3D>::compute_prefactors() {
#ifndef NDEBUG
        relative_z.fill( std::numeric_limits<double>::quiet_NaN() );
        z_deriv_prefactor = relative_z;
        delta_z_deriv_prefactor = relative_z;
#endif
}

template class BaseEvaluator< double, PSF::No3D >;
template class BaseEvaluator< double, PSF::Zhuang >;
template class BaseEvaluator< float, PSF::No3D >;
template class BaseEvaluator< float, PSF::Zhuang >;

}
}
}
