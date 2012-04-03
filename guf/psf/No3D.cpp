#include <Eigen/StdVector>
#include "unit_test.h"
#include "No3D.h"
#include "ReferenceEvaluation.h"
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Distance.hpp>
#include "guf/psf/Derivable.h"
#include "guf/psf/JointEvaluator.h"
#include "guf/psf/DisjointEvaluator.h"
#include <nonlinfit/plane/check_evaluator.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

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
template class Parameters< double, PSF::No3D >;
template class Parameters< float, PSF::No3D >;

using namespace nonlinfit::plane;

void check_no3d_evaluator( TestState& state ) {
    typedef xs_joint<double,LengthUnit, 8>::type Joint;
    typedef xs_joint<double,LengthUnit, 1>::type RefTag;
    MockDataTag::Data data = mock_data();
    No3D z = mock_model<No3D>();
    state.testrun( compare_evaluators< squared_deviations, Joint, RefTag >( z, data),
                   "No3D joint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, Joint, RefTag >( z, data),
                   "No3D joint ML evaluator works" );
    state.testrun( compare_evaluators< squared_deviations, MockDataTag, RefTag >( z, data),
                   "No3D disjoint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, MockDataTag, RefTag >( z, data),
                   "No3D disjoint ML evaluator works" );
}

}
}
}
