#include <boost/test/unit_test.hpp>
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/check_evaluator.hpp"
#include "nonlinfit/InvalidPositionError.h"

namespace dStorm {
namespace gaussian_psf {

template <typename Number>
Eigen::Array<Number,2,1>
Parameters<Number,No3D>::compute_sigma_() {
    return Eigen::Array<Number,2,1>( expr->best_sigma.cast<Number>() );
}

template <typename Number>
void Parameters<Number,No3D>::compute_prefactors_() {
    this->sigma_deriv = expr->best_sigma.cast<Number>().inverse();
}

Eigen::Vector2d No3D::get_sigma() const
{
    Parameters<double,No3D> evaluator(*this);
    return evaluator.compute_sigma();
}
template class Parameters< double, No3D >;
template class Parameters< float, No3D >;

template boost::unit_test::test_suite* check_evaluator<No3D>( const char* name );
}
}
