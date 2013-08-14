#include "AbstractMap.hpp"
#include <boost/mpl/vector.hpp>
#include <boost/units/systems/si/length.hpp>
#include <cassert>
#include <Eigen/Core>
#include "Evaluator.hpp"
#include <nonlinfit/functions/Constant.h>
#include <nonlinfit/functions/Polynom.h>
#include <nonlinfit/functions/Zero.h>
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/sum/Lambda.h>
#include <boost/test/unit_test.hpp>

namespace nonlinfit {
namespace sum {

struct test_functor {
    typedef std::pair<int,int> result_type;
    result_type operator()( int function, int parameter ) const {
        return result_type( function - function % 2, parameter % 2 );
    }
};

static void test_AbstractMap()
{
    std::bitset<5> bits;
    bits[0] = 1;
    bits[1] = 1;
    bits[4] = 1;

    AbstractMap<5> map(10,bits);
    
    BOOST_CHECK_EQUAL( map.function_count(), 10 );
    BOOST_CHECK_EQUAL( map.output_variable_count(), 23 );
    BOOST_CHECK_EQUAL( map(0,0), 0 );
    BOOST_CHECK_EQUAL( map(9,0), 0 );
    BOOST_CHECK_EQUAL( map(0,2), 2 );
    BOOST_CHECK_NE( map(0,2), map(7,2) );
    BOOST_CHECK_NE( map(0,3), map(7,3) );
    BOOST_CHECK_EQUAL( map(0,4), map(7,4) );

    AbstractMap<5> map2;
    for (int i = 0; i < 10; ++i) map2.add_function( test_functor() );
    BOOST_CHECK_EQUAL( map2.function_count(), 10 );
    BOOST_CHECK_EQUAL( map2.output_variable_count(), 10 );
    BOOST_CHECK_EQUAL( map2(9,4), map2(8,0) );
    BOOST_CHECK_NE( map2(9,4), map2(8,1) );
};

static void test_Evaluator()
{
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, constant::Expression, constant::Expression > > 
        MyExpression;
    typedef get_evaluator< MyExpression, 
        plane::xs_joint<double,1>::type >::type MyEvaluator;
    MyExpression m;
    m.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    m.get_part( boost::mpl::int_<1>() )( constant::Amount() ) = 4.0;
    m.get_part( boost::mpl::int_<2>() )( constant::Amount() ) = 8.0;
    MyEvaluator eval(m);

    bool can_evaluate = eval.prepare_iteration( 42 );
    BOOST_CHECK(( can_evaluate ));

    int dummy;
    eval.prepare_chunk( dummy );

    Eigen::Matrix< double, 1, 1 > derivatives;
    eval.derivative( derivatives.col(0), TermParameter< boost::mpl::int_<0>, constant::Amount >() );
    BOOST_CHECK_EQUAL( derivatives(0,0), 1.0 );
    eval.derivative( derivatives.col(0), TermParameter< boost::mpl::int_<2>, constant::Amount >() );
    BOOST_CHECK_EQUAL( derivatives(0,0), 1.0 );

    Eigen::Matrix< double, 1, 1 > value;
    eval.value( value );
    BOOST_CHECK_EQUAL( value(0,0), 17.0 );
}

static void test_Expression() {
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, zero::Expression, static_power::Expression > > 
        MyExpression;
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, zero::Expression, static_power::Expression, constant::Expression, static_power::Expression, zero::Expression, zero::Expression > > 
        MyOtherExpression;
    BOOST_STATIC_ASSERT(( boost::mpl::size< MyExpression::Variables >::type::value == 4 ));
    BOOST_STATIC_ASSERT(( boost::mpl::size< MyOtherExpression::Variables >::type::value == 8 ));
    MyExpression m;
    m.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    m.get_part( boost::mpl::int_<2>() )( static_power::Prefactor() ) = 1.0;
    const MyExpression& m2 = m;
    BOOST_CHECK_EQUAL( m2.get_part( boost::mpl::int_<0>() )( constant::Amount() ), 5.0 );
    BOOST_CHECK_EQUAL( m2.get_part( boost::mpl::int_<2>() )( static_power::Prefactor() ), 1.0 );

    MyOtherExpression mom;
    mom.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    mom.get_part( boost::mpl::int_<3>() )( constant::Amount() ) = 15.0;
    const MyOtherExpression& mom2 = mom;
    BOOST_CHECK_EQUAL( mom2.get_part( boost::mpl::int_<0>() )( constant::Amount() ), 5.0 );
    BOOST_CHECK_EQUAL( mom2.get_part( boost::mpl::int_<3>() )( constant::Amount() ), 15.0 );
}

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "sum" );
    rv->add( BOOST_TEST_CASE( &test_AbstractMap ) );
    rv->add( BOOST_TEST_CASE( &test_Evaluator ) );
    rv->add( BOOST_TEST_CASE( &test_Expression ) );
    return rv;
}

}
}
