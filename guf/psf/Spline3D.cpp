#include <Eigen/StdVector>
#include "unit_test.h"
#include "Spline3D.h"
#include "ReferenceEvaluation.h"
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Distance.hpp>
#include "guf/psf/Derivable.h"
#include "guf/psf/JointEvaluator.h"
#include "guf/psf/DisjointEvaluator.h"
#include <nonlinfit/plane/check_evaluator.hpp>
#include <dStorm/threed_info/Spline.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Number>
boost::optional< Eigen::Array<Number,2,1> > Parameters<Number,Spline3D>::compute_sigma_() 
{
    quantity<si::length> z( (*expr)( MeanZ() ) );
    Eigen::Array<Number,2,1> rv;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        threed_info::Spline::Sigma s = expr->spline->get_sigma(i, z);
        if ( s )
            rv[i] = quantity< BestSigma<0>::Unit >( *s ).value();
        else
            return boost::optional< Eigen::Array<Number,2,1> >();
    }
    return rv;
}

template <typename Number>
void Parameters<Number,Spline3D>::compute_prefactors_() {
    quantity<si::length> z( (*expr)( MeanZ() ) );
    this->sigma_deriv = this->sigmaI;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        z_deriv_prefactor[i] = - this->sigmaI[i] * 
            *expr->spline->get_sigma_deriv(i, z );
    }
}

Eigen::Matrix< quantity<LengthUnit>, 2, 1 > Spline3D::get_sigma() const
{
    Parameters<double,Spline3D> evaluator(*this);
    return boost::units::from_value< LengthUnit >( evaluator.compute_sigma() );
}


template class Parameters< double, PSF::Spline3D >;
template class Parameters< float, PSF::Spline3D >;

using namespace nonlinfit::plane;

void check_spline_evaluator( TestState& state ) {
    typedef  xs_joint<double,dStorm::guf::PSF::LengthUnit, 8>::type Joint;
    typedef  xs_joint<double,dStorm::guf::PSF::LengthUnit, 1>::type RefTag;
    MockDataTag::Data data = mock_data();
    Spline3D z = mock_model<Spline3D>();
    state.testrun( compare_evaluators< squared_deviations, Joint, RefTag >( z, data),
                   "Spline3D joint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, Joint, RefTag >( z, data),
                   "Spline3D joint ML evaluator works" );
    state.testrun( compare_evaluators< squared_deviations, MockDataTag, RefTag >( z, data),
                   "Spline3D disjoint LSQ evaluator works" );
    state.testrun( compare_evaluators< negative_poisson_likelihood, MockDataTag, RefTag >( z, data),
                   "Spline3D disjoint ML evaluator works" );
}

}
}
}
