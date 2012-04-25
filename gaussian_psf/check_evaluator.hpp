#ifndef DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP
#define DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP

#include <Eigen/StdVector>
#include <boost/test/unit_test.hpp>
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Distance.hpp>
#include "gaussian_psf/JointEvaluator.h"
#include "gaussian_psf/DisjointEvaluator.h"
#include <nonlinfit/plane/check_evaluator.hpp>
#include "ReferenceEvaluation.h"
#include "mock_model.h"

namespace dStorm {
namespace gaussian_psf {

BOOST_TEST_CASE_TEMPLATE_FUNCTION( check_evaluator_with_tag, Info ) {
    typedef typename boost::mpl::at_c< Info, 0 >::type Model;
    typedef typename boost::mpl::at_c< Info, 1 >::type Distance;
    typedef typename boost::mpl::at_c< Info, 2 >::type Data;
    typedef nonlinfit::plane::xs_joint<double,LengthUnit, 1>::type RefTag;
    MockDataTag::Data data = mock_data();
    Model z = mock_model<Model>();
    bool is_same = nonlinfit::plane::compare_evaluators< Distance, Data, RefTag >(z, data);
    BOOST_CHECK( is_same );
}

template <typename Model>
boost::unit_test::test_suite* check_evaluator( const char* name ) {
    typedef nonlinfit::plane::xs_joint<double,LengthUnit, 8>::type Joint;
    typedef boost::mpl::vector< 
        boost::mpl::vector<Model, nonlinfit::plane::squared_deviations, Joint>,
        boost::mpl::vector<Model, nonlinfit::plane::negative_poisson_likelihood, Joint>,
        boost::mpl::vector<Model, nonlinfit::plane::squared_deviations, MockDataTag>,
        boost::mpl::vector<Model, nonlinfit::plane::negative_poisson_likelihood, MockDataTag>
    > DataTags;
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( name );
    rv->add( BOOST_TEST_CASE_TEMPLATE( check_evaluator_with_tag, DataTags ) );
    return rv;
}


}
}

#endif
