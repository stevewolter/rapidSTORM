#ifndef DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP
#define DSTORM_GAUSSIAN_PSF_CHECK_EVALUATOR_HPP

#include <Eigen/StdVector>
#include <boost/test/unit_test.hpp>
#include <nonlinfit/plane/DisjointData.h>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Distance.hpp>
#include "gaussian_psf/JointEvaluator.h"
#include "gaussian_psf/DisjointEvaluator.h"
#include <nonlinfit/plane/check_evaluator.hpp>
#include "gaussian_psf/ReferenceEvaluation.h"
#include "gaussian_psf/mock_model.h"
#include "fit_window/chunkify.hpp"
#include "guf/PlaneFunction.hpp"

namespace dStorm {
namespace gaussian_psf {

BOOST_TEST_CASE_TEMPLATE_FUNCTION( check_evaluator_with_tag, Info ) {
    typedef typename boost::mpl::at_c< Info, 0 >::type Model;
    typedef typename boost::mpl::at_c< Info, 1 >::type MLE;
    typedef typename boost::mpl::at_c< Info, 2 >::type Data;
    typedef nonlinfit::plane::xs_joint<double, 1>::type RefTag;

    fit_window::Plane plane = mock_data();
    Model z = mock_model<Model>();
    std::auto_ptr<nonlinfit::AbstractFunction<double>> ref =
        guf::PlaneFunction::create<Model, RefTag>(z, plane, MLE::value);
    std::auto_ptr<nonlinfit::AbstractFunction<double>> test =
        guf::PlaneFunction::create<Model, Data>(z, plane, MLE::value);

    bool is_same = nonlinfit::plane::compare_evaluators<double>(*ref, *test);
    BOOST_CHECK( is_same );
}

template <typename Model>
boost::unit_test::test_suite* check_evaluator( const char* name ) {
    typedef nonlinfit::plane::xs_joint<double, 8>::type Joint;
    typedef boost::mpl::vector< 
        boost::mpl::vector<Model, boost::mpl::false_, Joint>,
        boost::mpl::vector<Model, boost::mpl::true_, Joint>,
        boost::mpl::vector<Model, boost::mpl::false_, MockDataTag>,
        boost::mpl::vector<Model, boost::mpl::true_, MockDataTag>
    > DataTags;
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( name );
    rv->add( BOOST_TEST_CASE_TEMPLATE( check_evaluator_with_tag, DataTags ) );
    return rv;
}


}
}

#endif
