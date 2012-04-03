#include <Eigen/StdVector>
#include "unit_test.h"
#include "Polynomial3D.h"
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
boost::optional< Eigen::Array<Number,2,1> > Parameters<Number,Polynomial3D>::compute_sigma_() {
    relative_z = expr->zposition.array().cast<Number>() - Eigen::Array<Number,2,1>::Constant( expr->axial_mean );
    threed_factor = Eigen::Array<Number,2,1>::Constant(1);
    for (int term = 1; term <= Polynomial3D::Order; ++term)
        threed_factor += (relative_z / expr->delta_sigma.col(term).cast<Number>()).pow(term);
    DEBUG("Computed threed factor of " << threed_factor.transpose() << " from " << expr->delta_sigma.transpose() << " and " << relative_z.transpose());
    if ( (threed_factor <= 0).any() || (expr->best_sigma <= 0).any() )
        return boost::optional< Eigen::Array<Number,2,1> >();
    else
        return Eigen::Array<Number,2,1>(expr->best_sigma.array().cast< Number >() * threed_factor.sqrt());
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

Eigen::Matrix< quantity<LengthUnit>, 2, 1 > Polynomial3D::get_sigma() const
{
    Parameters<double,Polynomial3D> evaluator(*this);
    return boost::units::from_value< LengthUnit >( evaluator.compute_sigma() );
}

template class Parameters< double, PSF::Polynomial3D >;
template class Parameters< float, PSF::Polynomial3D >;

using namespace nonlinfit::plane;

void check_zhuang_evaluator( TestState& state ) {
    typedef  xs_joint<double,dStorm::guf::PSF::LengthUnit, 8>::type Joint;
    typedef  xs_joint<double,dStorm::guf::PSF::LengthUnit, 1>::type RefTag;
    MockDataTag::Data data = mock_data();
    Polynomial3D z = mock_model<Polynomial3D>();
    state.testrun( compare_evaluators< squared_deviations, Joint, RefTag >( z, data),
                   "Polynomial3D joint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, Joint, RefTag >( z, data),
                   "Polynomial3D joint ML evaluator works" );
    state.testrun( compare_evaluators< squared_deviations, MockDataTag, RefTag >( z, data),
                   "Polynomial3D disjoint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, MockDataTag, RefTag >( z, data),
                   "Polynomial3D disjoint ML evaluator works" );
}

boost::units::quantity< Micrometers > Polynomial3D::get_delta_sigma( int dimension, int term) const
{
    return boost::units::quantity< Micrometers >::from_value( delta_sigma( dimension, term ) );
}

}
}
}
