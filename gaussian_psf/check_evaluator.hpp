#ifndef DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP
#define DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP

#include <Eigen/StdVector>
#include <boost/test/unit_test.hpp>

#include "fit_window/chunkify.hpp"
#include "gaussian_psf/DisjointEvaluator.h"
#include "gaussian_psf/JointEvaluator.h"
#include "gaussian_psf/mock_model.h"
#include "gaussian_psf/ReferenceEvaluation.h"
#include "guf/PlaneFunction.hpp"
#include "nonlinfit/plane/check_evaluator.hpp"
#include "nonlinfit/plane/create_term.hpp"
#include "nonlinfit/plane/DisjointData.h"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/plane/Joint.h"

namespace dStorm {
namespace gaussian_psf {

BOOST_TEST_CASE_TEMPLATE_FUNCTION( check_evaluator_with_tag, Info ) {
    typedef typename boost::mpl::at_c< Info, 0 >::type Model;
    typedef typename boost::mpl::at_c< Info, 1 >::type MLE;
    typedef typename boost::mpl::at_c< Info, 2 >::type Data;
    typedef nonlinfit::plane::xs_joint<double, 1>::type RefTag;

    fit_window::Plane plane = mock_data();
    Model z = mock_model<Model>();
    typename guf::PlaneFunction<RefTag>::Evaluators ref_evaluators;
    ref_evaluators.push_back(nonlinfit::plane::create_term(z, RefTag()));
    typename guf::PlaneFunction<Data>::Evaluators tested_evaluators;
    tested_evaluators.push_back(nonlinfit::plane::create_term(z, Data()));
    std::auto_ptr<guf::FitFunction> ref =
        guf::PlaneFunction<RefTag>::create(std::move(ref_evaluators), plane, MLE::value);
    std::auto_ptr<guf::FitFunction> test =
        guf::PlaneFunction<Data>::create(std::move(tested_evaluators), plane, MLE::value);

    bool is_same = nonlinfit::plane::compare_evaluators<double>(*ref->abstract_function(), *test->abstract_function());
    BOOST_CHECK( is_same );
}

template <typename Model>
void check_evaluator( boost::unit_test::test_suite* suite ) {
    typedef nonlinfit::plane::xs_joint<double, 8>::type Joint;
    typedef boost::mpl::vector< 
        boost::mpl::vector<Model, boost::mpl::false_, Joint>,
        boost::mpl::vector<Model, boost::mpl::true_, Joint>,
        boost::mpl::vector<Model, boost::mpl::false_, MockDataTag>,
        boost::mpl::vector<Model, boost::mpl::true_, MockDataTag>
    > DataTags;
    suite->add( BOOST_TEST_CASE_TEMPLATE( check_evaluator_with_tag, DataTags ) );
}


}
}

#endif
