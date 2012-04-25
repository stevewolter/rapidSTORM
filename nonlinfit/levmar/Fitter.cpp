#include <cassert>
#include <nonlinfit/functions/Polynom.h>
#include <nonlinfit/VectorPosition.h>
#include <nonlinfit/terminators/StepLimit.h>
#include "Fitter.hpp"
#include <boost/test/unit_test.hpp>

/** \cond */

namespace nonlinfit {
namespace levmar {

static void check_naive_fitter_finds_paraboles_minimum() {
    typedef nonlinfit::Bind< static_power::Expression, static_power::BaseValue > Lambda;
    Lambda a;
    static_power::SimpleFunction<0,1> function(a);

    a( static_power::Variable() ) = 5;
    a( static_power::Power() ) = 2;
    a( static_power::Prefactor() ) = 2;

    Config config;
    Fitter fitter(config);
    VectorPosition< Lambda > mover(a);
    terminators::StepLimit terminator(150);
    fitter.fit( function, mover, terminator );
    BOOST_CHECK_SMALL( a( static_power::Variable() ).value(), 0.01 );
}

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "levmar" );
    rv->add( BOOST_TEST_CASE( &check_naive_fitter_finds_paraboles_minimum ) );
    return rv;
}

}
}

/** \endcond */
