#include "nonlinfit/sum/VariableMap.hpp"

#include <boost/mpl/vector.hpp>
#include <cassert>
#include <Eigen/Core>
#include <boost/test/unit_test.hpp>

namespace nonlinfit {
namespace sum {

struct test_functor {
    typedef std::pair<int,int> result_type;
    result_type operator()( int function, int parameter ) const {
        return result_type( function - function % 2, parameter % 2 );
    }
};

static void test_VariableMap()
{
    std::vector<bool> bits(5, false);
    bits[0] = true;
    bits[1] = true;
    bits[4] = true;

    VariableMap map(10,bits);
    
    BOOST_CHECK_EQUAL( map.function_count(), 10 );
    BOOST_CHECK_EQUAL( map.output_variable_count(), 23 );
    BOOST_CHECK_EQUAL( map(0,0), 0 );
    BOOST_CHECK_EQUAL( map(9,0), 0 );
    BOOST_CHECK_EQUAL( map(0,2), 2 );
    BOOST_CHECK_NE( map(0,2), map(7,2) );
    BOOST_CHECK_NE( map(0,3), map(7,3) );
    BOOST_CHECK_EQUAL( map(0,4), map(7,4) );

    VariableMap map2(5);
    for (int i = 0; i < 10; ++i) map2.add_function(test_functor() );
    BOOST_CHECK_EQUAL( map2.function_count(), 10 );
    BOOST_CHECK_EQUAL( map2.output_variable_count(), 10 );
    BOOST_CHECK_EQUAL( map2(9,4), map2(8,0) );
    BOOST_CHECK_NE( map2(9,4), map2(8,1) );
};

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "sum" );
    rv->add( BOOST_TEST_CASE( &test_VariableMap ) );
    return rv;
}

}
}
