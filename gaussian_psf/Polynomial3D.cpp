#include "Polynomial3D.h"
#include "gaussian_psf/check_evaluator.hpp"
#include "nonlinfit/InvalidPositionError.h"

namespace dStorm {
namespace gaussian_psf {

struct InvalidThreeDFactorError 
    : public std::runtime_error, public nonlinfit::InvalidPositionError {
    InvalidThreeDFactorError() : std::runtime_error("Invalid sigma") {}
};

template <typename Number>
Eigen::Array<Number,2,1> Parameters<Number,Polynomial3D>::compute_sigma_() {
    relative_z = expr->zposition.array().cast<Number>() - Eigen::Array<Number,2,1>::Constant( expr->axial_mean );
    threed_factor = Eigen::Array<Number,2,1>::Constant(1);
    for (int term = 1; term <= Polynomial3D::Order; ++term)
        threed_factor += (relative_z / expr->delta_sigma.col(term).cast<Number>()).pow(term);
    DEBUG("Computed threed factor of " << threed_factor.transpose() << " from " << expr->delta_sigma.transpose() << " and " << relative_z.transpose());
    if ( (threed_factor <= 0).any() ) {
        throw InvalidThreeDFactorError();
    } else {
        return Eigen::Array<Number,2,1>(expr->best_sigma.array().cast< Number >() * threed_factor.sqrt());
    }
}

template <typename Number>
void Parameters<Number,Polynomial3D>::compute_prefactors_() {
    z_deriv_prefactor.fill(0);
    delta_z_deriv_prefactor.col(0).fill(0);
    Eigen::Array<Number,2,1> p = 0.5 * threed_factor.inverse(), common;
    for (int term = 1; term <= Polynomial3D::Order; ++term) {
        common = p * term * relative_z.pow(term-1) / expr->delta_sigma
            .col(term).cast<Number>().pow(term+1);
        delta_z_deriv_prefactor.col(term) = - common * relative_z;
        z_deriv_prefactor += common * expr->delta_sigma.col(term).cast<Number>();
    }
    assert( (delta_z_deriv_prefactor == delta_z_deriv_prefactor).all() );
    assert( (z_deriv_prefactor == z_deriv_prefactor).all() );
    this->sigma_deriv = expr->best_sigma.cast<Number>().inverse();
}

Eigen::Vector2d Polynomial3D::get_sigma() const
{
    Parameters<double,Polynomial3D> evaluator(*this);
    return evaluator.compute_sigma();
}

boost::units::quantity< Micrometers > Polynomial3D::get_delta_sigma( int dimension, int term) const
{
    return boost::units::quantity< Micrometers >::from_value( delta_sigma( dimension, term ) );
}

template class Parameters< double, Polynomial3D >;
template class Parameters< float, Polynomial3D >;

template boost::unit_test::test_suite* check_evaluator<Polynomial3D>( const char* name );

}
}
