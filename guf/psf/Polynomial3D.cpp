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
