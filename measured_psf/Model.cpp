#include <boost/test/unit_test.hpp>
#include "No3D.h"
#include "check_evaluator.hpp"

namespace dStorm {
namespace measured_psf {

template <typename Number>
boost::optional< Eigen::Array<Number,2,1> >
Parameters<Number,No3D>::compute_sigma_() {
    if ( (expr->best_sigma < 0).any() )
        return boost::optional< Eigen::Array<Number,2,1> >();
    else
        return Eigen::Array<Number,2,1>( expr->best_sigma.cast<Number>() );
}

template <typename Number>
void Parameters<Number,No3D>::compute_prefactors_() {
    this->sigma_deriv = expr->best_sigma.cast<Number>().inverse();
}

Eigen::Matrix< quantity<LengthUnit>, 2, 1 > No3D::get_sigma() const
{
    Parameters<double,No3D> evaluator(*this);
    return boost::units::from_value< LengthUnit >( evaluator.compute_sigma() );
}
template class Parameters< double, No3D >;
template class Parameters< float, No3D >;

template boost::unit_test::test_suite* check_evaluator<No3D>( const char* name );
}
}
